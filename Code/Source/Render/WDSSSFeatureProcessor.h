/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <WDScreenSpaceShadows/WDSSSFeatureProcessorInterface.h>
#include <AzCore/Math/Matrix4x4.h>
#include <AzCore/Math/Vector3.h>

namespace AZ
{
    namespace Render
    {
        //! Feature processor for Bend screen-space shadows.
        //!
        //! Responsibilities:
        //!  - Owns the runtime configuration (tunables + toggles).
        //!  - Each frame, caches the primary view's projection and the (config-driven) sun direction in
        //!    view space, which the compute pass turns into a Bend dispatch list.
        //!  - Injects the WDSSS parent pass into the pipeline immediately before the Forward pass.
        class WDSSSFeatureProcessor final
            : public WDSSSFeatureProcessorInterface
        {
        public:
            AZ_CLASS_ALLOCATOR(WDSSSFeatureProcessor, AZ::SystemAllocator)
            AZ_RTTI(AZ::Render::WDSSSFeatureProcessor, "{C3E5A7B9-4D6F-4A8B-9C1D-2E3F4A5B6C7D}", AZ::Render::WDSSSFeatureProcessorInterface);

            static void Reflect(AZ::ReflectContext* context);

            WDSSSFeatureProcessor() = default;
            virtual ~WDSSSFeatureProcessor() = default;

            // RPI::FeatureProcessor overrides
            void Activate() override;
            void Deactivate() override;
            void Render(const RenderPacket& packet) override;
            void AddRenderPasses(RPI::RenderPipeline* renderPipeline) override;

            // WDSSSFeatureProcessorInterface overrides
            void SetConfiguration(const WDSSSConfiguration& config) override { m_configuration = config; }
            const WDSSSConfiguration& GetConfiguration() const override { return m_configuration; }
            void SetEnabled(bool enabled) override { m_configuration.m_enabled = enabled; }
            bool GetEnabled() const override { return m_configuration.m_enabled; }
            void SetApplyToSun(bool apply) override { m_configuration.m_applyToSun = apply; }
            void SetDebugView(int mode) override { m_configuration.m_debugView = mode; }
            int GetDebugView() const override { return m_configuration.m_debugView; }

            //! Accessors used by the WDSSS passes.
            const AZ::Vector3& GetSunDirectionView() const { return m_sunDirView; }
            const AZ::Matrix4x4& GetViewToClip() const { return m_viewToClip; }

            //! True when the compute + debug passes should run this frame.
            bool ShouldRender() const { return m_configuration.m_enabled; }
            //! True when the apply pass should min the mask into the sun-visibility buffer.
            bool ShouldApply() const { return m_configuration.m_enabled && m_configuration.m_applyToSun; }

        private:
            AZ_DISABLE_COPY_MOVE(WDSSSFeatureProcessor);

            WDSSSConfiguration m_configuration;
            AZ::Vector3 m_sunDirView = AZ::Vector3(0.0f, 0.0f, -1.0f);
            AZ::Matrix4x4 m_viewToClip = AZ::Matrix4x4::CreateIdentity();
        };
    } // namespace Render
} // namespace AZ
