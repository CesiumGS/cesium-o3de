#include "Cesium/Systems/HttpAssetAccessor.h"
#include "Cesium/Systems/HttpManager.h"
#include <AzCore/Memory/PoolAllocator.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <CesiumAsync/AsyncSystem.h>

class HttpAssetAccessorTest : public UnitTest::AllocatorsTestFixture
{
public:
    void SetUp() override
    {
        UnitTest::AllocatorsTestFixture::SetUp();
        AZ::AllocatorInstance<AZ::PoolAllocator>::Create();
        AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Create();
    }

    void TearDown() override
    {
        AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Destroy();
        AZ::AllocatorInstance<AZ::PoolAllocator>::Destroy();
        UnitTest::AllocatorsTestFixture::TearDown();
    }
};

TEST_F(HttpAssetAccessorTest, TestRequestAsset)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::HttpManager httpManager;

    Cesium::HttpAssetAccessor accessor(&httpManager);
    auto completedRequestFuture = accessor.requestAsset(asyncSystem, "https://httpbin.org/ip");
    auto completedRequest = completedRequestFuture.wait();

    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "GET");
}

TEST_F(HttpAssetAccessorTest, TestPost)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::HttpManager httpManager;

    Cesium::HttpAssetAccessor accessor(&httpManager);
    auto completedRequestFuture = accessor.post(asyncSystem, "https://httpbin.org/post");
    auto completedRequest = completedRequestFuture.wait();

    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "POST");
}
