#include "RenderResourcesPreparer.h"
#include "GltfModelBuilder.h"
#include "GltfLoadContext.h"
#include <CesiumUtility/JsonValue.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
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
        , m_transform{1.0}
    {
    }

    RenderResourcesPreparer::~RenderResourcesPreparer() noexcept
    {
        for (auto& intrusiveModel : m_intrusiveModels)
        {
            // move the handler out before free it. Otherwise, stack overflow
            auto handler = std::move(intrusiveModel.m_self); 
            handler.Free();
        }
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
        GltfModelBuilderOption option{transform};
        AZStd::optional<glm::dvec3> rtc = GetRTCFromGltf(model);
        if (rtc)
        {
            option.m_transform = glm::translate(transform, rtc.value());
        }

        // build model
        AZStd::unique_ptr<GltfLoadModel> loadModel = AZStd::make_unique<GltfLoadModel>();
        GltfModelBuilder builder;
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

    void RenderResourcesPreparer::free([[maybe_unused]] Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept
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

    void* RenderResourcesPreparer::prepareRasterInLoadThread([[maybe_unused]] const CesiumGltf::ImageCesium& image)
    {
        return nullptr;
    }

    void* RenderResourcesPreparer::prepareRasterInMainThread(
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile, [[maybe_unused]] void* pLoadThreadResult)
    {
        return nullptr;
    }

    void RenderResourcesPreparer::freeRaster(
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        [[maybe_unused]] void* pLoadThreadResult,
        [[maybe_unused]] void* pMainThreadResult) noexcept
    {
    }

    void RenderResourcesPreparer::attachRasterInMainThread(
        [[maybe_unused]] const Cesium3DTilesSelection::Tile& tile,
        [[maybe_unused]] std::uint32_t overlayTextureCoordinateID,
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        [[maybe_unused]] void* pMainThreadRendererResources,
        [[maybe_unused]] const CesiumGeometry::Rectangle& textureCoordinateRectangle,
        [[maybe_unused]] const glm::dvec2& translation,
        [[maybe_unused]] const glm::dvec2& scale)
    {
    }

    void RenderResourcesPreparer::detachRasterInMainThread(
        [[maybe_unused]] const Cesium3DTilesSelection::Tile& tile,
        [[maybe_unused]] std::uint32_t overlayTextureCoordinateID,
        [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
        [[maybe_unused]] void* pMainThreadRendererResources,
        [[maybe_unused]] const CesiumGeometry::Rectangle& textureCoordinateRectangle) noexcept
    {
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

        glm::dvec3 rtc{0.0};
        rtc.x = array[0].getDoubleOrDefault(0.0);
        rtc.y = array[1].getDoubleOrDefault(0.0);
        rtc.z = array[2].getDoubleOrDefault(0.0);
        return rtc;
    }
} // namespace Cesium
