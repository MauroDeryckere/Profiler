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
#include <Profiler/ProfilerInstance.h>

int main()
{
    profiler::ProfilerInstance::Initialize();

    PROFILER_BEGIN_SESSION("MySession", "output/profile");

    {
        PROFILER_SCOPE("MyScope");
        // ... code to profile ...
    }

    // Writes output/profile.json
    PROFILER_END_SESSION();

    profiler::ProfilerInstance::Shutdown();
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

### Disabling Profiling

Set `PROFILER_ENABLED` to `OFF` for zero runtime cost — all macros expand to nothing:

```cmake
set(PROFILER_ENABLED OFF CACHE BOOL "" FORCE)
```

### Frame-Based Profiling

For game engines or frame-based applications, the profiler can auto-end a session after a fixed number of frames:

```cpp
auto& prof = profiler::ProfilerInstance::Get();
prof.SetMaxFrames(10);  // profile 10 frames, then auto-stop
prof.Start("profiling/gameplay");

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
| `PROFILER_FUNCTION()`                  | Profile the current function (full signature)          |
| `PROFILER_SCOPE(name)`                 | Profile a named scope                                  |
| `PROFILER_BEGIN_SESSION(name, path)`   | Start a profiling session, output to `path.json`       |
| `PROFILER_END_SESSION()`               | End the current session and flush output               |
| `PROFILER_UPDATE()`                    | Advance frame counter (for frame-based profiling)      |
| `PROFILER_THREAD(name)`               | Name a thread (Optick backend only)                    |
| `PROFILER_FRAME(name)`                | Mark a frame boundary (Optick backend only)            |

### Classes

**`profiler::ProfilerInstance`** — global profiler management

| Method                             | Description                                    |
|------------------------------------|------------------------------------------------|
| `Initialize()`                     | Create the default backend                     |
| `Set(std::unique_ptr<Profiler>)`   | Use a custom backend                           |
| `Get()`                            | Access the active profiler                     |
| `Shutdown()`                       | Reset to no-op profiler                        |

## Custom Backend

Implement the `profiler::Profiler` interface to create your own backend:

```cpp
#include <Profiler/Profiler.h>

class MyProfiler : public profiler::Profiler
{
    void BeginSessionInternal(std::string const& name, size_t reserveSize) override { /* ... */ }
    void WriteProfile(profiler::ProfileResult const& result, bool isFunction) override { /* ... */ }
    void WriteProfile(std::string const& name) override { /* ... */ }
    void EndSession() override { /* ... */ }
};

// Register it:
profiler::ProfilerInstance::Set(std::make_unique<MyProfiler>());
```

## Project Structure

```
include/Profiler/          Public API headers
  Profiler.h               Base profiler class & ProfileResult
  InstrumentorTimer.h      RAII scope timer
  ProfilerInstance.h        Global instance management
  ProfilerMacros.h         Instrumentation macros (main include)

src/                       Private implementation
  GoogleProfiler.h/.cpp    Chrome Trace Event Format backend
  OptickProfiler.h/.cpp    Optick backend (optional)
  NullProfiler.h           No-op backend
  ProfilerInstance.cpp      Singleton management

example/                   Example application
cmake/                     CMake package config
```

## Requirements

- C++17 compiler (MSVC 2017+, GCC 7+, Clang 5+)
- CMake 3.16+
- (Optional) [Optick](https://github.com/bombomby/optick) for the Optick backend

## License

[Choose your license]
