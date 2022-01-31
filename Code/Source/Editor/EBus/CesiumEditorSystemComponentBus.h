#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <glm/glm.hpp>

namespace Cesium
{
    class CesiumEditorSystemRequest : public AZ::ComponentBus
    {
    public:
        virtual void AddTilesetToLevel(
            const AZStd::string& tilesetName, std::uint32_t tilesetIonAssetId, int imageryIonAssetId, bool addToExistingEntity) = 0;

        virtual void PlaceOriginAtPosition(const glm::dvec3& position) = 0;

        virtual void AddImageryToLevel(std::uint32_t ionImageryAssetId, bool addToExistingEntity) = 0;

        virtual void AddBlankTilesetToLevel(bool addToExistingEntity) = 0;

        virtual void AddGeoreferenceCameraToLevel(bool addToExistingEntity) = 0;
    };

    class CesiumEditorSystemRequestEBusTraits : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
    };

    using CesiumEditorSystemRequestBus = AZ::EBus<CesiumEditorSystemRequest, CesiumEditorSystemRequestEBusTraits>;
} // namespace Cesium