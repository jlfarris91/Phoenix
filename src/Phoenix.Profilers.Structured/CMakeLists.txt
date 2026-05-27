file(GLOB_RECURSE PHOENIX_PROFILERS_STRUCTURED_SRC "*.h" "*.cpp")

add_library(Phoenix.Profilers.Structured STATIC ${PHOENIX_PROFILERS_STRUCTURED_SRC})
set_property(TARGET Phoenix.Profilers.Structured PROPERTY FOLDER "Libraries/Profilers")

target_link_libraries(Phoenix.Profilers.Structured
    PUBLIC Phoenix
)
