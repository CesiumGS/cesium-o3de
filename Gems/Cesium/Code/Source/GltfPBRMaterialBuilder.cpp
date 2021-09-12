#include "GltfPBRMaterialBuilder.h"
#include "CesiumSystemComponentBus.h"
#include "CriticalAssetManager.h"

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>

#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/Material.h>
#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAssetCreator.h>
#include <Atom/RPI.Reflect/Image/ImageMipChainAssetCreator.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace Cesium
{
    const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GltfPBRMaterialBuilder::GetDefaultMaterialType() const
    {
        return CesiumInterface::Get()->GetCriticalAssetManager().m_standardPbrMaterialType;
    }

    void GltfPBRMaterialBuilder::OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType)
    {
        m_overrideMaterialTypeAsset = materialType;
    }

    void GltfPBRMaterialBuilder::Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
            GltfLoadMaterial& result)
    {
        // Create material asset
        AZ::Data::Asset<AZ::RPI::MaterialTypeAsset> materialTypeAsset;
        if (m_overrideMaterialTypeAsset)
        {
            materialTypeAsset = m_overrideMaterialTypeAsset;
        }
        else
        {
            materialTypeAsset = GetDefaultMaterialType();
        }

        AZ::RPI::MaterialAssetCreator materialCreator;
        materialCreator.Begin(AZ::Uuid::CreateRandom(), *materialTypeAsset);

        ConfigurePbrMetallicRoughness(model, material, textureCache, materialCreator);
        ConfigureOcclusion(model, material, textureCache, materialCreator);
        ConfigureEmissive(model, material, textureCache, materialCreator);
        ConfigureOpacity(material, materialCreator);

        AZ::Data::Asset<AZ::RPI::MaterialAsset> standardPBRMaterialAsset;
        materialCreator.End(standardPBRMaterialAsset);

        // populate result
        result.m_materialAsset = std::move(standardPBRMaterialAsset);
        result.m_needTangents = false; // We don't load normal texture, so no need for tangents vertices for now
    }

    void GltfPBRMaterialBuilder::ConfigurePbrMetallicRoughness(
        const CesiumGltf::Model& model,
        const CesiumGltf::Material& material,
        TextureCache& textureCache,
        AZ::RPI::MaterialAssetCreator& materialCreator)
    {
        const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
        if (!pbrMetallicRoughness)
        {
            return;
        }

        const std::vector<double>& baseColorFactor = pbrMetallicRoughness->baseColorFactor;
        if (baseColorFactor.size() == 4)
        {
            materialCreator.SetPropertyValue(
                AZ::Name("baseColor.color"),
                AZ::Color(
                    static_cast<float>(baseColorFactor[0]), static_cast<float>(baseColorFactor[1]), static_cast<float>(baseColorFactor[2]),
                    static_cast<float>(baseColorFactor[3])));
        }

        const std::optional<CesiumGltf::TextureInfo>& baseColorTexture = pbrMetallicRoughness->baseColorTexture;
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> baseColorImage;
        std::int64_t baseColorTexCoord = -1;
        if (baseColorTexture)
        {
            baseColorImage = GetOrCreateRGBAImage(model, *baseColorTexture, textureCache);
            baseColorTexCoord = baseColorTexture->texCoord;
        }

        if (baseColorImage && baseColorTexCoord >= 0 && baseColorTexCoord < 2)
        {
            materialCreator.SetPropertyValue(AZ::Name("baseColor.useTexture"), true);
            materialCreator.SetPropertyValue(AZ::Name("baseColor.textureMapUv"), static_cast<std::uint32_t>(baseColorTexCoord));
            materialCreator.SetPropertyValue(AZ::Name("baseColor.textureMap"), baseColorImage);
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
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> metallicImage;
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> roughnessImage;
        std::int64_t metallicRoughnessTexCoord = -1;
        if (metallicRoughnessTexture)
        {
            GetOrCreateMetallicRoughnessImage(model, *metallicRoughnessTexture, metallicImage, roughnessImage, textureCache);
            metallicRoughnessTexCoord = metallicRoughnessTexture->texCoord;
        }

        if (metallicImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), true);
            materialCreator.SetPropertyValue(AZ::Name("metallic.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
            materialCreator.SetPropertyValue(AZ::Name("metallic.textureMap"), metallicImage);
        }
        else
        {
            materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), false);
        }

        if (roughnessImage && metallicRoughnessTexCoord >= 0 && metallicRoughnessTexCoord < 2)
        {
            materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), true);
            materialCreator.SetPropertyValue(AZ::Name("roughness.textureMapUv"), static_cast<std::uint32_t>(metallicRoughnessTexCoord));
            materialCreator.SetPropertyValue(AZ::Name("roughness.textureMap"), roughnessImage);
        }
        else
        {
            materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), false);
        }
    }

    void GltfPBRMaterialBuilder::ConfigureEmissive(
        const CesiumGltf::Model& model,
        const CesiumGltf::Material& material,
        TextureCache& textureCache,
        AZ::RPI::MaterialAssetCreator& materialCreator)
    {
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
            auto emissiveImage = GetOrCreateRGBAImage(model, *emissiveTexture, textureCache);
            std::int64_t emissiveTexCoord = emissiveTexture->texCoord;
            if (emissiveImage && emissiveTexCoord >= 0 && emissiveTexCoord < 2)
            {
                enableEmissive = true;
                materialCreator.SetPropertyValue(AZ::Name("emissive.useTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("emissive.textureMapUv"), static_cast<std::uint32_t>(emissiveTexCoord));
                materialCreator.SetPropertyValue(AZ::Name("emissive.textureMap"), emissiveImage);
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

    }

    void GltfPBRMaterialBuilder::ConfigureOcclusion(
        const CesiumGltf::Model& model,
        const CesiumGltf::Material& material,
        TextureCache& textureCache,
        AZ::RPI::MaterialAssetCreator& materialCreator)
    {
        const std::optional<CesiumGltf::MaterialOcclusionTextureInfo> occlusionTexture = material.occlusionTexture;
        if (occlusionTexture)
        {
            auto occlusionImage = GetOrCreateOcclusionImage(model, *occlusionTexture, textureCache);
            std::int64_t occlusionTexCoord = occlusionTexture->texCoord;
            if (occlusionImage && occlusionTexCoord >= 0 && occlusionTexCoord < 2)
            {
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseUseTexture"), true);
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseTextureMapUv"), static_cast<std::uint32_t>(occlusionTexCoord));
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseFactor"), static_cast<float>(occlusionTexture->strength));
                materialCreator.SetPropertyValue(AZ::Name("occlusion.diffuseTextureMap"), occlusionImage);
            }
        }
    }

    void GltfPBRMaterialBuilder::ConfigureOpacity(
        const CesiumGltf::Material& material,
        AZ::RPI::MaterialAssetCreator& materialCreator)
    {
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
    }

    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GltfPBRMaterialBuilder::GetOrCreateOcclusionImage(
        const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return {};
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return {};
        }

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return {};
        }

        // Lookup cache
        TextureId imageSourceIdx = AZStd::string::format("Occlusion_%d", texture->source);
        auto cachedAsset = textureCache.find(imageSourceIdx);
        if (cachedAsset != textureCache.end())
        {
            return cachedAsset->second.m_imageAsset;
        }

        // Create new asset
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> newImage;
        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::uint32_t width = static_cast<std::uint32_t>(imageData.width);
        std::uint32_t height = static_cast<std::uint32_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 1)
        {
            return {};
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return {};
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

        auto cache = textureCache.insert({ imageSourceIdx, std::move(newImage) });
        return cache.first->second.m_imageAsset;
    }

    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GltfPBRMaterialBuilder::GetOrCreateRGBAImage(
        const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache)
    {
        const CesiumGltf::Texture* texture = model.getSafe<CesiumGltf::Texture>(&model.textures, textureInfo.index);
        if (!texture)
        {
            return {};
        }

        const CesiumGltf::Image* image = model.getSafe<CesiumGltf::Image>(&model.images, texture->source);
        if (!image || image->cesium.pixelData.empty())
        {
            return {};
        }

        if (image->cesium.width <= 0 || image->cesium.height <= 0)
        {
            return {};
        }

        // Lookup cache
        TextureId imageSourceIdx = AZStd::string::format("RGBA_%d", texture->source);
        auto cachedAsset = textureCache.find(imageSourceIdx);
        if (cachedAsset != textureCache.end())
        {
            return cachedAsset->second.m_imageAsset;
        }

        // Create a new asset if cache doesn't have it
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> newImage;
        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::uint32_t width = static_cast<std::uint32_t>(imageData.width);
        std::uint32_t height = static_cast<std::uint32_t>(imageData.height);
        if (imageData.bytesPerChannel != 1 || imageData.channels < 3 || imageData.channels > 4)
        {
            return {};
        }

        if (imageData.pixelData.size() != width * height * imageData.channels)
        {
            return {};
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

        auto cache = textureCache.insert({ imageSourceIdx, std::move(newImage) });
        return cache.first->second.m_imageAsset;
    }

    void GltfPBRMaterialBuilder::GetOrCreateMetallicRoughnessImage(
        const CesiumGltf::Model& model,
        const CesiumGltf::TextureInfo& textureInfo,
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& metallic,
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& roughness,
        TextureCache& textureCache)
    {
        static const std::int32_t roughnessTextureSubIdx = 0;
        static const std::int32_t metallicTextureSubIdx = 1;

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
        TextureId roughnessImageIdx = AZStd::string::format("Roughness_%d", texture->source);
        TextureId metallicImageIdx = AZStd::string::format("Metallic_%d", texture->source);
        auto cachedRoughness = textureCache.find(roughnessImageIdx);
        auto cachedMetallic = textureCache.find(metallicImageIdx);
        if (cachedRoughness != textureCache.end() && cachedMetallic != textureCache.end())
        {
            roughness = cachedRoughness->second.m_imageAsset;
            metallic = cachedMetallic->second.m_imageAsset;
            return;
        }

        // Create new assets if caches are not found
        const CesiumGltf::ImageCesium& imageData = image->cesium;
        std::uint32_t width = static_cast<std::uint32_t>(imageData.width);
        std::uint32_t height = static_cast<std::uint32_t>(imageData.height);
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

        auto metallicCache = textureCache.insert({metallicImageIdx, Create2DImage(metallicPixels.data(), metallicPixels.size(), width, height, AZ::RHI::Format::R8_UNORM)});
        auto roughnessCache = textureCache.insert({roughnessImageIdx, Create2DImage(roughnessPixels.data(), roughnessPixels.size(), width, height, AZ::RHI::Format::R8_UNORM)});
        metallic = metallicCache.first->second.m_imageAsset;
        roughness = roughnessCache.first->second.m_imageAsset;
    }

    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GltfPBRMaterialBuilder::Create2DImage(
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
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset;
        imageCreator.End(imageAsset);

        return imageAsset;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

