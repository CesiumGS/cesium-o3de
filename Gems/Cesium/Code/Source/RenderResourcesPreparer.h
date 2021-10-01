#pragma once

#include "GltfModel.h"
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <Atom/Utils/StableDynamicArray.h>
#include <AzCore/std/optional.h>
#include <glm/glm.hpp>

namespace AZ
{
    namespace Render
    {
        class MeshFeatureProcessorInterface;
    }
}

namespace CesiumGltf
{
    struct Model;
}

namespace Cesium
{
    struct IntrusiveGltfModel
    {
        IntrusiveGltfModel(GltfModel&& model)
            : m_model{ std::move(model) }
        {
        }

        GltfModel m_model;
        AZ::StableDynamicArrayHandle<IntrusiveGltfModel> m_self;
    };

    class RenderResourcesPreparer : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
        RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor);

        ~RenderResourcesPreparer() noexcept;

        void SetTransform(const glm::dmat4& transform);

        const glm::dmat4& GetTransform() const;

        void SetVisible(void* renderResources, bool visible);

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
        AZStd::optional<glm::dvec3> GetRTCFromGltf(const CesiumGltf::Model& model);

        static constexpr char* CESIUM_RTC_CENTER_EXTRA = "RTC_CENTER";

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZ::StableDynamicArray<IntrusiveGltfModel> m_intrusiveModels;
        glm::dmat4 m_transform;
    };
} // namespace Cesium
