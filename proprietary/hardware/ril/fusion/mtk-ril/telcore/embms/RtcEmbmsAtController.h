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

#ifndef __RTC_EMBMS_AT_CONTROLLER_H__
#define __RTC_EMBMS_AT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Vector.h>
#include "RfxController.h"
#include "RfxTimer.h"
#include "RtcEmbmsSessionInfo.h"
#include <telephony/mtk_ril.h>

/*****************************************************************************
 * Class RtcEmbmsAtController
 *****************************************************************************/

#define EMBMS_ASSERT(_c)       assert(_c)
#define EMBMS_UNUSED(x) ((void)(x))

extern "C" int strStartsWith(const char *line, const char *prefix);

/*****************************************************************************
 * PRIVATE ENUM
 *****************************************************************************/


/*****************************************************************************
 * Class RtcEmbmsAtController
 *****************************************************************************/
class RtcEmbmsAtController : public RfxController {
    RFX_DECLARE_CLASS(RtcEmbmsAtController);  // Required: declare this class

public:
    RtcEmbmsAtController();

    virtual ~RtcEmbmsAtController();

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);
    virtual void handleRequestDefault(const sp<RfxMessage>& request);


private:
    // Private functions
    bool isEmbmsSupported();
    int strStartsWith(const char *line, const char *prefix);
    void responseToSocket(const sp<RfxMessage>& msg, const char *responseStr);
    void urcToSocket(const sp<RfxMessage>& msg, const char *responseStr, const char *responseStr_mask);
    void responseErrorString(const sp<RfxMessage>& msg);
    void handleAtRequest(const sp<RfxMessage>& request);
    void handleAtResponse(const sp<RfxMessage>& response);
    void handleAtUrc(const sp<RfxMessage>& message);

    void requestAtEnable(const sp<RfxMessage>& request);
    void handleEnableResponse(const sp<RfxMessage>& response);
    void handleDisableResponse(const sp<RfxMessage>& response);

    void requestAtGetNetworkTime(const sp<RfxMessage>& request);
    void handleGetNetworkTimeResponse(const sp<RfxMessage>& response);

    void requestAtGetCoverage(const sp<RfxMessage>& request);
    void handleGetCoverageResponse(const sp<RfxMessage>& response);

    void requestAtSetAvailbility(const sp<RfxMessage>& request);
    void requestAtActivate(const sp<RfxMessage>& request,
        embms_at_activesession_enum type);
    void handleStartSessionResponse(const sp<RfxMessage>& response);
    void handleStartSessionUrc(const sp<RfxMessage>& message);
    void handleStopSessionResponse(const sp<RfxMessage>& response);

    void handleTriggerCellInfoNotifyResponse(const sp<RfxMessage>& response);

    void handleCellInfoUrc(const sp<RfxMessage>& message);
    void handleCoverageUrc(const sp<RfxMessage>& message);
    void handleSessionListUrc(const sp<RfxMessage>& message);
    void handleActiveSessionStatus(const sp<RfxMessage>& response);
    void handleOosUrc(const sp<RfxMessage>& message);
    void handleSaiUrc(const sp<RfxMessage>& message);
    int getCoverageState(int status);

    void onMainCapabilityChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void onDefaultDataChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);

private:
    // Private members
    Vector<RtcEmbmsSessionInfo*>* mSessionInfoList;
    bool mIsActiveSession;
    bool mIsFlightOn;
    bool mIsEmbmsSupport;
    bool mEmbmsEnabled;
    int  mSdkVersion; //XX.YY.ZZ
};

#endif /* __RP_DATA_CONTROLLER_H__ */

