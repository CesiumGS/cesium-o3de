#pragma once

#include "GltfModel.h"
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/Utils/StableDynamicArray.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/containers/vector.h>
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
    struct LoadRasterOverlay
    {
        LoadRasterOverlay(AZ::Data::Asset<AZ::RPI::StreamingImageAsset>&& imageAsset)
            : m_imageAsset{ std::move(imageAsset) }
        {
        }

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_imageAsset;
    };

    struct RasterOverlay
    {
        RasterOverlay(AZ::Data::Instance<AZ::RPI::StreamingImage>&& image)
            : m_image{ std::move(image) }
        {
        }

        AZ::Data::Instance<AZ::RPI::StreamingImage> m_image;
    };

    struct IntrusiveGltfModel
    {
        IntrusiveGltfModel(GltfModel&& model)
            : m_model{ std::move(model) }
        {
        }

        GltfModel m_model;
        AZ::StableDynamicArrayHandle<IntrusiveGltfModel> m_self;
    };

    class RenderResourcesPreparer : public Cesium3DTilesSelection::IPrepareRendererResources, public AZ::TickBus::Handler
    {
    public:
        RenderResourcesPreparer(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor);

        ~RenderResourcesPreparer() noexcept;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

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
            std::int32_t overlayTextureCoordinateID,
            const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
            void* mainThreadRasterResources,
            const glm::dvec2& translation,
            const glm::dvec2& scale) override;

        void detachRasterInMainThread(
            const Cesium3DTilesSelection::Tile& tile,
            std::int32_t overlayTextureCoordinateID,
            const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
            void* mainThreadRasterResources) noexcept override;

    private:
        AZStd::optional<glm::dvec3> GetRTCFromGltf(const CesiumGltf::Model& model);

        static constexpr char* CESIUM_RTC_CENTER_EXTRA = "RTC_CENTER";

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZ::StableDynamicArray<IntrusiveGltfModel> m_intrusiveModels;
        AZStd::vector<AZ::Data::Instance<AZ::RPI::Material>> m_compileMaterialsQueue;
        glm::dmat4 m_transform;
    };
} // namespace Cesium
