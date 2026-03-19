#ifndef PROFILER_CROSS_LIB_HELPER_H
#define PROFILER_CROSS_LIB_HELPER_H

// Functions compiled in a separate static library to simulate
// cross-library profiler calls (like Engine calling into Renderer).
// Each function writes a profile event via the global PROFILER singleton.

void CrossLibWriteFunction();
void CrossLibWriteScope();

#endif
