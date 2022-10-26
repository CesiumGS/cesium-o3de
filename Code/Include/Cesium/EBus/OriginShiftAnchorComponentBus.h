/*
 * Copyright (c) Contributors to the Cesium for O3DE Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * 2022-09 - Modifications for Linux Platform support - Huawei Technologies Co., Ltd <foss@huawei.com>
 */

#pragma once

#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftAnchorRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual glm::dvec3 GetPosition() const = 0;

        virtual void SetPosition(const glm::dvec3& pos) = 0;
    };

    using OriginShiftAnchorRequestBus = AZ::EBus<OriginShiftAnchorRequest>;
} // namespace Cesium
