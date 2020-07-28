/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <regex.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <log/log.h>
#include <log/logprint.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <dlfcn.h>
#include <inttypes.h>

#include "config.h"
#include "global_var.h"
#include "bootmode.h"
#include "mlog.h"
#include "logging.h"
#include "libfunc.h"
#include "aee.h"


extern AEE_SYS_FUNC *aee_sys_exp;
extern AEE_SYS_FUNC *aee_sys_war;
extern const char* SUBLOG_NAME[MOBILOG_ID_MAX][2];

int is_mblog_prepare() {
    char is_prepare[PROPERTY_VALUE_MAX];
    property_get(PROP_PREPARE, is_prepare, "0");
    return atoi(is_prepare);
}
/*
* Globle variables related to log_dev_t structure initialization
*/
#define DEV_K_LOG       "/proc/kmsg"
#define DEV_ATF_LOG     "/proc/atf_log/atf_log"
#define DEV_GZ_LOG      "/proc/gz_log"
#define DEV_BSP_LOG     "/sys/kernel/debug/tracing/instances/bsp/trace_pipe"
#define DEV_ME_LOG      "/sys/kernel/debug/tracing/instances/mmedia/trace_pipe"
#define DEV_SCP_LOG     "/dev/scp"
#define DEV_SCP_B_LOG   "/dev/scp_B"
#define DEV_SSPM_LOG    "/dev/sspm"
#define DEV_ADSP_LOG    "/dev/adsp"
#define DEV_MCUPM_LOG   "/dev/mcupm"


/* defualt off,  size(MB) */
static int log_control_map[2][MOBILOG_ID_MAX] = {{0},
    /*0   1    2   3   4   5    6    7     8    9  10  11  12  13 14  15 16*/
    {15, 7, 5, 5, 2, 2, 2, 10, 10, 2, 5, 5, 3, 3, 3, 3, 3}};

/* device name --> log file name */
static const char* log_dev_map[2][MOBILOG_ID_MAX] = {
    { "main", "radio", "events", "system", "crash", "stats", "security", "kernel",
    DEV_ATF_LOG, DEV_GZ_LOG, DEV_BSP_LOG, DEV_ME_LOG, DEV_SCP_LOG, DEV_SCP_B_LOG, DEV_SSPM_LOG, DEV_ADSP_LOG,
    DEV_MCUPM_LOG },

    { "main_log", "radio_log", "events_log", "sys_log", "crash_log", "stats_log", "security_log", "kernel_log",
    "atf_log", "gz_log", "bsp_log", "mmedia_log", "scp_log", "scp_b_log", "sspm_log", "adsp_log", "mcupm_log" }
};

/*
* Globle variables related to storage space occupation control
*/
static long long folder_size_threshold;
static long long total_log_size;
static int rot_idx;
static list_declare(logf_list);

/*
* Globle variables related to memory occupation control
*/
static bool is_writing_log;
static struct timespec ts_0 = {0, 0};

/*
*
* Globle Variables Related to Reader and Writer
*
*/
// choose where to read kernel log
#define KERNEL_LOGD     0
#define KERNEL_KMSG     1

// for Android log and kernel log
static int logd_sock = -1;
static int has_android_log, has_kernel_log;
static int klog_src;

// for buffer count which can be flushed
static int gBufReadyCount;
// write thread will be exit flag
static int writer_stop_flag;

static pthread_t g_read_thread_t;
static pthread_t g_write_thread_t;
static pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t syncLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t syncCond = PTHREAD_COND_INITIALIZER;

static AndroidLogFormat *g_logformat;
static EventTagMap* g_event_tagmap;

static list_declare(devlist);
static list_declare(buflist);
static log_time last_log;
static log_time stopping_time;

static void create_log_devlist(void) {
    struct log_dev_t * ld_p = NULL;
    int flag;

    /* init all enabled log_dev_t structure  */
    for (int id = (int)MOBILOG_ID_MAIN; id < (int)MOBILOG_ID_MAX; id++) {
        if (log_control_map[0][id] == 1) {
            flag = 1;
            while (ld_p == NULL)
                ld_p = (struct log_dev_t *)malloc(sizeof(struct log_dev_t));
            switch (id) {
                case MOBILOG_ID_MAIN ... MOBILOG_ID_KERNEL:
                    break;
                case MOBILOG_ID_ATF:
                    break;
                case MOBILOG_ID_BSP:
                    property_set("vendor.MB.ftrace.log", "bsp");
                    if (access(DEV_BSP_LOG, F_OK))
                        flag = 0;
                    break;
                case MOBILOG_ID_MMEDIA:
                    property_set("vendor.MB.ftrace.log", "mmedia");
                    if (access(DEV_ME_LOG, F_OK))
                        flag = 0;
                    break;
                case MOBILOG_ID_SCP:
                case MOBILOG_ID_SCP_B:
                    enable_scp_poll();
                    break;
                case MOBILOG_ID_SSPM:
                    enable_sspm_poll();
                    break;
                case MOBILOG_ID_ADSP:
                    enable_adsp_poll();
                    break;
                case MOBILOG_ID_MCUPM:
                    enable_mcupm_poll();
                    break;
            }
            if (id != MOBILOG_ID_KERNEL)
                ld_p->type = (id > MOBILOG_ID_KERNEL) ? DIRECT_DEV : ANDROID_DEV;
            else {
                ld_p->type = (klog_src == KERNEL_KMSG) ? DIRECT_DEV : ANDROID_DEV;
                if (klog_src == KERNEL_KMSG) {
                    log_dev_map[0][id] = DEV_K_LOG;
                    MLOGI_BOTH("Read kernel log from kmsg");
                } else
                    MLOGI_BOTH("Read kernel log from logd");
            }
            ld_p->id = id;
            snprintf(ld_p->log_name, sizeof(ld_p->log_name), "%s", log_dev_map[1][id]);
            memset(ld_p->wf_name, 0x00, sizeof(ld_p->wf_name));
            ld_p->rfd = -1;
            ld_p->wfd = -1;

            ld_p->rotate_size = log_control_map[1][id] * 1024 * 1024;
            ld_p->total_write_size = 0;
            ld_p->buf_p = NULL;

            MLOGI_BOTH("ld_p type: %d, id: %d, log_name: %s, rotate_size: %lu",
                ld_p->type, ld_p->id, ld_p->log_name, ld_p->rotate_size);

            if (ld_p->type == DIRECT_DEV && flag == 1) {
                if (open_logger_dev(ld_p) == 0) {
                    free(ld_p);
                    ld_p = NULL;
                }
            }
            if (ld_p != NULL) {
                list_add_tail(&devlist, &ld_p->devlist);
                ld_p  = NULL;
            }
        }
    }
}


/*
 * For logs no need sort or parse,
 * So these logs can save to buffer directly
 */
int direct_log_read(struct log_dev_t *ld_p) {
    char buf[255];
    int ret, e;

    ret = TEMP_FAILURE_RETRY(read(ld_p->rfd, buf, sizeof(buf)));
    e = errno;
    if (ret < 0) {
        if (e == EAGAIN)
            return ret;
        else {
            close(ld_p->rfd);
            ld_p->rfd = -1;
            MLOGE_BOTH("read error from %s,%s", ld_p->log_name, strerror(errno));
            return ret;
        }
    }

    if (ld_p->buf_p == NULL) {
        ALOGE("%s maybe lost", ld_p->log_name);
        return -1;
    }

    if (ret + ld_p->buf_p->buf_used > BUFFER_SIZE) {
        ld_p->buf_p->ok = 1;
        get_free_buf(ld_p);
    }

    memcpy(ld_p->buf_p->m_buf + ld_p->buf_p->buf_used, buf, ret);
    ld_p->buf_p->buf_used += ret;

    return ret;
}

int android_log_read(void) {
    struct log_dev_t *ld_p = NULL;
    struct listnode *dev_p = NULL;
    struct log_msg logmsg;
    int ret, e;

    memset(&logmsg, 0x0, sizeof(logmsg));
    if (logd_sock < 0)
        return -1;
    ret = TEMP_FAILURE_RETRY(recv(logd_sock, &logmsg, sizeof(logmsg), 0));
    e = errno;
    if (ret < 0) {
        MLOGE_BOTH("read android log failed(%s)", strerror(e));
        close(logd_sock);
        logd_sock = -1;
        list_for_each(dev_p, &devlist) {
            ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
            if (ld_p->type == ANDROID_DEV)
                ld_p->rfd = -1;
        }
        return ret;
    }

    if (ret == 0) {
        MLOGE_BOTH("read android log unexpected EOF!");
        close(logd_sock);
        logd_sock = -1;
        MLOGI_BOTH("reconnecting to logd...");
        int retry = 20;
        while (logd_sock < 0 && retry-- > 0) {
            int r = usleep(100000); /* sleep 100ms */
            if (r < 0) {
                MLOGE_BOTH("usleep error %s", strerror(errno));
            }
            logd_sock = connect_to_logdr(has_android_log, has_kernel_log && klog_src == KERNEL_LOGD);
        }
        if (logd_sock > 0) {
            char buf[] = "Some logs maybe lost due to logd socket disconnected!\n";
            MLOGI_BOTH("new logd_sock: %d", logd_sock);
            list_for_each(dev_p, &devlist) {
                ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
                if (ld_p->type == ANDROID_DEV) {
                    ld_p->rfd = logd_sock;
                    write(ld_p->wfd, buf, sizeof(buf)-1); /*write to log file to indicate log lost*/
                }
            }
        } else {
            MLOGE_BOTH("cannot reconnect to logd!");
            set_status(STOPPED);
        }
        return ret;
    }

    list_for_each(dev_p, &devlist) {
        ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
        if (!strcmp(log_dev_map[0][ld_p->id], android_log_id_to_name(logmsg.entry.lid))) {
            if (ld_p->buf_p == NULL) {
                ALOGE("%s maybe lost", ld_p->log_name);
                return -1;
            }
            insert_to_buf(ld_p, &logmsg);   /* insert the log the it's buffer */
            break;
        }
    }

    return ret;
}

int open_logger_dev(struct log_dev_t *ld_p) {
    int f_mode = O_RDONLY | O_NONBLOCK;
    const char *dev_name = log_dev_map[0][ld_p->id];
    ld_p->rfd = TEMP_FAILURE_RETRY(open(dev_name, f_mode));
    if (ld_p->rfd < 0) {
        MLOGE_BOTH("open %s fail(%s)", dev_name, strerror(errno));
        return 0;
    }

    return 1;
}

/*
 * open log file
 */
int open_logfile(struct log_dev_t *ld_p, const char *logpath) {
    char full_path[255];
    char f_name[128];
    char curtime[64] = {'E', '\0'};
    struct tm *tmp;
    time_t lt;
    int fd = -1;

    /* get current time */
    lt = time(NULL);
    tmp = localtime(&lt);
    if (tmp != NULL) {
        strftime(curtime, sizeof(curtime), "_%Y_%m%d_%H%M%S", tmp);
    }
    /* create new log file */
    snprintf(f_name, sizeof(f_name), "%s%s.curf", ld_p->log_name, curtime);
    snprintf(full_path, sizeof(full_path), "%s%s", logpath, f_name);

    fd = mb_open(full_path);
    if (fd < 0) {
        MLOGE_BOTH("%s open file %s fail,(%s)", ld_p->log_name, full_path, strerror(errno));
        return fd;
    }

    /* update  'wf_name' and 'wfd' of ld_p */
    snprintf(ld_p->wf_name, sizeof(ld_p->wf_name), "%s", f_name);
    ld_p->wfd = fd;

    char buf[128], timezoner[PROPERTY_VALUE_MAX];
    property_get("persist.sys.timezone", timezoner, "GMT");
    snprintf(buf, sizeof(buf), "----- timezone:%s\n", timezoner);
    write(ld_p->wfd, buf, strlen(buf));
    ld_p->total_write_size = 0;

    return fd;
}

/*
* rename the current logging file. Update timestamp in file name and remove '.curf' suffix
*/
void rename_logfile(struct log_dev_t *ld_p, const char* logPath) {
    char s_name[255], d_name[255];
    char curtime[64] = {'E', '\0'};
    struct tm *tmp;
    time_t lt;
    logfull_entry_t *entry = NULL;


    /* close wfd firstly */
    if (ld_p->wfd > 0) {
        close(ld_p->wfd);
        ld_p->wfd = -1;
    }

    /* get current time */
    lt = time(NULL);
    tmp = localtime(&lt);
    if (tmp != NULL) {
        strftime(curtime, sizeof(curtime), "_%Y_%m%d_%H%M%S", tmp);
    }
    /* global rotated index ++ */
    rot_idx++;
    if (rot_idx >= 1000000) {
        MLOGI_BOTH("Rotation index has reached %d, back to 1", rot_idx);
        rot_idx = 1;
    }

    /* init rotated file name without '.curf' suffix */
    snprintf(s_name, sizeof(s_name), "%s%s", logPath, ld_p->wf_name);
    snprintf(d_name, sizeof(d_name), "%s%s_%d_%s", logPath, ld_p->log_name, rot_idx, curtime);

    if (rename(s_name, d_name) < 0) {
        MLOGE_BOTH("rename %s fail (%s)", ld_p->wf_name, strerror(errno));
        return;
    } else {
        snprintf(ld_p->wf_name, sizeof(ld_p->wf_name), "%s", d_name + strlen(logPath));
        /* create a logfull entry with the rotated log file info */
        entry = (logfull_entry_t *)malloc(sizeof(struct logfull_entry));
        if (entry == NULL) {
            MLOGE_BOTH("malloc fail");
            return;
        }
        list_init(&entry->node);
        snprintf(entry->name, sizeof(entry->name), "%s", ld_p->wf_name);
        entry->size = ld_p->total_write_size;
        snprintf(entry->timestamp, sizeof(entry->timestamp), "%s", curtime + 1);

        /* insert the newest entry into the head of logfull entry list */
        list_add_head(&logf_list, &entry->node);
    }
    return;
}

/*
* make free space by deleting the oldest log file
*/
void remove_oldest_logfile() {
    logfull_entry_t *entry = NULL;
    struct listnode *p_node = NULL;
    char file_path[255];


    // whether list is empty
    if (list_empty(&logf_list)) {
        return;
    }
    // fetch the oldest entry
    p_node = list_tail(&logf_list);
    entry = node_to_item(p_node, logfull_entry_t, node);

    snprintf(file_path, sizeof(file_path), "%s%s", cur_logging_path, entry->name);
    if (remove(file_path) < 0)
        MLOGE_BOTH("remove %s fail (%s)", entry->name, strerror(errno));
    else
        MLOGI_BOTH("recycle %s", entry->name);

    // free space
    total_log_size -= entry->size;
    list_remove(p_node);
    free(entry);
}

void insert_to_buf(struct log_dev_t *ld_p, struct log_msg *logmsg) {
    char default_buf[512];
    char *out_buf;
    size_t entry_len;
    AndroidLogEntry android_entry;
    int err;
    char tempbuf[1024];

#if !(PLATFORM_VERSION == R)
    if (!strncmp(log_dev_map[0][ld_p->id], "events", 6)) {
        err = android_log_processBinaryLogBuffer(&logmsg->entry_v1,
                &android_entry, g_event_tagmap, tempbuf, sizeof(tempbuf));
    } else {
        err = android_log_processLogBuffer(&logmsg->entry_v1, &android_entry);
    }
#else
    if (!strncmp(log_dev_map[0][ld_p->id], "events", 6)) {
        err = android_log_processBinaryLogBuffer(&logmsg->entry,
                &android_entry, g_event_tagmap, tempbuf, sizeof(tempbuf));
    } else {
        err = android_log_processLogBuffer(&logmsg->entry, &android_entry);
    }
#endif

    if (err < 0) {
        MLOGE_BOTH("error while convert android entry(%s)", strerror(errno));
        return;
    }

    last_log.tv_sec = android_entry.tv_sec;
    last_log.tv_nsec = android_entry.tv_nsec;
    out_buf = android_log_formatLogLine(g_logformat, default_buf,
            sizeof(default_buf), &android_entry, &entry_len);

    if (!out_buf) {
        MLOGE_BOTH("android log format log line failed! One msg of %s log lost",
                android_log_id_to_name(logmsg->entry.lid));
        return;
    }

    if (entry_len + ld_p->buf_p->buf_used > BUFFER_SIZE) {
        ld_p->buf_p->ok = 1;
        get_free_buf(ld_p);
    }

    memcpy(ld_p->buf_p->m_buf+ld_p->buf_p->buf_used, out_buf, entry_len);
    ld_p->buf_p->buf_used += entry_len;
    if (out_buf != default_buf) {
        free(out_buf);
    }
}

/*
*/

void get_free_buf(struct log_dev_t *ld_p) {
    struct buf_util *p = NULL;
    struct timespec ts_1;
    int64_t differ = 0;

    pthread_mutex_lock(&buf_lock);
    if (ld_p->buf_p != NULL) {
        if (ld_p->buf_p->ok == 1) {
            gBufReadyCount++;
            pthread_mutex_lock(&syncLock);
            pthread_cond_signal(&syncCond);
            pthread_mutex_unlock(&syncLock);
            ld_p->buf_p->marked = true;
        }
    }

    while (p == NULL)
        p = (struct buf_util *)malloc(sizeof(struct buf_util));

    p->buf_used = 0;
    p->ok = 0;
    p->marked = false;
    memset(p->m_buf, 0x0, sizeof(p->m_buf));
    p->owner = ld_p;
    ld_p->buf_p = p;

    list_add_tail(&buflist, &p->buflist);
    pthread_mutex_unlock(&buf_lock);

    /* getrusage of mobile_log_d self process  */
    if (gBufReadyCount > 1400) {
        if (is_writing_log) {
            clock_gettime(CLOCK_MONOTONIC, &ts_1);
            differ = (ts_1.tv_sec - ts_0.tv_sec) * 1000000000LL + ts_1.tv_nsec - ts_0.tv_nsec;
            MLOGE_DATA("For 256KB, Writing duration: %"PRId64" s, %10"PRId64" ns; And still Block on write Now!",
                    differ/1000000000, differ%1000000000);
        }

        system("echo 5 4 0 > /proc/msdc_debug");
        property_set(PROP_RUNNING, "0");
        if (aee_sys_war)
            aee_sys_war("mobile_log_d", NULL, DB_OPT_DEFAULT,
                "Is writing: %d, ReadyBufCnt: %d", is_writing_log, gBufReadyCount);

        property_set("vendor.MB.ftrace.log", "off");

        abort();
    }
}

extern uint64_t STORAGE_FULL_EVENT;
extern uint64_t FOLDER_REMOVED_EVENT;
extern int event_fd;

int normalWrite(void) {
    int ret, e;
    struct listnode * p = NULL;
    struct listnode * n = NULL;
    struct buf_util *bu_p = NULL;
    struct log_dev_t *ld_p = NULL;
    /* write rate calc */
    struct timespec ts_1;
    static struct timespec ts_interval = {0, 0};
    static int64_t accuml_logsize = 0;
    int64_t differ = 0;

    ret = check_storage();
    if (ret < 0) {
        if (ret == -STORAGE_FULL) {
            uint64_t u = STORAGE_FULL_EVENT;
            TEMP_FAILURE_RETRY(write(event_fd, &u, sizeof(u)));
            MLOGI_BOTH("Storage is Full! send %llu to event_fd", (unsigned long long)u);
            /* rename .cuf when storage is full */
            list_for_each_safe(p, n, &buflist) {
                bu_p = node_to_item(p, struct buf_util, buflist);
                ld_p = bu_p->owner;
                rename_logfile(ld_p, cur_logging_path);
            }
        } else if (ret == -ENOENT) {
            uint64_t u = FOLDER_REMOVED_EVENT;
            TEMP_FAILURE_RETRY(write(event_fd, &u, sizeof(u)));
            MLOGI_BOTH("APLog folder Removed! send %llu to event_fd", (unsigned long long)u);
        } else {
            MLOGE_BOTH("Storage check Fail!");
        }
        return -1;
    }

    list_for_each_safe(p, n, &buflist) {
        /* When log current folder changed on running state, we need change all log path */
        if (g_redirect_flag) {
            g_redirect_flag = 0;
            MLOGI_BOTH("change log path before next write.");
            if (change_log_path() < 0) {
                MLOGI_BOTH("Fail to change log path.");
                return -1;
            } else {
                // clear total log size for New current logging path
                rot_idx = 0;
                total_log_size = 0;
                set_rotate_size();
            }
        }

        /*
        * Normal write stage
        */
        bu_p = node_to_item(p, struct buf_util, buflist);
        ld_p = bu_p->owner;
        if (bu_p->ok != 1 || bu_p->marked != true)
            continue;
        clock_gettime(CLOCK_MONOTONIC, &ts_0);
        is_writing_log = true;
        /* for test
        if (ts_0.tv_sec > 400 && ts_0.tv_sec < 500)
            sleep(3600 * 2);
        */
        ret = TEMP_FAILURE_RETRY(write(ld_p->wfd, bu_p->m_buf, bu_p->buf_used));
        is_writing_log = false;
        clock_gettime(CLOCK_MONOTONIC, &ts_1);
        e = errno;
        if (ret < 0) {
            MLOGE_BOTH("mobilelog write %s error, errno = %d(%s)", ld_p->log_name, e, strerror(errno));
            if (aee_sys_exp && e != ENOTCONN && e != ECONNABORTED) {
                /*ENOTCONN or ECONNABORTED happened at unmout phase, skip it*/
                aee_sys_exp("mobile_log_d", NULL, DB_OPT_DEFAULT,
                        " %s write to storage error, errno = %d(%s)", ld_p->log_name, e, strerror(e));
            }
        } else {
            /* write success, calc write rate */
            differ = (ts_1.tv_sec - ts_0.tv_sec) * 1000000000LL + ts_1.tv_nsec - ts_0.tv_nsec;
            /* For test
            MLOGI_BOTH("Write time: %lld s, %10lld ns; ts_0: %6ld s, %10ld ns; ts_1: %6ld s, %10ld ns", differ/1000000000,
                        differ%1000000000, ts_0.tv_sec, ts_0.tv_nsec, ts_1.tv_sec, ts_1.tv_nsec);
            */
            if (differ > 1000000000) {/*  write rate is too low, 256kB for over 1 second */
                MLOGI_BOTH("Alert, For 256K bytes, Write time too long: %"PRId64" s, %10"PRId64" ns",
                        differ/1000000000, differ%1000000000);
            }

            ts_interval.tv_sec += (differ + ts_interval.tv_nsec)/1000000000;
            ts_interval.tv_nsec = (differ + ts_interval.tv_nsec)%1000000000;

            accuml_logsize += ret;
            if (accuml_logsize > 1024 * 1024 * 10) {
                MLOGI_BOTH("TIME_interval: %ld seconds, %ld ns, logsize: %"PRId64"", ts_interval.tv_sec,
                        ts_interval.tv_nsec, accuml_logsize);
                ts_interval.tv_sec = 0;
                ts_interval.tv_nsec = 0;
                accuml_logsize = 0;
            }

            /* check log rotate condition  */
            ld_p->total_write_size += ret;
            total_log_size += ret;
            if (ld_p->total_write_size > ld_p->rotate_size) {
                rename_logfile(ld_p, cur_logging_path);
                ret = open_logfile(ld_p, cur_logging_path);
                if (ret <= 0) {
                    MLOGE_DATA("rotate %s fail!", ld_p->log_name);
                    return -1;
                }
            }
            if (total_log_size > folder_size_threshold) {
                remove_oldest_logfile();
            }
        }
        pthread_mutex_lock(&buf_lock);
        list_remove(p);
        free(bu_p);
        gBufReadyCount--;
        pthread_mutex_unlock(&buf_lock);
    }
    return 1;
}

/*write thread was stopped by stop command*/
int flushToStorage(void) {
    struct listnode * p = NULL;
    struct buf_util *bu_p = NULL;
    struct log_dev_t *ld_p = NULL;
    int ret;
    /* save all log buffer content */
    MLOGI_BOTH("flush mobilelog buffer before stopped...");
    list_for_each(p, &buflist) {
        bu_p = node_to_item(p, struct buf_util, buflist);
        ld_p = bu_p->owner;
        ret = TEMP_FAILURE_RETRY(write(ld_p->wfd, bu_p->m_buf, bu_p->buf_used));
        if (ret < 0) {
            MLOGE_BOTH("%s write to file fail(%s)", ld_p->log_name, strerror(errno));
            continue;
        }

        ld_p->total_write_size += ret;
        total_log_size += ret;
        if (ld_p->total_write_size > ld_p->rotate_size) {
            rename_logfile(ld_p, cur_logging_path);
            ret = open_logfile(ld_p, cur_logging_path);
            if (ret < 0) {
                MLOGE_DATA("rotate %s failed.", ld_p->log_name);
                return -1;
            }
        }
    }

    /* close all logging files. And update the file name */
    list_for_each(p, &devlist) {
        ld_p = node_to_item(p, struct log_dev_t, devlist);
        rename_logfile(ld_p, cur_logging_path);
    }

    return 1;
}

int check_storage() {
    struct statfs path_st;
    unsigned long long avail_size;
    char prop_buf[PROPERTY_VALUE_MAX];
    int ret, e, count = 10;
    memset(&path_st, 0, sizeof(path_st));
    while (count) {
        if (!property_get(PROP_PATH, prop_buf, ""))
            return 0;
        ret = statfs(prop_buf, &path_st);
        e = errno;
        if (ret == 0)
            break;
        if (ret < 0) {
            if ((e == EINTR) || (e == ENOTCONN)) {
                int r;
                count--;
                r = usleep(100000);
                if (r < 0) {
                    MLOGE_BOTH("usleep error %s", strerror(errno));
                }
                continue;
            } else if (e == ENOENT) {
                MLOGE_BOTH("WARNING: %s disappeared!!", prop_buf);
                return -ENOENT;
            } else {
                MLOGE_BOTH("statfs(%s): %s", prop_buf, strerror(e));
                return -e;
            }
        }
    }

    if (count == 0) {
        MLOGE_BOTH("statfs(%s): %s", prop_buf, strerror(e));
        return -e;
    }

    avail_size = path_st.f_bavail * path_st.f_bsize;
    // internal sdcard space is limited by QUOTA feature, but ext-sdcard not
    if (sdContext.location == INTERNAL_SD) {
        unsigned long long quota_reserve = path_st.f_blocks * path_st.f_bsize / QUOTA_RATIO;

        if (avail_size < quota_reserve + REMAIN_SIZE) {
            MLOGE_BOTH("Remain space: %lluMB, will trigger QUOTA reserved level: %lluMB = "
                "bsize(%zu Byte) * blocks(%"PRIu64") * %d percents.",
                avail_size / 1024 / 1024, quota_reserve / 1024 / 1024,
                path_st.f_bsize, path_st.f_blocks, QUOTA_RATIO);
            return -STORAGE_FULL;
        }
    } else {
        if (avail_size < REMAIN_SIZE) {
            MLOGE_BOTH("storage size is less than %lluMB", REMAIN_SIZE / 1024 / 1024);
            return -STORAGE_FULL;
        }
    }

    return 0;
}

int change_log_path() {
    struct log_dev_t *ld_p = NULL;
    struct listnode *dev_p = NULL;
    int ret;

    logfull_entry_t *entry = NULL;
    struct listnode *p_node = NULL;
    struct listnode *n = NULL;


    /* close And update the file name */
    list_for_each(dev_p, &devlist) {
        ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
        rename_logfile(ld_p, last_logging_path);
    }

    /* Can_Do_Copy Now */
    g_copy_wait = 0;

    // clear 'logf_list'
    list_for_each_safe(p_node, n, &logf_list) {
        entry = node_to_item(p_node, logfull_entry_t, node);
        list_remove(p_node);
        free(entry);
    }

    // create log files on new path
    list_for_each(dev_p, &devlist) {
        ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
        ret = open_logfile(ld_p, cur_logging_path);
        if (ret < 0) {
            MLOGI_BOTH("Fail to create log file.");
            return -1;
        }
    }

    return 1;
}


void write_thread() {
    prctl(PR_SET_NAME, "mobile_log_d.wr");
    MLOGI_BOTH("write thread %d", gettid());
    struct timeval now;
    struct timespec timeout;
    int ret;
    /* detect while loop */
    static int while_cnt1;
    while (true) {
        pthread_mutex_lock(&syncLock);
        while (gBufReadyCount == 0 && g_mblog_status != STOPPED) {
            /* set timeout 60s to avoid dead lock */
            gettimeofday(&now, NULL);
            timeout.tv_sec = now.tv_sec + 60;
            timeout.tv_nsec = now.tv_usec * 1000;
            ret = pthread_cond_timedwait(&syncCond, &syncLock, &timeout);
            if (ret == ETIMEDOUT)
                ALOGD("conditional wait timedout");

            /* When log current folder changed on running state, we need change all log path */
            if (g_redirect_flag) {
                g_redirect_flag = 0;
                MLOGI_BOTH("change log path.");
                if (change_log_path() < 0) {
                    MLOGI_BOTH("Fail to change log path. mobile_log_d.wr exit!");
                    writer_stop_flag = 1;
                    pthread_exit(0);
                } else {
                    // clear total log size for New current logging path
                    rot_idx = 0;
                    total_log_size = 0;
                    set_rotate_size();
                }
            }
        }
        pthread_mutex_unlock(&syncLock);

        if (g_mblog_status == STOPPED) {
            flushToStorage();
            MLOGI_BOTH("mobile_log_d.wr exit!");
            pthread_exit(0);
        } else {
            ret = normalWrite();
            if (ret < 0) {
                MLOGI_BOTH("mobile_log_d.wr exit!");
                writer_stop_flag = 1;
                pthread_exit(0);
            }
        }

        /* detect while loop */
        {
            while (while_cnt1++ > 800000) {  // 80000(1 second) is a preferable threshold
                while_cnt1 = 0;
                MLOGE_BOTH("while end, BufReadyCount %d", gBufReadyCount);
            }
        }
    }
}

void read_thread(void) {
    prctl(PR_SET_NAME, "mobile_log_d.rd");
    int id;
    int max, ret;
    fd_set log_fd;
    long long elapsed = 0;
    struct log_dev_t *ld_p;
    struct listnode *dev_p;
    struct timeval timeout, curtime, lasttime;
    struct timeval maxElaps;

    MLOGI_BOTH("read thread %d", gettid());
    gettimeofday(&lasttime, NULL);
    if (g_bootmode == META_BOOT || g_bootmode == FACTORY_BOOT || g_bootmode == ATE_FACTORY_BOOT) {
        maxElaps.tv_sec = 2;
        maxElaps.tv_usec = 0;
    } else {
        maxElaps.tv_sec = 60;
        maxElaps.tv_usec = 0;
    }

    while (1) {
        if (g_mblog_status == STOPPED || writer_stop_flag ==1) {
            pthread_mutex_lock(&syncLock);
            pthread_cond_signal(&syncCond);
            pthread_mutex_unlock(&syncLock);
            stop_logging();
            MLOGI_BOTH("mobile_log_d.rd exit!");
            pthread_exit(0);
        }
        /* wake up writer thread to change log path */
        if (g_redirect_flag) {
            pthread_mutex_lock(&syncLock);
            pthread_cond_signal(&syncCond);
            pthread_mutex_unlock(&syncLock);
            MLOGI_BOTH("Read g_redirect_flag~");
        }

        max = -1;
        int logdr_has_add = 0;
        FD_ZERO(&log_fd);
        list_for_each(dev_p, &devlist) {
            ld_p = node_to_item(dev_p, struct log_dev_t, devlist);

            /* some kinds of MOBILOG may not finish intialization */
            if (ld_p->rfd == -1 && ld_p->type == DIRECT_DEV) {
                id = ld_p->id;
                if (!access(log_dev_map[0][id], F_OK)) {
                    MLOGE_BOTH("%s is ready now, and open it.", log_dev_map[0][id]);
                    if (open_logger_dev(ld_p) == 0)
                        ld_p->rfd = -2;  // open Fail, this kind of MOBILOG will be ignored
                }
            }

            if (ld_p->type == ANDROID_DEV) {
                if (logdr_has_add) {
                    continue;
                } else {
                    if (ld_p->rfd > 0) {
                        FD_SET(ld_p->rfd, &log_fd);
                        logdr_has_add = 1;
                    }
                }
            } else if (ld_p->rfd > 0) {
                FD_SET(ld_p->rfd, &log_fd);
            }
            if (max < ld_p->rfd) {
                max = ld_p->rfd;
            }
        }

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        ret = select(max+1, &log_fd, NULL, NULL, &timeout);
        if (ret == 0) {
            continue;
        }

        int logdr_has_read = 0;

        list_for_each(dev_p, &devlist) {
            ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
            if (ld_p->rfd > 0) {
                if (FD_ISSET(ld_p->rfd, &log_fd)) {
                    if (ld_p->type == ANDROID_DEV && !logdr_has_read) {
                        /* Read Android log */
                        android_log_read();
                        logdr_has_read = 1;
                    } else if (ld_p->type == DIRECT_DEV) {
                        direct_log_read(ld_p);
                    }
                }
            }
        }

        gettimeofday(&curtime, NULL);
        elapsed = (1000000LL * (curtime.tv_sec - lasttime.tv_sec) + (curtime.tv_usec - lasttime.tv_usec));
        if ((g_mblog_status != STOPPED) && (elapsed > maxElaps.tv_sec * 1000000LL || elapsed < 0)) {
            list_for_each(dev_p, &devlist) {
                ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
                ld_p->buf_p->ok = 1;
                get_free_buf(ld_p);
            }
            lasttime = curtime;
        }
        if (g_mblog_status == STOPPING) {
            if (stopping_time.tv_sec == 0) {
                stopping_time.tv_sec = curtime.tv_sec;
                stopping_time.tv_nsec = curtime.tv_usec * 1000;
                MLOGI_BOTH("stopping time %"PRIu32".%09"PRIu32"", stopping_time.tv_sec, stopping_time.tv_nsec);
                MLOGI_BOTH("Last log time %"PRIu32".%09"PRIu32"", last_log.tv_sec, last_log.tv_nsec);
            }
            if (stopping_time.tv_sec < last_log.tv_sec) {
                g_mblog_status = STOPPED;
                stopping_time.tv_sec = 0;
            } else if (stopping_time.tv_sec == last_log.tv_sec) {
                if (stopping_time.tv_nsec < last_log.tv_nsec) {
                    g_mblog_status = STOPPED;
                    stopping_time.tv_sec = 0;
                }
            } else {
                if (stopping_time.tv_sec > last_log.tv_sec + 60) {
                    g_mblog_status = STOPPED;
                    stopping_time.tv_sec = 0;
                }
            }
        }
    }
}

void set_rotate_size(void) {
    int logsize = 0;
    int factor = 1;
    struct log_dev_t *ld_p;
    struct listnode *p_node;

    logsize = atoi(read_bootmode_config(KEY_SIZE));
    if (logsize < 100) {
        MLOGI_BOTH("current folder size %d, can not less than 100", logsize);
        logsize = 100;
    }

    if (logsize < 500) {
        factor = 1;
    } else if (logsize >= 500 && logsize < 1000) {
        factor = 2;
    } else if (logsize >= 1000 && logsize < 1500) {
        factor = 3;
    } else
        factor = 6;

    // set current folder size threshold
    folder_size_threshold = (logsize - 20) * 1024LL * 1024LL;  // MBytes
    MLOGI_BOTH("current folder size limited is %d MB, factor is %d", logsize, factor);

    list_for_each(p_node, &devlist) {
        ld_p = node_to_item(p_node, struct log_dev_t, devlist);
        ld_p->rotate_size *= factor;
        MLOGI_BOTH("%s, rotation size %lu", ld_p->log_name, ld_p->rotate_size);
    }
}

// Property helper
static bool check_flag(const char *prop, const char *flag) {
    const char *cp = strcasestr(prop, flag);
    if (!cp) {
        return false;
    }
    // We only will document comma (,)
    static const char sep[] = ",:;|+ \t\f";
    if ((cp != prop) && !strchr(sep, cp[-1])) {
        return false;
    }
    cp += strlen(flag);
    return !*cp || !!strchr(sep, *cp);
}


int kernel_log_source() {
    char build_type[PROPERTY_VALUE_MAX];
    char is_lowram[PROPERTY_VALUE_MAX];
    char logd_k[PROPERTY_VALUE_MAX];

    property_get("ro.debuggable", build_type, "");
    property_get("ro.config.low_ram", is_lowram, "false");

    property_get("ro.logd.kernel", logd_k, "");

    if (!strcmp(is_lowram, "true") || check_flag(logd_k, "false")
            || strcmp(build_type, "1"))
        return KERNEL_KMSG;
    else
        return KERNEL_LOGD;
}

/*
*
*
* prepare logging all kinds of data
*
*
*/
int prepare_logging() {
    struct log_dev_t *ld_p = NULL;
    struct listnode *dev_p = NULL;
    pthread_attr_t attr;
    long int sublogConfig = 0;
    char sublog_buf[PROPERTY_VALUE_MAX];
    int ret = 0;
    // global var reset
    gBufReadyCount = 0;
    writer_stop_flag = 0;

    property_set(PROP_PREPARE, "1");
    /* on/off log from config item */
    for (int i = MOBILOG_ID_MIN; i < MOBILOG_ID_MAX; i++) {
        if (strlen(SUBLOG_NAME[i][0])) {
            log_control_map[0][i] =
                atoi(read_bootmode_config(SUBLOG_NAME[i][0]));
            if (log_control_map[0][i] == 1)
                sublogConfig |= 1 << i;
        } else if (i > MOBILOG_ID_MIN) {
            log_control_map[0][i] = log_control_map[0][i - 1];
        }
    }

    /*  If not all mode use unified configs, then config items for Android log and kernel log only affect
    *    normal mode. In other mode (meta/factory), no matter what values of the two configs are,
    *    we always enable logging Android log and kernel log.
    */
    if (!gAllMode &&
        (g_bootmode == META_BOOT || g_bootmode == FACTORY_BOOT || g_bootmode == ATE_FACTORY_BOOT)) {
        for (int i = MOBILOG_ID_MIN; i <= MOBILOG_ID_KERNEL; i++) {
            log_control_map[0][i] = 1;
        }
        sublogConfig |= (1 << MOBILOG_ID_MIN) + (1 << MOBILOG_ID_KERNEL);
    }

    snprintf(sublog_buf, sizeof(sublog_buf), "0x%08lX", sublogConfig);
    MLOGI_DATA("sublogs status: %s", sublog_buf);
    property_set(PROP_SUBLOG, sublog_buf);

    // hard code special log
    log_control_map[0][MOBILOG_ID_STATS] =
    log_control_map[0][MOBILOG_ID_SECURITY] = 0;

    log_control_map[0][MOBILOG_ID_GZ] = 1;
    log_control_map[0][MOBILOG_ID_MMEDIA] = 0;  // Mmdedia log is disable temporarily now

    // global var init
    has_android_log  = log_control_map[0][MOBILOG_ID_MAIN];
    has_kernel_log   = log_control_map[0][MOBILOG_ID_KERNEL];
    klog_src         = kernel_log_source();

    /* create log_dev list basic on log_control_map */
    create_log_devlist();

    /* set Android log format */
    if (has_android_log || (has_kernel_log && klog_src == KERNEL_LOGD)) {
        if (g_event_tagmap == NULL) {
            g_event_tagmap = android_openEventTagMap(EVENT_TAG_MAP_FILE);
            if (g_event_tagmap == NULL) {
                MLOGE_DATA("open tagmap fail");
            } else
                MLOGE_DATA("open tagmap done");
        }
        g_logformat = android_log_format_new();
        set_android_log_format("usec");
        set_android_log_format("threadtime");
    }

    /* set all kinds of logs rotation size, and clear rotated index */
    rot_idx = 0;
    total_log_size = 0;
    set_rotate_size();

    /* set current logging path */
    if (access(cur_logging_path, F_OK) != 0) {
        create_dir(cur_logging_path);
    }
    property_set(PROP_PATH, cur_logging_path);

    ret = usleep(500000); /* wait 500ms */
    if (ret < 0) {
        MLOGE_BOTH("usleep error %s", strerror(errno));
    }

    /* open (create ) log file */
    list_for_each(dev_p, &devlist) {
        int ret;
        ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
        get_free_buf(ld_p);
        ret = open_logfile(ld_p, cur_logging_path);
        if (ret < 0) {
            MLOGE_BOTH("Fail to open log file while preparing stage!");
            property_set(PROP_PREPARE, "0");
            return 0;
        }
    }

    last_log.tv_sec = 0;
    stopping_time.tv_sec = 0;
    /* wait until all other things have been down, then create logd socket to avoid logd memory leak */
    logd_sock = connect_to_logdr(has_android_log, has_kernel_log && klog_src == KERNEL_LOGD);
    if (logd_sock < 0)
        MLOGI_BOTH("logd_sock: %d, Not connect to logd", logd_sock);
    else {
        MLOGI_BOTH("logd_sock: %d", logd_sock);
        list_for_each(dev_p, &devlist) {
            ld_p = node_to_item(dev_p, struct log_dev_t, devlist);
            switch (ld_p->id) {
                case MOBILOG_ID_MAIN ... MOBILOG_ID_SECURITY:
                    ld_p->rfd = logd_sock;
                    break;
                case MOBILOG_ID_KERNEL:
                    if (klog_src == KERNEL_LOGD)
                        ld_p->rfd = logd_sock;
                    break;
            }
        }
    }

    /* set read_thread detached */
    if (!pthread_attr_init(&attr)) {
        if (!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
            if (!pthread_create(&g_read_thread_t, &attr, (void *)read_thread, NULL))
                pthread_attr_destroy(&attr);
        }
    }
    /* set write thread joinable, which will joined by read_thread */
    pthread_create(&g_write_thread_t, NULL, (void *)write_thread, NULL);

    property_set(PROP_RUNNING, "1");

    if ((customId == CUSTOME_ID_GOD_A) && (g_bootmode == NORMAL_BOOT)) {
        property_set(PROP_CONTROL, "copy");
    }

    init_debug_config();
    property_set(PROP_PREPARE, "0");

    return 1;
}

int set_android_log_format(const char * str) {
    static AndroidLogPrintFormat format;

    format = android_log_formatFromString(str);

    if (format == FORMAT_OFF) {
        return -1;
    }

    return android_log_setPrintFormat(g_logformat, format);
}

void stop_logging(void) {
    struct log_dev_t *ld_p = NULL;
    struct buf_util *bu_p = NULL;
    struct listnode *p = NULL;
    struct listnode *n = NULL;

    logfull_entry_t *entry = NULL;
    struct listnode *p_node = NULL;

    int has_scp_log = 0, has_sspm_log = 0, has_adsp_log = 0, sock_closed = 0;
    int has_mcupm_log = 0;


    MLOGE_BOTH("stop logging...");

    deinit_debug_config();

    pthread_join(g_write_thread_t, NULL);   // block until write thread exit

    list_for_each_safe(p, n, &devlist) { /* release the remained device */
        ld_p = node_to_item(p, struct log_dev_t, devlist);
        if (ld_p->type == ANDROID_DEV) { /* Android socket should only close once */
            if (!sock_closed) {
                if (logd_sock > 0) {
                    close(logd_sock);
                    logd_sock = -1;
                    sock_closed = 1;
                }
            }
        } else {
            if (ld_p->rfd > 0) {
                close(ld_p->rfd);
                ld_p->rfd = -1;
            }
        }
        if (ld_p->wfd > 0) {
            close(ld_p->wfd);
            ld_p->wfd = -1;
        }
        list_remove(p);
        free(ld_p);
    }

    list_for_each_safe(p, n, &buflist) {
        bu_p = node_to_item(p, struct buf_util, buflist);
        list_remove(p);
        free(bu_p);
    }

    // clear 'logf_list'
    list_for_each_safe(p_node, n, &logf_list) {
        entry = node_to_item(p_node, logfull_entry_t, node);
        list_remove(p_node);
        free(entry);
    }

    has_scp_log = atoi(read_bootmode_config(KEY_SCP));
    has_sspm_log = atoi(read_bootmode_config(KEY_SSPM));
    has_adsp_log = atoi(read_bootmode_config(KEY_ADSP));
    has_mcupm_log = atoi(read_bootmode_config(KEY_MCUPM));
    if (has_scp_log)
        disable_scp_poll();
    if (has_sspm_log)
        disable_sspm_poll();
    if (has_adsp_log)
        disable_adsp_poll();
    if (has_mcupm_log) {
        disable_mcupm_poll();
    }

    property_set(PROP_RUNNING, "0");
    property_set("vendor.MB.ftrace.log", "off");
}


int connect_to_logdr(int android_log, int kernel_log) {
    int sock;

    // if no need to connect to logdr
    if (android_log == 0 && kernel_log == 0)
        return -1;

    sock = socket_local_client("logdr",
            ANDROID_SOCKET_NAMESPACE_RESERVED,
            SOCK_SEQPACKET);
    if (sock < 0) {
        MLOGE_DATA("connect to logdr fail, %s", strerror(errno));
        return sock;
    }

    char cmd[96];
    memset(cmd, 0x00, sizeof(cmd));
    if (android_log && kernel_log)
        snprintf(cmd, sizeof(cmd), "stream lids=%d,%d,%d,%d,%d,%d",
                LOG_ID_MAIN, LOG_ID_RADIO, LOG_ID_EVENTS, LOG_ID_SYSTEM, LOG_ID_CRASH, LOG_ID_KERNEL);
    else if (android_log)
        snprintf(cmd, sizeof(cmd), "stream lids=%d,%d,%d,%d,%d",
                LOG_ID_MAIN, LOG_ID_RADIO, LOG_ID_EVENTS, LOG_ID_SYSTEM, LOG_ID_CRASH);
    else if (kernel_log)
        snprintf(cmd, sizeof(cmd), "stream lids=%d", LOG_ID_KERNEL);

    if (last_log.tv_sec != 0) {
        struct timespec ts;
        clock_gettime(android_log_clockid(), &ts);
        if (ts.tv_sec > ((long) last_log.tv_sec  + 30*60) || (ts.tv_sec < (long) last_log.tv_sec )) {
            /*may be change phone time in this time*/
            last_log.tv_sec  = ts.tv_sec - 10;
            last_log.tv_nsec  = 0;
        }
        snprintf(cmd + strlen(cmd), sizeof(cmd)- strlen(cmd), " start=%" PRIu32
            ".%09" PRIu32, last_log.tv_sec, last_log.tv_nsec);
        MLOGI_BOTH("re-connect %s.", cmd);
    }

    TEMP_FAILURE_RETRY(write(sock, cmd, strlen(cmd)));
    return sock;
}

int enable_scp_poll() {
    char cmd_a[] = "echo 1 > /sys/class/misc/scp/scp_mobile_log";
    char cmd_b[] = "echo 1 > /sys/class/misc/scp_B/scp_B_mobile_log";
    system(cmd_a);
    system(cmd_b);
    return 1;
}

int disable_scp_poll() {
    char cmd_a[] = "echo 0 > /sys/class/misc/scp/scp_mobile_log";
    char cmd_b[] = "echo 0 > /sys/class/misc/scp_B/scp_B_mobile_log";
    system(cmd_a);
    system(cmd_b);
    return 1;
}

int enable_sspm_poll() {
    char cmd[] = "echo 1 > /sys/class/misc/sspm/sspm_mobile_log";
    system(cmd);
    return 1;
}

int disable_sspm_poll() {
    char cmd[] = "echo 0 > /sys/class/misc/sspm/sspm_mobile_log";
    system(cmd);
    return 1;
}

int enable_adsp_poll() {
    char cmd[] = "echo 1 > /sys/class/misc/adsp/adsp_mobile_log";
    system(cmd);
    return 1;
}

int disable_adsp_poll() {
    char cmd[] = "echo 0 > /sys/class/misc/adsp/adsp_mobile_log";
    system(cmd);
    return 1;
}

int enable_mcupm_poll() {
    char cmd[] = "echo 1 > /sys/class/misc/mcupm/mcupm_mobile_log";
    system(cmd);
    return 1;
}

int disable_mcupm_poll() {
    char cmd[] = "echo 0 > /sys/class/misc/mcupm/mcupm_mobile_log";
    system(cmd);
    return 1;
}
