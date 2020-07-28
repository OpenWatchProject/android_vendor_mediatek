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

#ifndef __RMC_DC_DEFAULT_REQ_HANDLER_H__
#define __RMC_DC_DEFAULT_REQ_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcDcCommonReqHandler.h"

/*****************************************************************************
 * Class RmcDcDefaultReqHandler
 *****************************************************************************/
class RmcDcDefaultReqHandler : public RmcDcCommonReqHandler {
    public:
        RmcDcDefaultReqHandler(int slot_id, int channel_id, RmcDcPdnManager* pdnManager);
        virtual ~RmcDcDefaultReqHandler();
        virtual void requestSetupDataCall(const sp<RfxMclMessage>& msg);
        virtual void requestDeactivateDataCall(const sp<RfxMclMessage>& msg);
        virtual void onNwPdnAct(const sp<RfxMclMessage>& msg);
        virtual void onNwPdnDeact(const sp<RfxMclMessage>& msg);
        virtual void onNwModify(const sp<RfxMclMessage>& msg);
        virtual void onNwReact(const sp<RfxMclMessage>& msg);
        virtual void onMePdnAct(const sp<RfxMclMessage>& msg);
        virtual void onMePdnDeact(const sp<RfxMclMessage>& msg);
        virtual void onPdnChange(const sp<RfxMclMessage>& msg);
        virtual void onQualifiedNetworkTypeChanged(const sp<RfxMclMessage>& msg);
        virtual void requestSyncApnTable(const sp<RfxMclMessage>& msg);
        virtual void requestSyncDataSettingsToMd(const sp<RfxMclMessage>& msg);
        virtual void requestResetMdDataRetryCount(const sp<RfxMclMessage>& msg);
        void requestLastFailCause(const sp<RfxMclMessage>& msg);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg, Vector<int> *vAidList);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg, int aid);
        virtual void requestQueryPco(int aid, int ia, const char* apn, const char* iptype);
        void requestSetLteAccessStratumReport(const sp<RfxMclMessage>& msg);
        void onLteAccessStratumStateChange(const sp<RfxMclMessage>& msg);
        void requestSetLteUplinkDataTransfer(const sp<RfxMclMessage>& msg);
        void requestSetLteUplinkDataTransferCompleted(bool bSuccess, const sp<RfxMclMessage> msg);
        int convertNetworkType(int nAct);
        virtual void requestClearAllPdnInfo(const sp<RfxMclMessage>& msg);
        virtual void requestResendSyncDataSettingsToMd(const sp<RfxMclMessage>& msg);
        virtual void handleResetAllConnections(const sp<RfxMclMessage>& msg);
        virtual void requestSetPreferredDataModem(const sp<RfxMclMessage>& msg);
        virtual void requestGetDataContextIds(const sp<RfxMclMessage>& msg);
        virtual void requestSendQualifiedNetworkTypesChanged(const sp<RfxMclMessage>& msg);
        virtual void mobileDataUsageNotify(const sp<RfxMclMessage>& msg);
        virtual void requestStartKeepalive(const sp<RfxMclMessage>& msg);
        virtual void requestStopKeepalive(const sp<RfxMclMessage>& msg);
        virtual void updateKeepaliveStatus(const sp<RfxMclMessage>& msg);
        virtual void onNwLimitNotify(const sp<RfxMclMessage>& msg);
};
#endif /* __RMC_DC_DEFAULT_REQ_HANDLER_H__ */
