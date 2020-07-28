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

#ifndef __RMC_NETWORK_REQUEST_HANDLER_H__
#define __RMC_NETWORK_REQUEST_HANDLER_H__

#include "RmcNetworkHandler.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RmcNwRTReqHdlr"

class RmcNetworkRealTimeRequestHandler : public RmcNetworkHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcNetworkRealTimeRequestHandler);

    public:
        RmcNetworkRealTimeRequestHandler(int slot_id, int channel_id);
        virtual ~RmcNetworkRealTimeRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

    private:
        void resetVoiceRegStateCache(RIL_VOICE_REG_STATE_CACHE *voiceCache, RIL_CACHE_GROUP source);
        void resetDataRegStateCache(RIL_DATA_REG_STATE_CACHE *dataCache, RIL_CACHE_GROUP source);
        void updateVoiceRegStateCache(RIL_VOICE_REG_STATE_CACHE *voice, int source, int count);
        void updateDataRegStateCache(RIL_DATA_REG_STATE_CACHE *data, int source, int count);
        void combineVoiceRegState(const sp<RfxMclMessage>& msg);
        int convertToAndroidRegState(unsigned int uiRegState);
        void combineDataRegState(const sp<RfxMclMessage>& msg);
        void requestVoiceRegistrationState(const sp<RfxMclMessage>& msg);
        int requestVoiceRegistrationStateCdma(const sp<RfxMclMessage>& msg);
        void printVoiceCache(RIL_VOICE_REG_STATE_CACHE cache);
        void printDataCache();
        void requestDataRegistrationState(const sp<RfxMclMessage>& msg);
        void requestDataRegistrationStateGsm();
        void requestOperator(const sp<RfxMclMessage>& msg);
        void requestQueryNetworkSelectionMode(const sp<RfxMclMessage>& msg);
        void updateCaBandInfo();
        void resetCaCache(RIL_CA_CACHE *cacache);
        void updateDcStatus(int cell, int max);
        void updateServiceStateValue();
        void sendVoiceRegResponse(const sp<RfxMclMessage>& msg);
        void sendDataRegResponse(const sp<RfxMclMessage>& msg);
        int isCdmaVoiceInFemtocell();
        int isCdmaDataInFemtocell();
        int isInFemtocell(char *sector_id, char *subnet_mask, int network_id,
                int radio_technology);

    protected:
        int m_slot_id;
        int m_channel_id;
        int m_emergency_only;
        int m_ps_roaming_ind = 0;
        int m_cdma_emergency_only = 0;
        int m_ims_ecc_only = 0;
};

#endif
