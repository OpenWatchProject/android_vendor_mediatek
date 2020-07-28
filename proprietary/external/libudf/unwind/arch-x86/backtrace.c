/*
    Stub to build x86 binary for Cuttlestone
*/

#include <stdlib.h>

typedef uintptr_t backtrace_frame_t;

#if !defined(UNUSED)
#define UNUSED(x)	((void)(x))
#endif

__attribute__((visibility("default")))
ssize_t libudf_unwind_backtrace(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth)
{
    ssize_t frames = -1;

    UNUSED(backtrace); UNUSED(ignore_depth); UNUSED(max_depth);

    return frames;
}
