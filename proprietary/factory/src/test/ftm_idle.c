/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef FACTORY_C2K_SUPPORT
#ifndef MTK_ECCCI_C2K
#include <c2kutils.h>
#endif
#endif
#include <sys/mount.h>
#include <sys/statfs.h>
#include <termios.h>
#include <linux/fb.h>

#include <cutils/properties.h>

#include "ftm_suspend.h"

#include "common.h"
#include "miniui.h"
#include "utils.h"
#include "ftm.h"

#include "hardware/ccci_intf.h"

#ifdef FEATURE_FTM_IDLE

#define TAG "[IDLE] "

#define mod_to_idle(p) (struct ftm_idle *)((char *)(p) + sizeof(struct ftm_module))

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_ENTER_DEEP_FLIGHT      _IO(CCCI_IOC_MAGIC, 14) /* RILD // factory */
#define CCCI_IOC_LEAVE_DEEP_FLIGHT      _IO(CCCI_IOC_MAGIC, 15) /* RILD // factory */
#define CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED      _IO(CCCI_IOC_MAGIC, 123) /* RILD // factory */
#define CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED      _IO(CCCI_IOC_MAGIC, 124) /* RILD // factory */

// used by sendEsuo
#define HALT_TIME         100000
#define WAIT_AT_COMMAND_TIME 3
#define BUF_SIZE 128

struct ftm_idle
{
    struct ftm_module *mod;
    struct textview tv;
    text_t title;
};

static char ccci_md1_dev[32];

static int fd_atcmd2;
static char ccci_md2_dev[32];

#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_FTM_DT_VIA_SUPPORT)
static int fd_atcmd_dt;
#endif

#if !defined(EVDO_FTM_DT_VIA_SUPPORT) && defined(FACTORY_C2K_SUPPORT)
static int fd3;
#endif

static int fd_ioctl;
static int fd_ioctlmd2;
static char ccci_md1_power_ioctl_dev[32];
static char ccci_md2_power_ioctl_dev[32];

extern int usb_com_port;
extern int usb_plug_in;
extern int idle_current_done;
extern int com_type;
extern int uart_exit;

typedef enum
{
	ENTER_IDLE = 0,
	EXIT_IDLE,
}IDLE_STATE;

static int idle_key_handler()
{
    return 0;
}

static int is_md1_enable()
{
    char value[100] = {0};
    property_get("ro.vendor.mtk_md1_support",value,"0");
    LOGD(TAG "is_md1_enable value = %s",value);
    if (atoi(value) > 0) {
       return 1;
    }
    return 0;
}

static int is_md2_enable()
{
    char value[100] = {0};
    property_get("ro.vendor.mtk_md2_support",value,"0");
    LOGD(TAG "is_md2_enable value = %s",value);
    if (atoi(value) > 0) {
       return 1;
    }
    return 0;
}

void sendEsuo(const int fd, int value, int wait_echo) {
    char *pRsp = NULL;
    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
    sprintf(cmd_buf, "AT+ESUO=%d\r\n", value);
    write(fd, cmd_buf, strlen(cmd_buf));
    LOGD(TAG "Send AT+ESUO=%d\n", value);
    usleep(HALT_TIME);
    if (-1 == wait4_readAck(fd, WAIT_AT_COMMAND_TIME, rsp_buf)) {
        LOGE(TAG "read ack time out\n");
        goto error;
    }
    LOGD(TAG "@@wait_rsp %d, %s\n", wait_echo, rsp_buf);
    while (wait_echo) {
        pRsp = strstr(rsp_buf, "OK");
        if (strncmp(rsp_buf, "+ESUO", 5) == 0) {
            LOGD(TAG "sendEsuo got echo!");
            break;
        } else if (pRsp != NULL) {
            LOGD(TAG "sendEsuo got OK!");
            break;
        }
        pRsp = NULL;
        if (-1 == wait4_readAck(fd, WAIT_AT_COMMAND_TIME, rsp_buf)) {
            LOGE(TAG "read ack time out\n");
            goto error;
        }
        LOGD(TAG "@@read rsp for Esuo, %s\n", rsp_buf);
    }
    LOGD(TAG "sendEsuo end!\n");
    return;
error:
    return;
}

int check3GSwitchStatus(const int fd) {
    // to detect 3G capability
    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
    int sim_switch_flag = 1; // 1 -> 3G on SIM1, 2 -> 3G on SIM2, 3 -> 3G on SIM3

    tcflush(fd, TCIOFLUSH);   //ALPS01194291: clear buffer to avoid wrong data
    strncpy(cmd_buf, "AT+ES3G?\r\n", sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';
    write(fd, cmd_buf, strlen(cmd_buf));
    usleep(HALT_TIME);
    LOGD(TAG "Send AT+ES3G?\n");

    const char *TOK_ES3G = "+ES3G: ";
    char *p_es3g = NULL;
    while (p_es3g == NULL) {
        memset(cmd_buf, 0, BUF_SIZE);
        if (-1 == wait4_readAck(fd, WAIT_AT_COMMAND_TIME, rsp_buf)) {
            LOGE(TAG "AT+ES3G? read ack time out\n");
            break;
        }
        LOGD(TAG "3G Capability: %s\n", cmd_buf);
        p_es3g = strstr(cmd_buf, TOK_ES3G);
        if(p_es3g) {
            p_es3g += strlen(TOK_ES3G);
            if('2' == *p_es3g) {
                sim_switch_flag = 2;
            } else if ('4' == *p_es3g) {
                sim_switch_flag = 3;
            }
            LOGD(TAG "3G capability is on SIM %d\n", (sim_switch_flag));
            break;
        } else {
            LOGD(TAG "No response for AT+ES3G? %s\n", p_es3g);
        }
    }

    return sim_switch_flag;
}

void handleLPlusGMd1() {
    char value[100] = {0};
    int fd_atcmd = -1;
    property_get("persist.vendor.radio.mtk_ps2_rat",value,"G");
    LOGD(TAG "mtk_ps2_rat = %s",value);
    if (strcmp(value, "G") == 0) {
        char ccci_md1_dev[32];
        snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
        if (-1 == fd_atcmd) {
            LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
            fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
            if (-1 == fd_atcmd) {
                LOGD(TAG "Fail to open CCCI interface\n");
                return;
            }
        }
        int sim_switch_flag = check3GSwitchStatus(fd_atcmd);
        LOGD(TAG "sim_switch_flag:%d\n", sim_switch_flag);
        if (sim_switch_flag == 1) {
            sendEsuo(fd_atcmd, 5, 0);
        }
        LOGD(TAG "send ERAT=0 to ps2\n");
        if (send_at (fd_atcmd, "AT+ERAT=0\r\n")) {
            return;
        }
        wait4_ack(fd_atcmd, NULL, 15000);
        LOGD(TAG "send ERAT=0 to ps2 done\n");
        if (sim_switch_flag == 1) {
            sendEsuo(fd_atcmd, 4, 0);
        }
        closeDevice(fd_atcmd);
        fd_atcmd = -1;
    }
}

bool ifSupportEpofReviseNewVersion ()
{
    int i = 0;
    int fd_atcmd = -1;
    bool ifSupportNewVersion = false;
    snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
    fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
    if (-1 == fd_atcmd) {
        LOGD(TAG "Fail to open CCCI interface\n");
        return false;
    }
    for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup
    do
    {
        send_at (fd_atcmd, "AT\r\n");
    } while (wait4_ack (fd_atcmd, NULL, 300));
    LOGD(TAG "[AT]AT+EPOF=?\n");
    if (send_at (fd_atcmd, "AT+EPOF=?\r\n")) {
        closeDevice(fd_atcmd);
        fd_atcmd = -1;
        return false;
    }
    if (wait4_ack (fd_atcmd, "+EPOF: 1", 3000)) {
        closeDevice(fd_atcmd);
        fd_atcmd = -1;
        return false;
    } else {
        ifSupportNewVersion = true;
    }
    closeDevice(fd_atcmd);
    return ifSupportNewVersion;
}

int set_md_power_and_flight_mode (bool md_power)
{
    int fd_atcmd = -1;
    snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
    fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
    if (-1 == fd_atcmd) {
        LOGD(TAG "Fail to open CCCI interface\n");
        return -1;
    }
    snprintf(ccci_md1_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 Power IOCTL dev node: %s\n", ccci_md1_power_ioctl_dev);
    fd_ioctl = openDeviceWithDeviceName(ccci_md1_power_ioctl_dev);
    if (-1 == fd_ioctl) {
        LOGD(TAG "Fail to open CCCI IOCTL interface\n");
        closeDevice(fd_atcmd);
        return -1;
    }

    if (md_power) {
        LOGD(TAG "[AT]CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED \n");
        int ret_ioctl_val = ioctl(fd_ioctl, CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
        closeDevice(fd_atcmd);
        fd_atcmd = -1;
        closeDevice(fd_ioctl);
        return 0;
    } else {
        LOGD(TAG "[AT]Enable Sleep Mode:\n");
        if (send_at (fd_atcmd, "AT+ESLP=1\r\n")) goto err;
        if (wait4_ack (fd_atcmd, NULL, 3000)) goto err;

        LOGD(TAG "[AT]Power OFF Modem:\n");
        if (send_at (fd_atcmd, "AT+EFUN=0\r\n")) goto err;
        wait4_ack (fd_atcmd, NULL, 15000);
        if (send_at (fd_atcmd, "AT+EPOF\r\n")) goto err;
        wait4_ack (fd_atcmd, NULL, 10000);
        LOGD(TAG "[AT]CCCI_IOC_ENTER_DEEP_FLIGHT \n");
        int ret_ioctl_val = ioctl(fd_ioctl, CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
        closeDevice(fd_atcmd);
        closeDevice(fd_ioctl);
        return 0;
    }
err:
    closeDevice(fd_atcmd);
    closeDevice(fd_ioctl);
    return -1;
}

#if defined(FACTORY_C2K_SUPPORT ) && !defined(EVDO_FTM_DT_VIA_SUPPORT)
extern void C2KEnterFlightMode();
extern void C2KLeaveFlightMode();

static int wait_SDIO_ready()
{
    int ret = 0;
    char value[100] = {0};
    while(1) {
       usleep(5000);
       ret = property_get("vendor.net.cdma.mdmstat", value, NULL);
       LOGD(TAG "wait_SDIO_ready ret = %d",ret);
       LOGD(TAG "wait_SDIO_ready value = %s",value);
       if (ret > 0) {
           if (!strncmp(value,"ready",5)) {
               LOGD(TAG "Get SDIO ready");
               break;
           }
       }
    }
    return 1;
}

int get_ccci_path_md3(int modem_index,char * path)
{
    int idx[4] = {1,2,5,4};
    int md_sys[4] = {MD_SYS1, MD_SYS2, MD_SYS5, MD_SYS3};
    LOGD(TAG "modem_index:%d",modem_index);
    int i = 0;

    if(is_support_modem(idx[modem_index]))
    {
       if(modem_index == 3){
            LOGD(TAG "prepare to get md3 node");
            snprintf(path, 32, "%s", ccci_get_node_name(USR_C2K_AT, (CCCI_MD)md_sys[modem_index]));
            LOGD(TAG "got md3 node %s", path);
        } else {
           snprintf(path, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, (CCCI_MD)md_sys[modem_index]));
        }
       LOGD(TAG "CCCI Path:%s",path);
       return 1 ;
    }
    else
    {
       return 0 ;
    }
}

void EnterFlightModeForSrlte()
{
    char ret[32];
    int fd = -1;
    ssize_t ioctl_ret;
    snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
    LOGD(TAG "IOCTL node name:%s",ret);
    fd = open(ret, O_RDWR);
    if(fd >= 0) {
        LOGE("enter flightmode");
        ioctl_ret = ioctl(fd, CCCI_IOC_ENTER_DEEP_FLIGHT);
        if(ioctl_ret < 0){
        LOGE("Fail to enter flight mode.\n");
        }
        close(fd);
    } else {
        LOGE("open ccci_get_node_name USR_RILD_IOCTL failed");
    }
}

void LeaveFlightModeForSrlte()
{
    char ret[32];
    int fd = -1;
    ssize_t ioctl_ret;
    snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
    LOGD(TAG "LeaveFlightModeForSrlte IOCTL node name:%s",ret);
    fd = open(ret, O_RDWR);
    if(fd >= 0) {
        ioctl_ret = ioctl(fd, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        if(ioctl_ret < 0){
        LOGE("Fail to leave flight mode.\n");
        }
        close(fd);
    } else {
        LOGE("open ccci_get_node_name USR_RILD_IOCTL failed");
    }
}

static int PowerOffModem3(int fd) {
    LOGD(TAG "[C2K]AT polling first:\n");
    send_at(fd, "ATE0Q0V1\r\n");
    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 3000));


    LOGD(TAG "[C2K]Send AT+EPOF to reboot modem \n");
    if (send_at(fd, "AT+EPOF\r\n")) goto err;
    wait4_ack(fd, NULL, 5000);
err:
    return -1;
}

static void waitVPONAndEnterFlightMode(int fd) {
    LOGD(TAG "[C2K]Wait for +VPON:0, C2K modem turn off:\n");
    wait4_ack (fd, "+VPON:0", 10000);
    closeDevice(fd); // close fd before calling flightmode API.
    LOGD(TAG "[C2K]close fd3\n");
    #ifdef MTK_ECCCI_C2K
        EnterFlightModeForSrlte();
    #else
        C2KEnterFlightMode();
    #endif
}

#endif

static int power_off_md()
{
    int i = 0;
    char *atDevPath3 = NULL;
    int fd_atcmd = -1;
    snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
    fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
    if (-1 == fd_atcmd) {
        LOGD(TAG "Fail to open CCCI interface\n");
        return -1;
    }

    /* Make MD into flight mode */
    #if !defined(EVDO_FTM_DT_VIA_SUPPORT)
    #ifdef FACTORY_C2K_SUPPORT
        LOGD(TAG "[C2K]Go to open C2K modem fd3!");
        wait_SDIO_ready();
        // wait modem3 ready
        #ifdef MTK_ECCCI_C2K
            atDevPath3 = (char*)malloc(sizeof(char)*32);
            if (atDevPath3 == NULL) {
                closeDevice(fd_atcmd);
                return -1;
            }
            if (get_ccci_path_md3(3, atDevPath3) == 0) {
            LOGD(TAG "Can't get CCCI path!");
            }
        #else
            atDevPath3 = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_AT);
            if (atDevPath3 == NULL) {
                closeDevice(fd_atcmd);
                return -1;
            }
        #endif
        LOGD(TAG "[C2K]Go to open modem3 fd3 atDevPath3 = %s", atDevPath3);
        fd3 = openDeviceWithDeviceName(atDevPath3);
        if (fd3 < 0) {
            LOGD(TAG "[C2K]Fail to open ttySDIO4\n");
            if (atDevPath3 != NULL) {
                free(atDevPath3);
                atDevPath3 = NULL;
            }
            closeDevice(fd_atcmd);
            return -1;
        }
        LOGD(TAG "[C2K]OK to open atDevPath3\n");
        for (i = 0; i < 30; i++) usleep(50000);
        // sleep 1.5s wait for modem bootup
        PowerOffModem3(fd3);
    #endif
    #endif

    if (is_md1_enable() > 0) {
        for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup
        snprintf(ccci_md1_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS1));
        LOGD(TAG "Open CCCI MD1 Power IOCTL dev node: %s\n", ccci_md1_power_ioctl_dev);
        fd_ioctl = openDeviceWithDeviceName(ccci_md1_power_ioctl_dev);
        if (-1 == fd_ioctl) {
            LOGD(TAG "Fail to open CCCI IOCTL interface\n");
            if (atDevPath3 != NULL) {
                free(atDevPath3);
                atDevPath3 = NULL;
            }
            closeDevice(fd_atcmd);
            return -1;
        }

        ExitFlightMode_PowerOffModem(fd_atcmd,fd_ioctl,FALSE);
    }

    closeDevice(fd_atcmd);
    closeDevice(fd_ioctl);
        /*
    modem requred power off flow:
    1.send EPOF to MD3
    2.send EPOF to MD1
    3.MD1 send L4C_C2K_EFUN_STATE_IND to MD3
    4.MD3 begin shut down and send VPON
    5.MD1 received VPON and shut down
    */
    #ifdef FACTORY_C2K_SUPPORT
        waitVPONAndEnterFlightMode(fd3);
    #endif
    
    if (is_md2_enable() > 0) {
        snprintf(ccci_md2_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS2));
        LOGD(TAG "Open CCCI MD2 dev node: %s\n", ccci_md2_dev);
        fd_atcmd2 = openDeviceWithDeviceName(ccci_md2_dev);
        if (-1 == fd_atcmd2) {
            LOGD(TAG "Fail to open MD2 CCCI interface\n");
            if (atDevPath3 != NULL) {
                free(atDevPath3);
                atDevPath3 = NULL;
            }
            return -1;
        }
        for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup
        snprintf(ccci_md2_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS2));
        LOGD(TAG "Open CCCI MD2 Power IOCTL dev node: %s\n", ccci_md2_power_ioctl_dev);
        fd_ioctlmd2 = openDeviceWithDeviceName(ccci_md2_power_ioctl_dev);
        if (-1 == fd_ioctlmd2) {
            LOGD(TAG "Fail to open CCCI MD2 IOCTL interface\n");
            if (atDevPath3 != NULL) {
                free(atDevPath3);
                atDevPath3 = NULL;
            }
            return -1;
        }
        ExitFlightMode_PowerOffModem(fd_atcmd2,fd_ioctlmd2,FALSE);
    }
    #if defined(MTK_DT_SUPPORT) && defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_FTM_DT_VIA_SUPPORT) && !defined (FACTORY_C2K_SUPPORT)
            #if defined(PURE_AP_USE_EXTERNAL_MODEM)
                fd_atcmd_dt = openDeviceWithDeviceName(CCCI_MODEM_MT8135);
            #else
                fd_atcmd_dt= openDeviceWithDeviceName(CCCI_MODEM_MT6252);
            #endif

            if (-1 == fd_atcmd_dt) {
                LOGD(TAG "Fail to open CCCI interface\n");
                if (atDevPath3 != NULL) {
                    free(atDevPath3);
                    atDevPath3 = NULL;
            }
                return -1;
            }

            #if defined(PURE_AP_USE_EXTERNAL_MODEM)
                initTermIO(fd_atcmd_dt, 5);
            #endif

            for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup

            ExitFlightMode_idle(fd_atcmd_dt, FALSE);
    #endif
    if (atDevPath3 != NULL) {
        free(atDevPath3);
        atDevPath3 = NULL;
    }
    return 0;
}

static int power_on_md()
{
    int i = 0;

    int fd_atcmd = -1;
    snprintf(ccci_md1_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 dev node: %s\n", ccci_md1_dev);
    fd_atcmd = openDeviceWithDeviceName(ccci_md1_dev);
    if (-1 == fd_atcmd) {
        LOGD(TAG "Fail to open CCCI interface\n");
        return false;
    }

    int fd_ioctl = -1;
    snprintf(ccci_md1_power_ioctl_dev, 32, "%s", ccci_get_node_name(USR_FACTORY_IDLE_IOCTL, MD_SYS1));
    LOGD(TAG "Open CCCI MD1 Power IOCTL dev node: %s\n", ccci_md1_power_ioctl_dev);
    fd_ioctl = openDeviceWithDeviceName(ccci_md1_power_ioctl_dev);
    if (-1 == fd_ioctl) {
        LOGD(TAG "Fail to open CCCI ioctl interface\n");
        closeDevice(fd_atcmd);
        return false;
    }
    /* Make MD exit flight mode */
    if (is_md1_enable() > 0) {
        ExitFlightMode_PowerOffModem(fd_atcmd,fd_ioctl,TRUE);
        closeDevice(fd_atcmd);
        closeDevice(fd_ioctl);
    }
    if (is_md2_enable() > 0) {
        ExitFlightMode_PowerOffModem(fd_atcmd2,fd_ioctlmd2,TRUE);
        closeDevice(fd_ioctlmd2);
        closeDevice(fd_atcmd2);
    }
    #if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_FTM_DT_VIA_SUPPORT) && !defined (FACTORY_C2K_SUPPORT)
            closeDevice(fd_atcmd_dt);
    #endif
#if defined(FACTORY_C2K_SUPPORT) && !defined(EVDO_FTM_DT_VIA_SUPPORT)
         #ifdef MTK_ECCCI_C2K
            LeaveFlightModeForSrlte();
         #else
           C2KLeaveFlightMode();
         #endif
#endif
    for (i = 0; i < 30; i++) usleep(100000); // sleep 1s wait for modem bootup
    return 0;
}

static int Enter_Exit_Idle(IDLE_STATE idle_state)
{
    int fd_backlight = -1, fd_mdm = -1, fd_fb = -1;
    int ret = 0, err = 0;
    char *s_backlight_on = "102";
    char *s_backlight_off = "0";
    char *s_mdm_txpwr_disable = "0";

    char filename[32] = {0};
    static bool isSupportEpofRevise = false;
    if(ENTER_IDLE == idle_state)
    {
		LOGD(TAG "%s: Enter idle\n", __FUNCTION__);
		/* Turn off backlight */
		property_set("vendor.debug.factory.idle_state", "backlight_off_0");
		LOGD(TAG "%s: backlight_off_0\n", __FUNCTION__);
		fd_backlight = open("/sys/class/leds/lcd-backlight/brightness", O_RDWR, 0);
		if (fd_backlight < 0) {
			LOGD(TAG "%s: cannot open /sys/class/leds/lcd-backlight/brightness\n", __FUNCTION__);
			return -1;
		}
		ret = write(fd_backlight, s_backlight_off, strlen(s_backlight_off));
		close(fd_backlight);
		LOGD(TAG "%s: backlight_off_1\n", __FUNCTION__);
		property_set("vendor.debug.factory.idle_state", "backlight_off_1");

		/* make fb blank */
		property_set("vendor.debug.factory.idle_state", "display_blank_0");
		LOGD(TAG "%s: display_blank_0\n", __FUNCTION__);
		snprintf(filename, sizeof(filename), "/dev/graphics/fb0");
		fd_fb = open(filename, O_RDWR);
		if (fd_fb < 0) {
			LOGD("Failed to open fb0 device: %s", strerror(errno));
		}
		err = ioctl(fd_fb, FBIOBLANK, FB_BLANK_POWERDOWN);
		if (err < 0) {
			LOGD("Failed to blank fb0 device: %s", strerror(errno));
		}
		close(fd_fb);
		LOGD(TAG "%s: display_blank_1\n", __FUNCTION__);
		property_set("vendor.debug.factory.idle_state", "display_blank_1");
        /* set rat to 2G only if L/W/G+G project */
        if (is_md1_enable() > 0) {
            handleLPlusGMd1();
        }
		/* power off md */
		property_set("vendor.debug.factory.idle_state", "md_power_off_0");
		LOGD("power_off_md (start)");
        isSupportEpofRevise = ifSupportEpofReviseNewVersion();
		if (isSupportEpofRevise) {
			if (set_md_power_and_flight_mode(FALSE) < 0) {
				LOGD(TAG "%s: fail to power off md\n", __FUNCTION__);
			}
		}else {
			if (power_off_md() < 0) {
				LOGD(TAG "%s: fail to power off md\n", __FUNCTION__);
			}
		}
		LOGD("power_off_md (end)");
		property_set("vendor.debug.factory.idle_state", "md_power_off_1");

		/* Turn off thermal query MD TXPWR function */
		property_set("vendor.debug.factory.idle_state", "md_txpwr_off_0");
		LOGD(TAG "%s: md_txpwr_off_0\n", __FUNCTION__);
		fd_mdm = open("/proc/mtk_mdm_txpwr/txpwr_sw", O_RDWR, 0);
		if (fd_mdm < 0) {
			LOGD(TAG "%s: cannot open /proc/mtk_mdm_txpwr/txpwr_sw, not support\n", __FUNCTION__);
		}
		else
		{
		    ret = write(fd_mdm, s_mdm_txpwr_disable, strlen(s_mdm_txpwr_disable));
		    close(fd_mdm);
		    LOGD(TAG "%s: md_txpwr_off_1\n", __FUNCTION__);
		    property_set("vendor.debug.factory.idle_state", "md_txpwr_off_1");
		}
		/* Enable autosuspend */
		property_set("vendor.debug.factory.idle_state", "ap_enter_suspend_0");
		LOGD(TAG "%s: ap_enter_suspend_0", __FUNCTION__);
		enableAutoSuspend();
		LOGD(TAG "%s: ap_enter_suspend_1", __FUNCTION__);
		property_set("vendor.debug.factory.idle_state", "ap_enter_suspend_1");
    }
    else
    {
		LOGD(TAG "%s: Exit idle\n", __FUNCTION__);
		/* Disable autosuspend */
		disableAutoSuspend();
		LOGD(TAG "%s: disable autosuspend\n", __FUNCTION__);
		/* power on md */
		property_set("vendor.debug.factory.idle_state", "md_power_on_0");
		LOGD("power_on_md (start)");
		if (isSupportEpofRevise) {
			if (set_md_power_and_flight_mode(TRUE) < 0) {
				LOGD(TAG "%s: fail to power on md\n", __FUNCTION__);
			}
		} else {
			if (power_on_md() < 0) {
					LOGD(TAG "%s: fail to power on md\n", __FUNCTION__);
			}
		}
		LOGD("power_on_md (end)");
		property_set("vendor.debug.factory.idle_state", "md_power_on_1");

		/* make fb unblank */
		property_set("vendor.debug.factory.idle_state", "display_unblank_0");
		LOGD(TAG "%s: display_unblank_0", __FUNCTION__);
		snprintf(filename, sizeof(filename), "/dev/graphics/fb0");
		fd_fb = open(filename, O_RDWR);
		if (fd_fb < 0) {
			LOGD("Failed to open fb0 device: %s", strerror(errno));
		}
		err = ioctl(fd_fb, FBIOBLANK, FB_BLANK_UNBLANK);
		if (err < 0) {
			LOGD("Failed to unblank fb0 device: %s", strerror(errno));
		}
		close(fd_fb);
		LOGD(TAG "%s: display_unblank_1", __FUNCTION__);
		property_set("vendor.debug.factory.idle_state", "display_unblank_1");

		/* Turn on backlight */
		property_set("vendor.debug.factory.idle_state", "backlight_on_0");
		LOGD(TAG "%s: backlight_on_0", __FUNCTION__);
		fd_backlight = open("/sys/class/leds/lcd-backlight/brightness", O_RDWR, 0);
		if (fd_backlight < 0) {
			LOGD(TAG "%s: cannot open /sys/class/leds/lcd-backlight/brightness\n", __FUNCTION__);
			return -1;
		}
		ret = write(fd_backlight, s_backlight_on, strlen(s_backlight_on));
		close(fd_backlight);
		LOGD(TAG "%s: backlight_on_1", __FUNCTION__);
            property_set("vendor.debug.factory.idle_state", "backlight_on_1");
	}
	return 0;
}

int idle_entry(struct ftm_param *param, void *priv)
{
    struct ftm_idle *idle = (struct ftm_idle *)priv;
    struct textview *tv = &idle->tv;

    int ret = 0, key = 0, write_len = 0;
    char *close_com_port = "close com port!";
    char *enter_idle = "Enter idle";
    char *exit_idle = "Exit idle";

    LOGD(TAG "%s: idle_entry\n", __FUNCTION__);

    init_text(&idle->title, param->name, COLOR_YELLOW);

    ui_init_textview(tv, idle_key_handler, (void*)idle);
    tv->set_title(tv, &idle->title);

    ret = Enter_Exit_Idle(ENTER_IDLE);
    if(-1 == ret)
    {
		idle->mod->test_result = FTM_TEST_FAIL;
		return -1;
	}
    if (param->test_type == FTM_MANUAL_ITEM) {
		while (1) {
			key = ui_wait_phisical_key();
			LOGD(TAG "%s: %d\n", __FUNCTION__, key);

			/* Disable autosuspend */
			//autosuspend_disable();
			//LOGD(TAG "%s: disable autosuspend\n", __FUNCTION__);

            ret = Enter_Exit_Idle(EXIT_IDLE); //Exit idle mode
            if(-1 == ret)
            {
		        idle->mod->test_result = FTM_TEST_FAIL;
				return -1;
			}
			else
			{
			    break;
			}
		}
	} else {
		if(1 == com_type) //UART COM port.
		{	
			write_len = write(usb_com_port, enter_idle, strlen(enter_idle));
			LOGD(TAG "after write data to pc\n");
			if (write_len != (int)strlen(enter_idle)) {
				LOGD(TAG "write data to pc fail\n");
			}
			
			LOGD(TAG "Enter idle via UART, wait powerkey pressed down to exit idle\n");
			
			ui_wait_phisical_key(); //Wait for powerkey pressed down 			
	        
			ret = Enter_Exit_Idle(EXIT_IDLE); //Exit idle mode
			
			if(-1 == ret)
			{
				idle->mod->test_result = FTM_TEST_FAIL;
				return -1;
			}
			else
			{
				write_len = write(usb_com_port, exit_idle, strlen(exit_idle));
				LOGD(TAG "after write data to pc\n");
				if (write_len != (int)strlen(exit_idle)) {
					LOGD(TAG "write data to pc fail\n");
				}
			}

	    }
	    else if(2 == com_type)
	    {
			write_len = write(usb_com_port, close_com_port, strlen(close_com_port));
			LOGD(TAG "after write data to pc\n");
			if (write_len != (int)strlen(close_com_port)) {
				LOGD(TAG "write data to pc fail\n");
			}
			
			close_usb();

			while(is_usb_state_plugin()) {
				sleep(1);
			}

			sleep(2);

			while(!is_usb_state_plugin()) {
				sleep(1);
			}
			
			usb_plug_in = 1;
			idle_current_done = 1;
			
            ret = Enter_Exit_Idle(EXIT_IDLE); //Exit idle mode
            if(-1 == ret)
            {
		        idle->mod->test_result = FTM_TEST_FAIL;
				return -1;
			}	
		}
		else
		{
			LOGD(TAG "Invalid com type : %d\n", com_type);
			idle->mod->test_result = FTM_TEST_FAIL;
		    return -1;
		}
	}

    idle->mod->test_result = FTM_TEST_PASS;
    return 0;
}

int idle_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct ftm_idle *idle;

    LOGD(TAG "idle_init\n");

    mod = ftm_alloc(ITEM_IDLE, sizeof(struct ftm_idle));
    if (!mod)
        return -ENOMEM;

    idle = mod_to_idle(mod);
    idle->mod = mod;

    ret = ftm_register(mod, idle_entry, (void*) idle);
    if (ret) {
        LOGD(TAG "register IDLE failed (%d)\n", ret);
        return ret;
    }

    return 0;
}

#endif
