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
#include "RpCdmaOnlyController.h"
#include "Parcel.h"
#include "RfxMainThread.h"
#include <log/log.h>
#include "utils/RefBase.h"
#include <string.h>
#include "util/RpFeatureOptionUtils.h"

/*****************************************************************************
 * Class RpCdmaOnlyController
 * Handle CDMA only HIDL request, and give success response for VTS
 *****************************************************************************/

#undef LOG_TAG
#define LOG_TAG "RpCdmaOnlyController"

RFX_IMPLEMENT_CLASS("RpCdmaOnlyController", RpCdmaOnlyController, RfxController);

RpCdmaOnlyController::RpCdmaOnlyController() {
}

RpCdmaOnlyController::~RpCdmaOnlyController() {
}

void RpCdmaOnlyController::onInit() {

    RfxController::onInit();

    const int request_id_list[] ={
            RIL_REQUEST_NV_READ_ITEM,
            RIL_REQUEST_NV_WRITE_ITEM,
            RIL_REQUEST_NV_WRITE_CDMA_PRL,
            RIL_REQUEST_NV_RESET_CONFIG,
            RIL_REQUEST_CDMA_SUBSCRIPTION,
            RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE,
            RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,
            RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE,
            RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE,
            RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE,
            RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
    };
    if (RpFeatureOptionUtils::isC2kSupport() != 1) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(request_id_list[0]));
    }
}

bool RpCdmaOnlyController::onHandleRequest(const sp<RfxMessage>& message) {
    logD(LOG_TAG, "onHandleRequest id %d", message->getId());

    switch (message->getId()) {
        case RIL_REQUEST_NV_WRITE_ITEM:
        case RIL_REQUEST_NV_WRITE_CDMA_PRL:
        case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
        case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: {
            sp<RfxMessage> voidResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            responseToRilj(voidResponse);
            break;
        }
        case RIL_REQUEST_NV_RESET_CONFIG: {
            sp<RfxMessage> voidResponse = RfxMessage::obtainResponse(RIL_E_REQUEST_NOT_SUPPORTED, message);
            responseToRilj(voidResponse);
            break;
        }
        case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE: {
            sp<RfxMessage> voidResponse = RfxMessage::obtainResponse(RIL_E_SIM_ABSENT, message);
            responseToRilj(voidResponse);
            break;
        }
        case RIL_REQUEST_NV_READ_ITEM: {
            sp<RfxMessage> stringResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            Parcel* responseParcel = stringResponse->getParcel();
            writeStringToParcel(responseParcel, "OK");
            responseToRilj(stringResponse);
            break;
        }
        case RIL_REQUEST_CDMA_SUBSCRIPTION: {
            sp<RfxMessage> stringsResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            Parcel* responseParcel = stringsResponse->getParcel();
            const int count = 5;
            responseParcel->writeInt32(count);
            for (int i = 0; i < count; i++) {
                writeStringToParcel(responseParcel, "-1");
            }
            responseToRilj(stringsResponse);
            break;
        }
        case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
        case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE: {
            sp<RfxMessage> intsResponse = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            Parcel* responseParcel = intsResponse->getParcel();
            responseParcel->writeInt32(1);
            responseParcel->writeInt32(0);
            responseToRilj(intsResponse);
            break;
        }
        default:
            logD(LOG_TAG, "unknown request, ignore!");
            break;
    }
    return true;
}
