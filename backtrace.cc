#include <execinfo.h>
#include <cstdio>
#include <cstdlib>

void
printStackTrace()
{
  void* callstack[256];
  int i, frames = backtrace(callstack, 256);
  char** strs = backtrace_symbols(callstack, frames);
  for(i=1; i<frames; ++i) {
    printf("  at %s\n", strs[i]);
  }
  free(strs);
}
