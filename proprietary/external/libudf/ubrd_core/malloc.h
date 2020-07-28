#ifndef MALLOC_H
#define MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#define USE_DL_PREFIX

#define ONLY_MSPACES 1 // default: MSPACE 0
#define MSPACES 1 // default: MSPACE 0
//#define DEBUG 1
//#define IN_MSPACE 1

#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAVE_MORECORE 0
#define MORECORE_CANNOT_TRIM 1
#define MALLOC_INSPECT_ALL 1
//#define IN_DEBUG 1

#include "malloc.c"
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "MTK_UBRD_DEBUG"


#ifdef __cplusplus
}
#endif
#endif
