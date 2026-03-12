#include <gtest/gtest.h>
#include "GoogleProfiler.h"

#include <filesystem>
#include <fstream>
#include <thread>

namespace
{
	std::string const TEST_DIR = "test_output";

	std::string ReadFileContents(std::string const& path)
	{
		std::ifstream file(path);
		return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	}

	class GoogleProfilerTest : public ::testing::Test
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

TEST_F(GoogleProfilerTest, CreatesJsonFile)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());
	p.EndSession();

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/output.json"));
}

TEST_F(GoogleProfilerTest, OutputContainsValidJsonStructure)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());
	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"traceEvents\""), std::string::npos);
	EXPECT_EQ(content.front(), '{');
	EXPECT_EQ(content.back(), '}');
}

TEST_F(GoogleProfilerTest, WriteProfileProducesEntry)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult result{ "MyFunction", 1000, 2000, std::this_thread::get_id() };
	p.WriteProfile(result, true);

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"MyFunction\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"function\""), std::string::npos);
	EXPECT_NE(content.find("\"ph\":\"B\""), std::string::npos);
	EXPECT_NE(content.find("\"ph\":\"E\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, ScopeCategory)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult result{ "MyScope", 0, 500, std::this_thread::get_id() };
	p.WriteProfile(result, false);

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"cat\":\"scope\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, MultipleEntriesAreSeparated)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult r1{ "First", 0, 100, std::this_thread::get_id() };
	profiler::ProfileResult r2{ "Second", 100, 200, std::this_thread::get_id() };
	p.WriteProfile(r1, true);
	p.WriteProfile(r2, true);

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"First\""), std::string::npos);
	EXPECT_NE(content.find("\"Second\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, ThreadIdIsCaptured)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult result{ "Threaded", 0, 100, std::this_thread::get_id() };
	p.WriteProfile(result, true);

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"tid\":1"), std::string::npos);
}

TEST_F(GoogleProfilerTest, MultiThreadedWritesAreSafe)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	auto writeFromThread = [&p](std::string const& name)
	{
		for (int i = 0; i < 100; ++i)
		{
			profiler::ProfileResult result{ name + std::to_string(i), 0, 100, std::this_thread::get_id() };
			p.WriteProfile(result, true);
		}
	};

	std::thread t1(writeFromThread, "ThreadA_");
	std::thread t2(writeFromThread, "ThreadB_");
	t1.join();
	t2.join();

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	EXPECT_NE(content.find("\"ThreadA_0\""), std::string::npos);
	EXPECT_NE(content.find("\"ThreadB_0\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, DifferentThreadsProduceDifferentTids)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult mainResult{ "MainEntry", 0, 100, std::this_thread::get_id() };
	p.WriteProfile(mainResult, true);

	std::thread worker([&]()
	{
		profiler::ProfileResult workerResult{ "WorkerEntry", 0, 100, std::this_thread::get_id() };
		p.WriteProfile(workerResult, true);
	});
	worker.join();

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	// Main thread gets tid 1, worker gets tid 2
	EXPECT_NE(content.find("\"tid\":1"), std::string::npos);
	EXPECT_NE(content.find("\"tid\":2"), std::string::npos);
}

TEST_F(GoogleProfilerTest, EndSessionWithoutBeginIsSafe)
{
	profiler::GoogleProfiler p;
	p.EndSession();
}

TEST_F(GoogleProfilerTest, DoubleEndSessionIsSafe)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());
	p.EndSession();
	p.EndSession();
}

TEST_F(GoogleProfilerTest, NoEmptyEventInOutput)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult result{ "Entry", 0, 100, std::this_thread::get_id() };
	p.WriteProfile(result, true);

	p.EndSession();

	auto content = ReadFileContents(TEST_DIR + "/output.json");
	// Should start with [{ not [{},  (no empty event)
	EXPECT_EQ(content.find("[{}"), std::string::npos);
}
