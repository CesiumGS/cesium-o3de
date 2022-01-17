#include "Cesium/Components/DynamicUiImageComponent.h"
#include "Cesium/Systems/CesiumSystem.h"
#include <LyShine/Bus/UiTransformBus.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Public/Image/StreamingImagePool.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/StringFunc/StringFunc.h>

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltfReader/GltfReader.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif


namespace Cesium
{
    void DynamicUiImageComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DynamicUiImageComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void DynamicUiImageComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DynamicUiImageService"));
    }

    void DynamicUiImageComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void DynamicUiImageComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("UiElementService", 0x3dca7ad4));
        required.push_back(AZ_CRC("UiTransformService", 0x3a838e34));
    }

    void DynamicUiImageComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    DynamicUiImageComponent::DynamicUiImageComponent()
        : m_asyncSystem{ CesiumInterface::Get()->GetTaskProcessor() }
    {
    }

    void DynamicUiImageComponent::LoadImageUrl(const AZStd::string& url)
    {
        if (AZ::StringFunc::StartsWith(url, "data") && AZ::StringFunc::Contains(url, "base64"))
        {
            AZStd::string_view urlView = url;
            auto separator = urlView.find_first_of(",");
            auto base64View = url.substr(separator + 1);

            AZStd::vector<AZ::u8> decodeOutput;
            AZ::StringFunc::Base64::Decode(decodeOutput, base64View.data(), base64View.size());

            CesiumGltfReader::GltfReader gltfReader;
            auto imageResult = gltfReader.readImage(
                gsl::span<const std::byte>(reinterpret_cast<const std::byte*>(decodeOutput.data()), decodeOutput.size()));
            if (imageResult.image)
            {
                auto pool = AZ::RPI::ImageSystemInterface::Get()->GetStreamingPool();
                auto size = AZ::RHI::Size(imageResult.image->width, imageResult.image->height, 1);
                auto image = AZ::RPI::StreamingImage::CreateFromCpuData(
                    *pool, AZ::RHI::ImageDimension::Image2D, size, AZ::RHI::Format::R8G8B8A8_UNORM, imageResult.image->pixelData.data(),
                    imageResult.image->pixelData.size());
                SetImage(image, size);
            }

            return;
        }

        AZ::EntityId selfEntityId = GetEntityId();
        auto& httpManager = CesiumInterface::Get()->GetIOManager(Cesium::IOKind::Http);
        Cesium::IORequestParameter requestParameter{ url, "" };
        auto future = httpManager.GetFileContentAsync(m_asyncSystem, requestParameter)
                          .thenInMainThread(
                              [selfEntityId](IOContent&& content)
                              {
                                  if (content.empty())
                                  {
                                      return;
                                  }

                                  CesiumGltfReader::GltfReader gltfReader;
                                  auto imageResult = gltfReader.readImage(content);
                                  if (imageResult.image)
                                  {
                                      auto pool = AZ::RPI::ImageSystemInterface::Get()->GetStreamingPool();
                                      auto size = AZ::RHI::Size(imageResult.image->width, imageResult.image->height, 1); 
                                      auto image = AZ::RPI::StreamingImage::CreateFromCpuData(
                                          *pool, AZ::RHI::ImageDimension::Image2D, size, AZ::RHI::Format::R8G8B8A8_UNORM,
                                          imageResult.image->pixelData.data(), imageResult.image->pixelData.size());
                                      DynamicUiImageRequestBus::Event(
                                          selfEntityId, &DynamicUiImageRequestBus::Events::SetImage, image, size);
                                  }
                              });
    }

    void DynamicUiImageComponent::SetImage(AZ::Data::Instance<AZ::RPI::StreamingImage> image, AZ::RHI::Size size)
    {
        m_image = std::move(image);
        m_realImageSize = size;
        ScaleImageToFit();
    }

    void DynamicUiImageComponent::SetMaxImageSize(std::uint32_t width, std::uint32_t height)
    {
        m_maxSize = AZ::RHI::Size(width, height, 1);
        ScaleImageToFit();
    }

    void DynamicUiImageComponent::GetMaxImageSize(std::uint32_t& width, std::uint32_t& height) const
    {
        width = m_maxSize.m_width;
        height = m_maxSize.m_height;
    }

    void DynamicUiImageComponent::GetResizedImageSize(std::uint32_t &width, std::uint32_t &height) const
    {
        width = m_scaledImageSize.m_width;
        height = m_scaledImageSize.m_height;
    }

    void DynamicUiImageComponent::Init()
    {
    }

    void DynamicUiImageComponent::Activate()
    {
        UiElementNotificationBus::Handler::BusConnect(GetEntityId());
        UiCanvasEnabledStateNotificationBus::Handler::BusConnect();
        DynamicUiImageRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        auto viewportManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
        if (viewportManager)
        {
            m_draw2d = AZStd::make_unique<CDraw2d>(viewportManager->GetDefaultViewportContext());
            m_draw2dHelper = AZStd::make_unique<Draw2dHelper>(m_draw2d.get());
        }
    }

    void DynamicUiImageComponent::Deactivate()
    {
        DynamicUiImageRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void DynamicUiImageComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_isEnable && m_draw2dHelper)
        {
            m_asyncSystem.dispatchMainThreadTasks();

            if (m_image)
            {
                UiTransformInterface::RectPoints rectPoints;
                UiTransformBus::Event(GetEntityId(), &UiTransformBus::Events::GetCanvasSpacePoints, rectPoints);

                AZ::Matrix4x4 transform;
                UiTransformBus::Event(GetEntityId(), &UiTransformBus::Events::GetTransformToCanvasSpace, transform);
                AZ::Vector3 scale = transform.ExtractScale();

                static const int64_t topLayerKey = 0x1000000;
                m_draw2d->SetSortKey(topLayerKey);
                m_draw2dHelper->DrawImage(
                    m_image, rectPoints.TopLeft(),
                    AZ::Vector2(
                        scale.GetX() * static_cast<float>(m_scaledImageSize.m_width),
                        scale.GetY() * static_cast<float>(m_scaledImageSize.m_height)));
            }
        }
    }

    void DynamicUiImageComponent::OnUiElementEnabledChanged(bool isEnabled)
    {
        m_isEnable = isEnabled;
    }

    void DynamicUiImageComponent::OnUiElementAndAncestorsEnabledChanged(bool areElementAndAncestorsEnabled)
    {
        m_isEnable = areElementAndAncestorsEnabled;
    }

    void DynamicUiImageComponent::OnCanvasEnabledStateChanged(AZ::EntityId canvasEntityId, bool enabled)
    {
        AZ::EntityId currentCanvasEntityId;
        UiElementBus::EventResult(currentCanvasEntityId, GetEntityId(), &UiElementBus::Events::GetCanvasEntityId);
        if (canvasEntityId == currentCanvasEntityId)
        {
            m_isEnable = enabled;
        }
    }

    void DynamicUiImageComponent::ScaleImageToFit()
    {
        float imgScale = static_cast<float>(m_realImageSize.m_width) / static_cast<float>(m_realImageSize.m_height);
        float scaleWidth = static_cast<float>(m_maxSize.m_width) / static_cast<float>(m_realImageSize.m_width);
        float scaleHeight = static_cast<float>(m_maxSize.m_height) / static_cast<float>(m_realImageSize.m_height);
        if (scaleWidth < scaleHeight)
        {
            m_scaledImageSize.m_width = static_cast<std::uint32_t>(scaleWidth * static_cast<float>(m_realImageSize.m_width));
            m_scaledImageSize.m_height = static_cast<std::uint32_t>(1.0f / imgScale * static_cast<float>(m_scaledImageSize.m_width));
        }
        else
        {
            m_scaledImageSize.m_height = static_cast<std::uint32_t>(scaleHeight * static_cast<float>(m_realImageSize.m_height));
            m_scaledImageSize.m_width = static_cast<std::uint32_t>(imgScale * static_cast<float>(m_scaledImageSize.m_height));
        }
    }
} // namespace Cesium
