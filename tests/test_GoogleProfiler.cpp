#include <gtest/gtest.h>
#include "Profiler/GoogleProfiler.h"

#include <filesystem>
#include <fstream>
#include <thread>

namespace
{
	std::string const TEST_DIR{ "test_output" };

	[[nodiscard]] std::string ReadFileContents(std::string const& path) noexcept
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

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"traceEvents\""), std::string::npos);
	EXPECT_EQ(content.front(), '{');
	EXPECT_EQ(content.back(), '}');
}

TEST_F(GoogleProfilerTest, WriteProfileProducesEntry)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "MyFunction", 1000, 2000};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"MyFunction\""), std::string::npos);
	EXPECT_NE(content.find("\"cat\":\"function\""), std::string::npos);
	EXPECT_NE(content.find("\"ph\":\"X\""), std::string::npos);
	EXPECT_NE(content.find("\"dur\":"), std::string::npos);
}

TEST_F(GoogleProfilerTest, ScopeCategory)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "MyScope", 0, 500};
	p.WriteProfile(result, false);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"cat\":\"scope\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, MultipleEntriesAreSeparated)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const r1{ "First", 0, 100};
	profiler::ProfileResult const r2{ "Second", 100, 200};
	p.WriteProfile(r1, true);
	p.WriteProfile(r2, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"First\""), std::string::npos);
	EXPECT_NE(content.find("\"Second\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, ThreadIdIsCaptured)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "Threaded", 0, 100};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"tid\":1"), std::string::npos);
}

TEST_F(GoogleProfilerTest, MultiThreadedWritesAreSafe)
{
	uint32_t constexpr NUM_MULTITHREAD_WRITES{ 100 };

	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	auto writeFromThread{ [&p](char const* name)
	{
		for (uint32_t i{ 0 }; i < NUM_MULTITHREAD_WRITES; ++i)
		{
			profiler::ProfileResult const result{ name, 0, 100};
			p.WriteProfile(result, true);
		}
	} };

	std::thread t1(writeFromThread, "ThreadA");
	std::thread t2(writeFromThread, "ThreadB");
	t1.join();
	t2.join();

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"ThreadA\""), std::string::npos);
	EXPECT_NE(content.find("\"ThreadB\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, DifferentThreadsProduceDifferentTids)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const mainResult{ "MainEntry", 0, 100};
	p.WriteProfile(mainResult, true);

	std::thread worker([&]()
	{
		profiler::ProfileResult const workerResult{ "WorkerEntry", 0, 100};
		p.WriteProfile(workerResult, true);
	});
	worker.join();

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
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

	profiler::ProfileResult const result{ "Entry", 0, 100};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	// Should start with [{ not [{},  (no empty event)
	EXPECT_EQ(content.find("[{}"), std::string::npos);
}

TEST_F(GoogleProfilerTest, FlushToStringReturnsValidJson)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "Flushed", 0, 500};
	p.WriteProfile(result, true);

	auto const json{ p.FlushToString() };
	EXPECT_FALSE(json.empty());
	EXPECT_EQ(json.front(), '{');
	EXPECT_EQ(json.back(), '}');
	EXPECT_NE(json.find("\"Flushed\""), std::string::npos);
	EXPECT_NE(json.find("\"traceEvents\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, FlushToStringWithoutSessionReturnsEmpty)
{
	profiler::GoogleProfiler p;
	auto const json{ p.FlushToString() };
	EXPECT_TRUE(json.empty());
}

TEST_F(GoogleProfilerTest, FlushToStringIsReadOnly)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "Entry", 0, 100};
	p.WriteProfile(result, true);

	auto const json1{ p.FlushToString() };
	EXPECT_FALSE(json1.empty());

	// FlushToString is read-only — calling it again returns the same data
	auto const json2{ p.FlushToString() };
	EXPECT_EQ(json1, json2);

	p.EndSession();
}

TEST_F(GoogleProfilerTest, FlushToStringMidSessionReturnsSnapshot)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const r1{ "First", 0, 100};
	p.WriteProfile(r1, true);

	auto const snapshot{ p.FlushToString() };
	EXPECT_NE(snapshot.find("\"First\""), std::string::npos);

	// Write more data after snapshot
	profiler::ProfileResult const r2{ "Second", 100, 200};
	p.WriteProfile(r2, true);

	auto const full{ p.FlushToString() };
	EXPECT_NE(full.find("\"First\""), std::string::npos);
	EXPECT_NE(full.find("\"Second\""), std::string::npos);

	p.EndSession();
}

TEST_F(GoogleProfilerTest, BeginSessionWithoutFileDoesNotCreateFile)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test");

	profiler::ProfileResult const result{ "NoFile", 0, 100};
	p.WriteProfile(result, true);

	auto const json{ p.FlushToString() };
	EXPECT_NE(json.find("\"NoFile\""), std::string::npos);

	p.EndSession();

	// No file should have been created anywhere in test dir
	EXPECT_TRUE(std::filesystem::is_empty(TEST_DIR));
}

TEST_F(GoogleProfilerTest, EndSessionWritesFileAfterFlushToString)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "Both", 0, 100};
	p.WriteProfile(result, true);

	auto const json{ p.FlushToString() };
	EXPECT_NE(json.find("\"Both\""), std::string::npos);

	p.EndSession();

	// File should also have been written
	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"Both\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, SetThreadNameAppearsInOutput)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	p.SetThreadName("MainThread");
	profiler::ProfileResult const result{ "Entry", 0, 100};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"MainThread\""), std::string::npos);
	// Should NOT contain the default "Thread 1" name
	EXPECT_EQ(content.find("\"Thread 1\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, DefaultThreadNameUsedWhenNotSet)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	profiler::ProfileResult const result{ "Entry", 0, 100};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"Thread 1\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, SetThreadNamePerThread)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	p.SetThreadName("Main");
	profiler::ProfileResult const result{ "MainEntry", 0, 100};
	p.WriteProfile(result, true);

	std::thread worker([&]()
	{
		p.SetThreadName("Worker");
		profiler::ProfileResult const workerResult{ "WorkerEntry", 0, 100};
		p.WriteProfile(workerResult, true);
	});
	worker.join();

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"Main\""), std::string::npos);
	EXPECT_NE(content.find("\"Worker\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, MarkFrameNamesThread)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	p.MarkFrame("MainThread");
	profiler::ProfileResult const result{ "Entry", 0, 100};
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"MainThread\""), std::string::npos);
	EXPECT_EQ(content.find("\"Thread 1\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, SetThreadNameWithoutSessionIsSafe)
{
	profiler::GoogleProfiler p;
	p.SetThreadName("Orphan");
}

TEST_F(GoogleProfilerTest, MarkFrameWithoutSessionIsSafe)
{
	profiler::GoogleProfiler p;
	p.MarkFrame("Orphan");
}

TEST_F(GoogleProfilerTest, BeginSessionWhileActiveEndsFirst)
{
	profiler::GoogleProfiler p;
	p.BeginSession("first", (TEST_DIR + "/first").c_str());

	profiler::ProfileResult const r1{ "FirstEntry", 0, 100 };
	p.WriteProfile(r1, true);

	p.BeginSession("second", (TEST_DIR + "/second").c_str());

	// First session should have been ended and written
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/first.json"));
	auto const content{ ReadFileContents(TEST_DIR + "/first.json") };
	EXPECT_NE(content.find("\"FirstEntry\""), std::string::npos);

	p.EndSession();
	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/second.json"));
}

TEST_F(GoogleProfilerTest, WriteProfileWhenInactiveIsNoOp)
{
	profiler::GoogleProfiler p;

	profiler::ProfileResult const result{ "Ghost", 0, 100 };
	p.WriteProfile(result, true);

	p.BeginSession("test", (TEST_DIR + "/output").c_str());
	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_EQ(content.find("\"Ghost\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, DestructorEndsActiveSession)
{
	{
		profiler::GoogleProfiler p;
		p.BeginSession("test", (TEST_DIR + "/dtor").c_str());

		profiler::ProfileResult const result{ "BeforeDtor", 0, 100 };
		p.WriteProfile(result, true);
	} // destructor should call EndSession and write file

	EXPECT_TRUE(std::filesystem::exists(TEST_DIR + "/dtor.json"));
	auto const content{ ReadFileContents(TEST_DIR + "/dtor.json") };
	EXPECT_NE(content.find("\"BeforeDtor\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, FlushToStringAfterEndSessionReturnsEmpty)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test");

	profiler::ProfileResult const result{ "Entry", 0, 100 };
	p.WriteProfile(result, true);

	p.EndSession();

	auto const json{ p.FlushToString() };
	EXPECT_TRUE(json.empty());
}

TEST_F(GoogleProfilerTest, SetThreadNameOverwrite)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test", (TEST_DIR + "/output").c_str());

	p.SetThreadName("OldName");
	p.SetThreadName("NewName");

	profiler::ProfileResult const result{ "Entry", 0, 100 };
	p.WriteProfile(result, true);

	p.EndSession();

	auto const content{ ReadFileContents(TEST_DIR + "/output.json") };
	EXPECT_NE(content.find("\"NewName\""), std::string::npos);
	EXPECT_EQ(content.find("\"OldName\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, SessionReuse)
{
	profiler::GoogleProfiler p;

	p.BeginSession("first", (TEST_DIR + "/first").c_str());
	profiler::ProfileResult const r1{ "FirstEntry", 0, 100 };
	p.WriteProfile(r1, true);
	p.EndSession();

	p.BeginSession("second", (TEST_DIR + "/second").c_str());
	profiler::ProfileResult const r2{ "SecondEntry", 0, 100 };
	p.WriteProfile(r2, true);
	p.EndSession();

	auto const first{ ReadFileContents(TEST_DIR + "/first.json") };
	EXPECT_NE(first.find("\"FirstEntry\""), std::string::npos);
	EXPECT_EQ(first.find("\"SecondEntry\""), std::string::npos);

	auto const second{ ReadFileContents(TEST_DIR + "/second.json") };
	EXPECT_NE(second.find("\"SecondEntry\""), std::string::npos);
	EXPECT_EQ(second.find("\"FirstEntry\""), std::string::npos);
}

TEST_F(GoogleProfilerTest, FlushToStringContainsMultiThreadData)
{
	profiler::GoogleProfiler p;
	p.BeginSession("test");

	std::thread t1([&]()
	{
		profiler::ProfileResult const result{ "ThreadA", 0, 100 };
		p.WriteProfile(result, true);
	});
	std::thread t2([&]()
	{
		profiler::ProfileResult const result{ "ThreadB", 0, 100 };
		p.WriteProfile(result, true);
	});
	t1.join();
	t2.join();

	auto const json{ p.FlushToString() };
	EXPECT_NE(json.find("\"ThreadA\""), std::string::npos);
	EXPECT_NE(json.find("\"ThreadB\""), std::string::npos);

	p.EndSession();
}
