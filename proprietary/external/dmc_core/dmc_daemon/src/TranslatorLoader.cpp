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

#include "TranslatorLoader.h"
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>

using namespace libmdmonitor;

#undef TAG
#define TAG "DMC-TranslatorLoader"

// ============================ Public interface for TranslatorLoader ============================
TranslatorLoader::TranslatorLoader(): bInitialized(false), mTranslatorLibName(NULL)
{
    DMC_LOGD(TAG, "constructor()");
    mLock = new std::mutex();
    sRefLibTranslator = NULL;
}

TranslatorLoader::~TranslatorLoader() {
    DMC_LOGD(TAG, "destructor()");
    unload();
    delete mLock;
}

// Format "libtranslator_<application_identity>_<version>"
// example: libtranslator_mdmi_v2.8.2.so
bool TranslatorLoader::load(const char* appIdentity, const char* version) {
    if (NULL == appIdentity) {
        DMC_LOGE(TAG, "Invalid app identity");
        return false;
    }

    if (NULL == version) {
        DMC_LOGE(TAG, "Invalid version");
        return false;
    }

    if (bInitialized) {
        DMC_LOGE(TAG, "Already initialized for app identity: %s with version: %s",
                appIdentity, version);
        return false;
    }

    int ret = asprintf(&mTranslatorLibName, "%s_%s_%s%s",
            TRANSLATOR_LIB_PREFIX,
            appIdentity,
            version,
            TRANSLATOR_LIB_EXTENSION);

    if (ret < 0) {
        DMC_LOGE(TAG, "asprintf() failed: %d", ret);
        return false;
    }

    if (!loadTranslatorLibrary(mTranslatorLibName)) {
        DMC_LOGE(TAG, "load(%s) failed!", mTranslatorLibName);
        unload();
    } else {
        bInitialized = true;
        initializeTranslator();

        DMC_LOGI(TAG, "load(%s) successfully!", mTranslatorLibName);
    }

    return bInitialized;
}

void TranslatorLoader::unload(void) {
    // Tell translator closed
    finalizeTranslator();
    if (sRefLibTranslator != NULL) {
        dlclose(sRefLibTranslator);
    }
    sRefLibTranslator = NULL;
    sFpInit = 0;
    sFpFinal = 0;
    sFpGetTranslatorConfig = 0;
    sFpGetKpiTypeMapByOid = 0;
    sFpGetKpiGroupId = 0;
    sFpGetTraptable = 0;
    sFpTranslateKpi = 0;
    sFpQueryKpi = 0;
    sFpSetPacketPayloadSize = 0;
    sFpSetPacketPayloadType = 0;
    sFpSetModemBaseTime = 0;

    DMC_LOGD(TAG, "unload(%s)", mTranslatorLibName);
    if (mTranslatorLibName != NULL) {
        free(mTranslatorLibName);
        mTranslatorLibName = NULL;
    }
    bInitialized = false;
}

bool TranslatorLoader::isInitialized(void) {
    return bInitialized;
}

// ============================ Interfaces for Translator interface ============================
TRANSLATOR_RESULT_CODE TranslatorLoader::initializeTranslator(void) {
    if (isInitialized()) {
        return sFpInit();
    }
    return TRANSLATOR_RESULT_FATAL;
}

TRANSLATOR_RESULT_CODE TranslatorLoader::finalizeTranslator(void) {
    if (isInitialized()) {
        return sFpFinal();
    }
    return TRANSLATOR_RESULT_FATAL;
}

const TranslatorConfig *TranslatorLoader::getTranslatorConfig() {
    if (isInitialized()) {
        mLock->lock();
        const TranslatorConfig *pConfig = sFpGetTranslatorConfig();
        mLock->unlock();
        return pConfig;
    }
    return NULL;
}

uint8_t TranslatorLoader::getKpiTypeMapByOid(const ObjId &oid, map<KPI_OBJ_TYPE, ObjId *> &kpiTypeMap) {
    if (isInitialized()) {
        mLock->lock();
        uint8_t type = sFpGetKpiTypeMapByOid(oid, kpiTypeMap);
        mLock->unlock();
        return type;
    }
    return KPI_OBJ_TYPE_UNDEFINED;
}

KPI_GID TranslatorLoader::getKpiGroupId(KPI_OBJ_TYPE kpiObjType) {
    if (isInitialized()) {
        mLock->lock();
        KPI_GID gid = sFpGetKpiGroupId(kpiObjType);
        mLock->unlock();
        return gid;
    }
    return KPI_GROUP_TYPE_UNDEFINED;
}

const TrapTable *TranslatorLoader::getTrapTableByKpi(KPI_OBJ_TYPE kpiObjType) {
    if (isInitialized()) {
        mLock->lock();
        const TrapTable *pTable = sFpGetTraptable(kpiObjType);
        mLock->unlock();
        return pTable;
    }
    return NULL;
}

KPI_OBJ_RESULT_CODE TranslatorLoader::translateKpi(
        KPI_OBJ_TYPE kpiType,
        KPI_MSGID msgId,
        KPI_SIMID simId,
        uint64_t timestampUs,
        KPI_SOURCE_TYPE source,
        KpiObj &outObj,
        uint64_t datalen,
        const void *data,
        const void *param) {
    if (isInitialized()) {
        mLock->lock();
        KPI_OBJ_RESULT_CODE ret =
                sFpTranslateKpi(kpiType, msgId, simId, timestampUs, source, outObj, datalen, data, param);
        mLock->unlock();
        return ret;
    }
    return KPI_OBJ_RESULT_CODE_FAIL;
}

KPI_OBJ_RESULT_CODE TranslatorLoader::queryKpi(
        bool isPeriodicReport,
        uint64_t timestampUs,
        KPI_OBJ_TYPE kpiType,
        KPI_SIMID simId,
        KpiObj &outObj,
        const void *param) {
    if (isInitialized()) {
        mLock->lock();
        KPI_OBJ_RESULT_CODE ret = sFpQueryKpi(isPeriodicReport, timestampUs, kpiType, simId, outObj, param);
        mLock->unlock();
        return ret;
    }
    return KPI_OBJ_RESULT_CODE_FAIL;
}

KPI_OBJ_RESULT_CODE TranslatorLoader::setPacketPayloadSize(const ObjId &objId, uint32_t size) {
    if (isInitialized()) {
        mLock->lock();
        KPI_OBJ_RESULT_CODE ret = sFpSetPacketPayloadSize(objId, size);
        mLock->unlock();
        return ret;
    }
    return KPI_OBJ_RESULT_CODE_FAIL;
}

KPI_OBJ_RESULT_CODE TranslatorLoader::setPacketPayloadType(const ObjId &objId, OTA_PACKET_TYPE type) {
    if (isInitialized()) {
        mLock->lock();
        KPI_OBJ_RESULT_CODE ret = sFpSetPacketPayloadType(objId, type);
        mLock->unlock();
        return ret;
    }
    return KPI_OBJ_RESULT_CODE_FAIL;
}

void TranslatorLoader::setModemBaseTime(uint32_t baseRtosTime, uint64_t baseSysTimeUs) {
    if (isInitialized()) {
        mLock->lock();
        sFpSetModemBaseTime(baseRtosTime, baseSysTimeUs);
        mLock->unlock();
    }
}

// ============================ Private Interfaces of TranslatorLoader ============================
bool TranslatorLoader::loadTranslatorLibrary(const char* translatorLibName)
{
    sRefLibTranslator = dlopen(translatorLibName, RTLD_NOW | RTLD_LOCAL);
    if (sRefLibTranslator == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: open lib %s failed", translatorLibName);
        return false;
    }

    sFpInit = (FP_INIT) dlsym(sRefLibTranslator, FN_NAME_INIT);
    if (sFpInit == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_INIT);
        return false;
    }

    sFpFinal = (FP_FINAL) dlsym(sRefLibTranslator, FN_NAME_FINAL);
    if (sFpFinal == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_FINAL);
        return false;
    }

    sFpGetTranslatorConfig = (FP_GET_TRANSLATOR_CONFIG) dlsym(sRefLibTranslator, FN_NAME_GET_TRANSLATOR_CONFIG);
    if (sFpGetTranslatorConfig == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_GET_TRANSLATOR_CONFIG);
        return false;
    }

    sFpGetKpiTypeMapByOid = (FP_GET_KPI_TYPE_MAP_BY_OID) dlsym(sRefLibTranslator, FN_NAME_GET_KPI_TYPE_MAP_BY_OID);
    if (sFpGetKpiTypeMapByOid == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_GET_KPI_TYPE_MAP_BY_OID);
        return false;
    }

    sFpGetKpiGroupId = (FP_GET_KPI_GROUP_ID) dlsym(sRefLibTranslator, FN_NAME_GET_KPI_GROUP_ID);
    if (sFpGetKpiGroupId == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_GET_KPI_GROUP_ID);
        return false;
    }

    sFpGetTraptable = (FP_GET_TRAP_TABLE) dlsym(sRefLibTranslator, FN_NAME_GET_TRAP_TABLE);
    if (sFpGetTraptable == 0) {
        DMC_LOGE(TAG, "TranslatorLoader: dlsym %s failed", FN_NAME_GET_TRAP_TABLE);
        return false;
    }

    sFpTranslateKpi = (FP_TRANSLATE_KPI) dlsym(sRefLibTranslator, FN_NAME_TRANSLATE_KPI);
    if (sFpTranslateKpi == 0) {
        DMC_LOGE("TranslatorLoader: dlsym %s failed", FN_NAME_TRANSLATE_KPI);
        return false;
    }

    sFpQueryKpi = (FP_QUERY_KPI) dlsym(sRefLibTranslator, FN_NAME_QUERY_KPI);
    if (sFpQueryKpi == 0) {
        DMC_LOGE("TranslatorLoader: dlsym %s failed", FN_NAME_QUERY_KPI);
        return false;
    }

    sFpSetPacketPayloadSize = (FP_SET_PACKET_PAYLOAD_SIZE) dlsym(sRefLibTranslator, FN_NAME_SET_PACKET_PAYLOAD_SIZE);
    if (sFpSetPacketPayloadSize == 0) {
        DMC_LOGE("TranslatorLoader: dlsym %s failed", FN_NAME_SET_PACKET_PAYLOAD_SIZE);
        return false;
    }

    sFpSetPacketPayloadType = (FP_SET_PACKET_PAYLOAD_TYPE) dlsym(sRefLibTranslator, FN_NAME_SET_PACKET_PAYLOAD_TYPE);
    if (sFpSetPacketPayloadType == 0) {
        DMC_LOGE("TranslatorLoader: dlsym %s failed", FN_NAME_SET_PACKET_PAYLOAD_TYPE);
        return false;
    }

    sFpSetModemBaseTime = (FP_SET_MODEM_BASE_TIME) dlsym(sRefLibTranslator, FN_NAME_SET_MD_BASE_TIME);
    if (sFpSetModemBaseTime == 0) {
        DMC_LOGE("TranslatorLoader: dlsym %s failed", FN_NAME_SET_MD_BASE_TIME);
        return false;
    }

    DMC_LOGD(TAG, "TranslatorLoader: open lib %s successfully!", translatorLibName);

    return true;
}

