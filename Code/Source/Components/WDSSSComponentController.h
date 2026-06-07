/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <WDScreenSpaceShadows/WDSSSFeatureProcessorInterface.h>
#include <Components/WDSSSComponentConfig.h>

namespace AZ
{
    namespace Render
    {
        //! Drives the WDScreenSpaceShadows feature processor from a Level-entity component's configuration.
        //! There is one logical SSS effect per scene, so this is a non-placed, global component - it just
        //! forwards the settings block to the feature processor on activate / change.
        class WDSSSComponentController final
        {
        public:
            friend class EditorWDSSSComponent;

            AZ_TYPE_INFO(AZ::Render::WDSSSComponentController, "{C3D4E5F6-0718-492A-BC3D-4E5F60718293}");

            static void Reflect(AZ::ReflectContext* context);
            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
            static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

            WDSSSComponentController() = default;
            WDSSSComponentController(const WDSSSComponentConfig& config);

            void Activate(AZ::EntityId entityId);
            void Deactivate();
            void SetConfiguration(const WDSSSComponentConfig& config);
            const WDSSSComponentConfig& GetConfiguration() const;

        private:
            AZ_DISABLE_COPY(WDSSSComponentController);

            void OnConfigChanged();

            WDSSSComponentConfig m_configuration;
            WDSSSFeatureProcessorInterface* m_featureProcessor = nullptr;
            AZ::EntityId m_entityId;
        };
    } // namespace Render
} // namespace AZ
