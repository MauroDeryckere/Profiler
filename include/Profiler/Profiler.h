#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace profiler
{
	/** Holds the result of a single profiled scope or function. */
	struct ProfileResult final
	{
		std::string_view name;
		int64_t start;
		int64_t end;
	};

	/** Abstract base class for all profiler backends. */
	class Profiler
	{
	public:
		virtual ~Profiler() = default;

		using FlushCallback = std::function<void(std::string const&)>;

		/**
		 * Begins a new profiling session.
		 * @param name		Display name for the session.
		 * @param filepath	Output file path (extension is appended by the backend). Empty for string-only output via FlushToString().
		 * @param maxFrames	If > 0, Tick() will auto-end the session after this many calls. 0 means manual EndSession() only.
		 * @param callback	Called with the trace JSON when the session auto-ends via Tick().
		 */
		virtual void BeginSession(std::string const& name, std::string_view filepath = {}, uint32_t maxFrames = 0, FlushCallback callback = nullptr);

		/** Ends the current session and flushes all buffered data to disk. */
		virtual void EndSession() = 0;

		/**
		 * Returns the current session's buffered trace data as a JSON string
		 * without ending the session.
		 * Useful for environments without filesystem access (e.g. Emscripten)
		 * or for streaming data to a custom sink.
		 * @return The complete trace JSON, or an empty string if no session is active.
		 */
		[[nodiscard]] virtual std::string FlushToString() const = 0;

		/**
		 * Writes a single complete profile event with start time and duration.
		 * @param result		The profile timing data (start and end timestamps).
		 * @param isFunction	True if this is a function scope, false for a named scope.
		 */
		virtual void WriteProfile(ProfileResult const& result, bool isFunction) = 0;

		/**
		 * Advances the frame counter. When the configured frame count is reached,
		 * fires the flush callback (if set) then calls EndSession().
		 * No-op if the session has no frame limit (maxFrames was 0 in BeginSession).
		 */
		void Tick();

		/**
		 * Creates parent directories for filepath and removes any existing file at that path.
		 * @param filepath	The target file path to prepare.
		 */
		static void PrepareOutputPath(std::string_view filepath);

		Profiler(Profiler const&) = delete;
		Profiler(Profiler&&) = delete;
		Profiler& operator=(Profiler const&) = delete;
		Profiler& operator=(Profiler&&) = delete;

	protected:
		Profiler() = default;
		[[nodiscard]] std::string const& GetFileName() const noexcept { return m_FileName; }

	private:
		std::string m_FileName;
		uint32_t m_ProfiledFrames{ 0 };
		uint32_t m_MaxFrames{ 0 };
		FlushCallback m_FlushCallback;
	};
}

#endif
