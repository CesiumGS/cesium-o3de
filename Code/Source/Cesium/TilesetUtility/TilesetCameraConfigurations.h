#pragma once

#include <Atom/RPI.Public/Base.h>
#include <Cesium3DTilesSelection/ViewState.h>
#include <glm/glm.hpp>
#include <vector>

namespace Cesium
{
    class TilesetCameraConfigurations
    {
    public:
        TilesetCameraConfigurations();

        void SetTransform(const glm::dmat4& transform);

        const glm::dmat4& GetTransform() const;

        const std::vector<Cesium3DTilesSelection::ViewState>& UpdateAndGetViewStates();

    private:
        static Cesium3DTilesSelection::ViewState GetViewState(
            const AZ::RPI::ViewportContextPtr& viewportContextPtr, const glm::dmat4& transform);

        glm::dmat4 m_transform;
        std::vector<Cesium3DTilesSelection::ViewState> m_viewStates;
    };

} // namespace Cesium