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

#ifndef ATCID_CUST_CMD_PLATFORM_H
#define ATCID_CUST_CMD_PLATFORM_H

ATRESPONSE_t pas_wienable_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wimode_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wiband_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wifreq_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_widatarate_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wipow_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witxpow_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witx_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wirx_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wirpckg_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wirssi_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wigi_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wipreamble_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witxpktlen_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witxpktcnt_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witxpktinterval_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wichbandwidth_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_widatabandwidth_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wiprimarychset_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witxdatarate_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wirxstart_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witestset_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witestmode_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_witx2_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wirx2_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wimac_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t pas_wimacck_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t bsn_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t sn_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t get_emmc_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t get_ram_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t swver_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t fac_info_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t dev_boot_state_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t product_info_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t vendor_country_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t dev_model_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t sd_state_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t battery_volt_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t battery_temp_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t phy_num_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t power_down_handler(char* cmdline, ATOP_t at_op, char* response);
ATRESPONSE_t nv_backup_handler(char* cmdline, ATOP_t opType, char* response);
ATRESPONSE_t meta_flag_handler(char* cmdline, ATOP_t opType, char* response);
#endif
