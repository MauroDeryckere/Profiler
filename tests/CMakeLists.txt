include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.14.0
)
# Prevent gtest from overriding compiler/linker options on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Separate static library to simulate cross-library profiler calls.
# This reproduces the engine scenario: profiler macros called from
# a different static library than the one defining the profiler.
add_library(ProfilerCrossLibHelper STATIC cross_lib_helper.cpp)
target_link_libraries(ProfilerCrossLibHelper PUBLIC Profiler::Profiler)
target_compile_features(ProfilerCrossLibHelper PRIVATE cxx_std_23)

add_executable(ProfilerTests
	test_GoogleProfiler.cpp
	test_ServiceLocator.cpp
	test_InstrumentorTimer.cpp
	test_Profiler.cpp
	test_Macros.cpp
	bench_GoogleProfiler.cpp
)

target_link_libraries(ProfilerTests
	PRIVATE
		Profiler::Profiler
		ProfilerCrossLibHelper
		GTest::gtest_main
)

# Give tests access to private headers (GoogleProfiler, NullProfiler, etc.)
target_include_directories(ProfilerTests
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_compile_features(ProfilerTests PRIVATE cxx_std_23)

if(ipo_supported)
	set_target_properties(ProfilerTests PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include(GoogleTest)
gtest_discover_tests(ProfilerTests)
