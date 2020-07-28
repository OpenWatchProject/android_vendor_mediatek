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

// Wrapper of translator library
// Example: libtranslator_oem_v1.0.so

#include "translator.h"
#include "TranslatorManager.h"

TranslatorManager *g_manager;

extern "C" TRANSLATOR_RESULT_CODE initialize(void) {
    g_manager = new TranslatorManager();
    if (NULL == g_manager) {
        return TRANSLATOR_RESULT_FATAL;
    }
    return TRANSLATOR_RESULT_SUCCESS;
}

extern "C" TRANSLATOR_RESULT_CODE finalize(void) {
    delete g_manager;
    return TRANSLATOR_RESULT_SUCCESS;
}

extern "C" const TranslatorConfig *getTranslatorConfig(void) {
    if (g_manager != NULL) {
        return g_manager->getTranslatorConfig();
    }
    return NULL;
}

extern "C" uint8_t getKpiTypeMapByOid(const ObjId &objId, map<KPI_OBJ_TYPE, ObjId *> &kpiTypeMap) {
    if (g_manager != NULL) {
        return g_manager->getKpiTypeMapByOid(objId, kpiTypeMap);
    }
    return KPI_OBJ_TYPE_UNDEFINED;
}

extern "C" KPI_GID getKpiGroupId(KPI_OBJ_TYPE kpiObjType) {
    if (g_manager != NULL) {
        return g_manager->getKpiGroupId((KPI_OBJ_TYPE)kpiObjType);
    }
    return KPI_GROUP_TYPE_UNDEFINED;
}

extern "C" const TrapTable *getTrapTableByKpi(KPI_OBJ_TYPE kpiObjType) {
    if (g_manager != NULL) {
        return g_manager->getTrapTableByKpi((KPI_OBJ_TYPE)kpiObjType);
    }
    return NULL;
}

extern "C" KPI_OBJ_RESULT_CODE translateKpi(
        KPI_OBJ_TYPE kpiType,
        KPI_MSGID msgId,
        KPI_SIMID simId,
        uint64_t timestampUs,
        KPI_SOURCE_TYPE source,
        KpiObj &outObj,
        uint64_t datalen,
        const void *data,
        const void *param) {
    KPI_OBJ_RESULT_CODE ret = KPI_OBJ_RESULT_CODE_UNDEFIEND;
    if (g_manager != NULL) {
        ret = g_manager->translateKpi(
                kpiType, msgId, simId, timestampUs, source, outObj, datalen, data, param);
    }
    return ret;
}

extern "C" KPI_OBJ_RESULT_CODE queryKpi(
        bool isPeriodicReport,
        uint64_t timestampUs,
        KPI_OBJ_TYPE kpiType,
        KPI_SIMID simId,
        KpiObj &outObj,
        const void *param) {
    KPI_OBJ_RESULT_CODE ret = KPI_OBJ_RESULT_CODE_UNDEFIEND;
    if (g_manager != NULL) {
        ret = g_manager->queryKpi(
                isPeriodicReport, timestampUs, kpiType, simId, outObj, param);
    }
    return ret;
}

extern "C" KPI_OBJ_RESULT_CODE setPacketPayloadSize(const ObjId &objId, uint32_t size) {
    KPI_OBJ_RESULT_CODE ret = KPI_OBJ_RESULT_CODE_FAIL;
    if (g_manager != NULL) {
        ret = g_manager->setPacketPayloadSize(objId, size);
    }
    return ret;
}

extern "C" KPI_OBJ_RESULT_CODE setPacketPayloadType(const ObjId &objId, OTA_PACKET_TYPE type) {
    KPI_OBJ_RESULT_CODE ret = KPI_OBJ_RESULT_CODE_FAIL;
    if (g_manager != NULL) {
        ret = g_manager->setPacketPayloadType(objId, type);
    }
    return ret;
}

extern "C" void setModemBaseTime(uint32_t baseRtosTime, uint64_t baseSysTimeUs) {
    if (g_manager != NULL) {
        g_manager->setModemBaseTime(baseRtosTime, baseSysTimeUs);
    }
}