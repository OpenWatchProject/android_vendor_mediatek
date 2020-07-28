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

#ifndef ATCID_CUST_CMD_PROCESS_H
#define ATCID_CUST_CMD_PROCESS_H

#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd.h"
#include "atcid_cust_cmd_platform.h"

typedef struct custom_cmd_type
{
    //The prefix of AT command name
    char cmdName[MAX_AT_COMMAND_LEN];
    ATOP_t opType;
    //Command Handler function in case of no IOControl support
    ATRESPONSE_t (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);
} customcmd_type;

typedef struct ril_mmi_cmd_type
{
    //The prefix of AT command name
    char cmdName[MAX_AT_COMMAND_LEN];
    ATOP_t opType;
} rilmmicmd_type;

static customcmd_type custom_cmd_table[] = {
        {"ATQ0V1E", AT_BASIC_OP, pas_echo_handler},                 //Add for device echo
        {"ATE0", AT_BASIC_OP, pas_echo_handler},                    //Add for device echo
        {"ATE1", AT_BASIC_OP, pas_echo_handler},                    //Add for device echo
        {"AT#CLS", AT_TEST_OP, pas_modem_handler},                  //Add for device manager issue
        {"AT+GCI", AT_READ_OP | AT_TEST_OP, pas_modem_handler},     //Add for device manager issue
        {"AT+CCLK", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cclk_handler},
        {"AT+CSDF", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_csdf_handler},
        {"AT+ESUO", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_esuo_handler},
        {"AT+ATCI", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_atci_handler},
        {"AT%REBOOT", AT_ACTION_OP, pas_reboot_handler},
        {"AT^WIENABLE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wienable_handler},
        {"AT^WIMODE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wimode_handler},
        {"AT^WIBAND", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wiband_handler},
        {"AT^WIFREQ", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wifreq_handler},
        {"AT^WIDATARATE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_widatarate_handler},
        {"AT^WIPOW", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wipow_handler},
        {"AT^WITXPOW", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witxpow_handler},
        {"AT^WITX", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witx_handler},
        {"AT^WIRX", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirx_handler},
        {"AT^WIRPCKG", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirpckg_handler},
        {"AT^WIRSSI", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirssi_handler},
        {"AT^WIGI", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wigi_handler},
        {"AT^WIPREAMBLE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wipreamble_handler},
        {"AT^WITXPKTLEN", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witxpktlen_handler},
        {"AT^WITXPKTCNT", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witxpktcnt_handler},
        {"AT^WITXPKTINTERVAL", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witxpktinterval_handler},
        {"AT^WICHBANDWIDTH", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wichbandwidth_handler},
        {"AT^WIDATABANDWIDTH", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_widatabandwidth_handler},
        {"AT^WIPRESET", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wiprimarychset_handler},
        {"AT^WITXDATARATE", AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_witxdatarate_handler},
        {"AT^WIRXSTART", AT_ACTION_OP | AT_READ_OP | AT_SET_OP, pas_wirxstart_handler},
        {"AT^WITESTSET", AT_ACTION_OP | AT_READ_OP | AT_SET_OP, pas_witestset_handler},
        {"AT%WLAN", AT_ACTION_OP | AT_READ_OP | AT_SET_OP, pas_witestmode_handler},
        {"AT%WLANT", AT_ACTION_OP | AT_READ_OP | AT_SET_OP, pas_witx2_handler},
        {"AT%WLANR", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wirx2_handler},
        {"AT%MAC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wimac_handler},
        {"AT%MACCK", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_wimacck_handler},
        {"AT@USBCONFIG", AT_SET_OP | AT_READ_OP, pas_usbconfig_handler},
        {"AT%EURC", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_eurc_handler},
        {"AT+VZWATCICFG", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_vzwatcicfg_handler},
        {"AT+CMEC", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cmec_handler},
        {"AT^BSN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, bsn_handler},
        {"AT^SN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, sn_handler},
        {"AT^GETEMMC", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, get_emmc_handler},
        {"AT^GETRAM", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, get_ram_handler},
        {"AT^SWVER", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, swver_handler},
        {"AT^FACINFO", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, fac_info_handler},
        {"AT^DEVBOOTSTATE", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, dev_boot_state_handler},
        {"AT^GETPRODUCTINFO", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, product_info_handler},
        {"AT^VENDORCOUNTRY", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, vendor_country_handler},
        {"AT^DEVMODEL", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, dev_model_handler},
        {"AT^SDSTATE", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, sd_state_handler},
        {"AT^TBATVOLT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, battery_volt_handler},
        {"AT^TBATTEMP", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, battery_temp_handler},
        {"AT^POWERDOWN", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, power_down_handler},
        {"AT^PHYNUM", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, phy_num_handler},
        {"AT^NVBACKUP", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, nv_backup_handler},
        {"AT^INFORBU", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, nv_backup_handler},
        {"AT^METAFLAG", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, meta_flag_handler},
    };

static rilmmicmd_type ril_mmi_cmd_table[] = {
        {"AT+CPIN", AT_SET_OP | AT_READ_OP | AT_TEST_OP | AT_SET_MMI_OP},
    };

#endif
