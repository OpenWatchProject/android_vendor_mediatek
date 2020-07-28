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

#ifndef __RP_CDMA_SMS_CTRL_H__
#define __RP_CDMA_SMS_CTRL_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RpSmsCtrlBase.h"
#include "RpCdmaSmsCodec.h"
#include "RpCdmaSmsDefs.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define RP_CDMA_SMS_CTRL_ACK_ERROR_CLASS_TEMP_ERROR             2
#define RP_CDMA_SMS_CTRL_ACK_ERROR_CAUSE_DESTINATION_BUSY       33

/*****************************************************************************
 * Class RpCdmaSmsCtrl
 *****************************************************************************/
class RpCdmaSmsCtrl : public RpSmsCtrlBase {
    // Required: declare this class
    RFX_DECLARE_CLASS(RpCdmaSmsCtrl);

public:
    RpCdmaSmsCtrl();
    /**
     * Check RIL_REQUEST_IMS_SEND_SMS is 3gpp or 3gpp2.
     *
     * @param message RIL_REQUEST_IMS_SEND_SMS RfxMessage type.
     */
    bool is3gpp2ImsReq(const sp<RfxMessage>& message);

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
    bool isCtSimCard(void);
    void onCdmaSmsSend(const sp<RfxMessage>& message);
    void onHandleSmscAddress(const sp<RfxMessage>& message);
    bool handleImsSendSms(const sp<RfxMessage>& message);
    bool handleImsSendCdmaSms(const sp<RfxMessage>& message);
    bool handleCdmaSmsAck(const sp<RfxMessage>& message);
    bool handleCdmaSmsSpecificToGsmRsp(const sp<RfxMessage>& message);
    bool handleCdmaNewSms(const sp<RfxMessage>& message);
    bool handleCdmaSmsSpecificToGsmUrc(const sp<RfxMessage>& message);
    /**
     * Generic error response to rilj.
     * While receiveing the RIL_REQUEST_IMS_SEND_SMS with 3GPP2 format but meet the general
     * error, it will response to rilj with RIL_E_GENERIC_FAILURE
     *
     * @param message RIL_REQUEST_IMS_SEND_SMS RfxMessage type. It is used to get the common
     *     information from this message such as slot id
     */
    void genericErrorRspToRilj(const sp<RfxMessage>& message);
    void onHandleRuimSms(const sp<RfxMessage>& message);

// variable
private:
    const int TELESERVICE_WMT    = 0x1002;
    const int TELESERVICE_WEMT   = 0x1005;

    RpCdmaSmsCodec *mCodec;
    bool mIsHandlingImsMoSms;
    bool mIsHandlingImsMtSms;
    bool mIsHandlingCsMtSms;
    int mMtSmsSeqId;
    bool mNeedStatusReport;
    RIL_CDMA_SMS_Address mMtSmsAddress;
};

#endif /* __RP_CDMA_SMS_CTRL_H__ */
