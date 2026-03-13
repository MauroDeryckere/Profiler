#ifndef PROFILER_NULL_PROFILER_H
#define PROFILER_NULL_PROFILER_H

#include "Profiler/Profiler.h"

namespace profiler
{
	/** No-op profiler backend. Used when profiling is disabled or not yet initialized. */
	class NullProfiler final : public Profiler
	{
	public:
		NullProfiler() = default;
		~NullProfiler() override = default;

		/** @see Profiler::WriteProfile */
		void WriteProfile(ProfileResult const& result, bool isFunction) override {}

		/** @see Profiler::EndSession */
		void EndSession() override {}

		/** @see Profiler::FlushToString */
		std::string FlushToString() override { return {}; }

		NullProfiler(NullProfiler const&) = delete;
		NullProfiler(NullProfiler&&) = delete;
		NullProfiler& operator=(NullProfiler const&) = delete;
		NullProfiler& operator=(NullProfiler&&) = delete;

	private:
		void BeginSessionInternal(std::string const& name, size_t reserveSize = 100'000) override {}
	};
}

#endif
