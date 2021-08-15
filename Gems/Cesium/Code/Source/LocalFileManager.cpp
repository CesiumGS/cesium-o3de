#include "LocalFileManager.h"
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/IO/FileIO.h>

namespace Cesium
{
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
} // namespace Cesium
