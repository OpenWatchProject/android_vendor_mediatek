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
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/sysinfo.h>
#include "hardware/ccci_intf.h"
#include "CFG_MD_SBP_File.h"
#include "ccci_mdinit_cfg.h"
#include "mdinit_relate.h"
#include "platform/mdinit_platform.h"
#include "ccci_common.h"


// NVRAM issue make timeout value as 22s,
#define MAX_OPEN_PORT_RETRY_NUM            (600)

static int mdstatusfd = -1;

//--------------structure define-----------------//
typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
} CCCI_BUFF_T;

static struct md_status_event status_buf;

/* MD Message, this is for user space deamon use */
enum {
    CCCI_MD_MSG_FORCE_STOP_REQUEST = 0xFAF50001,
    CCCI_MD_MSG_FLIGHT_STOP_REQUEST,
    CCCI_MD_MSG_FORCE_START_REQUEST,
    CCCI_MD_MSG_FLIGHT_START_REQUEST,
    CCCI_MD_MSG_RESET_REQUEST,

    CCCI_MD_MSG_EXCEPTION,
    CCCI_MD_MSG_SEND_BATTERY_INFO,
    CCCI_MD_MSG_STORE_NVRAM_MD_TYPE,
    CCCI_MD_MSG_CFG_UPDATE,
    CCCI_MD_MSG_RANDOM_PATTERN,
};

/* MD Status, this is for user space deamon use */
enum {
    CCCI_MD_STA_INIT = -1,
    CCCI_MD_STA_BOOT_READY = 0,
    CCCI_MD_STA_BOOT_UP,
    CCCI_MD_STA_RESET,
    CCCI_MD_STA_STOP,
    CCCI_MD_STA_FLIGHT_MODE,
    CCCI_MD_STA_EXCEPTION,
};

enum {
    MD_DEBUG_REL_INFO_NOT_READY = 0,
    MD_IS_DEBUG_VERSION,
    MD_IS_RELEASE_VERSION
};

//----------------maro define-----------------//
// For MD1
#define MONITOR_DEV_FOR_MD1        "/dev/ccci_monitor"
#define MD1_INIT_CMD            "0"
#define MD1_TAG                "ccci_mdinit(1)"

// For MD2
#define MONITOR_DEV_FOR_MD2        "/dev/ccci2_monitor"
#define MD2_INIT_CMD            "1"
#define MD2_TAG                "ccci_mdinit(2)"

// For MD3
#define MONITOR_DEV_FOR_MD3        "/dev/ccci3_monitor"
#define MD3_INIT_CMD            "2"
#define MD3_TAG                "ccci_mdinit(3)"

// Common
#define MD_INIT_OLD_FILE        "/sys/class/BOOT/BOOT/boot/md"
#define MD_INIT_NEW_FILE        "/sys/kernel/ccci/boot"
#define CCCI_MONITOR_CH            (0xf0000000)
#define MD_COMM_TAG            "ccci_mdinit(0)"

//----------------variable define-----------------//
static int  md_ctl_fsm_curr_state = CCCI_MD_STA_INIT; /* Modem Control Finity State Machine global control variable */
static int  ignore_time_out_msg = 0;
static int  gotten_md_info = MD_DEBUG_REL_INFO_NOT_READY;
static int  md_id = -1;
static char md_boot_name[32];

static int set_md_boot_env_data(int md_id, int fd);

/* External functin list by env_setting.c */
extern int compute_random_pattern(unsigned int * p_val);
extern int get_stored_modem_type_val(int md_id);
extern int store_modem_type_val(int md_id, int new_val);
extern unsigned int parse_sys_env_rat_setting(int md_id);
extern int update_inf_to_bootprof(char str[]);

static int check_curret_md_status(int desired)
{
    if (md_ctl_fsm_curr_state == desired) {
        CCCI_LOGI("status already is %d\n", desired);
        return 1;
    }
    return 0;
}

static void set_current_md_status(int status, int flight_mode)
{
    char buf[32];
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
    case CCCI_MD_STA_STOP:
        len = snprintf(buf, sizeof(buf), "stop");
        break;
    case CCCI_MD_STA_FLIGHT_MODE:
        len = snprintf(buf, sizeof(buf), "flightmode");
        break;
    case CCCI_MD_STA_EXCEPTION:
        len = snprintf(buf, sizeof(buf), "exception");
        break;
    default:
        len = snprintf(buf, sizeof(buf), "undefined");
        break;
    }

    update_inf_to_bootprof(buf);
    time(&cur_time);
    store_md_statue(md_id, buf, sizeof(buf));
    notify_md_status(status, flight_mode, buf);
}

/****************************************************************************/
/* modem control message handle function                                                                  */
/*                                                                                                                           */
/****************************************************************************/

static int common_msg_handler(int msg, int resv)
{
    int ret = 1;
    int data = 0;

    switch (msg) {
    case CCCI_MD_MSG_SEND_BATTERY_INFO:
        if(ioctl(system_ch_handle, CCCI_IOC_SEND_BATTERY_INFO, &data))
            CCCI_LOGE("send md battery info fail: %d", errno);
        else
            CCCI_LOGD("send md battery info OK");
        break;
    case CCCI_MD_MSG_CFG_UPDATE:
        CCCI_LOGD("CFG UPDATE: 0x%x(dummy)\n", resv);
        break;
    case CCCI_MD_MSG_RANDOM_PATTERN:
        CCCI_LOGD("CCCI_MD_MSG_RANDOM_PATTERN\n");
        compute_random_pattern((unsigned int*) &data);
        if (0 != ioctl(system_ch_handle, CCCI_IOC_RESET_AP, &data))
            CCCI_LOGE("reset_ap_ioctl failed.\n");
        break;
    case CCCI_MD_MSG_STORE_NVRAM_MD_TYPE:
        CCCI_LOGD("CCCI_MD_MSG_STORE_SIM_MODE\n");
        ioctl(system_ch_handle, CCCI_IOC_GET_MD_TYPE_SAVING, &data);
        CCCI_LOGD("md%d type in kernel(%d)\n", md_id+1, data);
        if (data == get_stored_modem_type_val(md_id))
            CCCI_LOGD("No need to store md type(%d)\n", data);
        else
            store_modem_type_val(md_id, data);
        break;
    case CCCI_MD_MSG_EXCEPTION:
        if (check_curret_md_status(CCCI_MD_STA_EXCEPTION))
            break;
        CCCI_LOGD(" CCCI_MD_MSG_EXCEPTION\n");
        set_current_md_status(CCCI_MD_STA_EXCEPTION, 0);
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
static int is_md_command_finish(int md_id, int stop)
{
    int count = 0;
    char mdstatuspath[20];

    CCCI_LOGI("md_id = %d; mdstatusfd = %d\n", md_id, mdstatusfd);
    if(mdstatusfd < 0) {
        if (md_id == 2)
            snprintf(mdstatuspath, sizeof(mdstatuspath), "/dev/ccci_md3_sta");
        else if(!md_id)
            snprintf(mdstatuspath, sizeof(mdstatuspath), "/dev/ccci_md1_sta");
        else {
            CCCI_LOGE("incorrect md id %d\n", md_id);
            return -1;
        }
        mdstatusfd = open(mdstatuspath, O_RDWR);
        if (mdstatusfd < 0) {
            CCCI_LOGE("open %s fail: %d\n", mdstatuspath, errno);
            return -1;
        }
    }

    do {
        count = read(mdstatusfd, &status_buf, sizeof(struct md_status_event));
        if (count < 0) {
            CCCI_LOGE("read md status fail: %d\n", errno);
            continue;
        }

        if (count > 0) {
            CCCI_LOGD("md status event %d\n", status_buf.event_type);

            if (stop) {
                if (status_buf.event_type == MD_STA_EV_STOP ||
                    status_buf.event_type == MD_STA_EV_EXCEPTION)
                    break;

            } else {
                if (status_buf.event_type == MD_STA_EV_READY ||
                    status_buf.event_type == MD_STA_EV_EXCEPTION)
                    break;

            }

        }

    } while (1);

    return status_buf.event_type;
}

static int trigger_modem_to_run(unsigned int monitor_fd, int flight_mode, int first_boot)
{
    int fd, ret, Mdstatus;
    int current_md_status = MD_STATE_INVALID;
    char data[20];
    char md_id_str[16];

    CCCI_LOGD("trigger modem to run! %d %d\n", flight_mode, first_boot);
    set_current_md_status(CCCI_MD_STA_RESET, flight_mode);
    check_to_restart_md_v2(monitor_fd, first_boot);
    set_md_boot_env_data(md_id, monitor_fd);

    set_current_md_status(CCCI_MD_STA_BOOT_UP, 0);
    ret = ioctl(monitor_fd, CCCI_IOC_DO_START_MD, NULL);
    if (ret) {
        CCCI_LOGE("Fail to send start MD ioctl %d %d\n", ret, errno);
        perror("");
        return -1;
    }

    Mdstatus = is_md_command_finish(md_id, 0);
    if (Mdstatus != MD_STA_EV_READY) {
        CCCI_LOGE("Start MD fail, md status = %d\n", Mdstatus);
        return -1;
    }

start_service:
    ret = ioctl(monitor_fd, CCCI_IOC_GET_MD_STATE, &current_md_status);
    if (ret < 0) {
        CCCI_LOGE("fail to get modem state(%d) %d %d\n", current_md_status, ret, errno);
        return -1;
    }
    if (current_md_status == MD_STATE_READY) {
        set_current_md_status(CCCI_MD_STA_BOOT_READY, 0);
    } else {
        CCCI_LOGI("modem state %d\n", current_md_status);
        return -1;
    }
    CCCI_LOGD("modem boot ready and deamon begin to run!\n");
    return 0;
}

static int stop_modem(unsigned int monitor_fd, int *data)
{
    int ret, Mdstatus;

    ret = ioctl(monitor_fd, CCCI_IOC_DO_STOP_MD, data);
    if (ret) {
        CCCI_LOGE("Fail to send stop MD ioctl %d %d\n", ret, errno);
        perror("");
        return -1;
    }
    Mdstatus = is_md_command_finish(md_id, 1);
    if (Mdstatus != MD_STA_EV_STOP) {
        CCCI_LOGE("Stop MD fail, md status = %d\n", Mdstatus);
        return -1;
    }

    return ret;
}

extern void* monitor_time_update_thread(void* arg);
extern int time_srv_init(void);

int set_md_boot_env_data(int md_id, int fd)
{
    unsigned int data[16] = { 0 };

    data[0] = get_mdlog_boot_mode(md_id);
    data[1] = get_md_sbp_code(md_id, 2);
    data[2] = get_md_dbg_dump_flag(md_id);
    data[3] = get_sbp_subid_setting();
    CCCI_LOGI("set md boot data:mdl=%d sbp=%d dbg_dump=%d sbp_sub=%d\n", data[0], data[1], data[2], data[3]);
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

static void wait_for_rst_lock_free(int fd)
{
    int rst_cnt;
    int retry_cnt;

    if (fd < 0)
        CCCI_LOGD("Open bc port fail, ignore rst lock cnt check\n");
    else {
        retry_cnt = 10;
        while (retry_cnt-- > 0) {
            if (ioctl(fd, CCCI_IOC_GET_HOLD_RST_CNT, &rst_cnt) < 0) {
                CCCI_LOGD("get rst lock cnt fail\n");
                break;
            }
            if (rst_cnt == 0)
                break;
            sleep(1);
            if (ioctl(fd, CCCI_IOC_SHOW_LOCK_USER, &rst_cnt) < 0)
                CCCI_LOGD("show rst lock user fail\n");
        }
    }
}

int main_v2(int input_md_id, int sub_ver)
{
    int ret, fd, tmp=0;
    ssize_t s;
    CCCI_BUFF_T buff;
    char dev_port[32];
    pthread_t tid;
    unsigned int monitor_ch;
    int port_open_retry = MAX_OPEN_PORT_RETRY_NUM;
    int md_type = 0;
    char tmp_buf[4];
    char *lk_info_buf;
    int bc_fd;

    CCCI_LOGI("md_init ver:2.30, sub:%d, %d", sub_ver, input_md_id);

    //Check if input parameter is valid
    md_id = input_md_id - 1;
    monitor_ch = CCCI_MONITOR_CH;

    set_current_md_status(CCCI_MD_STA_INIT, md_id);

    //Configure service and dev port name
    switch(md_id) {
    case 0:
        snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS1));
        break;
    case 1:
        snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS2));
        break;
    case 2:
        snprintf(dev_port, 32, "%s", ccci_get_node_name(USR_CCCI_CTRL, MD_SYS3));
        break;
    default:
        CCCI_LOGE("[Error]Invalid md sys id: %d\n", md_id+1);
        return -1;
    }

    snprintf(md_boot_name, 32, MD_INIT_NEW_FILE); // <== Using new md boot file

    update_inf_to_bootprof("decpyt ready");

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

    system_ch_handle = fd;
    md_image_exist_check(fd, curr_md_id);

    // Get current MD type and update mux daemon name
    //update_service_name(); // no need, 82/92, muxd name will be changed at real time. 

    if(time_srv_init()==0){
        pthread_create(&tid, NULL, monitor_time_update_thread, NULL);
    }

    tmp = parse_sys_env_rat_setting(md_id);
    if(ioctl(fd, CCCI_IOC_RELOAD_MD_TYPE, &tmp) != 0)
        CCCI_LOGD("update modem type to kernel fail: err=%d", errno);

    ret = trigger_modem_to_run(fd, 0, 1);
    if (ret < 0)
        CCCI_LOGE("boot modem fail!\n");

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

    //system_ch_handle = fd;
    bc_fd = open("/dev/ccci_mdx_sta", O_RDWR);
    if (bc_fd < 0) {
        CCCI_LOGE("open '/dev/ccci_mdx_sta' fail: %d\n", errno);
        return -1;
    }
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

            switch (buff.data[1]) {
            case CCCI_MD_MSG_FORCE_STOP_REQUEST:
                if (check_curret_md_status(CCCI_MD_STA_STOP))
                    break;
                if (check_curret_md_status(CCCI_MD_STA_FLIGHT_MODE))
                    break;
                delay_to_reset_md(2);
                tmp = 0;
                ret = stop_modem(fd, &tmp);
                wait_for_rst_lock_free(bc_fd);
                if (!is_meta_mode()) {
                    stop_all_ccci_up_layer_services();
                }
                set_current_md_status(CCCI_MD_STA_STOP, 0);
                break;
            case CCCI_MD_MSG_FORCE_START_REQUEST:
                if (check_curret_md_status(CCCI_MD_STA_BOOT_READY))
                    break;

                trigger_modem_to_run(fd, 0, 0);
                break;

            case CCCI_MD_MSG_FLIGHT_STOP_REQUEST:
                if (check_curret_md_status(CCCI_MD_STA_STOP))
                    break;
                if (check_curret_md_status(CCCI_MD_STA_FLIGHT_MODE))
                    break;

                tmp = 1;
                ret = stop_modem(fd, &tmp);
                set_current_md_status(CCCI_MD_STA_FLIGHT_MODE, 0);
                break;
            case CCCI_MD_MSG_FLIGHT_START_REQUEST:
                if (check_curret_md_status(CCCI_MD_STA_BOOT_READY))
                    break;

                trigger_modem_to_run(fd, 1, 0);
                break;

            case CCCI_MD_MSG_RESET_REQUEST:
                delay_to_reset_md(2);
                tmp = 0;
                ret = stop_modem(fd, &tmp);
                wait_for_rst_lock_free(bc_fd);
                if (!is_meta_mode())
                    stop_all_ccci_up_layer_services();
                set_current_md_status(CCCI_MD_STA_STOP, 0);

                trigger_modem_to_run(fd, 0, 0);
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

    return 0;
}

