/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <Atom/RPI.Public/Pass/FullscreenTrianglePass.h>

namespace AZ
{
    namespace Render
    {
        class WDSSSFeatureProcessor;

        //! Fullscreen debug visualization of the screen-space shadow mask. Only runs when the feature
        //! processor's debug view is non-zero; otherwise it disables itself so it costs nothing and does
        //! not overwrite the colour target.
        class WDSSSDebugPass
            : public RPI::FullscreenTrianglePass
        {
        public:
            AZ_RTTI(AZ::Render::WDSSSDebugPass, "{3A4B5C6D-7E8F-4A0B-9C1D-2E3F4A5B6C7E}", RPI::FullscreenTrianglePass);
            AZ_CLASS_ALLOCATOR(WDSSSDebugPass, SystemAllocator);

            static RPI::Ptr<WDSSSDebugPass> Create(const RPI::PassDescriptor& descriptor);

        protected:
            explicit WDSSSDebugPass(const RPI::PassDescriptor& descriptor);

            WDSSSFeatureProcessor* GetFeatureProcessor() const;
            bool IsEnabled() const override;
        };
    } // namespace Render
} // namespace AZ
