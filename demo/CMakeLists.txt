link_libraries(pgamecc)

add_executable(cube cube.cc)
add_executable(planet planet.cc)

link_libraries(pgamecc_fonts)

add_executable(font font.cc)

link_libraries(pgamecc_ui)

add_executable(ui ui.cc)
