#include "LocalFileManager.h"
#include <AzCore/std/smart_ptr/make_shared.h>
#include <fstream>
#include <filesystem>

namespace Cesium
{
    AZStd::vector<std::byte> LocalFileManager::GetFileContent(const IORequestParameter& request)
    {
        std::filesystem::path parentPath = request.m_parentPath.c_str();
        std::filesystem::path path = request.m_path.c_str();
        path = std::filesystem::weakly_canonical(parentPath / path);

        std::ifstream f(path, std::ios::in | std::ios::binary);
        if (!f)
        {
            return {};
        }

        // Obtain the size of the file.
        const auto sz = std::filesystem::file_size(path.c_str());

        // Create a buffer.
        AZStd::vector<std::byte> content(sz);

        // Read the whole file into the buffer.
        f.read(reinterpret_cast<char*>(content.data()), sz);        
        return content;
    }

    AZStd::vector<std::byte> LocalFileManager::GetFileContent(IORequestParameter&& request)
    {
        return GetFileContent(request);
    }
} // namespace Cesium
