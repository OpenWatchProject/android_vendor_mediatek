/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __RTST_HANDLER_H__
#define __RTST_HANDLER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/RefBase.h>
#include <Parcel.h>
#include <utils/Looper.h>
#include "RfxMainThread.h"
#include "RfxClassInfo.h"
#include "RfxController.h"

/*****************************************************************************
 * Name Space Declaration
 *****************************************************************************/
using ::android::RefBase;
using ::android::sp;
using ::android::Parcel;
using ::android::Message;

/*****************************************************************************
 * Class RtstMessage
 *****************************************************************************/
class RtstMessage : public virtual RefBase {
// Constructor / Destructor
public:
    // Constructor
    RtstMessage() {}
    // Destructor
    virtual ~RtstMessage() {}

// Overidable
public:
    virtual void handle() {}
};

/*****************************************************************************
 * Class RtstInvokeStatusCbMsg
 *****************************************************************************/
class RtstInvokeStatusCbMsg: public RtstMessage {
// Constructor / Destructor
public:
    // Constructor
    RtstInvokeStatusCbMsg(
        int slot,
        const RfxClassInfo *classInfo,
        RfxStatusKeyEnum key,
        RfxVariant oldValue,
        RfxVariant newValue);
    // Destructor
    virtual ~RtstInvokeStatusCbMsg() {}

// Overide
public:
    virtual void handle();

// Implementation
private:
    int m_slot;
    const RfxClassInfo *m_classInfo;
    RfxStatusKeyEnum m_key;
    RfxVariant m_oldValue;
    RfxVariant m_newValue;
};


/*****************************************************************************
 * Class RtstHandler
 *****************************************************************************/
class RtstHandler : public RfxMainHandler {
// External Method
public:
    // Send message to RfxMainTrhead
    //
    // RETURNS: void
    void sendMessage();

// Constructor / Destructor
public:
    // Constructor
    explicit RtstHandler(const sp<RtstMessage>& msg) : m_msg(msg) {
    }
    // Destructor
    virtual ~RtstHandler() {
    }

// Override
public:
    virtual void onHandleMessage(const Message& message);

// Implementation
private:
    sp<RtstMessage> m_msg;
    // dummy message that makes handler happy
    Message m_dummyMsg;
};
#endif /* __RTST_HANDLER_H__ */
