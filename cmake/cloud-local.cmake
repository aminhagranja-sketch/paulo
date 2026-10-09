# Optional isolated Linux sysroot created by scripts/bootstrap-cloud.sh.
set(GRANJA_SYSROOT "/workspace/toolchain/sysroot" CACHE PATH "Local development libraries")
list(PREPEND CMAKE_PREFIX_PATH "${GRANJA_SYSROOT}/usr")
list(PREPEND CMAKE_INCLUDE_PATH "${GRANJA_SYSROOT}/usr/include")
list(PREPEND CMAKE_LIBRARY_PATH "${GRANJA_SYSROOT}/usr/lib/x86_64-linux-gnu")
set(FREETYPE_INCLUDE_DIR_freetype2 "${GRANJA_SYSROOT}/usr/include/freetype2" CACHE PATH "")
set(FREETYPE_INCLUDE_DIR_ft2build "${GRANJA_SYSROOT}/usr/include/freetype2" CACHE PATH "")
set(FREETYPE_LIBRARY_RELEASE "${GRANJA_SYSROOT}/usr/lib/x86_64-linux-gnu/libfreetype.so" CACHE FILEPATH "")
