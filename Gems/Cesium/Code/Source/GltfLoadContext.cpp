// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfLoadContext.h"
#include <CesiumGltf/GltfReader.h>

namespace Cesium
{
    GltfLoadContext::GltfLoadContext(AZStd::string&& parentPath, GenericIOManager* io)
        : m_parentPath{ std::move(parentPath) }
        , m_io{ io }
    {
    }

    GltfLoadContext::GltfLoadContext(const AZStd::string& parentPath, GenericIOManager* io)
        : m_parentPath{parentPath}
        , m_io{io}
    {
    }

    bool GltfLoadContext::LoadExternalBuffer(CesiumGltf::Buffer& externalBuffer)
    {
        if (!m_io)
        {
            return false;
        }

        if (!externalBuffer.uri.has_value())
        {
            return false;
        }

        AZStd::string path = externalBuffer.uri.value().c_str();
        IORequestParameter param;
        param.m_parentPath = m_parentPath;
        param.m_path = std::move(path);
        auto content = m_io->GetFileContent(param);
        if (content.empty())
        {
            return false;
        }

        externalBuffer.cesium.data.resize(content.size());
        std::memcpy(externalBuffer.cesium.data.data(), content.data(), content.size());
        return true;
    }

    bool GltfLoadContext::LoadExternalImage(CesiumGltf::Image& externalImage)
    {
        static CesiumGltf::GltfReader imageReader;

        if (!m_io)
        {
            return false;
        }

        if (!externalImage.uri.has_value())
        {
            return false;
        }

        AZStd::string path = externalImage.uri.value().c_str();
        IORequestParameter param;
        param.m_parentPath = m_parentPath;
        param.m_path = std::move(path);
        auto content = m_io->GetFileContent(param);
        if (content.empty())
        {
            return false;
        }

        auto readResult = imageReader.readImage(gsl::span<const std::byte>(content.data(), content.size()));
        if (!readResult.image)
        {
            return false;
        }

        externalImage.cesium = std::move(*readResult.image);
        return true;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
