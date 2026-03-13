#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace profiler
{
	/** Holds the result of a single profiled scope or function. */
	struct ProfileResult final
	{
		char const* name;
		long long start;
		long long end;
		std::thread::id threadID;
	};

	/** Abstract base class for all profiler backends. */
	class Profiler
	{
	public:
		virtual ~Profiler() = default;

		/**
		 * Begins a new profiling session.
		 * @param name		Display name for the session.
		 * @param filepath	Output file path (extension is appended by the backend). Pass nullptr for string-only output via FlushToString().
		 */
		virtual void BeginSession(std::string const& name, char const* filepath = nullptr);

		/** Ends the current session and flushes all buffered data to disk. */
		virtual void EndSession() = 0;

		/**
		 * Returns the current session's buffered trace data as a JSON string
		 * and ends the session without writing to disk.
		 * Useful for environments without filesystem access (e.g. Emscripten)
		 * or for streaming data to a custom sink.
		 * @return The complete trace JSON, or an empty string if no session is active.
		 */
		virtual std::string FlushToString() = 0;

		/**
		 * Writes a single profile event. Pass start=-1 for an end-only event,
		 * or end=-1 for a begin-only event.
		 * @param result		The profile timing data.
		 * @param isFunction	True if this is a function scope, false for a named scope.
		 */
		virtual void WriteProfile(ProfileResult const& result, bool isFunction) = 0;

		using FlushCallback = std::function<void(std::string const&)>;

		/**
		 * Starts a frame-based profiling session.
		 * Call Update() each frame; the session auto-ends after GetNumFramesToProfile() frames.
		 * @param path		Output file path (extension is appended by the backend). Pass nullptr to use a callback instead.
		 * @param callback	Called with the trace JSON when the session auto-ends. Only used when path is nullptr.
		 */
		void Start(char const* path, FlushCallback callback = nullptr);

		/** Advances the frame counter. Ends the session automatically once the configured frame count is reached. */
		void Update();

		/**
		 * Sets how many frames to capture before the session auto-ends.
		 * Only applies to frame-based profiling via Start()/Update().
		 * @param frames	Number of frames to profile before auto-stopping. Must be greater than 0.
		 */
		void SetNumFramesToProfile(uint32_t frames) { numFramesToProfile = frames; }

		/** @return The number of frames to capture before auto-stopping. */
		uint32_t GetNumFramesToProfile() const { return numFramesToProfile; }

		/**
		 * Creates parent directories for filepath and removes any existing file at that path.
		 * @param filepath	The target file path to prepare.
		 */
		static void PrepareOutputPath(char const* filepath);

		Profiler(Profiler const&) = delete;
		Profiler(Profiler&&) = delete;
		Profiler& operator=(Profiler const&) = delete;
		Profiler& operator=(Profiler&&) = delete;

	protected:
		Profiler() = default;
		std::string fileName;

	private:
		uint32_t profiledFrames{ 0 };
		bool isProfiling{ false };
		uint32_t numExecutedProfiles{ 0 };
		uint32_t numFramesToProfile{ 5 };
		FlushCallback flushCallback;
	};
}

#endif
