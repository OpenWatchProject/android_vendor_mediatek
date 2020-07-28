/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "RtcSuppServQueue.h"
#include "RfxRilUtils.h"
#include "RfxLog.h"

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RtcSuppServQueue"

// The instance of RtcSuppServQueue will be initialized by RtcSuppServController
RtcSuppServQueue* RtcSuppServQueue::mInstance = NULL;

RtcSuppServQueue* RtcSuppServQueue::getInstance() {
    if (mInstance == NULL) {
        mInstance = new RtcSuppServQueue();
    }

    return mInstance;
}

RtcSuppServQueue::RtcSuppServQueue() {
    RFX_LOG_D(RFX_LOG_TAG, "Init RtcSuppServQueue's singleton instance!");
    mSSLock = SS_UNLOCKED;
}

RtcSuppServQueue::~RtcSuppServQueue() {
    RFX_LOG_D(RFX_LOG_TAG, "Delete RtcSuppServQueue's singleton instance!");
    delete mInstance;
}

void RtcSuppServQueue::add(RtcSSEntry ssEntry) {
    mSuspendedMsgQueue.add(ssEntry);
}

void RtcSuppServQueue::clear() {
    mSuspendedMsgQueue.clear();
}

void RtcSuppServQueue::removeFront() {
    mSuspendedMsgQueue.removeAt(0);
}

bool RtcSuppServQueue::isEmpty() {
    return mSuspendedMsgQueue.isEmpty();
}

const RtcSSEntry& RtcSuppServQueue::itemAt(int index) {
    return mSuspendedMsgQueue.itemAt(index);
}

void RtcSuppServQueue::setSSLock(SSLockState lock) {
    RFX_LOG_D(RFX_LOG_TAG, "setSSLock: %s -> %s", lockToString(mSSLock), lockToString(lock));
    if (mSSLock == lock) {
        return;
    }
    mSSLock = lock;
}

SSLockState RtcSuppServQueue::getSSLock() {
    return mSSLock;
}

const char *RtcSuppServQueue::lockToString(SSLockState lock) {
    switch (lock) {
        case SS_UNLOCKED:
            return "SS_UNLOCKED";
        case SS_LOCKED:
            return "SS_LOCKED";
        default:
            // not possible here!
            return NULL;
    }
}