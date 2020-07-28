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

#ifndef __RP_GSM_SMS_CTRL_H__
#define __RP_GSM_SMS_CTRL_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RpSmsCtrlBase.h"
#include "RfxTimer.h"
#include <cutils/jstring.h>
#include "utils/String8.h"


using ::android::String8;

/*****************************************************************************
 * Class RpSmsController
 *****************************************************************************/
extern "C" const char * requestToString(int request);

class RpGsmSmsCtrl : public RpSmsCtrlBase {
    // Required: declare this class
    RFX_DECLARE_CLASS(RpGsmSmsCtrl);

public:
    RpGsmSmsCtrl();
    bool handleResponse(const sp<RfxMessage>& message);

// Override
protected:
    virtual void onInit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual int isPreviewedInternal(const sp<RfxMessage>& message);
    virtual int isHandledInternal(const sp<RfxMessage>& message);
    virtual int isResumedInternal(const sp<RfxMessage>& message);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

private:
    bool handleImsSendSms(const sp<RfxMessage>& message);
    const char* boolToString(bool value);
    void onHidlStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void delaySetSmsFwkReady();
    int getReferenceIdFromCDS(const char *hex);
    int smsHexCharToDecInt(const char *hex, int length);
    char *strdupReadString(Parcel *p);
    bool isCdmaPhoneMode();

private:
    bool mSmsFwkReady;
    TimerHandle mSmsTimerHandle;
    bool mSmsSending;
    bool mNeedStatusReport; // only for ims sms

#define DELAY_SET_SMS_FWK_READY_TIMER 5000 //5s
};

#endif /* __RP_GSM_SMS_CTRL_H__ */
