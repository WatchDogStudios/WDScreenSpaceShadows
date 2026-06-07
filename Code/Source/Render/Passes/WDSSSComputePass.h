/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/fixed_vector.h>
#include <Atom/RHI/DispatchItem.h>
#include <Atom/RPI.Public/Pass/RenderPass.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <Render/WDSSSDispatch.h>

namespace AZ
{
    namespace Render
    {
        class WDSSSFeatureProcessor;

        //! Compute pass that produces the Bend screen-space shadow mask. It derives RenderPass directly
        //! (rather than a single-dispatch helper) because Bend issues several dispatches per frame - one
        //! per entry in the CPU dispatch list - each with its own wave offset + light coordinate. We use
        //! one ShaderResourceGroup per dispatch (compiled up front), then submit them in sequence.
        class WDSSSComputePass
            : public RPI::RenderPass
        {
        public:
            AZ_RTTI(AZ::Render::WDSSSComputePass, "{1C2D3E4F-5A6B-7C8D-9E0F-1A2B3C4D5E6F}", RPI::RenderPass);
            AZ_CLASS_ALLOCATOR(WDSSSComputePass, SystemAllocator);

            static RPI::Ptr<WDSSSComputePass> Create(const RPI::PassDescriptor& descriptor);

        protected:
            explicit WDSSSComputePass(const RPI::PassDescriptor& descriptor);

            WDSSSFeatureProcessor* GetFeatureProcessor() const;
            const RHI::ImageView* GetInputImageView(const RHI::FrameGraphCompileContext& context, const AZ::Name& slotName) const;

            // RenderPass overrides
            bool IsEnabled() const override;
            void SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph) override;
            void CompileResources(const RHI::FrameGraphCompileContext& context) override;
            void BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context) override;

        private:
            void LoadShader();
            void BindDispatchSrg(RPI::ShaderResourceGroup* srg, const RHI::ImageView* depth,
                const RHI::ImageView* mask, const PerDispatchConstants& constants) const;

            Data::Instance<RPI::Shader> m_shader;
            const RHI::PipelineState* m_pipelineState = nullptr;
            RHI::Ptr<RHI::ShaderResourceGroupLayout> m_srgLayout;

            RHI::DispatchDirect m_dispatchArgs; // numthreads from the shader (WAVE_SIZE,1,1)
            AZStd::fixed_vector<DispatchEntry, 8> m_dispatches;
            AZStd::fixed_vector<Data::Instance<RPI::ShaderResourceGroup>, 8> m_dispatchSrgs;

            const AZ::Name m_depthSlotName{ "Depth" };
            const AZ::Name m_maskSlotName{ "ShadowMask" };
        };
    } // namespace Render
} // namespace AZ
