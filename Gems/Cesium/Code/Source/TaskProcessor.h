#pragma once

#include <CesiumAsync/ITaskProcessor.h>

namespace Cesium
{
    class TaskProcessor : public CesiumAsync::ITaskProcessor
    {
    public:
        void startTask(std::function<void()> task) override;
    };
}
