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

#ifndef __RTC_CDMA_SMS_CONTROLLER_H__
#define __RTC_CDMA_SMS_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RtcSmsMessage.h"

/*****************************************************************************
 * Class RtcCdmaSmsController
 *****************************************************************************/

class RtcCdmaSmsController : public RfxController {
RFX_DECLARE_CLASS(RtcCdmaSmsController);

public:
    RtcCdmaSmsController();
    virtual ~RtcCdmaSmsController();

public:
    void handleRequest(const sp<RfxMessage>& message);
    bool previewMessage(const sp<RfxMessage>& message);
    bool checkIfResumeMessage(const sp<RfxMessage>& message);
    void onTimer();
    void sendCdmaSms(RtcCdmaSmsMessage *msg);
    void sendCdmaSmsAck(const sp<RfxMessage>& message);

protected:
    virtual void onInit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& msg);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(
            const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);

private:
    void handleMoSmsRequests(const sp<RfxMessage>& message);
    void handleSmscAdressResponses(const sp<RfxMessage>& message);
    void handleCdmaSmsAck(const sp<RfxMessage>& message);
    void handleMoSmsResponses(const sp<RfxMessage>& msg);
    bool onCheckCdmaSupported(const sp<RfxMessage>& message);
    bool onCheckSimStatus(const sp<RfxMessage>& message);
    bool onHandleNewSms(const sp<RfxMessage>& message);
    bool isSupportSmsFormatConvert();
    bool isCdmaPhoneMode();
private:
    TimerHandle m_timer;
    bool m_needStatusReport;
    const int MT_SMS_ACK_TIME = 2;
    const char *TAG = "RtcCdmaSmsController";
};
#endif /* __RTC_CDMA_SMS_CONTROLLER_H__ */
