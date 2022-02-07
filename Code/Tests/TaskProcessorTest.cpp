#include "Cesium/Systems/TaskProcessor.h"
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/std/parallel/thread.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <future>

class TaskProcessorTest : public UnitTest::AllocatorsTestFixture
{
public:
    void SetUp() override
    {
        UnitTest::AllocatorsTestFixture::SetUp();
        AZ::AllocatorInstance<AZ::PoolAllocator>::Create();
        AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Create();

        AZ::JobManagerDesc managerDesc;
        AZ::JobManagerThreadDesc threadDesc;
        std::size_t hardwareConcurrency = AZStd::thread::hardware_concurrency();
        for (std::size_t i = 0; i < hardwareConcurrency; ++i)
        {
            managerDesc.m_workerThreads.push_back(threadDesc);
        }

        m_jobManager = aznew AZ::JobManager(managerDesc);
        m_jobContext = aznew AZ::JobContext(*m_jobManager);
        AZ::JobContext::SetGlobalContext(m_jobContext);
    }

    void TearDown() override
    {
        AZ::JobContext::SetGlobalContext(nullptr);
        delete m_jobContext;
        delete m_jobManager;
        AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Destroy();
        AZ::AllocatorInstance<AZ::PoolAllocator>::Destroy();
        UnitTest::AllocatorsTestFixture::TearDown();
    }

protected:
    AZ::JobManager* m_jobManager;
    AZ::JobContext* m_jobContext;
};

TEST_F(TaskProcessorTest, StartJobInAnotherThread)
{
    AZStd::vector<std::promise<AZStd::thread::id>> promises(m_jobManager->GetNumWorkerThreads());
    AZStd::vector<std::future<AZStd::thread::id>> futures;
    futures.reserve(promises.size());
    for (auto& promise : promises)
    {
        futures.emplace_back(promise.get_future());
    }

    Cesium::TaskProcessor processor;
    for (auto& promise : promises)
    {
        processor.startTask(
            [&promise]()
            {
                promise.set_value(AZStd::this_thread::get_id());
            });
    }

    for (auto& future : futures)
    {
        ASSERT_NE(future.get(), AZStd::this_thread::get_id());
    }
}
