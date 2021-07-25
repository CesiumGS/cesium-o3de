#pragma once

#include <CesiumAsync/Future.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <cstddef>

namespace CesiumAsync
{
    class AsyncSystem;
}

namespace Cesium
{
    struct IORequestParameter
    {
        AZStd::string m_path;
    };

    struct IOResponse
    {
        AZStd::vector<std::byte> m_body;
    };

    struct IOResult
    {
        IOResult(IORequestParameter&& request, IOResponse&& response)
            : m_request{ std::move(request) }
            , m_response{ std::move(response) }
        {
        }

        IORequestParameter m_request;
        IOResponse m_response;
    };

    class GenericIOManager
    {
    public:
        virtual ~GenericIOManager() = default;

        virtual CesiumAsync::Future<AZStd::shared_ptr<IOResult>> GetFileContent(
            const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request) = 0;

        virtual CesiumAsync::Future<AZStd::shared_ptr<IOResult>> GetFileContent(
            const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request) = 0;
    };
}
