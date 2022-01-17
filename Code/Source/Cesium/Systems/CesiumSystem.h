
#pragma once

#include "Cesium/Systems/LocalFileManager.h"
#include "Cesium/Systems/HttpManager.h"
#include "Cesium/Systems/CriticalAssetManager.h"
#include <AzCore/JSON/rapidjson.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Cesium3DTilesSelection/CreditSystem.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/ITaskProcessor.h>
#include <spdlog/logger.h>
#include <memory>

namespace Cesium
{
    class GenericIOManager;
    class CriticalAssetManager;

    enum class IOKind
    {
        LocalFile,
        Http
    };

    class CesiumSystem final
    {
    public:
        CesiumSystem();

        ~CesiumSystem() noexcept = default;

        GenericIOManager& GetIOManager(IOKind kind);

        const std::shared_ptr<CesiumAsync::IAssetAccessor>& GetAssetAccessor(IOKind kind) const;

        const std::shared_ptr<CesiumAsync::ITaskProcessor>& GetTaskProcessor() const;

        const std::shared_ptr<spdlog::logger>& GetLogger() const;

        const std::shared_ptr<Cesium3DTilesSelection::CreditSystem>& GetCreditSystem() const;

        const CriticalAssetManager& GetCriticalAssetManager() const;

    private:
        AZStd::unique_ptr<HttpManager> m_httpManager;
        AZStd::unique_ptr<LocalFileManager> m_localFileManager;
        std::shared_ptr<CesiumAsync::IAssetAccessor> m_httpAssetAccessor;
        std::shared_ptr<CesiumAsync::IAssetAccessor> m_localFileAssetAccessor;
        std::shared_ptr<CesiumAsync::ITaskProcessor> m_taskProcessor;
        std::shared_ptr<spdlog::logger> m_logger;
        std::shared_ptr<Cesium3DTilesSelection::CreditSystem> m_creditSystem;
        CriticalAssetManager m_criticalAssetManager;
    };
} // namespace Cesium

namespace AZ {
    AZ_TYPE_INFO_SPECIALIZE(Cesium::CesiumSystem, "{ad337a9b-fa16-4d1c-bdbd-ccb7200937f9}");
}

namespace Cesium {
    using CesiumInterface = AZ::Interface<CesiumSystem>;
}
