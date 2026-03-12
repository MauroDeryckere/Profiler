#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <cstdint>
#include <string>
#include <thread>

namespace profiler
{
	struct ProfileResult final
	{
		std::string name;
		long long start;
		long long end;
		std::thread::id threadID;
	};

	class Profiler
	{
	public:
		virtual ~Profiler() = default;

		void BeginSession(std::string const& name, char const* filepath, size_t reserveSize = 100'000);
		virtual void EndSession() = 0;

		virtual void WriteProfile(ProfileResult const& result, bool isFunction) = 0;

		/// Start a frame-based profiling session at the given path.
		/// Call Update() each frame; the session auto-ends after GetMaxFrames() frames.
		void Start(char const* path);
		void Update();

		void SetMaxFrames(uint32_t frames) { maxFrames = frames; }
		uint32_t GetMaxFrames() const { return maxFrames; }

		/// Ensures the parent directory exists and removes any existing file at filepath.
		static void EnsureDirectoryExists(char const* filepath);

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
