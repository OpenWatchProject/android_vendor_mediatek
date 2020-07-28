#include <fcntl.h>   /* File control definitions */
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "mtk_gps_type.h"
#include "gps_dbg_log.h"
#include "mtk_lbs_utility.h"
#include "libladder.h"
#include "LbsLogInterface.h"
#if ANDROID_MNLD_PROP_SUPPORT
#include <cutils/properties.h>
#endif

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#if 0
#define LOGD(...) tag_log(1, "[gps_dbg_log]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gps_dbg_log] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gps_dbg_log] ERR: ", __VA_ARGS__);
#else
#define LOG_TAG "MNLD_DUMP"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

#define MNLD_DUMP_FILE_LEN 128
#ifdef  __cplusplus
extern "C" {
#endif
extern bool mnld2logd_open_dumplog(char* file);
extern bool mnld2logd_write_dumplog(char * data, int len);
extern bool mnld2logd_close_dumplog(void);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

/**Disable mnld process dump, due to have never used mnld dump infor, and often blocked in unwind backtrace**/
//#define MNLD_PROCESS_DUMP

#ifdef MNLD_PROCESS_DUMP
char mnld_dump_file[] = "mnld";

/*****************************************************************************
 * FUNCTION
 *  mnld_dump_get_filename
 * DESCRIPTION
 *  To generate the full name of dump file
 * PARAMETERS
 *  dump_filename         [OUT]   the string to store the full name of dump file
 *  length                      [IN]    the max length of dump_filename
 * RETURNS
 *  None
 *****************************************************************************/
void mnld_dump_get_filename(char *dump_filename, size_t length)
{
    char path[MNLD_DUMP_FILE_LEN] = {0};

    do {
#if ANDROID_MNLD_PROP_SUPPORT
        char path_result[PROPERTY_VALUE_MAX] = {0};

        if (property_get(GPS_LOG_PERSIST_PATH, path_result, NULL)  //Store the dump file under GPS debug log folder
            && (strcmp(path_result, GPS_LOG_PERSIST_VALUE_NONE) != 0)) {
            MNLD_STRNCPY(path, path_result, length);
        } else {
            LOGW("log path not set!");
            break;
        }
#endif
        if(dump_filename == NULL)
        {
            LOGW("input NULL pointer!!!");
            break;
        }

        memset(dump_filename, 0x00, length);

        snprintf(dump_filename, length, "%s%s_%d.dumpc",
            path, mnld_dump_file, getpid());
        LOGD("dump file:%s", dump_filename);
    } while(0);
}

/*****************************************************************************
 * FUNCTION
 *  mnld_dump_process
 * DESCRIPTION
 *  Dump all threads of current process.
 * PARAMETERS
 *  dump_fd         [IN]   the file descripter to dump in to, -1: dump to mainlog
 *
 * RETURNS
 *  None
 *****************************************************************************/
void mnld_dump_process() {
    std::string dump;
    char data[LBS_LOG_INTERFACE_BUFF_SIZE] = {0};
    unsigned int len = 0;
    unsigned int i = 0;
    unsigned int n = 0;
    unsigned int data_max = 0;
    if (LBS_LOG_INTERFACE_BUFF_SIZE > 4*sizeof(int)) {
        data_max = LBS_LOG_INTERFACE_BUFF_SIZE - 4*sizeof(int);
    } else {
        LOGE("LBS_LOG_INTERFACE_BUFF_SIZE too log (%u)", LBS_LOG_INTERFACE_BUFF_SIZE);
        return;
    }
    UnwindCurProcessBT(&dump);
    len = dump.length();
    n = (len + (data_max-1))/data_max;
    for (i=0; i<n; i++) {
        memset(data, 0x0, LBS_LOG_INTERFACE_BUFF_SIZE);
        strncpy(data, dump.c_str()+i*data_max, data_max);
        data[data_max] = '\0';
        if (!mnld2logd_write_dumplog(data, strlen(data))) {
            LOGE("send write dump file cmd faile(%s)", strerror(errno));
        }
    }
    //write(dump_fd, dump.c_str(), dump.length());  //Dump to file
}
#if 0
/*****************************************************************************
 * FUNCTION
 *  mnld_dump_thread
 * DESCRIPTION
 *  To dump specified thread's backtrace.
 *  [Not used until now, keep here for demo code]
 * PARAMETERS
 *  dump_fd         [IN]   the file descripter to dump in to, -1: dump to mainlog
 *  tid                 [IN]   the thread id(tid) need to be dumped
 *
 * RETURNS
 *  None
 *****************************************************************************/
void mnld_dump_thread(int dump_fd, pid_t tid) {  //Dump specific thread backtrace
    std::string dump;
    UnwindCurThreadBT(&dump);
    if(dump_fd != -1) {
        write(dump_fd, dump.c_str(), dump.length());  //Dump to file
        LOGD("mnld_dump_thread tid:(%d)", tid);
    }
}
#endif
#endif
/*****************************************************************************
 * FUNCTION
 *  mnld_dump_exit
 * DESCRIPTION
 *  To dump specified thread's backtrace and exit current process.
 *  If the tid equal to current process id(pid), will dump all threads of current process.
 * PARAMETERS
 *  tid         [IN]   the thread id(tid) need to be dumped
 *
 * RETURNS
 *  None
 *****************************************************************************/
extern "C" void mnld_dump_exit(void)
{
#ifdef MNLD_PROCESS_DUMP
    char dump_file_full[MNLD_DUMP_FILE_LEN] = {0};

    mnld_dump_get_filename(dump_file_full, MNLD_DUMP_FILE_LEN);
    //dump_fd = open(dump_file_full, O_RDWR|O_NONBLOCK|O_CREAT, 0660);

    if(!mnld2logd_open_dumplog(dump_file_full)) {
        LOGE("send create dump file cmd faile(%s)", strerror(errno));
    } else {
        mnld_dump_process();
        mnld2logd_close_dumplog();
    }
#endif
    _exit(0);
}
