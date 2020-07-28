#include <errno.h>   /* Error number definitions */
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "mtk_gps_agps.h"
#include "agps_agent.h"
#include "data_coder.h"
#include "mnl_common.h"
#include "mnld.h"
#include "mtk_lbs_utility.h"
#include "epo.h"
#include "mtknav.h"
#include "curl.h"
#include "easy.h"
#include "mtk_gps.h"

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
#define LOGD(...) tag_log(1, "[epo]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[epo] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[epo] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mtknav"

#include <cutils/sockets.h>
#include <cutils/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#ifdef CONFIG_GPS_ENG_LOAD
#define LOGD_ENG(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#else
#define LOGD_ENG(fmt, arg ...) NULL
#endif
#endif

static char mtknav_file_name[GPS_EPO_FILE_LEN] = {0};
static char mtknav_md5_file_name[GPS_EPO_FILE_LEN] = {0};

bool mtknav_update_flag = false;
int mtknav_res = EPO_DOWNLOAD_RESULT_FAIL;

static int mtknav_file_update_impl();
typedef enum {
    MAIN2MTKNAV_EVENT_START            = 0,
} main2mtknav_event;

typedef struct mtknav_dl_state {
    bool MD5_DL_Today;
    bool DAT_DL_Today;
    int  last_DL_Date;
    unsigned int retry_time;
}MTKNAV_STATE_T;

static int g_fd_mtknav;
static int mtknav_downloading = 0;
static MTKNAV_STATE_T dl_state = {
    .MD5_DL_Today = false,
    .DAT_DL_Today = false,
    .last_DL_Date = -1,
    .retry_time = 0,
};

/////////////////////////////////////////////////////////////////////////////////
// static functions

static int counter = 1;
static void getMtkNavUrl(char* filename, char* url) {
    char count_str[15] = {0};

    if (counter <= 1) {
        strncat(url, EPO_URL_HOME_G, GPS_EPO_URL_LEN - strlen(url) - 1);
    } else {
        strncat(url, EPO_URL_HOME_C, GPS_EPO_URL_LEN -strlen(url) - 1);
    }

    strncat(url, filename, GPS_EPO_URL_LEN  - strlen(url) - 1);
    strncat(url, "?retryCount=", GPS_EPO_URL_LEN  - strlen(url) - 1);
    sprintf(count_str, "%d", counter-1);
    strncat(url, count_str, GPS_EPO_URL_LEN  - strlen(url) - 1);
    LOGD("url = %s\n", url);
}

static void check_mtknav_file_exist(void) {
    if ( (access(MTKNAV_MD5_FILE, F_OK) == -1) ||
         (access(MTKNAV_DAT_FILE, F_OK) == -1) ) {
        LOGD_ENG("mtknav file does not exist, need download\n");
        dl_state.MD5_DL_Today = false;
        dl_state.DAT_DL_Today = false;
        dl_state.retry_time = 0;
    }
}

static CURLcode curl_easy_download_mtknav_DAT(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};

    LOGD("curl_easy_download_mtknav_DAT");
    getMtkNavUrl(mtknav_file_name, url);
    res = curl_easy_download(url, MTKNAV_DAT_FILE_HAL);

    LOGD("mtknav DAT file curl_easy_download res = %d\n", res);
    if (res == 0) {
        counter = 1;
        res_val = chmod(MTKNAV_DAT_FILE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        counter++;
    }
    return res;
}

static CURLcode curl_easy_download_mtknav_MD5(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};

    LOGD_ENG("curl_easy_download_mtknav_MD5");
    getMtkNavUrl(mtknav_md5_file_name, url);
    res = curl_easy_download(url, MTKNAV_MD5_FILE_HAL);

    LOGD("mtknav MD5 file curl_easy_download res = %d\n", res);
    if (res == 0) {
        counter = 1;
        res_val = chmod(MTKNAV_MD5_FILE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod MD5 res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        counter++;
    }
    return res;
}

static int mtknav_MD5_file_check(void) {
    int ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    FILE *fp_old = NULL;
    FILE *fp_new = NULL;
    int new_file_size = 0;
    char old_md5[40];  // length = 40 because current our md5 file size is 37bytes, but only first 32 bytes are useful.
    char new_md5[40];

    memset(old_md5, 0x00, sizeof(old_md5));
    memset(new_md5, 0x00, sizeof(new_md5));

    if (is_file_exist(MTKNAV_MD5_FILE_HAL) == 0) {
        LOGE("new mtknav MD5 file doesn't exist, download failed!");
        return EPO_MD5_DOWNLOAD_RESULT_FAIL;
    }

    if (is_file_exist(MTKNAV_MD5_FILE) == 0) {
        LOGW("old mtknav MD5 file doesn't exist");
        new_file_size = get_file_size(MTKNAV_MD5_FILE_HAL);
        if (new_file_size < 32) {
            ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
            if (delete_file(MTKNAV_MD5_FILE_HAL) == -1) {
                LOGW("delete %s file error",MTKNAV_MD5_FILE_HAL);
            }
        } else {
            ret = EPO_MD5_FILE_UPDATED;
            if (rename(MTKNAV_MD5_FILE_HAL, MTKNAV_MD5_FILE) == -1) {  //update MTKNAV.MD5 on local
                LOGW("MD5 rename error");
            }
        }
    } else {
        new_file_size = get_file_size(MTKNAV_MD5_FILE_HAL);
        if (new_file_size < 32) {
            ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
            if (delete_file(MTKNAV_MD5_FILE_HAL) == -1) {
                LOGW("delete %s file error",MTKNAV_MD5_FILE_HAL);
            }
        } else {
            fp_old = fopen(MTKNAV_MD5_FILE, "r");
            if (fp_old != NULL) {
                if (fread(old_md5, 32, 1, fp_old) != 32) {  //only first 32 bytes are useful.
                    LOGW("read old MD5 file error");
                }
                fclose(fp_old);
            }
            fp_new = fopen(MTKNAV_MD5_FILE_HAL, "r");
            if (fp_new != NULL) {
                if (fread(new_md5, 32, 1, fp_new) != 32) {
                    LOGW("read new MD5 file error");
                }
                fclose(fp_new);
            }
            if (strncmp(old_md5,new_md5,32) != 0) {
                if (rename(MTKNAV_MD5_FILE_HAL, MTKNAV_MD5_FILE) == -1) { //update MTKNAV.MD5 on local only when file content update
                    LOGW("MD5 rename error");
                }
                ret = EPO_MD5_FILE_UPDATED;
            } else {
                ret = EPO_MD5_FILE_NO_UPDATE;
                if (delete_file(MTKNAV_MD5_FILE_HAL) == -1) {
                    LOGW("delete %s file error",MTKNAV_MD5_FILE_HAL);
                }
            }
        }
    }
    return ret;
}

static int is_mtknav_file_valid(void) {
    int fd = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    fd = open(MTKNAV_DAT_FILE_HAL, O_RDONLY);
    if (fd < 0) {
        LOGE("Open mtknav fail(%s), return\n",strerror(errno));
        ret = EPO_DOWNLOAD_RESULT_FAIL;
    } else {
        ret = EPO_DOWNLOAD_RESULT_SUCCESS;
        close(fd);
    }
    return ret;
}

static int mtknav_DAT_download_process(void) {
    int ret = EPO_DOWNLOAD_RESULT_FAIL;
    LOGD("mtknav_DAT_download_process begin");

    MNLD_STRNCPY(mtknav_file_name, "MTKNAV.DAT", sizeof(mtknav_file_name));
    if (curl_easy_download_mtknav_DAT() == CURLE_OK) {
        LOGD("download mtknav DAT file success,CURLE_OK");
        if (is_mtknav_file_valid() == EPO_DOWNLOAD_RESULT_SUCCESS) {
            LOGD("Check mtknav DAT file ok");
            ret = EPO_DOWNLOAD_RESULT_SUCCESS;
        } else {
            dl_state.retry_time++;
            ret = EPO_DOWNLOAD_RESULT_FAIL;
            LOGW("check mtknav DAT file error, retry:%d",dl_state.retry_time);
        }
    } else {
        LOGW("download mtknav MD5 file failed");
        ret = EPO_DOWNLOAD_RESULT_FAIL;
    }
    return ret;
}

static int mtknav_MD5_download_process(void) {
    LOGD_ENG("mtknav_MD5_download_process begin");
    int ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    MNLD_STRNCPY(mtknav_md5_file_name, "MTKNAV.MD5", sizeof(mtknav_md5_file_name));
    if (curl_easy_download_mtknav_MD5() == CURLE_OK) {
        LOGD("mtknav MD5 download success, start to check");
        ret = mtknav_MD5_file_check();
    } else {
        LOGD_ENG("download mtknav MD5 file failed");
        ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    }
    return ret;
}

static int mtknav_file_update_impl() {
    int ret_MD5 = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    int ret_DAT = EPO_DOWNLOAD_RESULT_FAIL;

    ret_MD5 = mtknav_MD5_download_process();
    if (ret_MD5 != EPO_MD5_DOWNLOAD_RESULT_FAIL) {
        LOGD("MD5 download success, no need download any more today");
        dl_state.MD5_DL_Today = true;
    }

    if (ret_MD5 == EPO_MD5_FILE_NO_UPDATE) {
        dl_state.DAT_DL_Today = true;  // new MD5 match old MD5, no need download DAT any more
        return EPO_DOWNLOAD_RESULT_NO_UPDATE;
    }

    if (ret_MD5 == EPO_MD5_FILE_UPDATED) {
        ret_DAT = mtknav_DAT_download_process();
    }

    if (ret_DAT == EPO_DOWNLOAD_RESULT_SUCCESS) {
        LOGD("DAT download success, no need download any more today");
        dl_state.DAT_DL_Today = true;
    }
    return ret_DAT;
}

void mtknav_update_mtknav_file(int mtknav_valid) {
    int xdownload_status = MTK_MTKNAV_RSP_DOWNLOAD_FAIL;

    if (mtknav_valid == EPO_DOWNLOAD_RESULT_SUCCESS) {
        if (mtk_agps_agent_mtknav_file_update() == MTK_GPS_ERROR) {
            xdownload_status = MTK_MTKNAV_RSP_UPDATE_FAIL;
        } else {
            LOGD("Update mtknav file successfully");
            xdownload_status = MTK_MTKNAV_RSP_UPDATE_SUCCESS;
            unlink(MTKNAV_DAT_FILE_HAL);
        }
    } else if (mtknav_valid == EPO_DOWNLOAD_RESULT_NO_UPDATE) {
        xdownload_status = MTK_MTKNAV_RSP_NO_UPDATE;
    } else if (mtknav_valid == EPO_DOWNLOAD_RESULT_RETRY_TOO_MUCH) {
        xdownload_status = MTK_MTKNAV_RSP_DOWNLOAD_FAIL;
    } else {
        xdownload_status = MTK_MTKNAV_RSP_DOWNLOAD_FAIL;
    }

    LOGD("xdownload_status = 0x%x\n", xdownload_status);
    if (MTK_GPS_ERROR ==  (mtk_agps_set_param (MTK_PARAM_MTKNAV_DOWNLOAD_RESPONSE,
        &xdownload_status, MTK_MOD_DISPATCHER, MTK_MOD_AGENT))) {
        LOGE("GPS MTKNAV update fail\n");
    }
}


//////////////////////////////////////////////////////////////////////////////////
// MAIN -> MTKNAV Download (handlers)
static int mnld_mtknav_download() {
    int ret;
    struct tm  tm;
    time_t now = time(NULL);
    gmtime_r(&now, &tm);

    check_mtknav_file_exist();

    LOGD_ENG("dl_state_date:%d, system_date:%d, MD5_flag:%d, DAT_flag:%d, retry:%d",
        dl_state.last_DL_Date,tm.tm_mday, dl_state.MD5_DL_Today, dl_state.DAT_DL_Today,dl_state.retry_time);

    if (dl_state.last_DL_Date != tm.tm_mday) {
        dl_state.MD5_DL_Today = false;
        dl_state.DAT_DL_Today = false;
        dl_state.retry_time = 0;
        dl_state.last_DL_Date = tm.tm_mday;
        LOGD_ENG("First mtknav request today, day is %d", dl_state.last_DL_Date);
    }

    if ((dl_state.DAT_DL_Today == false) && (dl_state.retry_time <= MTKNAV_DL_RETRY_TIME)) {
        mtknav_downloading = 1;
        ret = mtknav_file_update_impl();
        mtknav_downloading = 0;
    } else if (dl_state.DAT_DL_Today == true) {
        LOGW("mtknav has been downloaded today");
        ret = EPO_DOWNLOAD_RESULT_NO_UPDATE;
    } else {
        LOGW("mtknav has been retry too much, retry time=%d",dl_state.retry_time);
        ret = EPO_DOWNLOAD_RESULT_RETRY_TOO_MUCH;
    }
    mtknav_res = ret;
    if(mnld_mtknav_download_done(ret) == -1){
        LOGD_ENG("mtknav has not download done");
    }
    return ret;
}

static int mtknav_event_hdlr(int fd) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    main2mtknav_event cmd;
    int read_len;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("mtknav_event_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    cmd = get_int(buff, &offset, sizeof(buff));
    switch (cmd) {
    case MAIN2MTKNAV_EVENT_START: {
        LOGD_ENG("mnld_mtknav_download() before");
        mnld_mtknav_download();
        LOGD_ENG("mnld_mtknav_download() after");
        break;
    }
    default: {
        LOGE("mtknav_event_hdlr() unknown cmd=%d", cmd);
        return -1;
    }
    }
    return 0;
}

static void* mtknav_downloader_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    //timer_t hdlr_timer = init_timer(qepo_downloader_thread_timeout);
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("mtknav_downloader_thread() epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return 0;
    }

    if (epoll_add_fd(epfd, g_fd_mtknav) == -1) {
        LOGE("mtknav_downloader_thread() epoll_add_fd() failed for g_fd_mtknav failed");
        return 0;
    }

    while (1) {
        int i;
        int n;
        LOGD_ENG("mtknav_downloader_thread wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("mtknav_downloader_thread() epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        //start_timer(hdlr_timer, MNLD_QEPO_HANDLER_TIMEOUT);
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_mtknav) {
                if (events[i].events & EPOLLIN) {
                    mtknav_event_hdlr(g_fd_mtknav);
                }
            } else {
                LOGE("mtknav_downloader_thread() unknown fd=%d",
                    events[i].data.fd);
            }
        }
        //stop_timer(hdlr_timer);
    }

    LOGE("mtknav_downloader_thread() exit");
    return 0;
}

int mtknav_downloader_start() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    if (mtknav_downloading == 1) {
        LOGW("mtknav downloading... abort requst msg!");
        return 0;
    }
    put_int(buff, &offset, MAIN2MTKNAV_EVENT_START);
    return safe_sendto(MNLD_MTKNAV_DOWNLOAD_SOCKET, buff, offset);
}

int mtknav_downloader_init() {
    pthread_t pthread_mtknav;

    g_fd_mtknav = socket_bind_udp(MNLD_MTKNAV_DOWNLOAD_SOCKET);
    if (g_fd_mtknav < 0) {
        LOGE("socket_bind_udp(%s) failed",MNLD_MTKNAV_DOWNLOAD_SOCKET);
        return -1;
    }

    pthread_create(&pthread_mtknav, NULL, mtknav_downloader_thread, NULL);
    return 0;
}

