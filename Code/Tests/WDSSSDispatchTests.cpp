/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzTest/AzTest.h>
#include <Render/WDSSSDispatch.h>
#include <AzCore/Math/Matrix4x4.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>

using namespace AZ::Render;

namespace
{
    // A simple right-handed perspective-style projection so a directional light projects on-screen
    // (clip.w picks up the view-space z), exercising Bend's multi-dispatch path.
    AZ::Matrix4x4 MakeTestProjection()
    {
        const float aspect = 1920.0f / 1080.0f;
        AZ::Matrix4x4 m = AZ::Matrix4x4::CreateIdentity();
        m.SetRow(0, AZ::Vector4(1.0f / aspect, 0.0f, 0.0f, 0.0f));
        m.SetRow(1, AZ::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
        m.SetRow(2, AZ::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        m.SetRow(3, AZ::Vector4(0.0f, 0.0f, -1.0f, 0.0f)); // clip.w = -view.z
        return m;
    }
}

TEST(WDSSSDispatch, OnScreenSun_ProducesNonEmptyBoundedDeterministicList)
{
    WDSSSConfiguration cfg;
    const AZ::Matrix4x4 proj = MakeTestProjection();
    const AZ::Vector3 sunDirView(0.2f, -0.3f, -1.0f); // into the screen, slightly off-centre

    auto list = WDSSSDispatch::BuildDispatchList(sunDirView, proj, 1920, 1080, cfg);

    EXPECT_GT(list.size(), 0u);  // an on-screen light yields several dispatches
    EXPECT_LE(list.size(), 8u);  // Bend caps the list at 8

    for (const auto& e : list)
    {
        EXPECT_GT(e.m_groupCount[0] * e.m_groupCount[1] * e.m_groupCount[2], 0u);
        EXPECT_FLOAT_EQ(e.m_constants.m_invDepthTextureSize[0], 1.0f / 1920.0f);
        EXPECT_FLOAT_EQ(e.m_constants.m_invDepthTextureSize[1], 1.0f / 1080.0f);
        EXPECT_FLOAT_EQ(e.m_constants.m_surfaceThickness, cfg.m_surfaceThickness);
    }

    // Pure function of its inputs -> identical list on a second call.
    auto list2 = WDSSSDispatch::BuildDispatchList(sunDirView, proj, 1920, 1080, cfg);
    ASSERT_EQ(list.size(), list2.size());
    for (size_t i = 0; i < list.size(); ++i)
    {
        EXPECT_EQ(list[i].m_groupCount[0], list2[i].m_groupCount[0]);
        EXPECT_EQ(list[i].m_groupCount[1], list2[i].m_groupCount[1]);
        EXPECT_EQ(list[i].m_groupCount[2], list2[i].m_groupCount[2]);
        EXPECT_EQ(list[i].m_constants.m_waveOffset[0], list2[i].m_constants.m_waveOffset[0]);
        EXPECT_EQ(list[i].m_constants.m_waveOffset[1], list2[i].m_constants.m_waveOffset[1]);
    }

    // Flags pack through from the configuration.
    cfg.m_ignoreEdgePixels = true;
    cfg.m_bilinearSamplingOffsetMode = true;
    auto list3 = WDSSSDispatch::BuildDispatchList(sunDirView, proj, 1920, 1080, cfg);
    ASSERT_GT(list3.size(), 0u);
    EXPECT_EQ(list3[0].m_constants.m_flags & 1, 1); // IgnoreEdgePixels
    EXPECT_EQ(list3[0].m_constants.m_flags & 4, 4); // BilinearSamplingOffsetMode
}

TEST(WDSSSDispatch, ZeroSize_ReturnsEmpty)
{
    WDSSSConfiguration cfg;
    auto list = WDSSSDispatch::BuildDispatchList(
        AZ::Vector3(0.0f, 0.0f, -1.0f), AZ::Matrix4x4::CreateIdentity(), 0, 0, cfg);
    EXPECT_EQ(list.size(), 0u);
}
