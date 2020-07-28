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

#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <inttypes.h>
#include "KpiObj.h"
#include "TranslatorConfig.h"
#include "TrapTable.h"

#include <map>

using namespace std;

#define FN_NAME_INIT "initialize"
#define FN_NAME_FINAL "finalize"
#define FN_NAME_GET_TRANSLATOR_CONFIG "getTranslatorConfig"
#define FN_NAME_GET_KPI_TYPE_MAP_BY_OID "getKpiTypeMapByOid"
#define FN_NAME_GET_KPI_GROUP_ID "getKpiGroupId"
#define FN_NAME_GET_TRAP_TABLE "getTrapTableByKpi"
#define FN_NAME_TRANSLATE_KPI "translateKpi"
#define FN_NAME_QUERY_KPI "queryKpi"
#define FN_NAME_SET_PACKET_PAYLOAD_SIZE "setPacketPayloadSize"
#define FN_NAME_SET_PACKET_PAYLOAD_TYPE "setPacketPayloadType"
#define FN_NAME_SET_MD_BASE_TIME "setModemBaseTime"



/**
 * Error Code for translator interface
 */
typedef enum {
    TRANSLATOR_RESULT_SUCCESS = 0,
    TRANSLATOR_RESULT_FAIL,
    TRANSLATOR_RESULT_SKIP,
    TRANSLATOR_RESULT_FATAL
} TRANSLATOR_RESULT_CODE;

typedef int8_t KPI_GID;
typedef uint64_t KPI_MSGID;
typedef uint8_t KPI_SIMID;
typedef uint64_t KPI_TIMESTAMP;

/**
 * Initialize the translator library
 */
typedef TRANSLATOR_RESULT_CODE (*FP_INIT)(void);

/**
 * Finalize the translator library
 */
typedef TRANSLATOR_RESULT_CODE (*FP_FINAL)(void);

/**
 * Get the translator basic configuration.
 */
typedef const TranslatorConfig *(*FP_GET_TRANSLATOR_CONFIG)(void);

/**
 * Get KPI list from request OID for subscription task.
 * return the size of OID list.
 */
typedef uint8_t (*FP_GET_KPI_TYPE_MAP_BY_OID)(const ObjId &, map<KPI_OBJ_TYPE, ObjId *> &);

/**
 * Get the group ID by KPI type for statistic purpose.
 */
typedef KPI_GID (*FP_GET_KPI_GROUP_ID)(KPI_OBJ_TYPE);

/**
 * DMC-core loops over all KPI list and translate the source message.
 * i.e. one source message may trigger multiple times translation requests.
 *
 * Get the EM / ICD message ID according to the KPI type,
 * may return multiple results per KPI.
 */
typedef const TrapTable * (*FP_GET_TRAP_TABLE)(KPI_OBJ_TYPE);

/**
 * Translate and encode the KPI APM/MDM/Local/Packet message.
 * Update the translate code with KpiObj.setResultCode().
 */
typedef KPI_OBJ_RESULT_CODE (*FP_TRANSLATE_KPI)(
        KPI_OBJ_TYPE, // kpiType
        KPI_MSGID, // msgId
        KPI_SIMID, // SIM ID
        uint64_t, // time-stamp in us, 0 means unspecified
        KPI_SOURCE_TYPE, // KPI source
        KpiObj &, // Output KpiObj
        uint64_t, // Data length
        const void *, // Data
        const void *); // Additional parameter by source

/**
 * Query and return KPI data.
 * Update the query code with KpiObj.setResultCode().
 *
 * @param isPeriodicReport false for on demand query, true for periodic report.
 * @param timestamp in millisecond.
 * @param kpiType The KPI to be query.
 * @param simId Query which SIM, SIM_ID_DEFAULT for not SIM related KPI.
 * @param kpiObj The output KPI object to be return.
 * @param Additional parameter.
 *
 */
typedef KPI_OBJ_RESULT_CODE (*FP_QUERY_KPI)(
        bool, // isPeriodicReport
        uint64_t, // time-stamp in us, 0 means unspecified
        KPI_OBJ_TYPE, // kpiType
        KPI_SIMID, // simId
        KpiObj &, // Output KpiObj
        const void *); // Additional parameter

/**
 * Limit the size of packet payload reported.
 */
typedef KPI_OBJ_RESULT_CODE (*FP_SET_PACKET_PAYLOAD_SIZE)(const ObjId &, uint32_t size);

/**
 * Set the payload type OTA_PACKET_TYPE for packet object.
 */
typedef KPI_OBJ_RESULT_CODE (*FP_SET_PACKET_PAYLOAD_TYPE)(const ObjId &, OTA_PACKET_TYPE type);

/**
 * In EM based solution, AP calculate timestamp based on TRAP_TYPE_PSTIME from MD,
 * however, the update frequency can't meet requirment required accuracy time.
 * so MD will pass rtos system ticks with the EM message, so that AP has chance to
 * calculate more accurate timestamp for the MD KPI.
 *
 * @param baseRtosTime: The intial rtos time from MD.
 * @param baseSysTime: The corresponding Unix system time with initial baseRtosTime.
 */
typedef void (*FP_SET_MODEM_BASE_TIME)(uint32_t, uint64_t);

#endif
