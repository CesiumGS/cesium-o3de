#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <cstddef>

namespace Cesium
{
    struct IORequestParameter
    {
        AZStd::string m_parentPath;
        AZStd::string m_path;
    };

    class GenericIOManager
    {
    public:
        virtual ~GenericIOManager() = default;

        virtual AZStd::vector<std::byte> GetFileContent(const IORequestParameter& request) = 0;

        virtual AZStd::vector<std::byte> GetFileContent(IORequestParameter&& request) = 0;
    };
}
