#include <toad/stacktrace.hh>

#include <execinfo.h>
#include <ctype.h>
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>

void
toad::printStackTrace()
{
  void* callstack[256];
  int i, frames = backtrace(callstack, 256);
  char** strs = backtrace_symbols(callstack, frames);
#if 0
  for(i=1; i<frames; ++i) {
    printf("  at %s\n", strs[i]);
  }
#else
  for(i=1; i<frames; ++i) {
    // p0 := end of the 3rd white space sequence in p0
    // 3   fischland 0x0000000100101ca8 _ZN4toad13TFigureEditor10mouseEventERKNS_11TMouseEventE + 824
    char *p0=strs[i];
    for(int i=0; i<3; ++i) {
      while(*p0 && !isblank(*p0))
        ++p0;
      while(*p0 && isblank(*p0))
        ++p0;
    }

    // p1 := end of the string at p0
    char *p1=p0;
    while(*p1 && !isblank(*p1))
      ++p1;

    // demangle string between p0 and p1
    *p1 = 0;
    int status;
    size_t len;
    char* name = abi::__cxa_demangle(p0, NULL, &len, &status);
    *p1 = ' ';

    // print the result
    if (name) {
      *p0 = 0;
      printf("  at %s%s%s\n", strs[i], name, p1);
      free(name);
    } else {
      printf("  at %s\n", strs[i]);
    }
  }
#endif
  free(strs);
}
