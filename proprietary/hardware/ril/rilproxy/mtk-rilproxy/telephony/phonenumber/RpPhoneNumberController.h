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

#ifndef __RFX_PHONE_NUMBER_CONTROLLER_H__
#define __RFX_PHONE_NUMBER_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include <cutils/properties.h>
#include <string>
#include <vector>
#include "EccNumberSource.h"

/*****************************************************************************
 * Class RpPhoneNumberController
 *****************************************************************************/
#define MAX_ECC_PROPERTY_CHARS   30
#define MAX_ECC_CHARS            100

class RpPhoneNumberController : public RfxController {
    RFX_DECLARE_CLASS(RpPhoneNumberController);  // Required: declare this class

public:
    RpPhoneNumberController();

    virtual ~RpPhoneNumberController();

// Override
protected:
    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual void onInit();

    virtual void onDeinit();

// External Method
public:

    // Get the emergency call routing for the corresponding emergency number
    //
    // RETURNS:
    //     UNKNOWN if the number is not recognized as emergency emergency number in RILD
    //     EMERGENCY if the number should dial using emergency call routing
    //     NORMAL if the number should dial using normal call routing
    EmergencyCallRouting getEmergencyCallRouting(String8 number);

    // Check if given number is a emergency number or not based on SIM/Network/XML/Default
    // emergency number list.
    // NOTE: For speical emergency number which should dial using normal call
    //       This API will return true
    //
    // RETURNS: true if number is emergency number
    bool isEmergencyNumber(String8 number);

    // Get service category of emergency number
    //
    // RETURNS: service category value defined 3gpp 22.101, Section 10 - Emergency Calls
    int getServiceCategory(String8 number);

private:
    void onCardTypeChanged(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant newValue);
    void onSimStateChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void onPlmnChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void onConnectionStateChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void handleGSMEFECC(const sp<RfxMessage>& message);
    void handleC2KEFECC(const sp<RfxMessage>& message);
    void handleSimPlugOut(const sp<RfxMessage>& message);
    void parseSimEcc(String8 line, bool isGsm);
    bool isCdmaCard(int cardType);
    void updateEmergencyNumber();
    void handleUpdateEmergencyNumber(char *plmn);
    string getPlmn(int slot);
    void initEmergencyNumberSource();
    bool isEccMatchInList(String8 number, String8 eccList);
    void createEmergencyNumberListResponse(RIL_EmergencyNumber *data);
    void freeEmergencyNumberListResponse(RIL_EmergencyNumber *data);
    void emergencyNumberListToParcel(const sp<RfxMessage>& msg,
            RIL_EmergencyNumber *response, int responselen);
    string getSourcesString(int sources);
    char* strdupReadString(Parcel *p);
    const char* urcToString(int urcId);
    void updateSpecialEmergencyNumber();
    void delaySetFwkReady();
    string convertPlmnForRoaming(string plmn);
    bool isCtCard();
    bool isNeedCtaEcc();
    void dumpEccList();
    void testEcc();
    bool updateEmergencySourcesForPlmnChange(char *plmn, bool isSimInsert);
    int getSimState();

private:
    String8 mGsmEcc;
    String8 mC2kEcc;
    bool mIsSimInsert;
    vector<EccNumberSource *> mEccNumberSourceList;
    SimEccNumberSource *mSimEccSource;
    NetworkEccNumberSource *mNetworkEccSource;
    DefaultEccNumberSource *mDefaultEccSource;
    XmlEccNumberSource *mXmlEccSource;
    OemPropertyEccNumberSource *mPropertyEccSource;
    TestEccNumberSource *mTestEccSource;
    CtaEccNumberSource *mCtaEccSource;
    vector<EmergencyNumber> mEccList;
    bool mIsFwkReady; // Indicate if framework is ready to receive emergency number
    bool mIsPendingUpdate;
    TimerHandle mTimerHandle;
};

#endif /* __RFX_PHONE_NUMBER_CONTROLLER_H__ */

