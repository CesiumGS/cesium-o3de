#pragma once

#include <Cesium3DTiles/IPrepareRendererResources.h>

namespace AZ
{
    namespace Render
    {
        class MeshFeatureProcessorInterface;
    }
}

namespace Cesium
{
    class RenderResourcesPreparer : public Cesium3DTiles::IPrepareRendererResources
    {
    public:
        RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor);

        void* prepareInLoadThread(const CesiumGltf::Model& model, const glm::dmat4& transform) override;

        void* prepareInMainThread(Cesium3DTiles::Tile& tile, void* pLoadThreadResult) override;

        void free(Cesium3DTiles::Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept override;

        void* prepareRasterInLoadThread(const CesiumGltf::ImageCesium& image) override;

        void* prepareRasterInMainThread(const Cesium3DTiles::RasterOverlayTile& rasterTile, void* pLoadThreadResult) override;

        void freeRaster(
            const Cesium3DTiles::RasterOverlayTile& rasterTile, void* pLoadThreadResult, void* pMainThreadResult) noexcept override;

        void attachRasterInMainThread(
            const Cesium3DTiles::Tile& tile,
            std::uint32_t overlayTextureCoordinateID,
            const Cesium3DTiles::RasterOverlayTile& rasterTile,
            void* pMainThreadRendererResources,
            const CesiumGeometry::Rectangle& textureCoordinateRectangle,
            const glm::dvec2& translation,
            const glm::dvec2& scale) override;

        void detachRasterInMainThread(
            const Cesium3DTiles::Tile& tile,
            std::uint32_t overlayTextureCoordinateID,
            const Cesium3DTiles::RasterOverlayTile& rasterTile,
            void* pMainThreadRendererResources,
            const CesiumGeometry::Rectangle& textureCoordinateRectangle) noexcept override;

    private:
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
    };
} // namespace Cesium
