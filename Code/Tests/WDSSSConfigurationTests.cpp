/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzTest/AzTest.h>
#include <WDScreenSpaceShadows/WDSSSConfiguration.h>

using AZ::Render::WDSSSConfiguration;

TEST(WDSSSConfiguration, Defaults_AreOptInOff_AndSaneRanges)
{
    WDSSSConfiguration c;

    // Opt-in: nothing renders until explicitly enabled (house style).
    EXPECT_FALSE(c.m_enabled);
    EXPECT_FALSE(c.m_applyToSun);
    EXPECT_EQ(c.m_debugView, 0);

    // Bend's recommended starting values must be in valid ranges.
    EXPECT_GT(c.m_surfaceThickness, 0.0f);
    EXPECT_GE(c.m_bilinearThreshold, 0.0f);
    EXPECT_GE(c.m_shadowContrast, 1.0f);
}
