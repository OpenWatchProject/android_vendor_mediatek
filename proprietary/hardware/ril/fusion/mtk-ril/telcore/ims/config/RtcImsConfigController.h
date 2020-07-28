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

#ifndef __RFX_IMS_CONFIGSTORAGE_H__
#define __RFX_IMS_CONFIGSTORAGE_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include "RfxController.h"
#include <memory>
#include <string>
#include <mtkconfigdef.h>

/*****************************************************************************
 * Import
 *****************************************************************************/
using std::string;
using std::shared_ptr;
using std::make_shared;

/*****************************************************************************
 * Class RtcImsConfigController
 *****************************************************************************/
class RtcImsConfigController : public RfxController {
    RFX_DECLARE_CLASS(RtcImsConfigController);  // Required: declare this class


public:
    RtcImsConfigController(){};
    virtual ~RtcImsConfigController(){};

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff, int radioState);

private:
    void initProvisionValue();
    void setFeatureValue(const sp<RfxMessage>& message);
    void handleSetFeatureValue(bool success, const sp<RfxMessage>& message);
    void getFeatureValue(const sp<RfxMessage>& message);
    void handleGetFeatureValue(bool success, int value, const sp<RfxMessage>& message);
    void setProvisionValue(const sp<RfxMessage>& messag);
    bool handleImsConfigExt(const sp<RfxMessage> &message);
    void handleSetProvisionValue(bool success, const sp<RfxMessage>& message);
    void handleSetProvisionResponse(const sp<RfxMessage>& message);
    void getProvisionValue(const sp<RfxMessage>& message);
    void handleGetProvisionResponse(const sp<RfxMessage>& message);
    void handleGetProvisionUrc(const sp<RfxMessage>& message);
    void setImsResourceCapability(int feature_id, int value, bool isDynamicImsSwitchLast);
    void getImsResourceCapability(const sp<RfxMessage>& message);
    void handleGetImsResourceCapability(bool success, int value, const sp<RfxMessage>& message);
    void triggerImsCfgCommand(int slot_id);

    void onSimStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onRadioStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onCarrierConfigChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

    void resetFeatureSendCmd();
    void resetFeatureResource();
    void resetFeatureValue();
    void resetFeatureSendValue();

    string convertToString(int i);
    bool ensureStorageInitStatus(const sp<RfxMessage>& message);
    void processDynamicImsSwitch();
    int getCurrentMccMnc();

    void saveProvisionedValue();
    void loadProvisionedValue();
    void resetProvisionedValue();

    bool isLogEnable();

private:
    std::map<int, bool> mFeatureSendCmd;
    std::map<int, int> mFeatureResource;            // <Resource Id, Resource Value>
    std::map<int, int> mVoLteFeatureValue;          // <Network Id, Feature Value>
    std::map<int, int> mViLteFeatureValue;          // <Network Id, Feature Value>
    std::map<int, int> mVoWifiFeatureValue;         // <Network Id, Feature Value>
    std::map<int, int> mViWifiFeatureValue;         // <Network Id, Feature Value>
    std::map<int, int> mVoNrFeatureValue;          // <Network Id, Feature Value>
    std::map<int, int> mViNrFeatureValue;          // <Network Id, Feature Value>
    std::map<int, std::string> mProvisionValue;     // <Provision Id, Provision Value>
    std::map<int, std::string> mProvisionedValue;   // <Provisioned Id, Provisioned Value>
    std::map<int, int> mFeatureSendValue;

    bool mECCAllowSendCmd = false;
    bool mECCAllowNotify = false;
    bool mInitDone = false;
    int mMccmnc = -1;
    sp<RfxMessage> mSetProvisionMessage;
    int mGetProvisionId = 0;;
    sp<RfxMessage> mGetProvisionMessage;
    int mSendCfgMccmnc = -1;    // Use to check if call triggerImsCfgCommand() directly.
    String8 mSendCfgIccid;      // Use to check if call triggerImsCfgCommand() directly.

    bool DEBUG = false;

    typedef enum {
        ISLAST_NULL = -1,
        ISLAST_FALSE = 0,
        ISLAST_TRUE = 1
    } ENUM_ISLAST;

};

#endif /* __RFX_IMS_CONFIGSTORAGE_H__ */

