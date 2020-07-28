/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RTC_GSM_SMS_CONTROLLER_H__
#define __RTC_GSM_SMS_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxTimer.h"
#include "RtcSmsMessage.h"
/*****************************************************************************
 * Class RtcGsmSmsController
 *****************************************************************************/

class RtcGsmSmsController : public RfxController {
RFX_DECLARE_CLASS(RtcGsmSmsController);

public:
    RtcGsmSmsController();
    virtual ~RtcGsmSmsController();

// Override
protected:
    virtual void onInit();
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(
            const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
public:
    void handleRequest(const sp<RfxMessage>& message);
    bool previewMessage(const sp<RfxMessage>& message);
    bool checkIfResumeMessage(const sp<RfxMessage>& message);
    void sendGsmSms(RtcGsmSmsMessage *msg);
    void sendGsmSmsAck(int success, int cause, const sp<RfxMessage>& message);

private:
    void setTag(String8 tag) {mTag = tag;}
    const char* boolToString(bool value);
    void onHidlStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void delaySetSmsFwkReady();
    int getReferenceIdFromCDS(char *hex);
    int smsHexCharToDecInt(char *hex, int length);
    void handleNewSms(const sp<RfxMessage>& message);
    void handNewSmsAck(const sp<RfxMessage>& message);
    bool isSupportSmsFormatConvert();
    bool isUnderCryptKeeper();
    bool isCdmaPhoneMode();

private:
    String8 mTag;
    bool mSmsFwkReady;
    bool mSmsMdReady;
    TimerHandle mSmsTimerHandle;
    bool mSmsSending;
    bool mNeedStatusReport; // only for ims sms

#define GSM_SMS_MESSAGE_STRS_COUNT 2 // 0: smsc, 1: pdu
#define DELAY_SET_SMS_FWK_READY_TIMER 5000 //5s

};
#endif /* __RTC_GSM_SMS_CONTROLLER_H__ */
