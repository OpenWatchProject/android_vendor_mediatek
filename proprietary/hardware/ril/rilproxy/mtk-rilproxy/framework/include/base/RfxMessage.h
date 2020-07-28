/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxMessage.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Declare RfxMessage which is the general message type in RIL proxy framework
 */

#ifndef __RFX_MESSAGE_H__
#define __RFX_MESSAGE_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "utils/Timers.h"
#include <limits>
#include "utils/String8.h"
#include "utils/RefBase.h"
#include "utils/Vector.h"
#include "Parcel.h"
#include <telephony/mtk_ril.h>
#include "RfxDataBase.h"
#include "RfxDebugInfo.h"
#include "RfxStatusDefs.h"
#include "RfxVariant.h"

using ::android::RefBase;
using ::android::sp;
using ::android::Parcel;
using ::android::String8;
using ::android::Vector;


typedef enum {
    C_SLOT_STATUS_NOT_CURRENT_SLOT,  // 0: C Slot is not the current slot;
    C_SLOT_STATUS_IS_CURRENT_SLOT,  //  1: C Slot is the current slot;
    C_SLOT_STATUS_OTHERS  // 2 :NA
} RequestCSlotStatus;

/*****************************************************************************
 * Class RfxMessage
 *****************************************************************************/

class RfxMessage : public virtual RefBase
             #ifdef RFX_OBJ_DEBUG
                 , public IRfxDebugLogger
             #endif
{
private:

    RfxMessage();

    // RfxMessage can't be copied
    RfxMessage(const RfxMessage& o);
    RfxMessage& operator=(const RfxMessage& o);

    virtual ~RfxMessage();

    static int generateToken();
    static void resetToken();
    static int generatePToken();
    static void resetPToken();
    const char* typeToString(RFX_MESSAGE_TYPE type) const;
    const char* groupToString(RILD_RadioTechnology_Group group) const;

private:

    RFX_MESSAGE_TYPE type;  // msg type: request, response, urc
    RILD_RadioTechnology_Group source;  // msg source: gsm, c2k
    RILD_RadioTechnology_Group dest;  // msg destination: gsm, c2k
    int pId;  // parent msg id
    int pToken;  // parent msg token
    int id;  // msg id
    int token;  // msg token
    int slotId;  // slot id
    int clientId;
    Parcel *parcel;  // msg payload
    RIL_Errno error;  // response error
    nsecs_t timeStamp;
    nsecs_t pTimeStamp;
    mutable RfxDataBase *dataObj;  // rfx data object (transfer from parcel)
    RequestCSlotStatus isSentOnCdmaCapabilitySlot = C_SLOT_STATUS_OTHERS;

    static pthread_mutex_t token_mutex;
    static int tokenCounter;
    static pthread_mutex_t pTokenMutex;
    static int pTokenCounter;
    RIL_Token rilToken;

    // for StatusManager
    RfxStatusKeyEnum key;
    RfxVariant value;
    bool force_notify;
    bool is_default;

public:
    int getId() const {
        return id;
    }
    int getToken() const {
        return token;
    }
    int getPId() const {
        return pId;
    }
    int getPToken() const {
        return pToken;
    }
    Parcel* getParcel() const {
        return parcel;
    }
    int getSlotId() const {
        return slotId;
    }
    int getClientId() const {
        return clientId;
    }
    RILD_RadioTechnology_Group getSource() const {
        return source;
    }
    RILD_RadioTechnology_Group getDest() const {
        return dest;
    }
    RFX_MESSAGE_TYPE getType() const {
        return type;
    }
    RIL_Errno getError() const {
        return error;
    }
    nsecs_t getTimeStamp() const {
        return timeStamp;
    }
    nsecs_t getPTimeStamp() const {
        return pTimeStamp;
    }
    RfxDataBase* getDataObj() const {
        return dataObj;
    }
    RIL_Token getRilToken() const {
        return rilToken;
    }
    void setSlotId(int id) {
        slotId = id;
    }
    void setDataObj(RfxDataBase *obj) {
        dataObj = obj;
    }
    RequestCSlotStatus getSentOnCdmaCapabilitySlot() {
        return isSentOnCdmaCapabilitySlot;
    }
    void setSentOnCdmaCapabilitySlot(RequestCSlotStatus cSlotHere) {
        isSentOnCdmaCapabilitySlot = cSlotHere;
    }
        RfxStatusKeyEnum getStatusKey() const {
        return key;
    }
    RfxVariant getStatusValue() const {
        return value;
    }
    bool getForceNotify() const {
        return force_notify;
    }
    bool getIsDefault() const {
        return is_default;
    }
    bool resetParcelDataStartPos();
    String8 toString() const;

    static sp<RfxMessage> obtainRequest(RILD_RadioTechnology_Group target, int id,
            const sp<RfxMessage>& msg);
    static sp<RfxMessage> obtainRequest(RILD_RadioTechnology_Group target, int id,
            const sp<RfxMessage>& msg, bool copyParcel);
    static sp<RfxMessage> obtainRequest(int slotId, RILD_RadioTechnology_Group target, int id);
    static sp<RfxMessage> obtainRequest(int slotId, RILD_RadioTechnology_Group target, int pId,
            int pToken, Parcel* parcel);
    static sp<RfxMessage> obtainRequest(int slotId, RILD_RadioTechnology_Group target, int pId,
            int pToken, Parcel* parcel, int clientId);
    static sp<RfxMessage> obtainRequest(int slotId, RILD_RadioTechnology_Group target, int pId,
            int pToken, Parcel* parcel, RIL_Token t);

    // RequestAck obtain function
    static sp<RfxMessage> obtainRequestAck(int slotId, int pId, int pToken, int id,
                int token, RILD_RadioTechnology_Group source, Parcel* parcel);
    static sp<RfxMessage> obtainRequestAck(int slotId, int pId, int pToken, int id,
                int token, RILD_RadioTechnology_Group source, Parcel* parcel,
                nsecs_t pTimeStamp);
    static sp<RfxMessage> obtainRequestAck(int slotId, int pId, int pToken, int id,
                int token, RILD_RadioTechnology_Group source, Parcel* parcel, int clientId,
                nsecs_t pTimeStamp);
    static sp<RfxMessage> obtainRequestAck(int slotId, int pId, int pToken, int id,
                int token, RILD_RadioTechnology_Group source, Parcel* parcel,
                nsecs_t pTimeStamp, RIL_Token t);

    // Response obtain function
    static sp<RfxMessage> obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel);
    static sp<RfxMessage> obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp);
    static sp<RfxMessage> obtainResponse(int slotId, int pid, int ptoken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel,
            int clientId, nsecs_t pTimeStamp);
    static sp<RfxMessage> obtainResponse(int slotId, int pId, int pToken, int id,
            int token, RIL_Errno e, RILD_RadioTechnology_Group source, Parcel* parcel,
            nsecs_t pTimeStamp, RIL_Token t);
    static sp<RfxMessage> obtainResponse(RIL_Errno e, const sp<RfxMessage>& msg);
    static sp<RfxMessage> obtainResponse(RIL_Errno e, const sp<RfxMessage>& msg,
            bool copyParcel);
    static sp<RfxMessage> obtainResponse(int slotId, RILD_RadioTechnology_Group source,
            RIL_Errno e, const sp<RfxMessage>& msg, bool copyParcel);

    // URC obtain function
    static sp<RfxMessage> obtainUrc(int slotId, int urcId, RILD_RadioTechnology_Group source,
            Parcel* parcel);
    static sp<RfxMessage> obtainUrc(int slotId, int urcId);
    static sp<RfxMessage> obtainUrc(int slotId, int urcId, const sp<RfxMessage>& msg,
            bool copyParcel);

    static sp<RfxMessage> obtainStatusSync(int slotId, RfxStatusKeyEnum key,
        const RfxVariant value, bool force_notify, bool is_default);
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const;
    static void dumpMsgList();

private:
    RfxDebugInfo *m_debug_info;
    static Vector<RfxDebugInfo*> *s_obj_debug_info;
#endif //#ifdef RFX_OBJ_DEBUG

};
#endif // __RFX_MESSAGE_H__
