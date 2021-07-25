#include "LocalFileManager.h"
#include <AzCore/std/smart_ptr/make_shared.h>
#include <fstream>
#include <filesystem>

namespace Cesium
{
    CesiumAsync::Future<AZStd::shared_ptr<IOResult>> LocalFileManager::GetFileContent(const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request)
    {
        return asyncSystem.createFuture<AZStd::shared_ptr<IOResult>>(
            [this, param = request](const auto& promise) mutable
            {
                LocalFileRequest request(std::move(param), promise);
                {
                    AZStd::lock_guard<AZStd::mutex> lock(this->m_requestMutex);
                    this->m_requestsToHandle.push(AZStd::move(request));
                }
                this->m_requestConditionVar.notify_all();
            });
    }

    CesiumAsync::Future<AZStd::shared_ptr<IOResult>> LocalFileManager::GetFileContent(
        const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request)
    {
        return asyncSystem.createFuture<AZStd::shared_ptr<IOResult>>(
            [this, param = std::move(request)](const auto& promise) mutable
            {
                LocalFileRequest request(std::move(param), promise);
                {
                    AZStd::lock_guard<AZStd::mutex> lock(this->m_requestMutex);
                    this->m_requestsToHandle.push(AZStd::move(request));
                }
                this->m_requestConditionVar.notify_all();
            });
    }

    void LocalFileManager::ThreadFunction()
    {
        while (m_runThread)
        {
            HandleRequestBatch();
        }
    }

    void LocalFileManager::HandleRequestBatch()
    {
        AZStd::unique_lock<AZStd::mutex> lock(m_requestMutex);
        m_requestConditionVar.wait(
            lock,
            [&]
            {
                return !m_runThread || !m_requestsToHandle.empty();
            });

        // Swap queues
        AZStd::queue<LocalFileRequest> requestsToHandle;
        requestsToHandle.swap(m_requestsToHandle);

        // Release lock
        lock.unlock();

        // Handle requests
        while (!requestsToHandle.empty())
        {
            HandleRequest(requestsToHandle.front());
            requestsToHandle.pop();
        }
    }

    void LocalFileManager::HandleRequest(LocalFileRequest& request)
    {
        // Open the stream to 'lock' the file.
        const auto& path = request.m_request.m_path;
        std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
        if (!f)
        {
            request.m_promise.reject(std::runtime_error("Cannot open file: " + std::string(path.c_str())));
        }

        // Obtain the size of the file.
        const auto sz = std::filesystem::file_size(path.c_str());

        // Create a buffer.
        IOResponse response;
        response.m_body.resize(sz);

        // Read the whole file into the buffer.
        f.read(reinterpret_cast<char*>(response.m_body.data()), sz);        

        request.m_promise.resolve(AZStd::make_shared<IOResult>(std::move(request.m_request), std::move(response)));
    }
} // namespace Cesium
