# These fonts is probably available on the system, so no need to package it.
# However, it's still more convenient to include it in the binary.
set(FONTS)
foreach(FONT DejaVuSans.ttf DejaVuSans-Bold.ttf)
    execute_process(
        COMMAND fc-list : file
        COMMAND fgrep /${FONT}:
        COMMAND cut -d : -f 1
        OUTPUT_VARIABLE FONT_PATH)
    get_filename_component(FONT_PATH ${FONT_PATH} DIRECTORY)
    find_file(FONT_FILE_${FONT} ${FONT} HINTS ${FONT_PATH} NO_DEFAULT_PATH)
    # TODO: error if not found
    list(APPEND FONTS ${FONT_FILE_${FONT}})
endforeach()

add_custom_command(
    OUTPUT fonts.cc
    COMMAND ${CMAKE_SOURCE_DIR}/bin/gen-files.py -v pgamecc::fonts
            -o ${CMAKE_CURRENT_BINARY_DIR}/fonts.cc -b ${FONTS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${FONTS}
)

link_libraries(pgamecc)

add_library(pgamecc_fonts STATIC ${CMAKE_CURRENT_BINARY_DIR}/fonts.cc)

install(TARGETS
    pgamecc_fonts
    EXPORT pgameccTargets
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
)
