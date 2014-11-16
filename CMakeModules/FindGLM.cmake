find_path(GLM_INCLUDE_DIRS glm/glm.hpp)

set(GLM_DEFINITIONS)
if(GLM_INCLUDE_DIRS AND EXISTS "${GLM_INCLUDE_DIRS}/glm/detail/setup.hpp")
    # not using the preprocessor is weird but good for cross-compiling
    file(STRINGS "${GLM_INCLUDE_DIRS}/glm/detail/setup.hpp" GLM_VERSION REGEX
        "^[ \t]*#define[ \t]+GLM_VERSION[ \t]+[0-9]+[ \t]*$")
    string(REGEX REPLACE
        "^[ \t]*#define[ \t]+GLM_VERSION[ \t]+([0-9]+)[ \t]*$" "\\1"
        GLM_VERSION "${GLM_VERSION}")
    if(GLM_VERSION GREATER 94)
        # silence warnings, be careful in code
        set(GLM_DEFINITIONS "-DGLM_FORCE_RADIANS")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM
    REQUIRED_VARS GLM_INCLUDE_DIRS
    VERSION_VAR GLM_VERSION)
