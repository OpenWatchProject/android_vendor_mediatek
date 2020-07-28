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

#ifndef __RMC_SUPP_SERV_REQUEST_HANDLER_H__
#define __RMC_SUPP_SERV_REQUEST_HANDLER_H__

#include "RmcSuppServRequestBaseHandler.h"
#include "RmcSuppServUssdBaseHandler.h"
#include "SuppServDef.h"
#include <string>

#ifdef TAG
#undef TAG
#endif
#define TAG "RmcSSHandler"

using std::string;

class RmcSuppServRequestHandler : public RmcSuppServRequestBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcSuppServRequestHandler);

    public:
        RmcSuppServRequestHandler(int slot_id, int channel_id);
        virtual ~RmcSuppServRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);
        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);
        virtual void onHandleTimer();

    private:
        void requestSetClir(const sp<RfxMclMessage>& msg);
        void requestGetClir(const sp<RfxMclMessage>& msg);
        void requestSetCallForward(const sp<RfxMclMessage>& msg);
        void requestSetCallForwardInTimeSlot(const sp<RfxMclMessage>& msg);
        void requestSetCallWaiting(const sp<RfxMclMessage>& msg);
        void requestChangeBarringPassword(const sp<RfxMclMessage>& msg);
        void requestSendUSSD(const sp<RfxMclMessage>& msg);
        void requestCancelUssd(const sp<RfxMclMessage>& msg);
        void requestQueryClip(const sp<RfxMclMessage>& msg);
        void requestSetClip(const sp<RfxMclMessage>& msg);

        // Should be in protect.
        void requestGetColp(const sp<RfxMclMessage>& msg);
        void requestSetColp(const sp<RfxMclMessage>& msg);
        void requestGetColr(const sp<RfxMclMessage>& msg);
        void requestSetColr(const sp<RfxMclMessage>& msg);
        void requestSendCNAP(const sp<RfxMclMessage>& msg);
        void requestSendUSSI(const sp<RfxMclMessage>& msg);
        void requestCancelUssi(const sp<RfxMclMessage>& msg);
        void requestSetSuppSvcNotification(const sp<RfxMclMessage>& msg);
        void requestSetupXcapUserAgentString(const sp<RfxMclMessage>& msg);
        void requestSetSuppServProperty(const sp<RfxMclMessage>& msg);
        void requestGetSuppServProperty(const sp<RfxMclMessage>& msg);
        void requestGetModemVersion();
        void requestSetXcapConfig(const sp<RfxMclMessage>& msg);
        void syncCLIRToStatusManager();

        void* startUtInterface(const char* libutinterfacePath);

    private:
        RmcSuppServUssdBaseHandler *m_UssdHandler;

        /**
         * When EM wants to reset XCAP config, it should be trigger by oemhook using
         * reset and resetdone command. And except the reset cmd, all properties only need
         * to set the property for UI to get status.
         */
        bool isResetSession;
        void* hDll = NULL;
};

#endif
