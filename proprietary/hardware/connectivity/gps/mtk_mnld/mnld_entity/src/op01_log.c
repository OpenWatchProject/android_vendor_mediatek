#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>

#include "mtk_lbs_utility.h"
#include "data_coder.h"
#include "op01_log.h"
#include "mnld.h"
#include "gps_controller.h"

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
#define LOGD(...) tag_log(1, "[op01]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[op01] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[op01] ERR: ", __VA_ARGS__);
#else
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
typedef enum {
    MAIN2OP01_EVENT_LOG_WRITE       = 0,
} main2op01_event;

#ifdef MTK_GNSS_OP01_LOG_SUPPORT
static int g_fd_op01;
extern bool mnld_exiting;
static int send_op01_log_msg(const char *fmt, ...) {
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, MAIN2OP01_EVENT_LOG_WRITE);
    put_string(buff, &offset, buf);
    return safe_sendto(MNLD_OP01_LOG_WRITER_SOCKET, buff, offset);
}

static void get_time_stamp(char* time_str1, char* time_str2) {
    struct tm *tm_pt;
    time_t time_st;
    struct timeval tv;

    time(&time_st);
    gettimeofday(&tv, NULL);
    tm_pt = gmtime(&time_st);
    tm_pt = localtime(&tv.tv_sec);
    memset(time_str1, 0, sizeof(char)*30);
    memset(time_str2, 0, sizeof(char)*30);
    if (tm_pt) {
        sprintf(time_str1, "%d%02d%02d%02d%02d%02d.%1ld",
            tm_pt->tm_year+1900, tm_pt->tm_mon+1, tm_pt->tm_mday,
            tm_pt->tm_hour, tm_pt->tm_min, tm_pt->tm_sec, tv.tv_usec/100000);
        sprintf(time_str2, "%d%02d%02d%02d%02d%02d.%03ld",
            tm_pt->tm_year+1900, tm_pt->tm_mon+1, tm_pt->tm_mday,
            tm_pt->tm_hour, tm_pt->tm_min, tm_pt->tm_sec, tv.tv_usec/1000);
    }
    LOGE("time_str1=%s,time_str2=%s\n", time_str1, time_str2);
}
int op01_log_write_internal(const char* log) {
    // TODO add a configuration to enable/disable the op01 log
    if (get_file_size(MNLD_OP01_LOG_PATH) > 65535) {
        LOGD("delete_op01_file=[%s]", MNLD_OP01_LOG_PATH);
        delete_file(MNLD_OP01_LOG_PATH);
    }
    write_msg2file(MNLD_OP01_LOG_PATH, "%s", log);
    return 0;
}
static int op01_event_hdlr(int fd) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    main2op01_event cmd;
    int read_len;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("op01_event_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    cmd = get_int(buff, &offset, sizeof(buff));
    switch (cmd) {
    case MAIN2OP01_EVENT_LOG_WRITE: {
        char* log = get_string(buff, &offset, sizeof(buff));
        LOGD("op01_log_write_internal()  log=[%s]", log);
        op01_log_write_internal(log);
        break;
    }
    default: {
        LOGE("epo_event_hdlr() unknown cmd=%d", cmd);
        return -1;
    }
    }
    return 0;
}

static void op01_log_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("op01_log_thread_timeout() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("op01_log_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}
static void* op01_log_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    timer_t hdlr_timer = init_timer(op01_log_thread_timeout);
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("op01_log_thread() epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return 0;
    }

    if (epoll_add_fd(epfd, g_fd_op01) == -1) {
        LOGE("op01_log_thread() epoll_add_fd() failed for g_fd_op01 failed");
        return 0;
    }

    while (1) {
        int i;
        int n;
        LOGD("op01_log_thread wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("op01_log_thread() epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        start_timer(hdlr_timer, MNLD_OP01_HANDLER_TIMEOUT);
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_op01) {
                if (events[i].events & EPOLLIN) {
                    op01_event_hdlr(g_fd_op01);
                }
            } else {
                LOGE("op01_log_thread() unknown fd=%d",
                    events[i].data.fd);
            }
        }
        stop_timer(hdlr_timer);
    }
    LOGE("op01_log_thread() exit");
    return 0;
}
#endif
int op01_log_gps_start() {
#ifdef MTK_GNSS_OP01_LOG_SUPPORT
    char time_str1[30] = {0};
    char time_str2[30] = {0};
    get_time_stamp(time_str1, time_str2);
    return send_op01_log_msg("[%s]0x00000000: %s #gps start\r\n", time_str2, time_str2);
#else
    return 0;
#endif
}

int op01_log_gps_stop() {
#ifdef MTK_GNSS_OP01_LOG_SUPPORT
    char time_str1[30] = {0};
    char time_str2[30] = {0};
    get_time_stamp(time_str1, time_str2);
    return send_op01_log_msg("[%s]0x00000001: %s #gps stop\r\n", time_str2, time_str2);
#else
    return 0;
#endif
}

int op01_log_gps_location(double lat, double lng, int ttff) {
#ifdef MTK_GNSS_OP01_LOG_SUPPORT
    char time_str1[30] = {0};
    char time_str2[30] = {0};
    get_time_stamp(time_str1, time_str2);
    return send_op01_log_msg("[%s]0x00000002: %s, %f, %f, %d #position(time_stamp, lat, lon, ttff)\r\n",
        time_str2, time_str2, lat, lng, ttff);
#else
    UNUSED(lat);
    UNUSED(lng);
    UNUSED(ttff);
    return 0;
#endif
}

int op01_log_init() {
#ifdef MTK_GNSS_OP01_LOG_SUPPORT
    pthread_t pthread_op01;

    g_fd_op01 = socket_bind_udp(MNLD_OP01_LOG_WRITER_SOCKET);
    if (g_fd_op01 < 0) {
        LOGE("socket_bind_udp(MNLD_OP01_LOG_WRITER_SOCKET) failed");
        return -1;
    }

    pthread_create(&pthread_op01, NULL, op01_log_thread, NULL);
#endif
    return 0;
}

