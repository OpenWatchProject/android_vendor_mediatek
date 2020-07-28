/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef __RMC_SUPP_SERV_USSD_DOMAIN_SELECTOR_H__
#define __RMC_SUPP_SERV_USSD_DOMAIN_SELECTOR_H__

#include "RfxBaseHandler.h"
#include "RmcSuppServUssdBaseHandler.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "RmcSSUssdDomainSelector"

enum UssiAction {
    USSI_REQUEST  = 1,  // UE initial USSI request
    USSI_RESPONSE = 2   // Response network USSI
};

enum UssdReportCase {
    SEND_NOTHING_BACK  = 0,  // No need to send anything back to framework
    SEND_RESPONSE_BACK = 1,  // Send USSD Response back to framework
    SEND_URC_BACK      = 2   // Send USSD URC back to framework
};

class RmcSuppServUssdDomainSelector : public RmcSuppServUssdBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcSuppServUssdDomainSelector);

    public:
        RmcSuppServUssdDomainSelector(int slot_id, int channel_id);
        virtual ~RmcSuppServUssdDomainSelector();

        virtual void requestSendUssdDomainSelect(const sp<RfxMclMessage>& msg);
        virtual void requestCancelUssdDomainSelect(const sp<RfxMclMessage>& msg);
        virtual void handleOnUssd(const sp<RfxMclMessage>& msg);
        virtual void handleOnUssi(const sp<RfxMclMessage>& msg);

    private:
        void handleUssiCSFB(const sp<RfxMclMessage>& msg);
        void requestSendUSSD(const sp<RfxMclMessage>& msg, UssdReportCase reportCase);
        void requestCancelUssd(const sp<RfxMclMessage>& msg);
        void requestSendUSSI(const sp<RfxMclMessage>& msg);
        void requestCancelUssi(const sp<RfxMclMessage>& msg);
        sp<RfxMclMessage> convertUssiToUssdUrc(const sp<RfxMclMessage>& msg);

        // Operation of USSI action
        UssiAction getUssiAction();
        void setUssiAction(UssiAction action);
        const char *ussiActionToString(UssiAction action);

    private:
        // Java framework has the ability to know which action it should take, then
        // we can pass correct EIUSD's argument to IMS Stack
        // However, if USSI comes from GSM way, there is no dirtect information for RIL SS to
        // determine that, so we need a state variable to remember it.
        // If we get "further user action required" (<n>=1, <m>=1) from USSI URC,
        // we set this state variable to USSI_RESPONSE. It means current USSI session is
        // interactive, AP has to tell IMS stack it's not a request, it's a response.
        // Otherwirse, we treat the action as USSI_REQUEST by default.
        UssiAction mUssiAction;

        // To cancel correct USSD session (CS or IMS), need a flag to memorize
        // which domain it is for current ongoing USSD session
        // 0: CS,  1: IMS
        int mOngoingSessionDomain;

        // Snapshot the USSI string. If we receive error from USSI URC, need to do USSD CSFB
        // using the same string
        char *mUssiSnapshot;

};

#endif
