#include "HtmlUiComponentHelper.h"
#include <LyShine/UiComponentTypes.h>
#include <LyShine/Bus/UiCanvasManagerBus.h>
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <LyShine/Bus/UiTransform2dBus.h>
#include <LyShine/Bus/UiInitializationBus.h>
#include <tidybuffio.h>

namespace Cesium
{
    AZ::EntityId HtmlUiComponentHelper::CreateHtmlCanvasEntity(const AZStd::string& html)
    {
        // create canvas entity
        AZ::EntityId canvasEntityId;
        AZ::EntityId rootElementEntityId;
        UiCanvasManagerBus::BroadcastResult(canvasEntityId, &UiCanvasManagerBus::Events::CreateCanvas);
        if (!canvasEntityId.IsValid())
        {
            return AZ::EntityId{};
        }

        AZ::Entity* rootElementEntity = nullptr;
        UiCanvasBus::EventResult(
            rootElementEntity, canvasEntityId, &UiCanvasBus::Events::CreateChildElement, AZStd::string("CreditRoot"));
        if (!rootElementEntity)
        {
            return AZ::EntityId{};
        }

        rootElementEntity->Deactivate();
        rootElementEntity->CreateComponent(LyShine::UiTransform2dComponentUuid);
        rootElementEntity->Activate();

        rootElementEntityId = rootElementEntity->GetId();
        UiTransformBus::Event(
            rootElementEntityId, &UiTransformBus::Events::SetScaleToDeviceMode, UiTransformBus::Events::ScaleToDeviceMode::NonUniformScale);
        UiTransform2dBus::Event(
            rootElementEntityId, &UiTransform2dBus::Events::SetAnchors, UiTransform2dInterface::Anchors{ 0.0f, 0.0f, 1.0f, 1.0f },
            false, false);
        UiTransform2dBus::Event(
            rootElementEntityId, &UiTransform2dBus::Events::SetOffsets, UiTransform2dInterface::Offsets{ 0.0f, 0.0f, 0.0f, 0.0f });

        // begin parse html
        TidyDoc tdoc;
        TidyBuffer docbuf = { 0 };
        TidyBuffer tidy_errbuf = { 0 };
        int err;

        tdoc = tidyCreate();
        tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
        tidyOptSetInt(tdoc, TidyWrapLen, 4096);
        tidySetErrorBuffer(tdoc, &tidy_errbuf);
        tidyBufInit(&docbuf);
        tidyBufAppend(&docbuf, reinterpret_cast<void*>(const_cast<char*>(html.c_str())), static_cast<uint>(html.size()));

        err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
        if (err >= 0)
        {
            err = tidyCleanAndRepair(tdoc); /* fix any problems */
            if (err >= 0)
            {
                err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
                if (err >= 0)
                {
                    CreateHtmlNodeEntities(tdoc, tidyGetRoot(tdoc), rootElementEntityId, 0.0f);
                    AZ_Printf("Cesium", "%s\n", tidy_errbuf.bp); /* show errors */
                }
            }
        }

        tidyBufFree(&docbuf);
        tidyBufFree(&tidy_errbuf);
        tidyRelease(tdoc);

        return canvasEntityId;
    }

    float HtmlUiComponentHelper::CreateHtmlNodeEntities(TidyDoc tdoc, TidyNode tnode, const AZ::EntityId& rootElementEntityId, float beginHeight)
    {
        float height = 0.0f;
        TidyNode child;
        for (child = tidyGetChild(tnode); child; child = tidyGetNext(child))
        {
            ctmbstr name = tidyNodeGetName(child);
            if (name)
            {
                TidyTagId type = tidyNodeGetId(child);
                if (type == TidyTagId::TidyTag_IMG)
                {
                    auto srcAttr = tidyAttrGetById(child, TidyAttrId::TidyAttr_SRC);
                    if (srcAttr)
                    {
                        auto srcValue = tidyAttrValue(srcAttr);
                        if (srcValue)
                        {
                            // TODO: Get src value
                        }
                    }
                }
            }
            else
            {
                TidyBuffer buf;
                tidyBufInit(&buf);
                tidyNodeGetText(tdoc, child, &buf);
                if (buf.bp)
                {
                    height += CreateTextEntity(reinterpret_cast<const char*>(buf.bp), rootElementEntityId, beginHeight + height);
                }

                tidyBufFree(&buf);
            }

            height += CreateHtmlNodeEntities(tdoc, child, rootElementEntityId, beginHeight + height); 
        }

        return height;
    }

    float HtmlUiComponentHelper::CreateTextEntity(const AZStd::string& text, const AZ::EntityId& rootElementEntityId, float beginHeight)
    {
        float height = 0.0f;

        // Create the text element
        AZ::Entity* textEntity = nullptr;
        UiElementBus::EventResult(textEntity, rootElementEntityId, &UiElementBus::Events::CreateChildElement, AZStd::string("text"));

        // Set up the text element
        if (textEntity)
        {
            textEntity->Deactivate();
            textEntity->CreateComponent(LyShine::UiTransform2dComponentUuid);
            textEntity->CreateComponent(LyShine::UiTextComponentUuid);
            textEntity->Activate();

            AZ::EntityId textEntityId = textEntity->GetId();
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetAnchors, UiTransform2dInterface::Anchors{}, false, false);
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetOffsets, UiTransform2dInterface::Offsets{-300.0f, -256.0f + beginHeight, 300.0f, 256.0f});
            UiTransform2dBus::Event(textEntityId, &UiTransform2dBus::Events::SetPivotAndAdjustOffsets, AZ::Vector2(0.5f));

            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetText, text);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetTextAlignment, IDraw2d::HAlign::Left, IDraw2d::VAlign::Top);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetWrapText, UiTextInterface::WrapTextSetting::Wrap);
            UiTextBus::Event(textEntityId, &UiTextBus::Events::SetFontSize, 18.0f);

            if (!text.empty())
            {
                UiTransformInterface::RectPointsArray textBounding;
                UiTextBus::EventResult(height, textEntityId, &UiTextBus::Events::GetTextHeight);
            }

            // Trigger all InGamePostActivate
            UiInitializationBus::Event(textEntityId, &UiInitializationBus::Events::InGamePostActivate);
        }

        return height;
    }
} // namespace Cesium
