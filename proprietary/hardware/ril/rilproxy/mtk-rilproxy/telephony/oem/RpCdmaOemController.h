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

#ifndef __RP_CDMA_OEM_CONTROLLER_H__
#define __RP_CDMA_OEM_CONTROLLER_H__

/***************************************************************************** 
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxTimer.h"
#include <utils/String16.h>

using ::android::String16;
/***************************************************************************** 
 * Class RpCdmaOemController
 *****************************************************************************/

class RpCdmaOemController : public RfxController {
    RFX_DECLARE_CLASS(RpCdmaOemController); // Required: declare this class
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RpCdmaOemController);

public:

    RpCdmaOemController();

    virtual ~RpCdmaOemController();

// Override
protected:

    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);

    virtual void onInit();

public:
    void responseCallBack(const sp<RfxMessage>& message);

private:
    String16 m_str_cdma_imei;
    String16 m_str_cdma_imeisv;
    String16 m_str_cdma_esnHex;
    String16 m_str_cdma_meidHex;
    String16 m_str_cdma_uimid;
    String16 m_str_gsm_imei;
    String16 m_str_gsm_imeisv;
    String16 m_str_cdma_esnDec;
    String16 m_str_cdma_meidDec;

private:
    void requestBaseBand(const sp<RfxMessage>& message);
    void requestDeviceIdentity(const sp<RfxMessage>& message);
    void responseBaseBand(const sp<RfxMessage>& message);
    void responseDeviceIdentity(const sp<RfxMessage>& message);
    void handleOemHookRawReq(const sp<RfxMessage>& message);
    void handleOemHookStringsReq(const sp<RfxMessage>& message);
    void handleScreenStateReq(const sp<RfxMessage>& message);
    void handleScreenStateRsp(const sp<RfxMessage>& message);
    void responseSetScreenState(const sp<RfxMessage>& message);
    void getMeidFromMessage(const sp<RfxMessage>& message);
    void getImeiFromMessage(const sp<RfxMessage>& message);
    int judgeTrmMode(int mode);
    void requestSetTrm(const sp<RfxMessage>& message);
    void responseSetTrm(const sp<RfxMessage>& message);
    void requestSetUnsolResponseFilter(const sp<RfxMessage>& message);
    void responseSetUnsolResponseFilter(const sp<RfxMessage>& message);
    void requestSendDeviceState(const sp<RfxMessage>& message);
    void responseSendDeviceState(const sp<RfxMessage>& message);
    bool handleExtraState(const sp<RfxMessage>& message);
};

#endif /* __RP_CDMA_OEM_CONTROLLER_H__ */
