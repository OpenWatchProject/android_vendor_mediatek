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

#ifndef __RMC_SUPP_SERV_REQUEST_BASE_HANDLER_H__
#define __RMC_SUPP_SERV_REQUEST_BASE_HANDLER_H__

#include "RfxBaseHandler.h"
#include "RfxStringData.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxStringsData.h"
#include "SuppServDef.h"
#include "RfxCallForwardInfoData.h"
#include "RfxCallForwardInfosData.h"
#include "RfxCallForwardInfoExData.h"
#include "RfxCallForwardInfosExData.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "RmcSSBaseHandler"

class RmcSuppServRequestBaseHandler : public RfxBaseHandler {
    public:
        RmcSuppServRequestBaseHandler(int slot_id, int channel_id);
        virtual ~RmcSuppServRequestBaseHandler();

    protected:
        void requestClirOperation(const sp<RfxMclMessage>& msg);
        void requestCallForwardOperation(const sp<RfxMclMessage>& msg, CallForwardOperationE op);
        void requestCallForwardExOperation(const sp<RfxMclMessage>& msg, CallForwardOperationE op);
        void requestCallWaitingOperation(const sp<RfxMclMessage>& msg, CallWaitingOperationE op);
        bool checkTerminalBaseCallWaitingStatus(char* tbCWStatus,
                        int* responses, const int* inputData, CallWaitingOperationE op);
        int checkTerminalBaseCallWaitingType(sp<RfxAtResponse> p_response);
        void handleTerminalBaseCallWaitingResponse(const char* tbCWStatus, int* responses,
                int sendBsCode, const int* inputData, int &responseForAll,
                sp<RfxAtResponse> p_response, CallWaitingOperationE op);
        void requestColpOperation(const sp<RfxMclMessage>& msg);
        void requestColrOperation(const sp<RfxMclMessage>& msg);
        void requestCallBarring(const sp<RfxMclMessage>& msg, CallBarringOperationE op);
        void sleepMsec(long long msec);
        char* parseErrorMessageFromXCAP(sp<RfxAtResponse> p_response);
        void handleSetClirResponse(int clir_n);
        void handleGetClirResponse(int (&responses)[2], bool isTerminalBasedSolution);
        void resetClirProperty();
        void requestResetSuppServ(const sp<RfxMclMessage>& msg);
        void handleErrorMessageFromXcap(sp<RfxAtResponse> p_response, AT_CME_Error errorCode);
};

#endif
