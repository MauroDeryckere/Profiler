#include <Profiler/ProfilerMacros.h>

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
	// Profiler is auto-initialized via ServiceLocator — no setup needed.
	// To use a custom backend: profiler::ServiceLocator::RegisterProfiler(std::make_unique<MyProfiler>());

	PROFILER_BEGIN_SESSION("Example", "profiling/example");

	for (int frame = 0; frame < 3; ++frame)
	{
		PROFILER_SCOPE("Frame");
		ProcessData();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	PROFILER_END_SESSION();

	std::cout << "Profiling complete! Open 'profiling/example.json' in chrome://tracing\n";
	return 0;
}
