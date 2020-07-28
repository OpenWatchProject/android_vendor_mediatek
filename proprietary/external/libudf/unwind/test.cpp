#include <stdio.h>
#include <stdlib.h>

#include "libudf-unwind/backtrace.h"


int do_backtrace(float /* just to test demangling */, bool use_gcc) {
  const size_t MAX_DEPTH = 32;
  backtrace_frame_t* frames = (backtrace_frame_t*) malloc(sizeof(backtrace_frame_t) * MAX_DEPTH);
  ssize_t frame_count;
  if (use_gcc) {
    frame_count = libudf_unwind_backtrace_gcc(frames, 0, MAX_DEPTH);
  }
  else {
    frame_count = libudf_unwind_backtrace(frames, 0, MAX_DEPTH);
  }
  fprintf(stderr, "frame_count=%d\n", (int) frame_count);
  if (frame_count <= 0) {
    return frame_count;
  }

  for (size_t i = 0; i < (size_t) frame_count; ++i) {
    printf("#%02zd pc %08x\n", i, frames[i]);
  }
  free(frames);
  return frame_count;
}

struct C {
  int g(int i, bool use_gcc);
};

// __attribute__ ((noinline))
int C::g(int i, bool use_gcc) {
  if (i == 0) {
    return do_backtrace(0.1, use_gcc);
  }
  return g(i - 1, use_gcc);
}

// __attribute__ ((noinline))
extern "C" int f(bool use_gcc) {
  C c;
  return c.g(5, use_gcc);
}

int main() {
  f(false);

  f(true);

  *((volatile int *)0) = 0xdeaddead;
  return 0;
}
