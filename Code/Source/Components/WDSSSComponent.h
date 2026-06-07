/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Components/ComponentAdapter.h>
#include <Components/WDSSSComponentConfig.h>
#include <Components/WDSSSComponentConstants.h>
#include <Components/WDSSSComponentController.h>

namespace AZ
{
    namespace Render
    {
        //! Runtime Level component that configures the WDScreenSpaceShadows feature processor.
        class WDSSSComponent final
            : public AzFramework::Components::ComponentAdapter<WDSSSComponentController, WDSSSComponentConfig>
        {
        public:
            using BaseClass = AzFramework::Components::ComponentAdapter<WDSSSComponentController, WDSSSComponentConfig>;
            AZ_COMPONENT(AZ::Render::WDSSSComponent, WDSSSComponentTypeId, BaseClass);

            WDSSSComponent() = default;
            WDSSSComponent(const WDSSSComponentConfig& config);

            static void Reflect(AZ::ReflectContext* context);
        };
    } // namespace Render
} // namespace AZ
