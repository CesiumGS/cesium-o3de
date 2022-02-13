#include "Cesium/Systems/CesiumSystem.h"
#include "Cesium/Systems/LoggerSink.h"
#include "Cesium/Systems/HttpAssetAccessor.h"
#include "Cesium/Systems/GenericAssetAccessor.h"
#include "Cesium/Systems/TaskProcessor.h"
#include "Cesium/PlatformInfo/PlatformInfo.h"
#include <CesiumAsync/CachingAssetAccessor.h>
#include <CesiumAsync/SqliteCache.h>

namespace Cesium
{
    CesiumSystem::CesiumSystem()
    {
        // initialize logger
        m_logger = spdlog::default_logger();
        m_logger->sinks().clear();
        m_logger->sinks().push_back(std::make_shared<LoggerSink>());

        // initialize IO managers
        m_httpManager = AZStd::make_unique<HttpManager>();
        m_localFileManager = AZStd::make_unique<LocalFileManager>();

        // initialize asset accessors
        m_httpAssetAccessor = std::make_shared<CesiumAsync::CachingAssetAccessor>(
            m_logger, std::make_shared<HttpAssetAccessor>(m_httpManager.get()),
            std::make_shared<CesiumAsync::SqliteCache>(m_logger, PlatformInfo::GetHttpCacheDirectory().c_str()));
        m_localFileAssetAccessor = std::make_shared<GenericAssetAccessor>(m_localFileManager.get(), "");

        // initialize task processor
        m_taskProcessor = std::make_shared<TaskProcessor>();

        // initialize credit system
        m_creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();
    }

    GenericIOManager& CesiumSystem::GetIOManager(IOKind kind)
    {
        switch (kind)
        {
        case Cesium::IOKind::LocalFile:
            return *m_localFileManager;
        case Cesium::IOKind::Http:
            return *m_httpManager;
        default:
            return *m_httpManager;
        }
    }

    const std::shared_ptr<CesiumAsync::IAssetAccessor>& CesiumSystem::GetAssetAccessor(IOKind kind) const
    {
        switch (kind)
        {
        case Cesium::IOKind::LocalFile:
            return m_localFileAssetAccessor;
        case Cesium::IOKind::Http:
            return m_httpAssetAccessor;
        default:
            return m_httpAssetAccessor;
        }
    }

    const std::shared_ptr<CesiumAsync::ITaskProcessor>& CesiumSystem::GetTaskProcessor() const
    {
        return m_taskProcessor;
    }

    const std::shared_ptr<spdlog::logger>& CesiumSystem::GetLogger() const
    {
        return m_logger;
    }

    const std::shared_ptr<Cesium3DTilesSelection::CreditSystem>& CesiumSystem::GetCreditSystem() const
    {
        return m_creditSystem;
    }

    const CriticalAssetManager& CesiumSystem::GetCriticalAssetManager() const
    {
        return m_criticalAssetManager;
    }
} // namespace Cesium