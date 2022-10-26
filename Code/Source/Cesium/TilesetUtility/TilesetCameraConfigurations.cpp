/*
 * Copyright (c) Contributors to the Cesium for O3DE Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * 2022-09 - Modifications for Linux Platform support - Huawei Technologies Co., Ltd <foss@huawei.com>
 */

#include <Cesium/TilesetUtility/TilesetCameraConfigurations.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/View.h>

namespace Cesium
{
    TilesetCameraConfigurations::TilesetCameraConfigurations()
        : m_transform{ 1.0 }
    {
    }

    void TilesetCameraConfigurations::SetTransform(const glm::dmat4& transform)
    {
        m_transform = transform;
    }

    const glm::dmat4& TilesetCameraConfigurations::GetTransform() const
    {
        return m_transform;
    }

    const std::vector<Cesium3DTilesSelection::ViewState>& TilesetCameraConfigurations::UpdateAndGetViewStates()
    {
        m_viewStates.clear();
        auto viewportManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
        if (!viewportManager)
        {
            return m_viewStates;
        }

        viewportManager->EnumerateViewportContexts(
            [this](AZ::RPI::ViewportContextPtr viewportContextPtr) mutable
            {
                AzFramework::WindowSize windowSize = viewportContextPtr->GetViewportSize();
                if (windowSize.m_width == 0 || windowSize.m_height == 0)
                {
                    return;
                }

                m_viewStates.emplace_back(GetViewState(viewportContextPtr, m_transform));
            });

        return m_viewStates;
    }

    Cesium3DTilesSelection::ViewState TilesetCameraConfigurations::GetViewState(
        const AZ::RPI::ViewportContextPtr& viewportContextPtr, const glm::dmat4& transform)
    {
        // Get o3de camera configuration
        AZ::RPI::ViewPtr view = viewportContextPtr->GetDefaultView();
        AZ::Transform o3deCameraTransform = view->GetCameraTransform();
        AZ::Vector3 o3deCameraFwd = o3deCameraTransform.GetBasis(1);
        AZ::Vector3 o3deCameraUp = o3deCameraTransform.GetBasis(2);
        AZ::Vector3 o3deCameraPosition = o3deCameraTransform.GetTranslation();

        // Convert o3de coordinate to cesium coordinate
        glm::dvec3 position =
            glm::dvec3(transform * glm::dvec4{ o3deCameraPosition.GetX(), o3deCameraPosition.GetY(), o3deCameraPosition.GetZ(), 1.0 });
        glm::dvec3 direction = glm::dvec3(transform * glm::dvec4{ o3deCameraFwd.GetX(), o3deCameraFwd.GetY(), o3deCameraFwd.GetZ(), 0.0 });
        glm::dvec3 up = glm::dvec3(transform * glm::dvec4{ o3deCameraUp.GetX(), o3deCameraUp.GetY(), o3deCameraUp.GetZ(), 0.0 });
        direction = glm::normalize(direction);
        up = glm::normalize(up);

        const auto& projectMatrix = view->GetViewToClipMatrix();
        AzFramework::WindowSize windowSize = viewportContextPtr->GetViewportSize();
        glm::dvec2 viewportSize{ windowSize.m_width, windowSize.m_height };
        double aspect = viewportSize.x / viewportSize.y;
        double verticalFov = 2.0 * glm::atan(1.0 / projectMatrix.GetElement(1, 1));
        double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);
        return Cesium3DTilesSelection::ViewState::create(position, direction, up, viewportSize, horizontalFov, verticalFov);
    }
} // namespace Cesium