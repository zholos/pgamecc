find_package(PkgConfig REQUIRED)

# forward REQUIRED and QUIET to libraries required by GLFW
set(GLFW_PACKAGE_ARGS)
if(GLFW_FIND_QUIETLY)
    list(APPEND GLFW_PACKAGE_ARGS QUIET)
endif()
if(GLFW_FIND_REQUIRED)
    list(APPEND GLFW_PACKAGE_ARGS REQUIRED)
endif()

pkg_search_module(GLFW ${GLFW_PACKAGE_ARGS} glfw3)

# convert relative path to absolute path rather than exporting search path
find_library(GLFW_LIBRARY NAMES ${GLFW_LIBRARIES} PATHS ${GLFW_LIBRARY_DIRS})
set(GLFW_LIBRARIES ${GLFW_LIBRARY})
unset(GLFW_LIBRARY)


if(UNIX)
    find_package(X11 ${GLFW_PACKAGE_ARGS})
    foreach(GLFW_X11_COMPONENT X11 Xrandr Xinput Xxf86vm)
        list(APPEND GLFW_INCLUDE_DIRS ${X11_${GLFW_X11_COMPONENT}_INCLUDE_PATH})
        list(APPEND GLFW_LIBRARIES ${X11_${GLFW_X11_COMPONENT}_LIB})
    endforeach()

    find_package(OpenGL ${GLFW_PACKAGE_ARGS})
    list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${OPENGL_LIBRARIES})

    # FIXME: apparently must be included after GLEW
    list(APPEND GLFW_LIBRARIES rt)

    # FIXME: -lm ?
else()
    # TODO: need something here
endif()

find_package_handle_standard_args(GLFW
    FOUND_VAR GLFW_FOUND
    REQUIRED_VARS GLFW_LIBRARIES GLFW_INCLUDE_DIRS
    FAIL_MESSAGE "Could NOT find GLFW")

unset(GLFW_PACKAGE_ARGS)
