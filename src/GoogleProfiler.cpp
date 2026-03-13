#include "GoogleProfiler.h"

#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <fstream>

namespace profiler
{
	namespace
	{
		uint32_t nextSessionId()
		{
			static std::atomic<uint32_t> counter{ 0 };
			return ++counter;
		}

		struct ThreadCache final
		{
			ThreadBuffer* buffer{ nullptr };
			uint32_t sessionId{ 0 };
		};

		thread_local ThreadCache s_Cache;
	}

	GoogleProfiler::~GoogleProfiler()
	{
		EndSession();
	}

	void GoogleProfiler::BeginSessionInternal(std::string const& name, size_t reserveSize)
	{
		EndSession();

		if (!fileName.empty())
		{
			fileName += ".json";
		}

		m_ThreadBuffers.clear();
		m_NextThreadId = 1;
		m_SessionId = nextSessionId();

		m_CurrentSession = std::make_unique<InstrumentationSession>(InstrumentationSession{ name });
	}

	void GoogleProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		if (!m_CurrentSession)
		{
			return;
		}

		if (s_Cache.sessionId != m_SessionId)
		{
			std::lock_guard lock(m_Mutex);

			auto tb{ std::make_unique<ThreadBuffer>() };
			tb->data.reserve(100'000);
			tb->tidStr = std::to_string(m_NextThreadId++);

			tb->data += R"({"name":"thread_name","ph":"M","pid":0,"tid":)";
			tb->data += tb->tidStr;
			tb->data += R"(,"args":{"name":"Thread )";
			tb->data += tb->tidStr;
			tb->data += R"("}})";

			s_Cache.buffer = tb.get();
			s_Cache.sessionId = m_SessionId;
			m_ThreadBuffers.emplace_back(std::move(tb));
		}

		auto& d{ s_Cache.buffer->data };
		auto const* cat{ isFunction ? "function" : "scope" };
		char tsBuf[24];

		if (result.start >= 0)
		{
			snprintf(tsBuf, sizeof(tsBuf), "%" PRId64, static_cast<int64_t>(result.start));
			d += R"(,{"cat":")";
			d += cat;
			d += R"(","name":")";
			d += result.name;
			d += R"(","ph":"B","pid":0,"tid":)";
			d += s_Cache.buffer->tidStr;
			d += R"(,"ts":)";
			d += tsBuf;
			d += '}';
		}

		if (result.end >= 0)
		{
			snprintf(tsBuf, sizeof(tsBuf), "%" PRId64, static_cast<int64_t>(result.end));
			d += R"(,{"cat":")";
			d += cat;
			d += R"(","name":")";
			d += result.name;
			d += R"(","ph":"E","pid":0,"tid":)";
			d += s_Cache.buffer->tidStr;
			d += R"(,"ts":)";
			d += tsBuf;
			d += '}';
		}
	}

	std::string GoogleProfiler::BuildJson() const
	{
		std::string json;
		json += R"({"otherData":{},"traceEvents":[)";

		bool first{ true };
		for (auto const& tb : m_ThreadBuffers)
		{
			if (tb->data.empty()) continue;

			if (!first)
			{
				json += ',';
			}
			first = false;
			json += tb->data;
		}

		json += "]}";
		return json;
	}

	void GoogleProfiler::EndSession()
	{
		if (m_CurrentSession)
		{
			if (!fileName.empty())
			{
				PrepareOutputPath(fileName.c_str());

				std::ofstream out(fileName);
				if (out.is_open())
				{
					out << BuildJson();
				}
				else
				{
					fprintf(stderr, "[Profiler] Failed to open file: %s\n", fileName.c_str());
				}
			}

			m_ThreadBuffers.clear();
			m_CurrentSession = nullptr;
		}
	}

	std::string GoogleProfiler::FlushToString()
	{
		if (!m_CurrentSession)
		{
			return {};
		}

		return BuildJson();
	}
}
