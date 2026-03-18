#ifndef PROFILER_SERVICE_LOCATOR_H
#define PROFILER_SERVICE_LOCATOR_H

#if defined(PROFILER_USE_OPTICK)
	#include "Profiler/OptickProfiler.h"
#else
	#include "Profiler/GoogleProfiler.h"
#endif

namespace profiler
{
	namespace ServiceLocator
	{
		/** @return Reference to the compile-time selected profiler backend. */
#if defined(PROFILER_USE_OPTICK)
		[[nodiscard]] OptickProfiler& GetProfiler() noexcept;
#else
		[[nodiscard]] GoogleProfiler& GetProfiler() noexcept;
#endif
	}
}

#define PROFILER ::profiler::ServiceLocator::GetProfiler()

#endif
