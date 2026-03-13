#ifndef PROFILER_GOOGLE_PROFILER_H
#define PROFILER_GOOGLE_PROFILER_H

// TODO: Potential future optimizations (diminishing returns — hot path is dominated by clock calls):
// - Binary ring buffer: write fixed-size structs instead of JSON strings, convert at session end
// - Compile-time dispatch: replace virtual WriteProfile with templates/if constexpr to enable inlining

#include "Profiler/Profiler.h"

#include <memory>
#include <mutex>
#include <vector>

namespace profiler
{
	/** Per-thread buffer that accumulates trace events without locking. */
	struct ThreadBuffer final
	{
		std::string data;
		std::string tidStr;
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
		void BeginSession(std::string const& name, char const* filepath = nullptr, uint32_t maxFrames = 0, FlushCallback callback = nullptr) override;

		/** @see Profiler::WriteProfile */
		void WriteProfile(ProfileResult const& result, bool isFunction) override;

		/** @see Profiler::EndSession */
		void EndSession() override;

		/** @see Profiler::FlushToString */
		std::string FlushToString() override;

		GoogleProfiler(GoogleProfiler const&) = delete;
		GoogleProfiler(GoogleProfiler&&) = delete;
		GoogleProfiler& operator=(GoogleProfiler const&) = delete;
		GoogleProfiler& operator=(GoogleProfiler&&) = delete;

	private:
		std::string BuildJson() const;

		mutable std::mutex m_Mutex;
		bool m_Active{ false };
		std::vector<std::unique_ptr<ThreadBuffer>> m_ThreadBuffers;
		uint32_t m_NextThreadId{ 1 };
		uint32_t m_SessionId{ 0 };
	};
}

#endif
