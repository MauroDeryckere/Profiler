#include "OptickProfiler.h"

#include <Optick.h>
#include <cassert>

namespace profiler
{
	void OptickProfiler::BeginSessionInternal(std::string const& name, size_t reserveSize)
	{
		fileName += ".opt";

		OPTICK_START_CAPTURE()
	}

	void OptickProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		assert(false && "OptickProfiler uses OPTICK_EVENT macros — do not call WriteProfile directly");
	}

	void OptickProfiler::WriteProfile(std::string const& name)
	{
		assert(false && "OptickProfiler uses OPTICK_EVENT macros — do not call WriteProfile directly");
	}

	void OptickProfiler::EndSession()
	{
		EnsureDirectoryExists(fileName.c_str());

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(fileName.c_str())
	}
}
