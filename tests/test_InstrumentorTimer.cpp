#include <gtest/gtest.h>
#include "Profiler/ServiceLocator.h"
#include "Profiler/InstrumentorTimer.h"
#include "Profiler/GoogleProfiler.h"

#include <filesystem>
#include <fstream>
#include <thread>

namespace
{
	std::string const TEST_DIR{ "test_timer_output" };

	[[nodiscard]] std::string ReadFileContents(std::string const& path) noexcept
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
		profiler::InstrumentorTimer const timer("SleepTest", false);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	PROFILER.EndSession();
	auto const content{ ReadFileContents(TEST_DIR + "/timer.json") };
	EXPECT_NE(content.find("\"SleepTest\""), std::string::npos);

	EXPECT_NE(content.find("\"ph\":\"X\""), std::string::npos);
	EXPECT_NE(content.find("\"dur\":"), std::string::npos);
}

TEST_F(InstrumentorTimerTest, ManualStopPreventsDoubleReport)
{
	{
		profiler::InstrumentorTimer timer("ManualStop", true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		timer.Stop();
		// Destructor should not emit a second event
	}

	PROFILER.EndSession();
	auto const content{ ReadFileContents(TEST_DIR + "/timer.json") };

	// Should appear exactly once (single X event, no duplicate from destructor)
	auto const first{ content.find("\"ManualStop\"") };
	ASSERT_NE(first, std::string::npos);
	EXPECT_EQ(content.find("\"ManualStop\"", first + 1), std::string::npos);
}

TEST_F(InstrumentorTimerTest, WritesToActiveProfiler)
{
	{
		profiler::InstrumentorTimer const timer("ActiveWrite", true);
	}

	PROFILER.EndSession();
	auto const content{ ReadFileContents(TEST_DIR + "/timer.json") };
	EXPECT_NE(content.find("\"ActiveWrite\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"function\""), std::string::npos);
}

TEST(InstrumentorTimerStandalone, TimerWithoutActiveSessionIsNoOp)
{
	// No session active — timer should not crash
	profiler::InstrumentorTimer timer("Orphan", false);
	timer.Stop();
}
