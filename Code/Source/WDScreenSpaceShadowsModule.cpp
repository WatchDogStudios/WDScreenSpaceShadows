/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <WDScreenSpaceShadowsModule.h>
#include <Clients/WDSSSSystemComponent.h>
#include <Components/WDSSSComponent.h>

#ifdef WDSSS_EDITOR
#include <EditorComponents/EditorWDSSSComponent.h>
#endif

namespace AZ
{
    namespace Render
    {
        WDScreenSpaceShadowsModule::WDScreenSpaceShadowsModule()
        {
            m_descriptors.insert(m_descriptors.end(),
                {
                    WDSSSSystemComponent::CreateDescriptor(),
                    WDSSSComponent::CreateDescriptor(),

#ifdef WDSSS_EDITOR
                    EditorWDSSSComponent::CreateDescriptor(),
#endif
                });
        }

        AZ::ComponentTypeList WDScreenSpaceShadowsModule::GetRequiredSystemComponents() const
        {
            return AZ::ComponentTypeList{ azrtti_typeid<WDSSSSystemComponent>() };
        }
    } // namespace Render
} // namespace AZ

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), AZ::Render::WDScreenSpaceShadowsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_WDScreenSpaceShadows, AZ::Render::WDScreenSpaceShadowsModule)
#endif
