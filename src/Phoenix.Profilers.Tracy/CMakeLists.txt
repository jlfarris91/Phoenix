file(GLOB_RECURSE PHOENIX_PROFILERS_TRACY_SRC "*.h" "*.cpp")

add_library(Phoenix.Profilers.Tracy STATIC ${PHOENIX_PROFILERS_TRACY_SRC})
set_property(TARGET Phoenix.Profilers.Tracy PROPERTY FOLDER "Libraries/Profilers")

target_link_libraries(Phoenix.Profilers.Tracy
    PUBLIC  Phoenix Tracy::TracyClient
)

target_compile_definitions(Phoenix.Profilers.Tracy PUBLIC TRACY_ENABLE)
