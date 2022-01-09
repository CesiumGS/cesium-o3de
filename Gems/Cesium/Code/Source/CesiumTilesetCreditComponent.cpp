#include <Cesium/CesiumTilesetCreditComponent.h>
#include "HtmlUiComponentHelper.h"
#include "CesiumSystemComponentBus.h"
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiCanvasManagerBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/string/string.h>

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
        UiCanvasManagerBus::BroadcastResult(
            m_clickableCanvasEntityId, &UiCanvasManagerBus::Events::LoadCanvas,
            AZStd::string("UICanvas/TilesetCredit/TilesetCredit.uicanvas"));
        UiCursorBus::Broadcast(&UiCursorBus::Events::IncrementVisibleCounter);

        AZ::Vector2 currentCursorPos;
        UiCursorBus::BroadcastResult(currentCursorPos, &UiCursorBus::Events::GetUiCursorPosition);
        UiCanvasBus::Event(m_clickableCanvasEntityId, &UiCanvasBus::Events::ForceActiveInteractable, GetEntityId(), true, currentCursorPos);

        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumTilesetCreditComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CesiumTilesetCreditComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (!m_displayCreditList)
        {
            return;
        }

        const auto& creditSystem = CesiumInterface::Get()->GetCreditSystem();

        const auto& creditToShow = creditSystem->getCreditsToShowThisFrame();
        bool creditUpdated = (creditToShow.size() != m_lastCreditCount);
        if (creditUpdated)
        {
            AZStd::string textCredit = "<!DOCTYPE html><html><body><ul>";
            for (const auto &credit : creditToShow)
            {
                textCredit += "<li>"; 
                textCredit += creditSystem->getHtml(credit).c_str();
                textCredit += "</li>"; 
            }
            textCredit += "</ul></body></html>";

            // remove the current canvas
            if (m_creditCanvasEntityId.IsValid())
            {
                UiCanvasManagerBus::Broadcast(&UiCanvasManagerBus::Events::UnloadCanvas, m_creditCanvasEntityId);
                m_creditCanvasEntityId = AZ::EntityId{};
            }

            m_creditCanvasEntityId = HtmlUiComponentHelper::CreateHtmlCanvasEntity(textCredit);
            m_lastCreditCount = creditToShow.size();
        }

        creditSystem->startNextFrame();
    }
} // namespace Cesium
