#pragma once

#include <CesiumAsync/ITaskProcessor.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace AZ
{
    class JobManager;
    class JobContext;
    class Job;
}

namespace Cesium
{
    class TaskProcessor : public CesiumAsync::ITaskProcessor
    {
    public:
        TaskProcessor();

        ~TaskProcessor() noexcept;

        void startTask(std::function<void()> task) override;

    private:
        AZStd::unique_ptr<AZ::JobManager> m_jobManager;
        AZStd::unique_ptr<AZ::JobContext> m_jobContext;
    };
}
