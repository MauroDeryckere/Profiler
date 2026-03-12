#include <gtest/gtest.h>
#include "Profiler/ServiceLocator.h"
#include "NullProfiler.h"

TEST(ServiceLocator, GetProfilerReturnsValidReference)
{
	auto& prof = profiler::ServiceLocator::GetProfiler();
	// Should not crash — just verify we get a reference
	SUCCEED();
}

TEST(ServiceLocator, ProfilerMacroMatchesGetProfiler)
{
	auto& fromGetter = profiler::ServiceLocator::GetProfiler();
	auto& fromMacro = PROFILER;
	EXPECT_EQ(&fromGetter, &fromMacro);
}

TEST(ServiceLocator, RegisterProfilerSwapsBackend)
{
	auto& before = profiler::ServiceLocator::GetProfiler();
	auto custom = std::make_unique<profiler::NullProfiler>();
	auto* customPtr = custom.get();

	profiler::ServiceLocator::RegisterProfiler(std::move(custom));
	auto& after = profiler::ServiceLocator::GetProfiler();

	EXPECT_EQ(&after, customPtr);
	EXPECT_NE(&before, &after);
}

TEST(ServiceLocator, RegisterNullptrFallsBackToNullProfiler)
{
	profiler::ServiceLocator::RegisterProfiler(nullptr);
	auto& prof = profiler::ServiceLocator::GetProfiler();

	// Should not crash — we get a valid NullProfiler
	prof.EndSession();
	SUCCEED();
}
