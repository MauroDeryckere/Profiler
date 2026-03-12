#ifndef PROFILER_GOOGLE_PROFILER_H
#define PROFILER_GOOGLE_PROFILER_H

#include "Profiler/Profiler.h"

#include <fstream>
#include <memory>
#include <mutex>
#include <vector>

namespace profiler
{
	struct InstrumentationSession final
	{
		std::string name;
	};

	struct ThreadBuffer
	{
		std::string data;
		std::string tidStr;
	};

	/// Outputs profiling data in Google Trace Event Format (JSON).
	/// Results can be viewed in chrome://tracing or https://ui.perfetto.dev
	class GoogleProfiler final : public Profiler
	{
	public:
		GoogleProfiler() = default;
		~GoogleProfiler() override;

		void WriteProfile(ProfileResult const& result, bool isFunction) override;

		void EndSession() override;

		GoogleProfiler(GoogleProfiler const&) = delete;
		GoogleProfiler(GoogleProfiler&&) = delete;
		GoogleProfiler& operator=(GoogleProfiler const&) = delete;
		GoogleProfiler& operator=(GoogleProfiler&&) = delete;

	private:
		void BeginSessionInternal(std::string const& name, size_t reserveSize = 100'000) override;

		mutable std::mutex m_Mutex;
		std::unique_ptr<InstrumentationSession> m_CurrentSession{ nullptr };
		std::vector<std::unique_ptr<ThreadBuffer>> m_ThreadBuffers;
		std::ofstream m_OutputStream;
		uint32_t m_NextThreadId{ 1 };
		uint32_t m_SessionId{ 0 };
	};
}

#endif
