set(SOURCES
    window.cc
    entropy.cc
    util.cc
    color.cc
    gl/common.cc
    gl/buffer.cc
    gl/texture.cc
    gl/program.cc
    gl/font.cc
)
set(HEADERS
    window.h
    entropy.h
    util.h
    color.h
    image.h
    types.h
    loc.h
    tiles.h
    files.h
    gl.h
    gl/common.h
    gl/buffer.h
    gl/texture.h
    gl/program.h
    gl/font.h
    fonts.h
    ui.h
)

add_library(pgamecc STATIC ${SOURCES} ${HEADERS})
set_target_properties(pgamecc PROPERTIES PUBLIC_HEADER "${HEADERS}")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DPGAMECC_DEBUG)
    set_property(TARGET pgamecc APPEND PROPERTY
                 INTERFACE_COMPILE_DEFINITIONS PGAMECC_DEBUG)
endif()

find_package(GLEW REQUIRED)
target_include_directories(pgamecc PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(pgamecc ${GLEW_LIBRARIES})

find_package(GLFW REQUIRED)
target_include_directories(pgamecc PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(pgamecc ${GLFW_LIBRARIES})

find_package(GLM 95 REQUIRED)
target_include_directories(pgamecc PUBLIC ${GLM_INCLUDE_DIRS})
add_definitions(${GLM_DEFINITIONS})
string(REGEX REPLACE "^-D" "" GLM_DEFINITIONS ${GLM_DEFINITIONS})
set_property(TARGET pgamecc APPEND PROPERTY
             INTERFACE_COMPILE_DEFINITIONS "${GLM_DEFINITIONS}")

find_package(Noise REQUIRED)
target_include_directories(pgamecc PRIVATE ${NOISE_INCLUDE_DIRS})
target_link_libraries(pgamecc ${NOISE_LIBRARIES})
add_definitions(${NOISE_DEFINITIONS})

find_package(Freetype REQUIRED)
target_include_directories(pgamecc PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(pgamecc ${FREETYPE_LIBRARIES})
# TODO: check version >= 2

install(TARGETS
    pgamecc
    EXPORT pgameccTargets
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
)

foreach(HEADER ${HEADERS})
    get_filename_component(HEADER_DIR ${HEADER} DIRECTORY)
    install(FILES ${HEADER}
            DESTINATION ${INSTALL_INCLUDE_DIR}/pgamecc/${HEADER_DIR})
endforeach()

add_subdirectory(fonts)
add_subdirectory(ui)
