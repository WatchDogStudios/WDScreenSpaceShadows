#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

# Get the gem name from the gem.json
o3de_gem_setup("WDScreenSpaceShadows")

# Get platform-specific folder
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
    "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Include platform traits
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# Static library with the gem's runtime code (feature processor + passes). The vendored Bend
# headers live under External/BendSSS and are reached via the External include directory.
ly_add_target(
    NAME ${gem_name}.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        wdsss_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            ${CMAKE_CURRENT_LIST_DIR}/../External/BendSSS
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            Gem::Atom_RPI.Public
            Gem::Atom_Feature_Common.Public
            Gem::Atom_RHI.Public
            # PUBLIC so the IMGUI_ENABLED compile definition (from .imguilib), imgui.h, and ImGuiBus.h
            # (from .Static) propagate consistently to both this static library and the gem module - the
            # SystemComponent header's base-class list is guarded on IMGUI_ENABLED and must be identical
            # in every translation unit.
            Gem::ImGui.imguilib
            Gem::ImGui.Static
)

# Runtime gem module
ly_add_target(
    NAME ${gem_name} ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        wdsss_shared_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::${gem_name}.Static
)

# Inject the gem as a runtime dependency for the standard role aliases
ly_create_alias(NAME ${gem_name}.Clients NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Servers NAMESPACE Gem TARGETS Gem::${gem_name})
ly_create_alias(NAME ${gem_name}.Unified NAMESPACE Gem TARGETS Gem::${gem_name})

# Editor / tools build: a dedicated module that also registers the editor (Level) component. It reuses
# the same .Static runtime code (feature processor + passes + system component) so the effect runs in
# the editor too, and adds the EditorWDSSSComponent guarded by the WDSSS_EDITOR define.
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME ${gem_name}.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            wdsss_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        COMPILE_DEFINITIONS
            PRIVATE
                WDSSS_EDITOR
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::${gem_name}.Static
    )

    ly_create_alias(NAME ${gem_name}.Builders NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
    ly_create_alias(NAME ${gem_name}.Tools    NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
endif()

################################################################################
# Tests
################################################################################
if(PAL_TRAIT_BUILD_HOST_TOOLS AND PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    ly_add_target(
        NAME ${gem_name}.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
        NAMESPACE Gem
        FILES_CMAKE
            wdsss_tests_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Include
                Source
                Tests
                ${CMAKE_CURRENT_LIST_DIR}/../External/BendSSS
        BUILD_DEPENDENCIES
            PRIVATE
                AZ::AzTest
                AZ::AzFramework
                Gem::${gem_name}.Static
    )

    # Register the tests with CTest / the AzTestRunner.
    ly_add_googletest(NAME Gem::${gem_name}.Tests)
endif()
