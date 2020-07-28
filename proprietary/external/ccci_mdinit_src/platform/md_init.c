/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/sysinfo.h>
/*#ifndef MTK_TC1_FEATURE
#include <mtk_log.h>
#endif*/ //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1
#include "libnvram.h"
#include "libnvram_log.h"
#include "CFG_MDTYPE_File.h"
#include "CFG_file_lid.h"
#include "hardware/ccci_intf.h"
#include "CFG_MD_SBP_File.h"
#include "ccci_mdinit_cfg.h"

#include "ccci_common.h"

//For FSD wake_lock not released
#include <hardware_legacy/power.h>
#define FSD_WAKE_UNLOCK() release_wake_lock("ccci_fsd")

// Nvram issue make timeout value as 22s,
#define MAX_OPEN_PORT_RETRY_NUM             (600)


//--------------structure define-----------------//
typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
} CCCI_BUFF_T;

/* MD Message, this is for user space deamon use */
enum {
    CCCI_MD_MSG_BOOT_READY            = 0xFAF50001,
    CCCI_MD_MSG_BOOT_UP                = 0xFAF50002,
    CCCI_MD_MSG_EXCEPTION            = 0xFAF50003,
    CCCI_MD_MSG_RESET                = 0xFAF50004,
    CCCI_MD_MSG_RESET_RETRY            = 0xFAF50005, /*not used*/
    CCCI_MD_MSG_READY_TO_RESET        = 0xFAF50006,
    CCCI_MD_MSG_BOOT_TIMEOUT        = 0xFAF50007,
    CCCI_MD_MSG_STOP_MD_REQUEST        = 0xFAF50008,
    CCCI_MD_MSG_START_MD_REQUEST    = 0xFAF50009,
    CCCI_MD_MSG_ENTER_FLIGHT_MODE    = 0xFAF5000A,
    CCCI_MD_MSG_LEAVE_FLIGHT_MODE    = 0xFAF5000B,
    CCCI_MD_MSG_POWER_ON_REQUEST    = 0xFAF5000C, /*not used*/
    CCCI_MD_MSG_POWER_OFF_REQUEST    = 0xFAF5000D, /*not used*/
    CCCI_MD_MSG_SEND_BATTERY_INFO   = 0xFAF5000E,
    CCCI_MD_MSG_NOTIFY                = 0xFAF5000F,
    CCCI_MD_MSG_STORE_NVRAM_MD_TYPE = 0xFAF50010,
    CCCI_MD_MSG_CFG_UPDATE    = 0xFAF50011,
};

/* MD Status, this is for user space deamon use */
enum {
    CCCI_MD_STA_INIT=-1,
    CCCI_MD_STA_BOOT_READY = 0,
    CCCI_MD_STA_BOOT_UP = 1,
    CCCI_MD_STA_RESET = 2,
    CCCI_MD_STA_WAIT_SRV_RDY_TO_RST = 3,
    CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP = 4,
    CCCI_MD_STA_STOP = 5,
    CCCI_MD_STA_FLIGHT_MODE = 6,
    CCCI_MD_STA_EXCEPTION = 7,
};

enum {
    MD_DEBUG_REL_INFO_NOT_READY = 0,
    MD_IS_DEBUG_VERSION,
    MD_IS_RELEASE_VERSION
};


//----------------maro define-----------------//
// For rild common
#define RILD_PROXY_NAME            "vendor.ril-proxy"
#define RILD_PROXY_SERVICE_STATUS    "vendor.init.svc.ril-proxy"
// For MD1
#define MONITOR_DEV_FOR_MD1        "/dev/ccci_monitor"
#define MUXD_FOR_MD1_NAME        "vendor.gsm0710muxd"
#define RILD_FOR_MD1_NAME        "vendor.ril-daemon-mtk"
#define MD_LOG_FOR_MD1_NAME        "mdlogger"
//#define MD_LOG_FOR_MD1_NAME_E    "ecccimdlogger"
#define MD_LOG_FOR_MD1_NAME_E    "emdlogger1"
#define FSD_FOR_MD1_NAME        "vendor.ccci_fsd"
#define MD1_INIT_CMD            "0"
#define MD1_TAG                    "ccci_mdinit(1)"

// For MD2
#define MONITOR_DEV_FOR_MD2        "/dev/ccci2_monitor"
#define MUXD_FOR_MD2_NAME        "vendor.gsm0710muxdmd2"
#define RILD_FOR_MD2_NAME        "vendor.ril-daemon-mtkmd2"
#define MD_LOG_FOR_MD2_NAME        "dualmdlogger"
#define MD_LOG_FOR_MD2_NAME_E    "emdlogger2"
#define FSD_FOR_MD2_NAME        "vendor.ccci2_fsd"
#define MD2_INIT_CMD            "1"
#define MD2_TAG                    "ccci_mdinit(2)"

// For MD3
#define MONITOR_DEV_FOR_MD3        "/dev/ccci3_monitor"
#define MUXD_FOR_MD3_NAME        "invalid"
#define RILD_FOR_MD3_NAME        "vendor.viarild"
#define MD_LOG_FOR_MD3_NAME        "dualmdlogger"
#define MD_LOG_FOR_MD3_NAME_E    "emdlogger3"
#define FSD_FOR_MD3_NAME        "vendor.ccci3_fsd"
#define MD3_INIT_CMD            "2"
#define MD3_TAG                    "ccci_mdinit(3)"

// Common
#define MD_INIT_NEW_FILE        "/sys/kernel/ccci/boot"
#define BOOT_MODE_FILE            "/sys/class/BOOT/BOOT/boot/boot_mode" // controlled by mtxxxx_boot.c
#define META_MODE                '1'
#define CCCI_MONITOR_CH            (0xf0000000)
#define MD_COMM_TAG                "ccci_mdinit(0)"
#define MD_RESET_WAIT_TIME        "vendor.md.reset.wait"
#define MDLOGGER_CAN_RESET        "vendor.debug.md.reset"
// Phase out
#define MUX_DAEMON_SINGLE    "gsm0710muxd-s"
#define RIL_DAEMON_SINGLE    "ril-daemon-s"

md_type_struct *nvram_md_type = NULL;

//io control define
#define CCCI_MD_BROADCAST_EXCEPTION    (0x1)
#define CCCI_MD_BROADCAST_RESET        (0x2)
#define CCCI_MD_BROADCAST_READY        (0x3)


//----------------variable define-----------------//
static int  md_ctl_fsm_curr_state = CCCI_MD_STA_INIT; /* Modem Control Finity State Machine global control variable */
static int  need_silent_reboot = 0; /* This varialbe will set to 1 when modem exception at boot ready state */
static int  ignore_time_out_msg = 0;
static int  gotten_md_info = MD_DEBUG_REL_INFO_NOT_READY;
//static int  system_ch_handle = 0;
static int  mdlogger_cnt = 0;
static int  md_id = -1;
static char muxd_name[PROPERTY_KEY_MAX];
static char rild_name[PROPERTY_KEY_MAX];
static char mdlogger_name[PROPERTY_KEY_MAX];
static char fsd_name[PROPERTY_KEY_MAX];
static char md_boot_name[32];
//  service property name init.svc.service
static const char *pre_service_status = "init.svc.";
static char muxd_service_status[PROPERTY_KEY_MAX];
static char rild_service_status[PROPERTY_KEY_MAX];
static char mdlogger_service_status[PROPERTY_KEY_MAX];
static char fsd_service_status[PROPERTY_KEY_MAX];

static int dev_monitor_fd = 0;
// service start/stop wait time: s
#define MAX_WAIT_FOR_PROPERTY  2000  // wait 2s for service status changed
#define PROPERTY_WAIT_TIME 10    // 10 ms between polls

typedef struct _wait_prop_t
{
    char        *prop_name;
    char        *disr_value;
}wait_prop_t;
static int wait_property_ready(wait_prop_t *wp, const int count, const int waitmsec);
static void update_service_name(void);
static int set_md_boot_env_data(int md_id, int fd);
/* External functin list by env_setting.c */
extern int compute_random_pattern(unsigned int * p_val);
extern int get_stored_modem_type_val(int md_id);
extern int store_modem_type_val(int md_id, int new_val);
extern unsigned int parse_sys_env_rat_setting(int md_id);


/****************************************************************************/
/* create broadcast thread to broadcast md status to upper layer user                             */
/*                                                                                                                           */
/****************************************************************************/
// Add broadcast thread to fix block time too long
#define STA_QUEUE_LEN        (8)
static int  sta_queue[STA_QUEUE_LEN];
static volatile int  r_idx=0,w_idx=0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int quit = 0;
static char broadcast_str_buf[128];

static bool check_nvram_ready(void)
{
    int read_nvram_ready_retry = 0;
    int ret = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    const int MAX_RETRY_COUNT = 30;
    while (true)
    {
        read_nvram_ready_retry++;
        property_get("vendor.service.nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0 ||
            strcmp(nvram_init_val, "Pre_Ready") == 0)
        {
            ret = true;
            break;
        }
        else
        {
            CCCI_LOGE("%s(), property_get(\"vendor.service.nvram_init\") = %s, read_nvram_ready_retry = %d",
                  __FUNCTION__, nvram_init_val, read_nvram_ready_retry);
            usleep(500 * 1000);
        }
    }
    if (read_nvram_ready_retry >= MAX_RETRY_COUNT)
    {
        CCCI_LOGE("Get nvram restore ready faild !!!\n");
        ret = false;
    }
    return ret;
}


static void set_current_md_status(int status)
{
    char buf[PROPERTY_VALUE_MAX];
    char name[50];
    time_t cur_time;
    int len;

    md_ctl_fsm_curr_state = status;
    switch(status){
            case CCCI_MD_STA_INIT:
                len = snprintf(buf, sizeof(buf), "init");
                break;
            case CCCI_MD_STA_BOOT_READY:
                len = snprintf(buf, sizeof(buf), "ready");
                break;
            case CCCI_MD_STA_BOOT_UP:
                len = snprintf(buf, sizeof(buf), "bootup");
                break;
            case CCCI_MD_STA_RESET:
                len = snprintf(buf, sizeof(buf), "reset");
                break;
            case CCCI_MD_STA_WAIT_SRV_RDY_TO_RST:
                len = snprintf(buf, sizeof(buf), "wait to reset");
                break;
            case CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP:
                len = snprintf(buf, sizeof(buf), "wait to stop");
                break;
            case CCCI_MD_STA_STOP:
                len = snprintf(buf, sizeof(buf), "stop");
                break;
            case CCCI_MD_STA_FLIGHT_MODE:
                len = snprintf(buf, sizeof(buf), "flightmode");
                break;
            default:
                len = snprintf(buf, sizeof(buf), "undefined");
                break;
    }

    time(&cur_time);
    //snprintf(buf+len, sizeof(buf)-len, ctime(&cur_time));

    snprintf(name, sizeof(name), "vendor.mtk.md%d.status",md_id+1);
    CCCI_LOGD("set md status:%s=%s \n",name,buf);
/*#ifndef MTK_TC1_FEATURE
    mtkLogI("ccci_mdinit", "MD%d set status: %s=%s \n", md_id+1, name, buf);
#else*/  //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1
    RLOGI("MD%d set status: %s=%s \n", md_id+1, name, buf);
//#endif
    property_set(name,buf);
    if(md_id == MD_SYS3)
    {
        snprintf(name, sizeof(name), "vendor.net.cdma.mdmstat");
        CCCI_LOGD("set md status:%s=%s \n",name,buf);
        property_set(name,buf);
    }
}
static void check_to_restart_md(void)
{
    char this_md_status_key[32];
    char the_other_md_status_key[32];
    if(md_id == MD_SYS1) {
        snprintf(this_md_status_key, 32, "%s", "vendor.mtk.md1.status");
        snprintf(the_other_md_status_key, 32, "%s", "vendor.mtk.md3.status");
    }else if (md_id == MD_SYS3) {
        snprintf(this_md_status_key, 32, "%s", "vendor.mtk.md3.status");
        snprintf(the_other_md_status_key, 32, "%s", "vendor.mtk.md1.status");
    }
    char buf[PROPERTY_VALUE_MAX];
    int ret;
    int cnt=0;
    int ccif_on = 1;

    do{
    ret = property_get(the_other_md_status_key, buf, NULL);
    if(ret==0){
        if(md_id == MD_SYS1) {
            stop_service_verified(RILD_PROXY_NAME, RILD_PROXY_SERVICE_STATUS, 500);
            CCCI_LOGD("check_to_restart_md kill ril-proxy, md1 only\n");
        }
        CCCI_LOGI("check_to_restart_md:%s not exist\n", the_other_md_status_key);
        return;
    }
        if(strcmp(buf,"reset")==0){
            if(md_id == MD_SYS3){
                if (0 != ioctl(system_ch_handle, CCCI_IOC_RESET_MD1_MD3_PCCIF, NULL)){
                    CCCI_LOGE("reset pccif fail \n");
                }
                CCCI_LOGD("reset pccif/ccirq done\n");
            }
            property_set(the_other_md_status_key,"reset_start");
            CCCI_LOGI("check_to_restart_md:set %s reset_start done\n",the_other_md_status_key);
            break;
        }
        cnt++;
        if((cnt%100)==0){
            CCCI_LOGI("check_to_restart_md:waiting %s=%s...\n",the_other_md_status_key, buf);
        }
        usleep(30*1000);
    }while(1);
    CCCI_LOGI("md%d check_to_restart_md:waiting %s done\n", md_id, the_other_md_status_key);

    do{
    ret = property_get(this_md_status_key, buf, NULL);
    if(ret==0){
        CCCI_LOGI("check_to_restart_md:%s not exist\n", this_md_status_key);
        return;
    }
        if(strcmp(buf,"reset_start")==0){
            CCCI_LOGI("check_to_restart_md: %s reset_start\n",this_md_status_key);
            break;
        }
        cnt++;
        if((cnt%100)==0){
            CCCI_LOGI("check_to_restart_md:waiting %s =%s...\n", this_md_status_key, buf);
        }
        usleep(30*1000);
    }while(1);

    if(md_id == MD_SYS1) {
        stop_service_verified(RILD_PROXY_NAME, RILD_PROXY_SERVICE_STATUS, 500);
        CCCI_LOGD("check_to_restart_md kill ril-proxy\n");
    }
}


/****************************************************************************/
/* modem control message handle function                                                                  */
/*                                                                                                                           */
/****************************************************************************/

static void delay_to_reset_md(void)
{
    char buf[PROPERTY_VALUE_MAX];
    int val;
    property_get(MD_RESET_WAIT_TIME, buf, "none");
    if (0 != strcmp(buf, "none")) {
        val = atoi(buf);
    CCCI_LOGD("Wait modem %ds to reset md\n",val);
    if(0< val && val < 10)
        sleep(val);
    else
        CCCI_LOGD("Wait modem time invalid:%s\n", buf);
    }
}
static void stop_all_ccci_up_layer_services_v1(void)
{
    int retry= 0;
    char buf[PROPERTY_VALUE_MAX];
    wait_prop_t    wp[3];
    int            wpcount = 0;
    int            stopped = 0;

    CCCI_LOGD("stop all up layer service\n");

    // stop rild and fsd, then mdlogger and muxd.
    // stop_service_verified(rild_name, rild_service_status, MAX_WAIT_FOR_PROPERTY);
    property_set("ctl.stop", rild_name);

    // stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
    property_set("ctl.stop", fsd_name);
    wp[wpcount].prop_name = fsd_service_status;
    wp[wpcount].disr_value ="stopped";
    wpcount++;

    stopped = wait_for_property(rild_service_status, "stopped", 300 /*MAX_WAIT_FOR_PROPERTY*/);
    if (stopped >= 0)
    {
        CCCI_LOGD("stop all up layer service: stop %s succeeded!\n", rild_service_status);
    } else
    {
        CCCI_LOGD("stop all up layer service: stop %s failed!\n", rild_service_status);
    }

    // stop_service_verified(muxd_name, muxd_service_status, MAX_WAIT_FOR_PROPERTY);
    if(md_id != MD_SYS3){
        property_set("ctl.stop", muxd_name);
        wp[wpcount].prop_name = muxd_service_status;
        wp[wpcount].disr_value ="stopped";
        wpcount++;
    }

    if(mdlogger_cnt) {
        while(retry < 6){
            retry++;
            property_get(MDLOGGER_CAN_RESET, buf, "0");
            if (0 != strcmp(buf, "0"))    break;
            usleep(50*1000);
        }
        if(retry>=6)CCCI_LOGD("MDlogger not set %s\n",MDLOGGER_CAN_RESET);
        // stop_service_verified(mdlogger_name, mdlogger_service_status, MAX_WAIT_FOR_PROPERTY);
        property_set("ctl.stop", mdlogger_name);
        wp[wpcount].prop_name = mdlogger_service_status;
        wp[wpcount].disr_value ="stopped";
        wpcount++;

        mdlogger_cnt = 0;
    }

    stopped = wait_property_ready(wp, wpcount, MAX_WAIT_FOR_PROPERTY);
    if (stopped > 0)
    {
        CCCI_LOGD("stop all up layer service succeeded!\n");
    } else
    {
        CCCI_LOGD("stop all up layer service failed\n");
    }
    FSD_WAKE_UNLOCK();
}

static void start_all_ccci_up_layer_services_v1(void)
{
    CCCI_LOGD("start all ccci up layer services\n");

    if (need_silent_reboot) {
        CCCI_LOGD("set vendor.ril.mux.report.case 2\n");
        property_set("vendor.ril.mux.report.case", "2"); /* set mux flag here, should before muxd */
    }

    if(mdlogger_cnt == 0) {
        // CCCI_LOGD("start mdlogger\n");
        start_service_verified(mdlogger_name, mdlogger_service_status, 0);
        mdlogger_cnt = 1;
    }

    update_service_name();
    if(md_id != MD_SYS3)
        start_service_verified(muxd_name, muxd_service_status, MAX_WAIT_FOR_PROPERTY);
    else{
        // CCCI_LOGD("start c2k rild\n");
        start_service_verified(rild_name, rild_service_status, MAX_WAIT_FOR_PROPERTY);
    }
}

static int get_nvram_ef_data(int fid, int recsize, void* pdata)
{
    F_ID nvram_ef_fid = {0,0,0};
    int rec_size = 0;
    int rec_num = 0;
    bool isread = false;
    int ret = 0;

    if (pdata == NULL){
        CCCI_LOGE("get_nvram_ef_data: fid=%d, recsize=%d, pdata = NULL\n", fid, recsize);
        return -1;
    }

    nvram_ef_fid = NVM_GetFileDesc(fid, &rec_size, &rec_num, isread);
    if (nvram_ef_fid.iFileDesc < 0)
    {
        CCCI_LOGE("get_nvram_ef_data: Fail to get nvram file descriptor!! fid:%d, errno:0x%x\n", fid, errno);
        return -1;
    }

    if (rec_size != read(nvram_ef_fid.iFileDesc, pdata, rec_size))
    {
        CCCI_LOGE("get_nvram_ef_data: Fail to read nvram file!! fid:%d, errno:0x%x\n", fid, errno);
        ret = -1;
    }

    if (!NVM_CloseFileDesc(nvram_ef_fid)){
        CCCI_LOGE("get_nvram_ef_data: Fail to close nvram file!! fid:%d, errno:0x%x\n", fid, errno);
    }

    return ret;
}

static int store_nvram_ef_data(int fid, int recsize, void* pdata)
{
    F_ID nvram_ef_fid = {0,0,0};
    int rec_size = 0;
    int rec_num = 0;
    bool isread = false;
    void *old_buf = NULL;
    unsigned int *pold_data = NULL;
    int ret = 0;

    CCCI_LOGD("store_nvram_ef_data fid:%d, size:%d, pdata:0x%x\n",
        fid, recsize, ((pdata == NULL) ? 0 : ((unsigned int) pdata)));

    if (!pdata)
        return -1;

    // back up the old data
    old_buf = (void *)malloc(recsize);
    if (!old_buf) {
        CCCI_LOGE("store_nvram_ef_data fid: allocate data memory err:0x%x\n", errno);
    } else {
        ret = get_nvram_ef_data(fid, recsize, old_buf);
        pold_data = (unsigned int *)old_buf;
    }

    if (pold_data != NULL)
        CCCI_LOGD("store_nvram_ef_data ret:%d, data1:%d, data2:0x%x\n",
                fid, (!ret) ? (*pold_data) : 0, (!ret) ? (*(pold_data + 1)) : 0);
    else
        CCCI_LOGE("pold_data is NULL.\n");

    nvram_ef_fid = NVM_GetFileDesc(fid, &rec_size, &rec_num, isread);
    if (nvram_ef_fid.iFileDesc < 0)
    {
        CCCI_LOGE("store_nvram_ef_data: Fail to get nvram file descriptor!! fid:%d, errno:0x%x\n", fid, errno);
        if (old_buf)
            free(old_buf);
        return -1;
    }

    if (rec_size != write(nvram_ef_fid.iFileDesc, pdata, rec_size))
    {
        CCCI_LOGE("store_nvram_ef_data: Fail to write nvram file!! fid:%d, errno:0x%x, rec_size:%d, recsize:%d\n"
            , fid, errno, rec_size, recsize);
        ret = -1;
        // Try to recovery old data
        if (pold_data) {
            rec_size = write(nvram_ef_fid.iFileDesc, pold_data, recsize);
            CCCI_LOGE("store_nvram_ef_data: recovery data!! fid:%d, errno:0x%x, rec_size:%d, recsize:%d\n"
                , fid, errno, rec_size, recsize);
        }
    }

    if (!NVM_CloseFileDesc(nvram_ef_fid)){
        CCCI_LOGE("store_nvram_ef_data: Fail to close nvram file!! fid:%d, errno:0x%x\n", fid, errno);
    }

    if (old_buf)
            free(old_buf);
    return ret;
}

static void boot_ready_state_handler(int message)
{
    int md_type = 0;
    char property_name[50];

    switch(message)
    {
        case CCCI_MD_MSG_EXCEPTION:
            CCCI_LOGD("CCCI_MD_MSG_EXCEPTION\n");
            need_silent_reboot = 1;
            if(md_id == MD_SYS3)
            {
                /*c2krild will use this "exception" state to decide how to triger NE*/
                snprintf(property_name, sizeof(property_name), "vendor.net.cdma.mdmstat");
                CCCI_LOGD("set md3 status:%s=exception \n",property_name);
                property_set(property_name,"exception");
            }
            break;

        case CCCI_MD_MSG_RESET:
            delay_to_reset_md();
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_RST);
            CCCI_LOGD("reset modem at boot ready state\n");

            if( 0 != ioctl(system_ch_handle, CCCI_IOC_DO_STOP_MD, NULL) ){
                CCCI_LOGD("Stop md fail\n");
            }
            if (!is_meta_mode()) { /* Only restart ccci_fsd */
                stop_all_ccci_up_layer_services_v1();
                /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            } else {
                stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            }
            break;

        case CCCI_MD_MSG_STOP_MD_REQUEST:
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP);
            CCCI_LOGD("stop modem at boot ready state\n");
            if (!is_meta_mode()) {
                stop_all_ccci_up_layer_services_v1();
                /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            }
            /* After all services stoped, stop modem hw will be triggered */
            break;

        case CCCI_MD_MSG_ENTER_FLIGHT_MODE:
            CCCI_LOGD("CCCI_MD_MSG_ENTER_FLIGHT_MODE\n");
/*#ifndef MTK_TC1_FEATURE
            mtkLogI("ccci_mdinit", "MD%d enter flight mode\n", md_id+1);
#else*/  //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1
            RLOGI("MD%d enter flight mode\n", md_id+1);
//#endif
            set_current_md_status(CCCI_MD_STA_FLIGHT_MODE);
            if (!is_meta_mode()) {
                stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
                FSD_WAKE_UNLOCK();
            }
            break;
        case CCCI_MD_MSG_STORE_NVRAM_MD_TYPE:
            CCCI_LOGD("CCCI_MD_MSG_STORE_SIM_MODE\n");
            ioctl(system_ch_handle, CCCI_IOC_GET_MD_TYPE_SAVING, &md_type);
            CCCI_LOGD("md%d type in kernel(%d)\n", md_id+1,md_type);
            if (md_type == get_stored_modem_type_val(md_id)){
                CCCI_LOGD("No need to store md type(%d)\n",md_type);
            }
            else{
                store_modem_type_val(md_id,md_type);
            }
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at  boot_ready_state_handler\n", message);
            break;
    }
}

static void boot_up_state_handler(int message)
{
    wait_prop_t wp[2];
    int wpcount = 0;
    int srvstoped = 0;
    int current_boot_mode = MD_BOOT_MODE_INVALID;

    switch(message)
    {
        case CCCI_MD_MSG_BOOT_READY:
            set_current_md_status(CCCI_MD_STA_BOOT_READY);
            CCCI_LOGD("modem ready\n");
            ccci_ccb_check_users(md_id); // must before we start MDlogger
            if (!is_meta_mode()) {
                if(ioctl(system_ch_handle, CCCI_IOC_GET_MD_BOOT_MODE, &current_boot_mode) == 0) {
                    if (current_boot_mode != MD_BOOT_MODE_META)
                        start_all_ccci_up_layer_services_v1();
                    else
                        CCCI_LOGD("boot MD into META mode when system is in normal mode\n");
                } else {
                    start_all_ccci_up_layer_services_v1();
                }
            }
            need_silent_reboot = 0;
            break;

        case CCCI_MD_MSG_EXCEPTION:
            CCCI_LOGD("CCCI_MD_MSG_EXCEPTION\n");
            if(MD_IS_DEBUG_VERSION == gotten_md_info){
                ignore_time_out_msg = 1;
                CCCI_LOGD("Set ignore time out message flag\n");
            }
            if(mdlogger_cnt == 0) {
                CCCI_LOGD("start mdlogger when MD exception early happens at boot_up_state\n");
                start_service_verified(mdlogger_name, mdlogger_service_status, MAX_WAIT_FOR_PROPERTY);
                mdlogger_cnt = 1;
            }
            break;

        case CCCI_MD_MSG_RESET:
            CCCI_LOGD("reset modem at boot up state\n");
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_RST);
            if (!is_meta_mode()) {
                delay_to_reset_md();
                if( 0 != ioctl(system_ch_handle, CCCI_IOC_DO_STOP_MD, NULL) ){
                    CCCI_LOGD("Stop md fail\n");
                }

                /* Only restart ccci_fsd */
                wpcount = 0;
                // stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
                property_set("ctl.stop", fsd_name);
                wp[wpcount].prop_name = fsd_service_status;
                wp[wpcount].disr_value = "stopped";
                wpcount++;

                if(mdlogger_cnt == 1) {
                    CCCI_LOGD("stop mdlogger at boot_up_state\n");
                    // stop_service_verified(mdlogger_name, mdlogger_service_status, MAX_WAIT_FOR_PROPERTY);
                    property_set("ctl.stop", mdlogger_name);
                    wp[wpcount].prop_name = mdlogger_service_status;
                    wp[wpcount].disr_value = "stopped";
                    wpcount++;
                    mdlogger_cnt = 0;
                }

                srvstoped = wait_property_ready(wp, wpcount, MAX_WAIT_FOR_PROPERTY);
                if (srvstoped)
                {
                    CCCI_LOGI("boot_up_state_handler stop service Succeeded!\n");
                } else
                {
                    CCCI_LOGE("boot_up_state_handler stop service Failed!\n");
                }
                FSD_WAKE_UNLOCK();
            }
            break;

        case CCCI_MD_MSG_BOOT_TIMEOUT:
            CCCI_LOGD("modem boot up timeout after send runtime data\n");
            if(ignore_time_out_msg){
                ignore_time_out_msg = 0;
                CCCI_LOGD("Ignore time out message\n");
            }else{
                set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_RST);

                if (!is_meta_mode()) { /* Only restart ccci_fsd */
                    stop_all_ccci_up_layer_services_v1();
                    /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
                }
            }
            /* After all services stoped, modem reset flow will trigger */
            break;

        case CCCI_MD_MSG_STOP_MD_REQUEST:
            CCCI_LOGD("Get stop md request message\n");
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP);

            if (!is_meta_mode()) { /* Only restart ccci_fsd */
                stop_all_ccci_up_layer_services_v1();
                /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            }
            /* After all services stoped, stop modem hw will be triggered */
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at boot_up_state_handler\n", message);
            break;
    }
}

static void reset_state_handler(int message)
{
    int retry = 1;
    int tmp = 0;
    switch(message)
    {
        case CCCI_MD_MSG_BOOT_UP:
            set_current_md_status(CCCI_MD_STA_BOOT_UP);
            CCCI_LOGD("modem boot up, set runtime data\n");
            while( (0 != ioctl(system_ch_handle, CCCI_IOC_SEND_RUN_TIME_DATA, &tmp)) && retry ){
                CCCI_LOGD("Set runtime data fail, try again after 1s \n");
                sleep(1);
                retry--;
            }
            ccci_ccb_init_users(md_id);
            break;

        case CCCI_MD_MSG_EXCEPTION:
            CCCI_LOGD(" CCCI_MD_MSG_EXCEPTION\n");
            if(MD_IS_DEBUG_VERSION == gotten_md_info){
                ignore_time_out_msg = 1;
                CCCI_LOGD("Set ignore time out message flag\n");
            }
            if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA) {
                if(mdlogger_cnt == 0) {
                    CCCI_LOGD("start mdlogger when MD exception early happens at reset_state\n");
                    start_service_verified(mdlogger_name, mdlogger_service_status, MAX_WAIT_FOR_PROPERTY);
                    mdlogger_cnt = 1;
                }
            }
            break;

        case CCCI_MD_MSG_BOOT_TIMEOUT:
            CCCI_LOGD("modem boot up timeout when modem start to boot up\n");
            if(ignore_time_out_msg){
                ignore_time_out_msg = 0;
                CCCI_LOGD(" Ignore time out message\n");
            } // Or, do the CCCI_MD_MSG_RESET_RETRY work continuously
            break;

        case CCCI_MD_MSG_STOP_MD_REQUEST:
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP);
            CCCI_LOGD("stop modem at reset state, get stop message\n");

            if (!is_meta_mode()) { /* Only restart ccci_fsd */
                stop_all_ccci_up_layer_services_v1();
                /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            }
            /* After all services stoped, stop modem hw will be triggered */
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at reset_state_handler\n", message);
            break;
    }
}

// wait_prop_t
// return 1: ready,  0, not ready
static int wait_property_ready(wait_prop_t *wp, const int count, const int waitmsec)
{
    int i;
    int ready = 0;
    int watiflag = 0;
    int bitflag = 0;
    int maxtimes = waitmsec /PROPERTY_WAIT_TIME;
    int needtry = 1;
    wait_prop_t *curwt = NULL;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int needgetprop = 0;

    if (count > 32) {
        CCCI_LOGI("%s: count > %d, return 0\n", __func__, 32);
        return ready;
    }

    if (maxtimes < 1)
        maxtimes = 1;

    CCCI_LOGI("%s: count:%d, waitmsec:%d, loop:%d\n", __func__, count, waitmsec, maxtimes);
    while ((maxtimes-- > 0) && needtry) {
        curwt = wp;
        needtry = 0;

        // CCCI_LOGI("%s: watiflag:0x%x\n", __func__, watiflag);
        for (i=0; i<count; i++) {
            bitflag = ( 1<< i);

            needgetprop = (!(watiflag & bitflag));
            if (needgetprop) {
                if (wait_for_property(curwt->prop_name, curwt->disr_value, 0) >= 0)
                    watiflag |= bitflag;
                else
                    if (needtry == 0) needtry = 1;
            }

            if (needgetprop && (maxtimes % 10 == 0)) {
                CCCI_LOGI("%s: retry name:%s, disrvalue:%s, loop:%d\n",
                    __func__, curwt->prop_name, curwt->disr_value, maxtimes);
            }
            curwt++;
        }

        // if need to retry, sleep about 100ms
        if (needtry)
            usleep(PROPERTY_WAIT_TIME * 1000);
    }

    if (!needtry){
        CCCI_LOGI("%s:Succeeded! count:%d, waitmsec:%d, loop:%d\n", __func__, count, waitmsec, maxtimes);
        ready = 1;
    } else {
        CCCI_LOGI("%s:Failed! count:%d, waitmsec:%d, loop:%d\n", __func__, count, waitmsec, maxtimes);
    }

    return ready;
}

static void wait_service_ready_to_reset_state_handler(int message)
{
    int tmp = 0;
    int ret = 0;
    int curr_md_type;
    char md_id_str[16];

    switch(message)
    {
        case CCCI_MD_MSG_READY_TO_RESET:
            set_current_md_status(CCCI_MD_STA_RESET);
            CCCI_LOGD("All up layer ready, begin to reset\n");
            // Update active md start ----------------
            ret = ioctl(system_ch_handle, CCCI_IOC_GET_MD_TYPE, &curr_md_type);
            if(0 == ret){
                snprintf(md_id_str, 16, "%d", curr_md_type);
                property_set("vendor.ril.active.md", md_id_str);
            }else{
                CCCI_LOGD("[Active MD]get md type fail: %d(%d)\n", errno, ret);
            }
            // Update active md end ----------------
            check_to_restart_md();
            //meta mode need switch modem in world phone projects
            //if (!is_meta_mode()) { /* Only restart ccci_fsd */
                // property_set("ctl.start", fsd_name); /* restart ccci_fsd here */
                start_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);

                set_md_boot_env_data(md_id, dev_monitor_fd);
                while(0 != (ret=ioctl(system_ch_handle, CCCI_IOC_DO_START_MD, &tmp))){
                    CCCI_LOGD("Triggert modem reset ioctl fail, try again after 1s\n");
                    sleep(1);
                }
                if(ret!=0)
                    CCCI_LOGD("Triggert modem reset ioctl fail, after retry\n");
            //}
            /* After all services stoped, modem reset flow will trigger */
            break;

        case CCCI_MD_MSG_STOP_MD_REQUEST:
            set_current_md_status(CCCI_MD_STA_STOP);
            CCCI_LOGD("All up layer ready, begin to stop md\n");
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at wait_service_ready_to_reset_state_handler\n", message);
            break;
    }
}

static void wait_service_ready_to_stop_state_handler(int message)
{
    int tmp=0;

    switch(message)
    {
        case CCCI_MD_MSG_READY_TO_RESET:
            set_current_md_status(CCCI_MD_STA_STOP);
            CCCI_LOGD("Power off MD done\n");
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at wait_service_ready_to_stop_state_handler\n", message);
            break;
    }
}

static void stop_state_handler(int message)
{
    int tmp = 0;
    int retry = 1;
    switch(message)
    {
        case CCCI_MD_MSG_RESET:
        case CCCI_MD_MSG_START_MD_REQUEST:
            delay_to_reset_md();
            set_current_md_status(CCCI_MD_STA_RESET);
            check_to_restart_md();
            if (!is_meta_mode()) { /* Only restart ccci_fsd */
                start_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
                sleep(1);
            }
            CCCI_LOGD("Begin to start md\n");
            set_md_boot_env_data(md_id, dev_monitor_fd);
            while( (0 != ioctl(system_ch_handle, CCCI_IOC_DO_START_MD, &tmp)) && retry ){
                CCCI_LOGD("Start md fail, try again after 1s \n");
                sleep(1);
                retry--;
            }
            break;
        default:
            CCCI_LOGD("Ignore msg:%08x at stop_state_handler\n", message);
            break;
    }
}

static void flight_state_handler(int message)
{
    switch(message)
    {
        case CCCI_MD_MSG_LEAVE_FLIGHT_MODE:
/*#ifndef MTK_TC1_FEATURE
            mtkLogI("ccci_mdinit", "MD%d leave flight mode\n", md_id+1);
#else*/  //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1
            RLOGI("MD%d leave flight mode\n", md_id+1);
//#endif
            set_current_md_status(CCCI_MD_STA_WAIT_SRV_RDY_TO_RST);
            CCCI_LOGD("leave flight mode at flight mode state\n");
            wait_for_property("vendor.ril.getccci.response", "1", 500);
            if (!is_meta_mode()) {
                stop_all_ccci_up_layer_services_v1();
                /// stop_service_verified(fsd_name, fsd_service_status, MAX_WAIT_FOR_PROPERTY);
            }
            /* After all services stoped, modem reset flow will trigger */
            break;

        default:
            CCCI_LOGD("Ignore msg:%08x at flight_state_handler\n", message);
            break;
    }
}


static int common_msg_handler(int msg, int resv)
{
    int ret = 1;
    int data = 0;
    char str[32];

    switch (msg)
    {
        #if 0
        case CCCI_MD_MSG_DUMP_MD_MEM:
            dump_md_mem();
            break;
        #endif

        case CCCI_MD_MSG_SEND_BATTERY_INFO:
            if(ioctl(system_ch_handle, CCCI_IOC_SEND_BATTERY_INFO, &data))
                CCCI_LOGE("send md battery info fail: %d", errno);
            else
                CCCI_LOGD("send md battery info OK");
            break;

        case CCCI_MD_MSG_NOTIFY:
                CCCI_LOGD("Get notify case: %d\n", resv);
                break;

        case CCCI_MD_MSG_CFG_UPDATE:
            CCCI_LOGD("CFG UPDATE: 0x%x(dummy)\n", resv);
            break;

        default:
            ret = 0;
            break;
    }
    return ret;
}


/****************************************************************************/
/* initial and main thread                                                                                           */
/*                                                                                                                           */
/****************************************************************************/
static int trigger_modem_to_run(char *file_name)
{
    int fd;
    size_t s;
    char data[20];

    /*
     * MODEM boot-up:
     * step 1: write any char to MD_INTI_FILE
     */

    fd = open(file_name, O_RDWR);
    if (fd < 0) {
        CCCI_LOGD("fail to open %s: %d", file_name, errno);
        perror("");
        return -1;
    }

    if(md_id == 0) {
        snprintf(data, 4, "%s", MD1_INIT_CMD);
        s = write(fd, data, strlen(MD1_INIT_CMD));
    } else if(md_id == 1) {
        snprintf(data, 4, "%s", MD2_INIT_CMD);
        s = write(fd, data, strlen(MD2_INIT_CMD));
    } else if(md_id == 2) {
        snprintf(data, 4, "%s", MD3_INIT_CMD);
        s = write(fd, data, strlen(MD3_INIT_CMD));
    } else {
        CCCI_LOGD("Fail to trigger md run, invalid md id%d", md_id);
        perror("");
        goto out;
    }
    if (s <= 0) {
        CCCI_LOGD("fail to write %s to %s: %d", data, file_name, errno);
        perror("");
    }

out:
    close(fd);
    return 0;
}

static void update_service_name(void)
{
    if(md_id != 0)
        return;

    snprintf(muxd_name, 32, MUXD_FOR_MD1_NAME);
    snprintf(muxd_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, muxd_name);
    snprintf(rild_name, 32, RILD_FOR_MD1_NAME);
    snprintf(rild_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, rild_name);
    CCCI_LOGD("Current muxd: %s, rild:%s\n", muxd_name, rild_name);
}

static int get_usp_sbp_setting(void)
{
#define MTK_USP_SBP_NAME "persist.vendor.mtk_usp_md_sbp_code"
    char buf[PROPERTY_VALUE_MAX] ={ 0 };
    int ret;
    int sbp_code = 0;

    ret = property_get(MTK_USP_SBP_NAME, buf, NULL);
    if(ret==0){
        CCCI_LOGI("USP_SBP:%s not exist\n", MTK_USP_SBP_NAME);
        return 0;
    }
    sbp_code = (int)str2uint(buf);
    if (sbp_code < 0) {
        CCCI_LOGE("USP_SBP:%s=%s,usp_sbp=%d is a invalide value\n", MTK_USP_SBP_NAME, buf, sbp_code);
        sbp_code = 0;
    } else
        CCCI_LOGI("USP_SBP:%s=%s,usp_sbp=%d\n", MTK_USP_SBP_NAME, buf, sbp_code);

    return sbp_code;
}
// return: 0-fail,  others-setting value
static unsigned int get_cip_sbp_setting(int md_id)
{
#define CIP_SBP_FILE "CIP_MD_SBP"
#define CIP_MD2_SBP_FILE "CIP_MD2_SBP"
    char md_cip_sbp_file[100];
    int fd, read_len;
    struct stat sbp_stat;
    char cip_sbp_value[16];
    long retl = 0;
    unsigned int ret = 0;
    char *endptr = NULL;

    memset(md_cip_sbp_file, 0x0, sizeof(md_cip_sbp_file));
    if (md_id == MD_SYS1) {
        snprintf(md_cip_sbp_file, sizeof(md_cip_sbp_file), "%s%s", md_img_cip_folder, CIP_SBP_FILE);
    } else if (md_id == MD_SYS2) {
        snprintf(md_cip_sbp_file, sizeof(md_cip_sbp_file), "%s%s", md_img_cip_folder, CIP_MD2_SBP_FILE);
    } else {
        CCCI_LOGD("get_cip_sbp_setting, md_id:%d is error!\n", md_id);
        return 0;
    }

    umask(0007);
    if (stat(md_cip_sbp_file, &sbp_stat) < 0) {
        CCCI_LOGD("get_cip_sbp_setting, file %s NOT exists!\n", md_cip_sbp_file);
        return 0;
    }

    fd = open(md_cip_sbp_file, O_RDONLY);
    if(fd < 0) {
        CCCI_LOGD("get_cip_sbp_setting, open file %s Fail! err:%d\n", md_cip_sbp_file, errno);
        return 0;
    }

    memset(cip_sbp_value, 0x0, sizeof(cip_sbp_value));
    read_len = (int)read(fd, cip_sbp_value, sizeof(cip_sbp_value) - 1);
    if(read_len <= 0) {
        CCCI_LOGD("get_cip_sbp_setting, read file %s Fail! err:%d\n", md_cip_sbp_file, errno);
        close(fd);
        return 0;
    }
    close(fd);
    cip_sbp_value[read_len] = 0;

    retl = (long)str2uint(cip_sbp_value);
    if (retl > 0) {
        ret = (unsigned int)(retl & 0xFFFFFFFF);
        CCCI_LOGD("get_cip_sbp_setting, get sbp setting:0x%x\n", ret);
    } else {
        CCCI_LOGD("get_cip_sbp_setting, Error!! sbp setting is 0!\n");
    }

    return ret;
}
static int get_nvram_sbp_code(int md_id)
{
#define MD_SBP_PATH_FILE "/mnt/vendor/nvdata/APCFG/APRDCL/MD_SBP"
    int getsbpcode = 0;
    int store_sbp_code = 0;
    MD_SBP_Struct *nvram_sbp_info = NULL;
    int md_sbp_lid = -1;

    check_nvram_ready();

    md_sbp_lid = NVM_GetLIDByName(MD_SBP_PATH_FILE);
    if (md_sbp_lid < 0) {
        CCCI_LOGE("Error!! get sbp lid fail!!!%d\n", md_sbp_lid);
        goto EXIT_FUN;
    }
    nvram_sbp_info = (MD_SBP_Struct *)malloc(sizeof(MD_SBP_Struct));
    if (nvram_sbp_info == NULL) {
        CCCI_LOGD("Error!! malloc md sbp code fail! errno:%d\n", errno);
        goto EXIT_FUN;
    }
    memset((void *)nvram_sbp_info, 0, sizeof(MD_SBP_Struct));
    getsbpcode = get_nvram_ef_data(md_sbp_lid,    sizeof(MD_SBP_Struct), nvram_sbp_info);
    if (getsbpcode != 0) {
        CCCI_LOGD("Error!! get_nvram_ef_data fail lid=%d,ret:%d\n", md_sbp_lid, getsbpcode);
        goto EXIT_FUN;

    }
    if (md_id == MD_SYS2)
        store_sbp_code = nvram_sbp_info->md2_sbp_code;
    else
        store_sbp_code = nvram_sbp_info->md_sbp_code;
EXIT_FUN:
    if (nvram_sbp_info) {
        free(nvram_sbp_info);
        nvram_sbp_info = NULL;
    }

    return store_sbp_code;
}

static int get_project_sbp_code(int md_id)
{
    int sbp_code = 0;
    char tmp_buf[10] = {0};

    if (md_id == MD_SYS2) {
        // get md2 SBP code from ProjectConfig.mk from ccci_lib
        if (0 == query_prj_cfg_setting("MTK_MD2_SBP_CUSTOM_VALUE", tmp_buf, sizeof(tmp_buf)))
            sbp_code = (int)str2uint(tmp_buf);
    } else {
        // get md1 SBP code from ProjectConfig.mk from ccci_lib
        if (0 == query_prj_cfg_setting("MTK_MD_SBP_CUSTOM_VALUE", tmp_buf, sizeof(tmp_buf)))
            sbp_code = (int)str2uint(tmp_buf);
    }
    return sbp_code;
}

/*
 *  for MD SBP feature, diferent operators use the same md image
 *  MTK_MD_SBP_CUSTOM_VALUE_ must be definied on ProjectConfig.mk
 *    0: INVALID value, the project need SBP feature, but value 0 needn't transform to modem
 *  related files:
 *    MD_SBP: under /mnt/vendor/nvdata/APCFG/APRDCL/, see about CFG_MD_SBP_File.h
 *      the md_sbp_value of MD_SBP could be assigned by MTK_MD_SBP_CUSTOM_VALUE_ in ProjectConfig.mk
 *      if MTK_MD_SBP_CUSTOM_VALUE_=0, it means need SBP process flow, but not care ProjectConfig value.
 *    CIP_MD_SBP: under /custom/etc/firmware/, It is Hexadecimai number string, ex: 0x3
 *  Rules:
 *    wwop project use CIP_MD_SBP file, the number transform from CIP_MD_SBP MUST NOT be 0
 *        MTK_MD_SBP_CUSTOM_VALUE_ SHOULD be defined as 0x0(or 0) in ProjectConfig.mk of wwop project
 *    in non-wwop project:
 *      MTK_MD_SBP_CUSTOM_VALUE_ Defined: the MTK_MD_SBP_CUSTOM_VALUE_ should be transfer to md
 *      MTK_MD_SBP_CUSTOM_VALUE_ Undefined: the value in MD_SBP should be transfer to md
 *    The sb_code needn't transfer to md from the second boot up time, if the sb_code is not changed from the first boot.
 */
static int get_md_sbp_code(int md_id)
{
    int sbp_default = 0;
    int cip_sbp_value = 0;
    int stored_sbp_code = 0;
    static int sbpc = 0;
    int usp_sbp_value;

    /* NOTES:
    * priority: USP > CIP > ProjectConfig > meta tool
    * Assume:
    *    0. uniservice pack property for global device
    *    1. wwop(CIP) project could not be modified by meta tool
    *    2. ProjectConfiged project could not be modified by meta tool
    *    3. meta tool could modified project MUST NOT define MTK_MD_SBP_CUSTOM
    */
    usp_sbp_value =  get_usp_sbp_setting();
    if (usp_sbp_value && usp_sbp_value != sbpc)
        sbpc = usp_sbp_value;
    /*if static var sbpc has been set, then return directly*/
    if(sbpc)
        return sbpc;

    cip_sbp_value = get_cip_sbp_setting(md_id);
    stored_sbp_code = get_nvram_sbp_code(md_id);
    sbp_default = get_project_sbp_code(md_id);

    if (stored_sbp_code > 0)
        sbpc = stored_sbp_code;

    if (sbp_default > 0)
        sbpc = sbp_default;

    if (cip_sbp_value > 0)
        sbpc = cip_sbp_value;
    CCCI_LOGD("Get: usp_sbp=%d, cip_sbp=%d, project_sbp=%d, nvram_sbp=%d, set sbp=%d\n",
        usp_sbp_value, cip_sbp_value, sbp_default, stored_sbp_code, sbpc);

    return sbpc;
}

extern void* monitor_time_update_thread(void* arg);
extern int time_srv_init(void);
static void cal_md_starttime(CCCI_BUFF_T buff)
{
    char name[50];
    char time_val[PROPERTY_VALUE_MAX];
    static time_t time_HS1, time_HS2;
    if (buff.data[1] == CCCI_MD_MSG_BOOT_UP)
    {
        time(&time_HS1);
    }
    else if(buff.data[1] == CCCI_MD_MSG_BOOT_READY)
    {
        time(&time_HS2);
        if (snprintf(time_val, sizeof(time_val),"%lds %s", time_HS2-time_HS1, ctime(&time_HS2)) >=PROPERTY_VALUE_MAX)
        {
            CCCI_LOGE("time failure: %ld\n", time_HS2-time_HS1);
        }
        else if (time_HS2-time_HS1>30) //
        {
            CCCI_LOGE("time between HS1 and HS2 over 30s: %ld\n", time_HS2-time_HS1);
        }

        snprintf(name, sizeof(name), "vendor.mtk.md%d.starttime", md_id+1);
        property_set(name, time_val);
    }
}

typedef enum {
    MODE_UNKNOWN = -1,      /* -1 */
    MODE_IDLE,              /* 0 */
    MODE_USB,               /* 1 */
    MODE_SD,                /* 2 */
    MODE_POLLING,           /* 3 */
    MODE_WAITSD,            /* 4 */
} LOGGING_MODE;
/* MD logger configure file */
#define MD1_LOGGER_FILE_PATH "/data/mdlog/mdlog1_config"
#define MD2_LOGGER_FILE_PATH "/data/mdlog/mdlog2_config"
#define MD3_LOGGER_FILE_PATH "/data/mdlog/mdlog3_config"
#define MDLOGGER_FILE_PATH   "/data/mdl/mdl_config"

static int get_mdlog_boot_mode(int md_id)
{
    int fd, ret;
    unsigned int mdl_mode = 0;

    switch(md_id) {
    case 0:
        fd = open(MD1_LOGGER_FILE_PATH, O_RDONLY);
        if(fd < 0)
            fd = open(MDLOGGER_FILE_PATH, O_RDONLY);
        break;
    case 1:
        fd = open(MD2_LOGGER_FILE_PATH, O_RDONLY);
        break;
    case 3:
        fd = open(MD3_LOGGER_FILE_PATH, O_RDONLY);
        break;
    default:
        CCCI_LOGE("Open md_id=%d error!\n", md_id);
        fd = -1;
        break;
    }
    if (fd < 0) {
        CCCI_LOGE("Open md_log_config file failed, errno=%d!\n", errno);
        mdl_mode = MODE_SD;
        return mdl_mode;
    }
    ret = read(fd, &mdl_mode, sizeof(unsigned int));
    if (ret != sizeof(unsigned int)) {
        CCCI_LOGE("read failed ret=%d, errno=%d!\n", ret, errno);
        mdl_mode = MODE_SD;
    }
    close(fd);
    return mdl_mode;
}

static int get_md_dbg_dump_flag(int md_id)
{
    char buf[PROPERTY_VALUE_MAX] = { '\0' };
    int ret = -1; /* equal to 0xFFFFFFFF as Uint32 in kernel */

    if (md_id == MD_SYS1)
        property_get("persist.vendor.ccci.md1.dbg_dump", buf, "none");
    else if (md_id == MD_SYS3)
        property_get("persist.vendor.ccci.md3.dbg_dump", buf, "none");

    if (0 != strcmp(buf, "none"))
        ret = strtoul(buf, NULL, 16);
    return ret;
}

static int set_md_boot_env_data(int md_id, int fd)
{
    unsigned int data[16] = { 0 };

    data[0] = get_mdlog_boot_mode(md_id);
    data[1] = get_md_sbp_code(md_id);
    data[2] = get_md_dbg_dump_flag(md_id);
    CCCI_LOGI("set md boot data:mdl=%d sbp=%d dbg_dump=%d\n", data[0], data[1], data[2]);
    ioctl(fd, CCCI_IOC_SET_BOOT_DATA, &data);
    return 0;
}

/* Util function */
static int get_ft_inf_key_val_helper(char key[], int *pval, char str[], int length)
{
    int i;
    char curr_key[64];
    char curr_val[64];
    int step = 0;
    int key_size = 0;
    int val_size = 0;

    for (i = 0; i < length; i++){
        if (step == 0) {
            /* Get key step */
            if (((str[i] >= 'a') && (str[i] <= 'z')) ||
                ((str[i] >= 'A') && (str[i] <= 'Z')) ||
                ((str[i] >= '0') && (str[i] <= '9')) || (str[i]=='_')) {
                curr_key[key_size] = str[i];
                key_size++;
            } else if (str[i]==':') {
                step = 1;
            } else {
                /* Invalid raw string */
                return -1;
            }
        } else if (step == 1) {
            /* Get key step */
            if ((str[i] >= '0') && (str[i] <= '9')) {
                curr_val[val_size] = str[i];
                val_size++;
            } else if ((str[i]==',') || (str[i]=='\0')){
                /* find a key:val pattern? */
                if (val_size == 0) {
                    /* val size abnormal */
                    return -1;
                }
                curr_key[key_size] = 0;
                curr_val[val_size] = 0;
                if (strcmp(key, curr_key) != 0) {
                    key_size = 0;
                    val_size = 0;
                    step = 0;
                } else {
                    *pval = atoi(curr_val);
                    return 1;
                }
            } else {
                /* Invalid raw string */
                return -1;
            }
        } /* end of if (step...) */
    }/* end of for (... */

    curr_key[key_size] = 0;
    curr_val[val_size] = 0;
    if (strcmp(key, curr_key) != 0)
        return 0;

    if (val_size == 0)
        return -1;

    *pval = atoi(curr_val);
    return 1;
}

static int get_ft_inf_key_val(char key[], int *p_val)
{
    int fd;
    int ft_raw_size;
    char *p_ft_raw;
    int ret;
    fd = open("/sys/kernel/ccci/ft_info", O_RDONLY);
    if (fd < 0)
        return -11;

    p_ft_raw = malloc(4096);
    if (p_ft_raw == NULL) {
        close(fd);
        return -12;
    }

    ft_raw_size = read(fd, p_ft_raw, 4096);
    if (ft_raw_size <= 0) {
        free(p_ft_raw);
        p_ft_raw = NULL;
        close(fd);
        return -13;
    }

    ret = get_ft_inf_key_val_helper(key, p_val, p_ft_raw, ft_raw_size);
    free(p_ft_raw);
    p_ft_raw = NULL;
    close(fd);

    return ret;
}

static int get_int_property_val(const char *name, const char *desired_value)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int retpropget = 0;
    int val;

    retpropget = property_get(name, value, desired_value);
    if (retpropget > 0)
        val = atoi(value);
    else
        val = atoi(desired_value);

    CCCI_LOGI("get_int_property_val: %s [%d]", name, val);
    return val;
}

int main_v1(int input_md_id, int sub_ver)
{
    int ret, fd, tmp=0;
    ssize_t s;
    CCCI_BUFF_T buff;
    char nvram_init_val[PROPERTY_VALUE_MAX] = {0};
    char dev_port[32];
    pthread_t tid;
    unsigned int monitor_ch;
    int port_open_retry = MAX_OPEN_PORT_RETRY_NUM;
    unsigned int md_type = 0;
    char tmp_buf[4];
    int ft_query_ver = 0;
    char *lk_info_buf;

    CCCI_LOGI("md_init ver:1");

    //Check if input parameter is valid
    if(sub_ver == 0) {
        CCCI_LOGI("Use original version!\n");
        md_id = 0;
        monitor_ch = 2;
    } else {
        md_id = input_md_id - 1;
        monitor_ch = CCCI_MONITOR_CH;
    }

    set_current_md_status(CCCI_MD_STA_INIT);

    //Configure service and dev port name
    switch(md_id)
    {
        case 0:
            snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS1));
            snprintf(muxd_name, 32, "%s", MUXD_FOR_MD1_NAME);
            snprintf(rild_name, 32, "%s", RILD_FOR_MD1_NAME);
            if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA)
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD1_NAME_E);
            else
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD1_NAME);
            snprintf(fsd_name, 32, "%s", FSD_FOR_MD1_NAME);
            break;
        case 1:
            snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS2));
            snprintf(muxd_name, 32, "%s", MUXD_FOR_MD2_NAME);
            snprintf(rild_name, 32, "%s", RILD_FOR_MD2_NAME);
            if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA)
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD2_NAME_E);
            else
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD2_NAME);

            snprintf(fsd_name, 32, "%s", FSD_FOR_MD2_NAME);
            break;
        case 2:
            snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS3));
            snprintf(muxd_name, 32, "%s", MUXD_FOR_MD3_NAME);
            snprintf(rild_name, 32, "%s", RILD_FOR_MD3_NAME);
            if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA)
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD3_NAME_E);
            else
                snprintf(mdlogger_name, 32, "%s", MD_LOG_FOR_MD3_NAME);

            snprintf(fsd_name, 32, "%s", FSD_FOR_MD3_NAME);
            break;
        default:
            CCCI_LOGE("[Error]Invalid md sys id: %d\n", md_id+1);
            return -1;
    }

    snprintf(muxd_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, muxd_name);
    snprintf(rild_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, rild_name);
    snprintf(mdlogger_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, mdlogger_name);
    snprintf(fsd_service_status, PROPERTY_KEY_MAX, "%s%s", pre_service_status, fsd_name);

    CCCI_LOGI("[%s][%s][%s][%s]\n",
        muxd_service_status,
        rild_service_status,
        mdlogger_service_status,
        fsd_service_status);

    snprintf(md_boot_name, 32, MD_INIT_NEW_FILE); // <== Using new md boot file


    set_current_md_status(CCCI_MD_STA_RESET);

    // Retry to open if dev node attr not ready
    while(1) {
        fd = open(dev_port, O_RDWR);
        if (fd < 0) {
            port_open_retry--;
            if(port_open_retry>0) {
                usleep(10*1000);
                continue;
            } else {
                CCCI_LOGE("open %s fail: %d", dev_port, errno);
                return -1;
            }
        } else {
            CCCI_LOGD("%s is opened(%d).", dev_port, (MAX_OPEN_PORT_RETRY_NUM-port_open_retry));
            break;
        }
    }
    dev_monitor_fd = fd;

    md_image_exist_check(fd,curr_md_id);

    // Get current MD type and update mux daemon name
    update_service_name();

    if(time_srv_init()==0){
        pthread_create(&tid, NULL, monitor_time_update_thread, NULL);
    }

    set_md_boot_env_data(md_id, dev_monitor_fd);
    tmp = parse_sys_env_rat_setting(md_id);
    if(ioctl(fd, CCCI_IOC_RELOAD_MD_TYPE, &tmp) != 0)
        CCCI_LOGD("update modem type to kernel fail: err=%d", errno);

    ret = trigger_modem_to_run(md_boot_name);
    if (ret < 0) {
        CCCI_LOGE("boot modem fail!\n");
        return ret;
    }

    CCCI_LOGD("deamon begin to run!\n");

    if( gotten_md_info == MD_DEBUG_REL_INFO_NOT_READY ){
        if(0 == ioctl(fd, CCCI_IOC_GET_MD_INFO, &tmp)){
            gotten_md_info = tmp;
            if(gotten_md_info == MD_IS_DEBUG_VERSION)
                CCCI_LOGD("MD is debug version");
            else if(gotten_md_info == MD_IS_RELEASE_VERSION)
                CCCI_LOGD("MD is release version");
            else{
                CCCI_LOGD("MD version is not ready now");
                gotten_md_info = MD_DEBUG_REL_INFO_NOT_READY;
            }
        }else{
            CCCI_LOGD("MD version is unknow: err=%d", errno);
        }
    }

    system_ch_handle = fd;
    /* block on reading CCCI_MONITOR device until a modem reset message is gotten */
    do {
        s = read(fd, (void *)&buff, sizeof(buff));
        if (s<=0) {
            if(s != -1) {
                CCCI_LOGE("read fail ret=%d\n", errno);
            }
            continue;
        } else if (s!= sizeof(buff)) {
            CCCI_LOGE("read CCCI data with unexpected size: %d\n", (int)s);
            continue;
            //return -1;
        }

        CCCI_LOGD("buff.data[0] = 0x%08X, data[1] = 0x%08X, channel = %08X, reserved = 0x%08X\n",
            buff.data[0], buff.data[1], buff.channel, buff.reserved);

        if ( (buff.data[0] == 0xFFFFFFFF) && (buff.channel == monitor_ch) ) { /* Monitor channel message */
            /* Check common message first */
            if (common_msg_handler(buff.data[1], buff.reserved)) {
                continue;
            }

            cal_md_starttime(buff);

            if (buff.data[1] == 0xFAF51234)
            {
                unsigned int val;
                compute_random_pattern((unsigned int *) &val);
                if (0!=ioctl(system_ch_handle, CCCI_IOC_RESET_AP, &val))
                    CCCI_LOGE("reset_ap_ioctl failed.\n");
            }

            /* Process MD message */
            switch(md_ctl_fsm_curr_state)
            {
                case CCCI_MD_STA_BOOT_READY:
                    boot_ready_state_handler(buff.data[1]);
                    break;

                case CCCI_MD_STA_BOOT_UP:
                    boot_up_state_handler(buff.data[1]);
                    break;
                case CCCI_MD_STA_RESET:
                    reset_state_handler(buff.data[1]);
                    break;
                case CCCI_MD_STA_WAIT_SRV_RDY_TO_RST:
                    wait_service_ready_to_reset_state_handler(buff.data[1]);
                    break;

                case CCCI_MD_STA_WAIT_SRV_RDY_TO_STOP:
                    wait_service_ready_to_stop_state_handler(buff.data[1]);
                    break;

                case CCCI_MD_STA_STOP:
                    stop_state_handler(buff.data[1]);
                    break;

                case CCCI_MD_STA_FLIGHT_MODE:
                    flight_state_handler(buff.data[1]);
                    break;

                default:
                    CCCI_LOGE("Invalid message, should not enter here!!!\n");
                    break;
            }
        } else { /* pattern not invalid */
            CCCI_LOGE("[Error]Invalid pattern, data[0]:%08x channel:%08x\n", buff.data[0], buff.channel);
            continue;
        }
    } while (1);

    system_ch_handle = 0;
    quit = 1;
    pthread_cond_signal(&cond);

    return 0;
}

