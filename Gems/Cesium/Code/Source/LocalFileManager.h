#pragma once

#include "GenericIOManager.h"
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/Future.h>

namespace Cesium
{
    class SingleThreadScheduler;

    class LocalFileManager final : public GenericIOManager
    {
        struct RequestHandler;

    public:
        LocalFileManager(SingleThreadScheduler* scheduler);

        AZStd::string GetParentPath(const AZStd::string& path) override;

        IOContent GetFileContent(const IORequestParameter& request) override;

        IOContent GetFileContent(IORequestParameter&& request) override;

        CesiumAsync::Future<IOContent> GetFileContentAsync(
            const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request) override;

        CesiumAsync::Future<IOContent> GetFileContentAsync(
            const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request) override;

    private:
        SingleThreadScheduler* m_scheduler;
    };
} // namespace Cesium
