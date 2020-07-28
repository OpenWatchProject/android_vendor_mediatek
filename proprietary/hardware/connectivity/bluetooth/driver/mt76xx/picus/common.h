/**
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _COMMON_H_
#define _COMMON_H_

/* --------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <endian.h>
#include <sys/stat.h>

/* --------------------------------------------------------------------------- */
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/* --------------------------------------------------------------------------- */
/* This device node only for read firmware log */
#define CUST_BT_FWLOG_PORT  "/dev/stpbtfwlog"
/* Picus log default PATH is /data/misc/bluedroid/dump_0.picus */
/* FW dump default PATH is /data/misc/bluedroid/fw_dump.picus */
#define DEFAULT_PATH "/data/misc/bluedroid"
#define DUMP_PICUS_NAME_PREFIX "dump_"
#define DUMP_PICUS_NAME_EXT ".picus"
#define FW_DUMP_PICUS_NAME "fw_dump"


#define RETRY_COUNT         20
#define FW_LOG_SWITCH_SIZE  20 * 1024 * 1024
#define MT_TIMEOUT_VALUE    1000
#define IOC_MAGIC           0xb0
#define PICUS_EVENT_LEN     3
#define PICUS_ACL_LEN       4

/* add get chip id(ex:7668...) */
#define IOCTL_GET_CHIP_ID                           _IOWR('H', 1, int)
/* add for BT Tool, change ALTERNATE_SETTING for SCO */
#define IOCTL_CHANGE_ALTERNATE_SETTING_INTERFACE    _IOWR(IOC_MAGIC, 2, unsigned long)

#define HCE_CONNECTION_COMPLETE         0x03
#define HCE_COMMAND_COMPLETE            0x0E

/* --------------------------------------------------------------------------- */
typedef enum _MT_DEBUG_LEVEL
{
    MT_DEBUG_SHOW,      // debug off, priority highest
    MT_DEBUG_ERROR,     // only show eror
    MT_DEBUG_WARN,
    MT_DEBUG_TRACE,
    MT_DEBUG_DEEPTRACE,
    MT_DEBUG_HCITRACE,
} MT_DEBUG_LEVEL;

typedef enum _MT_API_RESULT
{
    MT_RESULT_FAIL = -1,
    MT_RESULT_SUCCESS,
} MMT_API_RESULT;

typedef enum {
    DATA_TYPE_COMMAND = 1,
    DATA_TYPE_ACL     = 2,
    DATA_TYPE_SCO     = 3,
    DATA_TYPE_EVENT   = 4
} serial_data_type_t;

/* --------------------------------------------------------------------------- */
void DBGPRINT(int level, const char *format, ...);

#endif /* _COMMON_H_ */
