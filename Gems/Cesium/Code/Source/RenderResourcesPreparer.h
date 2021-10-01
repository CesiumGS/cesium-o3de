#pragma once

#include <Cesium3DTilesSelection/IPrepareRendererResources.h>

namespace AZ
{
    namespace Render
    {
        class MeshFeatureProcessorInterface;
    }
}

namespace Cesium
{
    class RenderResourcesPreparer : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
        RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor);

        void* prepareInLoadThread(const CesiumGltf::Model& model, const glm::dmat4& transform) override;

        void* prepareInMainThread(Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult) override;

        void free(Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept override;

        void* prepareRasterInLoadThread(const CesiumGltf::ImageCesium& image) override;

        void* prepareRasterInMainThread(const Cesium3DTilesSelection::RasterOverlayTile& rasterTile, void* pLoadThreadResult) override;

        void freeRaster(
            const Cesium3DTilesSelection::RasterOverlayTile& rasterTile, void* pLoadThreadResult, void* pMainThreadResult) noexcept override;

        void attachRasterInMainThread(
            const Cesium3DTilesSelection::Tile& tile,
            std::uint32_t overlayTextureCoordinateID,
            const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
            void* pMainThreadRendererResources,
            const CesiumGeometry::Rectangle& textureCoordinateRectangle,
            const glm::dvec2& translation,
            const glm::dvec2& scale) override;

        void detachRasterInMainThread(
            const Cesium3DTilesSelection::Tile& tile,
            std::uint32_t overlayTextureCoordinateID,
            const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
            void* pMainThreadRendererResources,
            const CesiumGeometry::Rectangle& textureCoordinateRectangle) noexcept override;

    private:
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
    };
} // namespace Cesium
