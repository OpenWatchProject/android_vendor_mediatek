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

#ifndef __RFX_CALL_STATE_H
#define __RFX_CALL_STATE_H


/*****************************************************************************
 * Include
 *****************************************************************************/
#include "Rfx.h"
#include "utils/String8.h"

using ::android::String8;

/* Define call type. */
typedef enum {
    CALL_TYPE_UNKNOWN = -1,
    CALL_TYPE_C2K = 0,
    CALL_TYPE_GSM = 1,
    CALL_TYPE_IMS = 2
} CallTypeEnum;

/* Define call state. */
typedef enum {
    CALL_STATE_UNKNOWN = -1,
    CALL_STATE_CONNECTED = 0,
    CALL_STATE_DISCONNECTED = 1
} CallStateEnum;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RfxCallState {
public:
    RfxCallState();

    RfxCallState(CallTypeEnum callType, CallStateEnum callState, int callCount);

    ~RfxCallState();

public:
    CallTypeEnum getCallType();
    CallStateEnum getCallState();
    int getCallCount();
    void setCallType(CallTypeEnum callType);
    void setCallState(CallStateEnum callState);
    void setCallCount(int count);
    String8 toString();

private:
    CallTypeEnum mCallType;
    CallStateEnum mCallState;
    int mCallCount;

public:
    bool hasConnectedC2kCall();
    bool hasConnectedGsmCall();
    bool hasConnectedImsCall();

    bool equalTo(const RfxCallState &other) const {
        return (mCallType == other.mCallType)
                && (mCallState == other.mCallState)
                && (mCallCount == other.mCallCount);
    }

    RfxCallState &operator = (const RfxCallState &other) {
        mCallType = other.mCallType;
        mCallState = other.mCallState;
        mCallCount = other.mCallCount;
        return *this;
    }

    bool operator == (const RfxCallState &other) {
        return equalTo(other);
    }

    bool operator != (const RfxCallState &other) {
        return !equalTo(other);
    }
};

#endif /* __RFX_NW_CALL_STATE_H */
