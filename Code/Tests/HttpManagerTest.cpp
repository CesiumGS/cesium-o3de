#include "Cesium/Systems/HttpManager.h"
#include <AzCore/Memory/PoolAllocator.h>
#include <AzCore/UnitTest/TestTypes.h>

class HttpManagerTest : public UnitTest::AllocatorsTestFixture
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

TEST_F(HttpManagerTest, AddValidRequest)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::HttpManager httpManager;

    Cesium::HttpRequestParameter parameter("https://httpbin.org/ip", Aws::Http::HttpMethod::HTTP_GET);
    auto completedRequestFuture = httpManager.AddRequest(asyncSystem, std::move(parameter));
    auto completedRequest = completedRequestFuture.wait();

    ASSERT_EQ(completedRequest.m_response->GetResponseCode(), Aws::Http::HttpResponseCode::OK);
    ASSERT_EQ(completedRequest.m_request->GetMethod(), Aws::Http::HttpMethod::HTTP_GET);
}

TEST_F(HttpManagerTest, GetParentPath)
{
    // we don't care about io thread in this test
    Cesium::HttpManager httpManager;
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
    Cesium::HttpManager httpManager;
    Cesium::IOContent content = httpManager.GetFileContent(Cesium::IORequestParameter{ "", "https://httpbin.org/ip" });
    ASSERT_FALSE(content.empty());
}

TEST_F(HttpManagerTest, GetFileContentAsync)
{
    // we don't care about worker thread in this test
    CesiumAsync::AsyncSystem asyncSystem{ nullptr };
    Cesium::HttpManager httpManager;

    Cesium::IORequestParameter parameter{ "", "https://httpbin.org/ip" };
    auto contentFuture = httpManager.GetFileContentAsync(asyncSystem, parameter);
    auto content = contentFuture.wait();

    ASSERT_FALSE(content.empty());
}
