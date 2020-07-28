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

#ifndef __RMC_WP_REQUEST_HANDLER_H__
#define __RMC_WP_REQUEST_HANDLER_H__

#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxStringData.h"
#include "RfxMessageId.h"
#include "RfxVoidData.h"
#include <libmtkrilutils.h>

typedef enum {
    WORLD_PHONE_POLYCY_OP01A = 1,
    WORLD_PHONE_POLYCY_OP01B = 2,
    WORLD_PHONE_POLYCY_OM = 3,
    WORLD_PHONE_POLYCY_OP02 = 4,
} WorldPhonePolicyType;

class RmcWpRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcWpRequestHandler);

    public:
        RmcWpRequestHandler(int slot_id, int channel_id);
        virtual ~RmcWpRequestHandler();

    public:
        static int isWorldModeSwitching();
        void setActiveModemType();
        int getActiveModemType();
        int queryActiveMode();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

    private:
        void requestResumingWorldMode(const sp<RfxMclMessage>& msg);
        void onWorldModeStateChanged(int state);
        void handleWorldModeChanged(const sp<RfxMclMessage>& msg);
        WorldPhonePolicyType getWorldPhonePolicy();
        void setWorldPhonePolicy();
        int isWorldPhoneSupport();
        SVLTE_PROJ_TYPE getSvlteProjectType();
        int isSvlteLcgSupport();
        void worldPhoneInitialize(int slot_id);

    private:
        static int bWorldModeSwitching;
        static int ecsraUrcParams[5];
        static RIL_RadioState radioStateBackup[MAX_SIM_COUNT];
};

#endif /*__RMC_WP_REQUEST_HANDLER_H__*/
