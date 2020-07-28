#ifndef BACK_TRACE_H
#define BACK_TRACE_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unwind.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t get_backtrace_fp(void* fp, intptr_t*, size_t); /*fp unwind*/
size_t get_backtrace(intptr_t* addrs, size_t max_entries); /*gcc unwind*/

#ifdef __cplusplus
}
#endif
#endif // #ifndef BACK_TRACE_H