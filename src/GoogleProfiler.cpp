#include "GoogleProfiler.h"

#include <cstdio>

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

		PrepareOutputPath(fileName.c_str());

		m_OutputStream.open(fileName);

		if (!m_OutputStream.is_open())
		{
			fprintf(stderr, "[Profiler] Failed to open file: %s\n", fileName.c_str());
			return;
		}

		m_Buffer.clear();
		m_Buffer.reserve(reserveSize);
		m_ThreadIds.clear();
		m_NextThreadId = 1;
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

		auto const cat = std::string(isFunction ? "function" : "scope");
		auto const tidHash = std::hash<std::thread::id>{}(result.threadID);

		std::string entry;

		std::lock_guard lock(m_Mutex);

		auto const [it, inserted] = m_ThreadIds.emplace(tidHash, m_NextThreadId);
		if (inserted)
		{
			auto const tid = std::to_string(m_NextThreadId);
			++m_NextThreadId;

			if (!m_FirstEntry)
			{
				m_Buffer += ',';
			}
			m_FirstEntry = false;

			m_Buffer += R"({"name":"thread_name","ph":"M","pid":0,"tid":)" + tid
				+ R"(,"args":{"name":"Thread )" + tid + R"("}})";
		}

		auto const tid = std::to_string(it->second);

		if (result.start >= 0)
		{
			entry += R"({"cat":")" + cat
				+ R"(","name":")" + result.name
				+ R"(","ph":"B","pid":0,"tid":)" + tid
				+ R"(,"ts":)" + std::to_string(result.start)
				+ "}";
		}

		if (result.end >= 0)
		{
			if (!entry.empty())
			{
				entry += ",";
			}

			entry += R"({"cat":")" + cat
				+ R"(","name":")" + result.name
				+ R"(","ph":"E","pid":0,"tid":)" + tid
				+ R"(,"ts":)" + std::to_string(result.end)
				+ "}";
		}

		if (!m_FirstEntry)
		{
			m_Buffer += ',';
		}
		m_FirstEntry = false;
		m_Buffer += entry;

		if (m_Buffer.size() >= m_BufferFlushThreshold)
		{
			m_OutputStream << m_Buffer;
			m_Buffer.clear();
		}
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
		m_FirstEntry = true;
		m_OutputStream << R"({"otherData":{},"traceEvents":[)";
		m_OutputStream.flush();
	}

	void GoogleProfiler::WriteFooter()
	{
		m_Buffer += "]}";
	}
}
