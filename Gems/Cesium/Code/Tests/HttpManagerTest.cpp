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
    auto manager = AZStd::make_shared<Cesium::HttpManager>();
    bool requestMade = false;
    AZStd::mutex mutex;
    AZStd::condition_variable requestMadeSignal;

    // perform request
    Cesium::HttpRequestParameter parameter(
        "https://httpbin.org/ip", Aws::Http::HttpMethod::HTTP_GET,
        [&requestMadeSignal,
         &requestMade](const std::shared_ptr<Aws::Http::HttpRequest>& request, const std::shared_ptr<Aws::Http::HttpResponse>& response)
        {
            ASSERT_NE(request, nullptr);
            ASSERT_NE(response, nullptr);

            requestMade = true;
            requestMadeSignal.notify_all();
        });
    manager->AddRequest(std::move(parameter));

    // wait for the request to be finished
    AZStd::unique_lock<AZStd::mutex> lock(mutex);
    requestMadeSignal.wait(
        lock,
        [&requestMade]()
        {
            return requestMade;
        });

    ASSERT_EQ(requestMade, true);
}

