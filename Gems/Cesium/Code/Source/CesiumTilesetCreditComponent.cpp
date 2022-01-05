#include <Cesium/CesiumTilesetCreditComponent.h>
#include "HtmlUiComponentHelper.h"
#include "CesiumSystemComponentBus.h"
#include <LyShine/Bus/UiCanvasManagerBus.h>
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
        AZ::TickBus::Handler::BusConnect();
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
            AZStd::string textCredit = "<!DOCTYPE html><html><body><ul>";
            for (const auto &credit : creditToShow)
            {
                textCredit += "<li>"; 
                textCredit += creditSystem->getHtml(credit).c_str();
                textCredit += "</li>"; 
            }
            textCredit += "</ul></body></html>";

            // remove the current canvas
            if (m_canvasEntityId.IsValid())
            {
                UiCanvasManagerBus::Broadcast(&UiCanvasManagerBus::Events::UnloadCanvas, m_canvasEntityId);
                m_canvasEntityId = AZ::EntityId{};
            }

            m_canvasEntityId = HtmlUiComponentHelper::CreateHtmlCanvasEntity(textCredit);
            m_lastCreditCount = creditToShow.size();
        }

        creditSystem->startNextFrame();
    }
} // namespace Cesium
