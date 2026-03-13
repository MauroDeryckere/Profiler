#include <gtest/gtest.h>
#include "Profiler/Profiler.h"
#include "Profiler/ServiceLocator.h"
#include "GoogleProfiler.h"

#include <filesystem>
#include <fstream>

namespace
{
	std::string const TEST_DIR = "test_profiler_output";

	class ProfilerBaseTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			std::filesystem::create_directories(TEST_DIR);
		}

		void TearDown() override
		{
			std::filesystem::remove_all(TEST_DIR);
		}
	};
}

TEST_F(ProfilerBaseTest, PrepareOutputPathCreatesDirectories)
{
	std::string nested = TEST_DIR + "/a/b/c/file.json";
	profiler::Profiler::PrepareOutputPath(nested.c_str());

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/a/b/c"));
}

TEST_F(ProfilerBaseTest, PrepareOutputPathRemovesExistingFile)
{
	std::string filePath = TEST_DIR + "/existing.json";

	// Create the file
	std::ofstream(filePath) << "dummy";
	ASSERT_TRUE(std::filesystem::exists(filePath));

	profiler::Profiler::PrepareOutputPath(filePath.c_str());
	EXPECT_FALSE(std::filesystem::exists(filePath));
}

TEST_F(ProfilerBaseTest, TickAutoEndsSession)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER.BeginSession("test", (TEST_DIR + "/autoend").c_str(), 3);

	PROFILER.Tick(); // frame 1
	PROFILER.Tick(); // frame 2
	PROFILER.Tick(); // frame 3 — should auto-end

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/autoend.json"));
}

TEST_F(ProfilerBaseTest, TickNoOpWithoutFrameLimit)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER.BeginSession("test", (TEST_DIR + "/manual").c_str());

	for (int i = 0; i < 100; ++i)
	{
		PROFILER.Tick();
	}

	// Session should still be active — manually end
	PROFILER.EndSession();
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/manual.json"));
}

TEST_F(ProfilerBaseTest, BeginSessionWhileActiveEndsFirst)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER.BeginSession("first", (TEST_DIR + "/first").c_str());
	PROFILER.BeginSession("second", (TEST_DIR + "/second").c_str());

	// First session should have been ended and written
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/first.json"));

	PROFILER.EndSession();
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/second.json"));
}

TEST_F(ProfilerBaseTest, TickWithCallbackReceivesJson)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.BeginSession("test", nullptr, 2, [&](std::string const& json) { captured = json; });

	PROFILER.Tick(); // frame 1
	EXPECT_TRUE(captured.empty());

	PROFILER.Tick(); // frame 2 — triggers callback
	EXPECT_FALSE(captured.empty());
	EXPECT_NE(captured.find("\"traceEvents\""), std::string::npos);
}

TEST_F(ProfilerBaseTest, TickWithFileAndCallbackBothWork)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.BeginSession("test", (TEST_DIR + "/combo").c_str(), 2, [&](std::string const& json) { captured = json; });

	PROFILER.Tick(); // frame 1
	PROFILER.Tick(); // frame 2 — triggers callback + file write

	EXPECT_FALSE(captured.empty());
	EXPECT_NE(captured.find("\"traceEvents\""), std::string::npos);

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/combo.json"));
	std::ifstream file(TEST_DIR + "/combo.json");
	std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	EXPECT_NE(content.find("\"traceEvents\""), std::string::npos);
}

TEST_F(ProfilerBaseTest, TickWithCallbackOnlyDoesNotCreateFile)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.BeginSession("test", nullptr, 2, [&](std::string const& json) { captured = json; });

	PROFILER.Tick();
	PROFILER.Tick();

	EXPECT_FALSE(captured.empty());
	EXPECT_TRUE(std::filesystem::is_empty(TEST_DIR));
}
