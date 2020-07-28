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

#ifndef _AP_MONITOR_DEFS_LEGACY_H_
#define _AP_MONITOR_DEFS_LEGACY_H_

#include <stdio.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

//Send data to the HIDL server
//Arguments:
// - type: (in)  native module type present in KPI_TYPE enum
// - data: (in) data to be sent to HIDL server
//Returns: 0 on success, -1 for error
int apmSend(int type, void *data);

//Check if logging is enabled for kpi type
//Arguments:
// - type: (in) type present in KPI_TYPE enum
//Returns: kpi type enabled status
bool apmIsKpiEnabled(int type);

// APM KPI definition [Legacy solution]
typedef enum {
    KPI_TYPE_INVALID = -1,
    KPI_TYPE_MDMI_BEGIN = 0,
    KPI_TYPE_SUPL_STATISTICS = 54,
    KPI_TYPE_SUPL_MESSAGE = 55
} KPI_TYPE;

// APM_MSG_SUPL_STATISTICS [Legacy solution]
typedef struct ApmSuplStatistics {
    int32_t numSuplInit;
    int32_t numSuplPosInit;
    int32_t numSuplPos;
    int32_t averageTimeBetweenSuplPosInitAndSuplInit;
    int32_t miniTimeBetweenSuplPosInitAndSuplInit;
    int32_t maxTimeBetweenSuplPosInitAndSuplInit;
    int32_t averageTimeBetweenSuplPosAndSuplInit;
    int32_t miniTimeBetweenSuplPosAndSuplInit;
    int32_t maxTimeBetweenSuplPosAndSuplInit;
} ApmSuplStatistics;

// APM_MSG_SUPL_MESSAGE [Legacy solution]
typedef struct ApmSuplMessage {
    uint8_t suplType;
    uint8_t *rawMessage;
    size_t rawMessageLength;
} ApmSuplMessage;

#ifdef  __cplusplus
}
#endif

#endif
