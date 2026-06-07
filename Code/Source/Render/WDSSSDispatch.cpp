/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Render/WDSSSDispatch.h>
#include <AzCore/Math/Vector4.h>

// The vendored Bend CPU header is included ONLY here (Apache-2.0, see External/BendSSS/LICENSE).
// It is kept verbatim, so suppress the engine's strict warnings-as-errors for this third-party header
// (e.g. C4244 int->float on the sign() ternary) rather than editing the vendored source.
#if defined(_MSC_VER)
#   pragma warning(push, 0)
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Weverything"
#endif
#include <bend_sss_cpu.h>
#if defined(_MSC_VER)
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#endif

namespace AZ
{
    namespace Render
    {
        AZStd::fixed_vector<DispatchEntry, 8> WDSSSDispatch::BuildDispatchList(
            const AZ::Vector3& sunDirView, const AZ::Matrix4x4& viewToClip,
            AZ::u32 width, AZ::u32 height, const WDSSSConfiguration& cfg)
        {
            AZStd::fixed_vector<DispatchEntry, 8> out;
            if (width == 0 || height == 0)
            {
                return out;
            }

            // Bend wants the light's homogeneous clip coordinate ({light} * ViewProjection, no W divide).
            // For an infinite directional light, {light} = float4(normalized direction, 0).
            const AZ::Vector3 dir = sunDirView.GetNormalizedSafe();
            const AZ::Vector4 clip = viewToClip * AZ::Vector4(dir.GetX(), dir.GetY(), dir.GetZ(), 0.0f);

            float lightProjection[4] = { clip.GetX(), clip.GetY(), clip.GetZ(), clip.GetW() };
            int viewportSize[2] = { static_cast<int>(width), static_cast<int>(height) };
            int minBounds[2] = { 0, 0 };
            int maxBounds[2] = { static_cast<int>(width), static_cast<int>(height) };

            // Atom / D3D12 use a [0,1] clip-Z range, so the expanded-range flag is false.
            const Bend::DispatchList list = Bend::BuildDispatchList(
                lightProjection, viewportSize, minBounds, maxBounds, /*inExpandedZRange=*/false, /*inWaveSize=*/64);

            const float invW = 1.0f / static_cast<float>(width);
            const float invH = 1.0f / static_cast<float>(height);
            const AZ::s32 flags =
                (cfg.m_ignoreEdgePixels ? 1 : 0) |
                (cfg.m_usePrecisionOffset ? 2 : 0) |
                (cfg.m_bilinearSamplingOffsetMode ? 4 : 0);

            const int count = (list.DispatchCount < 8) ? list.DispatchCount : 8;
            for (int d = 0; d < count; ++d)
            {
                const Bend::DispatchData& src = list.Dispatch[d];
                DispatchEntry e;
                e.m_groupCount[0] = static_cast<AZ::u32>(src.WaveCount[0]);
                e.m_groupCount[1] = static_cast<AZ::u32>(src.WaveCount[1]);
                e.m_groupCount[2] = static_cast<AZ::u32>(src.WaveCount[2]);

                PerDispatchConstants& c = e.m_constants;
                c.m_lightCoordinate[0] = list.LightCoordinate_Shader[0];
                c.m_lightCoordinate[1] = list.LightCoordinate_Shader[1];
                c.m_lightCoordinate[2] = list.LightCoordinate_Shader[2];
                c.m_lightCoordinate[3] = list.LightCoordinate_Shader[3];
                c.m_waveOffset[0] = src.WaveOffset_Shader[0];
                c.m_waveOffset[1] = src.WaveOffset_Shader[1];
                c.m_invDepthTextureSize[0] = invW;
                c.m_invDepthTextureSize[1] = invH;
                c.m_surfaceThickness = cfg.m_surfaceThickness;
                c.m_bilinearThreshold = cfg.m_bilinearThreshold;
                c.m_shadowContrast = cfg.m_shadowContrast;
                c.m_depthBounds[0] = 0.0f;
                c.m_depthBounds[1] = 1.0f;
                c.m_farDepthValue = 0.0f;
                c.m_nearDepthValue = 1.0f;
                // Bits 0-2 = Bend edge/precision/bilinear flags; bits 8-10 = the gem's debug-view mode
                // (so the compute shader can output diagnostics instead of the shadow).
                c.m_flags = flags | ((cfg.m_debugView & 0x7) << 8);
                out.push_back(e);
            }
            return out;
        }
    } // namespace Render
} // namespace AZ
