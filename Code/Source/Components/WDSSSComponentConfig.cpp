/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/WDSSSComponentConfig.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace AZ
{
    namespace Render
    {
        void WDSSSComponentConfig::Reflect(ReflectContext* context)
        {
            // NOTE: the wrapped WDSSSConfiguration (and its edit context with the sliders) is reflected by
            // WDSSSSystemComponent::Reflect, which is always registered (it is a required system component
            // in the same module). We must NOT reflect it again here - double-registering its fields
            // corrupts serialization (duplicate keys + dropped values in the saved prefab / on play).
            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<WDSSSComponentConfig, ComponentConfig>()
                    ->Version(1)
                    ->Field("Settings", &WDSSSComponentConfig::m_settings);
            }
        }
    } // namespace Render
} // namespace AZ
