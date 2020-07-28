
/*
 *
 * (C) Copyright 2009
 * MediaTek <www.MediaTek.com>
 * Chunhui Li for multi-thread malloc/free test
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysconf.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#endif

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "MTM_TEST"
#include <log/log.h>

#define MALLOC_LOOP 5000
static int malloc_size[] = {4, 256, 24, 4, 64, 24, 64, 24, 4096, 24, 4, 64};

//zhy_change: to count variable reminder 0 & 1 amount
static size_t count0 = 0;
static size_t count1 = 0;
static size_t times = 0;  // make k = 0,1 to run run double thread_num times

static void *MallocThread(void *args) {
  size_t i = 0, j = 0;
  size_t **ptr;
  size_t array_size = sizeof(malloc_size)/sizeof(malloc_size[0]);
  size_t reminder;
  size_t mem_usage_ratio = (size_t)(*((size_t*)args));

  size_t idx;

  int k = 0;  // run double thread_num
  for (k = 0; k < 2; k++) {
    times++;
    reminder = times%16;
    ALOGD("times = %zu\n", times);
    if (reminder > mem_usage_ratio) {
        ALOGD("pid:%d, tid:%d scenraio 1\n", getpid(), gettid());
        count0++;
        ALOGD("zhy_debug1: reminder == 0, count:%zu\n", count0);  // zhy_change: count reminder = 0
        ptr = malloc(array_size * sizeof(size_t *));
        for (i = 0; i < MALLOC_LOOP; i++) {
            for (j = 0; j < array_size; j++) {
                ptr[j] = malloc(malloc_size[j]);
                if (ptr[j]) {
                    memset(ptr[j], 0xff, malloc_size[j]);
                }
            }

            for (j = 0; j < array_size; j++) {
            if (ptr[j]) {
                free(ptr[j]);
                ptr[j] = NULL;
            }
        }
    }
    free(ptr);
    ptr = NULL;
    } else {
        ALOGD("pid:%d, tid:%d scenraio 2\n", getpid(), gettid());
        count1++;
        ALOGD("zhy_debug1: reminder == 1, count:%zu\n", count1);  // zhy_change: count reminder = 1
        ptr = malloc(array_size * sizeof(size_t *) * MALLOC_LOOP);
        for (i = 0; i < MALLOC_LOOP; i++) {
            for (j = 0; j < array_size; j++) {
                idx = j + i*array_size;
                ptr[idx] = malloc(malloc_size[j]);
                if (ptr[idx]) {
                    memset(ptr[idx], 0xbb, malloc_size[j]);
                }
            }
        }

        for (i = 0; i < MALLOC_LOOP; i++) {
            for (j = 0; j < array_size; j++) {
                idx = j + i*array_size;
                if (ptr[idx]) {
                    free(ptr[idx]);
                    ptr[idx] = NULL;
                }
            }
        }
        free(ptr);
        ptr = NULL;
    }
    ALOGD("pid:%d, tid:%d stop\n", getpid(), gettid());
  }

  return NULL;
}

static void help_info() {
  fputs("Usage:\nno paramater will create threads base on cpu cores\n"
        "-t [<thread_num>] set the threads number need create\n", stderr);
  fputs("-r [<mem_usage_ratio>] set the ratio of memory usage mode\n", stderr);
}

int main (int argc, char **argv) {
  int i = 0;
  size_t j = 0;
  long cpu_num = sysconf(_SC_NPROCESSORS_CONF);
  long cpu_online = sysconf(_SC_NPROCESSORS_ONLN);
  pthread_t *t = NULL;
  struct timeval time_start, time_end;
  size_t thread_num = (size_t)cpu_num;
  size_t mem_usage_ratio = 5; // default = 5: thread_num = cpu_num = 8, simulate dex2oat behavior on L

  if (argc > 1) {
    int get_thread_num = 0;
    int get_mem_usage_ratio = 0;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-H")) {
            help_info();
            return 0;
        } else if (!strcmp(argv[i], "-t")) {
            get_thread_num = 1;
            continue;
        } else if (!strcmp(argv[i], "-r")) {
            get_mem_usage_ratio = 1;
            continue;
        } else if (get_thread_num) {
            thread_num = atoi(argv[i]);
        } else if (get_mem_usage_ratio) {
            mem_usage_ratio = atoi(argv[i]);
        } else {
            help_info();
            return 1;
        }
    }
  }

  ALOGD("cpu cores:%ld, online:%ld, thread_num:%zu\n", cpu_num, cpu_online, thread_num);
  t = (pthread_t *)malloc(thread_num * sizeof(pthread_t));
  if (t == NULL) {
        ALOGE("pthread_t malloc fail\n");
        return 1;
  }

  memset(t, 0x0, thread_num * sizeof(pthread_t));
  gettimeofday(&time_start, NULL);
  if (thread_num > 0){
    for (j = 0; j < thread_num; ++j) {
      pthread_create(&t[j], NULL, MallocThread, &mem_usage_ratio);
    }

    for (j = 0; j < thread_num; ++j) {
      pthread_join(t[j], NULL);
    }
  }

  gettimeofday(&time_end, NULL);
  time_end.tv_sec -= time_start.tv_sec;
  time_end.tv_usec -= time_start.tv_usec;
  if (time_end.tv_usec < 0) {
    --time_end.tv_sec;
    time_end.tv_usec += 1000000;
  }

  if (time_end.tv_usec/1000 > 0)
    ALOGD("[LCH_DEBUG]multi-thread malloc_free time used[%ld.%ld] seconds\n", time_end.tv_sec, time_end.tv_usec/1000);
  else
    ALOGE("[LCH_DEBUG]multi-thread malloc_free time used[%ld] seconds\n", time_end.tv_sec);

  free(t);
  t = NULL;

  return 0;
}
