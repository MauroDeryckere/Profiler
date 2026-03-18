#include "GoogleProfiler.h"

#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <string>

namespace profiler
{
	namespace
	{
		uint32_t constexpr INITIAL_EVENT_CAPACITY{ 4096 };

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

	void GoogleProfiler::BeginSession(std::string const& name, std::string_view filepath, uint32_t maxFrames, FlushCallback callback)
	{
		Profiler::BeginSession(name, filepath, maxFrames, std::move(callback));
		m_ThreadBuffers.clear();
		m_NextThreadId = 1;
		m_SessionId = nextSessionId();
		m_Active = true;
	}

	void GoogleProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		if (!m_Active)
		{
			return;
		}

		if (s_Cache.sessionId != m_SessionId)
		{
			std::lock_guard lock(m_Mutex);

			auto tb{ std::make_unique<ThreadBuffer>() };
			tb->events.reserve(INITIAL_EVENT_CAPACITY);
			tb->tid = m_NextThreadId++;

			s_Cache.buffer = tb.get();
			s_Cache.sessionId = m_SessionId;
			m_ThreadBuffers.emplace_back(std::move(tb));
		}

		s_Cache.buffer->events.emplace_back(result.name, result.start, result.end - result.start, isFunction);
	}

	std::string GoogleProfiler::BuildJson() const
	{
		std::string json;
		json += R"({"otherData":{},"traceEvents":[)";

		bool first{ true };
		char buf[64];

		for (auto const& tb : m_ThreadBuffers)
		{
			if (tb->events.empty()) continue;

			auto const tidLen{ snprintf(buf, sizeof(buf), "%u", tb->tid) };
			std::string_view const tidStr{ buf, static_cast<size_t>(tidLen) };

			// Thread metadata event
			if (!first) json += ',';
			first = false;

			json += R"({"name":"thread_name","ph":"M","pid":0,"tid":)";
			json += tidStr;
			json += R"(,"args":{"name":"Thread )";
			json += tidStr;
			json += R"("}})";

			// Trace events
			for (auto const& e : tb->events)
			{
				json += R"(,{"cat":")";
				json += e.isFunction ? "function" : "scope";
				json += R"(","name":")";
				json += e.name;
				json += R"(","ph":"X","pid":0,"tid":)";
				json += tidStr;
				json += R"(,"ts":)";
				auto len{ snprintf(buf, sizeof(buf), "%" PRId64, e.start) };
				json.append(buf, static_cast<size_t>(len));
				json += R"(,"dur":)";
				len = snprintf(buf, sizeof(buf), "%" PRId64, e.duration);
				json.append(buf, static_cast<size_t>(len));
				json += '}';
			}
		}

		json += "]}";
		return json;
	}

	void GoogleProfiler::EndSession()
	{
		if (m_Active)
		{
			if (!GetFileName().empty())
			{
				auto const path{ GetFileName() + ".json" };
				PrepareOutputPath(path);

				std::ofstream out(path);
				if (out.is_open())
				{
					out << BuildJson();
				}
				else
				{
					fprintf(stderr, "[Profiler] Failed to open file: %s\n", path.c_str());
				}
			}

			m_ThreadBuffers.clear();
			m_Active = false;
		}
	}

	std::string GoogleProfiler::FlushToString() const
	{
		if (!m_Active)
		{
			return {};
		}

		return BuildJson();
	}
}
