#ifndef PROFILER_NULL_PROFILER_H
#define PROFILER_NULL_PROFILER_H

#include "Profiler/Profiler.h"

namespace profiler
{
	/** No-op profiler backend. Used when profiling is disabled or not yet initialized. */
	class NullProfiler final : public Profiler<NullProfiler>
	{
	public:
		NullProfiler() = default;
		~NullProfiler() = default;

		void WriteProfile(ProfileResult const& /*result*/, bool /*isFunction*/) {}
		void EndSession() {}
		[[nodiscard]] std::string FlushToString() const { return {}; }

		NullProfiler(NullProfiler const&) = delete;
		NullProfiler(NullProfiler&&) = delete;
		NullProfiler& operator=(NullProfiler const&) = delete;
		NullProfiler& operator=(NullProfiler&&) = delete;
	};
}

#endif
