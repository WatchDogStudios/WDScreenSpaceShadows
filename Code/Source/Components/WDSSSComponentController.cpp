/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/WDSSSComponentController.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Atom/RPI.Public/Scene.h>

namespace AZ
{
    namespace Render
    {
        void WDSSSComponentController::Reflect(AZ::ReflectContext* context)
        {
            WDSSSComponentConfig::Reflect(context);

            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<WDSSSComponentController>()
                    ->Version(1)
                    ->Field("Configuration", &WDSSSComponentController::m_configuration);
            }
        }

        void WDSSSComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            // Distinct from the system component's "WDSSSService": this only enforces a single SSS level
            // component per entity hierarchy; it does not conflict with the always-present system component.
            provided.push_back(AZ_CRC_CE("WDSSSLevelService"));
        }

        void WDSSSComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("WDSSSLevelService"));
        }

        void WDSSSComponentController::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
        {
        }

        WDSSSComponentController::WDSSSComponentController(const WDSSSComponentConfig& config)
            : m_configuration(config)
        {
        }

        void WDSSSComponentController::Activate(AZ::EntityId entityId)
        {
            m_entityId = entityId;
            m_featureProcessor = RPI::Scene::GetFeatureProcessorForEntity<WDSSSFeatureProcessorInterface>(entityId);
            OnConfigChanged();
        }

        void WDSSSComponentController::Deactivate()
        {
            m_featureProcessor = nullptr;
            m_entityId = AZ::EntityId();
        }

        void WDSSSComponentController::SetConfiguration(const WDSSSComponentConfig& config)
        {
            m_configuration = config;
            OnConfigChanged();
        }

        const WDSSSComponentConfig& WDSSSComponentController::GetConfiguration() const
        {
            return m_configuration;
        }

        void WDSSSComponentController::OnConfigChanged()
        {
            if (m_featureProcessor)
            {
                m_featureProcessor->SetConfiguration(m_configuration.m_settings);
            }
        }
    } // namespace Render
} // namespace AZ
