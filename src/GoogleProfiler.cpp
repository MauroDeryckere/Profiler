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

	void GoogleProfiler::BeginSession(std::string const& name, char const* filepath, uint32_t maxFrames, FlushCallback callback)
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
		char durBuf[24];

		snprintf(tsBuf, sizeof(tsBuf), "%" PRId64, static_cast<int64_t>(result.start));
		snprintf(durBuf, sizeof(durBuf), "%" PRId64, static_cast<int64_t>(result.end - result.start));

		d += R"(,{"cat":")";
		d += cat;
		d += R"(","name":")";
		d += result.name;
		d += R"(","ph":"X","pid":0,"tid":)";
		d += s_Cache.buffer->tidStr;
		d += R"(,"ts":)";
		d += tsBuf;
		d += R"(,"dur":)";
		d += durBuf;
		d += '}';
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
		if (m_Active)
		{
			if (!m_FileName.empty())
			{
				auto const path{ m_FileName + ".json" };
				PrepareOutputPath(path.c_str());

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
