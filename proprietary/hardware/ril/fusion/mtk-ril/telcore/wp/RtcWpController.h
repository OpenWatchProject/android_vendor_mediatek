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

#ifndef __RFX_WP_CONTROLLER_H__
#define __RFX_WP_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxTimer.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "RfxTimer.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RtcWpController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcWpController);

public:
    RtcWpController();
    virtual ~RtcWpController();

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff,int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

public:
    // other module check if world mode is switching
    bool isWorldModeSwitching();

private:
    void processWorldModeResumingResponse(const sp<RfxMessage>& message);
    void resumeWorldModeChange();
    int getCallingSlotId();
    void handleWorldModePendedByCall(int slotId);
    void onCallCountChanged(int slotId, RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void resumeWorldModeChangeWithRadioOff();
    void requestRadioOn();
    void requestRadioOff();
    void onRequestRadioOffDone(int slotId);
    void onRequestRadioOnDone(int slotId);
    // handle start URC from MCL
    void handleWorldModeChangeStart(bool flag, int cause);
    bool canhandleMessage(bool log_flag);
    void onRetryResumeTimeOut();
    void onTelephonyAssistantStatusChanged(int slotId, RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void createNewMessageToRmc(int messageId, int slotId, int value);
    // other module needs to block world mode switch
    int blockWorldModeChanged();
    // other module agrees with world mode switch
    int resumeBlockedWorldModeChanged();
private:
    // record the number other module blocks world mode switch
    static int worldModeBlockedNum;
    // record the slot start urc coming
    static int mainSlotId;
    static int mainWorldModeId;
    int getBlockedWorldModeStatusKeyValue(bool log_flag);
    int setBlockedWorldModeStatusKeyValue(int value);
    bool backupRadioPower[MAX_SIM_COUNT];
    int closeRadioCount;
    int openRadioCount;
    TimerHandle mWpTimerHandle;
    int mRetryResumeCount;
};

#endif /* __RFX_WP_CONTROLLER_H__ */

