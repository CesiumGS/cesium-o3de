#include "RenderResourcesPreparer.h"
#include "GltfModelBuilder.h"
#include "GltfRasterMaterialBuilder.h"
#include "GltfLoadContext.h"
#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <CesiumUtility/JsonValue.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAssetCreator.h>
#include <Atom/RPI.Reflect/Image/ImageMipChainAssetCreator.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/algorithm.h>
#include <glm/gtc/matrix_transform.hpp>

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif


namespace Cesium
{
    RenderResourcesPreparer::RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor)
        : m_meshFeatureProcessor{ meshFeatureProcessor }
        , m_transform{ 1.0 }
    {
        AZ::TickBus::Handler::BusConnect();
    }

    RenderResourcesPreparer::~RenderResourcesPreparer() noexcept
    {
        AZ::TickBus::Handler::BusDisconnect();

        for (auto& intrusiveModel : m_intrusiveModels)
        {
            // move the handler out before free it. Otherwise, stack overflow
            auto handler = std::move(intrusiveModel.m_self);
            handler.Free();
        }
    }

    void RenderResourcesPreparer::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        auto it = AZStd::remove_if(
            m_compileMaterialsQueue.begin(), m_compileMaterialsQueue.end(),
            [](auto& material)
            {
                return !material->NeedsCompile() || material->Compile();
            });
        m_compileMaterialsQueue.erase(it, m_compileMaterialsQueue.end());
    }

    void RenderResourcesPreparer::SetTransform(const glm::dmat4& transform)
    {
        m_transform = transform;
        for (auto& intrusiveModel : m_intrusiveModels)
        {
            intrusiveModel.m_model.SetTransform(transform);
        }
    }

    const glm::dmat4& RenderResourcesPreparer::GetTransform() const
    {
        return m_transform;
    }

    void RenderResourcesPreparer::SetVisible(void* renderResources, bool visible)
    {
        if (renderResources)
        {
            IntrusiveGltfModel* intrusiveModel = reinterpret_cast<IntrusiveGltfModel*>(renderResources);
            if (intrusiveModel->m_model.IsVisible() != visible)
            {
                intrusiveModel->m_model.SetVisible(visible);
            }
        }
    }

    void* RenderResourcesPreparer::prepareInLoadThread(const CesiumGltf::Model& model, const glm::dmat4& transform)
    {
        // set option for model loaders. Especially RTC
        GltfModelBuilderOption option{ transform };
        AZStd::optional<glm::dvec3> rtc = GetRTCFromGltf(model);
        if (rtc)
        {
            option.m_transform = glm::translate(transform, rtc.value());
        }

        // build model
        AZStd::unique_ptr<GltfLoadModel> loadModel = AZStd::make_unique<GltfLoadModel>();
        GltfModelBuilder builder(AZStd::make_unique<GltfRasterMaterialBuilder>());
        builder.Create(model, option, *loadModel);
        return loadModel.release();
    }

    void* RenderResourcesPreparer::prepareInMainThread([[maybe_unused]] Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult)
    {
        if (pLoadThreadResult)
        {
            // we destroy loadModel after main thread is done
            AZStd::unique_ptr<GltfLoadModel> loadModel{ reinterpret_cast<GltfLoadModel*>(pLoadThreadResult) };
            auto handle = m_intrusiveModels.emplace(GltfModel(m_meshFeatureProcessor, *loadModel));
            IntrusiveGltfModel& intrusiveModel = *handle;
            intrusiveModel.m_self = std::move(handle);
            intrusiveModel.m_model.SetTransform(m_transform);
            intrusiveModel.m_model.SetVisible(false);
            return &intrusiveModel;
        }

        return nullptr;
    }

    void RenderResourcesPreparer::free(
        [[maybe_unused]] Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept
    {
        if (pLoadThreadResult)
        {
            GltfLoadModel* loadModel = reinterpret_cast<GltfLoadModel*>(pLoadThreadResult);
            delete loadModel;
        }

        if (pMainThreadResult)
        {
            IntrusiveGltfModel* intrusiveModel = reinterpret_cast<IntrusiveGltfModel*>(pMainThreadResult);
            auto handler = std::move(intrusiveModel->m_self); // move the handler out before free it. Otherwise, stack overflow
            handler.Free();
        }
    }

    void* RenderResourcesPreparer::prepareRasterInLoadThread(const CesiumGltf::ImageCesium& image)
    {
        if (!image.pixelData.empty() && image.width != 0 && image.height != 0)
        {
            // image has 4 channels, so we just copy the data over
            AZ::RHI::ImageDescriptor imageDesc;
            imageDesc.m_bindFlags = AZ::RHI::ImageBindFlags::ShaderRead;
            imageDesc.m_dimension = AZ::RHI::ImageDimension::Image2D;
            imageDesc.m_size = AZ::RHI::Size(image.width, image.height, 1);
            imageDesc.m_format = AZ::RHI::Format::R8G8B8A8_UNORM;

            AZ::RHI::ImageSubresourceLayout imageSubresourceLayout =
                AZ::RHI::GetImageSubresourceLayout(imageDesc, AZ::RHI::ImageSubresource{});

            // Create mip chain
            AZ::RPI::ImageMipChainAssetCreator mipChainCreator;
            mipChainCreator.Begin(AZ::Uuid::CreateRandom(), 1, 1);
            mipChainCreator.BeginMip(imageSubresourceLayout);
            mipChainCreator.AddSubImage(image.pixelData.data(), image.pixelData.size());
            mipChainCreator.EndMip();
            AZ::Data::Asset<AZ::RPI::ImageMipChainAsset> mipChainAsset;
            mipChainCreator.End(mipChainAsset);

            // Create streaming image
            AZ::RPI::StreamingImageAssetCreator imageCreator;
            imageCreator.Begin(AZ::Uuid::CreateRandom());
            imageCreator.SetImageDescriptor(imageDesc);
            imageCreator.AddMipChainAsset(*mipChainAsset);

            AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset;
            imageCreator.End(imageAsset);

            if (imageAsset)
            {
                return new LoadRasterOverlay{ std::move(imageAsset) };
            }
        }

        return nullptr;
    }

    void* RenderResourcesPreparer::prepareRasterInMainThread(
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile, void* pLoadThreadResult)
    {
        if (pLoadThreadResult)
        {
            AZStd::unique_ptr<LoadRasterOverlay> loadRasterOverlay(reinterpret_cast<LoadRasterOverlay*>(pLoadThreadResult));
            AZ::Data::Instance<AZ::RPI::StreamingImage> image = AZ::RPI::StreamingImage::FindOrCreate(loadRasterOverlay->m_imageAsset);

            if (image)
            {
                return new RasterOverlay{ std::move(image) };
            }
        }

        return nullptr;
    }

    void RenderResourcesPreparer::freeRaster(
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        void* pLoadThreadResult,
        void* pMainThreadResult) noexcept
    {
        if (pLoadThreadResult)
        {
            LoadRasterOverlay* loadRasterOverlay = reinterpret_cast<LoadRasterOverlay*>(pLoadThreadResult);
            delete loadRasterOverlay;
        }

        if (pMainThreadResult)
        {
            RasterOverlay* rasterOverlay = reinterpret_cast<RasterOverlay*>(pMainThreadResult);
            delete rasterOverlay;
        }
    }

    void RenderResourcesPreparer::attachRasterInMainThread(
        const Cesium3DTilesSelection::Tile& tile,
        std::int32_t overlayTextureCoordinateID,
        const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        void* mainThreadRasterResources,
        const glm::dvec2& translation,
        const glm::dvec2& scale)
    {
        if (tile.getState() == Cesium3DTilesSelection::Tile::LoadState::Done)
        {
            void* tileRenderResource = tile.getRendererResources();
            if (tileRenderResource && mainThreadRasterResources)
            {
                // find the layer of the raster
                const auto& tileset = tile.getTileset();
                const auto& rasterOverlays = tileset->getOverlays();
                const auto& currentRasterOverlay = rasterTile.getOverlay();
                auto it = AZStd::find_if(
                    rasterOverlays.begin(), rasterOverlays.end(),
                    [&currentRasterOverlay](const auto& overlay)
                    {
                        return overlay.get() == &currentRasterOverlay;
                    });
                std::uint32_t layer = static_cast<std::uint32_t>(it - rasterOverlays.begin());

                IntrusiveGltfModel* intrusiveGltfModel = reinterpret_cast<IntrusiveGltfModel*>(tileRenderResource);
                RasterOverlay* rasterOverlay = reinterpret_cast<RasterOverlay*>(mainThreadRasterResources);
                GltfRasterMaterialBuilder materialBuilder;
                GltfModel& model = intrusiveGltfModel->m_model;
                for (auto& material : model.GetMaterials())
                {
                    AZ::Vector4 uvTranslateScale{ static_cast<float>(translation.x), static_cast<float>(translation.y),
                                                  static_cast<float>(scale.x), static_cast<float>(scale.y) };
                    bool compile = materialBuilder.SetRasterForMaterial(
                        layer, rasterOverlay->m_image, static_cast<std::uint32_t>(overlayTextureCoordinateID), uvTranslateScale,
                        material.m_material);

                    // it's not guaranteed that the material will be able to compile right away, so we add it to the queue to compile later
                    if (!compile)
                    {
                        m_compileMaterialsQueue.emplace_back(material.m_material);
                    }
                }
            }
        }
    }

    void RenderResourcesPreparer::detachRasterInMainThread(
        const Cesium3DTilesSelection::Tile& tile,
        [[maybe_unused]] std::int32_t overlayTextureCoordinateID,
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        void* mainThreadRasterResources) noexcept
    {
        if (tile.getState() == Cesium3DTilesSelection::Tile::LoadState::Done)
        {
            void* tileRenderResource = tile.getRendererResources();
            if (tileRenderResource && mainThreadRasterResources)
            {
                // find the layer of the raster
                const auto& tileset = tile.getTileset();
                const auto& rasterOverlays = tileset->getOverlays();
                const auto& currentRasterOverlay = rasterTile.getOverlay();
                auto it = AZStd::find_if(
                    rasterOverlays.begin(), rasterOverlays.end(),
                    [&currentRasterOverlay](const auto& overlay)
                    {
                        return overlay.get() == &currentRasterOverlay;
                    });
                std::uint32_t layer = static_cast<std::uint32_t>(it - rasterOverlays.begin());

                IntrusiveGltfModel* intrusiveGltfModel = reinterpret_cast<IntrusiveGltfModel*>(tileRenderResource);
                GltfRasterMaterialBuilder materialBuilder;
                GltfModel& model = intrusiveGltfModel->m_model;
                for (auto& material : model.GetMaterials())
                {
                    bool compile = materialBuilder.UnsetRasterForMaterial(layer, material.m_material);

                    // it's not guaranteed that the material will be able to compile right away, so we add it to the queue to compile later
                    if (!compile)
                    {
                        m_compileMaterialsQueue.emplace_back(material.m_material);
                    }
                }
            }
        }
    }

    AZStd::optional<glm::dvec3> RenderResourcesPreparer::GetRTCFromGltf(const CesiumGltf::Model& model)
    {
        const CesiumUtility::JsonValue& extras = model.extras;
        const CesiumUtility::JsonValue* rtcObj = extras.getValuePtrForKey(CESIUM_RTC_CENTER_EXTRA);
        if (!rtcObj)
        {
            return AZStd::nullopt;
        }

        if (!rtcObj->isArray())
        {
            return AZStd::nullopt;
        }

        const auto& array = rtcObj->getArray();
        if (array.size() != 3)
        {
            return AZStd::nullopt;
        }

        glm::dvec3 rtc{ 0.0 };
        rtc.x = array[0].getDoubleOrDefault(0.0);
        rtc.y = array[1].getDoubleOrDefault(0.0);
        rtc.z = array[2].getDoubleOrDefault(0.0);
        return rtc;
    }
} // namespace Cesium
