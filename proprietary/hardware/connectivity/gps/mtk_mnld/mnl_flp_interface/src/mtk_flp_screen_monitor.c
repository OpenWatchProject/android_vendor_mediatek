#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/inotify.h>
#include "data_coder.h"
#include "mtk_gps.h"
#include "mnld.h"
#include "mtk_flp_screen_monitor.h"
#include "mtk_flp_main.h"
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
#define LOGD(...) tag_log(1, "[mnl2flp]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnl2flp] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnl2flp] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flp"

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

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)

/**********************************************************
 *  Global vars                                           *
 **********************************************************/
static UINT8 earlysuspend_state = EARLYSUSPEND_ON;
static pthread_mutex_t earlysuspend_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t earlysuspend_cond = PTHREAD_COND_INITIALIZER;

/**********************************************************
 *  Function Declaration                                  *
 **********************************************************/
static int wait_for_fb_wake(void) {
    int ino_len;
    int ino_fd;
    int ino_wd;
    char ino_buf[1024];
    char buf[100];
    FILE *fd;
    while(1) {
        ino_fd = inotify_init();
        if(ino_fd < 0) {
            LOGE("inotify_init error");
            return 1;
        }
        ino_wd = inotify_add_watch(ino_fd, "/sys/power/wake_lock", IN_MODIFY);
        if (ino_wd == -1) {
            LOGE("inotify add fail: %s(%d)\n", strerror(errno), errno);
            return 1;
        }
        ino_len = read(ino_fd, ino_buf, 1024);
        if(ino_len < 0) {
            LOGE("inotify read error");
            return 1;
        }
        (void)inotify_rm_watch(ino_fd, ino_wd);
        (void)close(ino_fd);

        fd = fopen("/sys/power/wake_lock" , "r");
        if(fd) {
            while(fscanf(fd, "%99s", buf) != EOF) {
                if(strstr(buf, "Display") != NULL) {
                    fclose(fd);
                    return 0;
                }
            }
        }
        else {
            LOGE("fopen error\n");
            return 1;
        }
        fclose(fd);
    }
}

static int wait_for_fb_sleep(void)
{
    int ino_len;
    int ino_fd;
    int ino_wd;
    char ino_buf[1024];
    char buf[100];
    FILE *fd;
    while(1) {
        ino_fd = inotify_init();
        if (ino_fd < 0) {
            LOGE("inotify_init error");
            return 1;
        }
        ino_wd = inotify_add_watch(ino_fd, "/sys/power/wake_unlock", IN_MODIFY);
        if (ino_wd == -1) {
            LOGE("inotify add fail: %s(%d)\n", strerror(errno), errno);
            return 1;
        }
        ino_len = read(ino_fd, ino_buf, 1024);
        if (ino_len < 0) {
            LOGE("inotify read error");
            return 1;
        }
        (void)inotify_rm_watch(ino_fd, ino_wd);
        (void)close(ino_fd);

        fd = fopen("/sys/power/wake_unlock" , "r");
        if(fd) {
            while(fscanf(fd, "%99s", buf) != EOF) {
                if (strstr(buf, "Display") != NULL) {
                    fclose(fd);
                    return 0;
                }
            }
        } else {
            LOGE("fopen error\n");
            return 1;
        }
        fclose(fd);
    }
}

static int flp_screen_send2mnl(const char* buff, int len) {
    int ret = 0;
    if (safe_sendto("flp_to_mnl", buff, len) < 0) {
        LOGE("flp_screen_send2mnl safe_sendto flp failed\n");
        ret = -1;
    }
    if (safe_sendto("gfc_to_mnl", buff, len) < 0) {
        LOGE("flp_screen_send2mnl safe_sendto gfc failed\n");
        ret = -1;
    }
    return ret;
}

static void* flp_screen_monitor_thread(void *arg) {
    MTK_FLP_MSG_T *flp_header;
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    UNUSED(arg);

    LOGE("Create\n");

    // Due to it listens screen off event firstly, we nned to send screen on
    // to main thread as an initial value.
    mnld_screen_on_notify();

    while (1) {
        LOGE("Wake Monitor Restart");
        if (wait_for_fb_sleep()) {
            LOGE("Failed reading wait_for_fb_sleep");
            break;
        }
        pthread_mutex_lock(&earlysuspend_mutex);
        earlysuspend_state = EARLYSUSPEND_MEM;
        pthread_cond_signal(&earlysuspend_cond);
        pthread_mutex_unlock(&earlysuspend_mutex);
        LOGE("Screen off");

        //Send NTF to CONNsys
        offset = 0;
        flp_header = malloc(sizeof(MTK_FLP_MSG_T) + sizeof(UINT8));
        if (flp_header) {
            flp_header->type = MTK_FLP_MSG_CONN_SCREEN_STATUS;
            flp_header->length = sizeof(UINT8);
            memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&earlysuspend_state, sizeof(UINT8));
            put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
            ret = flp_screen_send2mnl(buff, offset);
            free(flp_header);
        }

        //Send to mnld main handler thread
        mnld_screen_off_notify();

        if (wait_for_fb_wake()) {
            LOGE("Failed reading wait_for_fb_wake");
            break;
        }
        pthread_mutex_lock(&earlysuspend_mutex);
        earlysuspend_state = EARLYSUSPEND_ON;
        pthread_cond_signal(&earlysuspend_cond);
        pthread_mutex_unlock(&earlysuspend_mutex);
        LOGE("Screen on");

        offset = 0;
        flp_header = malloc(sizeof(MTK_FLP_MSG_T) + sizeof(UINT8));
        if (flp_header) {
            flp_header->type = MTK_FLP_MSG_CONN_SCREEN_STATUS;
            flp_header->length = sizeof(UINT8);
            memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&earlysuspend_state, sizeof(UINT8));
            put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
            ret = flp_screen_send2mnl(buff, offset);
            free(flp_header);
        }

        //Send to mnld main handler thread
        mnld_screen_on_notify();
    }

    // It should become unknown is this screen monitor thread exit
    mnld_screen_unknown_notify();
    LOGE("exit\n");
    pthread_exit(NULL);
    return 0;
}

void mtk_flp_get_wake_monitor_state(UINT8 *state) {
    if(state) {
        memcpy(state, &earlysuspend_state, sizeof(UINT8));
    }
}

void flp_monitor_init() {
    pthread_t pthread_wakelock;
    pthread_create(&pthread_wakelock, NULL, flp_screen_monitor_thread, NULL);
}



