#include <Cesium/CesiumTilesetCreditComponent.h>
#include "CesiumSystemComponentBus.h"
#include <LyShine/UiComponentTypes.h>
#include <LyShine/Bus/UiCanvasManagerBus.h>
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <LyShine/Bus/UiTransform2dBus.h>
#include <LyShine/Bus/UiInitializationBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumTilesetCreditComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetCreditComponent, AZ::Component>()->Version(0);
        }
    }

    void CesiumTilesetCreditComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("3DTilesCreditService"));
    }

    void CesiumTilesetCreditComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("3DTilesCreditService"));
    }

    void CesiumTilesetCreditComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumTilesetCreditComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void CesiumTilesetCreditComponent::Init()
    {
    }

    void CesiumTilesetCreditComponent::Activate()
    {
    }

    void CesiumTilesetCreditComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CesiumTilesetCreditComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        const auto& creditSystem = CesiumInterface::Get()->GetCreditSystem();

        const auto& creditToShow = creditSystem->getCreditsToShowThisFrame();
        bool creditUpdated = (creditToShow.size() != m_lastCreditCount);
        if (creditUpdated)
        {
            AZStd::string textCredit;
            for (const auto &credit : creditToShow)
            {
                textCredit += creditSystem->getHtml(credit).c_str(); 
                textCredit += "\n";
            }

            CreateCanvasEntity();
            CreateTextEntity(textCredit);

            m_lastCreditCount = creditToShow.size();
        }

        creditSystem->startNextFrame();
    }

    void CesiumTilesetCreditComponent::CreateCanvasEntity()
    {
        if (m_canvasEntityId.IsValid())
        {
            UiCanvasManagerBus::Broadcast(&UiCanvasManagerBus::Events::UnloadCanvas, m_canvasEntityId);
            m_canvasEntityId = AZ::EntityId{};
        }

        UiCanvasManagerBus::BroadcastResult(m_canvasEntityId, &UiCanvasManagerBus::Events::CreateCanvas);

        AZ::Entity* rootElementEntity = nullptr;
        UiCanvasBus::EventResult(rootElementEntity, m_canvasEntityId, &UiCanvasBus::Events::CreateChildElement, AZStd::string("CreditRoot"));
        if (rootElementEntity)
        {
            rootElementEntity->Deactivate();
            rootElementEntity->CreateComponent(LyShine::UiTransform2dComponentUuid);
            rootElementEntity->Activate();

            m_rootElementEntityId = rootElementEntity->GetId();
            UiTransform2dBus::Event(m_rootElementEntityId, &UiTransform2dBus::Events::SetAnchors, UiTransform2dInterface::Anchors{0.0f, 0.0f, 1.0f, 1.0f}, false, false);
            UiTransform2dBus::Event(m_rootElementEntityId, &UiTransform2dBus::Events::SetOffsets, UiTransform2dInterface::Offsets{0.0f, 0.0f, 0.0f, 0.0f});
        }
    }

    void CesiumTilesetCreditComponent::CreateTextEntity(const AZStd::string& text)
    {
        // Create the text element
        AZ::Entity* textEntity = nullptr;
        UiElementBus::EventResult(textEntity, m_rootElementEntityId, &UiElementBus::Events::CreateChildElement, AZStd::string("text"));

        // Set up the text element
        if (textEntity)
        {
            textEntity->Deactivate();
            textEntity->CreateComponent(LyShine::UiTransform2dComponentUuid);
            textEntity->CreateComponent(LyShine::UiTextComponentUuid);
            textEntity->Activate();

            AZ::EntityId textEntityId = textEntity->GetId();
            UiTransformBus::Event(
                textEntityId, &UiTransformBus::Events::SetScaleToDeviceMode, UiTransformBus::Events::ScaleToDeviceMode::UniformScaleToFill);
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetAnchors, UiTransform2dInterface::Anchors{}, false, false);
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetOffsets, UiTransform2dInterface::Offsets{-300.0f, -256.0f, 300.0f, 256.0f});
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetPivotAndAdjustOffsets, AZ::Vector2(0.5f));

            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetText, text);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetTextAlignment, IDraw2d::HAlign::Left, IDraw2d::VAlign::Top);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetWrapText, UiTextInterface::WrapTextSetting::Wrap);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetFontSize, 18.0f);

            // Trigger all InGamePostActivate
            UiInitializationBus::Event(textEntityId, &UiInitializationBus::Events::InGamePostActivate);
        }

    }
} // namespace Cesium
