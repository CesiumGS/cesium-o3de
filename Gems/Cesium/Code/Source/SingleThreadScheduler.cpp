#include "SingleThreadScheduler.h"

namespace Cesium
{
    SingleThreadScheduler::SingleThreadScheduler()
    {
        AZStd::thread_desc desc;
        desc.m_name = LOGGING_NAME;
        desc.m_cpuId = AFFINITY_MASK_USERTHREADS;
        m_runThread = true;
        auto function = AZStd::bind(&SingleThreadScheduler::ThreadFunction, this);
        m_thread = AZStd::thread(function, &desc);
    }

    SingleThreadScheduler::~SingleThreadScheduler() noexcept
    {
        m_runThread = false;
        m_taskConditionVar.notify_all();
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void SingleThreadScheduler::Schedule(SingleThreadSchedulerTask&& task)
    {
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_taskMutex);
            this->m_tasks.push(std::move(task));
        }

        this->m_taskConditionVar.notify_all();
    }

    void SingleThreadScheduler::ThreadFunction()
    {
        // Run the thread as long as directed
        while (m_runThread)
        {
            HandleTaskBatch();
        }
    }

    void SingleThreadScheduler::HandleTaskBatch()
    {
        // Lock mutex and wait for work to be signalled via the condition variable
        AZStd::unique_lock<AZStd::mutex> lock(m_taskMutex);
        m_taskConditionVar.wait(
            lock,
            [&]
            {
                return !m_runThread || !m_tasks.empty();
            });

        // Swap queues
        AZStd::queue<SingleThreadSchedulerTask> tasks;
        tasks.swap(m_tasks);

        // Release lock
        lock.unlock();

        // Handle requests
        while (!tasks.empty())
        {
            tasks.front()();
            tasks.pop();
        }
    }
} // namespace Cesium
