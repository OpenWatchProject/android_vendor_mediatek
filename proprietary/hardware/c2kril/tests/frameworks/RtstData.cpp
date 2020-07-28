/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <stdarg.h>
#include "RtstData.h"
#include "RtstUtils.h"
#include "RtstEnv.h"
#include "RtstLog.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define TAG "RTF"

/*****************************************************************************
 * Class RtstData
 *****************************************************************************/
RtstData::RtstData() {
}

void RtstData::setData(const char * data) {
    m_data.setTo(data);
}

/*****************************************************************************
 * Class RtstVoidData
 *****************************************************************************/
AssertionResult RtstVoidData::checkParcel(Parcel &p) {
    RTST_UNUSED(p);
    return AssertionSuccess();
}

/*****************************************************************************
 * Class RtstInt8Data
 *****************************************************************************/
void RtstInt8Data::toParcel(Parcel &p) {
    RTST_LOG_V(TAG, "RtstInt8Data::toParcel");
    uint8_t uct = toInt8();
    RTST_LOG_V(TAG, "uct:%02X", uct);
    p.write(&uct, sizeof(uct));
}

AssertionResult RtstInt8Data::checkParcel(Parcel &p) {
    uint8_t value;
    p.read(&value, sizeof(value));
    if (toInt8() != value) {
        return AssertionFailure() << "INT8 value mismatch!"
                                  << " Expected: " << toInt8()
                                  << " Actual: " << value;
    }
    return AssertionSuccess();
}

uint8_t RtstInt8Data::toInt8() {
    //In case the string representation of the number begins with a 0x prefix, one must should use 0 as base:
    uint8_t value = (uint8_t)strtol(getData().string(), NULL, 0);
    return value;
}

/*****************************************************************************
 * Class RtstInt32Data
 *****************************************************************************/
void RtstInt32Data::toParcel(Parcel &p) {
    RTST_LOG_V(TAG, "RtstInt32Data::toParcel");
    p.writeInt32(toInt32());
}

AssertionResult RtstInt32Data::checkParcel(Parcel &p) {
    int32_t value;
    p.readInt32(&value);
    if (toInt32() != value) {
        return AssertionFailure() << "INT32 value mismatch!"
                                  << " Expected: " << toInt32()
                                  << " Actual: " << value;
    }
    return AssertionSuccess();
}

int32_t RtstInt32Data::toInt32() {
    return atoi(getData().string());
}


/*****************************************************************************
 * Class RtstInt64Data
 *****************************************************************************/
void RtstInt64Data::toParcel(Parcel &p) {
    RTST_LOG_V(TAG, "RtstInt64Data::toParcel");
    p.writeInt64(toInt64());
}


AssertionResult RtstInt64Data::checkParcel(Parcel &p) {
    int64_t value;
    p.readInt64(&value);
    if (toInt64() != value) {
        return AssertionFailure() << "INT64 value mismatch!"
                                  << " Expected: " << toInt64()
                                  << " Actual: " << value;
    }
    return AssertionSuccess();
}


int64_t RtstInt64Data::toInt64() {
    return atoll(getData().string());
}


/*****************************************************************************
 * Class RtstStringData
 *****************************************************************************/
void RtstStringData::toParcel(Parcel &p) {
    RTST_LOG_D(TAG, "RtstStringData::toParcel");
    RtstUtils::writeStringToParcel(p, toString());
}


AssertionResult RtstStringData::checkParcel(Parcel &p) {
    char *str = RtstUtils::getStringFromParcel(p);
    if (str == NULL) {
        if (*toString() == 0) {
            return AssertionSuccess();
        } else {
            return AssertionFailure() << "String value mismatch!"
                                  << " Expected: " << toString()
                                  << " Actual Empty String";
        }
    }
    if (strcmp(str, toString()) != 0) {
        String8 temp(str);
        free(str);
        return AssertionFailure() << "String value mismatch!"
                                  << " Expected: " << toString()
                                  << " Actual: " << temp.string();
    }
    free(str);
    return AssertionSuccess();
}


const char* RtstStringData::toString() {
    return getData().string();
}


/*****************************************************************************
 * Class RtstRawData
 *****************************************************************************/
void RtstRawData::toParcel(Parcel &p) {
    RTST_LOG_D(TAG, "RtstRawData::toParcel");
    Vector<char> raw;
    toRaw(raw);
    int len = raw.size();
    if (len > 0) {
        p.writeInt32(len);
        p.write(raw.array(), len);
    } else {
        p.writeInt32(-1);
    }
}


AssertionResult RtstRawData::checkParcel(Parcel &p) {
    int len;
    p.readInt32(&len);
    Vector<char> raw;
    toRaw(raw);
    if ((int)raw.size() != len) {
        return AssertionFailure() << "Raw value length  mismatch!"
                                  << " Expected: " << raw.size()
                                  << " Actual: " << len;
    } else if (len > 0) {
        unsigned char *x = (unsigned char *)p.readInplace(len);
        if (memcmp(x, raw.array(), len) != 0) {
            return AssertionFailure() << "Raw value mismatch!";
        }
    }
    return AssertionSuccess();
}


void RtstRawData::toRaw(Vector<char> &raw) {
    int len = getRawLen();
    char *p = new char[len];
    getRaw(p, len);
    raw.appendArray(p, len);
    delete[] p;
}


int RtstRawData::getRawLen() const {
    RTST_ASSERT(getData().size() %2 == 0);
    return (getData().size() / 2);
}


void RtstRawData::getRaw(char *buf, int len) const {
    RTST_ASSERT(len == getData().size() / 2);
    RtstUtils::hex2Bin(getData().string(), buf, len);
}


/*****************************************************************************
 * Class RtstDataCreator
 *****************************************************************************/
RtstData::DataType RtstDataCreator::getDataType(const char *type) {
    const char *types[] =
        {"undefined", "int8", "int32", "int64", "string", "void", "raw"};
    for (int i = 0; i < RtstData::DATA_TYPE_MAX ; i++) {
        if (strcmp(type, types[i]) == 0) {
            return (RtstData::DataType)i;
        }
    }
    return RtstData::UNDEFINE;
}


RtstData * RtstDataCreator::createData(const char *type) {
    RtstData::DataType t = getDataType(type);
    switch (t) {
        case RtstData::INT8:
            return new RtstInt8Data();
        case RtstData::INT32:
            return new RtstInt32Data();
        case RtstData::INT64:
            return new RtstInt64Data();
        case RtstData::STRING:
            return new RtstStringData();
        case RtstData::RAW:
            return new RtstRawData();
        case RtstData::VOID:
            return new RtstVoidData();
        default:
            return new RtstData();
    }
}


/*****************************************************************************
 * Class RtstSeq
 *****************************************************************************/
void RtstSeq::toParcel(Parcel & p, Vector<RtstData *> &seq) {
    Vector<RtstData *>::iterator it;
    for (it = seq.begin(); it != seq.end(); it++) {
        (*it)->toParcel(p);
    }
}

AssertionResult RtstSeq::checkParcel(Parcel &p, Vector<RtstData *> &seq) {
    Vector<RtstData *>::iterator it;
    int i = 0;
    for (it = seq.begin(); it != seq.end(); it++) {
        AssertionResult r = (*it)->checkParcel(p);
        if (r != AssertionSuccess()) {
            return r << " data index is " << i;
        }
        i++;
    }
    return AssertionSuccess();
}


/*****************************************************************************
 * Class RtstCountedSeq
 *****************************************************************************/
void RtstCountedSeq::toParcel(Parcel &p, Vector < RtstData * > &seq) {
    RTST_LOG_D(TAG, "RtstCountedSeq::toParcel %zu", seq.size());
    p.writeInt32(seq.size());
    RtstSeq::toParcel(p, seq);
}

AssertionResult RtstCountedSeq::checkParcel(Parcel &p,
    Vector < RtstData * > &seq) {
    int32_t len;
    p.readInt32(&len);
    if (len != (int)seq.size()) {
        return AssertionFailure() << "Parcel length mismatch!"
                                  << " Expected: " << seq.size()
                                  << " Actual: " << len;
    }
    return RtstSeq::checkParcel(p, seq);
}


/*****************************************************************************
 * Class RtstDataSequency
 *****************************************************************************/
RtstDataSequency::RtstDataSequency() :m_seqTypePtr(NULL), m_seqCalType(AUTO) {
}

RtstDataSequency::~RtstDataSequency() {
    if (m_seqTypePtr != NULL) {
        delete m_seqTypePtr;
    }
    Vector<RtstData *>::iterator it;
    for (it = m_sequence.begin(); it != m_sequence.end(); it++) {
        delete (*it);
    }
}


bool RtstDataSequency::isSameTypeWith(RtstData::DataType type) {
    Vector<RtstData*>::iterator it;
    for (it = m_sequence.begin(); it != m_sequence.end(); it++) {
        if ((*it)->getType() != type) {
            return false;
        }
    }
    return true;
}

bool RtstDataSequency::isMultiple() {
   return (m_sequence.size() >= 2);
}

bool RtstDataSequency::isStrings() {
    return isMultiple() && isSameTypeWith(RtstData::STRING);
}

bool RtstDataSequency::isInt8s() {
    return isSameTypeWith(RtstData::INT8);
}

bool RtstDataSequency::isInt32s() {
    return isSameTypeWith(RtstData::INT32);
}

void RtstDataSequency::calSeqType() {
    if (m_seqTypePtr != NULL) {
        delete m_seqTypePtr;
    }
    if (m_seqCalType == AUTO) {
        if (isStrings() || isInt32s()) {
            m_seqTypePtr = new RtstCountedSeq();
        } else {
            m_seqTypePtr = new RtstSeq();
        }
    } else if (m_seqCalType == FORCE_NON_COUNTABLE) {
        m_seqTypePtr = new RtstSeq();
    } else if (m_seqCalType == FORCE_COUNTABLE) {
        m_seqTypePtr = new RtstCountedSeq();
    }
}


void RtstDataSequency::toParcel(Parcel &p) {
    calSeqType();
    m_seqTypePtr->toParcel(p, m_sequence);
}

AssertionResult RtstDataSequency::checkParcel(Parcel & p) {
    calSeqType();
    return m_seqTypePtr->checkParcel(p, m_sequence);
}

void RtstDataSequency::appendWith(int num, ...) {
    va_list argp;
    va_start(argp, num);
    appendWithVaList(num, argp);
    va_end(argp);
}


void RtstDataSequency::appendWithVaList(int num, va_list argp) {
    const char * type;
    const char *data;
    for (int i = 0; i < num; i++) {
        type = va_arg(argp, const char* );
        data = va_arg(argp, const char* );
        RtstData *p = m_creator.createData(type);
        p->setData(data);
        m_sequence.push(p);
    }
}


void RtstDataSequency::getDatas(Vector<const char *> &datas) {
    Vector<RtstData*>::iterator it;
    int i;
    for (it = m_sequence.begin(), i = 0; it != m_sequence.end(); it++, i++) {
        datas.push((*it)->getData().string());
    }
}


/*****************************************************************************
 * Class RtstItemBase
 *****************************************************************************/
AssertionResult RtstItemBase::run() {
    if (isMockType()) {
        sendData();
        return AssertionSuccess();
    } else if (isExpectedType()) {
        return checkData();
    } else {
        return AssertionFailure();
    }
}


/*****************************************************************************
 * Class RtstRilReqItem
 *****************************************************************************/
void RtstRilReqItem::sendData() {
    Parcel p;
    getDataSequency().toParcel(p);
    p.setDataPosition(0);
    RtstEnv::get()->sendRilRequest(getRilCmdId(), getSlotId(), p);
}

/*****************************************************************************
 * Class RtstExpectedRilRspItem
 *****************************************************************************/
AssertionResult RtstExpectedRilRspItem::checkData() {
    int reqId;
    int error;
    Parcel p;
    bool r;
    RTST_LOG_D(TAG, "getExpectedRilRsp slotId = %d, error = %d, reqId = %d",
            getSlotId(), getErrorCode(), getRilCmdId());
    r = RtstEnv::get()->getExpectedRilRsp(getSlotId(), reqId, error, p);
    if (!r) {
        return AssertionFailure() << "getExpectedRilRsp time out";
    }
    if (reqId != getRilCmdId()) {
        return AssertionFailure() << "Check RIL RSP"
                                  << " req ID mismatch!"
                                  << " Expected: " << getRilCmdId()
                                  << " Actual: " << reqId;
    }
    if (error != getErrorCode()) {
        return AssertionFailure() << "Check RIL RSP"
                                  << " error code mismatch!"
                                  << " Expected: " << getErrorCode()
                                  << " Actual: " << error;
    }
    return getDataSequency().checkParcel(p);
}

/*****************************************************************************
 * Class RtstExpectedRilUrcItem
 *****************************************************************************/
AssertionResult RtstExpectedRilUrcItem::checkData() {
    int urcId;
    Parcel p;
    bool r;
    RTST_LOG_D(TAG, "getExpectedRilUrc slodId = %d, urcId = %d",
            getSlotId(), getRilCmdId());
    r = RtstEnv::get()->getExpectedRilUrc(getSlotId(), urcId, p);
    if (!r) {
        return AssertionFailure() << "RtstExpectedRilUrcItem time out";
    }
    if (urcId != getRilCmdId()) {
        return AssertionFailure() << "Check RIL URC"
                                  << " URC ID mismatch!"
                                  << " Expected: " << getRilCmdId()
                                  << " Actual: " << urcId;
    }
    return getDataSequency().checkParcel(p);
}


/*****************************************************************************
 * Class RtstExpectedAtItem
 *****************************************************************************/
AssertionResult RtstExpectedAtItem::checkData() {
    bool r;
    String8 expectedAt;
    Vector<const char *> datas;
    getDataSequency().getDatas(datas);

    RTST_LOG_D(TAG, "getExpectedAt channelId = %d, slotId = %d, expectedAT = %s",
            getChannelId(), getSlotId(), datas[0]);
    r = RtstEnv::get()->getExpectedAt(getChannelId(), getSlotId(), expectedAt);
    if (!r) {
        return AssertionFailure() << "RtstExpectedAtItem time out"
                                  << " channel " << getChannelId()
                                  << " Slot " << getSlotId();
    }
    if (String8(datas[0]) != expectedAt) {
        return AssertionFailure() << "AT mismatch "
                                  << " Expected: " << datas[0]
                                  << " Actual: "   << expectedAt.string();
    }
    return AssertionSuccess();
}

/*****************************************************************************
 * Class RtstUrcStringItem
 *****************************************************************************/
void RtstUrcStringItem::sendData() {
    Vector<const char *> datas;
    getDataSequency().getDatas(datas);
    RtstEnv::get()->sendUrcString(
        getChannelId(), getSlotId(), datas[0]);
}

/*****************************************************************************
 * Class RtstAtResponseItem
 *****************************************************************************/
void RtstAtResponseItem::sendData() {
    Vector<const char *> datas;
    getDataSequency().getDatas(datas);
    Vector<const char *>::iterator it;
    for (it = datas.begin(); it != datas.end(); it++) {
        RtstEnv::get()->sendAtResponse(
            getChannelId(), getSlotId(), (*it));
    }
}

/*****************************************************************************
 * Class RtstSysPropItem
 *****************************************************************************/
void RtstSysPropItem::sendData() {
   RtstEnv::get()->setSysProp(getKey().string(), getValue().string());
}

/*****************************************************************************
 * Class RtstExpectedSysPropItem
 *****************************************************************************/
AssertionResult RtstExpectedSysPropItem::checkData() {
    String8 value;
    RtstEnv::get()->getExpectedSysProp(getKey(), value, m_delay);
    if (value == getValue()) {
        return AssertionSuccess();
    } else {
        return AssertionFailure() << "System Property mismatch!"
                                  << " Expected: " << getValue().string()
                                  << "Actaul " << value.string();
    }
}

/*****************************************************************************
 * Class RtstCase
 *****************************************************************************/
RtstCase::~RtstCase() {
    Vector<RtstItemBase *>::iterator it;
    for (it = m_caseItems.begin(); it != m_caseItems.end(); it++) {
        delete (*it);
    }
}

void RtstCase::put(RtstItemBase * item) {
    m_caseItems.push(item);
}

int RtstCase::generateAtDatas(int slot, int channel, const char *at, int num, ...) {
    RtstExpectedAtItem *expectedAt =  new RtstExpectedAtItem(channel, slot);
    expectedAt->getDataSequency().appendWith(1, "string", at);
    int index = m_caseItems.add(expectedAt);

    RtstAtResponseItem *atResponses = new RtstAtResponseItem(channel, slot);
    va_list argp;
    va_start(argp, num);
    for (int i = 0; i < num; i++) {
         const char *data;
         data = va_arg(argp, const char* );
         atResponses->getDataSequency().appendWith(1, "string", data);
    }
    put(atResponses);
    va_end(argp);
    return index;
}

AssertionResult RtstCase::stepRun(RtstItemBase * item, int num, ...) {
    va_list argp;
    va_start(argp, num);
    item->getDataSequency().appendWithVaList(num, argp);
    va_end(argp);
    put(item);
    if (isStepRun()) {
        return item->run();
    } else {
        return AssertionSuccess();
    }
}

AssertionResult RtstCase::runAt(unsigned int index) {
    if (isStepRun()) {
        RTST_ASSERT(((index + 1) < m_caseItems.size()));
        AssertionResult result = m_caseItems[index]->run();
        if (result != AssertionSuccess()) {
            return result;
        }
        m_caseItems[index + 1]->run();
    }
    return AssertionSuccess();
}

AssertionResult RtstCase::finalRun() {
    if (!isStepRun()) {
        Vector<RtstItemBase *>::iterator it;
        for (it = m_caseItems.begin(); it != m_caseItems.end(); it++) {
            AssertionResult result = (*it)->run();
            if (result != AssertionSuccess()) {
                return result;
            }
        }
    }
    return AssertionSuccess();
}
