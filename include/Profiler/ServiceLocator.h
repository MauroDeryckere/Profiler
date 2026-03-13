#ifndef PROFILER_SERVICE_LOCATOR_H
#define PROFILER_SERVICE_LOCATOR_H

#include "Profiler/Profiler.h"

#include <memory>

namespace profiler
{
	namespace ServiceLocator
	{
		/** @return Reference to the currently registered profiler backend. */
		[[nodiscard]] Profiler& GetProfiler() noexcept;

		/**
		 * Registers a profiler backend. Replaces any previously registered instance.
		 * @param profiler	The profiler backend to register. Ownership is transferred.
		 */
		void RegisterProfiler(std::unique_ptr<Profiler> profiler);
	}
}

#define PROFILER ::profiler::ServiceLocator::GetProfiler()

#endif
