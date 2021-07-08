# copy license file to Packages/CesiumNative/CesiumNative
file(COPY 
      ${CMAKE_CURRENT_LIST_DIR}/CesiumNative/LICENSE 
      DESTINATION ${CMAKE_CURRENT_LIST_DIR}/Packages/CesiumNative/CesiumNative/)

# generate SHA256 for files in Packages/CesiumNative

# create tarball file for Packages/CesiumNative directory

# save tarball SHA256 in a file for o3de