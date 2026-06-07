/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Render/Passes/WDSSSComputePass.h>
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
        RPI::Ptr<WDSSSComputePass> WDSSSComputePass::Create(const RPI::PassDescriptor& descriptor)
        {
            return aznew WDSSSComputePass(descriptor);
        }

        WDSSSComputePass::WDSSSComputePass(const RPI::PassDescriptor& descriptor)
            : RPI::RenderPass(descriptor)
        {
            LoadShader();
        }

        void WDSSSComputePass::LoadShader()
        {
            m_shader = RPI::LoadCriticalShader("Shaders/WDScreenSpaceShadows/WDSSSCompute.azshader");
            if (m_shader == nullptr)
            {
                AZ_Error("WDSSSComputePass", false, "Failed to load the WDSSS compute shader.");
                return;
            }

            RHI::PipelineStateDescriptorForDispatch pipelineStateDescriptor;
            const auto& shaderVariant = m_shader->GetVariant(RPI::ShaderAsset::RootShaderVariantStableId);
            shaderVariant.ConfigurePipelineState(pipelineStateDescriptor);
            m_pipelineState = m_shader->AcquirePipelineState(pipelineStateDescriptor);

            m_srgLayout = m_shader->FindShaderResourceGroupLayout(RPI::SrgBindingSlot::Pass);

            const auto outcome = RPI::GetComputeShaderNumThreads(m_shader->GetAsset(), m_dispatchArgs);
            if (!outcome.IsSuccess())
            {
                AZ_Error("WDSSSComputePass", false, "WDSSS compute shader has invalid numthreads:\n%s",
                    outcome.GetError().c_str());
            }
        }

        WDSSSFeatureProcessor* WDSSSComputePass::GetFeatureProcessor() const
        {
            RPI::Scene* scene = m_pipeline ? m_pipeline->GetScene() : nullptr;
            return scene ? scene->GetFeatureProcessor<WDSSSFeatureProcessor>() : nullptr;
        }

        const RHI::ImageView* WDSSSComputePass::GetInputImageView(
            const RHI::FrameGraphCompileContext& context, const AZ::Name& slotName) const
        {
            const RPI::PassAttachmentBinding* binding = FindAttachmentBinding(slotName);
            if (!binding || !binding->GetAttachment())
            {
                return nullptr;
            }
            return context.GetImageView(binding->GetAttachment()->GetAttachmentId());
        }

        bool WDSSSComputePass::IsEnabled() const
        {
            if (!RenderPass::IsEnabled())
            {
                return false;
            }
            if (!m_srgLayout || !m_pipelineState)
            {
                return false;
            }
            WDSSSFeatureProcessor* fp = GetFeatureProcessor();
            return fp && fp->ShouldRender();
        }

        void WDSSSComputePass::SetupFrameGraphDependencies(RHI::FrameGraphInterface frameGraph)
        {
            // Declares the connected slot attachments (Depth input, ShadowMask output).
            RenderPass::SetupFrameGraphDependencies(frameGraph);
            // Bend emits up to 8 dispatches per frame (DispatchList::Dispatch[8]); reserve the max number
            // of submit slots. The actual count (<= 8) is built in CompileResources and must not exceed this.
            frameGraph.SetEstimatedItemCount(8);
        }

        void WDSSSComputePass::CompileResources(const RHI::FrameGraphCompileContext& context)
        {
            WDSSSFeatureProcessor* fp = GetFeatureProcessor();
            if (!fp || !m_srgLayout)
            {
                return;
            }

            const RHI::ImageView* depthView = GetInputImageView(context, m_depthSlotName);
            const RHI::ImageView* maskView = GetInputImageView(context, m_maskSlotName);

            // Render-target size comes from the mask attachment (screen sized).
            const RPI::PassAttachmentBinding* maskBinding = FindAttachmentBinding(m_maskSlotName);
            RHI::Size targetSize;
            if (maskBinding && maskBinding->GetAttachment())
            {
                targetSize = maskBinding->GetAttachment()->m_descriptor.m_image.m_size;
            }
            if (targetSize.m_width == 0 || targetSize.m_height == 0)
            {
                m_dispatches.clear();
                return;
            }

            // Build the Bend dispatch list for this frame's sun + projection + screen size.
            m_dispatches = WDSSSDispatch::BuildDispatchList(
                fp->GetSunDirectionView(), fp->GetViewToClip(), targetSize.m_width, targetSize.m_height, fp->GetConfiguration());

            // Persistent pool of SRGs (one per dispatch, max 8), created lazily and REUSED across frames.
            // Creating/releasing SRGs every frame tears down the SRG pool mid-frame (an invalid operation
            // while the frame is processing), so we only ever grow the pool, then rebind + recompile the
            // SRGs we use this frame (they differ only in wave offset + light coordinate).
            while (m_dispatchSrgs.size() < m_dispatches.size())
            {
                Data::Instance<RPI::ShaderResourceGroup> srg = RPI::ShaderResourceGroup::Create(
                    m_shader->GetAsset(), m_shader->GetSupervariantIndex(), m_srgLayout->GetName());
                if (!srg)
                {
                    break;
                }
                m_dispatchSrgs.push_back(srg);
            }
            for (size_t i = 0; i < m_dispatches.size() && i < m_dispatchSrgs.size(); ++i)
            {
                BindDispatchSrg(m_dispatchSrgs[i].get(), depthView, maskView, m_dispatches[i].m_constants);
                m_dispatchSrgs[i]->Compile();
            }
        }

        void WDSSSComputePass::BindDispatchSrg(RPI::ShaderResourceGroup* srg, const RHI::ImageView* depth,
            const RHI::ImageView* mask, const PerDispatchConstants& c) const
        {
            const RHI::ShaderResourceGroupLayout* layout = srg->GetLayout();

            auto setImage = [&](const char* name, const RHI::ImageView* view)
            {
                if (!view)
                {
                    return;
                }
                const RHI::ShaderInputImageIndex index = layout->FindShaderInputImageIndex(AZ::Name(name));
                if (index.IsValid())
                {
                    srg->SetImageView(index, view);
                }
            };
            setImage("m_depth", depth);
            setImage("m_outputTexture", mask);

            auto setConst = [&](const char* name, const void* bytes, uint32_t byteCount)
            {
                const RHI::ShaderInputConstantIndex index = layout->FindShaderInputConstantIndex(AZ::Name(name));
                if (index.IsValid())
                {
                    srg->SetConstantRaw(index, bytes, byteCount);
                }
            };
            setConst("m_lightCoordinate", c.m_lightCoordinate, sizeof(c.m_lightCoordinate));
            setConst("m_waveOffset", c.m_waveOffset, sizeof(c.m_waveOffset));
            setConst("m_invDepthTextureSize", c.m_invDepthTextureSize, sizeof(c.m_invDepthTextureSize));
            setConst("m_surfaceThickness", &c.m_surfaceThickness, sizeof(c.m_surfaceThickness));
            setConst("m_bilinearThreshold", &c.m_bilinearThreshold, sizeof(c.m_bilinearThreshold));
            setConst("m_shadowContrast", &c.m_shadowContrast, sizeof(c.m_shadowContrast));
            setConst("m_depthBounds", c.m_depthBounds, sizeof(c.m_depthBounds));
            setConst("m_farDepthValue", &c.m_farDepthValue, sizeof(c.m_farDepthValue));
            setConst("m_nearDepthValue", &c.m_nearDepthValue, sizeof(c.m_nearDepthValue));
            setConst("m_flags", &c.m_flags, sizeof(c.m_flags));
        }

        void WDSSSComputePass::BuildCommandListInternal(const RHI::FrameGraphExecuteContext& context)
        {
            if (m_dispatches.empty() || m_dispatchSrgs.empty() || !m_pipelineState)
            {
                return;
            }

            const uint32_t deviceIndex = context.GetDeviceIndex();
            RHI::CommandList* commandList = context.GetCommandList();

            // m_dispatchSrgs is a persistent pool that may be larger than this frame's dispatch count.
            const size_t count = (m_dispatches.size() < m_dispatchSrgs.size()) ? m_dispatches.size() : m_dispatchSrgs.size();
            for (size_t i = 0; i < count; ++i)
            {
                const DispatchEntry& entry = m_dispatches[i];

                RHI::DispatchDirect args = m_dispatchArgs;
                args.m_totalNumberOfThreadsX = entry.m_groupCount[0] * args.m_threadsPerGroupX;
                args.m_totalNumberOfThreadsY = entry.m_groupCount[1] * args.m_threadsPerGroupY;
                args.m_totalNumberOfThreadsZ = entry.m_groupCount[2] * args.m_threadsPerGroupZ;

                RHI::DispatchItem dispatchItem{ RHI::MultiDevice::AllDevices };
                dispatchItem.SetPipelineState(m_pipelineState);
                dispatchItem.SetArguments(args);

                commandList->SetShaderResourceGroupForDispatch(
                    *m_dispatchSrgs[i]->GetRHIShaderResourceGroup()->GetDeviceShaderResourceGroup(deviceIndex));
                commandList->Submit(dispatchItem.GetDeviceDispatchItem(deviceIndex), static_cast<uint32_t>(i));
            }
        }
    } // namespace Render
} // namespace AZ
