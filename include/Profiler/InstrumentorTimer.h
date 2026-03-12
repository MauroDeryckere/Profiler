#ifndef PROFILER_INSTRUMENTOR_TIMER_H
#define PROFILER_INSTRUMENTOR_TIMER_H

#include <chrono>

namespace profiler
{
	/**
	 * RAII timer that emits B/E profile events on construction and destruction.
	 * Typically created via PROFILER_FUNCTION() or PROFILER_SCOPE() macros.
	 */
	class InstrumentorTimer final
	{
	public:
		/**
		 * Starts the timer and emits a begin event.
		 * @param timerName		Name shown in the trace viewer.
		 * @param isFunction	True if profiling a function, false for a named scope.
		 */
		explicit InstrumentorTimer(char const* timerName, bool isFunction);

		/** Stops the timer if not already stopped and emits an end event. */
		~InstrumentorTimer();

		/** Manually stops the timer early and emits an end event. Safe to call multiple times. */
		void Stop() noexcept;

		InstrumentorTimer(InstrumentorTimer const&) = delete;
		InstrumentorTimer(InstrumentorTimer&&) = delete;
		InstrumentorTimer& operator=(InstrumentorTimer const&) = delete;
		InstrumentorTimer& operator=(InstrumentorTimer&&) = delete;

	private:
		char const* m_Name{ "NO NAME" };
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartPoint{};
		bool m_IsStopped{ false };
		bool m_IsFunction{};
	};
}

#endif
