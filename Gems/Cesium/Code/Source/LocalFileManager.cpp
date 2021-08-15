#include "LocalFileManager.h"
#include "SingleThreadScheduler.h"
#include <CesiumAsync/Promise.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/IO/FileIO.h>

namespace Cesium
{
    struct LocalFileManager::RequestHandler
    {
        RequestHandler(const IORequestParameter& request, const CesiumAsync::Promise<IOContent>& promise)
            : m_request{ request }
            , m_promise{ promise }
        {
        }

        RequestHandler(IORequestParameter&& request, const CesiumAsync::Promise<IOContent>& promise)
            : m_request{ std::move(request) }
            , m_promise{ promise }
        {
        }

        void operator()()
        {
            AZStd::string absolutePath;
            AZ::StringFunc::Path::Join(m_request.m_parentPath.c_str(), m_request.m_path.c_str(), absolutePath);

            AZ::IO::FileIOStream stream(absolutePath.c_str(), AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary);
            if (!stream.IsOpen())
            {
                m_promise.reject(std::runtime_error("Request failed for file: " + std::string(absolutePath.c_str())));
            }

            // Create a buffer.
            std::size_t fileSize = stream.GetLength();
            IOContent content(fileSize);
            stream.Read(fileSize, content.data());
            m_promise.resolve(std::move(content));
        }

        IORequestParameter m_request;
        CesiumAsync::Promise<IOContent> m_promise;
    };

    LocalFileManager::LocalFileManager(SingleThreadScheduler* scheduler)
        : m_scheduler{ scheduler }
    {
    }

    AZStd::string LocalFileManager::GetParentPath(const AZStd::string& path)
    {
        AZStd::string parentPath(path);
        AZ::StringFunc::Path::StripFullName(parentPath);
        return parentPath;
    }

    IOContent LocalFileManager::GetFileContent(const IORequestParameter& request)
    {
        AZStd::string absolutePath;
        AZ::StringFunc::Path::Join(request.m_parentPath.c_str(), request.m_path.c_str(), absolutePath);

        AZ::IO::FileIOStream stream(absolutePath.c_str(), AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary);
        if (!stream.IsOpen())
        {
            return {};
        }

        // Create a buffer.
        std::size_t fileSize = stream.GetLength();
        IOContent content(fileSize);
        stream.Read(fileSize, content.data());
        return content;
    }

    IOContent LocalFileManager::GetFileContent(IORequestParameter&& request)
    {
        return GetFileContent(request);
    }

    CesiumAsync::Future<IOContent> LocalFileManager::GetFileContentAsync(
        const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request)
    {
        auto promise = asyncSystem.createPromise<IOContent>();
        m_scheduler->Schedule(RequestHandler{ request, promise });
        return promise.getFuture();
    }

    CesiumAsync::Future<IOContent> LocalFileManager::GetFileContentAsync(
        const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request)
    {
        auto promise = asyncSystem.createPromise<IOContent>();
        m_scheduler->Schedule(RequestHandler{ std::move(request), promise });
        return promise.getFuture();
    }

} // namespace Cesium
