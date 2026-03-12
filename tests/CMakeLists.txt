include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.14.0
)
# Prevent gtest from overriding compiler/linker options on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(ProfilerTests
	test_NullProfiler.cpp
	test_GoogleProfiler.cpp
	test_ServiceLocator.cpp
	test_InstrumentorTimer.cpp
	test_Profiler.cpp
	test_Macros.cpp
)

target_link_libraries(ProfilerTests
	PRIVATE
		Profiler::Profiler
		GTest::gtest_main
)

# Give tests access to private headers (GoogleProfiler, NullProfiler, etc.)
target_include_directories(ProfilerTests
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(ProfilerTests)
