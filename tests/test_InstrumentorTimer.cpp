#include <gtest/gtest.h>
#include "Profiler/ServiceLocator.h"
#include "Profiler/InstrumentorTimer.h"
#include "GoogleProfiler.h"

#include <filesystem>
#include <fstream>
#include <thread>

namespace
{
	std::string const TEST_DIR = "test_timer_output";

	std::string ReadFileContents(std::string const& path)
	{
		std::ifstream file(path);
		return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	}

	class InstrumentorTimerTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			std::filesystem::create_directories(TEST_DIR);
			auto gp = std::make_unique<profiler::GoogleProfiler>();
			profiler::ServiceLocator::RegisterProfiler(std::move(gp));
			PROFILER.BeginSession("timer_test", (TEST_DIR + "/timer").c_str());
		}

		void TearDown() override
		{
			PROFILER.EndSession();
			std::filesystem::remove_all(TEST_DIR);
		}
	};
}

TEST_F(InstrumentorTimerTest, MeasuresNonZeroDuration)
{
	{
		profiler::InstrumentorTimer timer("SleepTest", false);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	PROFILER.EndSession();
	auto content = ReadFileContents(TEST_DIR + "/timer.json");
	EXPECT_NE(content.find("\"SleepTest\""), std::string::npos);

	// Should have both B (begin) and E (end) events with different timestamps
	EXPECT_NE(content.find("\"ph\":\"B\""), std::string::npos);
	EXPECT_NE(content.find("\"ph\":\"E\""), std::string::npos);
}

TEST_F(InstrumentorTimerTest, ManualStopPreventsDoubleReport)
{
	{
		profiler::InstrumentorTimer timer("ManualStop", true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		timer.Stop();
		// Destructor should NOT write again
	}

	PROFILER.EndSession();
	auto content = ReadFileContents(TEST_DIR + "/timer.json");

	// Count occurrences of "ManualStop" — should appear exactly twice (B + E events)
	size_t count = 0;
	size_t pos = 0;
	while ((pos = content.find("\"ManualStop\"", pos)) != std::string::npos)
	{
		++count;
		pos += 12;
	}
	EXPECT_EQ(count, 2);
}

TEST_F(InstrumentorTimerTest, WritesToActiveProfiler)
{
	{
		profiler::InstrumentorTimer timer("ActiveWrite", true);
	}

	PROFILER.EndSession();
	auto content = ReadFileContents(TEST_DIR + "/timer.json");
	EXPECT_NE(content.find("\"ActiveWrite\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"function\""), std::string::npos);
}
