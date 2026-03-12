#include <Profiler/ProfilerMacros.h>
#include <Profiler/ProfilerInstance.h>

#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

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

int main()
{
	// Initialize the profiler (creates the Google/Chrome tracing backend by default)
	profiler::ProfilerInstance::Initialize();

	// Begin a profiling session — output goes to "profiling/example.json"
	PROFILER_BEGIN_SESSION("Example", "profiling/example");

	for (int frame = 0; frame < 3; ++frame)
	{
		PROFILER_SCOPE("Frame");
		ProcessData();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	PROFILER_END_SESSION();

	std::cout << "Profiling complete! Open 'profiling/example.json' in chrome://tracing\n";

	profiler::ProfilerInstance::Shutdown();
	return 0;
}
