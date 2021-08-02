#include "LocalFileManager.h"
#include <AzCore/StringFunc/StringFunc.h>
#include <fstream>
#include <filesystem>

namespace Cesium
{
    AZStd::string LocalFileManager::GetParentPath(const AZStd::string& path)
    {
        AZStd::string parentPath(path);
        AZ::StringFunc::Path::StripFullName(parentPath);
        return parentPath;
    }

    AZStd::vector<std::byte> LocalFileManager::GetFileContent(const IORequestParameter& request)
    {
        AZStd::string absolutePath;
        AZ::StringFunc::Path::Join(request.m_parentPath.c_str(), request.m_path.c_str(), absolutePath);

        std::ifstream f(absolutePath.c_str(), std::ios::in | std::ios::binary);
        if (!f)
        {
            return {};
        }

        // Obtain the size of the file.
        const auto sz = std::filesystem::file_size(absolutePath.c_str());

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
