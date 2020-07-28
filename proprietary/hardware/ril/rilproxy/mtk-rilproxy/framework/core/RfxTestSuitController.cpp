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
#include "RfxTestSuitController.h"
#include "RfxTestBasicController.h"
#include "RfxMainThread.h"
#include "RfxLog.h"

#define RFX_TEST_SUIT_LOG_TAG "RFX_TEST_SUIT_CONTROLLER"

#define RFX_TEST_CONTROLLER_ENTRY(_className) _className::createInstance

/*****************************************************************************
 * Class RfxTestSuitController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RfxTestSuitController", RfxTestSuitController, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxTestSuitController);

const RfxCreateControllerFuncPtr RfxTestSuitController::s_test_controllers[] = {
//    RFX_TEST_CONTROLLER_ENTRY(RfxTestHelloController)
};

void RfxTestSuitController::onInit() {
    RfxController::onInit();
    if(isEnableTest()) {
        RFX_LOG_D(RFX_TEST_SUIT_LOG_TAG, "onInit");
        const int request_id_list[] = {RIL_REQUEST_LOCAL_TEST};
        registerToHandleRequest(request_id_list, 1);
        RfxMainThread::enqueueMessage(
            RfxMessage::obtainRequest(0, RADIO_TECH_GROUP_C2K, RIL_REQUEST_LOCAL_TEST));
    }
}

bool RfxTestSuitController::onHandleRequest(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    uint32_t test_controller_size =
        sizeof(s_test_controllers) / sizeof(RfxCreateControllerFuncPtr);
    RFX_LOG_D(RFX_TEST_SUIT_LOG_TAG, "onHandleRequest, size: %d", test_controller_size);
    if(m_index < test_controller_size) {
        m_testObj = (RfxTestBasicController *)s_test_controllers[m_index](this);
        m_testObj->run();
    }
    return true;
}

bool RfxTestSuitController::isEnableTest() {
    return mTestSwitcher;
}

void RfxTestSuitController::checkSuccessAndEnqueueNext() {
    uint32_t test_controller_size = sizeof(s_test_controllers) / sizeof(RfxCreateControllerFuncPtr);
    RFX_LOG_D(RFX_TEST_SUIT_LOG_TAG, "checkSuccessAndEnqueueNext, size: %d", test_controller_size);
    if(m_index < test_controller_size) {
        m_testObj->checkSuccess();
        m_index++;
        RFX_OBJ_CLOSE(m_testObj);
    }
    if (m_index < test_controller_size) {
        RfxMainThread::enqueueMessage(RfxMessage::obtainRequest(0, RADIO_TECH_GROUP_C2K, RIL_REQUEST_LOCAL_TEST));
    }
}

