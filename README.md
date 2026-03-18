# Profiler

A lightweight, cross-platform C++20 profiling library with Chrome DevTools and Optick backend support.

## Features

- **Zero overhead when disabled** - all macros compile to nothing
- **Chrome DevTools integration** - output JSON viewable in `chrome://tracing` or [Perfetto](https://ui.perfetto.dev)
- **Optick support** — optional backend for the [Optick](https://github.com/bombomby/optick) profiler
- **Thread-safe** — lock-free per-thread binary event buffers with minimal contention (see [Threading Model](#threading-model))
- **RAII-based** — automatic scope timing via `PROFILER_SCOPE` and `PROFILER_FUNCTION`
- **Cross-platform** — Windows, Linux, macOS, Android (NDK), iOS
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
// Profile 5 frames, then auto-stop and write to disk
PROFILER_BEGIN_SESSION("Gameplay", "profiling/gameplay", 5);

// In your game loop:
while (running)
{
    PROFILER_FRAME("MainThread");  // names the thread
    PROFILER_SCOPE("Frame");
    // ... frame logic ...
    PROFILER_TICK();  // advances frame counter, auto-ends after 5
}
```

## API Reference

### Macros

| Macro                                  | Description                                            |
|----------------------------------------|--------------------------------------------------------|
| `PROFILER`                             | Access the active profiler instance                    |
| `PROFILER_FUNCTION()`                  | Profile the current function                           |
| `PROFILER_SCOPE(name)`                 | Profile a named scope                                  |
| `PROFILER_BEGIN_SESSION(name, ...)`    | Start a profiling session (path, maxFrames, callback are optional) |
| `PROFILER_END_SESSION()`               | End the current session and flush output               |
| `PROFILER_TICK()`                      | Advance frame counter (for frame-based profiling)      |
| `PROFILER_THREAD(name)`               | Name the calling thread in the trace output            |
| `PROFILER_FRAME(name)`                | Name the calling thread (Optick: also marks frame boundary) |

### ServiceLocator

**`profiler::ServiceLocator`** — global profiler management (namespace)

| Function                                       | Description                                    |
|------------------------------------------------|------------------------------------------------|
| `GetProfiler()`                                | Access the profiler (concrete type, no virtual dispatch) |

The backend is selected at compile time (`PROFILER_USE_OPTICK` flag). `GetProfiler()` returns the concrete
type (`GoogleProfiler&` or `OptickProfiler&`), enabling the compiler to devirtualize all calls. To disable
profiling at runtime, simply call `EndSession()` or don't call `BeginSession()` — `WriteProfile` is a no-op
when no session is active.

## Threading Model

`PROFILER_SCOPE`, `PROFILER_FUNCTION`, and `WriteProfile` are safe to call concurrently from any number
of threads — each thread writes to its own lock-free buffer with no contention.

**Session lifecycle functions (`BeginSession`, `EndSession`, `Tick`) are not thread-safe.** They must be
called from a single thread (typically the main thread), and no other thread may be calling `WriteProfile`
at the same time. In practice this means:

```cpp
PROFILER_BEGIN_SESSION("Game", "profiling/game");   // main thread, before spawning workers

// ... workers run, call PROFILER_SCOPE / PROFILER_FUNCTION freely ...

// join all worker threads first, then:
PROFILER_END_SESSION();                             // main thread, after joining workers
```

Calling `BeginSession` or `EndSession` while worker threads are actively writing profile events is
undefined behavior.

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
include/Profiler/          Public headers
  Profiler.h               Base profiler class & ProfileResult
  InstrumentorTimer.h      RAII scope timer
  ServiceLocator.h         Global instance management & PROFILER macro
  ProfilerMacros.h         Instrumentation macros (main include)
  GoogleProfiler.h         Chrome Trace Event Format backend
  OptickProfiler.h         Optick backend (optional)
  NullProfiler.h           No-op backend

src/                       Implementation
  GoogleProfiler.cpp       Google backend implementation
  OptickProfiler.cpp       Optick backend implementation (optional)
  ServiceLocator.cpp       Backend initialization

example/                   Example application (6 demos)
tests/                     Google Test suite (46 tests + 5 benchmarks)
cmake/                     CMake package config
```

## Benchmarks

We benchmark the profiler itself to make sure the instrumentation overhead stays minimal. The goal is to
quantify exactly how much time your application spends *inside the profiler* so you can make informed
decisions about where to instrument.

### Methodology

Each benchmark runs the measured operation **10 times**. The single highest and single lowest samples are
discarded to remove outliers caused by OS scheduling, CPU frequency scaling, or filesystem cache
cold-starts. The remaining **8 samples are averaged** to produce the reported number.

All benchmarks are compiled in **Release mode** (`/O2` on MSVC) and run on the same machine in a single
process. The benchmark source lives in `tests/bench_GoogleProfiler.cpp` and runs as part of the Google
Test suite (filter with `--gtest_filter="ProfilerBench.*"`).

### What we measure and why

| Benchmark | What it does | Why it matters |
|---|---|---|
| **WriteProfile** | Calls `GoogleProfiler::WriteProfile()` 100k times on a single thread. Each call stores a binary `TraceEvent` struct into a pre-reserved per-thread buffer. | This is the hot path. Every `PROFILER_SCOPE` and `PROFILER_FUNCTION` ultimately calls `WriteProfile` once when the scope ends. This number tells you the per-event cost of recording a trace event. |
| **WriteProfile (4 threads)** | Same as above but split across 4 threads (25k calls each), measuring wall-clock time. | Validates that the thread-local buffer design scales under contention. Each thread writes to its own `ThreadBuffer`, so the only lock acquisition happens once per thread per session (on first write). |
| **InstrumentorTimer RAII** | Constructs and destructs an `InstrumentorTimer` 50k times. Each cycle records a start time (constructor) and emits a single complete event (destructor). | This is what `PROFILER_SCOPE("name")` and `PROFILER_FUNCTION()` actually expand to. Measures the real end-to-end cost including two `chrono::high_resolution_clock::now()` calls, one `WriteProfile` dispatch through the `ServiceLocator`, and RAII overhead. |
| **FlushToString (10k events)** | Buffers 10k trace events, then calls `FlushToString()` 100 times (read-only, does not clear the buffer). | Measures the cost of converting binary event buffers to JSON. Relevant for streaming use-cases (e.g. Emscripten, live viewers) where you call `FlushToString()` periodically without ending the session. |
| **Session lifecycle** | Runs 1000 full cycles of: construct `GoogleProfiler` -> `BeginSession` (with file path) -> `WriteProfile` (1 event) -> `EndSession` (writes `.json` to disk). | Measures the worst-case path that includes filesystem I/O (directory creation, file open, write, close). This is *not* the hot path — it only happens once at the start and end of a profiling session — but it bounds the cost of session management. |

### Results

Measured on Windows 11, MSVC 19.x, Release build (`/O2`), AMD Ryzen 7 5800H.

| Benchmark | Calls | Avg time | Per-call |
|---|---|---|---|
| WriteProfile (single thread) | 100,000 | 2.9 ms | **29 ns/call** |
| WriteProfile (4 threads) | 100,000 | 3.0 ms | **30 ns/call** |
| InstrumentorTimer RAII | 50,000 | 5.8 ms | **116 ns/cycle** |
| FlushToString (10k events) | 100 | 280.4 ms | **2.80 ms/call** |
| Session lifecycle (with file I/O) | 1,000 | 1,640.8 ms | **1,641 us/cycle** |

### How to read these numbers

- **InstrumentorTimer at ~116 ns/cycle** is the real-world cost of `PROFILER_SCOPE()`. On a 60fps
  frame (~16.6 ms), profiling 100 scopes costs ~0.012 ms, or about **0.07% of your frame budget**.
- **Multi-threaded scaling** shows near-linear improvement (30 ns/call with 4 threads vs 29 ns
  single-threaded) because each thread writes to its own lock-free buffer.
- **Session lifecycle** is dominated by filesystem I/O (~1.6 ms per cycle). This only happens when
  you call `BeginSession`/`EndSession`, not during normal profiling.
- **FlushToString** converts binary event buffers to JSON on demand. The cost scales with the number
  of buffered events and only runs when you explicitly flush or end a session.

### Running benchmarks

```bash
cmake -B build -DPROFILER_BUILD_TESTS=ON
cmake --build build --config Release
./build/tests/Release/ProfilerTests --gtest_filter="ProfilerBench.*"
```

## Requirements

- C++20 compiler (MSVC 2019 16.10+, GCC 10+, Clang 10+, Apple Clang 13+)
- CMake 3.16+
- (Optional) [Optick](https://github.com/bombomby/optick) for the Optick backend
