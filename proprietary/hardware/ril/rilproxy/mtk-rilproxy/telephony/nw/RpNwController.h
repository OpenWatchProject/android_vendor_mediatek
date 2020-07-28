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

#ifndef __RP_NW_CONTROLLER_H__
#define __RP_NW_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
/// M:[Network][C2K] add for band8 desense. @{
enum BandMode {
    BM_FOR_DESENSE_RADIO_ON     = 200,
    BM_FOR_DESENSE_RADIO_OFF   = 201,
    BM_FOR_DESENSE_RADIO_ON_ROAMING   = 202,
    BM_FOR_DESENSE_B8_OPEN = 203,
};
#define OPERATOR_OP09 "OP09"
#define SEGDEFAULT "SEGDEFAULT"
/// @}
/*****************************************************************************
 * Class RpNwController
 *****************************************************************************/

class RpNwController: public RfxController {
    // Required: declare this class
RFX_DECLARE_CLASS(RpNwController);

public:
    RpNwController();

    virtual ~RpNwController();

protected:
    // Override
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);

private:
    // Request
    void notifyPlmnSearchState(int plmnSearchAction);
    void onConfigDcStateDone();
    char *strdupReadString(Parcel *p);
    void writeStringToParcel(Parcel *p, const char *s);
    void convertFromEiToNi(const sp<RfxMessage>& message);
    void getPrlVersion(const sp<RfxMessage>& message);
    ///M: [Network][C2K] add for band8 desense. @{
    void onC2kRadioChanged(int radioPower);
    bool isGsmCard();
    int getCapabilitySlotId();
    void onNwsModeChanged(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant newValue);
    void onCapabilityChange(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant newValue);
    void configBandMode();
    void queryAvaliableBandMode();
    void setBandMode(int bandMode, int forceSwitch);
    void checkCallState();
    void onCallCountChanged(int slotId, RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void onImsCallCountChanged(int slotId, RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void cancelListeningCallStatus();
    void cancelListeningImsCallStatus();

    bool isOP09ASupport() {
        char optr_value[PROPERTY_VALUE_MAX] = { 0 };
        char seg_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.operator.optr", optr_value, "0");
        property_get("persist.vendor.operator.seg", seg_value, "0");
        if (strcmp(optr_value, OPERATOR_OP09) == 0 &&
            strcmp(seg_value, SEGDEFAULT) == 0) {
            return true;
        }
        return false;
    }
    /// @}
protected:
    // Utility functions
    const char* requestToString(int reqId);
    const char* urcToString(int reqId);

private:
    /// M: [Network][C2K] add for band8 desense. @{
    bool mFirstRadioChange = true;
    int mBandMode = -1;
    int mForceSwitch = 0;
    int mC2kRadioPower = 0;
    int mLocalBandToken = 0;
    bool mIsListeningCall = false;
    bool mIsListeningImsCall = false;
    /// @}

    /// M: [Network][C2K]Mark for the Eng mode nw info. @{
    const char *Eng_Nw_Info_Type = "10000";
    /// @}
};

#endif /* __RP_NW_CONTROLLER_H__ */

