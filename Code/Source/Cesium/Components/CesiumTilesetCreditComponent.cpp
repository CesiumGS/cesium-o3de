#include <Cesium/Components/CesiumTilesetCreditComponent.h>
#include "Cesium/Components/HtmlUiComponentHelper.h"
#include "Cesium/Systems/CesiumSystem.h"
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiCanvasManagerBus.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
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

        UiCanvasBus::EventResult(
            m_clickableRootCanvasEntityId, m_clickableCanvasEntityId, &UiCanvasBus::Events::FindElementEntityIdByName,
            AZStd::string("Element"));

        UiCursorBus::Broadcast(&UiCursorBus::Events::IncrementVisibleCounter);
        AzFramework::InputChannelEventListener::BusConnect();

        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumTilesetCreditComponent::Deactivate()
    {
        AzFramework::InputChannelEventListener::BusDisconnect();
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

    bool CesiumTilesetCreditComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
    {
        const AzFramework::InputDevice& inputDevice = inputChannel.GetInputDevice();
        const AzFramework::InputDeviceId& inputDeviceId = inputDevice.GetInputDeviceId();
        if (AzFramework::InputDeviceMouse::IsMouseDevice(inputDeviceId))
        {
            // process mouse inputs
            AzFramework::InputChannel::State state = inputChannel.GetState();
            const AzFramework::InputChannelId& inputChannelId = inputChannel.GetInputChannelId();
            if (state == AzFramework::InputChannel::State::Ended)
            {
                if (inputChannelId == AzFramework::InputDeviceMouse::Button::Left)
                {
                    AZ::Vector2 mousePos;
                    UiCursorBus::BroadcastResult(mousePos, &UiCursorBus::Events::GetUiCursorPosition);
                    AZ::Entity* hitCanvasEntity = nullptr;
                    UiElementBus::EventResult(
                        hitCanvasEntity, m_clickableRootCanvasEntityId, &UiElementBus::Events::FindFrontmostChildContainingPoint, mousePos,
                        true);
                    if (hitCanvasEntity)
                    {
                        m_displayCreditList = true;
                        UiCanvasBus::Event(m_creditCanvasEntityId, &UiCanvasBus::Events::SetEnabled, true);
                    }
                    else
                    {
                        m_displayCreditList = false;
                        UiCanvasBus::Event(m_creditCanvasEntityId, &UiCanvasBus::Events::SetEnabled, false);
                    }
                }
            }
        }

        return false;
    }
} // namespace Cesium
