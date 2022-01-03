
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: ly_get_list_relative_pal_filename will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/Cesium/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/Cesium/Code
ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} ${o3de_gem_restricted_path} ${o3de_gem_path} ${o3de_gem_name})

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# Add CesiumNative as a third party
message(WARNING "YOU ARE USING DEVELOPMENT BUILD OF CESIUM GEM. BEFORE RELEASING THE GEM, YOU NEED TO: 
- REMOVE LY_PACKAGE_SERVER_URLS SETTING BELOW AND SET THE VALUE BY USING ENVIRONMENT VARIABLE INSTEAD 
- REPLACE THE CesiumNative_SHA256_PACKAGE WITH A REAL SHA256 OF CesiumNative PACKAGE")
set(LY_PACKAGE_SERVER_URLS "${LY_PACKAGE_SERVER_URLS};file:///${CMAKE_CURRENT_LIST_DIR}/../External/Packages/Install" FORCE)
file(READ ${CMAKE_CURRENT_LIST_DIR}/../External/Packages/Install/SHA256SUMS CesiumNative_SHA256_PACKAGE)
ly_associate_package(PACKAGE_NAME CesiumNative TARGETS CesiumNative PACKAGE_HASH ${CesiumNative_SHA256_PACKAGE})

# Add the Cesium.Static target
# Note: We include the common files and the platform specific files which are set in cesium_common_files.cmake
# and in ${pal_dir}/cesium_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
ly_add_target(
    NAME Cesium.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        cesium_files.cmake
        ${pal_dir}/cesium_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            3rdParty::AWSNativeSDK::Core
            3rdParty::CesiumNative
            3rdParty::mikkelsen
            3rdParty::zlib
            AZ::AzCore
            AZ::AzFramework
            AZ::AWSNativeSDKInit
            Gem::Atom_RPI.Public
            Gem::Atom_Feature_Common.Static
    COMPILE_DEFINITIONS
        PUBLIC
            SPDLOG_COMPILED_LIB
            LIBASYNC_STATIC
)

# Here add Cesium target, it depends on the Cesium.Static
ly_add_target(
    NAME Cesium ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        cesium_shared_files.cmake
        ${pal_dir}/cesium_shared_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Cesium.Static
)

# By default, we will specify that the above target Cesium would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME Cesium.Clients NAMESPACE Gem TARGETS Gem::Cesium)
ly_create_alias(NAME Cesium.Servers NAMESPACE Gem TARGETS Gem::Cesium)

# If we are on a host platform, we want to add the host tools targets like the Cesium.Editor target which
# will also depend on Cesium.Static
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME Cesium.Editor.Static STATIC
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            cesium_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::Cesium.Static
    )

    ly_add_target(
        NAME Cesium.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        OUTPUT_NAME Gem.Cesium.Editor
        FILES_CMAKE
            cesium_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::Cesium.Editor.Static
    )

    # By default, we will specify that the above target Cesium would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME Cesium.Tools    NAMESPACE Gem TARGETS Gem::Cesium.Editor)
    ly_create_alias(NAME Cesium.Builders NAMESPACE Gem TARGETS Gem::Cesium.Editor)


endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    # We globally support tests, see if we support tests on this platform for Cesium.Static
    if(PAL_TRAIT_CESIUM_TEST_SUPPORTED)
        # We support Cesium.Tests on this platform, add Cesium.Tests target which depends on Cesium.Static
        ly_add_target(
            NAME Cesium.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE
                cesium_files.cmake
                cesium_tests_files.cmake
            INCLUDE_DIRECTORIES
                PRIVATE
                    Tests
                    Source
            BUILD_DEPENDENCIES
                PRIVATE
                    AZ::AzTest
                    AZ::AzFramework
                    Gem::Cesium.Static
        )

        # Add Cesium.Tests to googletest
        ly_add_googletest(
            NAME Gem::Cesium.Tests
        )
    endif()

    # If we are a host platform we want to add tools test like editor tests here
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        # We are a host platform, see if Editor tests are supported on this platform
        if(PAL_TRAIT_CESIUM_EDITOR_TEST_SUPPORTED)
            # We support Cesium.Editor.Tests on this platform, add Cesium.Editor.Tests target which depends on Cesium.Editor
            ly_add_target(
                NAME Cesium.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                FILES_CMAKE
                    cesium_editor_tests_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::Cesium.Editor
            )

            # Add Cesium.Editor.Tests to googletest
            ly_add_googletest(
                NAME Gem::Cesium.Editor.Tests
            )
        endif()
    endif()
endif()
