#pragma once

#include "GenericIOManager.h"
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/Future.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>

namespace Cesium
{
    class LocalFileManager final : public GenericIOManager
    {
        struct RequestHandler;

    public:
        LocalFileManager();

        AZStd::string GetParentPath(const AZStd::string& path) override;

        IOContent GetFileContent(const IORequestParameter& request) override;

        IOContent GetFileContent(IORequestParameter&& request) override;

        CesiumAsync::Future<IOContent> GetFileContentAsync(
            const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request) override;

        CesiumAsync::Future<IOContent> GetFileContentAsync(
            const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request) override;

    private:
        AZStd::unique_ptr<AZ::JobManager> m_ioJobManager;
        AZStd::unique_ptr<AZ::JobContext> m_ioJobContext;
    };
} // namespace Cesium
