#include "Profiler/GoogleProfiler.h"

#include <charconv>
#include <fstream>
#include <string>

namespace profiler
{
	thread_local GoogleProfiler::ThreadCache GoogleProfiler::s_Cache;

	GoogleProfiler::~GoogleProfiler() noexcept
	{
		try { EndSession(); } catch (...) {}
	}

	void GoogleProfiler::BeginSession(std::string const& name, std::string_view filepath, uint32_t maxFrames, FlushCallback callback)
	{
		Profiler<GoogleProfiler>::BeginSession(name, filepath, maxFrames, std::move(callback));
		m_ThreadBuffers.clear();
		m_NextThreadId = 1;
		static std::atomic<uint32_t> counter{ 0 };
		m_SessionId = ++counter;
		m_Active = true;
	}

	void GoogleProfiler::RegisterThread() noexcept
	{
		std::lock_guard lock(m_Mutex);

		auto tb{ std::make_unique<detail::ThreadBuffer>() };
		tb->events.reserve(INITIAL_EVENT_CAPACITY);
		tb->tid = m_NextThreadId++;

		s_Cache.buffer = tb.get();
		s_Cache.sessionId = m_SessionId;
		m_ThreadBuffers.emplace_back(std::move(tb));
	}

	void GoogleProfiler::SetThreadName(std::string_view name) noexcept
	{
		if (!m_Active) [[unlikely]]
		{
			return;
		}

		EnsureThreadBuffer();
		s_Cache.buffer->threadName = name;
	}

	void GoogleProfiler::MarkFrame(std::string_view name) noexcept
	{
		// Google Trace Event Format has no frame boundary concept.
		// Falls back to naming the calling thread so the trace viewer can at least group events.
		SetThreadName(name);
	}

	std::string GoogleProfiler::BuildJson() const
	{
		size_t totalEvents{ 0 };
		for (auto const& tb : m_ThreadBuffers)
			totalEvents += tb->events.size();

		std::string json;
		json.reserve(totalEvents * 128 + 256);
		json += R"({"otherData":{},"traceEvents":[)";

		bool first{ true };
		char buf[64];

		for (auto const& tb : m_ThreadBuffers)
		{
			if (tb->events.empty()) continue;

			auto [tidEnd, _]{ std::to_chars(buf, buf + sizeof(buf), tb->tid) };
			std::string_view const tidStr{ buf, tidEnd };

			// Thread metadata event
			if (!first) json += ',';
			first = false;

			json += R"({"name":"thread_name","ph":"M","pid":0,"tid":)";
			json += tidStr;
			json += R"(,"args":{"name":")";
			if (tb->threadName.empty())
			{
				json += "Thread ";
				json += tidStr;
			}
			else
			{
				json += tb->threadName;
			}
			json += R"("}})";

			// Trace events
			for (auto const& e : tb->events)
			{
				json += R"(,{"cat":")";
				json += e.type == detail::TraceEventType::Function ? "function" : "scope";
				json += R"(","name":")";
				json += e.name;
				json += R"(","ph":"X","pid":0,"tid":)";
				json += tidStr;
				json += R"(,"ts":)";
				auto [tsEnd, ec1]{ std::to_chars(buf, buf + sizeof(buf), e.start) };
				json.append(buf, tsEnd);
				json += R"(,"dur":)";
				auto [durEnd, ec2]{ std::to_chars(buf, buf + sizeof(buf), e.duration) };
				json.append(buf, durEnd);
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
