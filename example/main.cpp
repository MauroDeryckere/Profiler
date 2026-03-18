#include <Profiler/ProfilerMacros.h>
#include <Profiler/ServiceLocator.h>

#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

#include "Profiler/InstrumentorTimer.h"
#include "GoogleProfiler.h"

// ---------------------------------------------------------------------------
// Basic profiling: functions and scopes
// ---------------------------------------------------------------------------
void HeavyComputation()
{
	PROFILER_FUNCTION();

	volatile double result{ 0.0 };
	for (uint32_t i{ 0 }; i < 1'000'000; ++i)
	{
		result += std::sin(static_cast<double>(i)) * std::cos(static_cast<double>(i));
	}
}

void ProcessData()
{
	PROFILER_FUNCTION();

	{
		PROFILER_SCOPE("Allocation");
		std::vector<int> data(100'000);
		for (uint32_t i{ 0 }; i < data.size(); ++i)
		{
			data[i] = static_cast<int>(i * 2);
		}
	}

	{
		PROFILER_SCOPE("Computation");
		HeavyComputation();
	}
}

// ---------------------------------------------------------------------------
// Multi-threaded profiling
// ---------------------------------------------------------------------------
void WorkerTask(int id)
{
	PROFILER_FUNCTION();

	for (uint32_t i{ 0 }; i < 3; ++i)
	{
		PROFILER_SCOPE("WorkerIteration");
		volatile double x{ 0.0 };
		for (uint32_t j{ 0 }; j < 500'000; ++j)
		{
			x += std::sin(static_cast<double>(j + id));
		}
	}
}

void RunMultiThreadedDemo()
{
	PROFILER_SCOPE("MultiThreaded");

	std::vector<std::jthread> workers;
	for (uint32_t i{ 0 }; i < 4; ++i)
	{
		workers.emplace_back(WorkerTask, i);
	}
}

// ---------------------------------------------------------------------------
// Frame-based profiling (auto-stops after N frames)
// ---------------------------------------------------------------------------
void SimulateFrame(int frame)
{
	PROFILER_SCOPE("GameFrame");

	{
		PROFILER_SCOPE("Update");
		volatile double x{ 0.0 };
		for (uint32_t i{ 0 }; i < 100'000; ++i)
		{
			x += static_cast<double>(i * frame);
		}
	}

	{
		PROFILER_SCOPE("Render");
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}

void RunFrameBasedDemo()
{
	PROFILER_BEGIN_SESSION("FrameDemo", "profiling/frames", 5);

	for (uint32_t frame{ 0 }; frame < 10; ++frame)
	{
		SimulateFrame(frame);
		PROFILER_TICK(); // auto-ends after 5 frames
	}
}

// ---------------------------------------------------------------------------
// String-only output (no file) + frame callback
// ---------------------------------------------------------------------------
void RunStringOutputDemo()
{
	PROFILER.BeginSession("CallbackDemo", {}, 3, [](std::string const& json)
	{
		std::cout << "  -> Callback received " << json.size() << " bytes of JSON\n";
	});

	for (uint32_t frame{ 0 }; frame < 3; ++frame)
	{
		SimulateFrame(frame);
		PROFILER_TICK();
	}
}

// ---------------------------------------------------------------------------
// Manual Stop(): profile only part of a scope
// ---------------------------------------------------------------------------
void RunManualStopDemo()
{
	PROFILER_BEGIN_SESSION("ManualStopDemo", "profiling/manual_stop");

	{
		profiler::InstrumentorTimer timer("PartialWork", true);

		// Only this part is measured
		volatile double x{ 0.0 };
		for (uint32_t i{ 0 }; i < 500'000; ++i)
		{
			x += std::sin(static_cast<double>(i));
		}

		timer.Stop();

		// This part is NOT included in the profile event
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	PROFILER_END_SESSION();
}

// ---------------------------------------------------------------------------
// Custom backend via RegisterProfiler
// ---------------------------------------------------------------------------
void RunRegisterProfilerDemo()
{
	// Swap in a fresh GoogleProfiler instance at runtime
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER_BEGIN_SESSION("CustomBackend", "profiling/custom_backend");

	{
		PROFILER_SCOPE("AfterSwap");
		volatile double x{ 0.0 };
		for (uint32_t i{ 0 }; i < 100'000; ++i)
		{
			x += static_cast<double>(i);
		}
	}

	PROFILER_END_SESSION();
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main()
{
	// --- Demo 1: Basic session profiling ---
	std::cout << "=== Demo 1: Basic Session Profiling ===\n";

	PROFILER_BEGIN_SESSION("BasicDemo", "profiling/basic");

	for (uint32_t frame{ 0 }; frame < 3; ++frame)
	{
		PROFILER_SCOPE("Frame");
		ProcessData();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	PROFILER_END_SESSION();
	std::cout << "  -> profiling/basic.json\n";

	// --- Demo 2: Multi-threaded profiling ---
	std::cout << "\n=== Demo 2: Multi-Threaded Profiling ===\n";

	PROFILER_BEGIN_SESSION("ThreadedDemo", "profiling/threaded");
	RunMultiThreadedDemo();
	PROFILER_END_SESSION();
	std::cout << "  -> profiling/threaded.json\n";

	// --- Demo 3: Frame-based auto-stop profiling ---
	std::cout << "\n=== Demo 3: Frame-Based Profiling (auto-stops after 5 frames) ===\n";
	RunFrameBasedDemo();
	std::cout << "  -> profiling/frames.json\n";

	// --- Demo 4: String-only output with callback ---
	std::cout << "\n=== Demo 4: String Output with Callback (no file) ===\n";
	RunStringOutputDemo();

	// --- Demo 5: Manual FlushToString ---
	std::cout << "\n=== Demo 5: Manual FlushToString ===\n";
	PROFILER_BEGIN_SESSION("FlushDemo");
	{
		PROFILER_SCOPE("SomeWork");
		volatile double x{ 0.0 };
		for (uint32_t i{ 0 }; i < 100'000; ++i)
		{
			x += static_cast<double>(i);
		}
	}
	auto const json{ PROFILER.FlushToString() };
	std::cout << "  -> FlushToString returned " << json.size() << " bytes\n";
	PROFILER_END_SESSION();

	// --- Demo 6: Manual Stop() ---
	std::cout << "\n=== Demo 6: Manual Timer Stop ===\n";
	RunManualStopDemo();
	std::cout << "  -> profiling/manual_stop.json\n";

	// --- Demo 7: RegisterProfiler (swap backend at runtime) ---
	std::cout << "\n=== Demo 7: RegisterProfiler (Custom Backend) ===\n";
	RunRegisterProfilerDemo();
	std::cout << "  -> profiling/custom_backend.json\n";

	std::cout << "\nAll done! Open .json files in chrome://tracing or https://ui.perfetto.dev\n";
	return 0;
}
