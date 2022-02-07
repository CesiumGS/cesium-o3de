#pragma once

#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <CesiumAsync/ITaskProcessor.h>

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
} // namespace Cesium
