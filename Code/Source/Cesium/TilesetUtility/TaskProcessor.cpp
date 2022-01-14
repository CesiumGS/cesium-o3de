#include "TaskProcessor.h"
#include <AzCore/Jobs/JobFunction.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>

namespace Cesium
{
    TaskProcessor::TaskProcessor()
    {
        AZ::JobManagerDesc jobDesc;
        for (size_t i = 0; i < AZStd::thread::hardware_concurrency(); ++i)
        {
            jobDesc.m_workerThreads.push_back({ static_cast<int>(i) });
        }
        m_jobManager = AZStd::make_unique<AZ::JobManager>(jobDesc);
        m_jobContext = AZStd::make_unique<AZ::JobContext>(*m_jobManager);
    }

    TaskProcessor::~TaskProcessor() noexcept
    {
        m_jobContext.reset();
        m_jobManager.reset();
    }

    void TaskProcessor::startTask(std::function<void()> task)
    {
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(task, true, nullptr);
        job->Start();
    }
} // namespace Cesium
