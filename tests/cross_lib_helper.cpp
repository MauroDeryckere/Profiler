#include "cross_lib_helper.h"
#include <Profiler/ProfilerMacros.h>

void CrossLibWriteFunction()
{
	PROFILER_FUNCTION();
}

void CrossLibWriteScope()
{
	PROFILER_SCOPE("CrossLibScope");
}
