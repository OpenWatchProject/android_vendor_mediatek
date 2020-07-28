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
#include "RfxHelloController.h"
#include "RfxSampleController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RfxHelloController", RfxHelloController, RfxController);

RfxHelloController::RfxHelloController() :
    m_member_data1(0),
    m_member_data2(1) {
}

RfxHelloController::~RfxHelloController() {
}

void RfxHelloController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    const int request_id_list[] = {
        RIL_REQUEST_GET_IMEI,
        RIL_REQUEST_GET_IMEISV
    };

    const int urc_id_list[] = {
        RIL_UNSOL_ON_USSD,
        RIL_UNSOL_ON_USSD_REQUEST,
        RIL_UNSOL_DATA_CALL_LIST_CHANGED
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleUrc(urc_id_list, 3);

    // register callbacks to get required information
    //getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
    //    RfxStatusChangeCallback(this, &RfxHelloController::onCardTypeChanged));

    RfxSampleController *sample_controller;

    // create the object of RfxSampleController as the child controller
    // of this object (an instance of RfxHelloController)
    RFX_OBJ_CREATE(sample_controller, RfxSampleController, this);

    // connect the signal defined by another module
    sample_controller->m_something_changed_singal.connect(this,
                                    &RfxHelloController::onSampleControlerSomethingChanged);
}

void RfxHelloController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {

    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int card_type = value.asInt();
}


bool RfxHelloController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch (msg_id) {
    case RIL_REQUEST_GET_IMEI: {
            /* Scenario 1: No modify to RfxMessage. Send the RfxMessage request to RILD directly. */
            requestToRild(message);

            /* Scenario 2: Create a new RfxMessage based on an existing RfxMessage.
             *     (without copying Parcel data to new message)
             */
            sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                    RIL_REQUEST_GET_IMEI, message);
                    // 1. Obtain function will create the Parcel in new RfxMessage automatically.
                    // 2. New Parcel has NO Data copied from base message.
                    // 3. Do manual Parcel data copy if needed.
                    // {
                    //     c2k_request->getParcel()->writeInt32(2);
                    //     c2k_request->getParcel()->writeInt32(2);
                    // }
            requestToRild(c2k_request);

            /* Scenario 3: Create a new RfxMessage based on an existing RfxMessage.
             *             (copy Parcel data to new message)

            sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                    RIL_REQUEST_GET_IMEI, message, true);
                    // 1. Obtain function will create the Parcel in new RfxMessage automatically.
                    // 2. Parcel data will also be copied into the new RfxMessage Parcel object.
            requestToRild(c2k_request);

            */

            /* Scenario 4: Create a new RfxMessage without a RfxMessage base.

            int slotId = 0;
            sp<RfxMessage> c2k_request = RfxMessage::obtainRequest(slotId, RADIO_TECH_GROUP_C2K,
                    RIL_REQUEST_GET_IMEI);
                    // 1. Obtain function will create the Parcel in new RfxMessage automatically.
                    // 2. Prepare Parcel data manually
                    // {
                    //     c2k_request->getParcel()->writeInt32(4);
                    //     c2k_request->getParcel()->writeInt32(4);
                    // }
            requestToRild(c2k_request);

            */

            // post invoke a function if required
            rfxPostInvoke0(this, &RfxHelloController::onGetIMEI);

            m_timer_handle = RfxTimer::start(RfxCallback0(this, &RfxHelloController::onTimer), s2ns(1));
        }
        break;
    default:
        break;
    }
    return true;
}

void RfxHelloController::onGetIMEI() {
    // post to do something
}

bool RfxHelloController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    if (msg_id == RIL_UNSOL_ON_USSD) {
        // 1. decompress message.parcel to get URC data

        // 2. if any predefined status need to be shared to other modules
        //    set it to status manager, status manager will inform
        //    registed callbacks
        //getStatusManager()->setIntValue(RFX_STATUS_KEY_CARD_TYPE, 2);

        RfxTimer::stop(m_timer_handle);

        // 3. if the URC need to be sent to RILJ, send it,
        //    be able to update parceled data if required
        responseToRilj(message);
    }
    return true;
}

bool RfxHelloController::onHandleResponse(const sp<RfxMessage>& message) {
    responseToRilj(message);
    sp<RfxMessage> msg = sp<RfxMessage>(NULL);
    ResponseStatus responseStatus = preprocessResponse(message, msg,
        RfxWaitResponseTimedOutCallback(this, &RfxHelloController::responseCallBack), s2ns(10));

    if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
        // DO SOMETHING.
    } else if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
         // DO SOMETHING.
    }
    return true;
}

void RfxHelloController::responseCallBack(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    /// DO Something.
}

void RfxHelloController::onSampleControlerSomethingChanged(int d1, int d2, int d3, int d4) {
    RFX_UNUSED(d1);
    RFX_UNUSED(d2);
    RFX_UNUSED(d3);
    RFX_UNUSED(d4);
    // do something
}

void RfxHelloController::onTimer() {
    // do something
}

