#pragma once

#include "GenericIOManager.h"
#include <CesiumGltf/Buffer.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class GltfLoadContext
    {
    public:
        GltfLoadContext(AZStd::string&& parentPath, GenericIOManager* io);

        GltfLoadContext(const AZStd::string& parentPath, GenericIOManager* io);

        bool LoadExternalBuffer(CesiumGltf::Buffer& externalBuffer);

    private:
        GenericIOManager* m_io;
        AZStd::string m_parentPath;
    };
} // namespace Cesium
