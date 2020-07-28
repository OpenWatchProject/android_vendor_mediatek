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
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/


#if 0
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
//#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
//#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
//#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
#endif

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
    LOGD("fd=%d\n", fd);

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
    LOGD("path=%s\n", path);
    return fd;
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
        if (errno == EINTR) {
            LOGE("errno==EINTR\n");
            continue;
        }
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                LOGE("errno==EAGAIN\n");
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

// -1 means failure
int safe_recvfrom(int fd, char* buff, int len) {
    int ret = 0;
    int retry = 10;

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        LOGE("fcntl failed reason=[%s]%d",
                    strerror(errno), errno);

        close(fd);
        return -1;
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

gps_cmd_record cmd_list[GPS_HAL_CMD_RECORD_NUM];
int cmd_rcd_idx = 0;
time_t enter_time_ms = 0;
static timer_t cmd_moniter_timer;
static int last_record_idx = 0xFF;

void gps_hal_cmd_enter_record(mnl2hal_cmd cmd) {
    int ret = 0;
    /*Clear parameters before setting*/
    memset(cmd_list[cmd_rcd_idx].enter_time, 0, GPS_HAL_TIME_STR_LEN);
    memset(cmd_list[cmd_rcd_idx].exit_time, 0, GPS_HAL_TIME_STR_LEN);
    cmd_list[cmd_rcd_idx].cmd = 0;
    cmd_list[cmd_rcd_idx].exec_time = 0;

    last_record_idx = 0xFF;  //Print all history command

    enter_time_ms = get_time_in_millisecond();
    ret = get_time_str(cmd_list[cmd_rcd_idx].enter_time, GPS_HAL_TIME_STR_LEN);
    if(ret == -1) {
        LOGW("get_time_str fail(%s)", strerror(errno));
    }
    cmd_list[cmd_rcd_idx].cmd = cmd;

    ret = start_timer(cmd_moniter_timer, GPS_HAL_CMD_MONITER_TIMEOUT);
    if(ret == -1) {
        LOGW("start_timer fail(%s)", strerror(errno));
    }
}

void gps_hal_cmd_exit_record(mnl2hal_cmd cmd) {
    if(cmd == cmd_list[cmd_rcd_idx].cmd) {
        int ret = 0;
        ret = get_time_str(cmd_list[cmd_rcd_idx].exit_time, GPS_HAL_TIME_STR_LEN);
        if(ret == -1) {
            LOGW("get_time_str fail(%s)", strerror(errno));
        }
        cmd_list[cmd_rcd_idx].exec_time = get_time_in_millisecond() - enter_time_ms;
        cmd_rcd_idx++;
        cmd_rcd_idx = cmd_rcd_idx%GPS_HAL_CMD_RECORD_NUM;  //Over-write oldest recording

        ret = stop_timer(cmd_moniter_timer);
        if(ret == -1) {
            LOGW("stop_timer fail(%s)", strerror(errno));
        }
    } else {
        LOGW("cmd not matched:enter->%d, exit->%d", cmd_list[cmd_rcd_idx].cmd, cmd);
    }
}

void gps_hal_cmd_list_init(void) {
    memset(cmd_list, 0, sizeof(cmd_list));
    cmd_rcd_idx = 0;
    enter_time_ms = 0;
    last_record_idx = 0xFF;

    cmd_moniter_timer = init_timer(gps_hal_cmd_list_dump);
}

void gps_hal_cmd_list_dump(void) {
    int print_idx = 0;
    int cur_rcd = cmd_rcd_idx;
    int print_more = 0;
    int empty_cnt = 0;

    if(last_record_idx != cur_rcd) {
        print_more = 1;
        last_record_idx = cur_rcd;
    }

    LOGW("Dump GPS HAL command record:");
    for(print_idx = 0; print_idx < GPS_HAL_CMD_RECORD_NUM; print_idx++){
        if(cmd_list[print_idx].cmd != 0) {  //Valid cmd
            if(print_idx == cur_rcd) {
                LOGW("[%2d]%s:%3d << Current command(%lld)", print_idx, cmd_list[print_idx].enter_time, cmd_list[print_idx].cmd, (long long)(get_time_in_millisecond() - enter_time_ms ));
            } else {
                if(print_more) {
                    LOGW("[%2d]%s~%s(%lld):%3d", print_idx, cmd_list[print_idx].enter_time, cmd_list[print_idx].exit_time, (long long)cmd_list[print_idx].exec_time, cmd_list[print_idx].cmd );
                }
            }
        } else {
            empty_cnt++;
        }
    }
    if(empty_cnt) {
        LOGW("empty_cnt:%d", empty_cnt);
    }
}
