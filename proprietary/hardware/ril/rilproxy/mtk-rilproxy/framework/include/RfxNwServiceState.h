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

#ifndef __RFX_NW_SERVICE_STATE_H
#define __RFX_NW_SERVICE_STATE_H

#define RIL_REG_STATE_NOT_REG_NOT_SEARCHING 0
#define RIL_REG_STATE_HOME 1
#define RIL_REG_STATE_NOT_REG_SEARCHING 2
#define RIL_REG_STATE_DENIED 3
#define RIL_REG_STATE_UNKNOWN 4
#define RIL_REG_STATE_ROAMING 5
#define RIL_REG_STATE_NOT_REG_EMERGENCY_CALL_ENABLED 10
#define RIL_REG_STATE_SEARCHING_EMERGENCY_CALL_ENABLED 12
#define RIL_REG_STATE_DENIED_EMERGENCY_CALL_ENABLED 13
#define RIL_REG_STATE_UNKNOWN_EMERGENCY_CALL_ENABLED 14

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "Rfx.h"
#include "utils/String8.h"

using ::android::String8;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RfxNwServiceState {
public:

    RfxNwServiceState();

    RfxNwServiceState(int rilVoiceRegState, int rilDataRegState,
        int rilVoiceRadioTech, int rilDataRadioTech, int cdmaNetworkExist);

    ~RfxNwServiceState();

public:

    int getRilVoiceRegState();
    int getRilDataRegState();
    int getRilVoiceRadioTech();
    int getRilDataRadioTech();
    int getCdmaNetworkExist();
    void setRilVoiceRegState(int rilVoiceRegState);
    void setRilDataRegState(int rilDataRegState);
    void setRilVoiceRadioTech(int rilVoiceRadioTech);
    void setRilDataRadioTech(int rilDataRadioTech);
    void setCdmaNetworkExist(int cdmaNetworkExist);
    String8 toString();

private:

    int mRilVoiceRegState;
    int mRilDataRegState;
    int mRilVoiceRadioTech;
    int mRilDataRadioTech;
    int mCdmaNetworkExist;

public:

    static bool isInService(int regState) {
        if (regState == 1 || regState == 5) {
            return true;
        }
        return false;
    }

    static bool isGsmGroup(int radioTech) {
        if (radioTech == RADIO_TECH_GPRS
                || radioTech == RADIO_TECH_EDGE
                || radioTech == RADIO_TECH_UMTS
                || radioTech == RADIO_TECH_HSDPA
                || radioTech == RADIO_TECH_HSUPA
                || radioTech == RADIO_TECH_HSPA
                || radioTech == RADIO_TECH_LTE
                || radioTech == RADIO_TECH_HSPAP
                || radioTech == RADIO_TECH_GSM
                || radioTech == RADIO_TECH_TD_SCDMA) {
            return true;
        }
        return false;
    }

    static bool isCdmaGroup(int radioTech) {
        if (radioTech == RADIO_TECH_IS95A
                || radioTech == RADIO_TECH_IS95B
                || radioTech == RADIO_TECH_1xRTT
                || radioTech == RADIO_TECH_EVDO_0
                || radioTech == RADIO_TECH_EVDO_A
                || radioTech == RADIO_TECH_EVDO_B
                || radioTech == RADIO_TECH_EHRPD) {
            return true;
        }
        return false;
    }

    bool equalTo(const RfxNwServiceState &other) const {
        return (mRilVoiceRegState == other.mRilVoiceRegState)
                && (mRilDataRegState == other.mRilDataRegState)
                && (mRilVoiceRadioTech == other.mRilVoiceRadioTech)
                && (mRilDataRadioTech == other.mRilDataRadioTech)
                && (mCdmaNetworkExist == other.mCdmaNetworkExist);
    }

    RfxNwServiceState &operator = (const RfxNwServiceState &other) {
        mRilVoiceRegState = other.mRilVoiceRegState;
        mRilDataRegState = other.mRilDataRegState;
        mRilVoiceRadioTech = other.mRilVoiceRadioTech;
        mRilDataRadioTech = other.mRilDataRadioTech;
        mCdmaNetworkExist = other.mCdmaNetworkExist;
        return *this;
    }

    bool operator == (const RfxNwServiceState &other) {
        return equalTo(other);
    }

    bool operator != (const RfxNwServiceState &other) {
        return !equalTo(other);
    }
};

#endif /* __RFX_NW_SERVICE_STATE_H */
