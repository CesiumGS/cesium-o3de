#include "Cesium/TilesetUtility/RenderResourcesPreparer.h"
#include "Cesium/TilesetUtility/GltfRasterMaterialBuilder.h"
#include "Cesium/Gltf/GltfModelBuilder.h"
#include "Cesium/Gltf/GltfLoadContext.h"
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

#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <CesiumGltf/Model.h>
#include <CesiumUtility/JsonValue.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

namespace Cesium
{
    RenderResourcesPreparer::RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor)
        : m_meshFeatureProcessor{ meshFeatureProcessor }
        , m_transform{ 1.0 }
    {
        m_freeRasterLayers.reserve(GltfRasterMaterialBuilder::MAX_RASTER_LAYERS);
        for (std::uint32_t i = 0; i < GltfRasterMaterialBuilder::MAX_RASTER_LAYERS; ++i)
        {
            m_freeRasterLayers.emplace_back(i);
        }

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

    bool RenderResourcesPreparer::AddRasterLayer(const Cesium3DTilesSelection::RasterOverlay* rasterOverlay)
    {
        if (m_freeRasterLayers.empty())
        {
            return false;
        }

        if (m_rasterOverlayLayers.find(rasterOverlay) == m_rasterOverlayLayers.end())
        {
            m_rasterOverlayLayers.insert(AZStd::make_pair(rasterOverlay, m_freeRasterLayers.back()));
            m_freeRasterLayers.pop_back();
        }

        return true;
    }

    void RenderResourcesPreparer::RemoveRasterLayer(const Cesium3DTilesSelection::RasterOverlay* rasterOverlay)
    {
        auto layerIt = m_rasterOverlayLayers.find(rasterOverlay);
        if (layerIt == m_rasterOverlayLayers.end())
        {
            return;
        }

        m_rasterOverlayLayers.erase(layerIt);
        m_freeRasterLayers.emplace_back(layerIt->second);
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
            imageDesc.m_format = AZ::RHI::Format::R8G8B8A8_UNORM_SRGB;

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
                auto rasterOverlay = new RasterOverlay();
                rasterOverlay->m_imageAsset = std::move(imageAsset);
                return rasterOverlay;
            }
        }

        return nullptr;
    }

    void* RenderResourcesPreparer::prepareRasterInMainThread(
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile, void* pLoadThreadResult)
    {
        if (pLoadThreadResult)
        {
            auto rasterOverlay = reinterpret_cast<RasterOverlay*>(pLoadThreadResult);
            rasterOverlay->m_image = AZ::RPI::StreamingImage::FindOrCreate(rasterOverlay->m_imageAsset);
            return rasterOverlay;
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
            RasterOverlay* rasterOverlay = reinterpret_cast<RasterOverlay*>(pLoadThreadResult);
            delete rasterOverlay;
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
                const auto& currentRasterOverlay = rasterTile.getOverlay();
                auto layerIt = m_rasterOverlayLayers.find(&currentRasterOverlay);
                if (layerIt == m_rasterOverlayLayers.end())
                {
                    return;
                }
                std::uint32_t layer = layerIt->second;

                IntrusiveGltfModel* intrusiveGltfModel = reinterpret_cast<IntrusiveGltfModel*>(tileRenderResource);
                RasterOverlay* rasterOverlay = reinterpret_cast<RasterOverlay*>(mainThreadRasterResources);
                GltfRasterMaterialBuilder materialBuilder;
                GltfModel& model = intrusiveGltfModel->m_model;
                for (auto& material : model.GetMaterials())
                {
                    if (!material.m_material)
                    {
                        continue;
                    }

                    AZ::Vector4 uvTranslateScale{ static_cast<float>(translation.x), static_cast<float>(translation.y),
                                                  static_cast<float>(scale.x), static_cast<float>(scale.y) };

                    // Just update material with raster if the current material can compile, so material can be updated right away
                    // in the next frame. Otherwise, we create the new material with the attached raster, so that the primitive is
                    // updated with the new material in the next frame. If we only update the material and not create new material
                    // the terrain can be rendered with old material if that material is still compiling and flickering can happen
                    bool canCompile = material.m_material->CanCompile();
                    if (canCompile)
                    {
                        canCompile = materialBuilder.SetRasterForMaterial(
                            layer, rasterOverlay->m_image, static_cast<std::uint32_t>(overlayTextureCoordinateID), uvTranslateScale,
                            material.m_material);
                    }

                    if (!canCompile)
                    {
                        auto materialAsset = materialBuilder.CreateRasterMaterial(
                            layer, rasterOverlay->m_imageAsset, static_cast<std::uint32_t>(overlayTextureCoordinateID), uvTranslateScale,
                            material.m_material->GetAsset());
                        material.m_material = AZ::RPI::Material::FindOrCreate(materialAsset);
                    }
                }

                for (auto& mesh : model.GetMeshes())
                {
                    for (auto& primitive : mesh.m_primitives)
                    {
                        model.UpdateMaterialForPrimitive(primitive);
                    }
                }
            }
        }
    }

    void RenderResourcesPreparer::detachRasterInMainThread(
        const Cesium3DTilesSelection::Tile& tile,
        [[maybe_unused]] std::int32_t overlayTextureCoordinateID,
        const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        void* mainThreadRasterResources) noexcept
    {
        if (tile.getState() == Cesium3DTilesSelection::Tile::LoadState::Done)
        {
            void* tileRenderResource = tile.getRendererResources();
            if (tileRenderResource && mainThreadRasterResources)
            {
                // find the layer of the raster
                const auto& currentRasterOverlay = rasterTile.getOverlay();
                auto layerIt = m_rasterOverlayLayers.find(&currentRasterOverlay);
                if (layerIt == m_rasterOverlayLayers.end())
                {
                    return;
                }
                std::uint32_t layer = layerIt->second;

                IntrusiveGltfModel* intrusiveGltfModel = reinterpret_cast<IntrusiveGltfModel*>(tileRenderResource);
                GltfRasterMaterialBuilder materialBuilder;
                GltfModel& model = intrusiveGltfModel->m_model;
                for (auto& material : model.GetMaterials())
                {
                    if (!material.m_material)
                    {
                        continue;
                    }

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
