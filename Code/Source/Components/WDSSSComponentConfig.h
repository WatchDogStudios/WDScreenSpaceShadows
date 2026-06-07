/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <WDScreenSpaceShadows/WDSSSConfiguration.h>

namespace AZ
{
    namespace Render
    {
        //! Component configuration for the WDScreenSpaceShadows Level component. Wraps the settings block
        //! so it can be authored on a Level entity and pushed to the feature processor.
        class WDSSSComponentConfig final
            : public ComponentConfig
        {
        public:
            AZ_RTTI(WDSSSComponentConfig, "{D4E5F607-1829-4A3B-CD4E-5F6071829304}", ComponentConfig);
            AZ_CLASS_ALLOCATOR(WDSSSComponentConfig, SystemAllocator);

            static void Reflect(ReflectContext* context);

            WDSSSConfiguration m_settings;
        };
    } // namespace Render
} // namespace AZ
