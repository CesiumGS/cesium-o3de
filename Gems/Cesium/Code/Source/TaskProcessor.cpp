#include "TaskProcessor.h"
#include <AzCore/Jobs/JobFunction.h>

namespace Cesium
{
    void TaskProcessor::startTask(std::function<void()> task)
    {
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(task, true, nullptr);
        job->Start();
    }
} // namespace Cesium
