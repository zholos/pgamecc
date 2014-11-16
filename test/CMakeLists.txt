link_libraries(pgamecc)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
# Use the single-header implementation because the library is incompatible when
# compiled with a different C++ compiler

enable_testing()

foreach(TEST types color image entropy tiles loc)
    add_executable(test_${TEST} ${TEST}.cc)
    add_test(${TEST} test_${TEST})
endforeach()
