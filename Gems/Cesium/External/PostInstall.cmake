set(PACKAGE_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/Packages)
set(PACKAGE_BUILD_DIR ${PACKAGE_BASE_DIR}/CesiumNative)
set(PACKAGE_INSTALL_DIR ${PACKAGE_BASE_DIR}/Install)

# copy license file to Packages/CesiumNative/CesiumNative
file(COPY 
      ${CMAKE_CURRENT_LIST_DIR}/CesiumNative/LICENSE 
      DESTINATION ${PACKAGE_BUILD_DIR}/CesiumNative/)

# generate SHA256 for files in Packages/CesiumNative
file(GLOB_RECURSE PACKAGE_FILES 
      LIST_DIRECTORIES false
      ${PACKAGE_BUILD_DIR}/*)
list(REMOVE_ITEM PACKAGE_FILES "${PACKAGE_BUILD_DIR}/SHA256SUMS")

set(SHA256SUMS_CONTENT "")
foreach(FILE ${PACKAGE_FILES})
      file(SHA256 ${FILE} FILE_CHECKSUM)
      file(RELATIVE_PATH FILE_RELATIVE_PATH ${PACKAGE_BUILD_DIR} ${FILE})
      set(SHA256SUMS_CONTENT "${SHA256SUMS_CONTENT}${FILE_CHECKSUM} *${FILE_RELATIVE_PATH}\n")
endforeach()

file(WRITE ${PACKAGE_BUILD_DIR}/SHA256SUMS ${SHA256SUMS_CONTENT})

# create tarball file for Packages/Install directory
file(MAKE_DIRECTORY ${PACKAGE_INSTALL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cJf" "${PACKAGE_INSTALL_DIR}/CesiumNative.tar.xz" "."
                WORKING_DIRECTORY ${PACKAGE_BUILD_DIR})

# save tarball SHA256 in a file for o3de
file(SHA256 ${PACKAGE_INSTALL_DIR}/CesiumNative.tar.xz TARBALL_CHECKSUM)
file(WRITE ${PACKAGE_INSTALL_DIR}/SHA256SUMS ${TARBALL_CHECKSUM})