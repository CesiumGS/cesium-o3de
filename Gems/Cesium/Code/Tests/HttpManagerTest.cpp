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

TEST_F(HttpManagerTest, GetParentPath)
{
    // we don't care about io thread in this test
    Cesium::HttpManager httpManager{ nullptr };
    AZStd::string parentPath = httpManager.GetParentPath("https://httpbin.org/ip/tileset");
    ASSERT_EQ(parentPath, "https://httpbin.org/ip/");

    parentPath = httpManager.GetParentPath("NoSlash");
    ASSERT_EQ(parentPath, "NoSlash");

    parentPath = httpManager.GetParentPath("SlashAtTheEnd/");
    ASSERT_EQ(parentPath, "SlashAtTheEnd/");
}

TEST_F(HttpManagerTest, GetFileContent)
{
    // we don't care about io thread in this test
    Cesium::HttpManager httpManager{ nullptr };
    Cesium::IOContent content = httpManager.GetFileContent(Cesium::IORequestParameter{ "", "https://httpbin.org/ip" });
    ASSERT_FALSE(content.empty());
}

TEST_F(HttpManagerTest, GetFileContentAsyn)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::SingleThreadScheduler scheduler;
    Cesium::HttpManager httpManager{ &scheduler };

    Cesium::IORequestParameter parameter{ "", "https://httpbin.org/ip" };
    Cesium::IOContent content = httpManager.GetFileContentAsync(asyncSystem, parameter).wait();
    ASSERT_FALSE(content.empty());

    scheduler.~SingleThreadScheduler();
}

