set(PACKAGE_BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/Packages)
set(PACKAGE_INSTALL_DIR ${PACKAGE_BUILD_DIR}/Install)

# copy license file to Packages/CesiumNative/CesiumNative
file(COPY 
      ${CMAKE_CURRENT_LIST_DIR}/CesiumNative/LICENSE 
      DESTINATION ${PACKAGE_BUILD_DIR}/CesiumNative/CesiumNative/)

# generate SHA256 for files in Packages/CesiumNative
file(GLOB_RECURSE PACKAGE_FILES 
      LIST_DIRECTORIES false
      ${PACKAGE_BUILD_DIR}/CesiumNative/*)

foreach(FILE ${PACKAGE_FILES})

endforeach()

# create tarball file for Packages/CesiumNative directory
file(MAKE_DIRECTORY ${PACKAGE_INSTALL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "${PACKAGE_INSTALL_DIR}/CesiumNative.tgz" "${PACKAGE_BUILD_DIR}/CesiumNative"
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# save tarball SHA256 in a file for o3de