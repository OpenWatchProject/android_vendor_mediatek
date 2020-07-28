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

#ifndef __RMC_NETWORK_URC_HANDLER_H__
#define __RMC_NETWORK_URC_HANDLER_H__

#include "RmcNetworkHandler.h"
#include "wp/RmcWpRequestHandler.h"

#define MAX_NITZ_TZ_DST_LENGTH      10

typedef struct {
    int cell_data_speed;
    int max_data_bearer;
} RIL_PS_BEARER_CACHE;

class RmcNetworkUrcHandler : public RmcNetworkHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcNetworkUrcHandler);

    public:
        RmcNetworkUrcHandler(int slot_id, int channel_id);
        virtual ~RmcNetworkUrcHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg) { RFX_UNUSED(msg); }

        virtual void onHandleUrc(const sp<RfxMclMessage>& msg);

        virtual void onHandleTimer();

        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

        virtual bool onCheckIfRejectMessage(const sp<RfxMclMessage>& msg,
                RIL_RadioState radioState);

    private:
        void handleCsNetworkStateChanged(const sp<RfxMclMessage>& msg);
        void handlePsDataServiceCapability(const sp<RfxMclMessage>& msg);
        void handleSignalStrength(const sp<RfxMclMessage>& msg);
        void handlePsNetworkStateChanged(const sp<RfxMclMessage>& msg);
        void handleOtaProvisionStatus(const sp<RfxMclMessage>& msg);
        void handleConfirmRatBegin(const sp<RfxMclMessage>& msg);
        void handleGmssRatChanged(const sp<RfxMclMessage>& msg);
        void handleSystemInPrlIndication(const sp<RfxMclMessage>& msg);
        void handleDefaultRoamingIndicator(const sp<RfxMclMessage>& msg);
        void handleNeighboringCellInfo(const sp<RfxMclMessage>& msg);
        void handleNetworkInfo(const sp<RfxMclMessage>& msg);
        void handleGsmFemtoCellInfo(const sp<RfxMclMessage>& msg);
        void handleCdmaFemtoCellInfo(const sp<RfxMclMessage>& msg);
        void handleCellInfoList(const sp<RfxMclMessage>& msg);
        void handleNitzTzReceived(const sp<RfxMclMessage>& msg);
        void handleNitzOperNameReceived(const sp<RfxMclMessage>& msg);
        void handleSib16TimeInfoReceived(const sp<RfxMclMessage>& msg);
        void handleNetworkEventReceived(const sp<RfxMclMessage>& msg);
        void handleMMRRStatusChanged(const sp<RfxMclMessage>& msg);
        void handleWfcStateChanged(const sp<RfxMclMessage>& msg);
        void handleACMT(const sp<RfxMclMessage>& msg);
        void handleModulationInfoReceived(const sp<RfxMclMessage>& msg);
        void handleEnhancedOperatorNameDisplay(const sp<RfxMclMessage>& msg);
        void handlePseudoCellInfo(const sp<RfxMclMessage>& msg);
        void handleNetworkScanResult(const sp<RfxMclMessage>& msg);
        void handleLteNetworkInfo(const sp<RfxMclMessage>& msg);
        void handleMccMncChanged(const sp<RfxMclMessage>& msg);
        void onImsEmergencySupportR9(const sp<RfxMclMessage>& msg);
        void handleCellularQualityReport(const sp<RfxMclMessage>& msg);

        int convertOtaProvisionStatus(int rawState);
        void updatePrlInfo(int system, int mode);
        unsigned int combineWfcEgregState();
        bool enableReportSignalStrengthWithWcdmaEcio();

        const char **allowed_urc;

protected:
        static int bSIB16Received;
        static int bNitzTzAvailble;
        static int bNitzDstAvailble;
        static char ril_nw_nitz_tz[MAX_NITZ_TZ_DST_LENGTH];
        static char ril_nw_nitz_dst[MAX_NITZ_TZ_DST_LENGTH];
        RIL_PS_BEARER_CACHE ril_ps_bearer_cache;

        /* GPRS network registration status URC value */
        int ril_data_urc_status;
        int ril_data_urc_rat;
};

#endif
