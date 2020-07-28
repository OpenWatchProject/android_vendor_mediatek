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

#ifndef __VIA_BASE_HANDLER_H__
#define __VIA_BASE_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxBaseHandler.h"
#include "nw/RmcNetworkHandler.h"

/*****************************************************************************
 * Class ViaBaseHandler
 *****************************************************************************/

typedef enum {
    SIGNAL_CDMA_DBM = 0,
    SIGNAL_CDMA_ECIO = 1,
    SIGNAL_EVDO_DBM = 2,
    SIGNAL_EVDO_ECIO = 3
} C2K_SIGNAL;

class ViaBaseHandler {
    public:
        ViaBaseHandler() {}
        virtual ~ViaBaseHandler() {}

        virtual void sendCommandDemo(RfxBaseHandler* handler, char* str) = 0;
        virtual void handleCdmaSubscription(RfxBaseHandler* handler, char **p_response, RIL_Errno *result) = 0;
        virtual void requestSetPreferredVoicePrivacyMode(RfxBaseHandler* handler, int value, RIL_Errno *result) = 0;
        virtual void requestQueryPreferredVoicePrivacyMode(RfxBaseHandler* handler, int *value, RIL_Errno *result) = 0;
        virtual void handleCdmaPrlChanged(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler,
                int slotId);
        virtual void registerForViaUrc(RfxBaseHandler* handler);
        virtual void handleViaUrc(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler, int slotId);
        virtual const char** getViaAllowedUrcForNw() = 0;
        virtual int convertCdmaEvdoSig(int sig, int tag);
        virtual int getCdmaLocationInfo(RfxBaseHandler* handler,
                RIL_VOICE_REG_STATE_CACHE *voice_reg_state_cache);
        virtual const char **getAgpsUrc() = 0;
        virtual void requestAgpsConnind(RfxBaseHandler* handler, int connected, RIL_Errno *result) = 0;
        virtual int getCdmaLocationInfo(RfxBaseHandler* handler,
                CDMA_CELL_LOCATION_INFO *cdma_cell_location);
};

typedef ViaBaseHandler* create_t();
typedef void destroy_t(ViaBaseHandler*);

#endif /* __VIA_BASE_HANDLER_H__ */

