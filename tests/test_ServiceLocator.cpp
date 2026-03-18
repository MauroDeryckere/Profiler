#include <gtest/gtest.h>
#include "Profiler/ServiceLocator.h"

TEST(ServiceLocator, GetProfilerReturnsValidReference)
{
	auto& prof{ profiler::ServiceLocator::GetProfiler() };
	// Should not crash — just verify we get a reference
	SUCCEED();
}

TEST(ServiceLocator, ProfilerMacroMatchesGetProfiler)
{
	auto& fromGetter = profiler::ServiceLocator::GetProfiler();
	auto& fromMacro = PROFILER;
	EXPECT_EQ(&fromGetter, &fromMacro);
}
