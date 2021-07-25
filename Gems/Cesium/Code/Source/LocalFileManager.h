#pragma once

#include "GenericIOManager.h"
#include <CesiumAsync/AsyncSystem.h>
#include <AzCore/std/containers/queue.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/parallel/thread.h>
#include <AzCore/std/parallel/condition_variable.h>

namespace Cesium
{
    class LocalFileManager : public GenericIOManager
    {
        struct LocalFileRequest
        {
            LocalFileRequest(IORequestParameter&& request, CesiumAsync::AsyncSystem::Promise<AZStd::shared_ptr<IOResult>> promise)
                : m_request{std::move(request)}
                , m_promise{promise}
            {
            }

            IORequestParameter m_request;
            CesiumAsync::AsyncSystem::Promise<AZStd::shared_ptr<IOResult>> m_promise;
        };

    public:
        CesiumAsync::Future<AZStd::shared_ptr<IOResult>> GetFileContent(
            const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request) override;

        CesiumAsync::Future<AZStd::shared_ptr<IOResult>> GetFileContent(
            const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request) override;

    private:
        void ThreadFunction();

        void HandleRequestBatch();

        void HandleRequest(LocalFileRequest& request);

        static constexpr const char* const LOGGING_NAME = "Local-File-Manager";

        AZStd::queue<LocalFileRequest> m_requestsToHandle;
        AZStd::mutex m_requestMutex;
        AZStd::condition_variable m_requestConditionVar;
        AZStd::atomic<bool> m_runThread;
        AZStd::thread m_thread;
    };
} // namespace Cesium
