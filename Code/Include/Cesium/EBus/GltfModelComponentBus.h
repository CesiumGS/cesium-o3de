#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class GltfModelRequest : public AZ::ComponentBus
    {
    public:
        virtual void LoadModel(const AZStd::string& filePath) = 0;
    };

    using GltfModelRequestBus = AZ::EBus<GltfModelRequest>;
} // namespace Cesium
