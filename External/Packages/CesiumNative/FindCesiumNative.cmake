#
# Copyright (c) Contributors to the Cesium for O3DE Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0
#
# 2022-09 - Modifications for Linux Platform support - Huawei Technologies Co., Ltd <foss@huawei.com>

set(LIB_NAME "CesiumNative")

set(TARGET_WITH_NAMESPACE "3rdParty::${LIB_NAME}")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(${LIB_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}/include)
set(${LIB_NAME}_LIBS_DIR ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}/lib/${CMAKE_SYSTEM_NAME})

file(GLOB ${LIB_NAME}_LIBRARY ${${LIB_NAME}_LIBS_DIR}/*${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(${TARGET_WITH_NAMESPACE} INTERFACE IMPORTED GLOBAL)

ly_target_include_system_directories(
    TARGET ${TARGET_WITH_NAMESPACE} INTERFACE ${${LIB_NAME}_INCLUDE_DIR})

target_link_libraries(
    ${TARGET_WITH_NAMESPACE}
    INTERFACE ${${LIB_NAME}_LIBRARY})

set(${LIB_NAME}_FOUND True)