#pragma once

#include "GenericIOManager.h"

namespace Cesium
{
    class LocalFileManager : public GenericIOManager
    {
    public:
        AZStd::string GetParentPath(const AZStd::string& path) override;

        AZStd::vector<std::byte> GetFileContent(const IORequestParameter& request) override;

        AZStd::vector<std::byte> GetFileContent(IORequestParameter&& request) override;
    };
} // namespace Cesium
