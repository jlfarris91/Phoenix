find_package(doctest CONFIG REQUIRED)

file(GLOB_RECURSE UNIT_TEST_SRC "*.cpp")

add_executable(PhoenixTests ${UNIT_TEST_SRC})

target_link_libraries(PhoenixTests
    PRIVATE
        PhoenixSim
        doctest::doctest
)

target_include_directories(PhoenixTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

include(doctest)
doctest_discover_tests(PhoenixTests)
