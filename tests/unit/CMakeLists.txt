find_package(doctest CONFIG REQUIRED)

file(GLOB_RECURSE UNIT_TEST_SRC "*.cpp")

add_executable(Phoenix.Tests ${UNIT_TEST_SRC})

target_link_libraries(Phoenix.Tests
    PRIVATE
        Phoenix.Sim
        doctest::doctest
)

# Force the linker to include all object files from Phoenix.Sim so that
# static-initializer-only TUs (e.g. FixedTypeRegistrations.cpp) are not
# dead-stripped before they have a chance to register their types.
if(MSVC)
    target_link_options(Phoenix.Tests PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:Phoenix.Sim>)
elseif(APPLE)
    target_link_options(Phoenix.Tests PRIVATE -Wl,-force_load,$<TARGET_FILE:Phoenix.Sim>)
else()
    target_link_options(Phoenix.Tests PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:Phoenix.Sim> -Wl,--no-whole-archive)
endif()

target_include_directories(Phoenix.Tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

include(doctest)
doctest_discover_tests(Phoenix.Tests)
