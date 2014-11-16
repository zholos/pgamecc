# installed under libnoise on Linux but under noise on FreeBSD
find_path(NOISE_INCLUDE_DIR NAMES libnoise/noise.h noise/noise.h)
if(EXISTS ${NOISE_INCLUDE_DIR}/libnoise/noise.h)
    set(NOISE_INCLUDE_FILE "libnoise/noise.h")
else()
    set(NOISE_INCLUDE_FILE "noise/noise.h")
endif()
set(NOISE_DEFINITIONS "-DNOISE_INCLUDE_FILE=<${NOISE_INCLUDE_FILE}>")

# FreeBSD doesn't have a libnoise.so link
find_library(NOISE_LIBRARY NAMES libnoise.so.0 noise)

set(NOISE_LIBRARIES ${NOISE_LIBRARY})
set(NOISE_INCLUDE_DIRS ${NOISE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Noise DEFAULT_MSG
    NOISE_LIBRARY NOISE_INCLUDE_DIR)
mark_as_advanced(NOISE_INCLUDE_DIR NOISE_LIBRARY)
