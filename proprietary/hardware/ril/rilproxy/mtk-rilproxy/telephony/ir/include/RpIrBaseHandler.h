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

#ifndef __RP_IR_BASEHANDLER_H__
#define __RP_IR_BASEHANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "IRpIrNwController.h"
#include "RfxController.h"
#include "RfxAction.h"
#include "RfxNwServiceState.h"
#include "RfxTimer.h"
#include <iostream>
#include <string>
#include "utils/String16.h"

using namespace std;

using ::android::Vector;
using ::android::String16;

/*****************************************************************************
 * Class RpIrBaseHandler
 *****************************************************************************/
class RpIrController;

typedef enum _StateEnum {
    STATE_UNKNOWN,
    STATE_INIT,
    STATE_NO_SERVICE,
    STATE_GETTING_PLMN,
    STATE_SELECTING_NETWORK,
    STATE_NETWORK_SELECTED
} StateEnum;

class RpIrBaseHandler : public RfxController, IRpIrNwController {
    RFX_DECLARE_CLASS(RpIrBaseHandler);  // Required: declare this class

public:
    RpIrBaseHandler();

    RpIrBaseHandler(RpIrController *ir_controller);
    virtual ~RpIrBaseHandler();

    virtual void setNwsMode(NwsMode nwsMode);
    virtual void resumeNetwork();
    virtual void dispose();
    virtual void registerListener(IRpIrNwControllerListener* listener);
    virtual void startNewSearchRound();
    virtual void setIfEnabled(bool isEnabled);
    void setPreviewPlmnHandleState(bool beHandled);

protected:
    virtual bool enableNoServiceDelay();

    void onNwsModeChanged();
    void sendNoServiceMessage(int nDelayedTime);
    void removeNoServiceMessage();
    void postponeNoServiceMessageIfNeeded(int nDelayedTime);
    void onNoServiceTimeout();
    int convertVoiceRegState(int nState);

    const char *stateToString(StateEnum nState) const;
    void setState(StateEnum nState);
    StateEnum getState() const;
    void resetToInitialState();
    void setServiceType(ServiceType serviceType);
    void processPlmnChanged(String8 plmn);
    void previewPlmn(String8 plmn);

private:
    void onImsiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onImsiChangedForPreviewPlmn(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

protected:
    RpIrController* mPtrIrController;
    ServiceType mServiceType;
    StateEnum mState;
    StateEnum mPreState;
    int mPreVoiceState;
    int mPreDataState;
    Vector<String8> mVecPlmns;
    bool mIsFirstRoundSearch;
    // set to NULL when time off or stoped, so it can be used to check if time is set or not.
    TimerHandle mNoServiceTimerHandle;
    IRpIrNwControllerListener *mPtrListener;
    String8 mPlmn;
    bool mPreviewPlmnHandled;
};

#endif /* __RP_IR_BASEHANDLER_H__ */

