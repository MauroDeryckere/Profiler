#include "Profiler/OptickProfiler.h"

#include <Optick.h>
#include <cassert>

namespace profiler
{
	void OptickProfiler::BeginSession(std::string const& name, std::string_view filepath, uint32_t maxFrames, FlushCallback callback)
	{
		assert(!filepath.empty() && "OptickProfiler requires a file path — use BeginSession(name, filepath)");
		Profiler<OptickProfiler>::BeginSession(name, filepath, maxFrames, std::move(callback));
		OPTICK_START_CAPTURE()
	}

	void OptickProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		assert(false && "OptickProfiler uses OPTICK_EVENT macros — do not call WriteProfile directly");
	}

	void OptickProfiler::EndSession()
	{
		auto const path{ GetFileName() + ".opt" };
		PrepareOutputPath(path);

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(path.c_str())
	}
}
