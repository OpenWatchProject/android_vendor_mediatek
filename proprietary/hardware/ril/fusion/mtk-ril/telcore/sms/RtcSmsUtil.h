/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __RTC_SMS_UTIL_H__
#define __RTC_SMS_UTIL_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include <utils/RefBase.h>
#include <utils/Parcel.h>
#include "RfxBasics.h"
#include "RfxMainThread.h"

/*****************************************************************************
 * Class Declaraion
 *****************************************************************************/


/*****************************************************************************
 * Namespace Declaration
 *****************************************************************************/
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;
using ::android::Looper;
using ::android::RefBase;
using ::android::Parcel;

/*****************************************************************************
 * Defines
 *****************************************************************************/
#define SMS_TAG "sms-ril"

/*****************************************************************************
 * Class RtcSmsParsingMessage
 *****************************************************************************/
class RtcSmsParsingMessage : public virtual RefBase {
private:
    RtcSmsParsingMessage() : m_id(-1), m_parcel(NULL) {}
    RtcSmsParsingMessage(const RtcSmsParsingMessage& o);
    RtcSmsParsingMessage& operator=(const RtcSmsParsingMessage& o);
    virtual ~RtcSmsParsingMessage();

public:
    int32_t getId() const {
        return m_id;
    }

    Parcel* getParcel() const {
        return m_parcel;
    }

    static sp<RtcSmsParsingMessage> obtainMessage(
        int32_t id,
        Parcel* parcel);

private:
    int32_t m_id;
    Parcel *m_parcel;
};


/*****************************************************************************
 * Class RtcSmsHandler
 *****************************************************************************/
/*
 * Base handler to handle SMS message
 */
class RtcSmsHandler : public RfxMainHandler {
public:
    explicit RtcSmsHandler(const sp<RtcSmsParsingMessage>& msg) : m_msg(msg) {}
    // Destructor
    virtual ~RtcSmsHandler() {}
    // Send SMS message
    void sendMessage(sp<Looper> looper);

protected:
    // SMS message referance
    sp<RtcSmsParsingMessage> m_msg;

    // dummy message that makes handler happy
    Message m_dummyMsg;
};

/*****************************************************************************
 * Class RtcSmsParsingThreadHandler
 *****************************************************************************/
/*
 * Handler that is used to send message to SMS parsing thread
 */
class RtcSmsParsingThreadHandler: public RtcSmsHandler {
public:
    // Constructor
    explicit RtcSmsParsingThreadHandler(
        const sp<RtcSmsParsingMessage>& msg)  // [IN] the SMS message
        : RtcSmsHandler(msg) {}

    // Destructor
    virtual ~RtcSmsParsingThreadHandler() {}

// Override
public:
    // Override handleMessage, don't the watch dog in RfxMainHandler
    virtual void handleMessage(const Message& message);
    virtual void onHandleMessage(const Message& message) { RFX_UNUSED(message);}
};

#endif /* __RTC_SMS_UTIL_H__ */
