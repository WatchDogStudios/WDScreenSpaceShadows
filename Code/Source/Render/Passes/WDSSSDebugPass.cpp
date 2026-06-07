/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Render/Passes/WDSSSDebugPass.h>
#include <Render/WDSSSFeatureProcessor.h>

#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>

namespace AZ
{
    namespace Render
    {
        RPI::Ptr<WDSSSDebugPass> WDSSSDebugPass::Create(const RPI::PassDescriptor& descriptor)
        {
            return aznew WDSSSDebugPass(descriptor);
        }

        WDSSSDebugPass::WDSSSDebugPass(const RPI::PassDescriptor& descriptor)
            : RPI::FullscreenTrianglePass(descriptor)
        {
        }

        WDSSSFeatureProcessor* WDSSSDebugPass::GetFeatureProcessor() const
        {
            RPI::Scene* scene = m_pipeline ? m_pipeline->GetScene() : nullptr;
            return scene ? scene->GetFeatureProcessor<WDSSSFeatureProcessor>() : nullptr;
        }

        bool WDSSSDebugPass::IsEnabled() const
        {
            if (!FullscreenTrianglePass::IsEnabled())
            {
                return false;
            }
            WDSSSFeatureProcessor* fp = GetFeatureProcessor();
            return fp && fp->GetConfiguration().m_enabled && fp->GetDebugView() != 0;
        }
    } // namespace Render
} // namespace AZ
