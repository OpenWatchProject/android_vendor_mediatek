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

#ifndef __RP_BASE_NW_RAT_SWITCH_HANDLER__
#define __RP_BASE_NW_RAT_SWITCH_HANDLER__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxObject.h"
#include "RfxAction.h"
#include "RfxLog.h"
#include <telephony/mtk_ril.h>
#include "RfxStatusDefs.h"
#include "RfxRilAdapter.h"
#include "RpNwRatController.h"
#include "power/RadioConstants.h"
#include <cutils/properties.h>

#define LOG_BUF_SIZE 1024

/*****************************************************************************
 * Class RpBaseNwRatSwitchHandler
 *****************************************************************************/

/*
 * Class RpBaseNwRatSwitchHandler,
 * prototype of base nw rat switch handler class, define the virtual functions which sub-class
 * need to override, implement the general behaviors of nw rat switch.
 */
class RpBaseNwRatSwitchHandler : public RfxObject {
    RFX_DECLARE_CLASS(RpBaseNwRatSwitchHandler); // Required: declare this class

public:
    RpBaseNwRatSwitchHandler();
    RpBaseNwRatSwitchHandler(RpNwRatController* nwRatController);
    virtual ~RpBaseNwRatSwitchHandler();

public:
    // Virtual functions.
    /* Handle request and response for RILJ. */
    virtual void requestGetPreferredNetworkType(const sp<RfxMessage>& message) {
        RFX_UNUSED(message);
    };
    virtual void responseGetPreferredNetworkType(
            const sp<RfxMessage>& response) {
        RFX_UNUSED(response);
    };
    virtual void responseGetPreferredNetworkType(const int prefNwType,
            const sp<RfxMessage>& message) {
        RFX_UNUSED(prefNwType);
        RFX_UNUSED(message);
    };
    virtual void requestSetPreferredNetworkType(const int prefNwType,
            const sp<RfxMessage>& message) {
        RFX_UNUSED(prefNwType);
        RFX_UNUSED(message);
    };
    virtual void responseSetPreferredNetworkType(
            const sp<RfxMessage>& response) {
        RFX_UNUSED(response);
    };
    virtual void requestSetEvdoMode(const int prefNwType) {
        RFX_UNUSED(prefNwType);
    };
    virtual void responseSetEvdoMode(const sp<RfxMessage>& response) {
        RFX_UNUSED(response);
    };
    virtual void requestSetRatMode() {};
    virtual void responseSetRatMode(const sp<RfxMessage>& response) {
        RFX_UNUSED(response);
    };
    /* Handle network RAT switch. */
    virtual void doNwRatSwitch(ModemSettings mdSettings,
            const RatSwitchCaller& ratSwitchCaller, const sp<RfxAction>& action,
            const sp<RfxMessage>& message) {
        RFX_UNUSED(mdSettings);
        RFX_UNUSED(ratSwitchCaller);
        RFX_UNUSED(action);
        RFX_UNUSED(message);
    };
    virtual ModemSettings calculateModemSettings(int prefNwType,
            AppFamilyType appFamType, NwsMode nwsMode, VolteState volteState) {
        RFX_UNUSED(prefNwType);
        RFX_UNUSED(appFamType);
        RFX_UNUSED(nwsMode);
        RFX_UNUSED(volteState);
        ModemSettings mdSettings;
        return mdSettings;
    };
    void requestGetVoiceRadioTech(const sp<RfxMessage>& message);
    void responseGetVoiceRadioTech(const sp<RfxMessage>& response);
    void updatePhone(const ModemSettings mdSettings);
    void updatePhone(const int prefNwType, const NwsMode nwsMode);
    int techFromPrefNwType(const int prefNwType);
    virtual void onResponseTimeOut(const sp<RfxMessage>& response) {
        RFX_UNUSED(response);
    };
    virtual void onNwRatSwitchDone(RatSwitchResult switchResult, int errorCode = 0) {
        RFX_UNUSED(switchResult);
        RFX_UNUSED(errorCode);
    };
    virtual bool needCdmaRadioOn(const int prefNwType, const NwsMode nwsMode) {
        RFX_UNUSED(prefNwType);
        RFX_UNUSED(nwsMode);
        return false;
    }

protected:
    void requestGetPreferredNetworkType(const RILD_RadioTechnology_Group radioTechType, const sp<RfxMessage>& msg);

public:
    CombinedPreferredNwType getCombinedPreferredNwType(const int nwType) {
        CombinedPreferredNwType type;
        type.gsm_preferred_nw_type = -1;
        type.c2k_preferred_nw_type = -1;

        switch (nwType) {
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_GSM_ONLY:
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_ONLY:
        case PREF_NET_TYPE_LTE_TDD_ONLY:
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
            type.gsm_preferred_nw_type = nwType;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_EVDO_ONLY:
            type.c2k_preferred_nw_type = -1;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            type.gsm_preferred_nw_type = PREF_NET_TYPE_GSM_WCDMA_AUTO;
            type.c2k_preferred_nw_type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            type.gsm_preferred_nw_type = PREF_NET_TYPE_LTE_ONLY;
            type.c2k_preferred_nw_type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            type.gsm_preferred_nw_type = PREF_NET_TYPE_LTE_GSM_WCDMA;
            type.c2k_preferred_nw_type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        }

        return type;
    }

    bool needGsmRadioOn(const int prefNwType) {
        switch (prefNwType) {
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_GSM_ONLY:
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_ONLY:
        case PREF_NET_TYPE_LTE_TDD_ONLY:
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
            return true;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_EVDO_ONLY:
            return false;
        }
        return false;
    }

    virtual bool isCdma3gDualModeCard() {
        return false;
    }

    void logD(const char *tag, const char *fmt, ...) const {
        va_list ap;
        char buf[LOG_BUF_SIZE];
        String8 tagString = String8::format("%s[%d]", tag, mSlotId);
        va_start(ap, fmt);
        vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
        va_end(ap);

        __android_log_buf_write(LOG_ID_RADIO, ANDROID_LOG_DEBUG, tagString.string(), buf);
    }

    virtual NwsMode getNwsModeForSwitchCardType() {
        return NWS_MODE_CSFB;
    }

    virtual RatSwitchCaller getRatSwitchCaller() {
        return RAT_SWITCH_NORMAL;
    }
protected:
    // Variables.
    RpNwRatController* mNwRatController;
    int mSlotId;
};

#endif /* __RP_BASE_NW_RAT_SWITCH_HANDLER__ */

