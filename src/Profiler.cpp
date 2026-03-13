#include "Profiler/Profiler.h"

#include <cassert>
#include <cstdio>
#include <filesystem>

namespace profiler
{
	void Profiler::BeginSession(std::string const& name, char const* path, uint32_t maxFrames, FlushCallback callback)
	{
		EndSession();
		m_FileName = path ? path : "";
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
