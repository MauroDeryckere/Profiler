#ifndef PROFILER_INSTRUMENTOR_TIMER_H
#define PROFILER_INSTRUMENTOR_TIMER_H

#include <chrono>

namespace profiler
{
	class InstrumentorTimer final
	{
	public:
		explicit InstrumentorTimer(char const* timerName, bool isFunction);
		~InstrumentorTimer();

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
