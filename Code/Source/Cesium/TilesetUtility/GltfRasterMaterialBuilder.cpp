#include "Cesium/TilesetUtility/GltfRasterMaterialBuilder.h"
#include "Cesium/Systems/CesiumSystem.h"
#include "Cesium/Systems/CriticalAssetManager.h"
#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>

namespace Cesium
{
    GltfRasterMaterialBuilder::GltfRasterMaterialBuilder()
    {
        const auto& defaultMaterialType = CesiumInterface::Get()->GetCriticalAssetManager().m_rasterMaterialType;
        m_pbrMaterialBuilder.OverrideMaterialType(defaultMaterialType);
    }

    const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GltfRasterMaterialBuilder::GetDefaultMaterialType() const
    {
        return CesiumInterface::Get()->GetCriticalAssetManager().m_rasterMaterialType;
    }

    void GltfRasterMaterialBuilder::OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType)
    {
        m_overrideMaterialTypeAsset = materialType;
        m_pbrMaterialBuilder.OverrideMaterialType(materialType);
    }

    void GltfRasterMaterialBuilder::Create(
        const CesiumGltf::Model& model,
        const CesiumGltf::Material& material,
        AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
        GltfLoadMaterial& result)
    {
        m_pbrMaterialBuilder.Create(model, material, textureCache, result);

        // add custom attributes to the material, so that primitive builder will know how to find attributes from gltf primitive to send
        // them to GPU correctly
        result.m_customVertexAttributes.insert_or_assign(
            "_CESIUMOVERLAY_0",
            GltfShaderVertexAttribute(AZ::RHI::ShaderSemantic("UV", 2), AZ::Name("m_raster_uv0"), AZ::RHI::Format::R32G32_FLOAT));
        result.m_customVertexAttributes.insert_or_assign(
            "_CESIUMOVERLAY_1",
            GltfShaderVertexAttribute(AZ::RHI::ShaderSemantic("UV", 3), AZ::Name("m_raster_uv1"), AZ::RHI::Format::R32G32_FLOAT));
    }

    AZ::Data::Asset<AZ::RPI::MaterialAsset> GltfRasterMaterialBuilder::CreateRasterMaterial(
        std::uint32_t rasterLayer,
        const AZ::Data::Asset<AZ::RPI::ImageAsset>& raster,
        std::uint32_t textureUv,
        const AZ::Vector4& uvTranslateScale,
        const AZ::Data::Asset<AZ::RPI::MaterialAsset>& parent)
    {
        AZStd::string prefix = AZStd::string::format("raster%d", rasterLayer);

        AZ::RPI::MaterialAssetCreator materialCreator;
        materialCreator.Begin(AZ::Uuid::CreateRandom(), parent, true);
        materialCreator.SetPropertyValue(AZ::Name(prefix + ".textureMap"), raster);
        materialCreator.SetPropertyValue(AZ::Name(prefix + ".useTexture"), true);
        materialCreator.SetPropertyValue(AZ::Name(prefix + ".textureMapUv"), textureUv);
        materialCreator.SetPropertyValue(AZ::Name(prefix + ".uvTranslateScale"), uvTranslateScale);

        AZ::Data::Asset<AZ::RPI::MaterialAsset> materialAsset;
        materialCreator.End(materialAsset);

        return materialAsset;
    }

    bool GltfRasterMaterialBuilder::SetRasterForMaterial(
        std::uint32_t rasterLayer,
        const AZ::Data::Instance<AZ::RPI::Image>& raster,
        std::uint32_t textureUv,
        const AZ::Vector4& uvTranslateScale,
        AZ::Data::Instance<AZ::RPI::Material>& material)
    {
        AZStd::string prefix = AZStd::string::format("raster%d", rasterLayer);

        auto rasterMapIndex = material->FindPropertyIndex(AZ::Name(prefix + ".textureMap"));
        material->SetPropertyValue(rasterMapIndex, raster);

        auto useRasterMapIndex = material->FindPropertyIndex(AZ::Name(prefix + ".useTexture"));
        material->SetPropertyValue(useRasterMapIndex, true);

        auto textureMapUvIndex = material->FindPropertyIndex(AZ::Name(prefix + ".textureMapUv"));
        material->SetPropertyValue(textureMapUvIndex, textureUv);

        auto uvTranslateScaleIndex = material->FindPropertyIndex(AZ::Name(prefix + ".uvTranslateScale"));
        material->SetPropertyValue(uvTranslateScaleIndex, uvTranslateScale);

        return material->Compile();
    }

    bool GltfRasterMaterialBuilder::UnsetRasterForMaterial(std::uint32_t rasterLayer, AZ::Data::Instance<AZ::RPI::Material>& material)
    {
        AZStd::string prefix = AZStd::string::format("raster%d", rasterLayer);

        auto rasterMapIndex = material->FindPropertyIndex(AZ::Name(prefix + ".textureMap"));
        material->SetPropertyValue(rasterMapIndex, AZ::Data::Asset<AZ::RPI::ImageAsset>());

        auto useRasterMapIndex = material->FindPropertyIndex(AZ::Name(prefix + ".useTexture"));
        material->SetPropertyValue(useRasterMapIndex, false);

        auto textureMapUvIndex = material->FindPropertyIndex(AZ::Name(prefix + ".textureMapUv"));
        material->SetPropertyValue(textureMapUvIndex, static_cast<std::uint32_t>(0));

        auto uvTranslateScaleIndex = material->FindPropertyIndex(AZ::Name(prefix + ".uvTranslateScale"));
        material->SetPropertyValue(uvTranslateScaleIndex, AZ::Vector4(0.0, 0.0, 1.0, 1.0));

        return material->Compile();
    }
} // namespace Cesium
