/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Module/Module.h>

namespace AZ
{
    namespace Render
    {
        class WDScreenSpaceShadowsModule
            : public AZ::Module
        {
        public:
            AZ_RTTI(WDScreenSpaceShadowsModule, "{7D1D0F3B-2C4E-5F60-AB8C-3D9E2F5A7B01}", AZ::Module);
            AZ_CLASS_ALLOCATOR(WDScreenSpaceShadowsModule, AZ::SystemAllocator);

            WDScreenSpaceShadowsModule();
            ~WDScreenSpaceShadowsModule() override = default;

            //! Add required SystemComponents to the SystemEntity.
            AZ::ComponentTypeList GetRequiredSystemComponents() const override;
        };
    } // namespace Render
} // namespace AZ
