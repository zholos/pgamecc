set(SOURCES
    controls.cc
    widgets.cc
    layer.cc
    window.cc
)
set(HEADERS
    controls.h
    widgets.h
    layer.h
    window.h
)

# separate library to avoid cyclic dependency between pgamecc and pgamecc_fonts
link_libraries(pgamecc_fonts)

add_library(pgamecc_ui STATIC ${SOURCES} ${HEADERS})
set_target_properties(pgamecc_ui PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS
    pgamecc_ui
    EXPORT pgameccTargets
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
)

foreach(HEADER ${HEADERS})
    get_filename_component(HEADER_DIR ${HEADER} DIRECTORY)
    install(FILES ${HEADER}
            DESTINATION ${INSTALL_INCLUDE_DIR}/pgamecc/ui/${HEADER_DIR})
endforeach()
