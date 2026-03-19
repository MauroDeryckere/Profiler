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
	class OptickProfiler final : public Profiler<OptickProfiler>
	{
	public:
		OptickProfiler() = default;
		~OptickProfiler() = default;

		void BeginSession(std::string const& name, std::string_view filepath = {}, uint32_t maxFrames = 0, FlushCallback callback = nullptr);
		void WriteProfile(ProfileResult const& result, bool isFunction);
		void EndSession();
		[[nodiscard]] bool IsSessionActive() const noexcept { return m_Active; }
		[[nodiscard]] std::string FlushToString() const { return {}; }

		OptickProfiler(OptickProfiler const&) = delete;
		OptickProfiler(OptickProfiler&&) = delete;
		OptickProfiler& operator=(OptickProfiler const&) = delete;
		OptickProfiler& operator=(OptickProfiler&&) = delete;

	private:
		bool m_Active{ false };
	};
}

#endif
