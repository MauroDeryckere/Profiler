#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <cstdint>
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
		 * @param name			Display name for the session.
		 * @param filepath		Output file path (extension is appended by the backend).
		 * @param reserveSize	Initial per-thread buffer reserve size in bytes.
		 */
		void BeginSession(std::string const& name, char const* filepath, size_t reserveSize = 100'000);

		/** Ends the current session and flushes all buffered data to disk. */
		virtual void EndSession() = 0;

		/**
		 * Writes a single profile event. Pass start=-1 for an end-only event,
		 * or end=-1 for a begin-only event.
		 * @param result		The profile timing data.
		 * @param isFunction	True if this is a function scope, false for a named scope.
		 */
		virtual void WriteProfile(ProfileResult const& result, bool isFunction) = 0;

		/**
		 * Starts a frame-based profiling session at the given path.
		 * Call Update() each frame; the session auto-ends after GetMaxFrames() frames.
		 * @param path	Output file path (extension is appended by the backend).
		 */
		void Start(char const* path);

		/** Advances the frame counter. Ends the session automatically once max frames is reached. */
		void Update();

		/**
		 * Sets the maximum number of frames to capture in frame-based profiling.
		 * @param frames	Number of frames before auto-stopping.
		 */
		void SetMaxFrames(uint32_t frames) { maxFrames = frames; }

		/** @return The maximum number of frames to capture. */
		uint32_t GetMaxFrames() const { return maxFrames; }

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
		virtual void BeginSessionInternal(std::string const& name, size_t reserveSize = 100'000) = 0;

		uint32_t profiledFrames{ 0 };
		bool isProfiling{ false };
		uint32_t numExecutedProfiles{ 0 };
		uint32_t maxFrames{ 5 };
	};
}

#endif
