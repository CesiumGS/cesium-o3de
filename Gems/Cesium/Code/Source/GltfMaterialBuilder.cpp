// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfMaterialBuilder.h"
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAssetCreator.h>
#include <Atom/RPI.Reflect/Image/ImageMipChainAssetCreator.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace Cesium
{
    GltfLoadMaterial GltfMaterialBuilder::Create(
        const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext)
    {
        // Load StandardPBR material type
        auto standardPBRMaterialType = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::MaterialTypeAsset>(STANDARD_PBR_MAT_TYPE);

        // Create PBR Material dynamically
        AZ::Data::Asset<AZ::RPI::MaterialAsset> standardPBRMaterialAsset;
        AZ::RPI::MaterialAssetCreator materialCreator;
        materialCreator.Begin(AZ::Uuid::CreateRandom(), *standardPBRMaterialType);
        materialCreator.End(standardPBRMaterialAsset);

        AZ::Data::Instance<AZ::RPI::Material> standardPBRMaterial = AZ::RPI::Material::Create(standardPBRMaterialAsset);

        // configure pbr mettalic roughness
        const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
        if (pbrMetallicRoughness)
        {
            ConfigurePbrMetallicRoughness(model, *pbrMetallicRoughness, standardPBRMaterial, loadContext);
        }

        // configure occlusion
        const std::optional<CesiumGltf::MaterialOcclusionTextureInfo> occlusionTexture = material.occlusionTexture;
        if (occlusionTexture)
        {
            AZ::Data::Instance<AZ::RPI::Image> occlusionImage = GetOrCreateOcclusionImage(model, *occlusionTexture, loadContext);
            std::int64_t occlusionTexCoord = occlusionTexture->texCoord;
            if (occlusionImage && occlusionTexCoord >= 0 && occlusionTexCoord < 2)
            {
                SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("occlusion.diffuseUseTexture"), true);
                SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("occlusion.diffuseTextureMap"), occlusionImage);
                SetMaterialPropertyValue(
                    standardPBRMaterial, AZ::Name("occlusion.diffuseTextureMapUv"), static_cast<std::uint32_t>(occlusionTexCoord));
                SetMaterialPropertyValue(
                    standardPBRMaterial, AZ::Name("occlusion.diffuseFactor"), static_cast<float>(occlusionTexture->strength));
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
                SetMaterialPropertyValue(
                    standardPBRMaterial, AZ::Name("emissive.color"),
                    AZ::Color{ static_cast<float>(material.emissiveFactor[0]), static_cast<float>(material.emissiveFactor[1]),
                               static_cast<float>(material.emissiveFactor[2]), 1.0f });
            }
        }

        const std::optional<CesiumGltf::TextureInfo>& emissiveTexture = material.emissiveTexture;
        if (emissiveTexture)
        {
            AZ::Data::Instance<AZ::RPI::Image> emissiveImage = GetOrCreateRGBAImage(model, *emissiveTexture, loadContext);
            std::int64_t emissiveTexCoord = emissiveTexture->texCoord;
            if (emissiveImage && emissiveTexCoord >= 0 && emissiveTexCoord < 2)
            {
                enableEmissive = true;
                SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("emissive.useTexture"), true);
                SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("emissive.textureMap"), emissiveImage);
                SetMaterialPropertyValue(
                    standardPBRMaterial, AZ::Name("emissive.textureMapUv"), static_cast<std::uint32_t>(emissiveTexCoord));
            }
            else
            {
                SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("emissive.useTexture"), false);
            }
        }

        if (enableEmissive)
        {
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("emissive.enable"), true);
        }

        // set opacity
        if (material.alphaMode == CesiumGltf::Material::AlphaMode::OPAQUE)
        {
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("opacity.mode"), static_cast<std::uint32_t>(0));
        }
        else if (material.alphaMode == CesiumGltf::Material::AlphaMode::MASK)
        {
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("opacity.mode"), static_cast<std::uint32_t>(1));
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("opacity.factor"), static_cast<float>(1.0 - material.alphaCutoff));
        }
        else if (material.alphaMode == CesiumGltf::Material::AlphaMode::BLEND)
        {
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("opacity.mode"), static_cast<std::uint32_t>(2));
        }

        if (material.doubleSided)
        {
            SetMaterialPropertyValue(standardPBRMaterial, AZ::Name("opacity.doubleSided"), true);
        }

        if (standardPBRMaterial->NeedsCompile())
        {
            standardPBRMaterial->Compile();
        }

        return GltfLoadMaterial(std::move(standardPBRMaterial), false);
    }

    void GltfMaterialBuilder::ConfigurePbrMetallicRoughness(
        const CesiumGltf::Model& model,
        const CesiumGltf::MaterialPBRMetallicRoughness& pbrMetallicRoughness,
        AZ::Data::Instance<AZ::RPI::Material>& material,
        GltfLoadContext& loadContext)
    {
        const std::vector<double>& baseColorFactor = pbrMetallicRoughness.baseColorFactor;
        if (baseColorFactor.size() == 4)
        {
            SetMaterialPropertyValue(
                material,
                AZ::Name("baseColor.color"),
                AZ::Color(
                    static_cast<float>(baseColorFactor[0]), static_cast<float>(baseColorFactor[1]), static_cast<float>(baseColorFactor[2]),
                    static_cast<float>(baseColorFactor[3])));
        }

        const std::optional<CesiumGltf::TextureInfo>& baseColorTexture = pbrMetallicRoughness.baseColorTexture;
        AZ::Data::Instance<AZ::RPI::Image> baseColorImage;
        std::int64_t baseColorTexCoord = -1;
        if (baseColorTexture)
        {
            baseColorImage = GetOrCreateRGBAImage(model, *baseColorTexture, loadContext);
            baseColorTexCoord = baseColorTexture->texCoord;
        }

        if (baseColorImage && baseColorTexCoord >= 0 && baseColorTexCoord < 2)
        {
            SetMaterialPropertyValue(material, AZ::Name("baseColor.useTexture"), true);
            SetMaterialPropertyValue(material, AZ::Name("baseColor.textureMap"), baseColorImage);
            SetMaterialPropertyValue(material, AZ::Name("baseColor.textureMapUv"), static_cast<std::uint32_t>(baseColorTexCoord));
        }
        else
        {
            SetMaterialPropertyValue(material, AZ::Name("baseColor.useTexture"), false);
        }

        // configure metallic and roughness
        double metallicFactor = pbrMetallicRoughness.metallicFactor;
        SetMaterialPropertyValue(material, AZ::Name("metallic.factor"), static_cast<float>(metallicFactor));

        double roughnessFactor = pbrMetallicRoughness.roughnessFactor;
        SetMaterialPropertyValue(material, AZ::Name("roughness.factor"), static_cast<float>(roughnessFactor));

        const std::optional<CesiumGltf::TextureInfo>& metallicRoughnessTexture = pbrMetallicRoughness.metallicRoughnessTexture;
        AZ::Data::Instance<AZ::RPI::Image> metallicImage;
        AZ::Data::Instance<AZ::RPI::Image> roughnessImage;
        std::int64_t metallicRoughnessTexCoord = -1;
        if (metallicRoughnessTexture)
        {
            GetOrCreateMetallicRoughnessImage(model, *metallicRoughnessTexture, metallicImage, roughnessImage, loadContext);
            metallicRoughnessTexCoord = metallicRoughnessTexture->texCoord;
        }

        if (metallicImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            SetMaterialPropertyValue(material, AZ::Name("metallic.useTexture"), true);
            SetMaterialPropertyValue(material, AZ::Name("metallic.textureMap"), metallicImage);
            SetMaterialPropertyValue(material, AZ::Name("metallic.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
        }
        else
        {
            SetMaterialPropertyValue(material, AZ::Name("metallic.useTexture"), false);
        }

        if (roughnessImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            SetMaterialPropertyValue(material, AZ::Name("roughness.useTexture"), true);
            SetMaterialPropertyValue(material, AZ::Name("roughness.textureMap"), roughnessImage);
            SetMaterialPropertyValue(material, AZ::Name("roughness.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
        }
        else
        {
            SetMaterialPropertyValue(material, AZ::Name("roughness.useTexture"), false);
        }
    }

    AZ::Data::Instance<AZ::RPI::Image> GltfMaterialBuilder::GetOrCreateOcclusionImage(
        const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, GltfLoadContext& loadContext)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        // Lookup cache
        std::uint32_t imageSourceIdx = static_cast<std::uint32_t>(texture->source);
        auto cachedAsset = loadContext.FindCachedImage(imageSourceIdx, imageSourceIdx);
        if (cachedAsset)
        {
            return cachedAsset;
        }

        // Create new asset
        AZ::Data::Instance<AZ::RPI::Image> newImage;
        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::size_t width = static_cast<std::size_t>(imageData.width);
        std::size_t height = static_cast<std::size_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 1)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        // Do the fast path. Copy the whole data over
        if (imageData.channels == 1)
        {
            newImage = Create2DImage(imageData.pixelData.data(), imageData.pixelData.size(), width, height, AZ::RHI::Format::R8_UNORM);
        }
        else
        {
            // Just copy the red channel
            std::size_t j = 0;
            AZStd::vector<std::byte> pixels(width * height);
            for (std::size_t i = 0; i < imageData.pixelData.size(); i += imageData.channels * imageData.bytesPerChannel)
            {
                pixels[j] = imageData.pixelData[i];
                ++j;
            }

            newImage = Create2DImage(pixels.data(), pixels.size(), width, height, AZ::RHI::Format::R8_UNORM);
        }

        loadContext.StoreImage(imageSourceIdx, imageSourceIdx, newImage);
        return newImage;
    }

    AZ::Data::Instance<AZ::RPI::Image> GltfMaterialBuilder::GetOrCreateRGBAImage(
        const CesiumGltf::Model& model,
        const CesiumGltf::TextureInfo& textureInfo,
        GltfLoadContext& loadContext)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        // Lookup cache
        std::uint32_t imageSourceIdx = static_cast<std::uint32_t>(texture->source);
        auto cachedImage = loadContext.FindCachedImage(imageSourceIdx, imageSourceIdx);
        if (cachedImage)
        {
            return cachedImage;
        }

        // Create a new asset if cache doesn't have it
        AZ::Data::Instance<AZ::RPI::Image> newImage;
        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::size_t width = static_cast<std::size_t>(imageData.width);
        std::size_t height = static_cast<std::size_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 3 || imageData.channels > 4)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
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

            newImage = Create2DImage(pixels.data(), pixels.size(), width, height, AZ::RHI::Format::R8G8B8A8_UNORM);
        }
        else
        {
            newImage = Create2DImage(imageData.pixelData.data(), imageData.pixelData.size(), width, height, AZ::RHI::Format::R8G8B8A8_UNORM);
        }

        loadContext.StoreImage(imageSourceIdx, imageSourceIdx, newImage);
        return newImage;
    }

    void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
        const CesiumGltf::Model& model,
        const CesiumGltf::TextureInfo& textureInfo,
        AZ::Data::Instance<AZ::RPI::Image>& metallic,
        AZ::Data::Instance<AZ::RPI::Image>& roughness,
        GltfLoadContext& loadContext)
    {
        static const std::uint32_t roughnessTextureSubIdx = 0;
        static const std::uint32_t metallicTextureSubIdx = 1;

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

        // Lookup cache
        std::uint32_t imageSourceIdx = static_cast<std::uint32_t>(texture->source);
        auto cachedRoughness = loadContext.FindCachedImage(imageSourceIdx, roughnessTextureSubIdx);
        auto cachedMetallic = loadContext.FindCachedImage(imageSourceIdx, metallicTextureSubIdx);
        if (cachedRoughness && cachedMetallic)
        {
            roughness = cachedRoughness;
            metallic = cachedMetallic;
            return;
        }

        // Create new assets if caches are not found
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
        loadContext.StoreImage(imageSourceIdx, roughnessTextureSubIdx, roughness);
        loadContext.StoreImage(imageSourceIdx, metallicTextureSubIdx, metallic);
    }

    AZ::Data::Instance<AZ::RPI::Image> GltfMaterialBuilder::Create2DImage(
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
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset;
        imageCreator.End(imageAsset);

        return AZ::RPI::StreamingImage::FindOrCreate(imageAsset);
    }

    void GltfMaterialBuilder::SetMaterialPropertyValue(
        AZ::Data::Instance<AZ::RPI::Material>& material, const AZ::Name& properyName, const AZ::RPI::MaterialPropertyValue& value)
    {
        const AZ::RPI::MaterialPropertyIndex& index = material->FindPropertyIndex(properyName);
        material->SetPropertyValue(index, value);
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")

