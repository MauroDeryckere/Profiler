#include "GoogleProfiler.h"

#include <cassert>
#include <cstdio>
#include <sstream>

namespace profiler
{
	GoogleProfiler::~GoogleProfiler()
	{
		EndSession();
	}

	void GoogleProfiler::BeginSessionInternal(std::string const& name, size_t reserveSize)
	{
		EndSession();
		fileName += ".json";

		EnsureDirectoryExists(fileName.c_str());

		m_OutputStream.open(fileName);

		if (!m_OutputStream.is_open())
		{
			fprintf(stderr, "[Profiler] Failed to open file: %s\n", fileName.c_str());
			return;
		}

		m_Buffer.clear();
		m_Buffer.reserve(reserveSize);
		m_BufferFlushThreshold = static_cast<size_t>(0.9f * static_cast<float>(reserveSize));
		m_BufferReserveSize = reserveSize;

		WriteHeader();

		m_CurrentSession = std::make_unique<InstrumentationSession>(InstrumentationSession{ name });
	}

	void GoogleProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		if (!m_CurrentSession)
		{
			return;
		}

		std::stringstream ss;
		ss << ",{";
		ss << R"("cat":")" << (isFunction ? "function" : "scope") << R"(",)";
		ss << R"("dur":)" << (result.end - result.start) << ",";
		ss << R"("name":")" << result.name << R"(",)";
		ss << R"("ph":"X",)";
		ss << R"("pid":0,)";
		ss << R"("tid":)" << std::hash<std::thread::id>{}(result.threadID) << ",";
		ss << R"("ts":)" << result.start;
		ss << "}";

		std::lock_guard lock(m_Mutex);
		m_Buffer += ss.str();

		if (m_Buffer.size() >= m_BufferFlushThreshold)
		{
			m_OutputStream << m_Buffer;
			m_Buffer.clear();
		}
	}

	void GoogleProfiler::WriteProfile(std::string const& name)
	{
		assert(false && "GoogleProfiler::WriteProfile(string) is not supported — use the ProfileResult overload");
	}

	void GoogleProfiler::EndSession()
	{
		if (m_CurrentSession)
		{
			WriteFooter();

			m_OutputStream << m_Buffer;
			m_OutputStream.close();

			m_CurrentSession = nullptr;
		}
	}

	void GoogleProfiler::WriteHeader()
	{
		m_OutputStream << R"({"otherData": {},"traceEvents":[{})";
		m_OutputStream.flush();
	}

	void GoogleProfiler::WriteFooter()
	{
		m_Buffer += "]}";
	}
}
