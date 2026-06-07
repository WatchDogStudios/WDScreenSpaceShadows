/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Clients/WDSSSSystemComponent.h>

#include <AzCore/Console/IConsole.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RPI.Public/Scene.h>

#include <WDScreenSpaceShadows/WDSSSConfiguration.h>
#include <WDScreenSpaceShadows/WDSSSFeatureProcessorInterface.h>
#include <Render/WDSSSFeatureProcessor.h>
#include <Render/Passes/WDSSSComputePass.h>
#include <Render/Passes/WDSSSDebugPass.h>
#include <Render/Passes/WDSSSApplyPass.h>

#if defined(IMGUI_ENABLED)
#include <imgui/imgui.h>
#endif

namespace AZ
{
    namespace Render
    {
        namespace
        {
            //! Resolve the WDSSS feature processor for the primary scene (null if absent).
            WDSSSFeatureProcessorInterface* GetWDSSSFeatureProcessor()
            {
                auto* rpiSystem = RPI::RPISystemInterface::Get();
                if (!rpiSystem)
                {
                    return nullptr;
                }
                if (RPI::Scene* scene = rpiSystem->GetSceneByName(AZ::Name("Main")))
                {
                    return scene->GetFeatureProcessor<WDSSSFeatureProcessorInterface>();
                }
                return nullptr;
            }

            template<typename Fn>
            void ForEachWDSSSFeatureProcessor(Fn&& fn)
            {
                if (auto* fp = GetWDSSSFeatureProcessor())
                {
                    fn(fp);
                }
            }
        } // namespace

        // --- Console variables -------------------------------------------------------------------

        static void OnWDSSSEnabledChanged(const bool& enabled)
        {
            ForEachWDSSSFeatureProcessor([enabled](WDSSSFeatureProcessorInterface* fp) { fp->SetEnabled(enabled); });
        }

        static void OnWDSSSApplyChanged(const bool& apply)
        {
            ForEachWDSSSFeatureProcessor([apply](WDSSSFeatureProcessorInterface* fp) { fp->SetApplyToSun(apply); });
        }

        static void OnWDSSSDebugViewChanged(const int32_t& mode)
        {
            ForEachWDSSSFeatureProcessor([mode](WDSSSFeatureProcessorInterface* fp) { fp->SetDebugView(mode); });
        }

        AZ_CVAR(bool, r_wdSSSEnabled, false, &OnWDSSSEnabledChanged, AZ::ConsoleFunctorFlags::Null,
            "Enable WD screen-space shadows (Bend contact shadows): produce the mask.");

        AZ_CVAR(bool, r_wdSSSApply, false, &OnWDSSSApplyChanged, AZ::ConsoleFunctorFlags::Null,
            "Apply the screen-space shadow mask to the sun (min into FullscreenShadowBuffer).");

        AZ_CVAR(int32_t, r_wdSSSDebugView, 0, &OnWDSSSDebugViewChanged, AZ::ConsoleFunctorFlags::Null,
            "WD screen-space shadows debug view: 0 = off, 1 = mask visualization.");

        // --- Component ---------------------------------------------------------------------------

        void WDSSSSystemComponent::Reflect(AZ::ReflectContext* context)
        {
            WDSSSConfiguration::Reflect(context);
            WDSSSFeatureProcessor::Reflect(context);

            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<WDSSSSystemComponent, AZ::Component>()
                    ->Version(1);

                if (auto* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<WDSSSSystemComponent>(
                        "WD Screen Space Shadows", "Bend wavefront screen-space contact shadows")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                            ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("System"))
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
                }
            }
        }

        void WDSSSSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("WDSSSService"));
        }

        void WDSSSSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("WDSSSService"));
        }

        void WDSSSSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("RPISystem"));
        }

        void WDSSSSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
        {
            dependent.push_back(AZ_CRC_CE("AtomFeatureCommonService"));
        }

        void WDSSSSystemComponent::Activate()
        {
            // Register the feature processor (exposed through its public interface).
            RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessorWithInterface<
                WDSSSFeatureProcessor, WDSSSFeatureProcessorInterface>();

            auto* passSystem = RPI::PassSystemInterface::Get();
            AZ_Assert(passSystem, "PassSystemInterface is null");

            // Register the WDSSS pass classes.
            passSystem->AddPassCreator(AZ::Name("WDSSSComputePass"), &WDSSSComputePass::Create);
            passSystem->AddPassCreator(AZ::Name("WDSSSDebugPass"), &WDSSSDebugPass::Create);
            passSystem->AddPassCreator(AZ::Name("WDSSSApplyPass"), &WDSSSApplyPass::Create);

            // Load the pass template mappings once the pass system is ready.
            m_loadTemplatesHandler = RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler(
                [this]() { this->LoadPassTemplateMappings(); });
            passSystem->ConnectEvent(m_loadTemplatesHandler);

#if defined(IMGUI_ENABLED)
            ImGui::ImGuiUpdateListenerBus::Handler::BusConnect();
#endif
        }

        void WDSSSSystemComponent::Deactivate()
        {
#if defined(IMGUI_ENABLED)
            ImGui::ImGuiUpdateListenerBus::Handler::BusDisconnect();
#endif
            m_loadTemplatesHandler.Disconnect();

            RPI::FeatureProcessorFactory::Get()->UnregisterFeatureProcessor<WDSSSFeatureProcessor>();
        }

#if defined(IMGUI_ENABLED)
        void WDSSSSystemComponent::OnImGuiMainMenuUpdate()
        {
            if (ImGui::BeginMenu("WD Screen Space Shadows"))
            {
                ImGui::MenuItem("SSS Panel", "", &m_showImGuiWindow);
                ImGui::EndMenu();
            }
        }

        void WDSSSSystemComponent::OnImGuiUpdate()
        {
            if (!m_showImGuiWindow)
            {
                return;
            }

            ImGui::SetNextWindowSize(ImVec2(380.0f, 360.0f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("WD Screen Space Shadows (Bend)", &m_showImGuiWindow))
            {
                WDSSSFeatureProcessorInterface* fp = GetWDSSSFeatureProcessor();
                if (!fp)
                {
                    ImGui::TextColored(ImVec4(1, 0.5f, 0.3f, 1), "No WDSSS feature processor in the active scene.");
                    ImGui::TextWrapped("Ensure the gem is enabled and the render pipeline has a 'Forward' pass.");
                    ImGui::End();
                    return;
                }

                WDSSSConfiguration config = fp->GetConfiguration();
                bool changed = false;

                ImGui::Separator();
                ImGui::TextUnformatted("Toggles");
                changed |= ImGui::Checkbox("Enabled (produce mask)", &config.m_enabled);
                changed |= ImGui::Checkbox("Apply to Sun (min into FullscreenShadow)", &config.m_applyToSun);

                int debugView = config.m_debugView;
                if (ImGui::Combo("Debug View", &debugView, "Off\0Mask\0\0"))
                {
                    config.m_debugView = debugView;
                    changed = true;
                }

                ImGui::Separator();
                ImGui::TextUnformatted("Sun (config-driven in v1)");
                float sunDir[3] = { config.m_sunDirectionWorld.GetX(), config.m_sunDirectionWorld.GetY(), config.m_sunDirectionWorld.GetZ() };
                if (ImGui::SliderFloat3("Sun Direction", sunDir, -1.0f, 1.0f))
                {
                    config.m_sunDirectionWorld.Set(sunDir[0], sunDir[1], sunDir[2]);
                    changed = true;
                }
                ImGui::TextWrapped("Set this to match the scene's sun travel direction (toward surfaces).");

                ImGui::Separator();
                ImGui::TextUnformatted("Tuning (Bend)");
                changed |= ImGui::SliderFloat("Surface Thickness", &config.m_surfaceThickness, 0.0f, 0.05f, "%.4f");
                changed |= ImGui::SliderFloat("Bilinear Threshold", &config.m_bilinearThreshold, 0.0f, 0.2f, "%.3f");
                changed |= ImGui::SliderFloat("Shadow Contrast", &config.m_shadowContrast, 1.0f, 8.0f);
                changed |= ImGui::Checkbox("Ignore Edge Pixels", &config.m_ignoreEdgePixels);
                changed |= ImGui::Checkbox("Use Precision Offset", &config.m_usePrecisionOffset);
                changed |= ImGui::Checkbox("Bilinear Sampling Offset Mode", &config.m_bilinearSamplingOffsetMode);

                if (changed)
                {
                    fp->SetConfiguration(config);
                }
            }
            ImGui::End();
        }
#endif

        void WDSSSSystemComponent::LoadPassTemplateMappings()
        {
            auto* passSystem = RPI::PassSystemInterface::Get();
            AZ_Assert(passSystem, "PassSystemInterface is null");

            const char* passTemplatesFile = "Passes/WDSSSTemplates.azasset";
            passSystem->LoadPassTemplateMappings(passTemplatesFile);
        }
    } // namespace Render
} // namespace AZ
