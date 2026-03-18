#include "Profiler/ServiceLocator.h"

namespace profiler
{
#if defined(PROFILER_USE_OPTICK)
	OptickProfiler& ServiceLocator::GetProfiler() noexcept
	{
		static OptickProfiler instance;
		return instance;
	}
#else
	GoogleProfiler& ServiceLocator::GetProfiler() noexcept
	{
		static GoogleProfiler instance;
		return instance;
	}
#endif
}
