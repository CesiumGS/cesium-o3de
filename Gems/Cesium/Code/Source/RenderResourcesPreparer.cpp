#include "RenderResourcesPreparer.h"
#include "GltfModelBuilder.h"
#include "GltfModel.h"
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

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
    {
    }

    void* RenderResourcesPreparer::prepareInLoadThread(const CesiumGltf::Model& model, const glm::dmat4& transform)
    {
        AZStd::unique_ptr<GltfLoadModel> loadModel = AZStd::make_unique<GltfLoadModel>();
        GltfModelBuilder builder;
        GltfModelBuilderOption option{transform};
        builder.Create(model, option, *loadModel);
        return loadModel.release();
    }

    void* RenderResourcesPreparer::prepareInMainThread([[maybe_unused]] Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult)
    {
        if (pLoadThreadResult)
        {
            GltfLoadModel* loadModel = reinterpret_cast<GltfLoadModel*>(pLoadThreadResult);
            AZStd::unique_ptr<GltfModel> model = AZStd::make_unique<GltfModel>(m_meshFeatureProcessor, *loadModel);
            model->SetVisible(false);
            return model.release();
        }

        return nullptr;
    }

    void RenderResourcesPreparer::free([[maybe_unused]] Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept
    {
        if (pLoadThreadResult)
        {
            delete pLoadThreadResult;
        }

        if (pMainThreadResult)
        {
            delete pMainThreadResult;
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
} // namespace Cesium
