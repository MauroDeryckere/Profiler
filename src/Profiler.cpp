#include "Profiler/Profiler.h"

#include <cstdio>
#include <filesystem>

namespace profiler
{
	void Profiler::BeginSession(std::string const& name, char const* path, size_t reserveSize)
	{
		fileName = path;
		BeginSessionInternal(name, reserveSize);
	}

	void Profiler::BeginSession(std::string const& name, size_t reserveSize)
	{
		fileName.clear();
		BeginSessionInternal(name, reserveSize);
	}

	void Profiler::Start(char const* path)
	{
		if (isProfiling)
		{
			fprintf(stderr, "[Profiler] Already profiling: %s\n", fileName.c_str());
		}
		else
		{
			fileName = path;
			fileName += std::to_string(numExecutedProfiles);

			BeginSessionInternal(fileName);
			isProfiling = true;
		}
	}

	void Profiler::Update()
	{
		if (isProfiling)
		{
			++profiledFrames;
		}

		if (maxFrames > 0 && profiledFrames >= maxFrames)
		{
			++numExecutedProfiles;
			profiledFrames = 0;
			isProfiling = false;

			EndSession();
		}
	}

	void Profiler::PrepareOutputPath(char const* filepath)
	{
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
