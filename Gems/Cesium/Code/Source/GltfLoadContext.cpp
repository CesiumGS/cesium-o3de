#include "GltfLoadContext.h"

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
} // namespace Cesium
