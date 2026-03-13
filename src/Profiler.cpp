#include "Profiler/Profiler.h"

#include <cassert>
#include <cstdio>
#include <filesystem>

namespace profiler
{
	void Profiler::BeginSession(std::string const& name, char const* path, uint32_t maxFrames, FlushCallback callback)
	{
		EndSession();
		fileName = path ? path : "";
		this->maxFrames = maxFrames;
		profiledFrames = 0;
		flushCallback = std::move(callback);
	}

	void Profiler::Tick()
	{
		if (maxFrames == 0)
		{
			return;
		}

		++profiledFrames;

		if (profiledFrames >= maxFrames)
		{
			if (flushCallback)
			{
				flushCallback(FlushToString());
				flushCallback = nullptr;
			}

			EndSession();
		}
	}

	void Profiler::PrepareOutputPath(char const* filepath)
	{
		assert(filepath && "PrepareOutputPath requires a non-null filepath");
		std::filesystem::path const dir{ std::filesystem::path(filepath).parent_path() };

		if (!dir.empty() && !std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		if (std::filesystem::exists(filepath))
		{
			std::filesystem::remove(filepath);
		}
	}
}
