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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RpCdmaNwRatSwitchHandler.h"

/*****************************************************************************
 * Class RpCdmaNwRatSwitchHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCdmaNwRatSwitchHandler", RpCdmaNwRatSwitchHandler,
        RpBaseNwRatSwitchHandler);

RpCdmaNwRatSwitchHandler::RpCdmaNwRatSwitchHandler() :
        RpBaseNwRatSwitchHandler() {
}

RpCdmaNwRatSwitchHandler::RpCdmaNwRatSwitchHandler(RpNwRatController* nwRatController) :
    RpBaseNwRatSwitchHandler(nwRatController) {
}

RpCdmaNwRatSwitchHandler::~RpCdmaNwRatSwitchHandler() {
}

void RpCdmaNwRatSwitchHandler::requestGetPreferredNetworkType(const sp<RfxMessage>& message) {
    RpBaseNwRatSwitchHandler::requestGetPreferredNetworkType(RADIO_TECH_GROUP_C2K, message);
}

void RpCdmaNwRatSwitchHandler::responseGetPreferredNetworkType(const sp<RfxMessage>& response) {
    int error = response->getError() ;
    if (error != RIL_E_SUCCESS) {
        // error
        RFX_LOG_D(LOG_TAG, "responseGetPreferredNetworkType(), error is %d.", error);
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        int32_t stgCount;
        int32_t nwType = -1;
        response->getParcel()->readInt32(&stgCount);
        response->getParcel()->readInt32(&nwType);
        if (source == RADIO_TECH_GROUP_C2K) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            resToRilj->getParcel()->writeInt32(nwType);
            mNwRatController->responseToRilj(resToRilj);
            RFX_LOG_D(LOG_TAG, "responseGetPreferredNetworkType(), send response %d.", nwType);
        }
    }
}

void RpCdmaNwRatSwitchHandler::responseSetPreferredNetworkType(const sp<RfxMessage>& response) {
    int error = response->getError() ;
    if (error != RIL_E_SUCCESS) {
        // error
        RFX_LOG_D(LOG_TAG, "responseSetPreferredNetworkType(), error is %d.", error);
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        if (source == RADIO_TECH_GROUP_C2K) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            mNwRatController->responseToRilj(resToRilj);
            RFX_LOG_D(LOG_TAG, "responseSetPreferredNetworkType(), send response");
        }
    }
}
