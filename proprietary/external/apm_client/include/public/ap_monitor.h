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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _AP_MONITOR_H_
#define _AP_MONITOR_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef uint64_t timeStamp;
typedef uint16_t APM_MSGID;
typedef uint8_t APM_SIMID;
typedef int32_t APM_SID;

#define SIM_ID_DEFAULT INT8_MAX
#define INVALID_SESSION_ID (0)
#define INVALID_APM_MSGID (UINT16_MAX)

//==================[ New APM Public Interface ]=========================
typedef enum {
    APM_RESULT_SUCCESS,
    APM_RESULT_FAILED,
    APM_RESULT_INVALID_ARGUMENTS
} APM_RESULT_CODE;

/**
 * Submit KPI through APM client.
 *
 * @param msgId Defined in apmMsgId_e of libapm_msg_defs_headers
 * @param len Lengh of the data.
 * @param data The KPI Object to be submitted, structure defined in vendor.mediatek.hardware.apmonitor or byte array.
 *
 * @return Returns APM_RESULT_SUCCESS if submit the KPI successfully.
 */
APM_RESULT_CODE apmSubmitKpi(APM_MSGID msgId,
        uint32_t len, const void *data);

/**
 * Submit KPI through APM client, "S" stands for SIM ID.
 *
 * @param msgId Defined in apmMsgId_e of libapm_msg_defs_headers
 * @param simId The SIM ID submitted with the KPI, if the KPI is not related to the SIM,
 *              please use the apmSubmitKpiX() without "S".
 * @param len Lengh of the data.
 * @param data The KPI Object to be submitted, structure defined in vendor.mediatek.hardware.apmonitor or byte array.
 *
 * @return Returns APM_RESULT_SUCCESS if submit the KPI successfully.
 */
APM_RESULT_CODE apmSubmitKpiS(APM_MSGID msgId, APM_SIMID simId,
        uint32_t len, const void *data);

/**
 * Submit KPI through APM client, "T" means the epoch timestamp in milliseconds.
 *
 * @param msgId Defined in apmMsgId_e of libapm_msg_defs_headers
 * @param timestampMs The timestamp binding with the KPI, epoch from 1970/01/01 00:00:00.
 *                    Caller can get current time by clock_gettime() API.
 * @param len Lengh of the data.
 * @param data The KPI Object to be submitted, structure defined in vendor.mediatek.hardware.apmonitor or byte array.
 *
 * @return Returns APM_RESULT_SUCCESS if submit the KPI successfully.
 */
APM_RESULT_CODE apmSubmitKpiT(APM_MSGID msgId, timeStamp timestampMs,
        uint32_t len, const void *data);

/**
 * Submit KPI through APM client with SIM ID and epoch timestamp information.
 *
 * @param msgId Defined in apmMsgId_e of libapm_msg_defs_headers
 * @param simId The SIM ID submitted with the KPI, if the KPI is not related to the SIM,
 *              please use the apmSubmitKpiX() without "S".
 * @param timestampMs The timestamp binding with the KPI, epoch from 1970/01/01 00:00:00.
 *                    Caller can get current time by clock_gettime() API.
 * @param len Lengh of the data.
 * @param data The KPI Object to be submitted, structure defined in vendor.mediatek.hardware.apmonitor or byte array.
 *
 * @return Returns APM_RESULT_SUCCESS if submit the KPI successfully.
 */
APM_RESULT_CODE apmSubmitKpiST(APM_MSGID msgId, APM_SIMID simId, timeStamp timestampMs,
        uint32_t len, const void *data);

/**
 * Submission layer check this API to decide to submit KPI or not, however,
 * This API is being called inside apmSubmitKpiXXX(), as the result,
 * submission layer can use this API to avoid overhead if necessary.
 *
 * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
 * @param len Lengh of the data.
 * @param data The KPI Object to be submitted, structure defined in vendor.mediatek.hardware.apmonitor or byte array.
 *
 * @return Returns true to tell submission layer to submit the KPI.
 */
bool apmShouldSubmitKpi(APM_MSGID msgId);

/**
 * The parameter comes from DMC core, to customize the behavior of APM client.
 * Currently, the parameter is used by Packet Monitor (PKM) only.
 *
 * @return Returns 64 bits parameter.
 */
int64_t apmGetSessionParam();

#ifdef  __cplusplus
}
#endif


#endif
