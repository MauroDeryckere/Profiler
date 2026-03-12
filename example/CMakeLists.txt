add_executable(ProfilerExample main.cpp)
target_link_libraries(ProfilerExample PRIVATE Profiler::Profiler)

# Run from project root so profiling output goes to a predictable location
set_target_properties(ProfilerExample PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
