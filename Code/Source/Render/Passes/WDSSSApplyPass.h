/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <Atom/RHI/DispatchItem.h>
#include <Atom/RPI.Public/Pass/RenderPass.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>

namespace AZ
{
    namespace Render
    {
        class WDSSSFeatureProcessor;

        //! Combines the screen-space shadow mask into the FullscreenShadowBuffer pipeline-global in place
        //! (min), so the Forward lighting darkens the sun where Bend found a contact shadow. Runs only
        //! when the feature processor's "apply to sun" toggle is on.
        class WDSSSApplyPass
            : public RPI::RenderPass
        {
        public:
            AZ_RTTI(AZ::Render::WDSSSApplyPass, "{2D3E4F5A-6B7C-4D8E-9F0A-1B2C3D4E5F60}", RPI::RenderPass);
            AZ_CLASS_ALLOCATOR(WDSSSApplyPass, SystemAllocator);

            static RPI::Ptr<WDSSSApplyPass> Create(const RPI::PassDescriptor& descriptor);

        protected:
            explicit WDSSSApplyPass(const RPI::PassDescriptor& descriptor);

            WDSSSFeatureProcessor* GetFeatureProcessor() const;
            const RHI::ImageView* GetInputImageView(const RHI::FrameGraphCompileContext& context, const AZ::Name& slotName) const;

            // RenderPass overrides
            bool IsEnabled() const override;
            void SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph) override;
            void CompileResources(const RHI::FrameGraphCompileContext& context) override;
            void BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context) override;

        private:
            void LoadShader();

            Data::Instance<RPI::Shader> m_shader;
            const RHI::PipelineState* m_pipelineState = nullptr;
            RHI::Ptr<RHI::ShaderResourceGroupLayout> m_srgLayout;
            Data::Instance<RPI::ShaderResourceGroup> m_passSrg;
            RHI::DispatchDirect m_dispatchArgs;
            RHI::DispatchItem m_dispatchItem{ RHI::MultiDevice::AllDevices };

            const AZ::Name m_maskSlotName{ "ShadowMask" };
            const AZ::Name m_fullscreenSlotName{ "FullscreenShadow" };
        };
    } // namespace Render
} // namespace AZ
