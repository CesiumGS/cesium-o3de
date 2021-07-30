#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GenericIOManager.h"
#include <CesiumGltf/Image.h>
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

        bool LoadExternalImage(CesiumGltf::Image& externalImage);

    private:
        GenericIOManager* m_io;
        AZStd::string m_parentPath;
    };
} // namespace Cesium
