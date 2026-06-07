/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Render/Passes/WDSSSApplyPass.h>
#include <Render/WDSSSFeatureProcessor.h>

#include <Atom/RHI/FrameGraphInterface.h>
#include <Atom/RHI/FrameGraphCompileContext.h>
#include <Atom/RHI/FrameGraphExecuteContext.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Scene.h>

namespace AZ
{
    namespace Render
    {
        RPI::Ptr<WDSSSApplyPass> WDSSSApplyPass::Create(const RPI::PassDescriptor& descriptor)
        {
            return aznew WDSSSApplyPass(descriptor);
        }

        WDSSSApplyPass::WDSSSApplyPass(const RPI::PassDescriptor& descriptor)
            : RPI::RenderPass(descriptor)
        {
            LoadShader();
        }

        void WDSSSApplyPass::LoadShader()
        {
            m_shader = RPI::LoadCriticalShader("Shaders/WDScreenSpaceShadows/WDSSSApply.azshader");
            if (m_shader == nullptr)
            {
                AZ_Error("WDSSSApplyPass", false, "Failed to load the WDSSS apply shader.");
                return;
            }

            RHI::PipelineStateDescriptorForDispatch pipelineStateDescriptor;
            const auto& shaderVariant = m_shader->GetVariant(RPI::ShaderAsset::RootShaderVariantStableId);
            shaderVariant.ConfigurePipelineState(pipelineStateDescriptor);
            m_pipelineState = m_shader->AcquirePipelineState(pipelineStateDescriptor);

            m_srgLayout = m_shader->FindShaderResourceGroupLayout(RPI::SrgBindingSlot::Pass);
            if (m_srgLayout)
            {
                m_passSrg = RPI::ShaderResourceGroup::Create(
                    m_shader->GetAsset(), m_shader->GetSupervariantIndex(), m_srgLayout->GetName());
            }

            const auto outcome = RPI::GetComputeShaderNumThreads(m_shader->GetAsset(), m_dispatchArgs);
            if (!outcome.IsSuccess())
            {
                AZ_Error("WDSSSApplyPass", false, "WDSSS apply shader has invalid numthreads:\n%s",
                    outcome.GetError().c_str());
            }
        }

        WDSSSFeatureProcessor* WDSSSApplyPass::GetFeatureProcessor() const
        {
            RPI::Scene* scene = m_pipeline ? m_pipeline->GetScene() : nullptr;
            return scene ? scene->GetFeatureProcessor<WDSSSFeatureProcessor>() : nullptr;
        }

        const RHI::ImageView* WDSSSApplyPass::GetInputImageView(
            const RHI::FrameGraphCompileContext& context, const AZ::Name& slotName) const
        {
            const RPI::PassAttachmentBinding* binding = FindAttachmentBinding(slotName);
            if (!binding || !binding->GetAttachment())
            {
                return nullptr;
            }
            return context.GetImageView(binding->GetAttachment()->GetAttachmentId());
        }

        bool WDSSSApplyPass::IsEnabled() const
        {
            if (!RenderPass::IsEnabled())
            {
                return false;
            }
            if (!m_passSrg || !m_pipelineState)
            {
                return false;
            }
            WDSSSFeatureProcessor* fp = GetFeatureProcessor();
            return fp && fp->ShouldApply();
        }

        void WDSSSApplyPass::SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph)
        {
            // Declares the connected slot attachments (ShadowMask read, FullscreenShadow read-write).
            RenderPass::SetupFrameGraphDependencies(frameGraph);
            frameGraph.SetEstimatedItemCount(1);
        }

        void WDSSSApplyPass::CompileResources(const RHI::FrameGraphCompileContext& context)
        {
            if (!m_passSrg)
            {
                return;
            }

            const RHI::ShaderResourceGroupLayout* layout = m_passSrg->GetLayout();
            auto setImage = [&](const char* name, const RHI::ImageView* view)
            {
                if (!view)
                {
                    return;
                }
                const RHI::ShaderInputImageIndex index = layout->FindShaderInputImageIndex(AZ::Name(name));
                if (index.IsValid())
                {
                    m_passSrg->SetImageView(index, view);
                }
            };
            setImage("m_shadowMask", GetInputImageView(context, m_maskSlotName));
            setImage("m_fullscreenShadow", GetInputImageView(context, m_fullscreenSlotName));

            if (!m_passSrg->IsQueuedForCompile())
            {
                m_passSrg->Compile();
            }

            // One thread per pixel of the fullscreen-shadow buffer.
            RHI::Size targetSize;
            if (const RPI::PassAttachmentBinding* binding = FindAttachmentBinding(m_fullscreenSlotName);
                binding && binding->GetAttachment())
            {
                targetSize = binding->GetAttachment()->m_descriptor.m_image.m_size;
            }

            RHI::DispatchDirect args = m_dispatchArgs;
            args.m_totalNumberOfThreadsX = targetSize.m_width;
            args.m_totalNumberOfThreadsY = targetSize.m_height;
            args.m_totalNumberOfThreadsZ = 1;
            m_dispatchItem.SetPipelineState(m_pipelineState);
            m_dispatchItem.SetArguments(args);
        }

        void WDSSSApplyPass::BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context)
        {
            if (!m_passSrg)
            {
                return;
            }

            const uint32_t deviceIndex = context.GetDeviceIndex();
            RHI::CommandList* commandList = context.GetCommandList();

            commandList->SetShaderResourceGroupForDispatch(
                *m_passSrg->GetRHIShaderResourceGroup()->GetDeviceShaderResourceGroup(deviceIndex));
            commandList->Submit(m_dispatchItem.GetDeviceDispatchItem(deviceIndex), 0);
        }
    } // namespace Render
} // namespace AZ
