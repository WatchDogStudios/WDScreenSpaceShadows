/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Component/EntityId.h>

namespace AZ
{
    namespace Render
    {
        //! Runtime tunables for Bend screen-space shadows. The Bend-derived fields map directly onto the
        //! scalar members of Bend::DispatchParameters (see External/BendSSS/bend_sss_gpu.h); the gem
        //! toggles (enabled/apply/debug) drive the gem's passes. Opt-in: m_enabled defaults to false.
        //!
        //! Note: the shadow length (Bend SAMPLE_COUNT / HARD_SHADOW_SAMPLES / FADE_OUT_SAMPLES) is a
        //! compile-time concern in v1 (it sizes a groupshared array + drives loop unrolling), so it is a
        //! shader #define rather than a runtime field here.
        struct WDSSSConfiguration
        {
            AZ_TYPE_INFO(AZ::Render::WDSSSConfiguration, "{D2F8C3B5-1A4E-4F60-9B7C-2E3D4F5A6B7C}");

            static void Reflect(AZ::ReflectContext* context);

            // Gem toggles
            bool m_enabled = false;     //!< master opt-in (r.WDSSS.Enable) - produce the mask
            bool m_applyToSun = false;  //!< min the mask into FullscreenShadowBuffer (r.WDSSS.Apply)
            int  m_debugView = 0;       //!< 0 = Off, 1 = Mask visualization (r.WDSSS.DebugView)

            //! World-space sun travel direction (the way light moves, toward surfaces). v1 is config-driven:
            //! DirectionalLightFeatureProcessorInterface exposes no CPU-side direction getter and Bend's
            //! dispatch builder needs the light's screen position on the CPU, so set this to match the
            //! scene's sun. Auto-follow is a documented follow-up. Normalized by the feature processor.
            AZ::Vector3 m_sunDirectionWorld = AZ::Vector3(0.4f, -0.85f, 0.35f);

            //! Optional: inherit the sun direction from this entity's transform (the +Y basis, matching
            //! O3DE's DirectionalLightComponent). When set to a valid entity it OVERRIDES m_sunDirectionWorld,
            //! so the screen-space shadows track the exact same sun the scene lighting uses.
            AZ::EntityId m_sunEntityId;

            // Bend DispatchParameters scalars (recommended starting values from bend_sss_gpu.h)
            float m_surfaceThickness = 0.005f;          //!< SurfaceThickness (0.5%)
            float m_bilinearThreshold = 0.02f;          //!< BilinearThreshold (2%)
            float m_shadowContrast = 4.0f;              //!< ShadowContrast (>= 1)
            bool  m_ignoreEdgePixels = false;           //!< IgnoreEdgePixels
            bool  m_usePrecisionOffset = false;         //!< UsePrecisionOffset
            bool  m_bilinearSamplingOffsetMode = false; //!< BilinearSamplingOffsetMode
        };
    } // namespace Render
} // namespace AZ
