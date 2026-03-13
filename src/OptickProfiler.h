#ifndef PROFILER_OPTICK_PROFILER_H
#define PROFILER_OPTICK_PROFILER_H

#include "Profiler/Profiler.h"

namespace profiler
{
	/**
	 * Optick profiling backend.
	 * Requires the Optick library to be available (linked via CMake).
	 * Uses OPTICK_EVENT / OPTICK_FRAME macros for instrumentation.
	 */
	class OptickProfiler final : public Profiler
	{
	public:
		OptickProfiler() = default;
		~OptickProfiler() override = default;

		/** @see Profiler::BeginSession */
		void BeginSession(std::string const& name, char const* filepath = nullptr) override;

		/** @see Profiler::WriteProfile */
		void WriteProfile(ProfileResult const& result, bool isFunction) override;

		/** @see Profiler::EndSession */
		void EndSession() override;

		/** @see Profiler::FlushToString */
		std::string FlushToString() override { return {}; }

		OptickProfiler(OptickProfiler const&) = delete;
		OptickProfiler(OptickProfiler&&) = delete;
		OptickProfiler& operator=(OptickProfiler const&) = delete;
		OptickProfiler& operator=(OptickProfiler&&) = delete;
	};
}

#endif
