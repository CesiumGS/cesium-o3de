#pragma once

#include <AzCore/IO/Path/Path.h>
#include <AzCore/Settings/SettingsRegistry.h>

namespace Cesium
{
    struct PlatformInfo
    {
        static AZ::SettingsRegistryInterface::FixedValueString GetEngineVersion();

        static AZ::SettingsRegistryInterface::FixedValueString GetPlatformName();

        static AZ::SettingsRegistryInterface::FixedValueString GetProjectName();

        static AZ::IO::FixedMaxPath GetHttpCacheDirectory();
    };
} // namespace Cesium
