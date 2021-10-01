#include "HttpAssetAccessor.h"
#include "HttpManager.h"
#include <CesiumAsync/AsyncSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/UnitTest/TestTypes.h>

class HttpAssetAccessorTest : public UnitTest::AllocatorsTestFixture
{
};

TEST_F(HttpAssetAccessorTest, TestRequestAsset)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    AZStd::shared_ptr<Cesium::HttpManager> httpManager = AZStd::make_shared<Cesium::HttpManager>();
    Cesium::HttpAssetAccessor accessor(httpManager);

    auto completedRequest = accessor.requestAsset(asyncSystem, "https://httpbin.org/ip").wait();
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "GET");
}

TEST_F(HttpAssetAccessorTest, TestPost)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    AZStd::shared_ptr<Cesium::HttpManager> httpManager = AZStd::make_shared<Cesium::HttpManager>();
    Cesium::HttpAssetAccessor accessor(httpManager);

    auto completedRequest = accessor.post(asyncSystem, "https://httpbin.org/post").wait();
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response()->statusCode(), 200);
    ASSERT_EQ(completedRequest->method(), "POST");
}
