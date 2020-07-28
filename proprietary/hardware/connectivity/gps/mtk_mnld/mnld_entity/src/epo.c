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
#include <openssl/md5.h>

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
#define LOG_TAG "epo"

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
#define MTK_EPO_MAX_DAY      30
#define MTK_EPO_DEFAULT_DL_DAY  3
//#define MTK_EPO_ONE_SV_SIZE  72 //Move to epo.h
#define GPS_CONF_FILE_SIZE 100
#define EPO_CONTROL_FILE_PATH "/data/vendor/gps/gps.conf"
#define IS_SPACE(ch) ((ch == ' ') || (ch == '\t') || (ch == '\n'))
#define MTK_EPO_SET_GPS_SVNum  32
#define MTK_EPO_SET_GLO_SVNum  24
#define MTK_EPO_SET_GPS_MAX_SIZE (72*MTK_EPO_SET_GPS_SVNum)  //72*32, One SET
#define MTK_EPO_SET_GLO_MAX_SIZE (72*MTK_EPO_SET_GLO_SVNum)  //72*24, One SET
#define MTK_EPO_SET_GPS_GLO_MAX_SIZE (MTK_EPO_SET_GPS_MAX_SIZE + MTK_EPO_SET_GLO_MAX_SIZE)  //72*(32+24), One SET for GPS+GLO
#define MTK_EPO_MD5_STR_LEN (MD5_DIGEST_LENGTH*2+1)
#define MTK_EPO_READ_BUFSIZE (72*56)

static int gps_epo_period = 3;
static int wifi_epo_period = 3;
static int gps_epo_download_days = MTK_EPO_DEFAULT_DL_DAY;
static int gps_epo_download_piece = 1;
static int gps_epo_enable = 1;
static int gps_epo_wifi_trigger = 0;
static int gps_epo_file_count = 0;
static char gps_epo_file_name[GPS_EPO_FILE_LEN] = {0};
static char gps_epo_MD5_file_name[GPS_EPO_FILE_LEN] = {0};
int gps_epo_type = 0;    // 0 for G+G;1 for GPS only, default is G+G
//static int epo_download_failed = 0;
static int epo_download_retry = 1;
static timer_t retry_download_timer;
//static timer_t hdlr_timer;
static EPO_Status_T epo_status = {
    .last_DL_Date = -1,
    .today_retry_time = 0,
};

static int mtk_epo_is_expired(int wifi_tragger);
static void gps_download_epo_file_name(int count);
static int mtk_gps_epo_file_time_hal(time_t uTime[]);
int mtk_gps_epo_gen_md5_str(FILE *f, char *md_str, unsigned int str_len);
int mtk_gps_epo_md5_match_check(char *file_epo, char *file_md5);
int mtk_gps_md5_hex2str(unsigned char *md, char *md_str, unsigned int str_len);
CURLcode curl_easy_download_epo_DAT(void);

CURLcode curl_easy_download_epo_MD5(void);

typedef enum {
    MAIN2EPO_EVENT_START            = 0,
} main2epo_event;

typedef enum
{
  MTK_EPO_GPS_GLO = 0,
  MTK_EPO_GPS,
  MTK_EPO_END
} MTK_EPO_TYPE;

static int g_fd_epo;

/////////////////////////////////////////////////////////////////////////////////
// static functions
static MTK_FILE EPO_File_Open (const char *szFileName, MTK_INT32 i4Mode)
{
    FILE *fp;
    char szMode[4];

    // For system which treats binary mode and text mode differently,
    // such as Windows / DOS, please make sure to open file in BINARY mode

    switch (i4Mode)
    {
    case MTK_FS_READ:       // 0
        sprintf(szMode, "rb");
        break;
    case MTK_FS_WRITE:      // 1
        sprintf(szMode, "wb");
        break;
    case MTK_FS_APPEND:     // 2
        sprintf(szMode, "ab");
        break;
    case MTK_FS_RW:         // 3
        sprintf(szMode, "r+b");
        break;
    case MTK_FS_RW_DISCARD: // 4
        sprintf(szMode, "w+b");
        break;
    case MTK_FS_RW_APPEND:  // 5
        sprintf(szMode, "a+b");
        break;
    default:
        return 0;
    }

    fp = fopen(szFileName, szMode);

    if (fp != NULL)
    {
        return (MTK_FILE)fp;
    }

    return 0;
}

static void EPO_File_Close (MTK_FILE hFile)
{
    fclose((FILE *)hFile);
}

static MTK_UINT32 EPO_File_Read (MTK_FILE hFile, void *DstBuf, MTK_UINT32 u4Length)
{
    if (hFile != 0)
    {
        return (MTK_UINT32)fread(DstBuf, 1, u4Length, (FILE *)hFile);
    }

    return 0;
}

void epo_update_epo_file() {
    unlink(EPO_FILE);
    if (mtk_agps_agent_epo_file_update() == MTK_GPS_ERROR) {
        LOGE("EPO file updates fail\n");
    } else {
        unlink(EPO_UPDATE_HAL);
    }
}

int mtk_gps_epo_md5_match_check(char *file_epo, char *file_md5)
{
    FILE *fp_epo = NULL, *fp_md5 = NULL;
    char md5_data[MTK_EPO_MD5_STR_LEN] = {0};
    char gen_md5_str[MTK_EPO_MD5_STR_LEN] = {0};
    int read_len = 0;

    if(file_epo == NULL || file_md5 == NULL)
    {
        return MTK_GPS_ERROR;
    }

    fp_epo = fopen(file_epo, "r");
    if (fp_epo == NULL)
    {
        LOGE("epo file(%s) open fail(%s).", file_epo, strerror(errno));
        return MTK_GPS_ERROR;
    }
    if (mtk_gps_epo_gen_md5_str(fp_epo, gen_md5_str, MTK_EPO_MD5_STR_LEN) == MTK_GPS_ERROR) {
        LOGE("Gen md5 str failed");
        fclose(fp_epo);
        return MTK_GPS_ERROR;
    }
    fclose(fp_epo);

    fp_md5 = fopen(file_md5, "r");
    if(fp_md5 == NULL)
    {
        LOGE("md5 file(%s) open fail(%s).", file_md5, strerror(errno));
        return MTK_GPS_ERROR;
    }
    read_len = fread(md5_data, 1, sizeof(md5_data), fp_md5);
    md5_data[MTK_EPO_MD5_STR_LEN - 1] = '\0';
    fclose(fp_md5);
    if (read_len <= 0) {
        return MTK_GPS_ERROR;
    }

    if(strncmp(gen_md5_str, md5_data, MTK_EPO_MD5_STR_LEN-1) == 0)
    {
        return MTK_GPS_SUCCESS;
    }else{
        LOGW("Gen:%s, MD5File:%s", gen_md5_str, md5_data);
        return MTK_GPS_ERROR;
    }
}

int mtk_gps_epo_gen_md5_str(FILE *f, char *md_str, unsigned int str_len)
{
    MD5_CTX c;
    int read_len;
    unsigned char buf[MTK_EPO_READ_BUFSIZE] = {0};
    unsigned char md5[MD5_DIGEST_LENGTH] = {0};
    int i;

    if (f == NULL || md_str == NULL) {
        LOGE("NULL pointer: f=0x%08x, md_str=0x%08x", (unsigned int)f, (unsigned int)md_str);
        return MTK_GPS_ERROR;
    }

    if(str_len < MTK_EPO_MD5_STR_LEN)
    {
        LOGE("str_len: %d", str_len);
        return MTK_GPS_ERROR;
    }

    MD5_Init(&c);
    do
    {
        read_len = fread(buf, 1, MTK_EPO_READ_BUFSIZE, f);
        if (read_len <= 0) break;
        MD5_Update(&c,buf,(unsigned long)read_len);
    }while(read_len > 0);
    MD5_Final(&(md5[0]),&c);

    for (i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        snprintf(&(md_str[i*2]), 3, "%02x", md5[i]);
    }
    md_str[i*2] = '\0';

    return MTK_GPS_SUCCESS;
}

int mtk_gps_md5_hex2str(unsigned char *md, char *md_str, unsigned int str_len)
{
    int i;

    if(str_len < MTK_EPO_MD5_STR_LEN)
    {
        LOGE("str_len: %d", str_len);
        return MTK_GPS_ERROR;
    }

    for (i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        snprintf(&(md_str[i*2]), 3, "%02x", md[i]);
    }
    md_str[i*2] = '\0';

    return MTK_GPS_SUCCESS;
}

static int epo_md5_update_retry(int retry_time) {
    int ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    UNUSED(retry_time);
    if (mnld_is_gps_or_ofl_started() || (is_wifi_network_connected())) {
        if (is_network_connected()) {
            //LOGW("download epo MD5 file failed, retry resume after 10s.(times:%d)", retry_time);
            usleep(10*1000*1000);
            if (curl_easy_download_epo_MD5() == CURLE_OK) {
                ret = EPO_MD5_FILE_UPDATED;
            } else {
                ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
            }
        } else {
            //LOGW("download epo MD5 file failed, no network connected, retry times:%d", retry_time);
        }

    } else {
        //LOGW("download epo MD5 file failed, not meet retry condition, retry times:%d", retry_time);
    }
    return ret;
}

static int epo_MD5_download_process(void) {
    int ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
    int md5_retry_cnt = 0;
    int loop_cnt = 0;
    int md5_DL_flag = 1;

    while(1) {
        if (is_qepo_download_finished()&&is_qepo_bd_download_finished()&&is_qepo_ga_download_finished()) {
            if (md5_DL_flag == 1) {
                if (curl_easy_download_epo_MD5() == CURLE_OK) {
                    LOGD("%s download success", gps_epo_MD5_file_name);
                    ret = EPO_MD5_FILE_UPDATED;
                    break;
                } else {
                    //LOGD("download MD5 first time failed");
                    md5_DL_flag = 0;
                }
            } else if (md5_retry_cnt < EPO_DL_MAX_RETRY_TIME) {
                md5_retry_cnt++;
                if (epo_md5_update_retry(md5_retry_cnt) == EPO_MD5_FILE_UPDATED) {
                    LOGD("%s download success", gps_epo_MD5_file_name);
                    ret = EPO_MD5_FILE_UPDATED;
                    break;
                }
            } else {
                LOGD_ENG("Meet max retry time(%d), give up download epo MD5", md5_retry_cnt);
                ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
                break;
            }
        } else {
            // Wait qepo download finish, Maximum wait 30s
            if (loop_cnt < 300) {
                loop_cnt++;
                usleep(100*1000);
            } else {
                LOGW("qepo download hung!!! EPO download exit");
                ret = EPO_MD5_DOWNLOAD_RESULT_FAIL;
                break;
            }
        }
    }
    return ret;
}

static int epo_dat_update_retry(int retry_time) {
    int ret = EPO_DOWNLOAD_RESULT_FAIL;
    if (mnld_is_gps_or_ofl_started() || (is_wifi_network_connected())) {
        if (is_network_connected()) {
            LOGW("download epo DAT file failed, retry resume after 10s.(times:%d)", retry_time);
            usleep(10*1000*1000);
            if (curl_easy_download_epo_DAT() == CURLE_OK) {
                ret = EPO_DOWNLOAD_RESULT_SUCCESS;
            } else {
                ret = EPO_DOWNLOAD_RESULT_FAIL;
            }
        } else {
            usleep(10*1000*1000);
            LOGW("download epo DAT file failed, no network connected, retry times:%d", retry_time);
        }
    } else {
        LOGW("download epo DAT file failed, not meet retry condition, retry times:%d", retry_time);
    }
    return ret;
}

static int epo_DAT_download_process(void) {
    int ret = EPO_DOWNLOAD_RESULT_FAIL;
    int loop_cnt = 0;
    int dat_retry_cnt = 0;
    int dat_DL_flag = 1;

    while(1) {
        if (is_qepo_download_finished()&&is_qepo_bd_download_finished()) {
            if (dat_DL_flag == 1) {
                if (curl_easy_download_epo_DAT() == CURLE_OK) {
                    LOGD("%s download success", gps_epo_file_name);
                    ret = EPO_DOWNLOAD_RESULT_SUCCESS;
                    break;
                } else {
                    dat_DL_flag = 0;
                }
            } else if (dat_retry_cnt < EPO_DL_MAX_RETRY_TIME) {
                dat_retry_cnt++;
                if (epo_dat_update_retry(dat_retry_cnt) == EPO_DOWNLOAD_RESULT_SUCCESS) {
                    LOGD("%s download success", gps_epo_file_name);
                    ret = EPO_DOWNLOAD_RESULT_SUCCESS;
                    break;
                }
            } else {
                LOGE("Meet max retry time(%d), give up download epo DAT", dat_retry_cnt);
                ret = EPO_DOWNLOAD_RESULT_FAIL;
                break;
            }
        } else {
            // Wait qepo download finish, Maximum wait 60s
            if (loop_cnt < 600) {
                loop_cnt++;
                usleep(100*1000);
            } else {
                LOGW("qepo download hung!!! EPO download exit");
                ret = EPO_DOWNLOAD_RESULT_FAIL;
                break;
            }
        }
    }

    return ret;
}

static int epo_DAT_file_merge(int file_length) {
    FILE *fp_temp = NULL;
    FILE *fp = NULL;
    int res_val;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};

    memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));

    strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    if (gps_epo_file_count == 0) {
        LOGD("first piece to merge, delete old EPOHAL.DAT");
        unlink(EPO_UPDATE_HAL);
    }
    LOGD("merge %s file to EPOHAL.DAT", gps_epo_data_file_name);
    res_val = chmod(gps_epo_data_file_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (res_val < 0) {
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    }

    fp_temp = fopen(EPO_UPDATE_HAL, "at");
    if (fp_temp != NULL) {
        fp = fopen(gps_epo_data_file_name, "r");
        if (fp != NULL) {
            #define buf_size  256
            char data[buf_size] = {0};
            int bytes_in = 0, bytes_out = 0;
            int len = 0;

            while ((bytes_in = fread(data, 1, sizeof(data), fp)) > 0
                    && (bytes_in <= (int)(buf_size* sizeof(char)))) {
                bytes_out = fwrite(data, 1, bytes_in, fp_temp);
                if (bytes_in != bytes_out) {
                    LOGD("bytes_in = %d,bytes_out = %d\n", bytes_in, bytes_out);
                }
                len += bytes_out;
                if (file_length != EPO_MERGE_FULL_FILE && len >= file_length) {
                    break;
                }
            }
            fclose(fp);
        } else {
            LOGE("Open merged file fp=NULL\n");
        }
        fclose(fp_temp);
    }
    else {
        LOGE("Open merged file failed\n");
        return -1;
    }
    res_val = chmod(EPO_UPDATE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (res_val < 0) {
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    }
    return 0;
}

static int mtk_epo_1sv_checksum_check( const UINT32 u4SVData[18] )
{
   UINT32  checksum = 0;
   int i=0;
   int fgRet = MTK_GPS_FALSE;

   for (i = 0; i < (18-1); i++)
   {
       checksum = checksum ^ u4SVData[i];  // exclusive OR
   }

   //check data checksum
   if (checksum == u4SVData[17])
   {
        fgRet = MTK_GPS_TRUE;
   }
   return fgRet;
}

static int mtk_extract_epo_data_1SV(MTK_FILE hFile, UINT32 u4SvEpoData[MTK_EPO_ONE_SV_SIZE/4])
{
    memset(u4SvEpoData, 0, MTK_EPO_ONE_SV_SIZE);

    if ( EPO_File_Read(hFile, u4SvEpoData, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE )
    {
        LOGE("read 1 sv data error");
        return MTK_GPS_ERROR;
    }

    return MTK_GPS_SUCCESS;
}

static int epo_DAT_checksum_check(void) {
    MTK_GPS_PARAM_EPO_DATA_CFG epo_data;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    MTK_FILE hFile = 0;
    int u4FileSize;
    int sv_cnt = 0;
    unsigned int max_sv_num;
    unsigned int max_segment_size;
    unsigned int valid_file_length;

    memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));

    strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);

    if ( (hFile = EPO_File_Open(gps_epo_data_file_name, MTK_FS_READ)) == 0)
    {
        LOGE("Open epo DAT file error");
        return MTK_GPS_ERROR;
    }

    if ( (u4FileSize = get_file_size(gps_epo_data_file_name)) == -1) {
        LOGE("get file size error");
        EPO_File_Close(hFile);
        return MTK_GPS_ERROR;
    }

    while (sv_cnt*MTK_EPO_ONE_SV_SIZE < u4FileSize) {
        if ( MTK_GPS_ERROR == mtk_extract_epo_data_1SV(hFile, epo_data.u4EPOWORD) )
        {
            LOGE("extract GPS EPO 1 SV data error");
            break;
        } else {
            if (mtk_epo_1sv_checksum_check(epo_data.u4EPOWORD) == MTK_GPS_TRUE) {
                sv_cnt++;
            } else {
                LOGE("find checksum error sv");
                break;
            }
        }
    }

    EPO_File_Close(hFile);

    if (gps_epo_type == MTK_EPO_GPS) {
        max_sv_num = MTK_EPO_SET_GPS_SVNum;
        max_segment_size = MTK_EPO_SET_GPS_MAX_SIZE;
    } else if (gps_epo_type == MTK_EPO_GPS_GLO) {
        max_sv_num = MTK_EPO_SET_GPS_SVNum + MTK_EPO_SET_GLO_SVNum;
        max_segment_size = MTK_EPO_SET_GPS_GLO_MAX_SIZE;
    } else {
        LOGE("gps epo type is error");
        return MTK_GPS_ERROR;
    }

    if ((sv_cnt / max_sv_num > 8) || gps_epo_file_count > 0) {     // check if valid epo set more than 2 days. (1 epo set is 6hours, 8 means 2days)
        valid_file_length = (unsigned int)(sv_cnt / max_sv_num) * max_segment_size;
        return valid_file_length;
    } else {
        LOGW("no enough epo exist, ignore this piece. sv_cnt(%d)", sv_cnt);
        return MTK_GPS_ERROR;
    }
}


static void check_epo_file_exist(void) {
    int i;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    char gps_epo_md5_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    for(i = 0; i< gps_epo_download_piece; ++i) {
        gps_download_epo_file_name(i);
        memset(gps_epo_md5_file_name, 0x00, sizeof(gps_epo_md5_file_name));
        memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));
        strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
        strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
        strncat(gps_epo_md5_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
        strncat(gps_epo_md5_file_name, gps_epo_MD5_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
        if ((epo_status.EPO_piece_flag[i] & EPO_MD5_AVAILABLE_BIT) == EPO_MD5_AVAILABLE_BIT) {
            if (access(gps_epo_md5_file_name, F_OK) == -1) {
                epo_status.EPO_piece_flag[i] &= (~EPO_MD5_AVAILABLE_BIT);
            }
        }
        if ((epo_status.EPO_piece_flag[i] & EPO_DAT_AVAILABLE_BIT) == EPO_DAT_AVAILABLE_BIT) {
            if (access(gps_epo_data_file_name, F_OK) == -1) {
                epo_status.EPO_piece_flag[i] &= (~EPO_DAT_AVAILABLE_BIT);
            }
        }
    }
}

static int epo_file_download_impl() {
    int ret = EPO_DOWNLOAD_RESULT_SUCCESS;   //DAT file download result
    int ret_MD5 = EPO_MD5_DOWNLOAD_RESULT_FAIL;  //MD5 file download result
    int valid_file_length;
    int i;
    int dl_retry_cnt = 0;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    char gps_epo_md5_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    struct tm  tm;
    time_t now = time(NULL);
    gmtime_r(&now, &tm);
    unlink(EPO_UPDATE_HAL);

    // Check how many pieces should be DL, based on download days of user config
    gps_epo_download_piece = gps_epo_download_days / 3;
    if ((gps_epo_download_days % 3) > 0) {
        gps_epo_download_piece++;
    }

    LOGD_ENG("download info: download piece = %d, today retry time =%d, last_date=%d", gps_epo_download_piece, epo_status.today_retry_time, epo_status.last_DL_Date);
    dl_retry_cnt = 0;
    do{
        // Check if a new day comes, reset flag to re-dl (have checked EPO expire before trigger dl)
        if (epo_status.last_DL_Date != tm.tm_mday) {
            memset(epo_status.EPO_piece_flag, 0x00, sizeof(unsigned int)*MAX_EPO_PIECE);
            epo_status.today_retry_time = 0;
            epo_status.last_DL_Date = tm.tm_mday;
            LOGD_ENG("First epo request today, day is %d", epo_status.last_DL_Date);
            for (i = 0; i < gps_epo_download_piece; i++) {
                memset(gps_epo_md5_file_name, 0x00, sizeof(gps_epo_md5_file_name));
                memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));
                gps_download_epo_file_name(i);
                strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_md5_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                strncat(gps_epo_md5_file_name, gps_epo_MD5_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                unlink(gps_epo_md5_file_name);
                unlink(gps_epo_data_file_name);
            }
        }

        check_epo_file_exist();

        for(gps_epo_file_count = 0; gps_epo_file_count < gps_epo_download_piece; ++gps_epo_file_count) {
            //LOGD("EPO begin download %d piece...", gps_epo_file_count);
            gps_download_epo_file_name(gps_epo_file_count);
            if (((epo_status.EPO_piece_flag[gps_epo_file_count] & EPO_MD5_AVAILABLE_BIT) == EPO_MD5_AVAILABLE_BIT) &&
                ((epo_status.EPO_piece_flag[gps_epo_file_count] & EPO_DAT_AVAILABLE_BIT) == EPO_DAT_AVAILABLE_BIT)) {
                LOGD_ENG("EPO MD5 DAT both valid, no need download %s", gps_epo_file_name);
                if (epo_DAT_file_merge(EPO_MERGE_FULL_FILE) == -1) {
                    LOGE("Merge piece EPO to DAT file error");
                    ret = EPO_DOWNLOAD_RESULT_FAIL;
                    break;
                }
                continue;
            } else if ((epo_status.EPO_piece_flag[gps_epo_file_count] & EPO_MD5_AVAILABLE_BIT) != EPO_MD5_AVAILABLE_BIT) {
                LOGD("EPO MD5 not valid, download %s ", gps_epo_MD5_file_name);
                ret_MD5 = epo_MD5_download_process();
                if (ret_MD5 == EPO_MD5_FILE_UPDATED) {
                    epo_status.EPO_piece_flag[gps_epo_file_count] |= EPO_MD5_AVAILABLE_BIT;
                    //LOGD("%s download success", gps_epo_file_name);
                }else {
                    LOGD_ENG("%s download failed", gps_epo_MD5_file_name);
                    if (gps_epo_file_count == 0) {
                        ret = EPO_DOWNLOAD_RESULT_FAIL;
                    }
                    break;
                }
            }

            ret = epo_DAT_download_process();
            if (ret == EPO_DOWNLOAD_RESULT_SUCCESS) {
                memset(gps_epo_md5_file_name, 0x00, sizeof(gps_epo_md5_file_name));
                memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));
                strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_md5_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                strncat(gps_epo_md5_file_name, gps_epo_MD5_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                if (mtk_gps_epo_md5_match_check(gps_epo_data_file_name, gps_epo_md5_file_name) == MTK_GPS_ERROR) {
                    LOGE("EPO DAT MD5 match check failed, confirm checksum");
                    if ((valid_file_length = epo_DAT_checksum_check()) > 0) {
                        if (epo_DAT_file_merge(valid_file_length) == -1) {
                            LOGE("Merge piece EPO to DAT file error");
                            ret = EPO_DOWNLOAD_RESULT_FAIL;
                            break;
                        } else {
                            ret = EPO_DOWNLOAD_RESULT_SUCCESS;
                            LOGD("have more than 2days EPO(%d piece + valid file size:%d bytes), do inject", gps_epo_file_count, valid_file_length);
                            break;
                        }
                    } else {
                        epo_status.EPO_piece_flag[gps_epo_file_count] &= (~EPO_DAT_AVAILABLE_BIT);
                        epo_status.EPO_piece_flag[gps_epo_file_count] &= (~EPO_MD5_AVAILABLE_BIT);
                        epo_status.today_retry_time++;
                        unlink(gps_epo_md5_file_name);
                        unlink(gps_epo_data_file_name);
                        ret = EPO_DOWNLOAD_RESULT_FAIL;
                        break;
                    }
                } else {
                    LOGD("EPO MD5 check success: %d", gps_epo_file_count);
                    epo_status.EPO_piece_flag[gps_epo_file_count] |= EPO_DAT_AVAILABLE_BIT;
                    if (epo_DAT_file_merge(EPO_MERGE_FULL_FILE) == -1) {   // 0 means merge all
                        LOGE("Merge piece EPO to DAT file error");
                        ret = EPO_DOWNLOAD_RESULT_FAIL;
                        break;
                    }
                }
            }else {
                LOGE("DAT file download failed");
                ret = EPO_DOWNLOAD_RESULT_FAIL;
                break;
            }
        }

        if (ret == EPO_DOWNLOAD_RESULT_SUCCESS) {
            //LOGD("Merge EPOHAL.DAT success, delete piece files");
            for (i = 0; i < gps_epo_download_piece; i++) {
                memset(gps_epo_md5_file_name, 0x00, sizeof(gps_epo_md5_file_name));
                memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));
                gps_download_epo_file_name(i);
                strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
                strncat(gps_epo_md5_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                strncat(gps_epo_md5_file_name, gps_epo_MD5_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
                unlink(gps_epo_md5_file_name);
                unlink(gps_epo_data_file_name);
            }
            break;
        } else {  //Download fail
            if (gps_epo_file_count == 0)  //Only pice 1 need to retry
            {
                dl_retry_cnt ++;
                if (dl_retry_cnt <= EPO_INVALIDE_DL_MAX_RETRY_TIME)
                {
                    usleep(EPO_INVALIDE_DL_RETRY_SLEEP);
                }
            } else {
                dl_retry_cnt = EPO_INVALIDE_DL_MAX_RETRY_TIME+1; //No need retry
            }
            LOGD_ENG("retry cnt:%d", dl_retry_cnt);
        }
    }while (dl_retry_cnt <= EPO_INVALIDE_DL_MAX_RETRY_TIME);

    return ret;
}

/*****************************************************************************/
static int get_val(char *pStr, char** ppKey, char** ppVal) {
    int len = (int)strlen(pStr);
    char *end = pStr + len;
    char *key = NULL, *val = NULL;

    LOGD("pStr = %s, len=%d!!\n", pStr, len);

    if (!len) {
        return -1;       // no data
    } else if (pStr[0] == '#') {   /*ignore comment*/
        *ppKey = *ppVal = NULL;
        return 0;
    } else if (pStr[len-1] != '\n') {
        if (len >= GPS_CONF_FILE_SIZE-1) {
            LOGD("buffer is not enough!!\n");
            return -1;
        } else {
            pStr[len] = '\n';
        }
    }
    key = pStr;

    LOGD("key = %s!!\n", key);
    while ((*pStr != '=') && (pStr < end)) pStr++;
    if (pStr >= end) {
        LOGD("'=' is not found!!\n");
        return -1;       // format error
    }

    *pStr++ = '\0';
    while (IS_SPACE(*pStr) && (pStr < end)) pStr++;       // skip space chars
    val = pStr;
    while (!IS_SPACE(*pStr) && (pStr < end)) pStr++;
    *pStr = '\0';
    *ppKey = key;
    *ppVal = val;

    LOGD("val = %s!!\n", val);
    return 0;
}

/*****************************************************************************/
int epo_read_cust_config(void) {
    char result[GPS_CONF_FILE_SIZE] = {0};

    FILE *fp = fopen(EPO_CONTROL_FILE_PATH, "r");
    char *key, *val;
    if (!fp) {
           // LOGD("%s: open %s fail!\n", __FUNCTION__, EPO_CONTROL_FILE_PATH);
        return 1;
    }

    while (fgets(result, sizeof(result), fp)) {
        if (get_val(result, &key, &val)) {
            LOGD("%s: Get data fails!!\n", __FUNCTION__);
            fclose(fp);
            return 1;
        }
        if (!key || !val)
            continue;
        if (!strcmp(key, "EPO_ENABLE")) {
            int len = strlen(val);

            LOGD("gps_epo_enablebg = %d, len =%d\n", gps_epo_enable, len);
            gps_epo_enable = str2int(val, val+len);   // *val-'0';
            if ((gps_epo_enable != 1) && (gps_epo_enable != 0)) {
                gps_epo_enable = 1;
            }
            LOGD("gps_epo_enableend = %d\n", gps_epo_enable);
        }
        if (!strcmp(key, "DW_DAYS")) {
            int len = strlen(val);
            gps_epo_download_days = str2int(val, val+len);         // *val-'0';
            if (gps_epo_download_days > MTK_EPO_MAX_DAY || gps_epo_download_days < 0) {
                gps_epo_download_days = MTK_EPO_DEFAULT_DL_DAY;
            }
        }
        if (!strcmp(key, "EPO_WIFI_TRIGGER")) {
            int len = strlen(val);
            LOGD("gps_epo_wifi_triggerbg = %d, len =%d\n", gps_epo_wifi_trigger, len);
            gps_epo_wifi_trigger = str2int(val, val+len);   // *val-'0';
            if ((gps_epo_wifi_trigger != 1) && (gps_epo_wifi_trigger != 0)) {
                gps_epo_wifi_trigger = 0;
            }
            LOGD("gps_epo_wifi_triggerend = %d\n", gps_epo_wifi_trigger);
        }
        LOGD("gps_epo_enable = %d, gps_epo_period = %d, \
            wifi_epo_period = %d, gps_epo_wifi_trigger = %d\n", gps_epo_enable, gps_epo_period,
            wifi_epo_period, gps_epo_wifi_trigger);
    }

    fclose(fp);
    return 1;
}

/*****************************************************************************/
static void gps_download_epo_file_name(int count) {
    //  LOGD("count is %d\n", count);
    if (gps_epo_type == 1) {
        if (count == 0) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_1.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_1.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 1) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_2.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_2.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 2) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_3.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_3.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 3) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_4.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_4.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 4) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_5.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_5.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 5) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_6.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_6.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 6) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_7.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_7.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 7) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_8.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_8.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 8) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_9.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_9.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 9) {
            strncpy(gps_epo_file_name, "EPO_GPS_3_10.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GPS_3_10.MD5", GPS_EPO_FILE_LEN);
        }
    }
    else if (gps_epo_type == 0) {
        if (count == 0) {
            strncpy(gps_epo_file_name, "EPO_GR_3_1.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_1.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 1) {
            strncpy(gps_epo_file_name, "EPO_GR_3_2.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_2.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 2) {
            strncpy(gps_epo_file_name, "EPO_GR_3_3.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_3.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 3) {
            strncpy(gps_epo_file_name, "EPO_GR_3_4.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_4.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 4) {
            strncpy(gps_epo_file_name, "EPO_GR_3_5.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_5.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 5) {
            strncpy(gps_epo_file_name, "EPO_GR_3_6.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_6.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 6) {
            strncpy(gps_epo_file_name, "EPO_GR_3_7.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_7.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 7) {
            strncpy(gps_epo_file_name, "EPO_GR_3_8.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_8.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 8) {
            strncpy(gps_epo_file_name, "EPO_GR_3_9.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_9.MD5", GPS_EPO_FILE_LEN);
        } else if (count == 9) {
            strncpy(gps_epo_file_name, "EPO_GR_3_10.DAT", GPS_EPO_FILE_LEN);
            strncpy(gps_epo_MD5_file_name, "EPO_GR_3_10.MD5", GPS_EPO_FILE_LEN);
        }
    }
    //LOGD("download request for file %d, gps_epo_file_name=%s, gps_epo_MD5_file_name=%s\n",
    //    count, gps_epo_file_name, gps_epo_MD5_file_name);
}

/*****************************************************************************/
int mtk_gps_sys_read_lock(int fd, off_t offset, int whence, off_t len) {
    struct flock lock;

    lock.l_type = F_RDLCK;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    if (fcntl(fd, F_SETLK, &lock) < 0) {
        return -1;
    }

    return 0;
}

/*****************************************************************************/
static unsigned int mtk_gps_sys_get_file_size() {
    unsigned int fileSize;
    int res_epo, res_epo_hal;
    struct stat st;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_UPDATE_HAL;
    char epofile[GPS_EPO_FILE_LEN] = {0};
    res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_UPDATE_HAL, F_OK);
    if (res_epo < 0 && res_epo_hal < 0) {
        LOGD("no EPO data yet\n");
        return -1;
    }
    if (res_epo_hal == 0) {  /*EPOHAL.DAT is here*/
        // LOGD("find EPOHAL.DAT here\n");
        MNLD_STRNCPY(epofile, epo_file_hal, GPS_EPO_FILE_LEN);
    } else if (res_epo == 0) {  /*EPO.DAT is here*/
        // LOGD("find EPO.DAT here\n");
        MNLD_STRNCPY(epofile, epo_file, GPS_EPO_FILE_LEN);
    } else
        LOGE("unknown error happened\n");

    if (stat(epofile, &st) < 0) {
        LOGE("Get file size error, return\n");
        return 0;
    }

    fileSize = st.st_size;
       // LOGD("EPO file size: %d\n", fileSize);
    return fileSize;
}

/*****************************************************************************/
void GpsToUtcTime(int i2Wn, double dfTow, time_t* uSecond) {
    struct tm target_time;
    int iYearsElapsed;        //  Years since 1980.
    unsigned int iDaysElapsed;         //  Days elapsed since Jan 1, 1980.
    double dfSecElapsed;
    unsigned int fgLeapYear;
    int pi2Yr = 0;
    int pi2Mo = 0;
    int pi2Day = 0;
    int pi2Hr = 0;
    int pi2Min = 0;
    double pdfSec = 0;
    int i = 0;


    //  Number of days into the year at the start of each month (ignoring leap
    //  years).
    unsigned int doy[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    //  Convert time to GPS weeks and seconds
    iDaysElapsed = i2Wn * 7 + ((int)dfTow / 86400) + 5;
    dfSecElapsed = dfTow - ((int)dfTow / 86400) * 86400;


    //  decide year
    iYearsElapsed = 0;       //  from 1980
    while (iDaysElapsed >= 365) {
        if ((iYearsElapsed % 100) == 20) {   //  if year % 100 == 0
            if ((iYearsElapsed % 400) == 20) {   //  if year % 400 == 0
                if (iDaysElapsed >= 366) {
                    iDaysElapsed -= 366;
                } else {
                    break;
                }
            } else {
                iDaysElapsed -= 365;
            }
        } else if ((iYearsElapsed % 4) == 0) {   //  if year % 4 == 0
            if (iDaysElapsed >= 366) {
                iDaysElapsed -= 366;
            } else {
                break;
            }
        } else {
            iDaysElapsed -= 365;
        }
        iYearsElapsed++;
    }
    pi2Yr = 1980 + iYearsElapsed;


    // decide month, day
    fgLeapYear = 0;
    if ((iYearsElapsed % 100) == 20) {    // if year % 100 == 0
        if ((iYearsElapsed % 400) == 20) {    // if year % 400 == 0
           fgLeapYear = 1;
        }
    }
    else if ((iYearsElapsed % 4) == 0) {   // if year % 4 == 0
        fgLeapYear = 1;
    }

    if (fgLeapYear) {
        for (i = 2; i < 12; i++) {
            doy[i] += 1;
        }
    }
    for (i = 0; i < 12; i++) {
        if (iDaysElapsed < doy[i]) {
            break;
        }
    }
    pi2Mo = i;
    if (i > 0) {
        pi2Day = iDaysElapsed - doy[i-1] + 1;
    }

    // decide hour, min, sec
    pi2Hr = dfSecElapsed / 3600;
    pi2Min = ((int)dfSecElapsed % 3600) / 60;
    pdfSec = dfSecElapsed - ((int)dfSecElapsed / 60) * 60;

    // change the UTC time to seconds
    memset(&target_time, 0, sizeof(target_time));
    target_time.tm_year = pi2Yr - 1900;
    target_time.tm_mon = pi2Mo - 1;
    target_time.tm_mday = pi2Day;
    target_time.tm_hour = pi2Hr;
    target_time.tm_min = pi2Min;
    target_time.tm_sec = pdfSec;
    target_time.tm_isdst = -1;
    *uSecond = mktime(&target_time);
    if (*uSecond < 0) {
        LOGE("Convert UTC time to seconds fail, return\n");
    }
}


/*****************************************************************************/
int mtk_gps_sys_epo_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    // if (fread(szBuf, 1, MTK_EPO_ONE_SV_SIZE, pFile) != MTK_EPO_ONE_SV_SIZE) {
    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    // LOGD("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

/*****************************************************************************/
int mtk_gps_sys_epo_bd_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 == lseek(fd, MTK_EPO_ONE_SV_SIZE, SEEK_SET)) { //Skip the header of BD QEPO file
        LOGE("lseek error(%s)\n",strerror(errno));
        return -1;
    }

    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

/*****************************************************************************/
int mtk_gps_sys_epo_ga_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 == lseek(fd, MTK_EPO_ONE_SV_SIZE, SEEK_SET)) { //Skip the header of GA QEPO file
        LOGE("lseek error(%s)\n",strerror(errno));
        return -1;
    }

    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

/*****************************************************************************/
static int mtk_gps_sys_epo_period_end(int fd, unsigned int *u4GpsSecs, time_t* uSecond) {           // no file lock
    int fileSize;
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    fileSize = mtk_gps_sys_get_file_size();
    if (fileSize < MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    if (-1 == lseek(fd, (fileSize - MTK_EPO_ONE_SV_SIZE), SEEK_SET)) {
        LOGE("lseek error\n");
        return -1;
    }

    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    (*u4GpsSecs) += 21600;

    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    // LOGD("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);

    return 0;
}

/*****************************************************************************/
static int mtk_gps_epo_file_time_hal(time_t uTime[]) {
    //LOGD("mtk_gps_epo_file_time_hal");
    struct stat filestat;
    int fd = 0;
    int res_epo, res_epo_hal;
    unsigned int u4GpsSecs_start;    // GPS seconds
    unsigned int u4GpsSecs_expire;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_UPDATE_HAL;
    char epofile[GPS_EPO_FILE_LEN] = {0};
    time_t uSecond_start;      // UTC seconds
    time_t uSecond_expire;
// int ret = 0;
    // pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

    res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_UPDATE_HAL, F_OK);
    if (res_epo < 0 && res_epo_hal < 0) {
        LOGD("no EPO data yet\n");
//        ret = pthread_mutex_unlock(&mutx);
        return -1;
    }
    if (res_epo_hal== 0) {  /*EPOHAL.DAT is here*/
        // LOGD("find EPOHAL.DAT here\n");
        MNLD_STRNCPY(epofile, epo_file_hal, GPS_EPO_FILE_LEN);
    } else if (res_epo == 0) {  /*EPO.DAT is here*/
           // LOGD("find EPO.DAT here\n");
        MNLD_STRNCPY(epofile, epo_file, GPS_EPO_FILE_LEN);
    } else
        LOGE("unknown error happened\n");

    // open file
    fd = open(epofile, O_RDONLY);
    if (fd < 0) {
        LOGE("Open EPO fail, return\n");
//        ret = pthread_mutex_unlock(&mutx);
        return -1;
    }

    // Add file lock
    if (mtk_gps_sys_read_lock(fd, 0, SEEK_SET, 0) < 0) {
        LOGE("Add read lock failed, return\n");
        close(fd);
//      ret = pthread_mutex_unlock(&mutx);
        return -1;
    }

    // EPO start time
    if (mtk_gps_sys_epo_period_start(fd, &u4GpsSecs_start, &uSecond_start)) {
        LOGE("Get EPO file start time error, return\n");
        close(fd);
//        ret = pthread_mutex_unlock(&mutx);
        return -1;
    } else {
        uTime[0] = uSecond_start;
        //  LOGD("The Start time of EPO file is %lld", uTime[0]);
        //  LOGD("The start time of EPO file is %s", ctime(&uTime[0]));
    }

    // download time
    if (stat(epofile, &filestat) == -1){
        LOGE("stat get file information failed reason=[%s]\n", strerror(errno));
    }

    uTime[1] = filestat.st_mtime;
    // uTime[1] = uTime[1] - 8 * 3600;
    // LOGD("Download time of EPO file is %lld", uTime[1]);
    // LOGD("Download time of EPO file is %s\n", ctime(&uTime[1]));

    // EPO file expire time
    if (mtk_gps_sys_epo_period_end(fd, &u4GpsSecs_expire, &uSecond_expire)) {
        LOGE("Get EPO file expire time error, return\n");
        close(fd);
//        ret = pthread_mutex_unlock(&mutx);
        return -1;
    } else {
        uTime[2] = uSecond_expire;
        // LOGD("The expire time of EPO file is %lld", uTime[2]);
        //  LOGD("The expire time of EPO file is %s", ctime(&uTime[2]));
    }

    close(fd);
//    ret = pthread_mutex_unlock(&mutx);
    return 0;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
CURLcode curl_easy_download(char* url, char* filename) {
    CURL *curl = NULL;
    FILE *fp = NULL;
    CURLcode res;

    //LOGD("curl_easy_download url: %s to %s", url, filename);
    if ((res = curl_global_init(CURL_GLOBAL_DEFAULT)) != 0) {
        LOGE("curl_global_init fail, res = %d, curl_easy_download url: %s to %s", res, url, filename);
    }
    curl = curl_easy_init();
    //LOGD("curl_easy_init done");
    if (curl) {
        fp = fopen(filename, "w+");
        if (fp == NULL) {
            curl_easy_cleanup(curl);
            return CURLE_FAILED_INIT;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
        //   curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        return res;
    } else {
        LOGE("curl_easy_init fail, curl_easy_download url: %s to %s", url, filename);
        return CURLE_FAILED_INIT;
    }
}
static int counter = 1;
void getEpoUrl(char* filename, char* url) {
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
    //LOGD("url = %s\n", url);
}

CURLcode curl_easy_download_epo_MD5(void) {
    int res_val;
    CURLcode res;
    char url[GPS_EPO_URL_LEN]={0};
    char gps_epo_md5_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    int filesize = 0;

    //LOGD("curl_easy_download_epo_MD5:%s", gps_epo_MD5_file_name);
    memset(gps_epo_md5_file_name, 0x00, sizeof(gps_epo_md5_file_name));
    getEpoUrl(gps_epo_MD5_file_name, url);

    strncat(gps_epo_md5_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
    strncat(gps_epo_md5_file_name, gps_epo_MD5_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_md5_file_name) - 1);
    res = curl_easy_download(url, gps_epo_md5_file_name);

    filesize = get_file_size(gps_epo_md5_file_name);

    if (filesize <= 0 || filesize > EPO_MD5_FILE_MAX_SIZE) {
        res = CURLE_READ_ERROR;
        LOGD_ENG("download file size error.");
    }

    //LOGD("epo MD5 file curl_easy_download res = %d\n", res);
    if (res == CURLE_OK) {
        counter = 1;
        res_val = chmod(gps_epo_md5_file_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (res_val < 0) {
            LOGD("chmod MD5 res_val = %d, %s\n", res_val, strerror(errno));
        }
    } else {
        unlink(gps_epo_md5_file_name);
        counter++;
        LOGE("epo MD5 file download failed res:%d. curl_easy_download url: %s to %s", res, url, gps_epo_md5_file_name);
    }
    return res;
}

CURLcode curl_easy_download_epo_DAT(void) {
    int res_val;
    CURLcode res;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    char url[GPS_EPO_URL_LEN]={0};
    int filesize = 0;

    //LOGD("curl_easy_download_epo_DAT:%s", gps_epo_file_name);
    memset(gps_epo_data_file_name, 0x00, sizeof(gps_epo_data_file_name));
    getEpoUrl(gps_epo_file_name, url);

    strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    res = curl_easy_download(url, gps_epo_data_file_name);

    filesize = get_file_size(gps_epo_data_file_name);

    if (filesize <= 0) {
        res = CURLE_PARTIAL_FILE;
        LOGE("download file size error.");
    }

    if (res == CURLE_OK) {
        counter = 1;
        res_val = chmod(gps_epo_data_file_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (res_val < 0) {
            LOGD("chmod DAT res_val = %d, %s\n", res_val, strerror(errno));
        }
    } else {
        unlink(gps_epo_data_file_name);
        counter++;
        LOGD("epo DAT file curl_easy_download res = %d\n", res);
    }
    return res;
}

CURLcode curl_easy_download_epo(void) {
    int res_val;
    CURLcode res;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};
    char url[GPS_EPO_URL_LEN]={0};

    LOGD("curl_easy_download_epo");
    getEpoUrl(gps_epo_file_name, url);

    strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);

    res = curl_easy_download(url, gps_epo_data_file_name);
    LOGD("epo file curl_easy_download res = %d\n", res);
    if (res == CURLE_OK) {
        FILE *fp_temp = NULL;
        FILE *fp = NULL;

        counter = 1;
        if (gps_epo_file_count == 0) {
            unlink(EPO_UPDATE_HAL);
        }
        res_val = chmod(gps_epo_data_file_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
        fp_temp = fopen(EPO_UPDATE_HAL, "at");
        if (fp_temp != NULL) {
            fp = fopen(gps_epo_data_file_name, "r");
            if (fp != NULL) {
            #define buf_size  256
                char data[buf_size] = {0};
                int bytes_in = 0, bytes_out = 0;
                int len = 0;

                while ((bytes_in = fread(data, 1, sizeof(data), fp)) > 0
                        && (bytes_in <= (int)(buf_size* sizeof(char)))) {
                    bytes_out = fwrite(data, 1, bytes_in, fp_temp);
                    if (bytes_in != bytes_out) {
                        LOGD("bytes_in = %d,bytes_out = %d\n", bytes_in, bytes_out);
                    }
                    len += bytes_out;
                    // LOGD("copying file...%d bytes copied\n",len);
                }
                fclose(fp);
            } else {
                LOGE("Open merged file fp=NULL\n");
            }
            fclose(fp_temp);
        }
        else {
            LOGE("Open merged file failed\n");
        }
        res_val = chmod(EPO_UPDATE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        LOGD("chmod res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        unlink(gps_epo_data_file_name);
        counter++;
    }
    return res;
}

#if 0
static unsigned int mtk_gps_epo_get_piece_file_size() {
    struct stat st;
    unsigned int fileSize;
    char gps_epo_data_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};

    strncat(gps_epo_data_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);
    strncat(gps_epo_data_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name) - 1);

    if (stat(gps_epo_data_file_name, &st) < 0) {
        LOGE("Get file size error, return\n");
        return 0;
    }
    fileSize = st.st_size;
    LOGD("EPO piece file size: %d\n", fileSize);
    return fileSize;
}
/*****************************************************************************/
static int mtk_gps_epo_piece_data_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;


    // if (fread(szBuf, 1, MTK_EPO_ONE_SV_SIZE, pFile) != MTK_EPO_ONE_SV_SIZE) {
    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    LOGD("mtk_gps_epo_piece_data_start");
    LOGD("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}
/*****************************************************************************/
static int mtk_gps_epo_piece_data_end(int fd, unsigned int *u4GpsSecs, time_t* uSecond) {
    int fileSize = 0;
    char szBuf[MTK_EPO_ONE_SV_SIZE] = {0};
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 != fd) {
        fileSize = mtk_gps_epo_get_piece_file_size();
        if (fileSize < MTK_EPO_ONE_SV_SIZE) {
            LOGE("Get file size is error\n");
            return -1;
        }
        if (-1 == lseek(fd, (fileSize - MTK_EPO_ONE_SV_SIZE), SEEK_SET)) {
            LOGE("lseek error\n");
            return -1;
        }

        if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
            LOGE("read epo file end data faied\n");
            return -1;
        }

        *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
        (*u4GpsSecs) += 21600;

        pi2WeekNo = (*u4GpsSecs) / 604800;
        pu4Tow = (*u4GpsSecs) % 604800;

        LOGD("mtk_gps_epo_piece_data_end");
        LOGD("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
        GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);
    }
    return 0;
}

/*****************************************************************************/
static int mtk_gps_epo_server_data_is_changed() {
    time_t uTime_end = 0;
    time_t uTime_start = 0;
    int fd_end = -1;
    int fd_start = -1;
    char gps_epo_data_file_name_end[EPO_FILE_NAME_MAX_SIZE] = {0};
    char gps_epo_data_file_name_start[EPO_FILE_NAME_MAX_SIZE] = {0};
    time_t uSecond_start;
    time_t uSecond_end;
    unsigned int u4GpsSecs_start;
    unsigned int u4GpsSecs_end;
    int ret = 0;

    strncat(gps_epo_data_file_name_start, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name_start) - 1);
    strncat(gps_epo_data_file_name_start, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name_start) - 1);

    fd_start = open(gps_epo_data_file_name_start, O_RDONLY);
    if (fd_start >= 0) {
        int res = 0;
        res = mtk_gps_epo_piece_data_start(fd_start, &u4GpsSecs_start, &uSecond_start);
        if (res == 0) {
            uTime_start = uSecond_start;
        } else {
            epo_download_failed = 1;
            ret = 1;
            LOGE("Get start time failed\n");
        }
        close(fd_start);
    } else {
        LOGE("Open start file failed\n");
    }
    if (gps_epo_file_count > 0) {
        gps_download_epo_file_name(gps_epo_file_count - 1);
        strncat(gps_epo_data_file_name_end, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name_end) - 1);
        strncat(gps_epo_data_file_name_end, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_data_file_name_end) - 1);

        // open file
        fd_end = open(gps_epo_data_file_name_end, O_RDONLY);
        if (fd_end >= 0) {
            int res = 0;
            res = mtk_gps_epo_piece_data_end(fd_end, &u4GpsSecs_end, &uSecond_end);
            if (res == 0) {
                uTime_end = uSecond_end;
            } else {
                epo_download_failed = 1;
                LOGE("Get end time failed\n");
                ret = 1;
            }
            close(fd_end);
        } else {
            LOGE("Open end file failed\n");
        }
    } else if (gps_epo_file_count == 0) {
        uTime_end = uTime_start;
    }

    // LOGD("gps_epo_data_file_start =%s, end =%s\n", gps_epo_data_file_name_start, gps_epo_data_file_name_end);
    LOGD("The end time of EPO file is %s, The start time of EPO file is %s\n",
        ctime(&uTime_end), ctime(&uTime_start));
    if (uTime_start >= ((24*60*60) + uTime_end)) {
        int i;
        LOGD("The epo data is updated on the server!!!\n");
        for (i = gps_epo_file_count; 0 <= i; i--) {
            char gps_epo_piece_file_name[EPO_FILE_NAME_MAX_SIZE] = {0};

            gps_download_epo_file_name(i);
            strncat(gps_epo_piece_file_name, EPO_PATH, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_piece_file_name) - 1);
            strncat(gps_epo_piece_file_name, gps_epo_file_name, EPO_FILE_NAME_MAX_SIZE - strlen(gps_epo_piece_file_name) - 1);
            unlink(gps_epo_piece_file_name);
        }
        unlink(EPO_UPDATE_HAL);
        gps_epo_file_count = 0;
        return 1;
    }
    return ret;
}
#endif
static int mtk_epo_is_expired(int wifi_tragger) {
    time_t uTime[3];  // [0] epo start time, [1] download time, [2] expired time
    memset(uTime, 0, sizeof(uTime));
    time_t         now = time(NULL);
    struct tm      tm_utc;
    time_t  time_utc;
    long expired_set = 0;
    int download_day = 0;

    gmtime_r(&now, &tm_utc);
    time_utc = mktime(&tm_utc);
    mtk_gps_epo_file_time_hal(uTime);

    if (wifi_tragger) {
        expired_set = wifi_epo_period*24*60*60;    // for wifi tragger we change checking expired time to 1 day.
    } else {
        download_day = (uTime[2] - uTime[0])/(24*60*60);
           // LOGD("epo data downloaded dat: %d\n", download_day);
        if (download_day < 3) {
            expired_set = 0;
        } else if (download_day < 6) {
            expired_set = 3*24*60*60;
        } else if ((6 <= download_day) && (download_day < 9)) {
            expired_set = 5*24*60*60;
        } else if ((9 <= download_day) && (download_day < 12)) {
            expired_set = 7*24*60*60;
        } else if ((12 <= download_day) && (download_day < 15)) {
            expired_set = 7*24*60*60;
        } else if ((15 <= download_day) && (download_day < 18)) {
            expired_set = 7*24*60*60;
        } else if (download_day >= 18) {
            expired_set = 7*24*60*60;
        }
    }

    LOGD("current time: %ld, current time:%s", time_utc, ctime(&time_utc));
    LOGD("EPO start time: %ld, EPO start time: %s", uTime[0], ctime(&uTime[0]));
      //  LOGD("EPO expired_set: %lld", expired_set);
    if ((time_utc - uTime[0]) >= expired_set) {
        LOGD("EPO file is expired");
        gps_epo_file_count = 0;
        return 1;
    } else if ((time_utc - uTime[0]) < 0) {
        LOGD("Current time is invalid");
        gps_epo_file_count = 0;
        return 1;
    } else {
        LOGD("EPO file is valid, no need update");
        return 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// MAIN -> EPO Download (handlers)
static int mnld_epo_download() {
    //LOGD("begin\n");
    int ret = EPO_DOWNLOAD_RESULT_FAIL;

    ret = epo_file_download_impl();

    LOGD_ENG("download epo file completed!file count=%d, epo_download_result=%d\n",
        gps_epo_file_count, ret);

    mnld_epo_download_done(ret);

    //LOGD("end\n");
    return ret;
}
static int epo_event_hdlr(int fd) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    main2epo_event cmd;
    int read_len;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("epo_event_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    cmd = get_int(buff, &offset, sizeof(buff));
    switch (cmd) {
    case MAIN2EPO_EVENT_START: {
        LOGD_ENG("mnld_epo_download() before");
        // need to call mnld_epo_download_done() when EPO download is done
        mnld_epo_download();
        LOGW("mnld_epo_download() after");
        break;
    }
    default: {
        LOGE("epo_event_hdlr() unknown cmd=%d", cmd);
        return -1;
    }
    }
    return 0;
}

#if 0
static void epo_downloader_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("epo_downloader_thread_timeout() dump and exit.");
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("epo_downloader_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}
#endif

static void retry_alarm_timeout_handler() {
    epo_download_retry = 1;
    LOGD("epo_download_retry is =%d\n", epo_download_retry);
}

static void* epo_downloader_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    //hdlr_timer = init_timer(epo_downloader_thread_timeout);
    retry_download_timer = init_timer(retry_alarm_timeout_handler);
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("epo_downloader_thread() epoll_create failure reason=[%s]%d\n",
            strerror(errno), errno);
        return 0;
    }

    if (epoll_add_fd(epfd, g_fd_epo) == -1) {
        LOGE("epo_downloader_thread() epoll_add_fd() failed for g_fd_epo failed");
        return 0;
    }
    while (1) {
        int i;
        int n;
        LOGD_ENG("epo_downloader_thread wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("epo_downloader_thread() epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        //start_timer(hdlr_timer, MNLD_EPO_HANDLER_TIMEOUT);
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_epo) {
                if (events[i].events & EPOLLIN) {
                    epo_event_hdlr(g_fd_epo);
                }
            } else {
                LOGE("epo_downloader_thread() unknown fd=%d",
                    events[i].data.fd);
            }
        }
        //stop_timer(hdlr_timer);
    }

    LOGE("epo_downloader_thread() exit");
    return 0;
}

int epo_downloader_is_file_invalid() {
    return mtk_epo_is_expired(0);
}

int epo_is_wifi_trigger_enabled() {
    return gps_epo_wifi_trigger;
}

int epo_is_epo_download_enabled() {
    return gps_epo_enable;
}

int epo_downloader_start() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, MAIN2EPO_EVENT_START);
    return safe_sendto(MNLD_EPO_DOWNLOAD_SOCKET, buff, offset);
}

int epo_downloader_init() {
    pthread_t pthread_epo;
    g_fd_epo = socket_bind_udp(MNLD_EPO_DOWNLOAD_SOCKET);
    if (g_fd_epo < 0) {
        LOGE("socket_bind_udp(MNLD_EPO_DOWNLOAD_SOCKET) failed");
        return -1;
    }

    pthread_create(&pthread_epo, NULL, epo_downloader_thread, NULL);
    return 0;
}

