
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

#ifndef __TRANSLATOR_LOADER_H__
#define __TRANSLATOR_LOADER_H__

#include "translator.h"
#include "KpiObj.h"
#include <libmdmonitor.h>
#include <inttypes.h>
#include <mutex>
#include "dmc_utils.h"

using namespace libmdmonitor;

#define TRANSLATOR_LIB_PREFIX "libtranslator"
#define TRANSLATOR_LIB_EXTENSION ".so"


class TranslatorLoader
{
public:
    TranslatorLoader();
    virtual ~TranslatorLoader();

    // Format "libtranslator_<application_identity>_<version>"
    bool load(const char* appIdentity, const char* version);
    void unload(void);
    bool isInitialized(void);

    TRANSLATOR_RESULT_CODE initializeTranslator(void);
    TRANSLATOR_RESULT_CODE finalizeTranslator(void);
    const TranslatorConfig *getTranslatorConfig();
    uint8_t getKpiTypeMapByOid(const ObjId &oid, map<KPI_OBJ_TYPE, ObjId *> &kpiTypeMap);
    KPI_GID getKpiGroupId(KPI_OBJ_TYPE kpiObjType);
    const TrapTable * getTrapTableByKpi(KPI_OBJ_TYPE kpiObjType);
    KPI_OBJ_RESULT_CODE translateKpi(
            KPI_OBJ_TYPE kpiType,
            KPI_MSGID msgId,
            KPI_SIMID simId,
            uint64_t timestampUs,
            KPI_SOURCE_TYPE source,
            KpiObj &outObj,
            uint64_t datalen,
            const void *data,
            const void *param);
    KPI_OBJ_RESULT_CODE queryKpi(
            bool isPeriodicReport,
            uint64_t timestampUs,
            KPI_OBJ_TYPE kpiType,
            KPI_SIMID simId,
            KpiObj &outObj,
            const void *param);
    KPI_OBJ_RESULT_CODE setPacketPayloadSize(const ObjId &objId, uint32_t size);
    KPI_OBJ_RESULT_CODE setPacketPayloadType(const ObjId &objId, OTA_PACKET_TYPE type);
    void setModemBaseTime(uint32_t baseRtosTime, uint64_t baseSysTimeUs);

private:
    bool bInitialized;
    char *mTranslatorLibName;
    std::mutex *mLock;
    // dlpoen handler pointer
    void *sRefLibTranslator;
    // function pointers
    FP_INIT sFpInit;
    FP_FINAL sFpFinal;
    FP_GET_TRANSLATOR_CONFIG sFpGetTranslatorConfig;
    FP_GET_KPI_TYPE_MAP_BY_OID sFpGetKpiTypeMapByOid;
    FP_GET_KPI_GROUP_ID sFpGetKpiGroupId;
    FP_GET_TRAP_TABLE sFpGetTraptable;
    FP_TRANSLATE_KPI sFpTranslateKpi;
    FP_QUERY_KPI sFpQueryKpi;
    FP_SET_PACKET_PAYLOAD_SIZE sFpSetPacketPayloadSize;
    FP_SET_PACKET_PAYLOAD_TYPE sFpSetPacketPayloadType;
    FP_SET_MODEM_BASE_TIME sFpSetModemBaseTime;

    bool loadTranslatorLibrary(const char* translatorLibName);
};

#endif
