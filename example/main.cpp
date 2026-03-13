#include <Profiler/ProfilerMacros.h>
#include <Profiler/ServiceLocator.h>

#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Basic profiling: functions and scopes
// ---------------------------------------------------------------------------

void HeavyComputation()
{
	PROFILER_FUNCTION();

	volatile double result = 0.0;
	for (int i = 0; i < 1'000'000; ++i)
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
		for (size_t i = 0; i < data.size(); ++i)
			data[i] = static_cast<int>(i * 2);
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

	for (int i = 0; i < 3; ++i)
	{
		PROFILER_SCOPE("WorkerIteration");
		volatile double x = 0.0;
		for (int j = 0; j < 500'000; ++j)
			x += std::sin(static_cast<double>(j + id));
	}
}

void RunMultiThreadedDemo()
{
	PROFILER_SCOPE("MultiThreaded");

	std::vector<std::thread> workers;
	for (int i = 0; i < 4; ++i)
		workers.emplace_back(WorkerTask, i);

	for (auto& t : workers)
		t.join();
}

// ---------------------------------------------------------------------------
// Frame-based profiling (auto-stops after N frames)
// ---------------------------------------------------------------------------

void SimulateFrame(int frame)
{
	PROFILER_SCOPE("GameFrame");

	{
		PROFILER_SCOPE("Update");
		volatile double x = 0.0;
		for (int i = 0; i < 100'000; ++i)
			x += static_cast<double>(i * frame);
	}

	{
		PROFILER_SCOPE("Render");
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}

void RunFrameBasedDemo()
{
	PROFILER.SetNumFramesToProfile(5);
	PROFILER.Start("profiling/frames");

	for (int frame = 0; frame < 10; ++frame)
	{
		SimulateFrame(frame);
		PROFILER_UPDATE(); // auto-ends after 5 frames
	}
}

// ---------------------------------------------------------------------------
// String-only output (no file) + frame callback
// ---------------------------------------------------------------------------

void RunStringOutputDemo()
{
	PROFILER.SetNumFramesToProfile(3);
	PROFILER.Start(nullptr, [](std::string const& json)
	{
		std::cout << "  -> Callback received " << json.size() << " bytes of JSON\n";
	});

	for (int frame = 0; frame < 3; ++frame)
	{
		SimulateFrame(frame);
		PROFILER_UPDATE();
	}
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main()
{
	// --- Demo 1: Basic session profiling ---
	std::cout << "=== Demo 1: Basic Session Profiling ===\n";

	PROFILER_BEGIN_SESSION("BasicDemo", "profiling/basic");

	for (int frame = 0; frame < 3; ++frame)
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
	std::cout << "  -> profiling/frames0.json\n";

	// --- Demo 4: String-only output with callback ---
	std::cout << "\n=== Demo 4: String Output with Callback (no file) ===\n";
	RunStringOutputDemo();

	// --- Demo 5: Manual FlushToString ---
	std::cout << "\n=== Demo 5: Manual FlushToString ===\n";
	PROFILER_BEGIN_SESSION("FlushDemo");
	{
		PROFILER_SCOPE("SomeWork");
		volatile double x = 0.0;
		for (int i = 0; i < 100'000; ++i)
			x += static_cast<double>(i);
	}
	auto json = PROFILER.FlushToString();
	std::cout << "  -> FlushToString returned " << json.size() << " bytes\n";
	PROFILER_END_SESSION();

	std::cout << "\nAll done! Open .json files in chrome://tracing or https://ui.perfetto.dev\n";
	return 0;
}
