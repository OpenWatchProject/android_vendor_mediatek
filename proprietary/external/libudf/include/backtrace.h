#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <stdlib.h>
#include <unwind.h>
#include <dlfcn.h>


//#define ENABLE_JAVA_BACKTRACE
#ifdef __cplusplus
extern "C" {
#endif


// =================================================================
// back trace size
// =================================================================
#define MAX_FP_BACKTRACE_SIZE 5
#define MAX_BACKTRACE_SIZE    32

size_t ubrd_get_backtrace_common(void *, uintptr_t*, size_t, size_t);
size_t ubrd_get_backtrace_fp(void*, uintptr_t*, size_t);
size_t ubrd_get_backtrace(uintptr_t*, size_t, size_t); /*gcc uwnind*/


#ifdef __cplusplus
}
#endif
#endif // #ifndef BACKTRACE_H
