find_package(doctest CONFIG REQUIRED)

file(GLOB_RECURSE UNIT_TEST_SRC "*.cpp")

add_executable(PhoenixTests ${UNIT_TEST_SRC})

target_link_libraries(PhoenixTests
    PRIVATE
        PhoenixSim
        doctest::doctest
)

# Force the linker to include all object files from PhoenixSim so that
# static-initializer-only TUs (e.g. FixedTypeRegistrations.cpp) are not
# dead-stripped before they have a chance to register their types.
if(MSVC)
    target_link_options(PhoenixTests PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSim>)
elseif(APPLE)
    target_link_options(PhoenixTests PRIVATE -Wl,-force_load,$<TARGET_FILE:PhoenixSim>)
else()
    target_link_options(PhoenixTests PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:PhoenixSim> -Wl,--no-whole-archive)
endif()

target_include_directories(PhoenixTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

include(doctest)
doctest_discover_tests(PhoenixTests)
