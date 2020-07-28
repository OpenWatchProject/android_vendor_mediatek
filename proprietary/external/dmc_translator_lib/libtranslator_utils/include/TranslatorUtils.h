
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
 * @file TranslatorUtils.h
 *
 * @brief The header file defines the interfaces that used by translators.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __TRANSLATOR_UTILS_H__
#define __TRANSLATOR_UTILS_H__

#include "KpiObj.h"
#include "TrapTable.h"
#include "KpiUtils.h"

#if TRANSLATE_WITH_ASN_DOT_1_ENCODE
    #include "AsnEncoder.h"
#endif

#include "libmdmonitor.h"
#include "FrameDecoder.h"
#include "log_utils.h"

#include <string>
#include <vector>
#include <map>

#undef MIN
#define MIN(a, b) ((a < b) ? a : b)

typedef uint8_t KPI_SIMID;
#define SIM_ID_DEFAULT INT8_MAX
#define MAX_SIM_NUM 4

#ifndef UNUSED
#define UNUSED(x) do{(void)(x); } while (0)
#endif

using namespace libmdmonitor;

#define BUILD_TYPE_PROP "ro.build.type"

// Macro to define source messages for the translator
#define IF_EM(msg_name) for (int dummy_index = 0; dummy_index < 1 && CheckEmMsg(msg_name); ++dummy_index)
#define IF_ICD(type, code) for (int dummy_index = 0; dummy_index < 1 && CheckICD(type, code); ++dummy_index)
#define IF_MSG(source, msgId) for (int dummy_index = 0; dummy_index < 1 && CheckMsg(source, msgId); ++dummy_index)

// Common macro
#define IS_USER_LOAD() isUserLoad()
#define SKIP_TRANSLATE() SkipTranslate()
#define SET_REPORT_PERIODIC_MS(ms) SetKpiReportPeriodic(ms)
#define ALLOC_TYPE(type) (type*)calloc(1, sizeof(type))
#define GET_EVENT_SIM_ID() GetEventSimId()
#define GET_MD_TIMESTAMP_US(rtos_time) GetReferenceMdTimestampUs(rtos_time)
#define UPDATE_TIMESTAMP(timeUs) obj.SetTimestamp(timeUs)
#define GET_PS_VERSION(versionType, version) FrameDecoder::GetInstance()->MDInfo_GetProtocolVersion(versionType, version)

// ASN1C macro
#if TRANSLATE_WITH_ASN_DOT_1_ENCODE
    #define ASN_ENCODE(type, ptr, len) AsnEncoder::Encode(m_pAsnDescriptor, ptr, len)
    #define ASN_FREE(type, ptr) AsnEncoder::FreeAsnBuf(m_pAsnDescriptor, ptr)
    #define ASN_FREE_EX(ptr) AsnEncoder::FreeAsnBuf(m_pAsnDescriptor, ptr)
    #define ASN_FREE_STRUCT(ptr) AsnEncoder::FreeAsnBufStruct(m_pAsnDescriptor, ptr)
    #define ASN_ALLOC_TYPE(type) (type*)calloc(1, sizeof(type))
#else
    #define ASN_ENCODE(type, ptr, len)
    #define ASN_FREE(type, ptr)
    #define ASN_FREE_EX(ptr)
    #define ASN_FREE_STRUCT(ptr)
    #define ASN_ALLOC_TYPE(type)
#endif

// APM translate macro
#define GET_TRANSLATE_DATA_VERSION() GetTranslateDataVersion()
#define GET_TRANSLATE_DATA() GetTranslateData()
#define GET_TRANSLATE_DATA_LEN() GetTranslateDataLength()

// EM/ICD to get raw payload
#define GET_RAW_DATA(size) GetRawData(size)

// EM to access fields
#define FIELD_VALUE(field, isSignedVal)  GET_INT_(field, isSignedVal)
#define FIELD_VALUE64(field, isSignedVal)  GET_INT64_(field, isSignedVal)
#define GET_INT(field, isSignedVal)  ((GetInt(NULL, field, isSignedVal))? GET_INT_(field, isSignedVal): NULL)
#define GET_INT64(field, isSignedVal)  ((GetInt64(NULL, field, isSignedVal))? GET_INT64_(field, isSignedVal): NULL)
#define GET_FIELD_ARRAY(field, arraySize, outBufSize) GetFieldBuf(field, arraySize, outBufSize)
// TODO: Integrated MAPI style
#define USE_FIELD(field, isSignedVal) \
    do {\
        if (GetInt(NULL, field, isSignedVal)) \
        { \
            m_field_value[field] = GET_INT_(field, isSignedVal); \
            m_field_signedMap[field] = isSignedVal; \
        } \
        else { \
            if (TRANSLATE_WITH_ASN_DOT_1_ENCODE ) { \
                AsnEncoder::FreeAsnBuf(m_pAsnDescriptor, &msgObj); \
            } \
            return; \
        } \
    } while(0);

#define USE_FIELD64(field, isSignedVal) \
    do { \
        if (GetInt64(NULL, field, isSignedVal)) \
        { \
            m_field_value[field] = GET_INT64_(field, isSignedVal); \
            m_field_signedMap[field] = isSignedVal; \
        } \
        else { \
            if (TRANSLATE_WITH_ASN_DOT_1_ENCODE ) { \
                AsnEncoder::FreeAsnBuf(m_pAsnDescriptor, &msgObj); \
            } \
            return; \
        } \
    } while(0);

#define FILL_FIELD_VALUE(val, type, field)  \
    val = (type*) calloc(1, sizeof(type)); \
    *(val) = m_field_value[field];
// MAPI style

// MD ICD translate macro
#define GET_ICD_MSG_BUFFER() GetICDMsgPtr()
#define GET_ICD_VERSION() GetICDVersion()
#define GET_ICD_TIMESTAMP() GetICDTimestamp()
#define GET_ICD_FRAME_NUMBER() GetICDFrameNumber()

// Translator framework macro
#define GET_TRANSLATOR_CLASS_NAME(feature, type) translator_class_##feature##_## type
#define TRANSLATE(feature, type) void GET_TRANSLATOR_CLASS_NAME(feature, type)::TranslateEx(KpiObj& obj)
#define ON_QUERY(feature, type) bool GET_TRANSLATOR_CLASS_NAME(feature, type)::Query(KpiObj& obj)
#define STORE_KPI_CACHE(obj) StoreKpiObjCache(obj, m_simId)

class BaseTranslator
{
public:
    BaseTranslator(bool supportQueryMode);
    virtual ~BaseTranslator();
    void SetListener(KpiObjListener listener, void *listenerParam);
    KPI_OBJ_RESULT_CODE Translate(
            KPI_OBJ_TYPE kpiType,
            KPI_MSGID msgId,
            KPI_SIMID simId,
            uint64_t &timestamp,
            KPI_SOURCE_TYPE source,
            KpiObj &outObj,
            uint64_t datalen,
            const void *data,
            const void *param);

    bool IsQueryModeSupport();
    KPI_OBJ_RESULT_CODE Query(
            KPI_OBJ_TYPE kpiType,
            KPI_SIMID simId,
            uint64_t &timestampUs,
            KpiObj &outObj);

    KPI_OBJ_RESULT_CODE getKpiObjWithPaylaod(
            KpiObj &outObj,
            uint64_t datalen,
            const uint8_t *data,
            uint64_t timestamp);

    const TrapTable *GetTrapTable() const;
    void CreateDumpFile();
    bool IsIncludedOid(const ObjId &oid);
    KPI_OBJ_TYPE GetKpiType() {return m_type;}
    void setObjId(ObjId *pOid) {m_pOid = pOid;}
    ObjId *getObjId() {return m_pOid;}
    void setAsnDescriptor(void *descriptor) {m_pAsnDescriptor = descriptor;}
    void SetMaxPayloadSize(unsigned int size) {m_maxPayloadSize = size;}
    unsigned int GetMaxPayloadSize(void) {return m_maxPayloadSize;}
    void SetPayloadType(OTA_PACKET_TYPE type) {m_packetType = type;}
    OTA_PACKET_TYPE GetPayloadType() {return m_packetType;}
    bool IsRegistingTrapTable() {return m_bRegistingTrapTable;}
    void setModemBaseTime(uint32_t baseRtosTime, uint64_t baseSysTimeUs);

    /* Msg Table */
    int32_t GET_INT_(const char *szFieldName, bool bSignedVal);
    bool GetInt(int32_t *val, const char *szFieldName, bool bSignedVal);
    int64_t GET_INT64_(const char *szFieldName, bool bSignedVal);
    bool GetInt64(int64_t *val, const char *szFieldName, bool bSignedVal);
    const uint8_t *GetFieldBuf(const char *szFieldName, size_t elementSize, size_t &outBufSize);

protected:
    bool m_bRegistingTrapTable;
    // Asn descriptor
    void *m_pAsnDescriptor;
    KPI_OBJ_TYPE m_type;
    std::string m_type_string;
    // The OID
    ObjId *m_pOid;
    // Default 0, no effect
    uint32_t m_reportPerodicMs;
    // An querable KPI or not
    bool m_supportQuery;
    void *m_listenerParam;
    TrapTable m_trapTable;

    bool isUserLoad();

    virtual void TranslateEx(KpiObj &obj) = 0;

    // Not support query mode in default implementation, translator class can
    // override by using macro ON_QUERY
    virtual bool Query(KpiObj &obj);
    bool StoreKpiObjCache(KpiObj &obj, KPI_SIMID simId);
    bool IsKpiObjCacheExist(KPI_SIMID simId);
    KpiObj *GetKpiObjCache(KPI_SIMID simId);

    bool InsertTrapTable(KPI_SOURCE_TYPE source, MSGID msgId, bool skipCheck);
    bool TranslatorDoneForCB;  // flag to trigger CB (for aggregate data using few EM messages)

    // IF_EM(), serve KPI_SOURCE_TYPE_MDM_EM_MSG, KPI_SOURCE_TYPE_MDM_EM_OTA
    bool CheckEmMsg(const char *szMsgName);
    // IF_ICD(), serve KPI_SOURCE_TYPE_MDM_ICD_EVENT, KPI_SOURCE_TYPE_MDM_ICD_RECORD
    bool CheckICD(const TRAP_TYPE icdType, uint64_t icdCode);
    // IF_MSG(), serve KPI_SOURCE_TYPE_APM, KPI_SOURCE_TYPE_LOCAL
    bool CheckMsg(KPI_SOURCE_TYPE source, MSGID msgId);

    // SET_REPORT_PERIODIC_MS
    void SetKpiReportPeriodic(uint32_t milliseconds) {m_reportPerodicMs = milliseconds;}

    void SkipTranslate();
    KPI_SIMID GetEventSimId();
    const void *GetTranslateData();
    int32_t GetTranslateDataVersion();
    uint32_t GetTranslateDataLength();
    const uint8_t *GetRawData(size_t &size);
    const uint8_t *GetICDMsgPtr();
    uint8_t GetICDVersion();
    uint64_t GetICDTimestamp();
    uint16_t GetICDFrameNumber();
    uint64_t GetReferenceMdTimestampUs(uint64_t currentRtosTime);

    /* Debug file */
    FILE *m_debug_fptr;
    void DumpEMTotalInfo(FILE *fp);     // for debug
    void DumpEMUsedFieldInfo(FILE *fp); // for debug em field in m_field_value
    void DumpRawData(FILE *fp, const uint8_t *data, size_t len);    // for debug

    /* Incoming data */
    KPI_MSGID m_msgId;
    KPI_SOURCE_TYPE m_sourceType;
    OTA_PACKET_TYPE m_packetType;
    //optional max payload size for some objects which support payload size restriction
    unsigned int m_maxPayloadSize;

    size_t m_len;
    const void *m_data;
    // Used for source KPI_SOURCE_TYPE_MDM_* and KPI_SOURCE_TYPE_MDM_PACKET
    const uint8_t *m_payload;
    // Used for source KPI_SOURCE_TYPE_MDM_* only
    uint16_t m_frameNumber;
    FRAME_INFO m_frameInfo;

    // SIM ID for the event
    KPI_SIMID m_simId;

    // KPI Cache
    KpiObj *m_kpiObjCache[MAX_SIM_NUM];

    // MD timestamp info
    uint32_t m_baseRtosTime;
    uint64_t m_baseSysTimeUs;

    std::map<std::string, int64_t> m_field_value;     // <field name, field value>
    std::map<std::string, bool> m_field_signedMap; // <field name, field is signedVal>
};

#endif
