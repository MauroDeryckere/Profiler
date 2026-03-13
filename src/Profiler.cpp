#include "Profiler/Profiler.h"

#include <cassert>
#include <cstdio>
#include <filesystem>

namespace profiler
{
	void Profiler::BeginSession(std::string const& name, char const* path)
	{
		EndSession();
		fileName = path ? path : "";
	}

	void Profiler::Start(char const* path, FlushCallback callback)
	{
		assert((path || callback) && "Start requires a file path or a flush callback");
		if (isProfiling)
		{
			fprintf(stderr, "[Profiler] Already profiling: %s\n", fileName.c_str());
		}
		else
		{
			flushCallback = std::move(callback);

			std::string framePath;
			if (path)
			{
				framePath = path;
				framePath += std::to_string(numExecutedProfiles);
			}

			BeginSession("", framePath.empty() ? nullptr : framePath.c_str());
			isProfiling = true;
		}
	}

	void Profiler::Update()
	{
		if (isProfiling)
		{
			++profiledFrames;
		}

		if (profiledFrames >= numFramesToProfile)
		{
			++numExecutedProfiles;
			profiledFrames = 0;
			isProfiling = false;

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
