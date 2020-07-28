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

#ifndef __RFX_PRECISE_CALL_STATE_H
#define __RFX_PRECISE_CALL_STATE_H


/*****************************************************************************
 * Include
 *****************************************************************************/
#include "Rfx.h"
#include "utils/String16.h"
#include "utils/String8.h"

using ::android::String16;
using ::android::String8;

typedef enum {
    CALL_TYPE_NONE = -1,
    CALL_TYPE_EMERGENCY = 0,
    CALL_TYPE_VOICE = 1,
    CALL_TYPE_VIDEO = 2,
} CallType;

typedef enum {
    CALL_RAT_NONE = -1,
    CALL_RAT_NO_SERIVCE = 0,
    CALL_RAT_GSM = 1,
    CALL_RAT_UMTS = 2,
    CALL_RAT_LTE = 3,
    CALL_RAT_CDMA = 4,
    CALL_RAT_WIFI = 5,
    CALL_RAT_NR = 6,
} CallRat;

typedef enum {
    CALL_DIR_NONE = -1,
    CALL_DIR_MO = 0,
    CALL_DIR_MT = 1,
} CallDirection;


typedef enum {
    ORIG_NONE = -1,
    ORIG_ACTIVE = 0,
    ORIG_HOLDING = 1,
    ORIG_DIALING = 2,    /* MO call only */
    ORIG_ALERTING = 3,   /* MO call only */
    ORIG_INCOMING = 4,   /* MT call only */
    ORIG_WAITING = 5,    /* MT call only */
    ORIG_DISCONNECTING = 6,
    ORIG_FOREGND_DISCONNECTING = 7,
    ORIG_BACKGND_DISCONNECTING = 8,
    ORIG_DISCONNECTED = 9,
} OrigState;


/*****************************************************************************
 * Class RfxPreciseCallState
 *****************************************************************************/

class RfxPreciseCallState {
public:
    RfxPreciseCallState();
    virtual ~RfxPreciseCallState();
    static OrigState RILStateToOrigState(RIL_CallState state);
    bool isValid();
    void dump();

public:
    int             mSlot;
    int             mCallId;
    CallType        mCallType;
    CallRat         mCallRat;
    CallDirection   mCallDir;
    String16        mCallNumber;
    OrigState       mOrigState;
};

#endif /* __RFX_PRECISE_CALL_STATE_H */
