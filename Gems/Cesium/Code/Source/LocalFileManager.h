#pragma once

#include "GenericIOManager.h"

namespace Cesium
{
    class LocalFileManager : public GenericIOManager
    {
    public:
        AZStd::string GetParentPath(const AZStd::string& path) override;

        IOContent GetFileContent(const IORequestParameter& request) override;

        IOContent GetFileContent(IORequestParameter&& request) override;
    };
} // namespace Cesium
