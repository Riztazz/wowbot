set(VCPKG_TARGET_ARCHITECTURE x64)

set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PLATFORM_TOOLSET v143)
# Remove? leave? We wanna load blizzards model files later and maybe turn them into fbxes for blender validation?
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED "FBXSDK_HEADER_LOCATION")
