#include "Cesium/PlatformInfo/PlatformInfo.h"
#include <AzCore/PlatformId/PlatformId.h>
#include <AzCore/Settings/SettingsRegistryImpl.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/IO/FileIO.h>

namespace Cesium
{
    AZ::SettingsRegistryInterface::FixedValueString PlatformInfo::GetEngineVersion()
    {
        AZ::SettingsRegistryInterface::FixedValueString engineVersion;
        auto engineSettingsPath = AZ::IO::FixedMaxPath{ AZ::Utils::GetEnginePath() } / "engine.json";
        if (AZ::IO::SystemFile::Exists(engineSettingsPath.c_str()))
        {
            AZ::SettingsRegistryImpl settingsRegistry;
            if (settingsRegistry.MergeSettingsFile(
                    engineSettingsPath.Native(), AZ::SettingsRegistryInterface::Format::JsonMergePatch,
                    AZ::SettingsRegistryMergeUtils::EngineSettingsRootKey))
            {
                settingsRegistry.Get(
                    engineVersion,
                    AZ::SettingsRegistryInterface::FixedValueString(AZ::SettingsRegistryMergeUtils::EngineSettingsRootKey) +
                        "/O3DEVersion");
            }
        }

        return engineVersion;
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

    AZ::IO::FixedMaxPath PlatformInfo::GetHttpCacheDirectory()
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        if (!fileIO->Exists("@user@/CesiumHttpCache"))
        {
            fileIO->CreatePath("@user@/CesiumHttpCache");
        }

        return AZ::IO::FixedMaxPath{ fileIO->GetAlias("@user@") } / "CesiumHttpCache" / "HttpCacheDatabase.sqlite";
    }
} // namespace Cesium
