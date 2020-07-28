#ifndef __MTK_LBS_UTILITY_H__
#define __MTK_LBS_UTILITY_H__

#include <time.h>
#include <stdint.h>
#include <pthread.h>

#include "gps_dbg_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************
* Basic Utilities
**************************************************/
#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

#define CRASH_TO_DEBUG() \
{\
    int* crash = 0;\
    gps_dbg_log_exit_flush(0);\
    *crash = 100;\
}

#define  ANDROID_MNLD_PROP_SUPPORT 1
#define  MTK_GPS_NVRAM  1
void tag_log(int type, const char* tag, const char *fmt, ...);

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#define LOGD(...) tag_log(1, "[mnld][Default]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnld] WARNING: [Default]", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnld] ERR: [Default]", __VA_ARGS__);

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)


typedef struct sync_lock {
    pthread_mutex_t mutx;
    pthread_cond_t con;
    int condtion;
    int index;
}SYNC_LOCK_T;

void msleep(int interval);

// in millisecond
time_t get_tick();

// in millisecond
time_t get_time_in_millisecond();

// -1 means failure
int block_here();

//exit block_here() and process will exit and restart
void mnld_block_exit(void);

/*************************************************
* Timer
**************************************************/
typedef void (* timer_callback)();

// -1 means failure
timer_t init_timer_id(timer_callback cb, int id);

// -1 means failure
timer_t init_timer(timer_callback cb);

// -1 means failure
int start_timer(timer_t timerid, int milliseconds);

// -1 means failure
int stop_timer(timer_t timerid);

// -1 means failure
int deinit_timer(timer_t timerid);

/*************************************************
* Epoll
**************************************************/
// -1 means failure
int epoll_add_fd(int epfd, int fd);

// -1 failed
int epoll_add_fd2(int epfd, int fd, uint32_t events);

// -1 failed
int epoll_del_fd(int epfd, int fd);

int epoll_mod_fd(int epfd, int fd, uint32_t events);

/*************************************************
* Local UDP Socket
**************************************************/
// -1 means failure
int socket_bind_udp(const char* path);

int mnld_flp_to_mnld_fd_init(const char* path);

int mnld_sendto_flp(void* dest, char* buf, int size);

// -1 means failure
int socket_set_blocking(int fd, int blocking);

// -1 means failure
int safe_sendto(const char* path, const char* buff, int len);

// -1 means failure
int safe_recvfrom(int fd, char* buff, int len);

/*************************************************
* File
**************************************************/
// 0 not exist, 1 exist
int is_file_exist(const char* path);

// -1 means failure
int get_file_size(const char* path);

// -1 failure
int delete_file(const char* file_path);

// -1 means failure
int write_msg2file(char* dest, char* msg, ...);

int asc_str_to_usc2_str(char* output, const char* input);

void raw_data_to_hex_string(char* output, char* input, int input_len);

void init_condition(SYNC_LOCK_T *lock);

void get_condition(SYNC_LOCK_T *lock);

void release_condition(SYNC_LOCK_T *lock);

void mnld_dump_exit(void);

#ifdef __cplusplus
}
#endif

#endif
