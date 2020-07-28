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
#include "qepo.h"
#include "curl.h"
#include "easy.h"
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
#define LOGD(...) tag_log(1, "[epo]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[epo] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[epo] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "qepo"

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

#define QUARTER_FILE_HAL "/data/vendor/gps/QEPOHAL.DAT"
#define QEPO_BD_INVALID_SLEEP (100*1000)  //Delay 100ms before fail response, when the BD QEPO data on server is invalid
#define QEPO_GA_INVALID_SLEEP (100*1000)  //Delay 100ms before fail response, when the GA QEPO data on server is invalid

static char quarter_epo_file_name[GPS_EPO_FILE_LEN] = {0};
static char quarter_epo_bd_file_name[GPS_EPO_FILE_LEN] = {0};
static char quarter_epo_bd_md_file_name[GPS_EPO_FILE_LEN] = {0};
static char quarter_epo_ga_file_name[GPS_EPO_FILE_LEN] = {0};

static int qepo_file_update_impl();
typedef enum {
    MAIN2QEPO_EVENT_START            = 0,
    MAIN2QEPO_BD_EVENT_START         = 1,
    MAIN2QEPO_GA_EVENT_START         = 2,
} main2qepo_event;

typedef struct qepo_gps_time {
    int wn;
    int tow;
    int sys_time;
}QEPO_GPS_TIME_T;

static QEPO_GPS_TIME_T gps_time;
static int g_fd_qepo;
static int qepo_download_finished = 1;
static int qepo_bd_download_finished = 1;
static int qepo_ga_download_finished = 1;
extern int gps_epo_type;
static time_t g_qbd_latest_dl_time_utc_s = 0;  //Record the latest time of downloading BD QEPO file from server
static time_t g_qbd_first_invalid_dl_time_utc_s = 0;  //Record the latest time of downloading BD QEPO file from server, and the file is valid
static int g_qepo_bd_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QBD.DAT file on server is valid or not, valid when init
static int g_qepo_bd_file_size_correct = MTK_GPS_TRUE;  //The file size is correct or not, correct in init
static int g_qepo_bd_file_time_correct = MTK_GPS_TRUE;  //The BD QEPO file time is correct or not, correct in init
int g_qepo_bd_invalid_dl_cnt = 0;
static time_t g_qga_latest_dl_time_utc_s = 0;  //Record the latest time of downloading BD QEPO file from server
static time_t g_qga_first_invalid_dl_time_utc_s = 0;  //Record the latest time of downloading BD QEPO file from server, and the file is valid
static int g_qepo_ga_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QBD.DAT file on server is valid or not, valid when init
static int g_qepo_ga_file_size_correct = MTK_GPS_TRUE;  //The file size is correct or not, correct in init
static int g_qepo_ga_file_time_correct = MTK_GPS_TRUE;  //The BD QEPO file time is correct or not, correct in init
int g_qepo_ga_invalid_dl_cnt = 0;
bool qepo_update_flag = false;
int qepo_dl_res = EPO_DOWNLOAD_RESULT_FAIL;
bool qepo_BD_update_flag = false;
int qepo_bd_dl_res = EPO_DOWNLOAD_RESULT_FAIL;
bool qepo_GA_update_flag = false;
int qepo_ga_dl_res = EPO_DOWNLOAD_RESULT_FAIL;

/////////////////////////////////////////////////////////////////////////////////
// static functions

static int pre_day = 0;
static int server_not_updated = 0;
static int Qepo_res = 0;
static int Qepo_bd_res = 0;
static int Qepo_ga_res = 0;
static int counter_GR = 1;
static int counter_BD = 1;
static int counter_GA = 1;

static void getQepoUrl(char* filename, char* url, int count) {
    char count_str[15] = {0};

    if (count > 1 || (access(EPO_FILE, F_OK) < 0) || epo_downloader_is_file_invalid()) {
        strncat(url, EPO_URL_HOME_C, GPS_EPO_URL_LEN - strlen(url) - 1);
    } else {
        strncat(url, EPO_URL_HOME_G, GPS_EPO_URL_LEN -strlen(url) - 1);
    }

    strncat(url, filename, GPS_EPO_URL_LEN  - strlen(url) - 1);
    strncat(url, "?retryCount=", GPS_EPO_URL_LEN  - strlen(url) - 1);
    sprintf(count_str, "%d", count-1);
    strncat(url, count_str, GPS_EPO_URL_LEN  - strlen(url) - 1);
    LOGD_ENG("url = %s\n", url);
}

static CURLcode curl_easy_download_quarter_epo(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};

    LOGD_ENG("curl_easy_download_quarter_epo");
    getQepoUrl(quarter_epo_file_name, url, counter_GR);

    res = curl_easy_download(url, QUARTER_FILE_HAL);
    LOGD("qepo file curl_easy_download res = %d\n", res);
    Qepo_res = res;
    if (res == 0) {
        counter_GR = 1;
        res_val = chmod(QUARTER_FILE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (res_val < 0) {
            LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
        }
    } else {
        counter_GR++;
    }
    return res;
}

static CURLcode curl_easy_download_quarter_epo_bd(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};

    LOGD_ENG("curl_easy_download_quarter_bdepo");
    getQepoUrl(quarter_epo_bd_file_name, url, counter_BD);

    res = curl_easy_download(url, QEPO_BD_UPDATE_FILE);

    LOGD("qbdepo file curl_easy_download res = %d\n", res);
    Qepo_bd_res = res;
    if (res == 0) {
        counter_BD = 1;
        res_val = chmod(QEPO_BD_UPDATE_FILE, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        counter_BD++;
    }
    return res;
}

static CURLcode curl_easy_download_quarter_epo_ga(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};

    LOGD_ENG("curl_easy_download_quarter_gaepo");
    getQepoUrl(quarter_epo_ga_file_name, url, counter_GA);

    res = curl_easy_download(url, QEPO_GA_UPDATE_FILE);

    LOGD("qgaepo file curl_easy_download res = %d\n", res);
    Qepo_ga_res = res;
    if (res == 0) {
        counter_GA = 1;
        res_val = chmod(QEPO_GA_UPDATE_FILE, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        counter_GA++;
    }
    return res;
}


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_qepo_bd_period_start
 * DESCRIPTION
 *  Get the start time of Beidou QEPO file
 * PARAMETERS
 *  [IN] fd: The file description of Beidou QEPO file
 *  [OUT] u4GpsSecs: The start time of Beidou QEPO file in second of GSP time
 *  [OUT] uSecond: The start time of Beidou QEPO file in second of UTC time
 * RETURNS
 *  success(0); failure(-1)
 *****************************************************************************/
int mtk_gps_sys_qepo_bd_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 == lseek(fd, MTK_QEPO_BD_HEADER_SIZE, SEEK_SET)) { //Skip the header of BD QEPO file
        LOGE("lseek error\n");
        return -1;
    }

    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        LOGE("read error\n");
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

int mtk_gps_sys_qepo_ga_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 == lseek(fd, MTK_QEPO_GA_HEADER_SIZE, SEEK_SET)) { //Skip the header of GA QEPO file
        LOGE("lseek error\n");
        return -1;
    }

    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        LOGE("read error\n");
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_qepo_bd_has_epo
 * DESCRIPTION
 *  Get the HAS_EPO flag of Beidou QEPO file
 * PARAMETERS
 *  [IN] header: The first word value of header in Beidou QEPO file
 *  [OUT] has_epo: The value of HAS_EPO flag
 * RETURNS
 *  success(0); failure(-1)
 *****************************************************************************/
int mtk_gps_sys_qepo_bd_has_epo(UINT32 header, int* has_epo) {
    if (NULL == has_epo) { //Read from the beginning of BD QEPO file
        LOGE("has_epo pointer check error\n");
        return -1;
    }

    if((header&QEPO_BD_HAS_EPO_BIT_MASK) == QEPO_BD_HAS_EPO_BIT_MASK)
    {
        *has_epo = MTK_GPS_TRUE;
        LOGD("BD QEPO has epo(0x%x)!!!\n", header);
    }else{
        *has_epo = MTK_GPS_FALSE;
        LOGE("BD QEPO file error(0x%x), doesn't have epo!!!\n",header);
    }

    return 0;
}

int mtk_gps_sys_qepo_ga_has_epo(UINT32 header, int* has_epo) {
    if (NULL == has_epo) { //Read from the beginning of GA QEPO file
        LOGE("has_epo pointer check error\n");
        return -1;
    }

    if((header&QEPO_GA_HAS_EPO_BIT_MASK) == QEPO_GA_HAS_EPO_BIT_MASK)
    {
        *has_epo = MTK_GPS_TRUE;
        LOGD("GA QEPO has epo(0x%x)!!!\n", header);
    }else{
        *has_epo = MTK_GPS_FALSE;
        LOGE("GA QEPO file error(0x%x), doesn't have epo!!!\n",header);
    }

    return 0;
}

/* count the number of 1 in a 32-bit word */
static UINT32
gps_mnl_epo_bit_cnt (UINT32 u4BitMask)
{
    UINT32 i, u4Cnt = 0;

    for (i = 0; i < 32; i++)
    {
        if (1 == ((u4BitMask >> i) & 1))
        {
            u4Cnt++;
        }
    }

    return  u4Cnt;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_qepo_bd_get_sv_cnt
 * DESCRIPTION
 *  Get the SV cnt of Beidou QEPO file
 * PARAMETERS
 *  [IN] header: The second word value of header in Beidou QEPO file
 *  [OUT] sv_cnt: The value of SV count
 * RETURNS
 *  success(0); failure(-1)
 *****************************************************************************/
int mtk_gps_sys_qepo_bd_get_sv_cnt(UINT32 header, int* sv_cnt) {
    if (NULL == sv_cnt) { //Read from the beginning of BD QEPO file
        LOGE("sv_cnt pointer check error\n");
        return -1;
    }

    *sv_cnt = gps_mnl_epo_bit_cnt(header);
    LOGD("sv cnt(0x%x):%d",header,*sv_cnt);

    return 0;
}

int mtk_gps_sys_qepo_ga_get_sv_cnt(UINT32 header, int* sv_cnt) {
    if (NULL == sv_cnt) { //Read from the beginning of GA QEPO file
        LOGE("sv_cnt pointer check error\n");
        return -1;
    }

    *sv_cnt = gps_mnl_epo_bit_cnt(header);
    LOGD("sv cnt(0x%x):%d",header,*sv_cnt);

    return 0;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_qepo_bd_get_header
 * DESCRIPTION
 *  Get the header of Beidou QEPO file(72byte)
 * PARAMETERS
 *  [IN] fd: The file description of Beidou QEPO file
 *  [OUT] bd_header: The pointer of HAS_EPO flag
 * RETURNS
 *  success(0); failure(-1)
 *****************************************************************************/
int mtk_gps_sys_qepo_bd_get_header(int fd, UINT32 bd_header[MTK_EPO_ONE_SV_SIZE/4]) {         // no file lock
    if(NULL == bd_header )
    {
        LOGE("bd_header pointer check error\n");
        return -1;
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) { //Read from the beginning of BD QEPO file
        LOGE("lseek error(%s)\n",strerror(errno));
        return -1;
    }

    memset(bd_header,0,MTK_EPO_ONE_SV_SIZE);
    if (read(fd, bd_header, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        LOGD_ENG("read error(%s)\n",strerror(errno));
        return -1;
    }

    return 0;
}

int mtk_gps_sys_qepo_ga_get_header(int fd, UINT32 ga_header[MTK_EPO_ONE_SV_SIZE/4]) {         // no file lock
    if(NULL == ga_header )
    {
        LOGE("ga_header pointer check error\n");
        return -1;
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) { //Read from the beginning of BD QEPO file
        LOGE("lseek error(%s)\n",strerror(errno));
        return -1;
    }

    memset(ga_header,0,MTK_EPO_ONE_SV_SIZE);
    if (read(fd, ga_header, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        LOGE("read error(%s)\n",strerror(errno));
        return -1;
    }

    return 0;
}

/*****************************************************************************/
static int is_quarter_epo_valid(void) {
    unsigned int u4GpsSecs_start;  // GPS seconds
    time_t uSecond_start;   // UTC seconds
    time_t mnl_time;
    time_t *mnl_gps_time = NULL;
    int fd = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    fd = open(QUARTER_FILE_HAL, O_RDONLY);
    if (fd < 0) {
        LOGE("Open QEPO fail, return\n");
        return ret;
    } else {
        if (mtk_gps_sys_epo_period_start(fd, &u4GpsSecs_start, &uSecond_start)) {
            LOGE("Read QEPO file failed\n");
            close(fd);
            return ret;
        } else {
            mnl_gps_time = &mnl_time;
            //LOGD("gps_time.wn, tow %d, %d\n", gps_time.wn, gps_time.tow);
            GpsToUtcTime(gps_time.wn, gps_time.tow, mnl_gps_time);
            //LOGD("The Start time of QEPO file is %lld\n", (long long)uSecond_start);
            LOGD("The start time of QEPO file is %s", ctime(&uSecond_start));
            LOGD("GPS time: %s", ctime(mnl_gps_time));

            if ((mnl_time >= uSecond_start) && (mnl_time < ((6*60*60) + uSecond_start))) {
                ret = EPO_DOWNLOAD_RESULT_SUCCESS;
            } else {
                ret = EPO_DOWNLOAD_RESULT_FAIL;
                if (uSecond_start >= ((18*60*60) + mnl_time)) {
                    // download time 23:55, server has updated
                    pre_day = 1;
                } else {
                    pre_day = 0;
                }
                if (mnl_time >= ((24*60*60) + uSecond_start)) {
                   // download time 00:04,server has not updated
                   server_not_updated = 1;
                } else {
                    server_not_updated = 0;
                }
            }
        }
        close(fd);
    }
    return ret;
}

/*****************************************************************************
 * FUNCTION
 *  gps_mnl_set_qbd_latest_dl_time
 * DESCRIPTION
 *  Set the latest Beidou QEPO download time in second of UTC time
 * PARAMETERS
 *  [IN] dl_time: The UTC time to be set
 * RETURNS
 *  All ways 0
 *****************************************************************************/
static int gps_mnl_set_qbd_latest_dl_time(time_t dl_time)
{
    g_qbd_latest_dl_time_utc_s = dl_time;

    return 0;
}

static int gps_mnl_set_qga_latest_dl_time(time_t dl_time)
{
    g_qga_latest_dl_time_utc_s = dl_time;

    return 0;
}

/*****************************************************************************
 * FUNCTION
 *  gps_mnl_get_qbd_latest_dl_time
 * DESCRIPTION
 *  Get the latest Beidou QEPO download time in second of UTC time
 * PARAMETERS
 *  None
 * RETURNS
 *  The value of g_qbd_latest_dl_time_utc_s
 *****************************************************************************/
time_t gps_mnl_get_qbd_latest_dl_time(void)
{
    return g_qbd_latest_dl_time_utc_s;
}

time_t gps_mnl_get_qga_latest_dl_time(void)
{
    return g_qga_latest_dl_time_utc_s;
}

 /*****************************************************************************
  * FUNCTION
  *  gps_mnl_set_qbd_first_invalid_dl_time
  * DESCRIPTION
  *  Set the latest Beidou QEPO invalid download time in second of UTC time
  *  invalid download: The file has been downloaded to local, but is invalid(size or start time is incorrect)
  * PARAMETERS
  *  [IN] dl_time: The UTC time to be set
  * RETURNS
  *  All ways 0
  *****************************************************************************/
 int gps_mnl_set_qbd_first_invalid_dl_time(time_t dl_time)
 {
    g_qbd_first_invalid_dl_time_utc_s = dl_time;
    return 0;
 }

 int gps_mnl_set_qga_first_invalid_dl_time(time_t dl_time)
 {
    g_qga_first_invalid_dl_time_utc_s = dl_time;
    return 0;
 }

 /*****************************************************************************
  * FUNCTION
  *  gps_mnl_get_qbd_first_invalid_dl_time
  * DESCRIPTION
  *  Get the latest Beidou QEPO invalid download time in second of UTC time
  *  invalid download: The file has been downloaded to local, but is invalid(size or start time is incorrect)
  * PARAMETERS
  *  None
  * RETURNS
  *  The value of g_qbd_first_invalid_dl_time_utc_s
  *****************************************************************************/
 time_t gps_mnl_get_qbd_first_invalid_dl_time(void)
 {
     return g_qbd_first_invalid_dl_time_utc_s;
 }

 time_t gps_mnl_get_qga_first_invalid_dl_time(void)
 {
     return g_qga_first_invalid_dl_time_utc_s;
 }

 /*****************************************************************************
  * FUNCTION
  *  gps_mnl_qbd_has_epo
  * DESCRIPTION
  *  Get the value of g_qepo_bd_has_epo
  * PARAMETERS
  *  None
  * RETURNS
  *  The value of g_qepo_bd_has_epo
  *****************************************************************************/
int gps_mnl_qbd_has_epo(void)
{
    return g_qepo_bd_has_epo;
}

int gps_mnl_qga_has_epo(void)
{
    return g_qepo_ga_has_epo;
}

 /*****************************************************************************
  * FUNCTION
  *  gps_mnl_epo_get_file_size
  * DESCRIPTION
  *  Get the file size
  * PARAMETERS
  *  [IN] fd: The file description
  * RETURNS
  *  File size
  *****************************************************************************/
int gps_mnl_epo_get_file_size(int fd)
{
    int u4FileSize;

    if (fd == 0)
    {
        return -1;
    }

    u4FileSize = lseek(fd, 0, SEEK_END);

    if (-1 == u4FileSize) { //lseek fail
        LOGE("lseek end error(%s)\n",strerror(errno));
        return -1;
    }

    if (-1 == lseek(fd, 0, SEEK_SET)) { //Back to the start of file
        LOGE("lseek set error(%s)\n",strerror(errno));
        return -1;
    }

    return u4FileSize;
}

/*****************************************************************************
 * FUNCTION
 *  gps_mnl_bd_qepo_init
 * DESCRIPTION
 *  Reset the init condition of BD QEPO
 * PARAMETERS
 *  None
 * RETURNS
 *  None
 *****************************************************************************/
void gps_mnl_bd_qepo_init(void)
{
        g_qepo_bd_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QBD.DAT file on server is valid or not, valid when init
        g_qepo_bd_file_size_correct = MTK_GPS_TRUE;  //The file size is correct or not, correct in init
        g_qepo_bd_file_time_correct = MTK_GPS_TRUE;  //The BD QEPO file time is correct or not, correct in init
}

void gps_mnl_ga_qepo_init(void)
{
        g_qepo_ga_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QGA.DAT file on server is valid or not, valid when init
        g_qepo_ga_file_size_correct = MTK_GPS_TRUE;  //The file size is correct or not, correct in init
        g_qepo_ga_file_time_correct = MTK_GPS_TRUE;  //The GA QEPO file time is correct or not, correct in init
}

/*****************************************************************************
 * FUNCTION
 *  is_quarter_epo_bd_on_server_valid
 * DESCRIPTION
 *  Judge the Beidou QEPO file on server is valid or not
 *  if invalid, will retry downloading.
 *  Valid: HAS_EPO flag is true && the time is correct
 * PARAMETERS
 *  None
 * RETURNS
 *  MTK_GPS_TRUE; MTK_GPS_FALSE
 *****************************************************************************/
static int is_quarter_epo_bd_on_server_valid(void) {
    if(g_qepo_bd_file_time_correct == MTK_GPS_TRUE &&
      g_qepo_bd_has_epo == MTK_GPS_TRUE){
      return MTK_GPS_TRUE;
    }else{
      return MTK_GPS_FALSE;
    }
}

static int is_quarter_epo_ga_on_server_valid(void) {
    if(g_qepo_ga_file_time_correct == MTK_GPS_TRUE &&
      g_qepo_ga_has_epo == MTK_GPS_TRUE){
      return MTK_GPS_TRUE;
    }else{
      return MTK_GPS_FALSE;
    }
}

 /*****************************************************************************
  * FUNCTION
  *  is_quarter_epo_bd_valid
  * DESCRIPTION
  *  Judge the Beidou QEPO file of current download is valid or not
  *  if invalid, will retry downloading.
  *  Valid: file open successfully && get HAS_EPO flag successfully && get start time successfully
  *           && request time is in the valid durration of current Beidou QEPO file (start time+6H).
  * PARAMETERS
  *  None
  * RETURNS
  *  EPO_DOWNLOAD_RESULT_FAIL; EPO_DOWNLOAD_RESULT_SUCCESS
  *****************************************************************************/
static int is_quarter_epo_bd_valid(void) {
    unsigned int u4GpsSecs_start;  // GPS seconds
    time_t current_qbd_start_time_utc_s;   // UTC seconds
    time_t current_rqst_time_utc_s;
    time_t current_dl_time_utc_s;
    UINT32 bd_header_data[MTK_EPO_ONE_SV_SIZE/4] = {0};
    int file_size = 0;
    int sv_cnt = 0;

    int fd = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    fd = open(QEPO_BD_UPDATE_FILE, O_RDONLY);
    if (fd < 0) {
        /*Something goes wrong, set HAS_EPO & time correct flag to true for next downloading*/
        g_qepo_bd_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QBD.DAT file on server is valid or not
        g_qepo_bd_file_size_correct = MTK_GPS_FALSE;
        g_qepo_bd_file_time_correct = MTK_GPS_TRUE;

        LOGE("Open QEPO_BD fail(%s), return\n",strerror(errno));
        ret = EPO_DOWNLOAD_RESULT_FAIL;
        goto func_exit;
    } else {
        LOGD_ENG("gps_time.wn, tow %d, %d\n", gps_time.wn, gps_time.tow);
        GpsToUtcTime(gps_time.wn, gps_time.tow, &current_rqst_time_utc_s);
        LOGD("Request GPS time: %s", ctime(&current_rqst_time_utc_s));

        gps_mnl_set_qbd_latest_dl_time(current_rqst_time_utc_s);

        if(mtk_gps_sys_qepo_bd_get_header(fd, bd_header_data)){
             /*Something goes wrong, set HAS_EPO & time correct flag to true for next downloading*/
            g_qepo_bd_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QBD.DAT file on server is valid or not
            g_qepo_bd_file_size_correct = MTK_GPS_FALSE;
            g_qepo_bd_file_time_correct = MTK_GPS_TRUE;

            LOGD_ENG("qepo bd get header fail, return\n");
            ret = EPO_DOWNLOAD_RESULT_FAIL;
            goto func_exit;
        }

        //Check HAS_EPO flag
        mtk_gps_sys_qepo_bd_has_epo(bd_header_data[0], &g_qepo_bd_has_epo);
        mtk_gps_sys_qepo_bd_get_sv_cnt(bd_header_data[1], &sv_cnt);

        //Check file size
        file_size = gps_mnl_epo_get_file_size(fd);
        LOGD_ENG("qepo bd file size:%d\n",file_size);
        if((file_size==0) || ((file_size%MTK_EPO_ONE_SV_SIZE != 0)||
            (sv_cnt != (file_size/MTK_EPO_ONE_SV_SIZE-1)))){  //The BD QEPO file size and sv cnt not match, something goes wrong
            LOGE("qepo bd file size check error\n");
            g_qepo_bd_file_size_correct = MTK_GPS_FALSE;
        }else{
            g_qepo_bd_file_size_correct = MTK_GPS_TRUE;
        }

        //Check time
        if (mtk_gps_sys_epo_bd_period_start(fd, &u4GpsSecs_start, &current_qbd_start_time_utc_s)) {
            /*Something goes wrong, set time correct flag to true for next downloading*/
            g_qepo_bd_file_time_correct = MTK_GPS_TRUE;

            LOGE("Read QEPO file failed\n");
            ret = EPO_DOWNLOAD_RESULT_FAIL;
            goto func_exit;
        } else {
            LOGD_ENG("The Start time of QEPO file is %lld\n", (long long)current_qbd_start_time_utc_s);
            LOGD("The start time of QEPO file is %s\n", ctime(&current_qbd_start_time_utc_s));
            current_dl_time_utc_s = current_rqst_time_utc_s+(time((time_t *)NULL)-gps_time.sys_time);
            LOGD_ENG("Time after download: %s\n", ctime(&current_dl_time_utc_s));
            if ((current_dl_time_utc_s >= current_qbd_start_time_utc_s) && (current_dl_time_utc_s < ((6*60*60) + current_qbd_start_time_utc_s))) {
                g_qepo_bd_file_time_correct = MTK_GPS_TRUE;
            }else{
                LOGE("qepo bd file time check error\n");
                g_qepo_bd_file_time_correct = MTK_GPS_FALSE;
            }
        }
    }

    if(g_qepo_bd_has_epo == MTK_GPS_TRUE
        && g_qepo_bd_file_size_correct == MTK_GPS_TRUE
        && g_qepo_bd_file_time_correct == MTK_GPS_TRUE)
    {
        ret = EPO_DOWNLOAD_RESULT_SUCCESS;
    }else{
        ret = EPO_DOWNLOAD_RESULT_FAIL;
    }

func_exit:
    if(fd >= 0){
        close(fd);
    }

    return ret;
}

static int is_quarter_epo_ga_valid(void) {
    unsigned int u4GpsSecs_start;  // GPS seconds
    time_t current_qga_start_time_utc_s;   // UTC seconds
    time_t current_rqst_time_utc_s;
    time_t current_dl_time_utc_s;
    UINT32 ga_header_data[MTK_EPO_ONE_SV_SIZE/4] = {0};
    int file_size = 0;
    int sv_cnt = 0;

    int fd = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    fd = open(QEPO_GA_UPDATE_FILE, O_RDONLY);
    if (fd < 0) {
        /*Something goes wrong, set HAS_EPO & time correct flag to true for next downloading*/
        g_qepo_ga_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QGA.DAT file on server is valid or not
        g_qepo_ga_file_size_correct = MTK_GPS_FALSE;
        g_qepo_ga_file_time_correct = MTK_GPS_TRUE;

        LOGE("Open QEPO_GA fail(%s), return\n",strerror(errno));
        ret = EPO_DOWNLOAD_RESULT_FAIL;
        goto func_exit;
    } else {
        LOGD_ENG("gps_time.wn, tow %d, %d\n", gps_time.wn, gps_time.tow);
        GpsToUtcTime(gps_time.wn, gps_time.tow, &current_rqst_time_utc_s);
        LOGD("Request GPS time: %s", ctime(&current_rqst_time_utc_s));

        gps_mnl_set_qga_latest_dl_time(current_rqst_time_utc_s);

        if(mtk_gps_sys_qepo_ga_get_header(fd, ga_header_data)){
             /*Something goes wrong, set HAS_EPO & time correct flag to true for next downloading*/
            g_qepo_ga_has_epo = MTK_GPS_TRUE;  //This flag to indentify the QGA.DAT file on server is valid or not
            g_qepo_ga_file_size_correct = MTK_GPS_FALSE;
            g_qepo_ga_file_time_correct = MTK_GPS_TRUE;

            LOGD_ENG("qepo ga get header fail, return\n");
            ret = EPO_DOWNLOAD_RESULT_FAIL;
            goto func_exit;
        }

        //Check HAS_EPO flag
        mtk_gps_sys_qepo_ga_has_epo(ga_header_data[0], &g_qepo_ga_has_epo);
        mtk_gps_sys_qepo_ga_get_sv_cnt(ga_header_data[1], &sv_cnt);

        //Check file size
        file_size = gps_mnl_epo_get_file_size(fd);
        LOGD_ENG("qepo ga file size:%d\n",file_size);
        if((file_size==0) || ((file_size%MTK_EPO_ONE_SV_SIZE != 0)||
            (sv_cnt != (file_size/MTK_EPO_ONE_SV_SIZE-1)))){  //The GA QEPO file size and sv cnt not match, something goes wrong
            LOGE("qepo ga file size check error\n");
            g_qepo_ga_file_size_correct = MTK_GPS_FALSE;
        }else{
            g_qepo_ga_file_size_correct = MTK_GPS_TRUE;
        }

        //Check time
        if (mtk_gps_sys_epo_ga_period_start(fd, &u4GpsSecs_start, &current_qga_start_time_utc_s)) {
            /*Something goes wrong, set time correct flag to true for next downloading*/
            g_qepo_ga_file_time_correct = MTK_GPS_TRUE;

            LOGE("Read QEPO file failed\n");
            ret = EPO_DOWNLOAD_RESULT_FAIL;
            goto func_exit;
        } else {
            LOGD_ENG("The Start time of QEPO file is %lld\n", (long long)current_qga_start_time_utc_s);
            LOGD("The start time of QEPO file is %s\n", ctime(&current_qga_start_time_utc_s));
            current_dl_time_utc_s = current_rqst_time_utc_s+(time((time_t *)NULL)-gps_time.sys_time);
            LOGD_ENG("Time after download: %s\n", ctime(&current_dl_time_utc_s));
            if ((current_dl_time_utc_s >= current_qga_start_time_utc_s) && (current_dl_time_utc_s < ((6*60*60) + current_qga_start_time_utc_s))) {
                g_qepo_ga_file_time_correct = MTK_GPS_TRUE;
            }else{
                LOGE("qepo ga file time check error\n");
                g_qepo_ga_file_time_correct = MTK_GPS_FALSE;
            }
        }
    }

    if(g_qepo_ga_has_epo == MTK_GPS_TRUE
        && g_qepo_ga_file_size_correct == MTK_GPS_TRUE
        && g_qepo_ga_file_time_correct == MTK_GPS_TRUE)
    {
        ret = EPO_DOWNLOAD_RESULT_SUCCESS;
    }else{
        ret = EPO_DOWNLOAD_RESULT_FAIL;
    }

func_exit:
    if(fd >= 0){
        close(fd);
    }

    return ret;
}
#if 0
static void
gps_download_quarter_epo_file_name(int count) {
    if (gps_epo_type == 1) {
        if (count == 1) {
            strncpy(quarter_epo_file_name, "QG_R_1.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 2) {
            strncpy(quarter_epo_file_name, "QG_R_2.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 3) {
            strncpy(quarter_epo_file_name, "QG_R_3.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 4) {
            strncpy(quarter_epo_file_name, "QG_R_4.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 5) {
            strncpy(quarter_epo_file_name, "QG_R_5.DAT", GPS_EPO_FILE_LEN);
        }
        LOGD("quarter_epo_file_name=%s\n", quarter_epo_file_name);
    } else if (gps_epo_type == 0) {
        if (count == 1) {
            strncpy(quarter_epo_file_name, "QG_R_1.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 2)  {
            strncpy(quarter_epo_file_name, "QG_R_2.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 3) {
            strncpy(quarter_epo_file_name, "QG_R_3.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 4) {
            strncpy(quarter_epo_file_name, "QG_R_4.DAT", GPS_EPO_FILE_LEN);
        } else if (count == 5) {
            strncpy(quarter_epo_file_name, "QG_R_5.DAT", GPS_EPO_FILE_LEN);
        }
        LOGD("quarter_epo_file_name=%s\n", quarter_epo_file_name);
    }
}
#endif
void gps_mnl_set_gps_time(int wn, int tow, int sys_time) {
    gps_time.wn = wn;
    gps_time.tow = tow;
    gps_time.sys_time = sys_time;
}
static void quarter_epo_download_process(void) {
    // LOGD("quarter_epo_download_process begin");
/*    int index = 1;
    INT32 SecofDay = gps_time.tow % 86400;

    if ((SecofDay > 300) && (SecofDay <= 21900)) {
        index = 1;
    } else if ((SecofDay > 21900) && (SecofDay <= 43500)) {
        index = 2;
    } else if ((SecofDay > 43500) && (SecofDay <= 65100)) {
        index = 3;
    } else if ((SecofDay > 65100) && (SecofDay <= 85500)) {
        index = 4;
    } else if ((SecofDay <= 300) || (SecofDay > 85500)) {
        if (server_not_updated) {
            index = 4;
        } else {
            index = 5;
        }
    }

    if (pre_day) {
        index = 5;
    }
    LOGD("SecofDay = %d , index = %d\n", SecofDay, index);
    gps_download_quarter_epo_file_name(index);*/
    strncpy(quarter_epo_file_name, "QG_R.DAT", sizeof(quarter_epo_file_name));
    curl_easy_download_quarter_epo();
}

static void quarter_epo_bd_download_process(void) {
    strncpy(quarter_epo_bd_file_name, "QBD2.DAT",sizeof(quarter_epo_bd_file_name));
    strncpy(quarter_epo_bd_md_file_name, "QBD2.MD5",sizeof(quarter_epo_bd_md_file_name));
    curl_easy_download_quarter_epo_bd();
}

static void quarter_epo_ga_download_process(void) {
    strncpy(quarter_epo_ga_file_name, "QGA.DAT",sizeof(quarter_epo_ga_file_name));
    curl_easy_download_quarter_epo_ga();
}

static int qepo_file_update_impl() {
    int try_time = QEPO_GR_DL_RETRY_TIME;  // for network issue download failed retry.
    int qepo_valid = EPO_DOWNLOAD_RESULT_FAIL;

    //LOGD("qepo_download_finished = 0\n");
    quarter_epo_download_process();
    while (((qepo_valid = is_quarter_epo_valid()) == EPO_DOWNLOAD_RESULT_FAIL)
           && (try_time > 0) && is_network_connected()) {
        try_time--;

        LOGD("qepo download failed try again, try_time = %d\n", try_time);
        quarter_epo_download_process();
    }
    LOGD_ENG("try time is %d, qepo_valid is %d\n", try_time, qepo_valid);
    if (try_time < QEPO_GR_DL_RETRY_TIME) {
        try_time = QEPO_GR_DL_RETRY_TIME;
    }
    if (server_not_updated) {
        Qepo_res = CURLE_RECV_ERROR;  // server has not updated
    }
    return qepo_valid;  // success
}

static int qepo_bd_file_update_impl() {
    int qepo_bd_valid = EPO_DOWNLOAD_RESULT_FAIL;

 /*Download directly.
  *If the notwork is unavailable, will download fail, is_quarter_epo_bd_valid() will return FAIL.
  *If the network is available and download successfully, is_quarter_epo_bd_valid will return TRUE*/
    LOGD_ENG("qepo download start, network conncet:%d...\n",is_network_connected());
    quarter_epo_bd_download_process();

    qepo_bd_valid = is_quarter_epo_bd_valid();
    LOGD("qepo_bd_valid is %d, has epo:%d, file time correct:%d, file size correct:%d\n",  qepo_bd_valid,g_qepo_bd_has_epo,g_qepo_bd_file_time_correct,g_qepo_bd_file_size_correct);
    if((is_quarter_epo_bd_on_server_valid() == MTK_GPS_FALSE) ||
        (g_qepo_bd_file_size_correct == MTK_GPS_FALSE))
    {
        if(g_qepo_bd_invalid_dl_cnt == 0)
        {
            gps_mnl_set_qbd_first_invalid_dl_time(gps_mnl_get_qbd_latest_dl_time());//record the firest invalid download times
        }
        g_qepo_bd_invalid_dl_cnt++;
    }

    return qepo_bd_valid;
}

static int qepo_ga_file_update_impl() {
    int qepo_ga_valid = EPO_DOWNLOAD_RESULT_FAIL;

 /*Download directly.
  *If the notwork is unavailable, will download fail, is_quarter_epo_ga_valid() will return FAIL.
  *If the network is available and download successfully, is_quarter_epo_ga_valid will return TRUE*/
    LOGD_ENG("qepo download start, network conncet:%d...\n",is_network_connected());
    quarter_epo_ga_download_process();

    qepo_ga_valid = is_quarter_epo_ga_valid();
    LOGD("qepo_ga_valid is %d, has epo:%d, file time correct:%d, file size correct:%d\n",  qepo_ga_valid,g_qepo_ga_has_epo,g_qepo_ga_file_time_correct,g_qepo_ga_file_size_correct);
    if((is_quarter_epo_ga_on_server_valid() == MTK_GPS_FALSE) ||
        (g_qepo_ga_file_size_correct == MTK_GPS_FALSE))
    {
        if(g_qepo_ga_invalid_dl_cnt == 0)
        {
            gps_mnl_set_qga_first_invalid_dl_time(gps_mnl_get_qga_latest_dl_time());//record the firest invalid download times
        }
        g_qepo_ga_invalid_dl_cnt++;
    }

    return qepo_ga_valid;
}

void qepo_update_quarter_epo_file(int qepo_valid) {
    int qdownload_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;

    if (qepo_valid == EPO_DOWNLOAD_RESULT_SUCCESS) {
        if (mtk_agps_agent_qepo_file_update() == MTK_GPS_ERROR) {
            qdownload_status = MTK_QEPO_RSP_UPDATE_FAIL;
        } else {
            qdownload_status = MTK_QEPO_RSP_UPDATE_SUCCESS;
            unlink(QEPO_UPDATE_HAL);
        }
    } else {
        qdownload_status = Qepo_res;
        if (qdownload_status > 0) {
            qdownload_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;
        }
    }

    qdownload_status |= ((AGT_QEPO_GP_BIT|AGT_QEPO_GL_BIT)<< MTK_QEPO_RSP_BIT_NUM);
    LOGD("qdownload_status = 0x%x\n", qdownload_status);
    if (MTK_GPS_ERROR ==  (mtk_agps_set_param (MTK_PARAM_QEPO_DOWNLOAD_RESPONSE,
            &qdownload_status, MTK_MOD_DISPATCHER, MTK_MOD_AGENT))) {
            LOGE("GPS QEPO update fail\n");
        }
}

void qepo_update_quarter_epo_bd_file(int qepo_bd_valid) {
    int qdownload_bd_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;

    if (qepo_bd_valid == EPO_DOWNLOAD_RESULT_SUCCESS) {
        if (MTK_GPS_ERROR == mtk_agps_agent_qepo_bd_file_update()) {
            qdownload_bd_status = MTK_QEPO_RSP_UPDATE_FAIL;
            LOGE("qdownload_bd_status = %d, download fail\n", qdownload_bd_status);
    } else {
            qdownload_bd_status = MTK_QEPO_RSP_UPDATE_SUCCESS;
            unlink(QEPO_BD_UPDATE_FILE);
        }
    } else {
        if((is_quarter_epo_bd_on_server_valid() == MTK_GPS_TRUE) && (g_qepo_bd_file_size_correct == MTK_GPS_FALSE))
        {
             if (MTK_GPS_ERROR == mtk_agps_agent_qepo_bd_file_update()) {
                qdownload_bd_status = MTK_QEPO_RSP_UPDATE_FAIL;
                LOGE("qdownload_bd_status = %d, download fail\n", qdownload_bd_status);
            } else {
                 qdownload_bd_status = MTK_QEPO_RSP_SIZE_FAIL;
                unlink(QEPO_BD_UPDATE_FILE);
        }
    }
        else
        {
            qdownload_bd_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;
        }
        }

    qdownload_bd_status = qdownload_bd_status |(AGT_QEPO_BD_BIT<< MTK_QEPO_RSP_BIT_NUM);   // high 4 bit
    if (MTK_GPS_ERROR == (mtk_agps_set_param (MTK_PARAM_QEPO_DOWNLOAD_RESPONSE,
        &qdownload_bd_status, MTK_MOD_DISPATCHER, MTK_MOD_AGENT)) ) {
        LOGE("GPS QEPO_BD update fail\n");
    }
}

void qepo_update_quarter_epo_ga_file(int qepo_ga_valid) {
    int qdownload_ga_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;

    if (qepo_ga_valid == EPO_DOWNLOAD_RESULT_SUCCESS) {
        if (MTK_GPS_ERROR == mtk_agps_agent_qepo_ga_file_update()) {
            qdownload_ga_status = MTK_QEPO_RSP_UPDATE_FAIL;
            LOGE("qdownload_ga_status = %d, download fail\n", qdownload_ga_status);
        } else {
            qdownload_ga_status = MTK_QEPO_RSP_UPDATE_SUCCESS;
            unlink(QEPO_GA_UPDATE_FILE);
        }
    } else {
        if((is_quarter_epo_ga_on_server_valid() == MTK_GPS_TRUE) && (g_qepo_ga_file_size_correct == MTK_GPS_FALSE))
        {
            if (MTK_GPS_ERROR == mtk_agps_agent_qepo_ga_file_update()) {
                qdownload_ga_status = MTK_QEPO_RSP_UPDATE_FAIL;
                LOGE("qdownload_ga_status = %d, download fail\n", qdownload_ga_status);
            } else {
                qdownload_ga_status = MTK_QEPO_RSP_SIZE_FAIL;
                unlink(QEPO_GA_UPDATE_FILE);
            }
        } else {
            qdownload_ga_status = MTK_QEPO_RSP_DOWNLOAD_FAIL;
        }
    }

    qdownload_ga_status = qdownload_ga_status |(AGT_QEPO_GA_BIT<< MTK_QEPO_RSP_BIT_NUM);   // high 4 bit
    if (MTK_GPS_ERROR == (mtk_agps_set_param (MTK_PARAM_QEPO_DOWNLOAD_RESPONSE,
        &qdownload_ga_status, MTK_MOD_DISPATCHER, MTK_MOD_AGENT)) ) {
        LOGE("GPS QEPO_GA update fail\n");
    }
}

//////////////////////////////////////////////////////////////////////////////////
// MAIN -> EPO Download (handlers)
static int mnld_qepo_download() {
    //LOGD("mnld_qepo_download");

    qepo_download_finished = 0;
    int ret = qepo_file_update_impl();
    qepo_download_finished = 1;
    qepo_dl_res = ret;
    mnld_qepo_download_done(ret);
    return ret;
}

static int mnld_qepo_bd_download() {
    time_t current_rqst_time_utc_s = 0;
    time_t qbd_rqst_time_diff = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    GpsToUtcTime(gps_time.wn, gps_time.tow, &current_rqst_time_utc_s);

    qbd_rqst_time_diff = current_rqst_time_utc_s/SECONDS_PER_HOUR - gps_mnl_get_qbd_first_invalid_dl_time()/SECONDS_PER_HOUR;
    LOGD_ENG("QBD current request time: %ld, latest invalid dl time:%ld, time_diff:%ld,\n", current_rqst_time_utc_s,gps_mnl_get_qbd_first_invalid_dl_time(),qbd_rqst_time_diff);
    if(qbd_rqst_time_diff != 0){ //Rest the count value hourly
        g_qepo_bd_invalid_dl_cnt = 0;
        LOGD_ENG("reset invalid download cnt\n");
    }

    //qbd_rqst_time_diff = current_rqst_time_utc_s/SECONDS_PER_HOUR - gps_mnl_get_qbd_latest_dl_time()/SECONDS_PER_HOUR;
    LOGD_ENG("has epo:%d, time correct:%d,size correct:%d,invalid download cnt:%d\n", g_qepo_bd_has_epo,g_qepo_bd_file_time_correct,g_qepo_bd_file_size_correct,g_qepo_bd_invalid_dl_cnt);
    if(g_qepo_bd_invalid_dl_cnt < QEPO_BD_DL_RETRY_TIME)//The invalid download count of one hour
    {

        qepo_bd_download_finished = 0;
        ret = qepo_bd_file_update_impl();
        qepo_bd_download_finished = 1;
    }else{
        usleep(QEPO_BD_INVALID_SLEEP);
        ret = EPO_DOWNLOAD_RESULT_FAIL;
        LOGW("The qbd data on server is invalid, will download again when the requesting is come in in next hour!\n");
    }

    qepo_bd_dl_res = ret;

    mnld_qepo_bd_download_done(ret);
    return ret;
}

static int mnld_qepo_ga_download() {
    time_t current_rqst_time_utc_s = 0;
    time_t qga_rqst_time_diff = 0;
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    GpsToUtcTime(gps_time.wn, gps_time.tow, &current_rqst_time_utc_s);

    qga_rqst_time_diff = current_rqst_time_utc_s/SECONDS_PER_HOUR - gps_mnl_get_qga_first_invalid_dl_time()/SECONDS_PER_HOUR;
    LOGD_ENG("QGA current request time: %ld, latest invalid dl time:%ld, time_diff:%ld,\n", current_rqst_time_utc_s,gps_mnl_get_qga_first_invalid_dl_time(),qga_rqst_time_diff);
    if(qga_rqst_time_diff != 0){ //Rest the count value hourly
        g_qepo_ga_invalid_dl_cnt = 0;
        LOGD_ENG("reset invalid download cnt\n");
    }

    //qbd_rqst_time_diff = current_rqst_time_utc_s/SECONDS_PER_HOUR - gps_mnl_get_qbd_latest_dl_time()/SECONDS_PER_HOUR;
    LOGD_ENG("has epo:%d, time correct:%d,size correct:%d,invalid download cnt:%d\n", g_qepo_ga_has_epo,g_qepo_ga_file_time_correct,g_qepo_ga_file_size_correct,g_qepo_ga_invalid_dl_cnt);
    if(g_qepo_ga_invalid_dl_cnt < QEPO_GA_DL_RETRY_TIME)//The invalid download count of one hour
    {

        qepo_ga_download_finished = 0;
        ret = qepo_ga_file_update_impl();
        qepo_ga_download_finished = 1;
    }else{
        usleep(QEPO_GA_INVALID_SLEEP);
        ret = EPO_DOWNLOAD_RESULT_FAIL;
        LOGW("The qga data on server is invalid, will download again when the requesting is come in in next hour!\n");
    }

    qepo_ga_dl_res = ret;

    mnld_qepo_ga_download_done(ret);
    return ret;
}

int is_qepo_download_finished() {
    return qepo_download_finished;
}

int is_qepo_bd_download_finished() {
    return qepo_bd_download_finished;
}

int is_qepo_ga_download_finished() {
    return qepo_ga_download_finished;
}

static int qepo_event_hdlr(int fd) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    main2qepo_event cmd;
    int read_len;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("qepo_event_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    cmd = get_int(buff, &offset, sizeof(buff));
    switch (cmd) {
    case MAIN2QEPO_EVENT_START: {
        LOGD_ENG("mnld_qepo_download() before");
        // need to call mnld_qepo_download_done() when QEPO download is done
        mnld_qepo_download();
        LOGD_ENG("mnld_qepo_download() after");
        break;
    }
    case MAIN2QEPO_BD_EVENT_START: {
        LOGD_ENG("mnld_qepo_bd_download() before");
        mnld_qepo_bd_download();
        LOGD_ENG("mnld_qepo_bd_download() after");
        break;
    }
    case MAIN2QEPO_GA_EVENT_START: {
        LOGD_ENG("mnld_qepo_ga_download() before");
        mnld_qepo_ga_download();
        LOGD_ENG("mnld_qepo_ga_download() after");
        break;
    }

    default: {
        LOGE("qepo_event_hdlr() unknown cmd=%d", cmd);
        return -1;
    }
    }
    return 0;
}
#if 0
static void qepo_downloader_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("qepo_downloader_thread_timeout() dump and exit.");
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("qepo_downloader_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}
#endif
static void* qepo_downloader_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    //timer_t hdlr_timer = init_timer(qepo_downloader_thread_timeout);
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("qepo_downloader_thread() epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return 0;
    }

    if (epoll_add_fd(epfd, g_fd_qepo) == -1) {
        LOGE("qepo_downloader_thread() epoll_add_fd() failed for g_fd_qepo failed");
        return 0;
    }

    while (1) {
        int i;
        int n;
        LOGD_ENG("qepo_downloader_thread wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("qepo_downloader_thread() epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        //start_timer(hdlr_timer, MNLD_QEPO_HANDLER_TIMEOUT);
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_qepo) {
                if (events[i].events & EPOLLIN) {
                    qepo_event_hdlr(g_fd_qepo);
                }
            } else {
                LOGE("qepo_downloader_thread() unknown fd=%d",
                    events[i].data.fd);
            }
        }
        //stop_timer(hdlr_timer);
    }

    LOGE("qepo_downloader_thread() exit");
    return 0;
}

int qepo_downloader_start() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    if (qepo_download_finished == 0) {
        LOGW("qepo downloading... abort requst msg!");
        return 0;
    }
    put_int(buff, &offset, MAIN2QEPO_EVENT_START);
    return safe_sendto(MNLD_QEPO_DOWNLOAD_SOCKET, buff, offset);
}


int qepo_bd_downloader_start() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    if (qepo_bd_download_finished == 0) {
        LOGW("qepo bd downloading... abort requst msg!");
        return 0;
    }
    put_int(buff, &offset, MAIN2QEPO_BD_EVENT_START);
    return safe_sendto(MNLD_QEPO_DOWNLOAD_SOCKET, buff, offset);
}

int qepo_ga_downloader_start() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    if (qepo_ga_download_finished == 0) {
        LOGW("qepo ga downloading... abort requst msg!");
        return 0;
    }
    put_int(buff, &offset, MAIN2QEPO_GA_EVENT_START);
    return safe_sendto(MNLD_QEPO_DOWNLOAD_SOCKET, buff, offset);
}

int qepo_downloader_init() {
    pthread_t pthread_qepo;

    g_fd_qepo = socket_bind_udp(MNLD_QEPO_DOWNLOAD_SOCKET);
    if (g_fd_qepo < 0) {
        LOGE("socket_bind_udp(MNLD_QEPO_DOWNLOAD_SOCKET) failed");
        return -1;
    }

    pthread_create(&pthread_qepo, NULL, qepo_downloader_thread, NULL);
    gps_mnl_bd_qepo_init();
    return 0;
}

void qepo_invalid_dl_cnt_clear(void) {
    g_qepo_bd_invalid_dl_cnt = 0;
    g_qepo_ga_invalid_dl_cnt = 0;
}
