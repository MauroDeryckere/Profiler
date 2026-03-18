#include <gtest/gtest.h>
#include "GoogleProfiler.h"
#include "Profiler/InstrumentorTimer.h"
#include "Profiler/ServiceLocator.h"

#include <stdint.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <numeric>
#include <print>
#include <string>
#include <thread>
#include <vector>

namespace
{
	uint32_t constexpr SAMPLE_COUNT{ 10 };

	[[nodiscard]] auto Now() noexcept { return std::chrono::high_resolution_clock::now(); }

	[[nodiscard]] double ElapsedUs(std::chrono::high_resolution_clock::time_point start,
								   std::chrono::high_resolution_clock::time_point end) noexcept
	{
		return std::chrono::duration<double, std::micro>(end - start).count();
	}

	// Runs a callable SAMPLE_COUNT times, discards the highest and lowest
	// samples, and returns the trimmed average in microseconds.
	[[nodiscard]] double TrimmedMeanUs(std::function<double()> const& fn) noexcept
	{
		std::vector<double> samples(SAMPLE_COUNT);
		for (uint32_t i{ 0 }; i < SAMPLE_COUNT; ++i)
		{
			samples[i] = fn();
		}

		std::sort(samples.begin(), samples.end());

		double const sum{ std::accumulate(samples.begin() + 1, samples.end() - 1, 0.0) };
		double const mean{ sum / (SAMPLE_COUNT - 2) };

		double const min{ samples.front() };
		double const max{ samples.back() };

		std::println("[{} samples, discarded min={:.1f} us / max={:.1f} us, averaged {}]", SAMPLE_COUNT, min, max, SAMPLE_COUNT - 2);

		return mean;
	}

	std::string const BENCH_DIR{ "bench_output" };

	class ProfilerBench : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			std::filesystem::create_directories(BENCH_DIR);
		}

		void TearDown() override
		{
			std::filesystem::remove_all(BENCH_DIR);
		}
	};
}

TEST_F(ProfilerBench, WriteProfileThroughput)
{
	uint32_t constexpr ITERATIONS{ 100'000 };

	double const avgUs{ TrimmedMeanUs([&]()
	{
		profiler::GoogleProfiler p;
		p.BeginSession("bench");
		profiler::ProfileResult result{ "BenchFunc", 0, 100, std::this_thread::get_id() };

		auto const start{ Now() };
		for (uint32_t i{ 0 }; i < ITERATIONS; ++i)
		{
			p.WriteProfile(result, true);
		}
		auto const end{ Now() };

		p.EndSession();
		return ElapsedUs(start, end);
	}) };

	std::println("WriteProfile: {} calls, avg {:.1f} ms ({:.0f} ns/call)", ITERATIONS, avgUs / 1000.0, (avgUs * 1000.0) / ITERATIONS);
}

TEST_F(ProfilerBench, WriteProfileMultiThreaded)
{
	uint32_t constexpr THREADS{ 4 };
	uint32_t constexpr ITERATIONS_PER_THREAD{ 25'000 };
	uint32_t constexpr totalCalls{ THREADS * ITERATIONS_PER_THREAD };

	double const avgUs{ TrimmedMeanUs([&]()
	{
		profiler::GoogleProfiler p;
		p.BeginSession("bench_mt");

		auto const start{ Now() };

		std::vector<std::thread> threads;
		for (uint32_t t{ 0 }; t < THREADS; ++t)
		{
			threads.emplace_back([&p, t, ITERATIONS_PER_THREAD]()
			{
				std::string const name{ std::format("Thread{}", t) };
				profiler::ProfileResult const result{ name, 0, 100, std::this_thread::get_id() };
				for (uint32_t i{ 0 }; i < ITERATIONS_PER_THREAD; ++i)
				{
					p.WriteProfile(result, true);
				}
			});
		}

		for (auto& t : threads)
		{
			t.join();
		}
		auto const end{ Now() };

		p.EndSession();
		return ElapsedUs(start, end);
	}) };

	std::println("WriteProfile ({} threads): {} calls, avg {:.1f} ms ({:.0f} ns/call)", THREADS, totalCalls, avgUs / 1000.0, (avgUs * 1000.0) / totalCalls);
}

TEST_F(ProfilerBench, SessionLifecycle)
{
	uint32_t constexpr ITERATIONS{ 1'000 };

	double const avgUs{ TrimmedMeanUs([&]()
	{
		auto const start{ Now() };
		for (uint32_t i{ 0 }; i < ITERATIONS; ++i)
		{
			profiler::GoogleProfiler p;
			p.BeginSession("bench", (BENCH_DIR + "/lifecycle").c_str());
			profiler::ProfileResult result{ "Func", 0, 100, std::this_thread::get_id() };
			p.WriteProfile(result, true);
			p.EndSession();
		}
		auto const end{ Now() };
		return ElapsedUs(start, end);
	}) };

	std::println("Begin+Write+End+File: {} cycles, avg {:.1f} ms ({:.0f} us/cycle)", ITERATIONS, avgUs / 1000.0, avgUs / ITERATIONS);
}

TEST_F(ProfilerBench, FlushToStringCost)
{
	uint32_t constexpr FLUSH_ITERATIONS{ 100 };
	uint32_t constexpr NUM_WRITE_PROFILES{ 10'000 };

	double const avgUs{ TrimmedMeanUs([&]()
	{
		profiler::GoogleProfiler p;
		p.BeginSession("bench");

		for (uint32_t i{ 0 }; i < NUM_WRITE_PROFILES; ++i)
		{
			profiler::ProfileResult result{ "Entry", 0, 100, std::this_thread::get_id() };
			p.WriteProfile(result, true);
		}

		auto const start{ Now() };
		for (uint32_t i{ 0 }; i < FLUSH_ITERATIONS; ++i)
		{
			auto const json{ p.FlushToString() };
			(void)json;
		}
		auto const end{ Now() };

		p.EndSession();
		return ElapsedUs(start, end);
	}) };

	std::println("FlushToString (10k events): {} calls, avg {:.1f} ms ({:.2f} ms/call)", FLUSH_ITERATIONS, avgUs / 1000.0, avgUs / 1000.0 / FLUSH_ITERATIONS);
}

TEST_F(ProfilerBench, InstrumentorTimerOverhead)
{
	uint32_t constexpr ITERATIONS{ 50'000 };

	double const avgUs{ TrimmedMeanUs([&]()
	{
		profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());
		PROFILER.BeginSession("bench");

		auto const start{ Now() };
		for (uint32_t i{ 0 }; i < ITERATIONS; ++i)
		{
			profiler::InstrumentorTimer timer("BenchTimer", true);
		}
		auto const end{ Now() };

		PROFILER.EndSession();
		return ElapsedUs(start, end);
	}) };

	std::println("InstrumentorTimer RAII: {} cycles, avg {:.1f} ms ({:.0f} ns/cycle)", ITERATIONS, avgUs / 1000.0, (avgUs * 1000.0) / ITERATIONS);
}
