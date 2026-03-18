#ifndef PROFILER_INSTRUMENTOR_TIMER_H
#define PROFILER_INSTRUMENTOR_TIMER_H

#include <chrono>

namespace profiler
{
	/**
	 * RAII timer that records start time on construction and emits a complete (X)
	 * profile event on destruction. Typically created via PROFILER_FUNCTION() or
	 * PROFILER_SCOPE() macros.
	 */
	class InstrumentorTimer final
	{
	public:
		/**
		 * Starts the timer. The complete profile event is emitted when Stop() is called.
		 * @param timerName		Name shown in the trace viewer.
		 * @param isFunction	True if profiling a function, false for a named scope.
		 */
		explicit InstrumentorTimer(char const* timerName, bool isFunction);

		/** Stops the timer if not already stopped and emits the complete profile event. */
		~InstrumentorTimer();

		/** Manually stops the timer early and emits the complete profile event. Safe to call multiple times. */
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
