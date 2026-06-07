#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

set(FILES
    Include/WDScreenSpaceShadows/WDSSSConfiguration.h
    Include/WDScreenSpaceShadows/WDSSSFeatureProcessorInterface.h
    Source/WDScreenSpaceShadowsModule.h
    Source/WDSSSConfiguration.cpp
    Source/Clients/WDSSSSystemComponent.cpp
    Source/Clients/WDSSSSystemComponent.h
    Source/Components/WDSSSComponent.cpp
    Source/Components/WDSSSComponent.h
    Source/Components/WDSSSComponentConfig.cpp
    Source/Components/WDSSSComponentConfig.h
    Source/Components/WDSSSComponentController.cpp
    Source/Components/WDSSSComponentController.h
    Source/Components/WDSSSComponentConstants.h
    Source/Render/WDSSSDispatch.cpp
    Source/Render/WDSSSDispatch.h
    Source/Render/WDSSSFeatureProcessor.cpp
    Source/Render/WDSSSFeatureProcessor.h
    Source/Render/Passes/WDSSSComputePass.cpp
    Source/Render/Passes/WDSSSComputePass.h
    Source/Render/Passes/WDSSSDebugPass.cpp
    Source/Render/Passes/WDSSSDebugPass.h
    Source/Render/Passes/WDSSSApplyPass.cpp
    Source/Render/Passes/WDSSSApplyPass.h
)
