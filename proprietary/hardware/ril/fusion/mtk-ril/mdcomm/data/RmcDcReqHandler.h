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

#ifndef __RMC_DC_REQ_HANDLER_H__
#define __RMC_DC_REQ_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcDataDefs.h"
#include "RmcDcDefaultReqHandler.h"
#include "RmcDcImsReqHandler.h"
#include "RmcDcOnDemandReqHandler.h"
#include "RmcDcPdnManager.h"

/*****************************************************************************
 * Class RmcDcReqHandler
 *****************************************************************************/
class RmcDcReqHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcDcReqHandler);

    public:
        RmcDcReqHandler(int slot_id, int channel_id);
        virtual ~RmcDcReqHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);
        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

    private:
        void handleLastFailCauseRequest(const sp<RfxMclMessage>& msg);
        void handleSetupDataCallRequest(const sp<RfxMclMessage>& msg);
        void handleDeactivateDataCallRequest(const sp<RfxMclMessage>& msg);
        void handleDataCallListRequest(const sp<RfxMclMessage>& msg);
        int deactivateDataCall(int aid);
        void handleNwPdnAct(const sp<RfxMclMessage>& msg);
        void handleNwPdnDeact(const sp<RfxMclMessage>& msg);
        void handleNwModify(const sp<RfxMclMessage>& msg);
        void handleNwReact(const sp<RfxMclMessage>& msg);
        void handleMePdnAct(const sp<RfxMclMessage>& msg);
        void handleMePdnDeact(const sp<RfxMclMessage>& msg);
        void handlePdnChange(const sp<RfxMclMessage>& msg);
        void handleSyncApnTableRequest(const sp<RfxMclMessage>& msg);
        void handleSetInitialAttachApnRequest(const sp<RfxMclMessage>& msg);
        void handleSyncDataSettingsToMdRequest(const sp<RfxMclMessage>& msg);
        void handleQualifiedNetworkTypeChanged(const sp<RfxMclMessage>& msg);
        void handleResetMdDataRetryCount(const sp<RfxMclMessage>& msg);
        void handleUtTest(const sp<RfxMclMessage>& msg);
        void handleSetLteAccessStratumReportRequest(const sp<RfxMclMessage>& msg);
        void handleLteAccessStratumStateChange(const sp<RfxMclMessage>& msg);
        void handleSetLteUplinkDataTransferRequest(const sp<RfxMclMessage>& msg);
        void handleClearAllPdnInfoRequest(const sp<RfxMclMessage>& msg);
        void handleResendSyncDataSettingsToMd(const sp<RfxMclMessage>& msg);
        void handleQueryPco(const sp<RfxMclMessage>& msg);
        void handleResetAllConnections(const sp<RfxMclMessage>& msg);
        void handleSetPreferredDataModem(const sp<RfxMclMessage>& msg);
        bool isValidInitialAttachApn(const char* apn);
        void handleGetDataContextIds(const sp<RfxMclMessage>& msg);
        void handleSendQualifiedNetworkTypesChanged(const sp<RfxMclMessage>& msg);
        void handleIfVendorSelfIaNeeded(const sp<RfxMclMessage>& msg);
        void handleMobileDataUsageNotify(const sp<RfxMclMessage>& msg);
        void handleStartKeepaliveRequest(const sp<RfxMclMessage>& msg);
        void handleStopKeepaliveRequest(const sp<RfxMclMessage>& msg);
        void handleKeepaliveStatus(const sp<RfxMclMessage>& msg);
        void handleNwLimitNotify(const sp<RfxMclMessage>& msg);

    private:
        RmcDcDefaultReqHandler *m_pRmcDcDefaultReqHdlr;
        RmcDcImsReqHandler *m_pRmcDcImsReqHdlr;
        RmcDcOnDemandReqHandler *m_pRmcDcOnDemandReqHdlr;
        RmcDcPdnManager *m_pPdnManager;
        int iaMode;
};
#endif /* __RMC_DC_REQ_HANDLER_H__ */

