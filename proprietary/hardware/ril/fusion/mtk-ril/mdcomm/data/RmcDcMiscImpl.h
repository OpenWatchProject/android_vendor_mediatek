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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RfxVoidData.h"
#include "RfxLceStatusResponseData.h"
#include "RfxLceDataResponseData.h"
#include "RfxLinkCapacityEstimateData.h"

/*****************************************************************************
 * Class RmcDcMiscImpl
 *****************************************************************************/
class RmcDcMiscImpl {
    public:
        RmcDcMiscImpl(RfxBaseHandler* handler, int slotId);
        virtual ~RmcDcMiscImpl();
        void init();
        void setFdMode(const sp<RfxMclMessage>& msg);
        void requestStartLce(const sp<RfxMclMessage>& msg);
        void requestStopLce(const sp<RfxMclMessage>& msg);
        void requestPullLceData(const sp<RfxMclMessage>& msg);
        void onLceStatusChanged(const sp<RfxMclMessage>& msg);
        void requestSetLinkCapacityReportingCriteria(const sp<RfxMclMessage>& msg);
        void onLinkCapacityEstimate(const sp<RfxMclMessage>& msg);

    protected:
        int getIntValue(int slotId, const RfxStatusKeyEnum key, int default_value);
        RfxMclStatusManager* getMclStatusManager(int slotId);
        sp<RfxAtResponse> atSendCommand(const String8 cmd);
        sp<RfxAtResponse> atSendCommandSingleline(const String8 cmd, const char *rspPrefix);
        void responseToTelCore(const sp<RfxMclMessage> msg);

    private:
        void initializeFastDormancy();
        int isFastDormancySupport();
        void syncEpdgConfigToMd();
        void syncIwlanOperationModeToMd();
        void disableLegacyFastDormancy();

    private:
        RfxBaseHandler* mHandler;
        int mSlotId;
};
