#ifndef PROFILER_GOOGLE_PROFILER_H
#define PROFILER_GOOGLE_PROFILER_H

#include "Profiler/Profiler.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

namespace profiler
{
	namespace detail
	{
		enum class TraceEventType : uint8_t { Function, Scope };

		/** A single recorded trace event. Stored as binary data on the hot path; converted to JSON at session end. */
		struct TraceEvent final
		{
			std::string_view name;
			int64_t start;
			int64_t duration;
			TraceEventType type;
		};

		/** Per-thread buffer that accumulates trace events without locking. */
		struct ThreadBuffer final
		{
			std::vector<TraceEvent> events;
			std::string threadName;
			uint32_t tid{ 0 };
		};
	}

	/**
	 * Profiler backend that outputs data in Google Trace Event Format (JSON).
	 * Results can be viewed in chrome://tracing or https://ui.perfetto.dev
	 */
	class GoogleProfiler final : public Profiler<GoogleProfiler>
	{
	public:
		GoogleProfiler() = default;
		~GoogleProfiler() noexcept;

		void BeginSession(std::string const& name, std::string_view filepath = {}, uint32_t maxFrames = 0, FlushCallback callback = nullptr);
		void EndSession();
		void SetThreadName(std::string_view name) noexcept;
		void MarkFrame(std::string_view name) noexcept;
		[[nodiscard]] std::string FlushToString() const;

		inline void WriteProfile(ProfileResult const& result, bool isFunction) noexcept
		{
			if (!m_Active) [[unlikely]]
			{
				return;
			}

			EnsureThreadBuffer();
			s_Cache.buffer->events.emplace_back(result.name, result.start, result.end - result.start,
				isFunction ? detail::TraceEventType::Function : detail::TraceEventType::Scope);
		}

		GoogleProfiler(GoogleProfiler const&) = delete;
		GoogleProfiler(GoogleProfiler&&) = delete;
		GoogleProfiler& operator=(GoogleProfiler const&) = delete;
		GoogleProfiler& operator=(GoogleProfiler&&) = delete;

	private:
		static uint32_t constexpr INITIAL_EVENT_CAPACITY{ 4096 };

		struct ThreadCache final
		{
			detail::ThreadBuffer* buffer{ nullptr };
			uint32_t sessionId{ 0 };
		};

		static thread_local ThreadCache s_Cache;

		inline void EnsureThreadBuffer() noexcept
		{
			if (s_Cache.sessionId != m_SessionId) [[unlikely]]
			{
				RegisterThread();
			}
		}

		void RegisterThread() noexcept;
		[[nodiscard]] std::string BuildJson() const;

		mutable std::mutex m_Mutex;
		bool m_Active{ false };
		std::vector<std::unique_ptr<detail::ThreadBuffer>> m_ThreadBuffers;
		uint32_t m_NextThreadId{ 1 };
		uint32_t m_SessionId{ 0 };
	};
}

#endif
