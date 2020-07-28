/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#include "RpSmsCtrlBase.h"
#include "RfxLog.h"

/*****************************************************************************
 * Class RpSmsCtrlBase
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpSmsCtrlBase", RpSmsCtrlBase, RfxController);

RpSmsCtrlBase::RpSmsCtrlBase() :
    mName(String8("RpSmsCtrlBase")),
    mIsReady(false) {
}

void RpSmsCtrlBase::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
}

void RpSmsCtrlBase::log(String8 s) {
    RFX_LOG_D(getName().string(), "%s, (slot %d)", s.string(), getSlotId());
}

void RpSmsCtrlBase::setName(String8 s) {
    mName = s;
}

String8 RpSmsCtrlBase::getName() {
    return mName;
}

int RpSmsCtrlBase::isPreviewed(const sp<RfxMessage>& message) {
    return isPreviewedInternal(message);
}

int RpSmsCtrlBase::isHandled(const sp<RfxMessage>& message) {
    return isHandledInternal(message);
}

int RpSmsCtrlBase::isResumed(const sp<RfxMessage>& message) {
    return isResumedInternal(message);
}

int RpSmsCtrlBase::isPreviewedInternal(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    // default not preview
    return RIL_REQUEST_PREVIEW_REASON_NOT_PREVIEW;
}

int RpSmsCtrlBase::isHandledInternal(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    // default not handled
    return RIL_REQUEST_HANDLE_REASON_NOT_HANDLE;
}

int RpSmsCtrlBase::isResumedInternal(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    // default not resume
    return RIL_REQUEST_RESUME_REASON_NOT_RESUME;
}
void RpSmsCtrlBase::setSmsReady(bool ready) {
    mIsReady = ready;
}

bool RpSmsCtrlBase::getSmsReady() {
    return mIsReady;
}
