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

#ifndef __RP_RADIO_CONTROLLER_H__
#define __RP_RADIO_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <unordered_map>
#include "RfxController.h"
#include "RfxAction.h"
#include "RfxTimer.h"
#include "RadioMessage.h"
#include "RfxSignal.h"
//#include "RpNwRatController.h"
#include "RadioConstants.h"

typedef RfxCallback1<SuggestRadioResult> RpSuggestRadioCapabilityCallback;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

class RpRadioController : public RfxController {
    RFX_DECLARE_CLASS(RpRadioController); // Required: declare this class

public:

    RpRadioController();
    virtual ~RpRadioController();
    void dynamicSwitchRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action);
    void capabilitySwitchRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action);
    void requestRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action);
    void dynamicSwitchRadioOn();
    void requestRadioOn();

// Override
protected:

    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual void onInit();

private:
    int slotId;
    int canHandleErrStep;    //this is just for logs output.
    int lastSuggestedCapability;
    RadioMessage* MessageQueue;
    static bool inDynamicSwitchRadioOff;
    static bool isDynamicSwitchRadioOn;
    bool isUnderCapabilitySwitch;
    void requestRadioPower(const sp<RfxMessage>& message);
    void registerForStatusChange();
    void onCapabilityChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onRadioStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onModemOffStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void enqueueToMessageQueue(RadioMessage*);
    RadioMessage* findRadioMessageUsingSerial(int);
    RadioMessage* findRadioMessageUsingToken(int token, int source);
    bool findRadioMessageSendToCdma(void);
    bool checkIfRequestComplete(RadioMessage*);
    void sendResponse(RadioMessage* radioMessage);
    void dequeueFromMessageQueue(RadioMessage*);
    RIL_Errno combineErrno(RIL_Errno, RIL_Errno);
    void handleRadioStateChanged(const sp<RfxMessage>& message);
    bool canHandleRequest(const sp<RfxMessage>& message);
    bool handleRadioPowerResponse(const sp<RfxMessage>& message);
    RpSuggestRadioCapabilityCallback checkIfNeedCallback(std::unordered_map<int, RpSuggestRadioCapabilityCallback> map,
            const sp<RfxMessage>& message);
    void refreshRadioState();

public:
    void suggestedCapability(int desiredCapability, RpSuggestRadioCapabilityCallback &callback);
    RfxSignal1<int> m_c2k_radio_changed_singal;

private:
    std::unordered_map<int, RpSuggestRadioCapabilityCallback> suggestCapabilityMap;

};

#endif /* __RP_RADIO_CONTROLLER_H__ */

