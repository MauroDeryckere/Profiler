# Profiler

A lightweight, cross-platform C++17 profiling library with Chrome DevTools and Optick backend support.

## Features

- **Zero overhead when disabled** — all macros compile to nothing
- **Chrome DevTools integration** — output JSON viewable in `chrome://tracing` or [Perfetto](https://ui.perfetto.dev)
- **Optick support** — optional backend for the [Optick](https://github.com/bombomby/optick) profiler
- **Thread-safe** — safe for multi-threaded applications
- **RAII-based** — automatic scope timing via `PROFILER_SCOPE` and `PROFILER_FUNCTION`
- **Cross-platform** — Windows, Linux, macOS
- **Easy CMake integration** — `add_subdirectory`, `FetchContent`, or `find_package`

## Quick Start

### CMake Integration

**add_subdirectory:**

```cmake
add_subdirectory(path/to/Profiler)
target_link_libraries(YourTarget PRIVATE Profiler::Profiler)
```

**FetchContent:**

```cmake
include(FetchContent)
FetchContent_Declare(
    Profiler
    GIT_REPOSITORY https://github.com/your-username/Profiler.git
    GIT_TAG main
)
FetchContent_MakeAvailable(Profiler)
target_link_libraries(YourTarget PRIVATE Profiler::Profiler)
```

### Basic Usage

```cpp
#include <Profiler/ProfilerMacros.h>

int main()
{
    // Profiler is auto-initialized — no setup needed
    PROFILER_BEGIN_SESSION("MySession", "output/profile");

    {
        PROFILER_SCOPE("MyScope");
        // ... code to profile ...
    }

    // Writes output/profile.json
    PROFILER_END_SESSION();
}

void MyFunction()
{
    PROFILER_FUNCTION();  // profiles the entire function
    // ... function body ...
}
```

### Viewing Results

Open the generated `.json` file in:

- **Chrome/Edge**: navigate to `chrome://tracing` (or `edge://tracing`) and load the file
- **Perfetto**: upload at [ui.perfetto.dev](https://ui.perfetto.dev)

## Configuration

### CMake Options

| Option                  | Default | Description                                      |
|-------------------------|---------|--------------------------------------------------|
| `PROFILER_ENABLED`      | `ON`    | Enable profiling instrumentation                 |
| `PROFILER_USE_OPTICK`   | `OFF`   | Use Optick backend (auto-fetched via FetchContent)|
| `PROFILER_BUILD_EXAMPLE`| `OFF`   | Build the example application                    |
| `PROFILER_BUILD_TESTS`  | `OFF`   | Build the test suite (Google Test)               |

### Disabling Profiling

Set `PROFILER_ENABLED` to `OFF` for zero runtime cost — all macros expand to nothing:

```cmake
set(PROFILER_ENABLED OFF CACHE BOOL "" FORCE)
```

### Frame-Based Profiling

For game engines or frame-based applications, the profiler can auto-end a session after a fixed number of frames:

```cpp
PROFILER.SetMaxFrames(10);  // profile 10 frames, then auto-stop
PROFILER.Start("profiling/gameplay");

// In your game loop:
while (running)
{
    PROFILER_SCOPE("Frame");
    // ... frame logic ...
    PROFILER_UPDATE();
}
```

## API Reference

### Macros

| Macro                                  | Description                                            |
|----------------------------------------|--------------------------------------------------------|
| `PROFILER`                             | Access the active profiler instance                    |
| `PROFILER_FUNCTION()`                  | Profile the current function                           |
| `PROFILER_SCOPE(name)`                 | Profile a named scope                                  |
| `PROFILER_BEGIN_SESSION(name, path)`   | Start a profiling session, output to `path.json`       |
| `PROFILER_END_SESSION()`               | End the current session and flush output               |
| `PROFILER_UPDATE()`                    | Advance frame counter (for frame-based profiling)      |
| `PROFILER_THREAD(name)`               | Name a thread (Optick backend only)                    |
| `PROFILER_FRAME(name)`                | Mark a frame boundary (Optick backend only)            |

### ServiceLocator

**`profiler::ServiceLocator`** — global profiler management (namespace)

| Function                                       | Description                                    |
|------------------------------------------------|------------------------------------------------|
| `GetProfiler()`                                | Access the active profiler                     |
| `RegisterProfiler(std::unique_ptr<Profiler>)`  | Swap in a custom backend                       |

The profiler is auto-initialized at static init time based on compile flags — no manual setup needed.

## Custom Backend

Implement the `profiler::Profiler` interface to create your own backend:

```cpp
#include <Profiler/Profiler.h>

class MyProfiler : public profiler::Profiler
{
    void BeginSessionInternal(std::string const& name, size_t reserveSize) override { /* ... */ }
    void WriteProfile(profiler::ProfileResult const& result, bool isFunction) override { /* ... */ }
    void EndSession() override { /* ... */ }
};

// Register it:
profiler::ServiceLocator::RegisterProfiler(std::make_unique<MyProfiler>());
```

## Building & Testing

```bash
# Configure with tests and example
cmake -B build -DPROFILER_BUILD_TESTS=ON -DPROFILER_BUILD_EXAMPLE=ON

# Build
cmake --build build --config Release

# Run tests
cd build && ctest -C Release --output-on-failure

# Run example
./example/Release/ProfilerExample
```

## Project Structure

```
include/Profiler/          Public API headers
  Profiler.h               Base profiler class & ProfileResult
  InstrumentorTimer.h      RAII scope timer
  ServiceLocator.h         Global instance management & PROFILER macro
  ProfilerMacros.h         Instrumentation macros (main include)

src/                       Private implementation
  GoogleProfiler.h/.cpp    Chrome Trace Event Format backend
  OptickProfiler.h/.cpp    Optick backend (optional)
  NullProfiler.h           No-op backend
  ServiceLocator.cpp       Backend initialization

example/                   Example application (3 demos)
tests/                     Google Test suite (32 tests)
cmake/                     CMake package config
```

## Requirements

- C++17 compiler (MSVC 2017+, GCC 7+, Clang 5+)
- CMake 3.16+
- (Optional) [Optick](https://github.com/bombomby/optick) for the Optick backend

## License

[Choose your license]
