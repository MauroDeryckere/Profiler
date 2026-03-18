#ifndef PROFILER_MACROS_H
#define PROFILER_MACROS_H

// ============================================================================
// Profiler — main include for instrumentation macros.
//
// Usage:
//   #include <Profiler/ProfilerMacros.h>
//
//   void MyFunction() {
//       PROFILER_FUNCTION();           // profiles entire function
//       {
//           PROFILER_SCOPE("section"); // profiles a named scope
//           // ...
//       }
//   }
//
// All macros compile to nothing when PROFILER_ENABLED is not defined.
// ============================================================================

#define PROFILER_CONCAT_IMPL(a, b) a##b
#define PROFILER_CONCAT(a, b) PROFILER_CONCAT_IMPL(a, b)

// Portable function name macro
#define PROFILER_FUNC_SIG __FUNCTION__

#if defined(PROFILER_ENABLED)

	#include "Profiler/ServiceLocator.h"

	#if defined(PROFILER_USE_OPTICK)
		#include <Optick.h>

		#define PROFILER_FUNCTION()   OPTICK_EVENT()
		#define PROFILER_SCOPE(name)  OPTICK_EVENT(name)
		#define PROFILER_THREAD(name) OPTICK_THREAD(name)
		#define PROFILER_FRAME(name)  OPTICK_FRAME(name)
	#else
		#include "Profiler/InstrumentorTimer.h"

		#define PROFILER_FUNCTION()   ::profiler::InstrumentorTimer PROFILER_CONCAT(profTimer_, __LINE__){PROFILER_FUNC_SIG, true}
		#define PROFILER_SCOPE(name)  ::profiler::InstrumentorTimer PROFILER_CONCAT(profTimer_, __LINE__){name, false}
		#define PROFILER_THREAD(name) PROFILER.SetThreadName(name)
		#define PROFILER_FRAME(name)  PROFILER.MarkFrame(name)
	#endif

	#define PROFILER_BEGIN_SESSION(name, ...)	 PROFILER.BeginSession(name, ##__VA_ARGS__)
	#define PROFILER_END_SESSION()               PROFILER.EndSession()
	#define PROFILER_TICK()                      PROFILER.Tick()

#else

	#define PROFILER_FUNCTION()
	#define PROFILER_SCOPE(name)
	#define PROFILER_THREAD(name)
	#define PROFILER_FRAME(name)
	#define PROFILER_BEGIN_SESSION(name, ...)
	#define PROFILER_END_SESSION()
	#define PROFILER_TICK()

#endif

#endif
