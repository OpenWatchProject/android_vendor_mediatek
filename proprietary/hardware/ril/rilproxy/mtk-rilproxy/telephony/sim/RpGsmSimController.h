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

#ifndef __RP_GSM_SIM_CONTROLLER_H__
#define __RP_GSM_SIM_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpSimControllerBase.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

class RpGsmSimController : public RpSimControllerBase {
    RFX_DECLARE_CLASS(RpGsmSimController); // Required: declare this class

public:

    RpGsmSimController();

    virtual ~RpGsmSimController();

// Override
protected:

    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);

    virtual void onInit();


public:

    void handlePinPukReq(const sp<RfxMessage>& message);
    void handlePinPukRsp(const sp<RfxMessage>& message);

    void handleQuerySimFacilityReq(const sp<RfxMessage>& message);
    void handleQuerySimFacilityRsp(const sp<RfxMessage>& message);

    void handleSetSimFacilityReq(const sp<RfxMessage>& message);
    void handleSetSimFacilityRsp(const sp<RfxMessage>& message);

    void handleIccOpenChannelReq(const sp<RfxMessage>& message);
    void handleIccOpenChannelRsp(const sp<RfxMessage>& message);

    void handleIccCloseChannelReq(const sp<RfxMessage>& message);
    void handleIccCloseChannelRsp(const sp<RfxMessage>& message);

    void handleTransmitApduReq(const sp<RfxMessage>& message);
    void handleTransmitApduRsp(const sp<RfxMessage>& message);

    void handleGetAtrReq(const sp<RfxMessage>& message);
    void handleGetAtrRsp(const sp<RfxMessage>& message);

    void handleSetUiccSubscriptionReq(const sp<RfxMessage>& message);
    void handleSetUiccSubscriptionRsp(const sp<RfxMessage>& message);

    void handleEnterNetworkDepersonalizationReq(const sp<RfxMessage>& message);
    void handleEnterNetworkDepersonalizationRsp(const sp<RfxMessage>& message);

    void handleSetSimNetworkLockReq(const sp<RfxMessage>& message);
    void handleSetSimNetworkLockRsp(const sp<RfxMessage>& message);

    void handleSimIoReq(const sp<RfxMessage>& message);
    void handleSimIoRsp(const sp<RfxMessage>& message);

    void handleSimAuthenticationReq(const sp<RfxMessage>& message);
    void handleSimAuthenticationRsp(const sp<RfxMessage>& message);

    void handleSimTransmitApduBasicReq(const sp<RfxMessage>& message);
    void handleSimTransmitApduBasicRsp(const sp<RfxMessage>& message);

    void handleSubsidylockRequest(const sp<RfxMessage>& message);
    void handleSubsidylockResponse(const sp<RfxMessage>& message);

    void handleSetAllowedCarriersReq(const sp<RfxMessage>& message);
    void handleSetAllowedCarriersRsp(const sp<RfxMessage>& message);

    void handleGetAllowedCarriersReq(const sp<RfxMessage>& message);
    void handleGetAllowedCarriersRsp(const sp<RfxMessage>& message);

private:
    void sendReq(const sp<RfxMessage>& message);
    void sendRsp(const sp<RfxMessage>& message);
};

#endif /* __RP_GSM_SIM_CONTROLLER_H__ */

