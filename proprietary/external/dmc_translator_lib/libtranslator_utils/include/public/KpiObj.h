
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

#ifndef __KPI_OBJ_H__
#define __KPI_OBJ_H__
#if defined (WIN32)
#include "win32_header.h"
#else
#include <stdint.h>
#endif
#include <stddef.h>
#include <string>

#include "ObjId.h"


class KpiObjValImpl;

#define KPI_OBJ_TYPE_UNDEFINED (-1)
#define KPI_GROUP_TYPE_UNDEFINED (-1)

#define KPI_TYPE_BEGIN (0)

typedef int KPI_OBJ_TYPE;
typedef uint64_t KPI_MSGID;
typedef int8_t KPI_GROUP_TYPE;

typedef enum {
    KPI_OBJ_VAL_TYPE_UNDEFINED = 0,
    KPI_OBJ_VAL_TYPE_INTEGER,
    KPI_OBJ_VAL_TYPE_STRING,
    KPI_OBJ_VAL_TYPE_BYTE_ARRAY
} KPI_OBJ_VAL_TYPE;

typedef enum {
    KPI_OBJ_RESULT_CODE_UNDEFIEND = 0,
    KPI_OBJ_RESULT_CODE_SUCESS,
    KPI_OBJ_RESULT_CODE_SKIPPED,
    KPI_OBJ_RESULT_CODE_FAIL
} KPI_OBJ_RESULT_CODE;

typedef enum {
    KPI_SOURCE_TYPE_START = 0,
    KPI_SOURCE_TYPE_LOCAL = KPI_SOURCE_TYPE_START,
    KPI_SOURCE_TYPE_APM,
    KPI_SOURCE_TYPE_PKM, // Subset of APM
    KPI_SOURCE_TYPE_MDM, // Only used for kpiRange
    KPI_SOURCE_TYPE_MDM_EM_MSG,
    KPI_SOURCE_TYPE_MDM_EM_OTA,
    KPI_SOURCE_TYPE_MDM_ICD_EVENT,
    KPI_SOURCE_TYPE_MDM_ICD_RECORD,
    KPI_SOURCE_TYPE_UNDEFINED = -1,
    KPI_SOURCE_TYPE_SIZE = 4 // Only used for kpiRange
} KPI_SOURCE_TYPE;

typedef enum {
    PACKET_TYPE_HEADER_ONLY = 0,
    PACKET_TYPE_WITH_PAYLOAD,
    PACKET_TYPE_DEFAULT = PACKET_TYPE_WITH_PAYLOAD,
    PACKET_TYPE_NONE = -1
} OTA_PACKET_TYPE;

/**
 * @brief The class is used to store the value of MDMI objects.
 */
class KpiObjVal
{
    friend class KpiObjValImpl;
public:
    KpiObjVal& operator[](const std::string &szName);
    const KpiObjVal& operator[](const std::string &szName) const;
    KpiObjVal& operator[](size_t index);
    const KpiObjVal& operator[](size_t index) const;
    size_t ArraySize() const;
    size_t MapSize() const;
    KpiObjVal& operator=(const KpiObjVal &rhs);
    long operator=(long rhs);
    long operator=(int rhs);
    long GetInt() const;
    int64_t GetInt64() const;
    bool HasMember(const std::string &szName) const;
    const char *operator=(const char *rhs);
    const char *GetStr() const;
    void SetByteArray(const uint8_t *byteArray, size_t size);
    const uint8_t *GetByteArray(size_t &size) const;
    KPI_OBJ_VAL_TYPE GetValType() const;
    bool IsNull() const;
protected:
    KpiObjVal();
    virtual ~KpiObjVal();
    void Dump(FILE *fp, int indent, int recursive = -1) const;
private:
    static const KpiObjVal nullObj;
    mutable KpiObjValImpl *m_pImpl;
};
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to represent MDMI objects including KPI and OTA messages.
 */
class KpiObj : public KpiObjVal
{
public:
    /**
     * @brief Create a empty KpiObj of type.
     *
     * @param type [IN] The object type.
     */
    KpiObj(KPI_OBJ_TYPE type): m_type(type), m_timestamp(0) {}

    /**
     * @brief Create a empty KpiObj.
     */
    KpiObj(): m_type(KPI_OBJ_TYPE_UNDEFINED), m_timestamp(0) {}

    /**
     * @brief Assignment operator for integer type.
     *
     * @param rhs [IN] The integer value to be assigned.
     *
     * @return @a rhs integer.
     */
    long operator=(long rhs) { return KpiObjVal::operator=(rhs); }
    long operator=(int rhs) { return KpiObjVal::operator=(rhs); }

    /**
     * @brief Assignment operator for string type.
     *
     * @param rhs [IN] The string to be assigned.
     *
     * @return @a rhs string.
     */
    const char *operator=(const char *rhs) { return KpiObjVal::operator=(rhs); }

    /**
     * @brief Copy assignment operator.
     *
     * @param rhs [IN] The object to be copied.
     *
     * @return reference of this object.
     */
    KpiObj& operator=(const KpiObj &rhs)
    {
        m_type = rhs.m_type;
        KpiObjVal::operator=(rhs);
        return *this;
    }
    void Dump(FILE *fp, int recursive = -1) const
    {
        if (NULL == fp) {
            return;
        }
        //fprintf(fp, "%s = {\n", GetMDMITypeName(m_type));
        KpiObjVal::Dump(fp, 1, recursive);
        fprintf(fp, "}\n");
    }

    /**
     * @brief Get MDMI object type.
     */
    KPI_OBJ_TYPE GetType() const { return m_type; }

    void SetType(KPI_OBJ_TYPE type) { m_type = type; }

    void SetTimestamp(uint64_t timestamp) { m_timestamp = timestamp; }
    uint64_t GetTimestamp(void) { return m_timestamp; }

    /**
     * @brief Null or Invalid value.
     */
    static const long NullVal;
    static const int64_t NullVal64;

protected:
    /**
     * @brief MDMI object type.
     */
    KPI_OBJ_TYPE m_type;
    uint64_t m_timestamp;
};

typedef void (*KpiObjListener)(void *listenerParam, const KpiObj &obj, const uint64_t timestamp);

#endif
