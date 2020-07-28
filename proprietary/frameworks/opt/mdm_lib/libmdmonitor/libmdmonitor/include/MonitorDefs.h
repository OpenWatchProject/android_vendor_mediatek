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
/**
 * @file MonitorDefs.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file contains the defines that are used in modem monitor library.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MONITOR_DEFS_H__
#define __MONITOR_DEFS_H__
namespace libmdmonitor {
#define DHL_PRIMITIVE 0
#define DHL_INDEX_TRACE 1
#define DHL_TIME_INFO  5
#define DHL_ICD_TUNNELING  0xE0
#define DHL_MAX_FRAME_SIZE 65536
#define MAX_RAWDATA_LEN  DHL_MAX_FRAME_SIZE + sizeof(discard_info_struct)*TRAP_TYPE_SIZE

#define MAX_SOCK_ADDR_LEN 128
#define MAX_SECRET_LEN 256
#ifndef offsetof
#define offsetof(type,member) ((std::size_t) &(((type*)0)->member))
#endif
#define MONITOR_COMMAND_SERVICE_ABSTRACT_NAME "com.mediatek.mdmonitor.command"
#define SECRET_ROOT "oOEMrmmRimOToNKdTtO"
#define SECRET_SESSION_MANAGER "GnoOEMrMmamimToNKdTReA"
#define SESSION_EXPIRE_DURATION_IN_SECOND (60*60*24*7)
#define MAX_READ_BUFFER_SIZE (256*1024)
#define MAX_PAYLOAD_LEN (128*1024)
#define MAX_SOCKET_CLIENT_COUNT 256
#define MAX_SOCKET_SERVER_COUNT 8
#define DHL_FRAME_RX_BUFFER_SIZE (2048*1024) //Circular buffer size in TrapSender
#define BULK_READ_BUFFER_SIZE (128*1024) //Max Checkout from Circular buffer in TrapSender

#define DEFAULT_LAYOUT_DESC_DIR_SEARCH_PATH_WO_MD_SINGLE_BIN "/vendor/etc/mddb"   // Q DEV only now, Need to confirm final path for modem single bin off in Q
#define DEFAULT_LAYOUT_DESC_PREFIX "mdm_layout_desc"
#define DEFAULT_LAYOUT_DESC_SUFFIX ".dat"

#define DEFAULT_EM_FILTER_DIR_SEARCH_PATH_WO_MD_SINGLE_BIN "/vendor/etc/firmware"    // Q DEV only now, Need to confirm final path for modem single bin off in Q
#define DEFAULT_EM_FILTER_PREFIX "em_filter"
#define DEFAULT_EM_FILTER_SUFFIX ".bin"

// for single bin modem support
#define DIR_SEARCH_PATH_DATA "/data/vendor/md_mon/"
#define DEFAULT_LAYOUT_DESC_FILE_PATH_DATA_FORMAT DIR_SEARCH_PATH_DATA "mdm_layout_desc_%s.dat"
#define MON_EM_FILTER_FILE_PATH_DATA_FORMAT DIR_SEARCH_PATH_DATA "em_filter_%s.bin"
#define DEFAULT_LAYOUT_DESC_KEY_IN_SINGLE_BIN "md1_mdmlayout"
#define MON_EM_FILTER_KEY_IN_SINGLE_BIN "md1_emfilter"

// Clients
#define CLIENT_SAMPLE "com.mediatek.mdmlsample"
#define CLIENT_EM "com.mediatek.engineermode"
#define CLIENT_SWIFT "com.mediatek.swift"
#define CLIENT_CAT "com.mediatek.autotest"
#define CLIENT_RECORDING "com.mediatek.modemMonitorRecording"

//ALPS03661792
#define CLIENT_ECHOLOCATE "com.tct.echolocatedata"
//ALPS03973954
#define CLIENT_SPRINT "com.tinno.hiddenmenu"
//ALPS04285745
#define CLIENT_ZTE_SPRINT "com.android.zte.hiddenmenu"
//ALPS04411642
#define CLIENT_YU_LONG "com.borqs.hiddenmenu"
//ALPS04754132
#define CLIENT_TSDSMARTCOMM "com.transsion.tsdsmartcomm"
//ALPS04878559
#define CLIENT_VZW_TEST "com.mediatek.omadm" //test app, should be removed after customer's feature done
#define CLIENT_VZW "com.hmdglobal.app.omadm"

#define PROPERTY_PLATFORM "ro.board.platform"
#define PROPERTY_ANDROID_VER "ro.build.version.sdk"
#define PROPERTY_SINGLE_BIN_MODEM_SUPPORT "ro.vendor.mtk_single_bin_modem_support"

#define MAX_SESSION_COUNT (1 << 31)
#define COMBINED_SERVER_NAME(sessionId, szServerName) ((std::to_string(sessionId) + std::string(szServerName)).c_str())
} //namespace libmdmonitor {

#endif
