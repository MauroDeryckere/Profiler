#include <gtest/gtest.h>
#include "Profiler/ProfilerMacros.h"
#include "Profiler/ServiceLocator.h"
#include "Profiler/GoogleProfiler.h"

#include <filesystem>
#include <fstream>

namespace
{
	std::string const TEST_DIR{ "test_macros_output" };

	[[nodiscard]] std::string ReadFileContents(std::string const& path) noexcept
	{
		std::ifstream file(path);
		return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	}

	class MacrosTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			std::filesystem::create_directories(TEST_DIR);
		}

		void TearDown() override
		{
			PROFILER.EndSession();
			std::filesystem::remove_all(TEST_DIR);
		}
	};
}

void ProfiledFunction()
{
	PROFILER_FUNCTION();
	volatile uint32_t x{ 0 };
	for (uint32_t i{ 0 }; i < 1000; ++i)
	{
		x += i;
	}
}

TEST_F(MacrosTest, ProfilerFunctionProducesEntry)
{
	PROFILER_BEGIN_SESSION("macro_func", (TEST_DIR + "/func").c_str());
	ProfiledFunction();
	PROFILER_END_SESSION();

	auto const content{ ReadFileContents(TEST_DIR + "/func.json") };
	EXPECT_NE(content.find("\"ProfiledFunction\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"function\""), std::string::npos);
}

TEST_F(MacrosTest, ProfilerScopeProducesEntry)
{
	PROFILER_BEGIN_SESSION("macro_scope", (TEST_DIR + "/scope").c_str());

	{
		PROFILER_SCOPE("TestScope");
		volatile uint32_t x{ 0 };
		for (uint32_t i{ 0 }; i < 1000; ++i)
		{
			x += i;
		}
	}

	PROFILER_END_SESSION();

	auto const content{ ReadFileContents(TEST_DIR + "/scope.json") };
	EXPECT_NE(content.find("\"TestScope\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"scope\""), std::string::npos);
}

TEST_F(MacrosTest, BeginAndEndSessionLifecycle)
{
	PROFILER_BEGIN_SESSION("lifecycle", (TEST_DIR + "/lifecycle").c_str());
	PROFILER_END_SESSION();

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/lifecycle.json"));
}

TEST_F(MacrosTest, NestedScopesProduceMultipleEntries)
{
	PROFILER_BEGIN_SESSION("nested", (TEST_DIR + "/nested").c_str());

	{
		PROFILER_SCOPE("Outer");
		{
			PROFILER_SCOPE("Inner");
			volatile uint32_t x{ 42 };
			(void)x;
		}
	}

	PROFILER_END_SESSION();

	auto const content{ ReadFileContents(TEST_DIR + "/nested.json") };
	EXPECT_NE(content.find("\"Outer\""), std::string::npos);
	EXPECT_NE(content.find("\"Inner\""), std::string::npos);
}

TEST_F(MacrosTest, ProfilerThreadNamesThread)
{
	PROFILER_BEGIN_SESSION("thread", (TEST_DIR + "/thread").c_str());

	PROFILER_THREAD("MainThread");
	{
		PROFILER_SCOPE("Entry");
	}

	PROFILER_END_SESSION();

	auto const content{ ReadFileContents(TEST_DIR + "/thread.json") };
	EXPECT_NE(content.find("\"MainThread\""), std::string::npos);
}

TEST_F(MacrosTest, ProfilerFrameNamesThread)
{
	PROFILER_BEGIN_SESSION("frame", (TEST_DIR + "/frame").c_str());

	PROFILER_FRAME("GameThread");
	{
		PROFILER_SCOPE("Entry");
	}

	PROFILER_END_SESSION();

	auto const content{ ReadFileContents(TEST_DIR + "/frame.json") };
	EXPECT_NE(content.find("\"GameThread\""), std::string::npos);
}

TEST_F(MacrosTest, ProfilerTickAdvancesFrameCounter)
{
	PROFILER_BEGIN_SESSION("tick", (TEST_DIR + "/tick").c_str(), 2);

	PROFILER_SCOPE("Frame1");
	PROFILER_TICK();
	PROFILER_TICK(); // should auto-end

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/tick.json"));
}
