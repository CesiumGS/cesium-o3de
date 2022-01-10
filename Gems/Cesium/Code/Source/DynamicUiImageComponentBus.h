#pragma once

#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <AtomCore/Instance/Instance.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Math/Vector2.h>

namespace Cesium
{
    class DynamicUiImageRequest : public AZ::ComponentBus
    {
    public:
        virtual void LoadImageUrl(const AZStd::string& url) = 0;

        virtual void SetImage(AZ::Data::Instance<AZ::RPI::StreamingImage> image, AZ::RHI::Size imageSize) = 0;

        virtual void SetMaxImageSize(std::uint32_t width, std::uint32_t height) = 0;

        virtual void GetMaxImageSize(std::uint32_t& width, std::uint32_t& height) const = 0;

        virtual void GetResizedImageSize(std::uint32_t& width, std::uint32_t& height) const = 0;
    };

    using DynamicUiImageRequestBus = AZ::EBus<DynamicUiImageRequest>;
}
