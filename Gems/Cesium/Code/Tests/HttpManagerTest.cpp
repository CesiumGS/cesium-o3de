#include <HttpManager.h>
#include <SingleThreadScheduler.h>
#include <AzCore/UnitTest/TestTypes.h>

class HttpManagerTest : public UnitTest::AllocatorsTestFixture
{
};

TEST_F(HttpManagerTest, AddValidRequest)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::SingleThreadScheduler scheduler;
    Cesium::HttpManager httpManager{ &scheduler };

    Cesium::HttpRequestParameter parameter("https://httpbin.org/ip", Aws::Http::HttpMethod::HTTP_GET);
    auto completedRequest = httpManager.AddRequest(asyncSystem, std::move(parameter)).wait();
    ASSERT_EQ(completedRequest.m_response->GetResponseCode(), Aws::Http::HttpResponseCode::OK);
    ASSERT_EQ(completedRequest.m_request->GetMethod(), Aws::Http::HttpMethod::HTTP_GET);

    // flush scheduler first before http manager shutdown
    scheduler.~SingleThreadScheduler();
}

