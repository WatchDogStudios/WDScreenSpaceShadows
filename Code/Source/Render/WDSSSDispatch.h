/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/base.h>
#include <AzCore/std/containers/fixed_vector.h>
#include <AzCore/Math/Matrix4x4.h>
#include <AzCore/Math/Vector3.h>
#include <WDScreenSpaceShadows/WDSSSConfiguration.h>

namespace AZ
{
    namespace Render
    {
        //! Per-dispatch shader constants. These mirror the scalar inputs of Bend::DispatchParameters
        //! (see External/BendSSS/bend_sss_gpu.h). The compute pass uploads each field by name onto its
        //! pass SRG (FindShaderInputConstantIndex + SetConstantRaw), so this C++ layout need not
        //! byte-match the SRG cbuffer; the member names below are what the AZSL SRG must expose.
        struct PerDispatchConstants
        {
            float   m_lightCoordinate[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; //!< DispatchList::LightCoordinate_Shader
            AZ::s32 m_waveOffset[2] = { 0, 0 };                        //!< DispatchData::WaveOffset_Shader
            float   m_invDepthTextureSize[2] = { 0.0f, 0.0f };
            float   m_surfaceThickness = 0.005f;
            float   m_bilinearThreshold = 0.02f;
            float   m_shadowContrast = 4.0f;
            float   m_depthBounds[2] = { 0.0f, 1.0f };
            float   m_farDepthValue = 0.0f;  //!< Atom reversed-Z: far-plane depth value = 0
            float   m_nearDepthValue = 1.0f; //!< near-plane depth value = 1
            AZ::s32 m_flags = 0;             //!< bit0 IgnoreEdgePixels, bit1 UsePrecisionOffset, bit2 BilinearSamplingOffsetMode
        };

        //! One compute dispatch produced by Bend::BuildDispatchList.
        struct DispatchEntry
        {
            AZ::u32 m_groupCount[3] = { 0, 0, 0 }; //!< Bend WaveCount[0..2] = Dispatch(X,Y,Z) group counts
            PerDispatchConstants m_constants;
        };

        //! Thin wrapper around External/BendSSS/bend_sss_cpu.h. The vendored header is included only by
        //! WDSSSDispatch.cpp, so the rest of the gem never depends on it directly.
        class WDSSSDispatch
        {
        public:
            //! Build the list of compute dispatches needed to shadow the screen for one directional light.
            //! @param sunDirView  sun direction in view space (the light travels along this vector).
            //! @param viewToClip  the camera's view->clip (projection) matrix.
            //! @param width,height  render-target dimensions in pixels.
            //! @param cfg  tuning (surface thickness, contrast, edge flags).
            //! @return up to 8 dispatches; empty if width/height is zero.
            static AZStd::fixed_vector<DispatchEntry, 8> BuildDispatchList(
                const AZ::Vector3& sunDirView, const AZ::Matrix4x4& viewToClip,
                AZ::u32 width, AZ::u32 height, const WDSSSConfiguration& cfg);
        };
    } // namespace Render
} // namespace AZ
