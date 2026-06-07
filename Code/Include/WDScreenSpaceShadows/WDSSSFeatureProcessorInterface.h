/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <WDScreenSpaceShadows/WDSSSConfiguration.h>

namespace AZ
{
    namespace Render
    {
        //! Public interface to the screen-space-shadows feature processor. The system component's console
        //! variables and ImGui panel drive the effect through this interface; the gem's passes resolve the
        //! concrete feature processor directly for per-frame data (sun direction, projection).
        class WDSSSFeatureProcessorInterface
            : public RPI::FeatureProcessor
        {
        public:
            AZ_RTTI(AZ::Render::WDSSSFeatureProcessorInterface, "{F4A6C8E0-2B5D-4E7F-8A9B-0C1D2E3F4A50}", RPI::FeatureProcessor);

            virtual void SetConfiguration(const WDSSSConfiguration& config) = 0;
            virtual const WDSSSConfiguration& GetConfiguration() const = 0;

            virtual void SetEnabled(bool enabled) = 0;
            virtual bool GetEnabled() const = 0;

            virtual void SetApplyToSun(bool apply) = 0;

            virtual void SetDebugView(int mode) = 0;
            virtual int GetDebugView() const = 0;
        };
    } // namespace Render
} // namespace AZ
