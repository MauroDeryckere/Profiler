#include "Profiler/ServiceLocator.h"
#include "NullProfiler.h"
#include "GoogleProfiler.h"

#if defined(PROFILER_USE_OPTICK)
	#include "OptickProfiler.h"
#endif

namespace profiler
{
	namespace
	{
#if defined(PROFILER_ENABLED)
	#if defined(PROFILER_USE_OPTICK)
		std::unique_ptr<Profiler> g_pProfiler{ std::make_unique<OptickProfiler>() };
	#else
		std::unique_ptr<Profiler> g_pProfiler{ std::make_unique<GoogleProfiler>() };
	#endif
#else
		std::unique_ptr<Profiler> g_pProfiler{ std::make_unique<NullProfiler>() };
#endif
	}

	Profiler& ServiceLocator::GetProfiler()
	{
		return *g_pProfiler;
	}

	void ServiceLocator::RegisterProfiler(std::unique_ptr<Profiler> profiler)
	{
		g_pProfiler = profiler ? std::move(profiler) : std::make_unique<NullProfiler>();
	}
}
