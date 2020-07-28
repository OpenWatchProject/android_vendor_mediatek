/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef __RP_IR_STRATEGY_COMMON_H__
#define __RP_IR_STRATEGY_COMMON_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <telephony/mtk_ril.h>
#include "RpIrStrategy.h"

#include "utils/String8.h"
#include "utils/Timers.h"
#include "utils/Vector.h"

#include <string>

using namespace std;
/*****************************************************************************
 * Class RpIrStrategyCommon
 *****************************************************************************/
#define CHINA_TELECOM_MAINLAND_MCC 460
#define CHINA_TELECOM_MACAU_MCC 455
#define NO_SEERVICE_WATCHDOG_DELAY_TIME (300 * 1000)
#define SWITCH_RESUME_DELAY_TIME  (20 * 1000)
#define FIND_NETWORK_DELAY_TIME (30 * 1000)
#define WATCHDOG_RETRY_DELAY_STEP (30 * 1000)  // 30s
#define MAX_WATCHDOG_RETRY_DELAY (30 * 60 * 1000)  // 30m

class RpIrStrategyCommon : public RpIrStrategy {
    RFX_DECLARE_CLASS(RpIrStrategyCommon);

public:
    RpIrStrategyCommon(RpIrController* ric, IRpIrNwController* lwgController,
            IRpIrNwController* cdmaController);

    virtual ~RpIrStrategyCommon();

    virtual NwsMode getNwsModeByPlmn(const String8& plmn);
    virtual void onLwgPlmnChanged(const String8 &plmn);
    virtual void onCdmaPlmnChanged(const String8 &plmn);
    virtual void onRatSwitchStart(const int newPrefNwType, const NwsMode newNwsMode,
            const RatSwitchCaller ratSwitchCaller);
    virtual void onRatSwitchDone(int curPrefNwType, int newPrefNwType);
    virtual void onWwopRatSwitchDone(int curPrefNwType, int newPrefNwType);

    void postponeNoServiceWatchdogIfNeeded();
    void restartNoSerivceWatchdogIfNeeded();
    void setLwgServiceState(ServiceType serviceType);
    ServiceType getLwgServiceState() { return mLwgServiceState; }
    void setCdmaServiceState(ServiceType serviceType);
    ServiceType getCdmaServiceState() { return mCdmaServiceState; }
    bool switchForNoService(bool forceSwitch);
    void updateWatchdog();
    void onRadioStateChanged();
    void onActionNwsModeChanged();
    bool onPreviewLwgPlmn(const String8 &plmn);

protected:
    virtual void onInit();

protected:
    static int sWatchdogDelayTime;
    static int sSwitchModeOrResumeDelayTime;

    NwsMode mCdmaNwsMode;
    ServiceType mLwgServiceState;
    ServiceType mCdmaServiceState;

    nsecs_t mNoServiceTimeStamp;
    nsecs_t mWatchdogStartTime;
    int mContinousRetryCount;

    void triggerNoServiceWatchdog();
    void startNoServiceWatchdog();
    void stopNoServiceWatchdog();

    void onSetIfEnabled(bool enabled);
    int getHomeMcc();

private:
    IRpIrNwControllerListener* m_lwgListener;
    IRpIrNwControllerListener* m_cdmaListener;
    TimerHandle m_timer_handle_watchDog;
    bool m_timer_handle_watchDog_started;
    NwsMode mLastInServiceNwsMode;
    String8 mLastInServiceIccId;

    bool isDualServiceNotInService();
    bool isDualRadioOff();
    String8 getIccId();
};

#endif /* __RP_IR_STRATEGY_COMMON_H__ */

