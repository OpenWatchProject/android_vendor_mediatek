/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef ATCI_GENERIC_CMD_TABLE_H
#define ATCI_GENERIC_CMD_TABLE_H

#define MAX_AT_COMMAND_LEN 32
#define MAX_AT_RIL_COMMAND_LEN 512
#define MAX_AT_RESPONSE 2048
#define MAX_DATA_SIZE 6144

#include "atci_telephony_cmd.h"
#include "atci_system_cmd.h"
#if defined(ENABLE_GPS_AT_CMD)
    #include "atci_gps_cmd.h"
#endif
#if defined(ENABLE_WLAN_AT_CMD)
    #include "atci_wlan_cmd.h"
#endif
#if defined(ENABLE_MMC_AT_CMD)
    #include "atci_mmc_cmd.h"
#endif
#if defined(ENABLE_CODECRC_AT_CMD)
    #include "atci_code_cmd.h"
#endif
#if defined(ENABLE_BLK_VIBR_AT_CMD)
#include "atci_lcdbacklight_vibrator_cmd.h"
#endif
#if defined(ENABLE_TOUCHPANEL_AT_CMD)
#include "atci_touchpanel_cmd.h"
#endif
#include "atci_pq_cmd.h"
#if defined(ENABLE_CCAP_AT_CMD)
    #include "atci_cct_cmd.h"
#endif
#include "atci_battery_cmd.h"
#include "atci_gm_cmd.h"
#include "atci_touch_cmd.h"

int process_generic_command(char* line, int size);
char* cut_cmd_line(char* line);

typedef struct generic_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                           //The prefix of AT command name
    ATOP_t opType;                                              //The suppport operation type
    //Generic command handler function
    int (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);
} generic_cmd_type;


//Handle those commands in ATCI generic service
static generic_cmd_type generic_cmd_table[] = {
        {"AT%TEST", AT_TEST_OP, pas_ecall_handler},  //Add for test
#if defined(ENABLE_GPS_AT_CMD)
        {"AT%GPS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gps_handler}, //Add for GPS test
        {"AT%GNSS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gnss_handler},
        {"AT%CWGPS", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cw_handler},
#endif
#if defined(ENABLE_MMC_AT_CMD)
        {"AT%MMCCHK",       AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_chk_handler},  //Check the SD/MMC Card
        {"AT%MMCFORMAT",    AT_ACTION_OP                                      , mmc_format_handler},  //Format the SD/MMC Card
        {"AT%MMCTOTALSIZE", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_totalsize_handler},  //Get the total size of SD/MMC Card
        {"AT%MMCUSEDSIZE",  AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_usedsize_handler},  //Get the used size of SD/MMC Card
#endif
#if defined(ENABLE_CODECRC_AT_CMD)
        {"AT%CODECRC",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, code_crc_handler},  //Get the crc code of system image
#endif
#if defined(ENABLE_BLK_VIBR_AT_CMD)
        {"AT%LEDON",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcdbacklight_power_on_cmd_handler},
        {"AT%MOT",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, vibrator_power_off_cmd_handler},
#endif
#if defined(ENABLE_TOUCHPANEL_AT_CMD)
        {"AT%TOUCHFWVER",    AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, touchpanel_fwver_handler},
#endif
        {"AT%FBOOT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_fboot_handler},
        {"AT%RESTART", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_restart_handler},
        {"AT%SAFERESTART", AT_ACTION_OP, system_safe_restart_handler},
        {"AT%IMEI", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei_cmd_handler},
        {"AT%IMEI2", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei2_cmd_handler},
        {"AT%IMEI3", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei3_cmd_handler},
        {"AT%IMEI4", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, misc_imei4_cmd_handler},
        {"AT%PQ", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pq_cmd_handler},
#if defined(ENABLE_CCAP_AT_CMD)
        {"AT%CCT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, cct_cmd_handler},
#endif
        {"AT+CBC", AT_ACTION_OP | AT_TEST_OP, cbc_cmd_handler},
        {"AT%CAR", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, car_tune_cmd_handler},
        {"AT%NAFG", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, nafg_cmd_handler},
        {"AT+CBKLT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcd_backlight_cmd_handler},
        {"AT+CTSA", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, touch_cmd_handler},
    };
#endif
