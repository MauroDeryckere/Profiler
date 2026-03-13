#include "OptickProfiler.h"

#include <Optick.h>
#include <cassert>

namespace profiler
{
	void OptickProfiler::BeginSession(std::string const& name, char const* filepath)
	{
		assert(filepath && "OptickProfiler requires a file path — use BeginSession(name, filepath)");
		Profiler::BeginSession(name, filepath);
		OPTICK_START_CAPTURE()
	}

	void OptickProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		assert(false && "OptickProfiler uses OPTICK_EVENT macros — do not call WriteProfile directly");
	}

	void OptickProfiler::EndSession()
	{
		auto const path{ fileName + ".opt" };
		PrepareOutputPath(path.c_str());

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(path.c_str())
	}
}
