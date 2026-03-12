cmake_minimum_required(VERSION 3.16)
project(Profiler
	VERSION 1.0.0
	LANGUAGES CXX
	DESCRIPTION "Lightweight, cross-platform C++17 profiling library"
)

# --- Options ---
option(PROFILER_ENABLED      "Enable profiling instrumentation"                ON)
option(PROFILER_USE_OPTICK   "Use Optick backend (requires Optick library)"    OFF)
option(PROFILER_BUILD_EXAMPLE "Build the example application"                  OFF)
option(PROFILER_BUILD_TESTS   "Build the test suite"                           OFF)

# --- Sources ---
set(PROFILER_SOURCES
	src/Profiler.cpp
	src/InstrumentorTimer.cpp
	src/GoogleProfiler.cpp
	src/ServiceLocator.cpp
)

if(PROFILER_USE_OPTICK)
	list(APPEND PROFILER_SOURCES src/OptickProfiler.cpp)
endif()

# --- Library ---
add_library(Profiler STATIC ${PROFILER_SOURCES})
add_library(Profiler::Profiler ALIAS Profiler)

target_include_directories(Profiler
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(Profiler PUBLIC cxx_std_17)

# --- Compile definitions ---
if(PROFILER_ENABLED)
	target_compile_definitions(Profiler PUBLIC PROFILER_ENABLED)
endif()

if(PROFILER_USE_OPTICK)
	target_compile_definitions(Profiler PUBLIC PROFILER_USE_OPTICK)

	# Fetch Optick if not already available
	if(NOT TARGET OptickCore)
		include(FetchContent)
		FetchContent_Declare(
			Optick
			GIT_REPOSITORY https://github.com/bombomby/optick.git
			GIT_TAG master
		)
		# Disable Optick's own samples/tests
		set(OPTICK_BUILD_GUI_APP OFF CACHE BOOL "" FORCE)
		set(OPTICK_BUILD_CONSOLE_APP OFF CACHE BOOL "" FORCE)
		set(OPTICK_INSTALL OFF CACHE BOOL "" FORCE)
		FetchContent_MakeAvailable(Optick)
	endif()

	target_link_libraries(Profiler PUBLIC OptickCore)
endif()

# --- Platform ---
find_package(Threads REQUIRED)
target_link_libraries(Profiler PRIVATE Threads::Threads)

# --- Example ---
if(PROFILER_BUILD_EXAMPLE)
	add_subdirectory(example)
endif()

# --- Tests ---
if(PROFILER_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

# --- Install ---
include(GNUInstallDirs)

install(TARGETS Profiler
	EXPORT ProfilerTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/Profiler
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ProfilerTargets
	FILE ProfilerTargets.cmake
	NAMESPACE Profiler::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Profiler
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProfilerConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/ProfilerConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Profiler
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/ProfilerConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/ProfilerConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/ProfilerConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Profiler
)
