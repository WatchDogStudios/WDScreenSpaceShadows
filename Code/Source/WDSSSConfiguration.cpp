/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <WDScreenSpaceShadows/WDSSSConfiguration.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace AZ
{
    namespace Render
    {
        void WDSSSConfiguration::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<WDSSSConfiguration>()
                    ->Version(1)
                    ->Field("Enabled", &WDSSSConfiguration::m_enabled)
                    ->Field("ApplyToSun", &WDSSSConfiguration::m_applyToSun)
                    ->Field("DebugView", &WDSSSConfiguration::m_debugView)
                    ->Field("SunDirectionWorld", &WDSSSConfiguration::m_sunDirectionWorld)
                    ->Field("SunEntityId", &WDSSSConfiguration::m_sunEntityId)
                    ->Field("SurfaceThickness", &WDSSSConfiguration::m_surfaceThickness)
                    ->Field("BilinearThreshold", &WDSSSConfiguration::m_bilinearThreshold)
                    ->Field("ShadowContrast", &WDSSSConfiguration::m_shadowContrast)
                    ->Field("IgnoreEdgePixels", &WDSSSConfiguration::m_ignoreEdgePixels)
                    ->Field("UsePrecisionOffset", &WDSSSConfiguration::m_usePrecisionOffset)
                    ->Field("BilinearSamplingOffsetMode", &WDSSSConfiguration::m_bilinearSamplingOffsetMode);

                if (auto* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<WDSSSConfiguration>("WD Screen Space Shadows", "Bend contact-shadow settings")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &WDSSSConfiguration::m_enabled, "Enabled",
                            "Produce the screen-space shadow mask. Opt-in; also drivable via r.wdSSSEnabled.")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &WDSSSConfiguration::m_applyToSun, "Apply to Sun",
                            "Darken the directional sun with the mask (min into the FullscreenShadowBuffer). Also r.wdSSSApply.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &WDSSSConfiguration::m_sunEntityId, "Sun Entity",
                            "Point this at the scene's directional-light entity to inherit its direction automatically (recommended - "
                            "tracks the exact sun the lighting uses). Leave unset to use the manual Sun Direction below.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &WDSSSConfiguration::m_sunDirectionWorld, "Sun Direction",
                            "Manual fallback sun travel direction (toward surfaces), used only when Sun Entity is unset. "
                            "Y should dominate for an overhead sun, e.g. (0.2, -1.0, 0.3); a near-horizontal sun grazes flat floors.")
                        ->ClassElement(AZ::Edit::ClassElements::Group, "Tuning")
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &WDSSSConfiguration::m_surfaceThickness, "Surface Thickness",
                            "Assumed pixel thickness for shadow casting (fraction of non-linear depth). Bend default 0.005.")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 0.05f)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &WDSSSConfiguration::m_bilinearThreshold, "Bilinear Threshold",
                            "Depth-discontinuity threshold for edge detection. Bend default 0.02.")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 0.2f)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &WDSSSConfiguration::m_shadowContrast, "Shadow Contrast",
                            "Contrast boost on the in/out-of-shadow transition. Bend default 4 (>= 1).")
                            ->Attribute(AZ::Edit::Attributes::Min, 1.0f)
                            ->Attribute(AZ::Edit::Attributes::Max, 8.0f)
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &WDSSSConfiguration::m_ignoreEdgePixels, "Ignore Edge Pixels",
                            "Detected edge pixels do not cast a shadow (helps grazing-angle flat surfaces; can thin foliage shadows).")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &WDSSSConfiguration::m_usePrecisionOffset, "Use Precision Offset",
                            "Small offset to compensate for an imprecise depth buffer (usually off).")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &WDSSSConfiguration::m_bilinearSamplingOffsetMode, "Bilinear Sampling Offset Mode",
                            "Alternative bilinear sampling mode; subtle visual difference. Usually off.");
                }
            }
        }
    } // namespace Render
} // namespace AZ
