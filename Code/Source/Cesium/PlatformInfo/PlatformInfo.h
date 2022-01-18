#pragma once

#include <AzCore/Settings/SettingsRegistry.h>

namespace Cesium
{
    struct PlatformInfo
    {
        static AZ::SettingsRegistryInterface::FixedValueString GetEngineVersion();

        static AZ::SettingsRegistryInterface::FixedValueString GetPlatformName();

        static AZ::SettingsRegistryInterface::FixedValueString GetProjectName();
    };
} // namespace Cesium
