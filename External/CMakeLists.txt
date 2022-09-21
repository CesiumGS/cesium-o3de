cmake_minimum_required(VERSION 3.20)

project(
    cesium-unreal-extern
    VERSION 0.1.0
    LANGUAGES CXX
)

# build and install cesium native
set(PRIVATE_CESIUM_SQLITE OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/Packages/CesiumNative/CesiumNative)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SYSTEM_NAME})

set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Force position independent code generation using -fPIC compiler flag" FORCE)

add_subdirectory(CesiumNative)
set_target_properties(cesium-native-tests PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

if (BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIB ON CACHE BOOL "Override build shared for TidyHtml5" FORCE)
else()
    set(BUILD_SHARED_LIB OFF CACHE BOOL "Override build shared for TidyHtml5" FORCE)
endif()
set(SUPPORT_CONSOLE_APP OFF CACHE BOOL "Override build console app for TidyHtml5" FORCE)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
add_subdirectory(TidyHtml5)

# trigger post install script. E.g:
# - copy License file to Package/CesiumNative
# - generate SHA256 for every files in the Package/CesiumNative 
install (SCRIPT "${CMAKE_CURRENT_LIST_DIR}/PostInstall.cmake")
