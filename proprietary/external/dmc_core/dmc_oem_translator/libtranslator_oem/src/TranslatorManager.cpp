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

#include <TranslatorManager.h>
#include "log_utils.h"

#include <errno.h>
#include <map>

#undef TAG
#define TAG "OEM-TranslatorManager"

// ============================ Public Interfaces of OEM TranslatorManager ============================
TranslatorManager::TranslatorManager(): TranslatorManagerBase(TAG, OEM_OBJ_TYPE_SIZE)
{
    DMC_LOGD(TAG, "Constructor()");

    // Include macro header to initialize KPI objects
    #include "BeginTranslatorClassNew.h"
    #include "TranslatorList_v1.0.inc"
    #include "EndTranslatorClassNew.h"

    TranslatorManagerBase::initialize();
}

TranslatorManager::~TranslatorManager() {
    // Uninitialize what you want.
}

// Public APIs
KPI_OBJ_RESULT_CODE TranslatorManager::translateKpi(
        KPI_OBJ_TYPE kpiType,
        KPI_MSGID msgId,
        KPI_SIMID simId,
        uint64_t &timestampUs,
        KPI_SOURCE_TYPE source,
        KpiObj &outObj,
        uint64_t datalen,
        const void *data,
        const void *param) {

    // This call to super class will fire TRANSLATE(OEM, XXX) function block
    // And the output data will be put in KpiObj object.
    KPI_OBJ_RESULT_CODE ret = TranslatorManagerBase::translateKpi(
            kpiType, msgId, simId, timestampUs, source, outObj, datalen, data, param);

    if (KPI_OBJ_RESULT_CODE_SUCESS == ret) {
        // Make outObj to any data format
        ret = makeOemTrapMessage(kpiType, simId, timestampUs, m_pTranslatorList[kpiType]->getObjId(), outObj);
    }

    return ret;
}

// Internal APIs
void TranslatorManager::setupTranslatorConfig() {
    // Index to optimize performance, we can skip this first
    translator_kpi_range kpiRange[KPI_SOURCE_TYPE_SIZE];
    kpiRange[KPI_SOURCE_TYPE_LOCAL].source = KPI_SOURCE_TYPE_LOCAL;
    kpiRange[KPI_SOURCE_TYPE_LOCAL].beginIdx = OEM_OBJ_TYPE_KPI_BEGIN;
    kpiRange[KPI_SOURCE_TYPE_LOCAL].endIdx = OEM_OBJ_TYPE_KPI_END;
    kpiRange[KPI_SOURCE_TYPE_APM].source = KPI_SOURCE_TYPE_APM;
    kpiRange[KPI_SOURCE_TYPE_APM].beginIdx = OEM_OBJ_TYPE_KPI_BEGIN;
    kpiRange[KPI_SOURCE_TYPE_APM].endIdx = OEM_OBJ_TYPE_KPI_END;
    kpiRange[KPI_SOURCE_TYPE_PKM].source = KPI_SOURCE_TYPE_PKM; // Skip PKM source
    kpiRange[KPI_SOURCE_TYPE_PKM].beginIdx = OEM_OBJ_TYPE_KPI_BEGIN;
    kpiRange[KPI_SOURCE_TYPE_PKM].endIdx = OEM_OBJ_TYPE_KPI_BEGIN;
    kpiRange[KPI_SOURCE_TYPE_MDM].source = KPI_SOURCE_TYPE_MDM;
    kpiRange[KPI_SOURCE_TYPE_MDM].beginIdx = OEM_OBJ_TYPE_KPI_BEGIN;
    kpiRange[KPI_SOURCE_TYPE_MDM].endIdx = OEM_OBJ_TYPE_KPI_END;

    TranslatorConfig *pConfig = new TranslatorConfig(
            OEM_TRANSLATOR_VERSION, // Translator version
            getKpiSize(), // Size of KPIs
            OEM_GROUP_MAX, // Size of KPI group
            0, // Define shift macro seconds from 1970/01/01 00:00:00
            kpiRange,
            0); // Additional parameters
    setTranslatorConfig((const TranslatorConfig *) pConfig);
}

KPI_GID TranslatorManager::getKpiGroupId(KPI_OBJ_TYPE kpiType) {
    DMC_LOGD(TAG, "getKpiGroupId(%d) = %d", kpiType, OemKpiMap::GetKpiLibType(kpiType));
    return (KPI_GID)OemKpiMap::GetKpiLibType(kpiType);
}

ObjId *TranslatorManager::getObjId(KPI_OBJ_TYPE kpiType) {
    if (!OemKpiMap::isValidKpiType(kpiType)) {
        DMC_LOGE(TAG, "getObjId() with invalid objType=%d", kpiType);
        return NULL;
    }
    return &OemKpiMap::sKpiMap.at(kpiType).oid;
}

KPI_OBJ_RESULT_CODE TranslatorManager::makeOemTrapMessage(
        KPI_OBJ_TYPE kpiType, KPI_SIMID simId, uint64_t timestamp, ObjId *pOid, KpiObj &outObj) {
    UNUSED(timestamp);
    UNUSED(pOid);

    DMC_LOGD(TAG, "[%" PRIu32"]makeOemTrapMessage() kpiType=%d", simId, kpiType);
    uint8_t *payload_buf = NULL;
    unsigned int payload_len = 0;
    size_t kpi_size = 0;
    // Get data from TRANSLATE(OEM, XXX) function block
    unsigned char *data = (unsigned char *)outObj.GetByteArray(kpi_size);
    UNUSED(data);

    // Do any customization here, the payload will be sent back to DMC client.
    outObj.SetByteArray((const uint8_t *)payload_buf, payload_len);

    return KPI_OBJ_RESULT_CODE_SUCESS;
}

