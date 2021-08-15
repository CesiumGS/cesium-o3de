#include <HttpAssetAccessor.h>
#include <HttpManager.h>
#include <SingleThreadScheduler.h>
#include <CesiumAsync/AsyncSystem.h>
#include <AzCore/UnitTest/TestTypes.h>

class HttpAssetAccessorTest : public UnitTest::AllocatorsTestFixture
{
};

TEST_F(HttpAssetAccessorTest, TestRequestAsset)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::SingleThreadScheduler scheduler;
    Cesium::HttpManager httpManager{ &scheduler };

    Cesium::HttpAssetAccessor accessor(&httpManager);
    auto completedRequest = accessor.requestAsset(asyncSystem, "https://httpbin.org/ip").wait();
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "GET");

    // flush scheduler first before http manager shutdown
    scheduler.~SingleThreadScheduler();
}

TEST_F(HttpAssetAccessorTest, TestPost)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::SingleThreadScheduler scheduler;
    Cesium::HttpManager httpManager{ &scheduler };

    Cesium::HttpAssetAccessor accessor(&httpManager);
    auto completedRequest = accessor.post(asyncSystem, "https://httpbin.org/post").wait();
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "POST");

    // flush scheduler first before http manager shutdown
    scheduler.~SingleThreadScheduler();
}
