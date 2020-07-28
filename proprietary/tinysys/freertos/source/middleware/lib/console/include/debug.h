#ifndef __DEBUG_H
#define __DEBUG_H

#include <assert.h>
#include <stdarg.h>
#include <compiler.h>
#include <platform/debug.h>
#include <printf.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef ASSERT
#define ASSERT(expr) if(!(expr)) printf("fail! %s, %d\n", __FILE__, __LINE__);
#endif  // ASSERT

#if defined(__cplusplus)
}
#endif

#endif
