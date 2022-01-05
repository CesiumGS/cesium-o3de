#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/stack.h>
#include <tidy.h>

namespace Cesium
{
    class HtmlUiComponentHelper
    {
    public:
        static AZ::EntityId CreateHtmlCanvasEntity(const AZStd::string& html);

    private:
        static float CreateHtmlNodeEntities(TidyDoc doc, TidyNode node, const AZ::EntityId& rootElementEntityId, float beginHeight);

        static float CreateTextEntity(const AZStd::string& text, const AZ::EntityId& rootElementEntityId, float beginHeight);
    };
}
