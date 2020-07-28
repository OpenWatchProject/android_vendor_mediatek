/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RFX_MCL_MESSAGE__H__
#define __RFX_MCL_MESSAGE__H__

#include "utils/RefBase.h"
#include <utils/Timers.h>
#include <telephony/mtk_ril.h>
#include "RfxDefs.h"
#include "RfxBaseData.h"
#include "RfxDataCloneManager.h"
#include "RfxAtLine.h"
#include "RfxStatusDefs.h"
#include "RfxVariant.h"

using ::android::RefBase;
using ::android::sp;

class RfxMclMessage : public virtual RefBase {
private:
    RfxMclMessage();
    virtual ~RfxMclMessage();

public:
    RFX_MESSAGE_TYPE getType() const {
        return m_type;
    }
    int getId() const {
        return m_id;
    }
    int getChannelId() const {
        return m_channel_id;
    }
    void setChannelId(int channelId) {
        m_channel_id = channelId;
    }
    int getSlotId() const {
        return m_slot_id;
    }
    int getClientId() const {
        return m_client_id;
    }
    RfxBaseData *getData() const {
        return m_data;
    }
    RfxAtLine* getRawUrc() const {
        return m_raw_urc;
    }
    RfxAtLine* getRawUrc2() const {
        return m_raw_urc2;
    }
    RIL_Errno getError() const {
        return m_err;
    }
    void setError(RIL_Errno err) {
        m_err = err;
    }
    /*void setData(void *data, int length) {
        RfxBaseData *response_data = RfxDataCloneManager::copyData(m_id, data, length);
        m_response_data = response_data;
    }*/
    nsecs_t getDelayTime() const {
        return m_delayTime;
    }
    int getToken() const {
        return m_token;
    }
    MTK_RIL_REQUEST_PRIORITY getPriority() const {
        return m_priority;
    }
    void setPriority(MTK_RIL_REQUEST_PRIORITY priority) {
        m_priority = priority;
    }
    RfxStatusKeyEnum getStatusKey() const {
        return m_key;
    }
    RfxVariant getStatusValue() const {
        return m_value;
    }
    bool getForceNotify() const {
        return m_force_notify;
    }
    bool getIsDefault() const {
        return m_is_default;
    }
    bool getIsUpdateForMock() const {
        return m_update_for_mock;
    }
    void setSendToMainProtocol(bool toMainProtocol) {
        mSendToMainProtocol = toMainProtocol;
    }
    bool getSendToMainProtocol() {
        return mSendToMainProtocol;
    }
    RIL_Token getRilToken() const {
        return rilToken;
    }

    String8 toString() const;

    int getMainProtocolSlotId() const {
        return m_main_protocol_slot_id;
    }

    void setMainProtocolSlotId(int mainProtocolSlotId) {
        m_main_protocol_slot_id = mainProtocolSlotId;
    }

    nsecs_t getTimeStamp() const {
        return mTimeStamp;
    }

    bool getAddAtFront() const {
        return mAddAtFront;
    }

    void setAddAtFront(bool value) {
        mAddAtFront = value;
    }

    public:
        static sp<RfxMclMessage> obtainRequest(int id, RfxBaseData *data, int slot_id,
                int token, bool sendToMainProtocol, RIL_Token rilToken, nsecs_t timeStamp,
                bool addAtFront);

        static sp<RfxMclMessage> obtainRequest(int id, RfxBaseData *data, int slot_id,
                int token, bool sendToMainProtocol, RIL_Token rilToken, nsecs_t delayTime,
                nsecs_t timeStamp, bool addAtFront);

        static sp<RfxMclMessage> obtainResponse(int id, RIL_Errno err, const RfxBaseData &data,
                sp<RfxMclMessage> msg, bool copyData = false);

        static sp<RfxMclMessage> obtainResponse(RIL_Errno err, const RfxBaseData &data,
                sp<RfxMclMessage> msg);

        // framework use only, for raw URC
        static sp<RfxMclMessage> obtainRawUrc(int channel_id, RfxAtLine* line1,
                RfxAtLine* line2);

        static sp<RfxMclMessage> obtainUrc(int id, int slot_id, const RfxBaseData &data);

        static sp<RfxMclMessage> obtainEvent(int id, const RfxBaseData &data, int channel_id,
                int slot_id, int client_id = -1, int token = -1, nsecs_t delayTime = 0,
                MTK_RIL_REQUEST_PRIORITY priority =
                MTK_RIL_REQUEST_PRIORITY::MTK_RIL_REQUEST_PRIORITY_MEDIUM);

        static sp<RfxMclMessage> obtainStatusSync(int slot_id, RfxStatusKeyEnum key,
                const RfxVariant value, bool force_notify, bool is_default,
                bool update_for_mock = false);

        // SAP
        static sp<RfxMclMessage> obtainSapRequest(int id, RfxBaseData *data, int slot_id,
                int token);

        static sp<RfxMclMessage> obtainSapResponse(int id, RIL_Errno e,
                const RfxBaseData &data, sp<RfxMclMessage> msg, bool copyData = false);

        static sp<RfxMclMessage> obtainSapUrc(int id, int slot_id, const RfxBaseData &data);

private:

    RFX_MESSAGE_TYPE m_type;  // msg type: request, response, urc
    int m_id;
    RfxBaseData *m_data;
    int m_channel_id;
    int m_slot_id;
    int m_client_id;
    int m_token; // pair with RfxMessage
    RfxAtLine *m_raw_urc;
    RfxAtLine *m_raw_urc2; // for SMS
    RIL_Errno m_err;
    nsecs_t m_delayTime;
    MTK_RIL_REQUEST_PRIORITY m_priority;
    RIL_Token rilToken;

    // for StatusManager
    RfxStatusKeyEnum m_key;
    RfxVariant m_value;
    bool m_force_notify;
    bool m_is_default;
    bool m_update_for_mock;

    bool mSendToMainProtocol;
    int m_main_protocol_slot_id;
    nsecs_t mTimeStamp;
    bool mAddAtFront;
};

#endif
