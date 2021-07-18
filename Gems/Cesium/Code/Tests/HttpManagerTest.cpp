#include "HttpManager.h"
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/parallel/conditional_variable.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/UnitTest/TestTypes.h>

class HttpManagerTest : public UnitTest::AllocatorsTestFixture
{
};

TEST_F(HttpManagerTest, AddValidRequest)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    AZStd::shared_ptr<Cesium::HttpManager> httpManager = AZStd::make_shared<Cesium::HttpManager>();
    Cesium::HttpRequestParameter parameter("https://httpbin.org/ip", Aws::Http::HttpMethod::HTTP_GET);
    auto result = httpManager->AddRequest(asyncSystem, std::move(parameter)).wait();
    auto completedRequest = std::get_if<Cesium::HttpResult>(&result);
    ASSERT_NE(completedRequest, nullptr);
    ASSERT_EQ(completedRequest->response->GetResponseCode(), Aws::Http::HttpResponseCode::OK);
    ASSERT_EQ(completedRequest->request->GetMethod(), Aws::Http::HttpMethod::HTTP_GET);
}

