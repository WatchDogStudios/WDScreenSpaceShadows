/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Render/WDSSSFeatureProcessor.h>

#include <AzCore/Math/Vector4.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/View.h>

namespace AZ
{
    namespace Render
    {
        void WDSSSFeatureProcessor::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<WDSSSFeatureProcessor, FeatureProcessor>()
                    ->Version(1);
            }
        }

        void WDSSSFeatureProcessor::Activate()
        {
            EnableSceneNotification();
        }

        void WDSSSFeatureProcessor::Deactivate()
        {
            DisableSceneNotification();
        }

        void WDSSSFeatureProcessor::Render(const RenderPacket& packet)
        {
            if (!m_configuration.m_enabled)
            {
                return;
            }

            // Cache the primary view's projection + the (config-driven) sun direction in view space.
            // Passing (sunDirView, viewToClip) to Bend is equivalent to (sunDirWorld, worldToClip):
            // viewToClip * (worldToView * sunWorld) == worldToClip * sunWorld.
            if (!packet.m_views.empty() && packet.m_views.front())
            {
                const auto& view = packet.m_views.front();
                m_viewToClip = view->GetViewToClipMatrix();

                // Sun travel direction (toward surfaces). If a Sun Entity is referenced, inherit the exact
                // direction the lighting uses: O3DE's DirectionalLightComponent sets the light direction to
                // the entity's +Y basis (DirectionalLightComponentController). Otherwise use the config vector.
                AZ::Vector3 sunTravel = m_configuration.m_sunDirectionWorld;
                if (m_configuration.m_sunEntityId.IsValid())
                {
                    AZ::Transform sunTransform = AZ::Transform::CreateIdentity();
                    AZ::TransformBus::EventResult(
                        sunTransform, m_configuration.m_sunEntityId, &AZ::TransformBus::Events::GetWorldTM);
                    sunTravel = sunTransform.GetBasisY();
                }

                // Bend marches shadow rays TOWARD the light, so negate the travel direction.
                const AZ::Vector3 sunWorld = (-sunTravel).GetNormalizedSafe();
                const AZ::Vector4 sunView =
                    view->GetWorldToViewMatrix() * AZ::Vector4(sunWorld.GetX(), sunWorld.GetY(), sunWorld.GetZ(), 0.0f);
                m_sunDirView = sunView.GetAsVector3().GetNormalizedSafe();
            }
        }

        void WDSSSFeatureProcessor::AddRenderPasses(RPI::RenderPipeline* renderPipeline)
        {
            // Only inject into pipelines that expose a Forward (lighting) pass - that is where the
            // FullscreenShadowBuffer the apply pass writes is consumed.
            RPI::PassFilter forwardFilter = RPI::PassFilter::CreateWithPassName(AZ::Name("Forward"), renderPipeline);
            if (!RPI::PassSystemInterface::Get()->FindFirstPass(forwardFilter))
            {
                return;
            }

            // Skip if we have already injected the parent into this pipeline.
            RPI::PassFilter existingFilter = RPI::PassFilter::CreateWithPassName(AZ::Name("WDSSSPass"), renderPipeline);
            if (RPI::PassSystemInterface::Get()->FindFirstPass(existingFilter))
            {
                return;
            }

            // Insert BEFORE the Forward pass: the mask is produced (and optionally min'd into the
            // FullscreenShadowBuffer pipeline-global) before Forward reads m_fullscreenShadow.
            RPI::AddPassRequestToRenderPipeline(
                renderPipeline,
                "Passes/WDSSSParentPassRequest.azasset",
                "Forward",
                /*beforeReferencePass=*/true);

            // Inject the debug visualization pass LATE - just before the swap-chain copy - so its overlay
            // lands on the final composited image instead of being overwritten by the scene + post-process.
            // (It only runs when the debug view is enabled; otherwise it disables itself.)
            RPI::PassFilter copyFilter = RPI::PassFilter::CreateWithPassName(AZ::Name("CopyToSwapChain"), renderPipeline);
            if (RPI::PassSystemInterface::Get()->FindFirstPass(copyFilter))
            {
                RPI::AddPassRequestToRenderPipeline(
                    renderPipeline,
                    "Passes/WDSSSDebugPassRequest.azasset",
                    "CopyToSwapChain",
                    /*beforeReferencePass=*/true);
            }
        }
    } // namespace Render
} // namespace AZ
