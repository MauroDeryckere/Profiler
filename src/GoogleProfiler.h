#ifndef PROFILER_GOOGLE_PROFILER_H
#define PROFILER_GOOGLE_PROFILER_H

#include "Profiler/Profiler.h"

#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

namespace profiler
{
	enum class TraceEventType : uint8_t { Function, Scope, FrameMark };

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

	/**
	 * Profiler backend that outputs data in Google Trace Event Format (JSON).
	 * Results can be viewed in chrome://tracing or https://ui.perfetto.dev
	 */
	class GoogleProfiler final : public Profiler
	{
	public:
		GoogleProfiler() = default;
		~GoogleProfiler() override;

		/** @see Profiler::BeginSession */
		void BeginSession(std::string const& name, std::string_view filepath = {}, uint32_t maxFrames = 0, FlushCallback callback = nullptr) override;

		/** @see Profiler::WriteProfile */
		void WriteProfile(ProfileResult const& result, bool isFunction) override;

		/** @see Profiler::SetThreadName */
		void SetThreadName(std::string_view name) override;

		/** @see Profiler::MarkFrame */
		void MarkFrame(std::string_view name) override;

		/** @see Profiler::EndSession */
		void EndSession() override;

		/** @see Profiler::FlushToString */
		[[nodiscard]] std::string FlushToString() const override;

		GoogleProfiler(GoogleProfiler const&) = delete;
		GoogleProfiler(GoogleProfiler&&) = delete;
		GoogleProfiler& operator=(GoogleProfiler const&) = delete;
		GoogleProfiler& operator=(GoogleProfiler&&) = delete;

	private:
		void EnsureThreadBuffer();
		[[nodiscard]] std::string BuildJson() const;

		mutable std::mutex m_Mutex;
		bool m_Active{ false };
		std::vector<std::unique_ptr<ThreadBuffer>> m_ThreadBuffers;
		uint32_t m_NextThreadId{ 1 };
		uint32_t m_SessionId{ 0 };
	};
}

#endif
