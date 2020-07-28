#ifndef __MTK_ION_TEST_H__
#define __MTK_ION_TEST_H__

#include <sys/mman.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/resource.h>
#include <linux/fb.h>
#include <wchar.h>
#include <pthread.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <unistd.h>
#include <linux/mtk_ion.h>
#include <ion.h>
/* ion_custom_ioctl test code */
#include "include/sizes.h"
#include <getopt.h>
#include <sys/time.h>


#define IONMSG(string, args...)	printf("[ION]"string, ##args)

#if RAND_MAX == 0x7FFFFFFF
#define RAND_32()    (rand() | (rand() << 31))
#define RAND_64()    (((0ULL | rand()) << 33) | ((0ULL | rand()) << 2) | (rand() & 0x3))
#else
#error "Unexpected RAND_MAX value. Please add support."
#endif


inline
long ION_PRINT_TIME_NAME(void *name) {
	struct timeval time_t;
	long time = 0;

	gettimeofday(&time_t, NULL);
	time = time_t.tv_sec * 1000000 + time_t.tv_usec;
	IONMSG("%s :%ld us\n", name, time);
	return time;
}

inline
long ION_TIME(bool flag) {
	struct timeval time_t;
	long time = 0;

	gettimeofday(&time_t, NULL);
	time = time_t.tv_sec * 1000000 + time_t.tv_usec;
	if (flag)
		IONMSG("%ld us\n", time);
	return time;
}

inline
long ION_GET_TIME_PTINT() {
	return ION_TIME(true);
}

inline
long ION_GET_TIME() {
	return ION_TIME(false);
}


inline
int ion_get_random_32() {
	return RAND_32();
}

inline
unsigned long long ion_get_random_64() {
	return RAND_64();
}


/* test api */
int ion_alloc_fd_test(void);
void ion_fd_leak_test(void);
int ion_custom_ioctl_test(void);
void ion_mmap_test(void);
void ion_cache_sync_va_test(void);
int ion_alloc_fd_test(void);
void show_usage(void);
void ion_custom_ioctl_random_test(void);
void ion_alloc_test_for_ref_check(void);



/* func needed by test api */
void ion_multithread_test(void);
void *ion_thread_1(void *arg);
void *ion_custom_random_data(int fd, unsigned int cmd);


#endif
