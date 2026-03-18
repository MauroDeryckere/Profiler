#include "Profiler/OptickProfiler.h"

#include <Optick.h>
#include <cassert>

namespace profiler
{
	void OptickProfiler::BeginSession(std::string const& name, std::string_view filepath, uint32_t maxFrames, FlushCallback callback)
	{
		assert(!filepath.empty() && "OptickProfiler requires a file path — use BeginSession(name, filepath)");
		Profiler<OptickProfiler>::BeginSession(name, filepath, maxFrames, std::move(callback));
		m_Active = true;
		OPTICK_START_CAPTURE()
	}

	void OptickProfiler::WriteProfile([[maybe_unused]] ProfileResult const& result, [[maybe_unused]] bool isFunction)
	{
		assert(false && "OptickProfiler uses OPTICK_EVENT macros — do not call WriteProfile directly");
	}

	void OptickProfiler::EndSession()
	{
		if (!m_Active)
		{
			return;
		}

		m_Active = false;

		auto const path{ GetFileName() + ".opt" };
		PrepareOutputPath(path);

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(path.c_str())
	}
}
