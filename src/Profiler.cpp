#include "Profiler/Profiler.h"

#include <cassert>
#include <cstdio>
#include <filesystem>

namespace profiler
{
	void Profiler::BeginSession(std::string const& name, std::string_view filepath, uint32_t maxFrames, FlushCallback callback)
	{
		EndSession();
		m_FileName = filepath;
		m_MaxFrames = maxFrames;
		m_ProfiledFrames = 0;
		m_FlushCallback = std::move(callback);
	}

	void Profiler::Tick()
	{
		if (m_MaxFrames == 0)
		{
			return;
		}

		++m_ProfiledFrames;

		if (m_ProfiledFrames >= m_MaxFrames)
		{
			if (m_FlushCallback)
			{
				m_FlushCallback(FlushToString());
				m_FlushCallback = nullptr;
			}

			EndSession();
		}
	}

	void Profiler::PrepareOutputPath(std::string_view filepath)
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
