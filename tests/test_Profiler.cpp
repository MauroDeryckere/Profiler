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

TEST_F(ProfilerBaseTest, SetNumFramesToProfileRoundTrip)
{
	auto prof = std::make_unique<profiler::GoogleProfiler>();

	prof->SetNumFramesToProfile(42);
	EXPECT_EQ(prof->GetNumFramesToProfile(), 42u);
}

TEST_F(ProfilerBaseTest, StartAndUpdateAutoEndsSession)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER.SetNumFramesToProfile(3);
	PROFILER.Start((TEST_DIR + "/autoend").c_str());

	PROFILER.Update(); // frame 1
	PROFILER.Update(); // frame 2
	PROFILER.Update(); // frame 3 — should auto-end

	// File should exist after auto-end (filename is path + "0" + ".json")
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/autoend0.json"));
}

TEST_F(ProfilerBaseTest, StartWhileProfilingDoesNotCrash)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	PROFILER.SetNumFramesToProfile(100);
	PROFILER.Start((TEST_DIR + "/first").c_str());
	// Second Start while already profiling — should print warning but not crash
	PROFILER.Start((TEST_DIR + "/second").c_str());

	// Cleanup
	PROFILER.EndSession();
}


TEST_F(ProfilerBaseTest, StartWithCallbackReceivesJson)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.SetNumFramesToProfile(2);
	PROFILER.Start(nullptr, [&](std::string const& json) { captured = json; });

	PROFILER.Update(); // frame 1
	EXPECT_TRUE(captured.empty());

	PROFILER.Update(); // frame 2 — triggers callback
	EXPECT_FALSE(captured.empty());
	EXPECT_NE(captured.find("\"traceEvents\""), std::string::npos);
}

TEST_F(ProfilerBaseTest, StartWithFileAndCallbackBothWork)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.SetNumFramesToProfile(2);
	PROFILER.Start((TEST_DIR + "/combo").c_str(), [&](std::string const& json) { captured = json; });

	PROFILER.Update(); // frame 1
	PROFILER.Update(); // frame 2 — triggers callback + file write

	// Callback should have received JSON
	EXPECT_FALSE(captured.empty());
	EXPECT_NE(captured.find("\"traceEvents\""), std::string::npos);

	// File should also have been written
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/combo0.json"));
	std::ifstream file(TEST_DIR + "/combo0.json");
	std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	EXPECT_NE(content.find("\"traceEvents\""), std::string::npos);
}

TEST_F(ProfilerBaseTest, StartWithCallbackOnlyDoesNotCreateFile)
{
	profiler::ServiceLocator::RegisterProfiler(std::make_unique<profiler::GoogleProfiler>());

	std::string captured;
	PROFILER.SetNumFramesToProfile(2);
	PROFILER.Start(nullptr, [&](std::string const& json) { captured = json; });

	PROFILER.Update();
	PROFILER.Update();

	EXPECT_FALSE(captured.empty());
	EXPECT_TRUE(std::filesystem::is_empty(TEST_DIR));
}
