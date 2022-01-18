#include "Cesium/PlatformInfo/PlatformInfo.h"
#include <AzCore/PlatformId/PlatformId.h>
#include <AzCore/Settings/SettingsRegistryImpl.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/IO/Path/Path.h>

namespace Cesium
{
    AZ::SettingsRegistryInterface::FixedValueString PlatformInfo::GetEngineVersion()
    {
        auto registry = AZ::SettingsRegistry::Get();
        if (registry != nullptr)
        {
            AZ::SettingsRegistryInterface::FixedValueString engineVersionKey{ AZ::SettingsRegistryMergeUtils::EngineSettingsRootKey };
            engineVersionKey += "/O3DEVersion";
            AZ::SettingsRegistryInterface::FixedValueString settingsValue;
            if (registry->Get(settingsValue, engineVersionKey))
            {
                return settingsValue;
            }
        }

        return "Unknown";
    }

    AZ::SettingsRegistryInterface::FixedValueString PlatformInfo::GetPlatformName()
    {
        return AZ::GetPlatformName(AZ::g_currentPlatform);
    }

    AZ::SettingsRegistryInterface::FixedValueString PlatformInfo::GetProjectName()
    {
        AZ::SettingsRegistryInterface::FixedValueString projectName = AZ::Utils::GetProjectName();
        if (projectName != "")
        {
            return projectName;
        }

        return "Unknown";
    }
} // namespace Cesium
