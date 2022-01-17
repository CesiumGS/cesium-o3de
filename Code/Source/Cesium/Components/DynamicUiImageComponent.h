#pragma once

#include "Cesium/EBus/DynamicUiImageComponentBus.h"
#include <LyShine/Draw2d.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiCanvasBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <CesiumAsync/AsyncSystem.h>

namespace Cesium
{
    class DynamicUiImageComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public DynamicUiImageRequestBus::Handler
        , public UiElementNotificationBus::Handler
        , public UiCanvasEnabledStateNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(DynamicUiImageComponent, "{002FCB42-3714-4159-9444-73FA60CC1C25}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        DynamicUiImageComponent();

        void LoadImageUrl(const AZStd::string& url) override;

        void SetImage(AZ::Data::Instance<AZ::RPI::StreamingImage> image, AZ::RHI::Size size) override;

        void SetMaxImageSize(std::uint32_t width, std::uint32_t height) override;

        void GetMaxImageSize(std::uint32_t& width, std::uint32_t& height) const override;

        void GetResizedImageSize(std::uint32_t& width, std::uint32_t& height) const override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnUiElementEnabledChanged(bool isEnabled) override;

        void OnUiElementAndAncestorsEnabledChanged(bool areElementAndAncestorsEnabled) override; 

        void OnCanvasEnabledStateChanged(AZ::EntityId canvasEntityId, bool enabled) override;

        void ScaleImageToFit();

        bool m_isEnable{ true };
        AZ::RHI::Size m_realImageSize{ 0, 0, 0 };
        AZ::RHI::Size m_scaledImageSize{ 0, 0, 0 };
        AZ::RHI::Size m_maxSize{ 256, 30, 0 };
        CesiumAsync::AsyncSystem m_asyncSystem;
        AZ::Data::Instance<AZ::RPI::StreamingImage> m_image;
        AZStd::unique_ptr<CDraw2d> m_draw2d;
        AZStd::unique_ptr<Draw2dHelper> m_draw2dHelper;
    };
}
