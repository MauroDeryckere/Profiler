#include "Profiler/Profiler.h"

#include <cassert>
#include <filesystem>

namespace profiler
{
	void PrepareOutputPath(std::string_view filepath)
	{
		assert(!filepath.empty() && "PrepareOutputPath requires a non-empty filepath");
		std::filesystem::path const fsPath{ filepath };
		std::filesystem::path const dir{ fsPath.parent_path() };

		if (!dir.empty() && !std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		if (std::filesystem::exists(fsPath))
		{
			std::filesystem::remove(fsPath);
		}
	}
}
