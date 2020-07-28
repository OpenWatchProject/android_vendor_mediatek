#include <errno.h>   /* Error number definitions */
#include <fcntl.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <sys/ioctl.h>
#include <time.h>
#include <cutils/properties.h>
#include "mtk_gps_sys_fp.h"
#include "nmea_parser.h"
#include "mtk_lbs_utility.h"
#include "mtk_gps_agps.h"
#include "gps_dbg_log.h"
#include "mnld_utile.h"

// for read NVRAM
#if MTK_GPS_NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#endif
#include "SUPL_encryption.h"
#include <cutils/android_filesystem_config.h>

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
#define LOGD(...) tag_log(1, "[pseudo]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[pseudo] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[pseudo] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pseudo"
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
// #define LIB_MQUEUE

#define MAX_RETRY_COUNT       20
#define PMTK_FS_REQ_MEAS                736
#define PMTK_FRAME_TIME_ACK             737
#define PMTK_FS_SLEEPMODE               738
#define MTK_GPS_HIDE_PRINT_MAX 17

#if defined(LIB_MQUEU)
// message queue file descriptor
static mqd_t mnl_agps_mq_fd = -1;
struct mq_attr mnl_agps_mq_attr;
#endif

MTK_GPS_BOOL enable_dbg_log = MTK_GPS_TRUE;

// for read NVRAM
#if MTK_GPS_NVRAM
extern ap_nvram_gps_config_struct stGPSReadback;
extern int gps_nvram_valid;
#if ANDROID_MNLD_PROP_SUPPORT
char nvram_init_val[PROPERTY_VALUE_MAX];
#else
char nvram_init_val[100];
#endif
#endif
int mtk_gps_sys_init() {
#if MTK_GPS_NVRAM
    F_ID gps_nvram_fd;
    int file_lid = AP_CFG_CUSTOM_FILE_GPS_LID;
#endif
    int rec_size;
    int rec_num;
    int read_nvram_ready_retry = 0;
    int ret;
    /* create message queue */
#if defined(LIB_MQUEUE)
    mnl_agps_mq_attr.mq_maxmsg = 72;
    mnl_agps_mq_attr.mq_msgsize = sizeof(MTK_GPS_AGPS_AGENT_MSG);
    mnl_agps_mq_attr.mq_flags   = 0;
    mnl_agps_mq_fd = mq_open(MNL_AGPS_MQ_NAME, O_CREAT|O_RDWR|O_EXCL, PMODE, &mnl_agps_mq_attr);

    if (mnl_agps_mq_fd == -1) {
        LOGD("Fail to create mnl_agps_msg_queue, errno=%s\n", strerror(errno));
        if (errno == EEXIST) {
            LOGD("mnl_agps_msg_queue already exists, unlink it now ...\n");
            mq_unlink(MNL_AGPS_MQ_NAME);
        }
        return MTK_GPS_ERROR;
    }
#else
#endif
#if MTK_GPS_NVRAM
    //LOGD("Start to read nvram");
    while (read_nvram_ready_retry < MAX_RETRY_COUNT) {
        read_nvram_ready_retry++;
        #if ANDROID_MNLD_PROP_SUPPORT
        property_get("vendor.service.nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0 || strcmp(nvram_init_val, "Pre_Ready") == 0) {
            LOGD("nvram_init_val Ready");
            break;
        } else {
            LOGD("nvram_init_val not Ready,sleep 500ms");
            usleep(500*1000);
        }
        #endif
    }
    //LOGD("Get nvram restore ready retry cc=%d\n", read_nvram_ready_retry);
    if (read_nvram_ready_retry >= MAX_RETRY_COUNT) {
        LOGD("Get nvram restore ready faild, return\n");
        return MTK_GPS_ERROR;
    }

    memset(&stGPSReadback, 0, sizeof(stGPSReadback));

    gps_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
    if (gps_nvram_fd.iFileDesc >= 0) {
        ret = read(gps_nvram_fd.iFileDesc, &stGPSReadback , rec_size*rec_num);
        if (ret < 0){
            LOGW("Read GPS NVRam Fail");
        }
        NVM_CloseFileDesc(gps_nvram_fd);
        stGPSReadback.dsp_dev[sizeof(stGPSReadback.dsp_dev) - 1] = '\0';

        if (strlen(stGPSReadback.dsp_dev) != 0) {
            gps_nvram_valid = 1;
            // strncpy(mnl_config.dev_dsp, stGPSReadback.dsp_dev, sizeof(mnl_config.dev_dsp));

            LOGD_ENG("GPS NVRam :(%d * %d), dsp_dev(/dev/stpgps) : %s\n", rec_size, rec_num, stGPSReadback.dsp_dev);
            LOGD_ENG("gps_tcxo_hz : %d,gps_tcxo_ppb : %d,gps_tcxo_type : %d, gps_lna_mode : %d\n",
                stGPSReadback.gps_tcxo_hz,
                stGPSReadback.gps_tcxo_ppb,
                stGPSReadback.gps_tcxo_type,
                stGPSReadback.gps_lna_mode);
        } else {
            LOGD("GPS NVRam mnl_config.dev_dsp == NULL \n");
        }
    }
    else {
           LOGD("GPS NVRam gps_nvram_fd == NULL \n");
    }
#endif
    return MTK_GPS_SUCCESS;
}
int mtk_gps_sys_uninit() {
#if defined(LIB_MQUEUE)
    mq_close(mnl_mq_fd);         /* Close message queue in parent */
    mq_unlink(MNL_MQ_NAME);      /* Unlink message queue */
    mq_close(mnl_agps_mq_fd);    /* Close message queue in parent */
    mq_unlink(MNL_AGPS_MQ_NAME);  /* Unlink message queue */
#else
#endif
    return MTK_GPS_SUCCESS;
}

int mtk_gps_sys_strncmp(const char * str1,const char * str2){
    return strncmp(str1,str2,strlen(str2));
}

//Log filter for NMEA sentence
MTK_GPS_BOOL mtk_gps_sys_nmea_log_filter(const char * buffer) {
    MTK_GPS_BOOL ret = MTK_GPS_FALSE;

    if('$' == buffer[0]
       && ((mtk_gps_sys_strncmp(&buffer[3],"GGA")==0) || (mtk_gps_sys_strncmp(&buffer[3],"RMC")==0))){// NMEA sentence
        ret = MTK_GPS_TRUE;
    }else{
        ret = MTK_GPS_FALSE;
    }

    return ret;
}
//Log filter for Offload printing
MTK_GPS_BOOL mtk_gps_sys_ofl_log_filter(const char * buffer, UINT32 length) {
    MTK_GPS_BOOL ret = MTK_GPS_FALSE;
    UNUSED(length);
    if(mtk_gps_sys_strncmp(buffer,"gfns_tx,dump") == 0){
        ret = MTK_GPS_TRUE;
    }else if(mtk_gps_sys_strncmp(buffer,"DUMP,GFNS") == 0){
        ret = MTK_GPS_TRUE;
    }else if(mtk_gps_sys_strncmp(buffer,"[FLP]") == 0){// FLP
        ret = MTK_GPS_TRUE;
    }else{
        ret = MTK_GPS_FALSE;
        //LOGD("len:%d, %s", length, buffer);
    }

    return ret;
}

INT32 mtk_gps_sys_nmea_output_to_app(const char * buffer, UINT32 length) {
    if (enable_dbg_log == MTK_GPS_TRUE) {  // Need to use prop to control debug on/of
        char buf_print[MTK_GPS_HIDE_PRINT_MAX+1] = {0};
        if (mtk_gps_sys_nmea_log_filter(buffer) == MTK_GPS_TRUE) {
            MNLD_STRNCPY(buf_print, buffer, MTK_GPS_HIDE_PRINT_MAX);
            buf_print[MTK_GPS_HIDE_PRINT_MAX] = '\0';
            LOGD("len:%d, [NMEA]%s...", length, buf_print);
        } else if (mtk_gps_sys_ofl_log_filter(buffer, length) == MTK_GPS_TRUE) {
            if (mtk_gps_log_is_hide()) {
                // Forbit to print location log
                MNLD_STRNCPY(buf_print, buffer, MTK_GPS_HIDE_PRINT_MAX);
                buf_print[MTK_GPS_HIDE_PRINT_MAX] = '\0';
                LOGD("len:%d, [OFL]%s...", length, buf_print);
            } else {
                LOGDX("len:%d, %s", length, buffer);
            }
        } else {
            // No limitation, print directly
            LOGD("len:%d, %s", length, buffer);
        }
    }
    return MTK_GPS_SUCCESS;
}

INT32 mtk_gps_sys_nmea_output_to_mnld(const char * buffer, UINT32 length) {
    LOGD("received %d bytes:\n", length);
    mtk_mnl_nmea_parser_process(buffer, length);
    return MTK_GPS_SUCCESS;
}

static unsigned char calc_nmea_checksum1(const char* sentence) {
    unsigned char checksum = 0;

    while (*sentence) {
        checksum ^= (unsigned char)*sentence++;
    }
    return  checksum;
}

INT32 mtk_gps_sys_frame_sync_meas_req(MTK_GPS_FS_WORK_MODE mode) {
    char szBuf_cipher[64];
    char sztmp[64];
    char outbuf[64];

    memset(outbuf, 0, sizeof(outbuf));
    memset(sztmp, 0, sizeof(sztmp));
    memset(szBuf_cipher, 0, sizeof(szBuf_cipher));
    sprintf(sztmp, "PMTK%d,1,%d", PMTK_FS_REQ_MEAS, mode);
    sprintf(outbuf, "$%s*%02X\r\n", sztmp, calc_nmea_checksum1(sztmp));

    // SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
    memcpy(szBuf_cipher, outbuf, strlen(outbuf));
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);

    return MTK_GPS_SUCCESS;
}

INT32 mtk_gps_sys_frame_sync_enable_sleep_mode(unsigned char mode) {
    char szBuf_cipher[64];
    char sztmp[64];
    char outbuf[64];

    memset(outbuf, 0, sizeof(outbuf));
    memset(sztmp, 0, sizeof(sztmp));
    memset(szBuf_cipher, 0, sizeof(szBuf_cipher));
    sprintf(sztmp, "PMTK%d,%d", PMTK_FS_SLEEPMODE, mode);
    sprintf(outbuf, "$%s*%02X\r\n", sztmp, calc_nmea_checksum1(sztmp));

    // SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
    memcpy(szBuf_cipher, outbuf, strlen(outbuf));
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);

    return MTK_GPS_SUCCESS;
}

INT32 mtk_gps_sys_frame_sync_meas_req_by_network(void) {
    char szBuf_cipher[64];
    char sztmp[64];
    char outbuf[64];

    memset(outbuf, 0, sizeof(outbuf));
    memset(sztmp, 0, sizeof(sztmp));
    memset(szBuf_cipher, 0, sizeof(szBuf_cipher));
    sprintf(sztmp, "PMTK%d,0,0", PMTK_FS_REQ_MEAS);
    sprintf(outbuf, "$%s*%02X\r\n", sztmp, calc_nmea_checksum1(sztmp));

    // SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
    memcpy(szBuf_cipher, outbuf, strlen(outbuf));
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);

    return MTK_GPS_SUCCESS;
}


