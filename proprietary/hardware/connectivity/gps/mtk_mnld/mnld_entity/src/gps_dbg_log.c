/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>

#include "mnld_utile.h"
#include "mtk_lbs_utility.h"
#include "data_coder.h"
#include "mnld.h"
#include "mtk_gps.h"
#include "mtk_gps_type.h"

#include "gps_dbg_log.h"
#include "mpe.h"
#include "LbsLogInterface.h"


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
#define LOGD(...) tag_log(1, "[gps_dbg_log]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gps_dbg_log] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gps_dbg_log] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "gps_dbg_log"

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
/*Close the GPS debug log file, and rename to ".nma".
  *fd: file descriptor, should be a global variable for NULL check;
  *fn: Current writing file name;

#define GPSLOG_FCLOSE(fd,fn) do {\
    if (NULL != fd) {\
        fclose(fd);\
        fd = NULL;\
        gps_log_file_rename(fn);\
        property_set(GPS_LOG_PERSIST_FLNM, GPS_LOG_PERSIST_VALUE_NONE);\
    }\
} while (0)
*/
#ifndef MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

#define GPSLog_TIMESTRING_LEN 25
#define C_INVALID_TID  (-1)   /*invalid thread id*/

unsigned char gps_debuglog_state = MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD;
char gps_debuglog_file_name[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = "/mnt/sdcard/debuglogger/connsyslog/gpshost/GPS_HOST";
char storagePath_mtklogger_set[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = "/mnt/sdcard/debuglogger/connsyslog/gpshost/";
char storagePath[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = "/mnt/sdcard/debuglogger/connsyslog/gpshost/";
static char log_filename_suffix[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = "GPS_HOST";
static char gsGpsLogFileName[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
static char gsGpsLogFileName_full[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
static int total_file_count = 0;
static int Current_FileSize = 0;
static int Filenum = 0;
static int DirLogSize = 0;
UINT32 g_dbglog_file_size_in_config = 0;   //Max dbg log file size read from config file
UINT32 g_dbglog_folder_size_in_config = 0;   //Max dbg log folder size read from config file
static int fg_create_dir_done = 0;
pthread_mutex_t g_mnldMutex[MNLD_MUTEX_MAX];

FILE* g_hLogFile = NULL;
bool g_gpsdbglogThreadExit = true;
bool g_pingpang_init = false;
pthread_mutex_t FILE_WRITE_MUTEX;
pthread_t pthread_dbg_log = C_INVALID_TID;

#define PINGPANG_WRITE_LOCK 0
#define PINGPANG_FLUSH_LOCK 1

static SYNC_LOCK_T lock_for_sync[] = {{PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0},
                                      {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 1}};

//  data
#define DATA_MAX_SEND2HIDL (15*1024)
#define PINGPANG_BUFFER_SIZE         (25*1024)
#define GPSDBGLOG_EXIT_RETRY_SLEEP (20*1000) //20ms
#define GPSDBGLOG_EXIT_RETRY_CNT (100) //Total retry time is 100*20ms
#define GPSDBGLOG_EXIT_RETRY_SGL_INTERVAL (25)  //send signal every 500ms
#define GPSDBGLOG_EXIT_SIG (SIGRTMIN+1)

// describle one single buffer
typedef enum {
    NOTINIT = 0,
    WRITABLE,  // the state which can swtich to writing state
    WRITING,   // means the buffer is writing
    READABLE,  // means mtklogger thread now can read this buffer and write data to flash
    READING    // means mtklogger thread is writing data to flash
} buffer_state;

typedef struct {
    char* volatile next_write;
    char* start_address_buffer1;
    char* start_address_buffer2;
    char* end_address_buffer1;
    char* end_address_buffer2;
    // int* p_buffer1_lenth_to_write;
    // int* p_buffer2_lenth_to_write;
    // buffer_state* p_buffer1_state;
    // buffer_state* p_buffer2_state;
    int volatile buffer1_lenth_to_write;
    int volatile buffer2_lenth_to_write;
    buffer_state volatile buffer1_state;
    buffer_state volatile buffer2_state;
    int num_loose;
} ping_pang_buffer;
// static buffer_state buffer1_state = NOTINIT;
// static buffer_state buffer2_state = NOTINIT;
// static int lenth_to_write_buffer1 = 0;
// static int lenth_to_write_buffer2 = 0;

mtk_socket_fd gpslogd_fd;

static ping_pang_buffer ping_pang_buffer_body;

// function related
// called when mtklogger thread init if debugtype set true
static INT32 create_debug_log_file();
static INT32 mtklog_gps_set_debug_file(char* file_name);
static INT32 gps_dbg_log_pingpang_init();
// called in function mnl_sys_alps_nmea_output
static INT32 gps_dbg_log_pingpang_copy(ping_pang_buffer* pingpang, const char* buffer, INT32 len);
// called in mtklogger thread when it is need to write
static bool gps_dbg_log_pingpang_write(ping_pang_buffer* pingpang);
// called when debugtype set 1 to 0 or  mtklogger thread exit
static INT32 gps_dbg_log_pingpang_free();
// called when debugtype set 1 to 0 or  mtklogger thread exit
//static INT32 gps_dbg_log_pingpang_flush(ping_pang_buffer* pingpang, FILE * filp);
static bool gps_dbg_log_pingpang_flush(ping_pang_buffer* pingpang);

void gps_dbg_log_thread_exit(int sig_no);

static void mnld_create_mutex( mnld_mutex_enum mutex_idx);

static void mnld_destroy_mutex(mnld_mutex_enum mutex_idx);

static void mnld_take_mutex(mnld_mutex_enum mutex_idx);

static void mnld_give_mutex(mnld_mutex_enum mutex_idx);

// GPS log sender
bool mnld2logd_open_gpslog(char* file) {
    return LbsLogInterface_openLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_GPS_LOG, file);
}

bool mnld2logd_write_gpslog(char * data, int len) {
    return LbsLogInterface_writeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_GPS_LOG, data, len);
}

bool mnld2logd_close_gpslog(void) {
    return LbsLogInterface_closeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_GPS_LOG);
}

bool mnld2logd_write_gpslog_subpackage(char * data, int len) {
    unsigned int n = 0;
    unsigned int i = 0;
    char buff[DATA_MAX_SEND2HIDL+1];

    if ((len>PINGPANG_BUFFER_SIZE-2) || (len<=0)) {
        LOGE("len = %d, wrong lenth, return", len);
        return false;
    }


    n = (len + (DATA_MAX_SEND2HIDL-1))/DATA_MAX_SEND2HIDL;
    for (i=0; i<n; i++) {
        memset(buff, 0x0, sizeof(buff));
        memcpy(buff, data+i*DATA_MAX_SEND2HIDL, MIN(DATA_MAX_SEND2HIDL,len-i*DATA_MAX_SEND2HIDL));
        if (!mnld2logd_write_gpslog(buff, MIN(DATA_MAX_SEND2HIDL,len-i*DATA_MAX_SEND2HIDL))) {
            LOGE("send gps log to HIDL fail(%s)", strerror(errno));
        }
    }
    LOGD("send total %u bytes of gps log", len);
    return true;
}


// MPE log sender
bool mnld2logd_open_mpelog(char* file) {
    return LbsLogInterface_openLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_MPE_LOG, file);
}

bool mnld2logd_write_mpelog(char * data, int len) {
    return LbsLogInterface_writeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_MPE_LOG, data, len);
}

bool mnld2logd_close_mpelog(void) {
    return LbsLogInterface_closeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_MPE_LOG);
}


// DUMP log sender
bool mnld2logd_open_dumplog(char* file) {
    return LbsLogInterface_openLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_DUMP_LOG, file);
}

bool mnld2logd_write_dumplog(char * data, int len) {
    return LbsLogInterface_writeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_DUMP_LOG, data, len);
}

bool mnld2logd_close_dumplog(void) {
    return LbsLogInterface_closeLog(&gpslogd_fd, LBS_LOG_INTERFACE_LOG_CATEGORY_DUMP_LOG);
}


static void mnld_create_mutex( mnld_mutex_enum mutex_idx){
    // mutex index is not within the supported range
    if (mutex_idx >= MNLD_MUTEX_MAX)
    {
        LOGD("mnld_create_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_init(&g_mnldMutex[mutex_idx], NULL))
    {
        LOGD("mnld_create_mutex fail(%s)\n", strerror(errno));
    }
}

static void mnld_destroy_mutex(mnld_mutex_enum mutex_idx){

    if (mutex_idx >= MNLD_MUTEX_MAX)
    {
        LOGD("mnld_destroy_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_destroy(&g_mnldMutex[mutex_idx]))
    {
        LOGD("mnld_destroy_mutex fail(%s)\n", strerror(errno));
    }
}

static void mnld_take_mutex(mnld_mutex_enum mutex_idx){

    if (mutex_idx >= MNLD_MUTEX_MAX)
    {
        LOGD("mnld_take_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_lock(&g_mnldMutex[mutex_idx]))
    {
        LOGD("mnld_take_mutex fail(%s)\n", strerror(errno));
    }
}

static void mnld_give_mutex(mnld_mutex_enum mutex_idx){

    if (mutex_idx >= MNLD_MUTEX_MAX)
    {
        LOGD("mnld_give_mutex fail,mutex_idx error\n");
        return;
    }

    if (pthread_mutex_unlock(&g_mnldMutex[mutex_idx]))
    {
        LOGD("mnld_give_mutex fail(%s)\n", strerror(errno));
    }
}

void gps_stop_dbglog_release_condition(void) {
    g_gpsdbglogThreadExit = true;
    release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
    release_condition(&lock_for_sync[PINGPANG_FLUSH_LOCK]);
    LOGD("exit while, gps_dbg_log thread exit\n");
}

/*****************************************************************************
 * FUNCTION
 *  gps_dbg_log_exit_flush
 * DESCRIPTION
 *  Set exit flag to true and release condition let gps debug log thread exit.
 *  Will do GPS debug log flush and pingpang buffer free when thread exit.
 *  And wait seconds for flush(fwrite) blocking issue.
 * PARAMETERS
 *  force_exit      [IN] force exit mnld process, to avoid resource leakage issue.
 * RETURNS
 *  None
 *****************************************************************************/
void gps_dbg_log_exit_flush(int force_exit) {
    //int err = 0;
    int cnt = 0;

    gps_stop_dbglog_release_condition(); //Exit gps debug log thread, will flush gps debug log when exit
    do {
        if (pthread_dbg_log == C_INVALID_TID)//The thread has exited, will clear pthread_dbg_log at gps dbg log thread exit time
        {
            LOGD("gpsdbglog thread exit OK!");
            break;
        } else {//The thread still alive
            if((cnt % GPSDBGLOG_EXIT_RETRY_SGL_INTERVAL == 0)) {
                gps_stop_dbglog_release_condition(); //Exit gps debug log thread, will flush gps debug log when exit
            }
            usleep(GPSDBGLOG_EXIT_RETRY_SLEEP);
            //Wait here to flush gps debug log, to avoid blocked in flush(fwrite)
            cnt++;
        }
    } while(cnt < GPSDBGLOG_EXIT_RETRY_CNT);

    if(cnt >= GPSDBGLOG_EXIT_RETRY_CNT) {
        LOGW("GPS debug log may be not flushed completely!!!");
        if(force_exit == 1) {  //Process exit to avoid resource leakage issue, process will restart
            LOGE("mnld will exit due to block at flushing GPS debug log!!!");
            _exit(0);//Exit process directly, and mnld will be restarted. Some GPS debug log will be lost.
            //Remove process dump due to it will take a long time to dump process backtraces when system loading is heavy, and easy to tigger false NE and EE.
        }
    }
}

void gps_log_file_rename(char *filename_cur) {
    char tmp_suffix = 'a'-1;
    char filename1[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    char filename2[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    int indx = 0;

    if (filename_cur == NULL) {
        LOGE("[gps_log_file_rename]: NULL Pointer!!!");
        return;
    }

    MNLD_STRNCPY(filename1, filename_cur, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
    indx = strlen(filename1) - strlen(LOG_FILE_WRITING_EXTEN_NAME);

    if(strncmp(&(filename1[indx]), LOG_FILE_WRITING_EXTEN_NAME, strlen(LOG_FILE_WRITING_EXTEN_NAME)) != 0) {
        LOGE("[gps_log_file_rename]: file extension name not match:%s", filename1);
        return;
    }

    filename1[indx] = '\0';//remove ".nmac"

    //Get new file name , storage into filename2
    do {
        if(tmp_suffix < 'a') {
            snprintf(filename2, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s"LOG_FILE_EXTEN_NAME, filename1);
        } else if(tmp_suffix <= 'z') {
            snprintf(filename2, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s_%c"LOG_FILE_EXTEN_NAME, filename1, tmp_suffix);
        } else {
            snprintf(filename2, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s"LOG_FILE_EXTEN_NAME, filename1);
            break;
        }
        tmp_suffix++;
    } while (access(filename2, F_OK) == 0);  //File has existed

    LOGD("Writing file: %s, rename to:%s", filename_cur, filename2);
    if(rename(filename_cur, filename2) != 0) {
       LOGE("[gps_log_file_rename]:rename fail:%s", strerror(errno));
    }
}

static int get_mtklog_path(char *logpath) {
    //char mtklogpath[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    char temp[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    unsigned int len;

    char* ptr;
    ptr = strchr(logpath, ',');
    if (ptr) {
        MNLD_STRNCPY(temp, ptr + 1, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        LOGD("logpath for mtklogger socket msg: %s", temp);
    } else {
        LOGD("logpath for mtklogger socket msg has not ',': %s", temp);
        MNLD_STRNCPY(logpath, LOG_FILE_PATH, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        return 0;
    }

    len = strlen(temp);
    if (len != 0  && temp[len-1] != '/') {
        temp[len++] = '/';
        if (len < GPS_DEBUG_LOG_FILE_NAME_MAX_LEN) {
            temp[len] = '\0';
        }
    }
    if (len <= GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - strlen(PATH_SUFFIX)) {
        sprintf(logpath, "%sdebuglogger/connsyslog/gpshost/", temp);
        LOGD("get_mtklog_path:logpath is %s", logpath);
    }
    /*
    if (len <= GPS_DEBUG_LOG_FILE_NAME_MAX_LEN-7) {
        snprintf(mtklogpath, sizeof(mtklogpath), "%smtklog/connsyslog/", temp);
        if (0 != access(mtklogpath, F_OK)) {    // if mtklog dir is not exit, mkdir
             LOGD("access dir error(%s), Try to create dir", mtklogpath);
             if (mkdir(mtklogpath, 0775) == -1) {
                 MNLD_STRNCPY(logpath, LOG_FILE_PATH, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);  // if mkdir fail, set default path
                 LOGD("mkdir %s fail(%s), set default logpath(%s)", mtklogpath, strerror(errno), logpath);
             }
        }
    }
    */
    return 1;
}
static int mode = 0;

bool Mnld2Mtklogger_WriteDate(mtk_socket_fd* client_fd, char* msg, int len) {
    if (msg == NULL || len > MNLD2MTKLOGGER_ANS_BUFF_SIZE) {
        SOCK_LOGE("Mnld2Mtklogger_WriteDate() input parameter error");
        return false;
    }
    pthread_mutex_lock(&client_fd->mutex);
    if(!mtk_socket_client_connect(client_fd)) {
        SOCK_LOGE("Mnld2Mtklogger_WriteDate() mtk_socket_client_connect() failed");
        pthread_mutex_unlock(&client_fd->mutex);
        return false;
    }
    int _ret;
    _ret = mtk_socket_write(client_fd->fd, msg, len);
    if(_ret == -1) {
        SOCK_LOGE("Mnld2Mtklogger_WriteDate() mtk_socket_write() failed, fd=%d err=[%s]%d",
            client_fd->fd, strerror(errno), errno);
        mtk_socket_client_close(client_fd);
        pthread_mutex_unlock(&client_fd->mutex);
        return false;
    }
    mtk_socket_client_close(client_fd);
    pthread_mutex_unlock(&client_fd->mutex);
    return true;
}

int mtklogger2mnl_hdlr(int server_fd, mtk_socket_fd* client_fd) {
    int ret = 0;
    char ans[MNLD2MTKLOGGER_ANS_BUFF_SIZE] = {0};
    char buff_msg[MTKLOGGER2MNLD_READ_BUFF_SIZE] = {0};

    ret = mtk_socket_read(server_fd, buff_msg, sizeof(buff_msg) - 1);
    if(ret == -1) {
        LOGE("mtk_socket_read() failed, server_fd=%d err=[%s]%d",
            server_fd, strerror(errno), errno);
        return false;
    }

    // response "msg,1" to mtklogger
    snprintf(ans, sizeof(ans), "%s,1", buff_msg);
    LOGD("notify client, recv %s from mtklogger, ans: %s\n", buff_msg, ans);
    if (strstr(buff_msg, "set_storage_path")) {  // buff is "set_storage_path,storagePath"
        MNLD_STRNCPY(storagePath_mtklogger_set, buff_msg, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        get_mtklog_path(storagePath_mtklogger_set);
        Mnld2Mtklogger_WriteDate(client_fd, ans, strlen(ans));
    } else if (!strncmp(buff_msg, "deep_start", 10)) {
        time_t tm;
        struct tm *p;
        static char GPSLog_timestamp[GPSLog_TIMESTRING_LEN];

        time(&tm);
        p = localtime(&tm);
        if(p == NULL) {
            LOGE("Get localtime fail:[%s]%d", strerror(errno), errno);
            return false;
        }

        /*/ GPS debug log dir is not exit, mkdir, before add time string, return value 0:success, -1 : fail
        if (0 != access(storagePath_mtklogger_set, F_OK)) {
            LOGE("Create dir %s\r\n", strerror(errno));
            if (mkdir(storagePath_mtklogger_set, 0775) == -1) {
               // mkdir before GPSLog_%timestamp%,if fail print the fail info to main log
                LOGE("mkdir %s fail(%s)", storagePath, strerror(errno));
             }
        }
        */
        memset(GPSLog_timestamp, 0x00, sizeof(GPSLog_timestamp));
        snprintf(GPSLog_timestamp, sizeof(GPSLog_timestamp), "CSLog_%04d_%02d%02d_%02d%02d%02d",
        1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec);

        snprintf(storagePath, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s%s/", storagePath_mtklogger_set, GPSLog_timestamp);

        // strncat(storagePath, GPSLog_timestamp, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);

        LOGD("storagePath:%s\n", storagePath);

        gps_dbg_log_state_set_output_enable();
        LOGD("gps_debuglog_state:%d", gps_debuglog_state);

        #if ANDROID_MNLD_PROP_SUPPORT
        property_set(GPS_LOG_PERSIST_STATE, GPS_LOG_PERSIST_VALUE_ENABLE);
        property_set(GPS_LOG_PERSIST_PATH, storagePath);
        #endif
        Mnld2Mtklogger_WriteDate(client_fd, ans, strlen(ans));

        mode = 0;
        MNLD_STRNCPY(gps_debuglog_file_name, storagePath, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        strncat(gps_debuglog_file_name, log_filename_suffix, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - strlen(gps_debuglog_file_name) - 1);

        LOGD("gps_debuglog_file_name:%s\n", gps_debuglog_file_name);

        if (mnld_is_gps_or_ofl_started_done()) {
            ret = mtk_gps_set_debug_type(gps_debuglog_state);
            if (MTK_GPS_ERROR == ret) {
                LOGE("deep_start,1,mtk_gps_set_debug_type fail");
            } else {
                LOGD("start gpsdbglog successfully\n");
            }
            fg_create_dir_done = 0;
            release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
        }
    } else if (!strncmp(buff_msg, "deep_start,2", 12)) {
        gps_dbg_log_state_set_output_enable();
        LOGD("gps_debuglog_state:%d", gps_debuglog_state);

        #if ANDROID_MNLD_PROP_SUPPORT
        property_set(GPS_LOG_PERSIST_STATE, GPS_LOG_PERSIST_VALUE_ENABLE);
        property_set(GPS_LOG_PERSIST_PATH, storagePath);
        #endif
        Mnld2Mtklogger_WriteDate(client_fd, ans, strlen(ans));

        MNLD_STRNCPY(gps_debuglog_file_name, LOG_FILE, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        MNLD_STRNCPY(storagePath, LOG_FILE_PATH, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
        if (mnld_is_gps_or_ofl_started_done()) {
            ret = mtk_gps_set_debug_type(gps_debuglog_state);
            if (MTK_GPS_ERROR == ret) {
                LOGE("deep_start,2,mtk_gps_set_debug_type fail");
            } else {
                LOGD("start gpsdbglog successfully\n");
            }
            fg_create_dir_done = 0;
            release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
        }

        mode = 1;
    } else if (!strncmp(buff_msg, "deep_stop", 9)) {
        gps_dbg_log_state_set_output_disable();
        LOGD("gps_debuglog_state:%d", gps_debuglog_state);

        #if ANDROID_MNLD_PROP_SUPPORT
        property_set(GPS_LOG_PERSIST_STATE, GPS_LOG_PERSIST_VALUE_DISABLE);
        property_set(GPS_LOG_PERSIST_PATH, GPS_LOG_PERSIST_VALUE_NONE);
        #endif
        Mnld2Mtklogger_WriteDate(client_fd, ans, strlen(ans));

        if (mnld_is_gps_or_ofl_started_done()) {
            ret = mtk_gps_set_debug_type(gps_debuglog_state);
            if (MTK_GPS_ERROR== ret) {
                LOGE("deep_stop, mtk_gps_set_debug_type fail");
            } else {
                LOGD("stop gpsdbglog successfully\n");
            }
            release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
        }
        mnl2mpe_set_log_path(storagePath, 0, 0);
        mode = 0;
    } else {
        Mnld2Mtklogger_WriteDate(client_fd, ans, strlen(ans));
        LOGE("unknown message: %s\n", buff_msg);
    }
    return 0;
}

void mtklogger_mped_reboot_message_update(void) {
    if (gps_dbg_log_state_is_output_disabled()) {
        mnl2mpe_set_log_path(storagePath, 0, mode);
    } else if (gps_dbg_log_state_is_output_enabled()) {
        mnl2mpe_set_log_path(storagePath, 1, mode);
    }
}
void* gps_dbg_log_thread(void* arg) {
    //INT32 count = 0;
    LOGD("create: %.8X, arg = %p\r\n", (unsigned int)pthread_self(), arg);
    pthread_detach(pthread_self());

    init_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
    init_condition(&lock_for_sync[PINGPANG_FLUSH_LOCK]);
    //GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);

    if (gps_dbg_log_state_is_output_enabled()) {
        create_debug_log_file();
        mnl2mpe_set_log_path(storagePath, 1, 0); //Notify MPE to enable log and pass the log path to MPE, the 3rd param is unused
    }
    /*
    if ((access(storagePath, F_OK|R_OK|W_OK)) == 0) {
        DirLogSize = gps_log_dir_check(storagePath);
    }
    */
    while (!g_gpsdbglogThreadExit) {
        get_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
        //LOGD("get_condition PINGPANG_WRITE_LOCK");
        if (gps_dbg_log_state_is_output_enabled()) {
            if ((fg_create_dir_done == 0) && (!g_gpsdbglogThreadExit)) {
                mtklog_gps_set_debug_file(gps_debuglog_file_name);
                mnl2mpe_set_log_path(storagePath, 1, 0);
            }
            /*
            if (0 != access(storagePath, F_OK|R_OK|W_OK)) {  // return value 0:success, -1 : fail
                LOGE("Access gps debug log dir fail(%s)!\r\n", strerror(errno));
                GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);  // close file before open,  if the file has been open.

                if (mnld_is_gps_or_ofl_started_done()) {
                    if (MTK_GPS_ERROR == mtk_gps_set_debug_type(gps_debuglog_state)) {
                        LOGE("GPS_DEBUGLOG_DISABLE, mtk_gps_set_debug_type fail");
                    } else {
                        LOGD("GPS_DEBUGLOG_DISABLE, stop gpsdbglog successfully\n");
                    }
                }
            }
            if (g_hLogFile != NULL) {
                if (Current_FileSize + PINGPANG_BUFFER_SIZE > MAX_DBG_LOG_FILE_SIZE) {
                    //char tmpfilename[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN]={0};
                    Filenum++;
                    GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);  // close file before open, if the file has been open.
                    DirLogSize = DirLogSize + Current_FileSize;
                    snprintf(gsGpsLogFileName_full, sizeof(gsGpsLogFileName_full), "%s-%d", gsGpsLogFileName, Filenum);
                    strncat(gsGpsLogFileName_full, LOG_FILE_WRITING_EXTEN_NAME, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - strlen(gsGpsLogFileName_full) -1);
#if ANDROID_MNLD_PROP_SUPPORT
                    property_set(GPS_LOG_PERSIST_FLNM, strstr(gsGpsLogFileName_full, log_filename_suffix));
#endif
                    g_hLogFile = fopen(gsGpsLogFileName_full, "w");
                    if (NULL == g_hLogFile) {
                        LOGE("open file fail, NULL == g_hLogFile\r\n");
                        break;
                    }
                    if (DirLogSize  > (int)(MAX_DBG_LOG_DIR_SIZE - MAX_DBG_LOG_FILE_SIZE)
                        || (total_file_count + Filenum) > GPS_DBG_LOG_FILE_NUM_LIMIT) {
                        DirLogSize = gps_log_dir_check(storagePath);
                    }

                    Current_FileSize = 0;
                }
            }
            if (g_hLogFile != NULL) {
                //count = gps_dbg_log_pingpang_write(&ping_pang_buffer_body, g_hLogFile);
                gps_dbg_log_pingpang_write(&ping_pang_buffer_body);
                Current_FileSize = count + Current_FileSize;
            }
            */
            if (!gps_dbg_log_pingpang_write(&ping_pang_buffer_body)) {
                LOGE("send gpslog to gps logd fail");
            }
        } else {
            if ((ping_pang_buffer_body.start_address_buffer1 != NULL)
                && (ping_pang_buffer_body.start_address_buffer2 != NULL)) {
                //LOGD("debuglog switch closed, flush gpsdbglog to flash from pingpang\r\n");
                /*
                if (g_hLogFile != NULL) {
                    gps_dbg_log_pingpang_flush(&ping_pang_buffer_body, g_hLogFile);
                }
                */
                gps_dbg_log_pingpang_flush(&ping_pang_buffer_body);
                gps_dbg_log_pingpang_free();
                //GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);
                if (!mnld2logd_close_gpslog())
                    LOGE("send close gpslog cmd to gps logd fail");
            }
        }
    }

    //if (g_hLogFile != NULL) {
        if ((ping_pang_buffer_body.start_address_buffer1 != NULL)
            && (ping_pang_buffer_body.start_address_buffer2 != NULL)) {
            // flush
            LOGD("thread will exit,flush gpsdbglog to flash from pingpang\r\n");
            //gps_dbg_log_pingpang_flush(&ping_pang_buffer_body, g_hLogFile);
            gps_dbg_log_pingpang_flush(&ping_pang_buffer_body);
            // free pingpang
            //LOGD("free pingpang buffer now\r\n");
            gps_dbg_log_pingpang_free();
        }

        //GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);
        g_hLogFile = NULL;
        LOGD("send close gpslog cmd to gps logd\r\n");
        if (!mnld2logd_close_gpslog())
            LOGE("send close gpslog cmd to gps logd fail");
    //}

    mnld_destroy_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    pthread_dbg_log = C_INVALID_TID;
    return NULL;
}

int gps_dbg_log_thread_init() {
    mnld_create_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    g_gpsdbglogThreadExit = false;
    if (!g_pingpang_init && gps_dbg_log_state_is_output_enabled()) {
        if (MTK_GPS_ERROR == gps_dbg_log_pingpang_init()) {
            g_gpsdbglogThreadExit = true;
            LOGE("gps dbg log pingpang init fail, thread exit");
        }
    }
    pthread_create(&pthread_dbg_log, NULL, gps_dbg_log_thread, NULL);
    return 0;
}

int create_mtklogger2mnl_fd() {
    static int socket_fd = 0;

    socket_fd = socket_local_server("gpslogd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (socket_fd < 0) {
        LOGE("create server fail(%s)", strerror(errno));
        return -1;
    }
    LOGD("socket_fd = %d", socket_fd);

    if (listen(socket_fd, 5) < 0) {
        LOGE("listen socket fail(%s)", strerror(errno));
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

static int FindGPSlogFile(char Filename[]) {
    int i;
    int GPSlogNamelen = 0;

    GPSlogNamelen = strlen(log_filename_suffix);

    for (i = 0; i < GPSlogNamelen; i++) {
        if (Filename[i]!= log_filename_suffix[i]) {
            return MTK_GPS_ERROR;
        }
    }
    return MTK_GPS_SUCCESS;
}

static int GetFileSize(char *filename) {
    char dir_filename[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN];
    struct stat statbuff;

    memset(dir_filename, 0x00, sizeof(dir_filename));

    if (NULL == filename) {
        LOGE("[GetFileSize][error]: File name is NULL!!\r\n");
        return 0;
    }

    // LOGD("[GetFileSize]File name:%s!\r\n", filename);
    snprintf(dir_filename, sizeof(dir_filename), "%s%s", storagePath,
            filename);

    if (stat(dir_filename, &statbuff) < 0) {
        LOGE("[GetFileSize][error]: open file(%s) state fail(%s)!\r\n", dir_filename, strerror(errno));
        return 0;
    } else {
        return statbuff.st_size;  // return the file size
    }
}

#if 0
static int CmpStrFile(char a[], char b[]) {  // compare two log files
    char *pa = a, *pb = b;
    while (*pa == *pb) {
        pa++;
        pb++;
    }
    return (*pa - *pb);
}
#endif
static int CmpFileTime(char *filename1, char *filename2) {
    char dir_filename1[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN];
    char dir_filename2[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN];
    struct stat statbuff1;
    struct stat statbuff2;

    memset(dir_filename1, 0x00, sizeof(dir_filename1));
    memset(dir_filename2, 0x00, sizeof(dir_filename2));

    if (NULL == filename1 || NULL == filename2) {
        LOGE("[CmpFileTime][error]: File name is NULL!!\r\n");
        return 0;
    }
    // LOGD("[CmpFileTime]File name1:%s, filename2:%s!!\r\n", filename1, filename2);

    snprintf(dir_filename1, sizeof(dir_filename1), "%s%s", storagePath, filename1);
    snprintf(dir_filename2, sizeof(dir_filename2), "%s%s", storagePath, filename2);

    if (stat(dir_filename1, &statbuff1) < 0) {
        LOGD("[CmpFileTime][error]: open file(%s) state  fail(%s)!!\r\n", dir_filename1, strerror(errno));
        return 0;
    }

    if (stat(dir_filename2, &statbuff2) < 0) {
        LOGD("[CmpFileTime][error]: open file(%s) state  fail(%s)!!\r\n", dir_filename2, strerror(errno));
        return 0;
    }
    return (statbuff1.st_mtime-statbuff2.st_mtime);
}

static INT32 create_debug_log_file() {
    time_t tm;
    struct tm *p;
    //char file_tree[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    //FILE* fd = NULL;
    //INT32 res;
    char *gps_debuglog_file_name_format_str;

    time(&tm);
    p = localtime(&tm);
    if (strlen((const char*)gps_debuglog_file_name) && (p != NULL)) {  // if filename length > 0
        // initialize debug log (use OPEN_ALWAYS to append debug log)
        // GPS debug log dir is not exit, mkdir, return value 0:success, -1 : fail
        /*
        if (0 != access(storagePath, F_OK)) {
            LOGD("Try to create dir(%s)\r\n", storagePath);
            if (mkdir(storagePath, 0775) == -1) {  // mkdir ,if fail print the fail info to main log
                LOGE("mkdir %s fail(%s)", storagePath, strerror(errno));
                return MTK_GPS_ERROR;
             } else {  // Create dir successfully
                snprintf(file_tree, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%sfile_tree.txt", storagePath_mtklogger_set);
                fd = fopen(file_tree, "at");  // Write the latest storage path to the end of file_tree.txt
                if (NULL != fd) {
                    fwrite(storagePath, 1, strlen(storagePath), fd);
                    fwrite("\n", 1, 1, fd);
                    fclose(fd);
                } else {
                    LOGE("write file %s fail(%s)", file_tree, strerror(errno));
                }
             }
        }
        */

        if (gps_dbg_log_state_is_encrypt_enabled()) {
            gps_debuglog_file_name_format_str = "%s_%04d_%02d%02d_%02d%02d%02d.enc";
        } else {
            gps_debuglog_file_name_format_str = "%s_%04d_%02d%02d_%02d%02d%02d";
        }

        memset(gsGpsLogFileName, 0x00, sizeof(gsGpsLogFileName));
        snprintf(gsGpsLogFileName, sizeof(gsGpsLogFileName), gps_debuglog_file_name_format_str, gps_debuglog_file_name,
        1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec);
        Filenum = 0;
        //GPSLOG_FCLOSE(g_hLogFile, gsGpsLogFileName_full);  // close file before open, if the file has been open.

        //snprintf(gsGpsLogFileName_full, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s"LOG_FILE_WRITING_EXTEN_NAME, gsGpsLogFileName);
        snprintf(gsGpsLogFileName_full, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s", gsGpsLogFileName);
#if ANDROID_MNLD_PROP_SUPPORT
        property_set(GPS_LOG_PERSIST_FLNM, strstr(gsGpsLogFileName_full, log_filename_suffix));
#endif

        if (!mnld2logd_open_gpslog(gsGpsLogFileName_full)) {
            LOGE("file(%s) created fail(0x%x)\r\n", gsGpsLogFileName_full, (unsigned int)g_hLogFile);
        }
        LOGD("cmd create file(%s) send to gps debug daemon successfully", gsGpsLogFileName_full);
        /*
        int LogFile_fd = open(gsGpsLogFileName_full, O_RDWR|O_NONBLOCK|O_CREAT, 0660);
        if (LogFile_fd < 0) {
            LOGE("open file fail(%s)", strerror(errno));
            return MTK_GPS_ERROR;
        } else {
            int flags = fcntl(LogFile_fd, F_GETFL, 0);
            if (fcntl(LogFile_fd, F_SETFL, flags|O_NONBLOCK) < 0) {
                LOGD("fcntl logFile_fd fail");
            }
            g_hLogFile = fdopen(LogFile_fd, "w");
        }
        LOGD("file(%s) created successfully(0x%x)\r\n", gsGpsLogFileName, (unsigned int)g_hLogFile);
        */
        Current_FileSize = 0;
        fg_create_dir_done = 1;
        return MTK_GPS_SUCCESS;
    }
    LOGD("create_debug_log_file fail");
    return MTK_GPS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  mtklog_gps_set_debug_file
 * DESCRIPTION
 *  Set the GPS debug file name(include the path name) in running time
 * PARAMETERS
 *  file_name         [IN]   the debug file name needs to be changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
static INT32 mtklog_gps_set_debug_file(char* file_name) {
    if (NULL == file_name) {  // Null pointer, return error
        LOGE("file_name is NULL pointer! \r\n");
        return MTK_GPS_ERROR;
    }
// The length of file_name is too long, return error
    if (GPS_DEBUG_LOG_FILE_NAME_MAX_LEN <= (strlen(file_name) + 1)) {
        LOGE("file_name is too long! \r\n");
        return MTK_GPS_ERROR;
    }

    if (!g_pingpang_init) {
        if (MTK_GPS_ERROR == gps_dbg_log_pingpang_init()) {
            g_gpsdbglogThreadExit = true;
            LOGE("gps dbg log pingpang init fail, thread exit");
        }
    }

    if (MTK_GPS_ERROR == create_debug_log_file()) {
        LOGD("create debug file(%s) error\r\n", file_name);
    } else {  // Create file success, check dir size
        DirLogSize = gps_log_dir_check(storagePath);
    }

    return MTK_GPS_SUCCESS;
}

INT32 gps_log_dir_check(char * dirname) {   // file size check
    char temp_filename[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN];
    char OldGpsFile[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};
    unsigned int DirLogSize_temp;
    DIR *p_dir;
    char OldFile[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN];

    struct dirent *p_dirent;
    do {
        if (0 != access(dirname, F_OK|R_OK)) {  // Check if the dir exist, can read,return value 0:success, -1 : fail
            LOGE("Access gps debug log dir fail(%s)!\r\n", strerror(errno));
            return MTK_GPS_ERROR;
        }

        if ((p_dir = opendir(dirname)) == NULL) {
            LOGE("open dir error(%s)\r\n", strerror(errno));
            return MTK_GPS_ERROR;
        } else {
            LOGD("open dir sucess\r\n");
        }

        total_file_count = 0;
        memset(OldGpsFile, 0x00, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);  // For compare file name,set a max char value
        DirLogSize_temp = 0;

        while ((p_dirent = readdir(p_dir)) && !g_gpsdbglogThreadExit) {
            if (NULL == p_dirent || (0 != access(dirname, F_OK))) {  // return value 0:success, -1 : fail
                LOGE("Access gps debug log dir fail(%s)!\r\n", dirname);
                break;
            }
            if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {  // Ignore the "." & ".."
                continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
            }

            if (GPS_DEBUG_LOG_FILE_NAME_MAX_LEN > strlen(p_dirent->d_name)) {
                memset(temp_filename, 0 , GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
                MNLD_STRNCPY(temp_filename, (void *)&p_dirent->d_name, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - 1);
            } else {  // The length of d_name is too long, ignore this file
                LOGD("d_name is too long!\r\n");
                continue;
            }

            if (FindGPSlogFile(temp_filename) == MTK_GPS_SUCCESS) {
                // LOGD("%s is a GPS debug log file!\r\n", temp_filename);

                DirLogSize_temp = GetFileSize(temp_filename) + DirLogSize_temp;
                total_file_count++;
                if (strncmp(OldGpsFile, temp_filename, strlen(log_filename_suffix)) != 0) {
                    MNLD_STRNCPY(OldGpsFile, temp_filename, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - 1);
                    LOGD("copy file name to OldGpsFile: %s, and continue\r\n", OldGpsFile);
                    continue;  // while((p_dirent=readdir(p_dir)) && !g_gpsdbglogThreadExit)
                }

                if (CmpFileTime(temp_filename, OldGpsFile) < 0) {  // Find the latest old file
                    memset(OldGpsFile, '\0', GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);
                    MNLD_STRNCPY(OldGpsFile, temp_filename, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN - 1);
                }
            }
        }
        LOGD("DirLogSize_temp:%d, the latest OldGpsFile:%s!\r\n", DirLogSize_temp, OldGpsFile);
        closedir(p_dir);
        if (DirLogSize_temp >= (MAX_DBG_LOG_DIR_SIZE - MAX_DBG_LOG_FILE_SIZE)
            || (total_file_count > GPS_DBG_LOG_FILE_NUM_LIMIT)) {
            // Over size or the number of GPS debug log file over the limitation
            // when OldGpsFile is small, it will cause many re-calculation in the second loop.  need to avoid it.
            INT32 ret = 0;

            ret = GetFileSize(OldGpsFile);
            memset(OldFile, 0x00, sizeof(OldFile));
            snprintf(OldFile, sizeof(OldFile), "%s%s", storagePath, OldGpsFile);

            LOGD("need delete OldFile:%s\r\n", OldFile);

            if (remove(OldFile) != 0) {  // Error handle
                LOGW("Remove file %s error(%s)!\r\n", OldFile, strerror(errno));
            }
            DirLogSize_temp = DirLogSize_temp - ret;
        }

        LOGD("After remove file gpsdebug log dir size:%d!\r\n", DirLogSize_temp);
    }while (((DirLogSize_temp > (MAX_DBG_LOG_DIR_SIZE - MAX_DBG_LOG_FILE_SIZE))
    || (total_file_count > GPS_DBG_LOG_FILE_NUM_LIMIT)) && !g_gpsdbglogThreadExit);

    if ((DirLogSize_temp <= (MAX_DBG_LOG_DIR_SIZE - MAX_DBG_LOG_FILE_SIZE)) && g_gpsdbglogThreadExit) {
        LOGD("gps_log_dir_check interrupted size=%d!!\r\n", DirLogSize_temp);
    }
    LOGD("dir size:%d\r\n", DirLogSize_temp);
    return DirLogSize_temp;
}

INT32 mnl_sys_alps_gps_dbg2file_mnld(const char* buffer, UINT32 length) {
    //INT32 ret = MTK_GPS_SUCCESS;
    //UINT32 count = 0;

    if (mtk_gps_log_is_hide()) { //Forbit to print gps debug log
        return MTK_GPS_SUCCESS;
    }

    if (gps_dbg_log_state_is_output_enabled() \
                 && (ping_pang_buffer_body.next_write != NULL)) {
        gps_dbg_log_pingpang_copy(&ping_pang_buffer_body, buffer, length);
    } else {
        LOGD("will not copy to pingpang, DebugType:%d, buffer1:%p, buffer2:%p, g_hLogFile:%p\r\n",
            gps_debuglog_state, ping_pang_buffer_body.start_address_buffer1,
            ping_pang_buffer_body.start_address_buffer2, g_hLogFile);
    }

    return MTK_GPS_SUCCESS;
}

// if return error , gpsdbglog will not be writen
static INT32 gps_dbg_log_pingpang_init() {
    LOGD("gps_dbg_log_pingpang_init");
    mnld_take_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    memset(&ping_pang_buffer_body, 0x00, sizeof(ping_pang_buffer_body));
    if (((ping_pang_buffer_body.start_address_buffer1 = calloc(1, PINGPANG_BUFFER_SIZE)) == NULL) || \
        ((ping_pang_buffer_body.start_address_buffer2 = calloc(1, PINGPANG_BUFFER_SIZE)) == NULL)) {
        if (ping_pang_buffer_body.start_address_buffer1 != NULL) {
            free(ping_pang_buffer_body.start_address_buffer1);
            ping_pang_buffer_body.start_address_buffer1 = NULL;
        }
        if (ping_pang_buffer_body.start_address_buffer2 != NULL) {
            free(ping_pang_buffer_body.start_address_buffer2);
           ping_pang_buffer_body.start_address_buffer2 = NULL;
        }
        mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
        return MTK_GPS_ERROR;
    }
    // lenth_to_write_buffer1 = 0;
    // lenth_to_write_buffer2 = 0;
    ping_pang_buffer_body.buffer1_state = WRITING;
    ping_pang_buffer_body.buffer2_state = WRITABLE;
    // buffer1_state = WRITING;
    // buffer2_state = WRITABLE;
    // ping_pang_buffer_body.p_buffer1_lenth_to_write = &lenth_to_write_buffer1;
    // ping_pang_buffer_body.p_buffer2_lenth_to_write = &lenth_to_write_buffer2;
    ping_pang_buffer_body.end_address_buffer1 = ping_pang_buffer_body.start_address_buffer1 + PINGPANG_BUFFER_SIZE-2;
    ping_pang_buffer_body.end_address_buffer2 = ping_pang_buffer_body.start_address_buffer2 + PINGPANG_BUFFER_SIZE-2;
    ping_pang_buffer_body.next_write = ping_pang_buffer_body.start_address_buffer1;

    g_pingpang_init = true;
    mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    return MTK_GPS_SUCCESS;
}

static INT32 gps_dbg_log_pingpang_copy(ping_pang_buffer* pingpang, const char* buffer, INT32 len) {
    if ((len >= PINGPANG_BUFFER_SIZE-1) || (len < 0)) {
        LOGW("len = %d out of range\n", len);
        return MTK_GPS_ERROR;
    }
    mnld_take_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    if (pingpang->next_write == NULL) {
        LOGW("next_write is NULL\n");
        return MTK_GPS_ERROR;
    }
    if ((pingpang->buffer1_state == WRITING) && (pingpang->buffer2_state != WRITING)) {
        if (pingpang->next_write+len > pingpang->end_address_buffer1) {
            if (pingpang->buffer2_state == WRITABLE) {
                pingpang->buffer1_lenth_to_write = pingpang->next_write - pingpang->start_address_buffer1;
                pingpang->next_write = pingpang->start_address_buffer2;
                pingpang->buffer2_state = WRITING;
                pingpang->buffer1_state = READABLE;
                release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
            } else {
                memset(pingpang->start_address_buffer1, 0x0, PINGPANG_BUFFER_SIZE);
                pingpang->next_write = pingpang->start_address_buffer1;
                pingpang->num_loose++;
                LOGD("loose log ,num is %d \r\n", pingpang->num_loose);
            }
        }
    } else if ((pingpang->buffer2_state == WRITING) && (pingpang->buffer1_state != WRITING)) {
        if (pingpang->next_write+len > pingpang->end_address_buffer2) {
            if (pingpang->buffer1_state == WRITABLE) {
                pingpang->buffer2_lenth_to_write = pingpang->next_write - pingpang->start_address_buffer2;
                pingpang->next_write = pingpang->start_address_buffer1;
                pingpang->buffer1_state = WRITING;
                pingpang->buffer2_state = READABLE;
                release_condition(&lock_for_sync[PINGPANG_WRITE_LOCK]);
            } else {
                memset(pingpang->start_address_buffer2, 0x0, PINGPANG_BUFFER_SIZE);
                pingpang->next_write = pingpang->start_address_buffer2;
                pingpang->num_loose++;
                LOGD("loose log ,num is %d \r\n", pingpang->num_loose);
            }
        }
    } else {
        LOGE("abnormal happens, buffer1_state=%d, buffer2_state=%d\r\n", pingpang->buffer1_state, pingpang->buffer2_state);
        mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
        return MTK_GPS_ERROR;
    }

    memcpy(pingpang->next_write, buffer, len);
    pingpang->next_write += len;
    mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    return MTK_GPS_SUCCESS;
}

bool gps_dbg_log_print_status(ping_pang_buffer* pingpang) {
    LOGD("=====================================================\n");
    LOGD("start_address_buffer1:%p\n", pingpang->start_address_buffer1);
    LOGD("start_address_buffer2:%p\n", pingpang->start_address_buffer2);
    LOGD("end_address_buffer1:%p\n", pingpang->end_address_buffer1);
    LOGD("end_address_buffer2:%p\n", pingpang->end_address_buffer2);
    LOGD("buffer1_lenth_to_write:%d\n", pingpang->buffer1_lenth_to_write);
    LOGD("buffer2_lenth_to_write:%d\n", pingpang->buffer2_lenth_to_write);
    LOGD("buffer1_state:%d\n", pingpang->buffer1_state);
    LOGD("buffer2_state:%d\n", pingpang->buffer2_state);
    LOGD("num_loose:%d\n", pingpang->num_loose);
    return true;
}
// the real write to flash
//static INT32 gps_dbg_log_pingpang_write(ping_pang_buffer* pingpang, FILE* filp) {
static bool gps_dbg_log_pingpang_write(ping_pang_buffer* pingpang) {
    //LOGD("gps_dbg_log_pingpang_write\n");
    bool ret;
    if ((pingpang->buffer1_state == READABLE)\
        && (pingpang->buffer2_state != READABLE)\
        && ((pingpang->buffer1_lenth_to_write) != 0)) {
        pingpang->buffer1_state = READING;
        //count = fwrite(pingpang->start_address_buffer1, 1, pingpang->buffer1_lenth_to_write, filp);
        ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer1, pingpang->buffer1_lenth_to_write);
        memset(pingpang->start_address_buffer1, 0x0, PINGPANG_BUFFER_SIZE);
        pingpang->buffer1_lenth_to_write = 0;
        pingpang->buffer1_state = WRITABLE;
    } else if ((pingpang->buffer2_state == READABLE)\
        && (pingpang->buffer1_state != READABLE)\
        && ((pingpang->buffer2_lenth_to_write) != 0)) {
        pingpang->buffer2_state = READING;
        //count = fwrite(pingpang->start_address_buffer2, 1, pingpang->buffer2_lenth_to_write, filp);
        ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer2, pingpang->buffer2_lenth_to_write);
        memset(pingpang->start_address_buffer2, 0x0, PINGPANG_BUFFER_SIZE);
        pingpang->buffer2_lenth_to_write = 0;
        pingpang->buffer2_state = WRITABLE;
    } else {
        ret = true;
    }
    return ret;
}

// when mnl exit or mtklogger set 1 to 0, there is a need free pingpang buffer
static INT32 gps_dbg_log_pingpang_free() {
    mnld_take_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    if (ping_pang_buffer_body.start_address_buffer1 != NULL) {
        free(ping_pang_buffer_body.start_address_buffer1);
        ping_pang_buffer_body.start_address_buffer1 = NULL;
    }
    if (ping_pang_buffer_body.start_address_buffer2 != NULL) {
        free(ping_pang_buffer_body.start_address_buffer2);
        ping_pang_buffer_body.start_address_buffer2 = NULL;
    }
    memset(&ping_pang_buffer_body, 0x00, sizeof(ping_pang_buffer_body));
    // lenth_to_write_buffer1 = 0;
    // lenth_to_write_buffer2 = 0;
    // buffer1_state = NOTINIT;
    // buffer2_state = NOTINIT;

    g_pingpang_init = false;
    LOGD("free pingpang buffer\r\n");
    mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    return MTK_GPS_SUCCESS;
}

// when mnl exit or mtklogger set 1 to 0, there is a need to flush the data to flash from buffer
static bool gps_dbg_log_pingpang_flush(ping_pang_buffer * pingpang) {
    char* tmp_next_write = NULL;
    bool ret;
    mnld_take_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    tmp_next_write = pingpang->next_write;
    if ((pingpang->buffer1_state == WRITING) && (pingpang->buffer2_state != WRITING)) {
        if (pingpang->buffer2_state == READABLE) {
            //fwrite(pingpang->start_address_buffer2, 1, pingpang->buffer2_lenth_to_write, filp);
            ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer2, pingpang->buffer2_lenth_to_write);
        }
        //fwrite(pingpang->start_address_buffer1, 1, tmp_next_write - pingpang->start_address_buffer1, filp);
        ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer1, tmp_next_write - pingpang->start_address_buffer1);
    } else if ((pingpang->buffer2_state == WRITING) && (pingpang->buffer1_state != WRITING)) {
        if (pingpang->buffer1_state == READABLE) {
            //fwrite(pingpang->start_address_buffer1, 1, pingpang->buffer1_lenth_to_write, filp);
            ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer1, pingpang->buffer1_lenth_to_write);
        }
        //fwrite(pingpang->start_address_buffer2, 1, tmp_next_write - pingpang->start_address_buffer2, filp);
        ret = mnld2logd_write_gpslog_subpackage(pingpang->start_address_buffer2, tmp_next_write - pingpang->start_address_buffer2);
    } else {
        LOGE("abnormal happens\r\n");
    }
    LOGD("flush gpsdbg to flash done!\r\n");
    mnld_give_mutex(MNLD_MUTEX_PINGPANG_WRITE);
    return MTK_GPS_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  gps_dbg_log_property_load
 * DESCRIPTION
 *  Load properties to set gps_debuglog_state, storagePath, gps_debuglog_file_name and
 *  to rename the legacy gps debug log .nmac to .nma.
 *  The legacy gps debug log file name has been stored in GPS_LOG_PERSIST_PATH & GPS_LOG_PERSIST_FLNM
 * PARAMETERS
 *  None
 * RETURNS
 *  None
 *****************************************************************************/
void gps_dbg_log_property_load(void) {
#if ANDROID_MNLD_PROP_SUPPORT
        char path_result[PROPERTY_VALUE_MAX] = {0};
        char flnm_result[PROPERTY_VALUE_MAX] = {0};
        char state_result[PROPERTY_VALUE_MAX] = {0};
        char filename_full[GPS_DEBUG_LOG_FILE_NAME_MAX_LEN] = {0};

        if(property_get(GPS_LOG_PERSIST_STATE, state_result, NULL) != 0) {
            if(state_result[0] == '1') {
                gps_dbg_log_state_set_output_enable();
            } else {
                gps_dbg_log_state_set_output_disable();
            }
        }

        if ((property_get(GPS_LOG_PERSIST_PATH, path_result, NULL) != 0)
            && (strcmp(path_result, GPS_LOG_PERSIST_VALUE_NONE) != 0)) {
            MNLD_STRNCPY(storagePath, path_result, PROPERTY_VALUE_MAX);
            snprintf(gps_debuglog_file_name, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s%s", storagePath, log_filename_suffix);

            if((property_get(GPS_LOG_PERSIST_FLNM, flnm_result, NULL) != 0)
            && (strcmp(flnm_result, GPS_LOG_PERSIST_VALUE_NONE) != 0)
            && ((strlen(path_result) + strlen(flnm_result)) < GPS_DEBUG_LOG_FILE_NAME_MAX_LEN)) {
                snprintf(filename_full, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN, "%s%s", path_result, flnm_result);
                LOGD("Rename legacy gpsdbgfile:%s", filename_full);
                gps_log_file_rename(filename_full);
            } else {
                LOGE("length fail: %s(len:%d), %s(len:%d)", path_result, strlen(path_result), flnm_result, strlen(flnm_result));
            }
        }
#endif
}

#define HAS_BITS(x, bits)   (!!((x) & (bits)))
#define SET_BITS(x, bits)   ((x) |= (bits))
#define CLR_BITS(x, bits)   ((x) &= (~(bits)))

void gps_dbg_log_state_init() {
    //default value is: MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD
    //gps_debuglog_state = MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD;

    //create mutex here
    mnld_create_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

void gps_dbg_log_state_set_bitmask(unsigned int bitmask) {
    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    SET_BITS(gps_debuglog_state, bitmask);
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

void gps_dbg_log_state_set_output_enable() {
    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    SET_BITS(gps_debuglog_state, MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD); //set 0x11
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

void gps_dbg_log_state_set_output_disable() {
    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    CLR_BITS(gps_debuglog_state, MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNL); //clear 0x01, keep 0x10
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

bool gps_dbg_log_state_is_output_enabled() {
    bool is_enabled;

    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    // Note: currently 0x10 always be true, it equals "just check 0x01 is set"
    is_enabled = (MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD ==
        (gps_debuglog_state & MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD));
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);

    return is_enabled;
}

bool gps_dbg_log_state_is_output_disabled() {
    bool is_disabled;

    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    // Note: currently 0x10 always be true, it equals "just check 0x01 is clear"
    is_disabled = (MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD ==
        (gps_debuglog_state & MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD));
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);

    return is_disabled;
}

void gps_dbg_log_state_set_encrypt_enable() {
    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    SET_BITS(gps_debuglog_state, MTK_GPS_ENCRYPT_DEBUG_MSG_BY_MNL);
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

void gps_dbg_log_state_set_encrypt_disable() {
    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    CLR_BITS(gps_debuglog_state, MTK_GPS_ENCRYPT_DEBUG_MSG_BY_MNL);
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
}

bool gps_dbg_log_state_is_encrypt_enabled() {
    bool is_enabled;

    mnld_take_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);
    is_enabled = HAS_BITS(gps_debuglog_state, MTK_GPS_ENCRYPT_DEBUG_MSG_BY_MNL);
    mnld_give_mutex(MNLD_MUTEX_GPS_DBG_LOG_STATE);

    return is_enabled;
}

#define MNL_LOG_PRINTX_TAG "MNLD"
void mnld_log_printx(mnld_log_level_t log_lv, int skip_chars, char *fmt, ...) {
    char out_buf[1024];
    va_list ap;
    int log_len;
    unsigned int enc_len = 0;

    if (mtk_gps_log_is_hide()) {
        return;
    }

    va_start(ap, fmt);
    log_len = vsnprintf(out_buf, sizeof(out_buf)-1, fmt, ap);
    va_end(ap);

    if (mtk_gps_log_get_hide_opt() == 2) {

        if (log_len > skip_chars) {
            // Not encrypt the lead "skip_chars" of line, typically the 1st "X"
            enc_len = mtk_gps_log_line_enc_inplace(&out_buf[1], (UINT32)(log_len - skip_chars));
        }

        if (enc_len == 0) {
            LOGD("XXLOGX dropped: log_len=%d, skip_len=%d, enc_len=%d", log_len, skip_chars, enc_len);
            return; //not ouptut due to nothing is encrypt
        }

        //LOGD("XXLOGX encrypt: log_len=%d, skip_len=%d, enc_len=%d", log_len, skip_chars, enc_len);
    }

    switch(log_lv) {
    case LV_DEBUG:
        __android_log_write(ANDROID_LOG_DEBUG, MNL_LOG_PRINTX_TAG, out_buf);
        break;
    case LV_VERBOSE:
        __android_log_write(ANDROID_LOG_VERBOSE, MNL_LOG_PRINTX_TAG, out_buf);
        break;
    case LV_INFO:
        __android_log_write(ANDROID_LOG_INFO, MNL_LOG_PRINTX_TAG, out_buf);
        break;
    case LV_WARN:
        __android_log_write(ANDROID_LOG_WARN, MNL_LOG_PRINTX_TAG, out_buf);
        break;
    case LV_ERROR:
        __android_log_write(ANDROID_LOG_ERROR, MNL_LOG_PRINTX_TAG, out_buf);
        break;
    default:
        __android_log_write(ANDROID_LOG_DEBUG, MNL_LOG_PRINTX_TAG, out_buf);
    }
}

