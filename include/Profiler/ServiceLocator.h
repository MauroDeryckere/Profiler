#ifndef PROFILER_SERVICE_LOCATOR_H
#define PROFILER_SERVICE_LOCATOR_H

#include "Profiler/Profiler.h"

#include <memory>

namespace profiler
{
	namespace ServiceLocator
	{
		Profiler& GetProfiler();
		void RegisterProfiler(std::unique_ptr<Profiler> profiler);
	}
}

#define PROFILER ::profiler::ServiceLocator::GetProfiler()

#endif
