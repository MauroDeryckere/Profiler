#include "Profiler/InstrumentorTimer.h"
#include "Profiler/ServiceLocator.h"

namespace profiler
{
	InstrumentorTimer::InstrumentorTimer(std::string_view timerName, bool isFunction) :
		m_Name{ timerName },
		m_StartPoint{ std::chrono::high_resolution_clock::now() },
		m_IsStopped{ false },
		m_IsFunction{ isFunction } { }

	InstrumentorTimer::~InstrumentorTimer()
	{
		if (!m_IsStopped)
		{
			Stop();
		}
	}

	void InstrumentorTimer::Stop() noexcept
	{
		auto const endPoint{ std::chrono::high_resolution_clock::now() };
		auto const start{ std::chrono::time_point_cast<std::chrono::microseconds>(m_StartPoint).time_since_epoch().count() };
		auto const end{ std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count() };

		m_IsStopped = true;

		PROFILER.WriteProfile({ m_Name, start, end }, m_IsFunction);
	}
}
