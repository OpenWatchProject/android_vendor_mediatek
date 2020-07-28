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

#ifndef __VIA_HANDLER_H__
#define __VIA_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxBaseHandler.h"
#include "ViaBaseHandler.h"

/*****************************************************************************
 * Class ViaHandler
 *****************************************************************************/

class ViaHandler : public ViaBaseHandler {
    public:
        ViaHandler();
        virtual ~ViaHandler();
        void sendCommandDemo(RfxBaseHandler* handler, char* str);
        void handleCdmaSubscription(RfxBaseHandler* handler, char **p_response, RIL_Errno *result);
        void requestSetPreferredVoicePrivacyMode(RfxBaseHandler* handler, int value, RIL_Errno *result);
        void requestQueryPreferredVoicePrivacyMode(RfxBaseHandler* handler, int *value, RIL_Errno *result);
        void handleCdmaPrlChanged(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler, int slotId);
        void registerForViaUrc(RfxBaseHandler* handler);
        void handleViaUrc(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler, int slotId);
        const char** getViaAllowedUrcForNw();
        int convertCdmaEvdoSig(int sig, int tag);
        int getCdmaLocationInfo(RfxBaseHandler* handler, RIL_VOICE_REG_STATE_CACHE *voice_reg_state_cache);
        const char** getAgpsUrc();
        void requestAgpsConnind(RfxBaseHandler* handler, int connected, RIL_Errno *result);
        int getCdmaLocationInfo(RfxBaseHandler* handler,
                CDMA_CELL_LOCATION_INFO *cdma_cell_location);
    private:
        int mSystemId;
};

extern "C" ViaBaseHandler* create() {
    return new ViaHandler;
}

extern "C" void destroy(ViaBaseHandler* handler) {
    delete handler;
}

#endif /* __VIA_HANDLER_H__ */

