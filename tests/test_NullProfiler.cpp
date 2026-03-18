#include <gtest/gtest.h>
#include "NullProfiler.h"

TEST(NullProfiler, CanConstruct)
{
	profiler::NullProfiler p;
}

TEST(NullProfiler, WriteProfileIsNoOp)
{
	profiler::NullProfiler p;
	profiler::ProfileResult const result{ "test", 0, 100, std::this_thread::get_id() };
	p.WriteProfile(result, true);
	p.WriteProfile(result, false);
}

TEST(NullProfiler, BeginAndEndSessionAreNoOps)
{
	profiler::NullProfiler p;
	p.BeginSession("test", "dummy/path");
	p.EndSession();
}

TEST(NullProfiler, DoubleEndSessionIsSafe)
{
	profiler::NullProfiler p;
	p.BeginSession("test", "dummy/path");
	p.EndSession();
	p.EndSession();
}

TEST(NullProfiler, EndSessionWithoutBeginIsSafe)
{
	profiler::NullProfiler p;
	p.EndSession();
}

TEST(NullProfiler, FlushToStringReturnsEmpty)
{
	profiler::NullProfiler p;
	p.BeginSession("test");
	EXPECT_TRUE(p.FlushToString().empty());
	p.EndSession();
}
