cmake_minimum_required(VERSION 3.0)

project(pgamecc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL GNU)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    add_definitions(-Wreturn-type)
    add_definitions(-Wall -Wno-parentheses)
    # don't use -Werror in release, compilers may add stylistic warnings
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL Clang)
    add_definitions(-w)
endif()

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/CMakeModules")

# library source directory is named pgamecc to allow including as
# <pgamecc/window.h> both inside source tree and externally
include_directories(BEFORE .)

# for possible future configuration
set(INSTALL_INCLUDE_DIR include           CACHE PATH "")
set(INSTALL_LIB_DIR     lib               CACHE PATH "")
set(INSTALL_CMAKE_DIR   lib/cmake/pgamecc CACHE PATH "")

add_subdirectory(pgamecc)
add_subdirectory(test)
add_subdirectory(demo)
add_subdirectory(bin)

install(FILES
    pgamecc.h
    DESTINATION ${INSTALL_INCLUDE_DIR}
)


# following stuff is for using the library in other CMake projects
include(CMakePackageConfigHelpers)
configure_package_config_file(
    pgameccConfig.cmake.in
    pgameccConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CMAKE_DIR}
    PATH_VARS INSTALL_INCLUDE_DIR
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pgameccConfig.cmake
    DESTINATION ${INSTALL_CMAKE_DIR}
)
install(EXPORT
    pgameccTargets
    DESTINATION ${INSTALL_CMAKE_DIR}
)
