#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>  // offsetof
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>  // usleep
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>  // inet_addr
#include <sys/un.h>  // struct sockaddr_un
#include <pthread.h>
#include <sys/epoll.h>
#include <signal.h>
#include <semaphore.h>

#if defined(__ANDROID_OS__)
#include <cutils/log.h>     // Android log
#endif

#include "mtk_lbs_utility.h"

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[mnld]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnld] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnld] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mtk_lbs_utility"

#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#ifdef CONFIG_GPS_ENG_LOAD
#define LOGD_ENG(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#else
#define LOGD_ENG(fmt, arg ...) NULL
#endif
#endif

int asc_str_to_usc2_str(char* output, const char* input) {
    int len = 2;

    output[0] = 0xfe;
    output[1] = 0xff;

    while (*input != 0) {
        output[len++] = 0;
        output[len++] = *input;
        input++;
    }

    output[len] = 0;
    output[len+1] = 0;
    return len;
}

void raw_data_to_hex_string(char* output, char* input, int input_len) {
    int i = 0;
    for (i = 0; i < input_len; i++) {
        int tmp = (input[i] >> 4) & 0xf;

        if (tmp >= 0 && tmp <= 9) {
            output[i*2] = tmp + '0';
        } else {
            output[i*2] = (tmp - 10) + 'A';
        }

        tmp = input[i] & 0xf;
        if (tmp >= 0 && tmp <= 9) {
            output[(i*2)+1] = tmp + '0';
        } else {
            output[(i*2)+1] = (tmp - 10) + 'A';
        }
    }
    output[i*2] = 0;
}

// -1 means failure
static int get_time_str(char* buf, int len) {
    struct timeval  tv;
    struct timezone tz;
    struct tm      *tm;

    gettimeofday(&tv, &tz);
    tm = localtime(&tv.tv_sec);

    memset(buf, 0, len);
    if(tm != NULL) {
        sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
            tm->tm_year + 1900, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min,
            tm->tm_sec, (int)(tv.tv_usec / 1000));
    }
    return 0;
}

/*************************************************
* Basic Utilities
**************************************************/
void tag_log(int type, const char* tag, const char *fmt, ...) {
    char out_buf[1100] = {0};
    char buf[1024] = {0};
    va_list ap;
    int prio = 0;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    snprintf(out_buf, sizeof(out_buf), "%s %s", tag, buf);

#if defined(__ANDROID_OS__)
    if (type == 0) {
        prio = ANDROID_LOG_DEBUG;
    } else {
        prio = ANDROID_LOG_ERROR;
    }
        __android_log_print(prio, "lbs", "%s", out_buf);
#else
    char time_buf[64] = {0};
    UNUSED(type);
    UNUSED(prio);
    get_time_str(time_buf, sizeof(time_buf));
    printf("%s %s\n", time_buf, out_buf);
    // printf("%s 0x%08x %s\n", time_buf, pthread_self(), out_buf);
#endif
}

void msleep(int interval) {
    usleep(interval * 1000);
}

// in millisecond
time_t get_tick() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        LOGE("clock_gettime failed reason=[%s]", strerror(errno));
        return -1;
    }
    return (ts.tv_sec*1000) + (ts.tv_nsec/1000000);
}

// in millisecond
time_t get_time_in_millisecond() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        LOGE("get_time_in_millisecond  failed reason=[%s]", strerror(errno));
        return -1;
    }
    return ((long long)ts.tv_sec*1000) + ((long long)ts.tv_nsec/1000000);
}


sem_t g_mnld_exit_sem;
extern bool mnld_exiting;

// -1 means failure
int block_here() {
    if (sem_init(&g_mnld_exit_sem, 0, 0) == -1) {
        LOGE("block_here() sem_init failure reason=%s\n", strerror(errno));
        return -1;
    }
    sem_wait(&g_mnld_exit_sem);
    if (sem_destroy(&g_mnld_exit_sem) == -1) {
        LOGE("block_here() sem_destroy reason=%s\n", strerror(errno));
    }
    return 0;
}

void mnld_block_exit(void) {
    mnld_exiting = true;
    if(sem_post(&g_mnld_exit_sem) == -1) {
        LOGE("sem_post failed");
        _exit(0);
    }
}

/*************************************************
* Timer
**************************************************/
// -1 means failure
timer_t init_timer_id(timer_callback cb, int id) {
    struct sigevent sevp;
    timer_t timerid;

    memset(&sevp, 0, sizeof(sevp));
    sevp.sigev_value.sival_int = id;
    sevp.sigev_notify = SIGEV_THREAD;
    sevp.sigev_notify_function = cb;

    if (timer_create(CLOCK_MONOTONIC, &sevp, &timerid) == -1) {
        LOGE("timer_create  failed reason=[%s]", strerror(errno));
        return (timer_t)-1;
    }
    return timerid;
}

// -1 means failure
timer_t init_timer(timer_callback cb) {
    return init_timer_id(cb, 0);
}

// -1 means failure
int start_timer(timer_t timerid, int milliseconds) {
    struct itimerspec expire;
    expire.it_interval.tv_sec = 0;
    expire.it_interval.tv_nsec = 0;
    expire.it_value.tv_sec = milliseconds/1000;
    expire.it_value.tv_nsec = (milliseconds%1000)*1000000;
    return timer_settime(timerid, 0, &expire, NULL);
}

// -1 means failure
int stop_timer(timer_t timerid) {
    return start_timer(timerid, 0);
}

// -1 means failure
int deinit_timer(timer_t timerid) {
    if (timer_delete(timerid) == -1) {
        // errno
        return -1;
    }
    return 0;
}

/*************************************************
* Epoll
**************************************************/
// -1 means failure
int epoll_add_fd(int epfd, int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    // don't set the fd to edge trigger
    // the some event like accept may be lost if two or more clients are connecting to server at the same time
    // level trigger is preferred to avoid event lost
    // do not set EPOLLOUT due to it will always trigger when write is available
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOGE("epoll_add_fd() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

// -1 failed
int epoll_add_fd2(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = events;
    // don't set the fd to edge trigger
    // the some event like accept may be lost if two or more clients are connecting to server at the same time
    // level trigger is preferred to avoid event lost
    // do not set EPOLLOUT due to it will always trigger when write is available
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOGE("epoll_add_fd2() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

int epoll_del_fd(int epfd, int fd) {
    struct epoll_event  ev;
    int                 ret;

    if (epfd == -1)
        return -1;

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

// -1 failed
int epoll_mod_fd(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        LOGE("epoll_mod_fd() epoll_ctl() failed reason=[%s]%d epfd=%d fd=%d",
            strerror(errno), errno, epfd, fd);
        return -1;
    }
    return 0;
}

/*************************************************
* Local UDP Socket
**************************************************/
// -1 means failure
int socket_bind_udp(const char* path) {
    int fd;
    struct sockaddr_un addr;

    fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("socket_bind_udp() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }
    LOGD("fd=%d,path=%s\n", fd, path);

    memset(&addr, 0, sizeof(addr));
    addr.sun_path[0] = 0;
    MNLD_STRNCPY(addr.sun_path + 1, path,sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;
    unlink(addr.sun_path);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOGE("socket_bind_udp() bind() failed path=[%s] reason=[%s]%d",
            path, strerror(errno), errno);
        close(fd);
        return -1;
    }
    return fd;
}

int mnld_flp_to_mnld_fd_init(const char* path) {
    int flp_fd = -1;
    struct sockaddr_un cmd_local;

    if ((flp_fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1) {
        LOGE("flp2mnl socket create failed\n");
        return flp_fd;
    }

    unlink(path);
    memset(&cmd_local, 0, sizeof(cmd_local));
    cmd_local.sun_family = AF_LOCAL;
    MNLD_STRNCPY(cmd_local.sun_path, path,sizeof(cmd_local.sun_path));

    if (bind(flp_fd, (struct sockaddr *)&cmd_local, sizeof(cmd_local)) < 0) {
        LOGE("flp2mnl socket bind failed\n");
        close(flp_fd);
        flp_fd = -1;
        return flp_fd;
    }

    int res = chmod(path, 0660);  // (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    LOGD("chmod res = %d, %s\n", res, strerror(errno));
    return flp_fd;
}

// -1 means failure
int socket_set_blocking(int fd, int blocking) {
    if (fd < 0) {
        LOGE("socket_set_blocking() invalid fd=%d", fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOGE("socket_set_blocking() fcntl() failed invalid flags=%d reason=[%s]%d",
            flags, strerror(errno), errno);
        return -1;
    }

    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : -1;
}

// -1 means failure
int safe_sendto(const char* path, const char* buff, int len) {
    int ret = 0;
    struct sockaddr_un addr;
    int retry = 10;
    int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("safe_sendto() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);

        close(fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_path[0] = 0;
    MNLD_STRNCPY(addr.sun_path + 1, path,sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;

    while ((ret = sendto(fd, buff, len, 0,
        (const struct sockaddr *)&addr, sizeof(addr))) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("safe_sendto() sendto() failed path=[%s] ret=%d reason=[%s]%d",
            path, ret, strerror(errno), errno);
        break;
    }

    close(fd);
    return ret;
}

int mnld_sendto_flp(void* dest, char* buf, int size) {
    // dest: MTK_MNLD2HAL
    int ret = 0;
    int len = 0;
    struct sockaddr_un soc_addr;
    socklen_t addr_len;
    int retry = 10;

    int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOGE("safe_sendto() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    MNLD_STRNCPY(soc_addr.sun_path, dest,sizeof(soc_addr.sun_path));
    soc_addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(soc_addr.sun_path) + 1);

    //LOGD("mnld2flp fd: %d\n", fd);
    while ((len = sendto(fd, buf, size, 0,
        (const struct sockaddr *)&soc_addr, (socklen_t)addr_len)) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("[mnld2hal] ERR: sendto dest=[%s] len=%d reason =[%s]\n",
            (char *)dest, size, strerror(errno));
        ret = -1;
        break;
    }
    close(fd);
    return ret;
}

// -1 means failure
int safe_recvfrom(int fd, char* buff, int len) {
    int ret = 0;
    int retry = 10;

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);
    }

    while ((ret = recvfrom(fd, buff, len, 0,
         NULL, NULL)) == -1) {
        LOGW("safe_recvfrom() ret=%d len=%d", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("safe_recvfrom() recvfrom() failed reason=[%s]%d",
            strerror(errno), errno);
        break;
    }
    return ret;
}

/*************************************************
* File
**************************************************/
// 0 not exist, 1 exist
int is_file_exist(const char* path) {
    FILE* fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        return 0;
    } else {
        fclose(fp);
        return 1;
    }
    return 0;
}

// -1 means failure
int get_file_size(const char* path) {
    struct stat s;
    if (stat(path, &s) == -1) {
        LOGD("get_file_size() stat() fail for [%s] reason=[%s]",
            path, strerror(errno));
        return -1;
    }
    return s.st_size;
}

// -1 failure
int delete_file(const char* file_path) {
    return remove(file_path);
}

// -1 means failure
int write_msg2file(char* dest, char* msg, ...) {
    FILE* fp;
    char buf[1024] = {0};
    va_list ap;

    if (dest == NULL || msg == NULL) {
        return -1;
    }

    va_start(ap, msg);
    vsnprintf(buf, sizeof(buf), msg, ap);
    va_end(ap);

    fp = fopen(dest, "a");
    if (fp == NULL) {
        LOGE("write_msg2file() fopen() fail reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }
    fprintf(fp, "%s", buf);
    fclose(fp);
    return 0;
}

void init_condition(SYNC_LOCK_T *lock) {
    int ret = 0;

    pthread_mutex_lock(&(lock->mutx));
    lock->condtion = 0;
    ret = pthread_mutex_unlock(&(lock->mutx));
    LOGD_ENG("ret mutex unlock = %d\n", ret);

    return;
}

void get_condition(SYNC_LOCK_T *lock) {
    int ret = 0;

    ret = pthread_mutex_lock(&(lock->mutx));
    //LOGD("ret mutex lock = %d\n", ret);

    while (!lock->condtion) {
        ret = pthread_cond_wait(&(lock->con), &(lock->mutx));
        //LOGD("ret cond wait = %d\n" , ret);
    }

    lock->condtion = 0;
    ret = pthread_mutex_unlock(&(lock->mutx));
    LOGD_ENG("ret mutex unlock = %d\n", ret);

    return;
}

void release_condition(SYNC_LOCK_T *lock) {
    int ret = 0;

    ret = pthread_mutex_lock(&(lock->mutx));
    //LOGD("ret mutex lock = %d\n", ret);

    lock->condtion = 1;
    ret = pthread_cond_signal(&(lock->con));
    //LOGD("ret cond_signal = %d\n", ret);

    ret = pthread_mutex_unlock(&(lock->mutx));
    LOGD_ENG("ret unlock= %d\n", ret);

    return;
}
