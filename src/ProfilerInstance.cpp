#include "Profiler/ProfilerInstance.h"
#include "Profiler/Profiler.h"
#include "NullProfiler.h"
#include "GoogleProfiler.h"

#if defined(PROFILER_USE_OPTICK)
	#include "OptickProfiler.h"
#endif

#include <memory>

namespace profiler
{
	namespace
	{
		std::unique_ptr<Profiler>& GetInstance()
		{
			static std::unique_ptr<Profiler> instance = std::make_unique<NullProfiler>();
			return instance;
		}
	}

	Profiler& ProfilerInstance::Get()
	{
		return *GetInstance();
	}

	void ProfilerInstance::Initialize()
	{
#if defined(PROFILER_USE_OPTICK)
		Set(std::make_unique<OptickProfiler>());
#else
		Set(std::make_unique<GoogleProfiler>());
#endif
	}

	void ProfilerInstance::Set(std::unique_ptr<Profiler> profiler)
	{
		GetInstance() = std::move(profiler);
	}

	void ProfilerInstance::Shutdown()
	{
		GetInstance() = std::make_unique<NullProfiler>();
	}
}
