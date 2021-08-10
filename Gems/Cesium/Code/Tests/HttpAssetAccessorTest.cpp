#include <HttpAssetAccessor.h>
#include <HttpManager.h>
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

    auto result = accessor.requestAsset(asyncSystem, "https://httpbin.org/ip").wait();
    auto completedRequest = std::get_if<std::shared_ptr<CesiumAsync::IAssetRequest>>(&result);
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ((*completedRequest)->response()->statusCode(), 200);
    ASSERT_EQ((*completedRequest)->method(), "GET");
}

TEST_F(HttpAssetAccessorTest, TestPost)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    AZStd::shared_ptr<Cesium::HttpManager> httpManager = AZStd::make_shared<Cesium::HttpManager>();
    Cesium::HttpAssetAccessor accessor(httpManager);

    auto result = accessor.post(asyncSystem, "https://httpbin.org/post").wait();
    auto completedRequest = std::get_if<std::shared_ptr<CesiumAsync::IAssetRequest>>(&result);
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ((*completedRequest)->response()->statusCode(), 200);
    ASSERT_EQ((*completedRequest)->method(), "POST");
}
