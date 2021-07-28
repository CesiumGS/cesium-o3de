// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfMaterialBuilder.h"
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Public/Image/StreamingImagePool.h>
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAssetCreator.h>
#include <Atom/RPI.Reflect/Image/ImageMipChainAssetCreator.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace Cesium
{
    AZ::Data::Asset<AZ::RPI::MaterialAsset> GltfMaterialBuilder::Create(
        [[maybe_unused]] const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext)
    {
        // Load StandardPBR material type
        auto standardPBRMaterialType = AZ::RPI::AssetUtils::LoadCriticalAsset<AZ::RPI::MaterialTypeAsset>(STANDARD_PBR_MAT_TYPE);

        // Create PBR Material dynamically
        AZ::Data::Asset<AZ::RPI::MaterialAsset> standardPBRMaterial;
        AZ::RPI::MaterialAssetCreator materialCreator;
        materialCreator.Begin(AZ::Uuid::CreateRandom(), *standardPBRMaterialType);

        // configure pbr mettalic roughness
        const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
        if (pbrMetallicRoughness)
        {
            const std::vector<double>& baseColorFactor = pbrMetallicRoughness->baseColorFactor;
            if (baseColorFactor.size() == 4)
            {
                materialCreator.SetPropertyValue(
                    AZ::Name("baseColor.color"),
                    AZ::Color(
                        static_cast<float>(baseColorFactor[0]), static_cast<float>(baseColorFactor[1]),
                        static_cast<float>(baseColorFactor[2]), static_cast<float>(baseColorFactor[3])));
            }

            const std::optional<CesiumGltf::TextureInfo>& baseColorTexture = pbrMetallicRoughness->baseColorTexture;
            AZ::Data::Asset<AZ::RPI::ImageAsset> baseColorImage;
            std::int32_t baseColorTexCoord = -1;
            if (baseColorTexture)
            {
                baseColorImage = GetOrCreateImage(model, *baseColorTexture, loadContext);
                baseColorTexCoord = static_cast<std::int32_t>(baseColorTexture->texCoord);
            }

            if (baseColorImage && baseColorTexCoord >= 0 && baseColorTexCoord < 2)
            {
                materialCreator.SetPropertyValue(AZ::Name("baseColor.useTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("baseColor.textureMap"), baseColorImage);
                materialCreator.SetPropertyValue(AZ::Name("baseColor.textureMapUv"), static_cast<std::uint32_t>(baseColorTexCoord));
            }
            else
            {
                materialCreator.SetPropertyValue(AZ::Name("baseColor.useTexture"), false);
            }

            // configure metallic and roughness
            double metallicFactor = pbrMetallicRoughness->metallicFactor;
            materialCreator.SetPropertyValue(AZ::Name("metallic.factor"), static_cast<float>(metallicFactor));

            double roughnessFactor = pbrMetallicRoughness->roughnessFactor;
            materialCreator.SetPropertyValue(AZ::Name("roughness.factor"), static_cast<float>(roughnessFactor));

            const std::optional<CesiumGltf::TextureInfo>& metallicRoughnessTexture = pbrMetallicRoughness->metallicRoughnessTexture; 
            AZ::Data::Asset<AZ::RPI::ImageAsset> metallicRoughnessImage;
            std::int32_t metallicRoughnessTexCoord = -1;
            if (metallicRoughnessTexture)
            {
                metallicRoughnessImage = GetOrCreateImage(model, *metallicRoughnessTexture, loadContext);
                metallicRoughnessTexCoord = static_cast<std::int32_t>(metallicRoughnessTexture->texCoord);
            }

            if (metallicRoughnessImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
            {
                materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("metallic.textureMap"), metallicRoughnessImage);
                materialCreator.SetPropertyValue(AZ::Name("metallic.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));

                materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("roughness.textureMap"), metallicRoughnessImage);
                materialCreator.SetPropertyValue(AZ::Name("roughness.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
            }
            else
            {
                materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), false);
                materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), false);
            }
        }

        materialCreator.End(standardPBRMaterial);

        return standardPBRMaterial;
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfMaterialBuilder::GetOrCreateImage(
        const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, [[maybe_unused]] GltfLoadContext& loadContext)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        const CesiumGltf::ImageCesium& imageData = image->cesium;
        AZ::RHI::Format imageFormat = AZ::RHI::Format::Unknown;
        if (imageData.bytesPerChannel != 1)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        if (imageData.channels == 1)
        {
            imageFormat = AZ::RHI::Format::R8_UNORM;
        }
        else if (imageData.channels == 2)
        {
            imageFormat = AZ::RHI::Format::R8G8_UNORM;
        }
        else if (imageData.channels == 4)
        {
            imageFormat = AZ::RHI::Format::R8G8B8A8_UNORM;
        }
        else
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        AZ::RHI::ImageDescriptor imageDesc;
        imageDesc.m_bindFlags = AZ::RHI::ImageBindFlags::ShaderRead;
        imageDesc.m_dimension = AZ::RHI::ImageDimension::Image2D;
        imageDesc.m_size = AZ::RHI::Size(static_cast<std::uint32_t>(imageData.width), static_cast<std::uint32_t>(imageData.height), 1);
        imageDesc.m_format = imageFormat;

        AZ::RHI::ImageSubresourceLayout imageSubresourceLayout = AZ::RHI::GetImageSubresourceLayout(imageDesc, AZ::RHI::ImageSubresource{});

        // Create mip chain
        AZ::RPI::ImageMipChainAssetCreator mipChainCreator;
        mipChainCreator.Begin(AZ::Uuid::CreateRandom(), 1, 1);
        mipChainCreator.BeginMip(imageSubresourceLayout);
        mipChainCreator.AddSubImage(imageData.pixelData.data(), imageData.pixelData.size());
        mipChainCreator.EndMip();
        AZ::Data::Asset<AZ::RPI::ImageMipChainAsset> mipChainAsset;
        mipChainCreator.End(mipChainAsset);

        // Create streaming image
        AZ::RPI::StreamingImageAssetCreator imageCreator;
        imageCreator.Begin(AZ::Uuid::CreateRandom());
        imageCreator.SetImageDescriptor(imageDesc);
        imageCreator.AddMipChainAsset(*mipChainAsset);
        imageCreator.SetFlags(AZ::RPI::StreamingImageFlags::NotStreamable);
        imageCreator.SetPoolAssetId(AZ::RPI::ImageSystemInterface::Get()->GetSystemStreamingPool()->GetAssetId());
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset;
        imageCreator.End(imageAsset);

        return imageAsset;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")

