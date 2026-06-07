/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>
#include <Components/WDSSSComponent.h>

namespace AZ
{
    namespace Render
    {
        //! Editor wrapper for the WDScreenSpaceShadows Level component. Appears under Add Component >
        //! Level/Game > Graphics/Lighting and edits the SSS settings block in the entity inspector.
        class EditorWDSSSComponent final
            : public AzToolsFramework::Components::EditorComponentAdapter<WDSSSComponentController, WDSSSComponent, WDSSSComponentConfig>
        {
        public:
            using BaseClass = AzToolsFramework::Components::EditorComponentAdapter<WDSSSComponentController, WDSSSComponent, WDSSSComponentConfig>;
            AZ_EDITOR_COMPONENT(AZ::Render::EditorWDSSSComponent, EditorWDSSSComponentTypeId, BaseClass);

            static void Reflect(AZ::ReflectContext* context);

            EditorWDSSSComponent() = default;
            EditorWDSSSComponent(const WDSSSComponentConfig& config);

            //! EditorComponentAdapter overrides...
            AZ::u32 OnConfigurationChanged() override;
        };
    } // namespace Render
} // namespace AZ
