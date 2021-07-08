cmake_minimum_required(VERSION 3.20)

project(
    cesium-unreal-extern
    VERSION 0.1.0
    LANGUAGES CXX C
)

# build and install cesium native
set(PRIVATE_CESIUM_SQLITE OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/Packages/CesiumNative/CesiumNative)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SYSTEM_NAME})

add_subdirectory(CesiumNative)
set_target_properties(cesium-native-tests PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
