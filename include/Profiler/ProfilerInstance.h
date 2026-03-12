#ifndef PROFILER_INSTANCE_H
#define PROFILER_INSTANCE_H

#include "Profiler/Profiler.h"

#include <memory>

namespace profiler
{

	/// Manages the global profiler instance.
	/// Call Initialize() once at startup, Shutdown() at exit.
	class ProfilerInstance
	{
	public:
		/// Returns the active profiler. Returns a no-op profiler if not initialized.
		static Profiler& Get();

		/// Creates the default backend (Google/Chrome tracing, or Optick if PROFILER_USE_OPTICK is defined).
		static void Initialize();

		/// Sets a custom profiler backend.
		static void Set(std::unique_ptr<Profiler> profiler);

		/// Resets to the no-op profiler.
		static void Shutdown();

	private:
		ProfilerInstance() = default;
	};
}

#endif
