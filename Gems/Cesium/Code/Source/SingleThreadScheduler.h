#pragma once

#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/parallel/thread.h>
#include <AzCore/std/parallel/condition_variable.h>
#include <AzCore/std/containers/queue.h>
#include <AzCore/std/functional.h>

namespace Cesium
{
    using SingleThreadSchedulerTask = AZStd::function<void()>;

    class SingleThreadScheduler final
    {
    public:
        SingleThreadScheduler();

        ~SingleThreadScheduler() noexcept;

        void Schedule(SingleThreadSchedulerTask&& task);

    private:
        void ThreadFunction();

        void HandleTaskBatch();

        static constexpr const char* const LOGGING_NAME = "SingleThreadScheduler";

        AZStd::queue<SingleThreadSchedulerTask> m_tasks;
        AZStd::mutex m_taskMutex;
        AZStd::condition_variable m_taskConditionVar;
        AZStd::atomic<bool> m_runThread;
        AZStd::thread m_thread;
    };
} // namespace Cesium
