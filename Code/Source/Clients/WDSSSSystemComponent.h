/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>

#if defined(IMGUI_ENABLED)
#include <ImGuiBus.h>
#endif

namespace AZ
{
    namespace Render
    {
        //! System component for the WD Screen Space Shadows (Bend) gem. Registers the feature processor,
        //! the compute/debug/apply pass classes and their template mappings, owns the opt-in console
        //! variables, and draws the "WD Screen Space Shadows" ImGui tuning panel when ImGui is available.
        class WDSSSSystemComponent final
            : public AZ::Component
#if defined(IMGUI_ENABLED)
            , public ImGui::ImGuiUpdateListenerBus::Handler
#endif
        {
        public:
            AZ_COMPONENT(AZ::Render::WDSSSSystemComponent, "{B1E7B2A4-9C3D-4E5F-A6B7-0C1D2E3F4A5B}");

            static void Reflect(AZ::ReflectContext* context);

            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
            static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
            static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

            WDSSSSystemComponent() = default;
            ~WDSSSSystemComponent() override = default;

        protected:
            // AZ::Component overrides
            void Activate() override;
            void Deactivate() override;

#if defined(IMGUI_ENABLED)
            // ImGui::ImGuiUpdateListenerBus::Handler overrides
            void OnImGuiMainMenuUpdate() override;
            void OnImGuiUpdate() override;
#endif

        private:
            void LoadPassTemplateMappings();

            AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler m_loadTemplatesHandler;

#if defined(IMGUI_ENABLED)
            bool m_showImGuiWindow = false;
#endif
        };
    } // namespace Render
} // namespace AZ
