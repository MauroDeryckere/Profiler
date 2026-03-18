#include "Profiler/ServiceLocator.h"

namespace profiler
{
	namespace
	{
#if defined(PROFILER_USE_OPTICK)
		OptickProfiler g_Profiler;
#else
		GoogleProfiler g_Profiler;
#endif
	}

#if defined(PROFILER_USE_OPTICK)
	OptickProfiler& ServiceLocator::GetProfiler() noexcept
#else
	GoogleProfiler& ServiceLocator::GetProfiler() noexcept
#endif
	{
		return g_Profiler;
	}
}
