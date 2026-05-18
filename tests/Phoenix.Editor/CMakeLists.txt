file(GLOB_RECURSE EDITOR_TEST_SRC "*.cpp")

add_executable(Phoenix.Editor.Tests ${EDITOR_TEST_SRC})

target_link_libraries(Phoenix.Editor.Tests PRIVATE
    Phoenix.Editor
    doctest::doctest
)

include(doctest)
doctest_discover_tests(Phoenix.Editor.Tests)
