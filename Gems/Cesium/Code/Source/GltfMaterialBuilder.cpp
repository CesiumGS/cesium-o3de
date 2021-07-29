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
            ConfigurePbrMetallicRoughness(model, *pbrMetallicRoughness, materialCreator, loadContext);
        }

        // configure occlusion
        const std::optional<CesiumGltf::MaterialOcclusionTextureInfo> occlusionTexture = material.occlusionTexture;
        if (occlusionTexture)
        {
            AZ::Data::Asset<AZ::RPI::ImageAsset> occlusionImage = GetOrCreateOcclusionImage(model, *occlusionTexture, loadContext);
            std::int64_t occlusionTexCoord = occlusionTexture->texCoord;
            if (occlusionImage && occlusionTexCoord >= 0 && occlusionTexCoord < 2)
            {
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseUseTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseTextureMap"), occlusionImage);
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseTextureMapUv"), static_cast<std::uint32_t>(occlusionTexCoord));
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseFactor"), static_cast<float>(occlusionTexture->strength));
            }
        }

        // configure emissive 
        bool enableEmissive = false;
        if (material.emissiveFactor.size() == 3)
        {
            bool isBlack = std::all_of(
                material.emissiveFactor.begin(), material.emissiveFactor.end(),
                [](double c)
                {
                    return c == 0.0;
                });

            if (!isBlack)
            {
                enableEmissive = true;
                materialCreator.SetPropertyValue(
                    AZ::Name("emissive.color"),
                    AZ::Color{ static_cast<float>(material.emissiveFactor[0]), static_cast<float>(material.emissiveFactor[1]),
                               static_cast<float>(material.emissiveFactor[2]), 1.0f });
            }
        }

        const std::optional<CesiumGltf::TextureInfo>& emissiveTexture = material.emissiveTexture;
        if (emissiveTexture)
        {
            AZ::Data::Asset<AZ::RPI::ImageAsset> emissiveImage = GetOrCreateRGBAImage(model, *emissiveTexture, loadContext);
            std::int64_t emissiveTexCoord = emissiveTexture->texCoord;
            if (emissiveImage && emissiveTexCoord >= 0 && emissiveTexCoord < 2)
            {
                enableEmissive = true;
                materialCreator.SetPropertyValue(AZ::Name("emissive.useTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("emissive.textureMap"), emissiveImage);
                materialCreator.SetPropertyValue(AZ::Name("emissive.textureMapUv"), static_cast<std::uint32_t>(emissiveTexCoord));
            }
            else
            {
                materialCreator.SetPropertyValue(AZ::Name("emissive.useTexture"), false);
            }
        }

        if (enableEmissive)
        {
            materialCreator.SetPropertyValue(AZ::Name("emissive.enable"), true);
        }

        // set opacity
        if (material.alphaMode == CesiumGltf::Material::AlphaMode::OPAQUE)
        {
            materialCreator.SetPropertyValue(AZ::Name("opacity.mode"), static_cast<std::uint32_t>(0));
        }
        else if (material.alphaMode == CesiumGltf::Material::AlphaMode::MASK)
        {
            materialCreator.SetPropertyValue(AZ::Name("opacity.mode"), static_cast<std::uint32_t>(1));
            materialCreator.SetPropertyValue(AZ::Name("opacity.factor"), static_cast<float>(1.0 - material.alphaCutoff));
        }
        else if (material.alphaMode == CesiumGltf::Material::AlphaMode::BLEND)
        {
            materialCreator.SetPropertyValue(AZ::Name("opacity.mode"), static_cast<std::uint32_t>(2));
        }

        if (material.doubleSided)
        {
            materialCreator.SetPropertyValue(AZ::Name("opacity.doubleSided"), true);
        }

        materialCreator.End(standardPBRMaterial);

        return standardPBRMaterial;
    }

    void GltfMaterialBuilder::ConfigurePbrMetallicRoughness(
        const CesiumGltf::Model& model,
        const CesiumGltf::MaterialPBRMetallicRoughness& pbrMetallicRoughness,
        AZ::RPI::MaterialAssetCreator& materialCreator,
        GltfLoadContext& loadContext)
    {
        const std::vector<double>& baseColorFactor = pbrMetallicRoughness.baseColorFactor;
        if (baseColorFactor.size() == 4)
        {
            materialCreator.SetPropertyValue(
                AZ::Name("baseColor.color"),
                AZ::Color(
                    static_cast<float>(baseColorFactor[0]), static_cast<float>(baseColorFactor[1]), static_cast<float>(baseColorFactor[2]),
                    static_cast<float>(baseColorFactor[3])));
        }

        const std::optional<CesiumGltf::TextureInfo>& baseColorTexture = pbrMetallicRoughness.baseColorTexture;
        AZ::Data::Asset<AZ::RPI::ImageAsset> baseColorImage;
        std::int64_t baseColorTexCoord = -1;
        if (baseColorTexture)
        {
            baseColorImage = GetOrCreateRGBAImage(model, *baseColorTexture, loadContext);
            baseColorTexCoord = baseColorTexture->texCoord;
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
        double metallicFactor = pbrMetallicRoughness.metallicFactor;
        materialCreator.SetPropertyValue(AZ::Name("metallic.factor"), static_cast<float>(metallicFactor));

        double roughnessFactor = pbrMetallicRoughness.roughnessFactor;
        materialCreator.SetPropertyValue(AZ::Name("roughness.factor"), static_cast<float>(roughnessFactor));

        const std::optional<CesiumGltf::TextureInfo>& metallicRoughnessTexture = pbrMetallicRoughness.metallicRoughnessTexture;
        AZ::Data::Asset<AZ::RPI::ImageAsset> metallicImage;
        AZ::Data::Asset<AZ::RPI::ImageAsset> roughnessImage;
        std::int64_t metallicRoughnessTexCoord = -1;
        if (metallicRoughnessTexture)
        {
            GetOrCreateMetallicRoughnessImage(model, *metallicRoughnessTexture, metallicImage, roughnessImage, loadContext);
            metallicRoughnessTexCoord = metallicRoughnessTexture->texCoord;
        }

        if (metallicImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), true);
            materialCreator.SetPropertyValue(AZ::Name("metallic.textureMap"), metallicImage);
            materialCreator.SetPropertyValue(AZ::Name("metallic.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
        }
        else
        {
            materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), false);
        }

        if (roughnessImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), true);
            materialCreator.SetPropertyValue(AZ::Name("roughness.textureMap"), roughnessImage);
            materialCreator.SetPropertyValue(AZ::Name("roughness.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
        }
        else
        {
            materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), false);
        }
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfMaterialBuilder::GetOrCreateOcclusionImage(
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

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::size_t width = static_cast<std::size_t>(imageData.width);
        std::size_t height = static_cast<std::size_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 1)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        // Do the fast path. Copy the whole data over
        if (imageData.channels == 1)
        {
            return Create2DImage(imageData.pixelData.data(), imageData.pixelData.size(), width, height, AZ::RHI::Format::R8_UNORM);
        }

        // Just copy the red channel
        std::size_t j = 0;
        AZStd::vector<std::byte> pixels(width * height);
        for (std::size_t i = 0; i < imageData.pixelData.size(); i += imageData.channels * imageData.bytesPerChannel)
        {
            pixels[j] = imageData.pixelData[i];
            ++j;
        }

        return Create2DImage(pixels.data(), pixels.size(), width, height, AZ::RHI::Format::R8_UNORM);
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfMaterialBuilder::GetOrCreateRGBAImage(
        const CesiumGltf::Model& model,
        const CesiumGltf::TextureInfo& textureInfo,
        [[maybe_unused]] GltfLoadContext& loadContext)
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

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::size_t width = static_cast<std::size_t>(imageData.width);
        std::size_t height = static_cast<std::size_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 3 || imageData.channels > 4)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        if (imageData.channels == 3)
        {
            AZStd::vector<std::byte> pixels(width * height * 4);
            for (std::size_t i = 0; i < imageData.pixelData.size(); i += 3)
            {
                pixels[i] = imageData.pixelData[i];
                pixels[i + 1] = imageData.pixelData[i + 1];
                pixels[i + 2] = imageData.pixelData[i + 2];
                pixels[i + 3] = static_cast<std::byte>(255);
            }

            return Create2DImage(pixels.data(), pixels.size(), width, height, AZ::RHI::Format::R8G8B8A8_UNORM);
        }
        else
        {
            return Create2DImage(imageData.pixelData.data(), imageData.pixelData.size(), width, height, AZ::RHI::Format::R8G8B8A8_UNORM);
        }
    }

    void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
        const CesiumGltf::Model& model,
        const CesiumGltf::TextureInfo& textureInfo,
        AZ::Data::Asset<AZ::RPI::ImageAsset>& metallic,
        AZ::Data::Asset<AZ::RPI::ImageAsset>& roughness,
        [[maybe_unused]] GltfLoadContext& loadContext)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return;
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return;
        }

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return;
        }

        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::size_t width = static_cast<std::size_t>(imageData.width);
        std::size_t height = static_cast<std::size_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 3 || imageData.channels > 4)
        {
            return;
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return;
        }

        std::size_t j = 0;
        AZStd::vector<std::byte> metallicPixels(width * height);
        AZStd::vector<std::byte> roughnessPixels(width * height);
        for (std::size_t i = 0; i < imageData.pixelData.size(); i += imageData.channels * imageData.bytesPerChannel)
        {
            roughnessPixels[j] = imageData.pixelData[i + 1];
            metallicPixels[j] = imageData.pixelData[i + 2];
            ++j;
        }

        metallic = Create2DImage(metallicPixels.data(), metallicPixels.size(), width, height, AZ::RHI::Format::R8_UNORM);
        roughness = Create2DImage(roughnessPixels.data(), roughnessPixels.size(), width, height, AZ::RHI::Format::R8_UNORM);
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfMaterialBuilder::Create2DImage(
        const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format)
    {
        // image has 4 channels, so we just copy the data over
        AZ::RHI::ImageDescriptor imageDesc;
        imageDesc.m_bindFlags = AZ::RHI::ImageBindFlags::ShaderRead;
        imageDesc.m_dimension = AZ::RHI::ImageDimension::Image2D;
        imageDesc.m_size = AZ::RHI::Size(width, height, 1);
        imageDesc.m_format = format;

        AZ::RHI::ImageSubresourceLayout imageSubresourceLayout = AZ::RHI::GetImageSubresourceLayout(imageDesc, AZ::RHI::ImageSubresource{});

        // Create mip chain
        AZ::RPI::ImageMipChainAssetCreator mipChainCreator;
        mipChainCreator.Begin(AZ::Uuid::CreateRandom(), 1, 1);
        mipChainCreator.BeginMip(imageSubresourceLayout);
        mipChainCreator.AddSubImage(pixelData, bytesPerImage);
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

