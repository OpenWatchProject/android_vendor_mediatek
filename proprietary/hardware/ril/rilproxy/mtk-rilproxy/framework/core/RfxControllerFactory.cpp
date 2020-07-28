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
/*
 * File name:  rfx_controller_factory.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Implementation of controller factory.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxControllerFactory.h"
#include "RfxController.h"
#include "RfxRootController.h"
#include "RfxSlotRootController.h"

#include "RfxRilAdapter.h"
#include "RfxHelloController.h"
#include "call/RpCallController.h"
#include "call/RpCallCenterController.h"
#include "cat/RpCatController.h"
#include "capabilityswitch/RpCapabilitySwitchController.h"
#include "capabilityswitch/RpCapabilityGetController.h"
#include "sim/RpSimController.h"
#include "power/RpModemController.h"
#include "power/RpRadioController.h"
#include "power/RpCdmaRadioController.h"
#include "data/RpDataController.h"
#include "data/RpDataAllowController.h"
#include "data/RpDataOpController.h"
#include "modecontroller/RpCdmaLteModeController.h"
#include "modecontroller/RpModeInitController.h"
#include "oem/RpCdmaOemController.h"
#include "phonenumber/RpPhoneNumberController.h"
#include "phb/RpPhbController.h"
#include "nw/RpNwStateController.h"
#include "nw/RpNwController.h"
#include "sms/RpSmsCtrl.h"
#include "sms/RpSmsNSlotController.h"
#include "lwx/RpLwxController.h"
#include "atci/RpAtciController.h"
#include "oem/RpCdmaOnlyController.h"
#include "ss/RpSsController.h"

/// M: eMBMS feature
#include "embms/RpEmbmsControllerProxy.h"
#include "embms/RpEmbmsAtController.h"
/// M: eMBMS end
#include "client/RpRilClientController.h"
#include "client/RpMalController.h"

#include "ims/RpImsController.h"
#include "agps/RpAgpsSlotController.h"
#include "agps/RpAgpsNSlotController.h"

/*****************************************************************************
 * Define
 *****************************************************************************/

#define RFX_CONTROLLER_CREATION_ENTRY(_className) _className::createInstance

/*****************************************************************************
 * Class RfxControllerFactory
 *****************************************************************************/
const RfxCreateControllerFuncptr RfxControllerFactory::s_pre_non_slot_controllers[] = {
    RFX_CONTROLLER_CREATION_ENTRY(RpModeInitController)
};

const RfxCreateControllerFuncptr RfxControllerFactory::s_slot_controllers[] = {
//    RFX_CONTROLLER_CREATION_ENTRY(RfxHelloController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCallController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCatController),
    RFX_CONTROLLER_CREATION_ENTRY(RpNwStateController),
    RFX_CONTROLLER_CREATION_ENTRY(RpSimController),
    RFX_CONTROLLER_CREATION_ENTRY(RpDataController),
    RFX_CONTROLLER_CREATION_ENTRY(RpDataOpController),
    RFX_CONTROLLER_CREATION_ENTRY(RpRadioController),
    RFX_CONTROLLER_CREATION_ENTRY(RpNwController),
    RFX_CONTROLLER_CREATION_ENTRY(RpPhoneNumberController),
    RFX_CONTROLLER_CREATION_ENTRY(RpPhbController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCdmaOemController),
    RFX_CONTROLLER_CREATION_ENTRY(RpSmsCtrl),
    RFX_CONTROLLER_CREATION_ENTRY(RpAtciController),
    RFX_CONTROLLER_CREATION_ENTRY(RpAgpsSlotController),
/// M: eMBMS feature
    RFX_CONTROLLER_CREATION_ENTRY(RpEmbmsControllerProxy),
/// M: eMBMS end
    RFX_CONTROLLER_CREATION_ENTRY(RpImsController),
    RFX_CONTROLLER_CREATION_ENTRY(RpLwxController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCdmaOnlyController),
    RFX_CONTROLLER_CREATION_ENTRY(RpSsController)
};

const RfxCreateControllerFuncptr RfxControllerFactory::s_non_slot_controllers[] = {
    RFX_CONTROLLER_CREATION_ENTRY(RpDataAllowController),
    RFX_CONTROLLER_CREATION_ENTRY(RpModemController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCapabilitySwitchController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCapabilityGetController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCallCenterController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCdmaLteModeController),
    RFX_CONTROLLER_CREATION_ENTRY(RpCdmaRadioController),
    RFX_CONTROLLER_CREATION_ENTRY(RpAgpsNSlotController),
    RFX_CONTROLLER_CREATION_ENTRY(RpRilClientController),
    RFX_CONTROLLER_CREATION_ENTRY(RpMalController),
    RFX_CONTROLLER_CREATION_ENTRY(RpSmsNSlotController)
};

void RfxControllerFactory::createControllers() {
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);

    RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->initSocketNotifyByFwk();

    RfxSlotRootController *non_slot_controller;

    // create a special slot root controller as non-slot based controllers parent
    RFX_OBJ_CREATE_EX(non_slot_controller, RfxSlotRootController, root, (RFX_SLOT_ID_UNKNOWN));
    root->setSlotRootController(RFX_SLOT_ID_UNKNOWN, non_slot_controller);

    // create some non-slot based controllers firstly
    createControllerInternal(s_pre_non_slot_controllers,
        sizeof(s_pre_non_slot_controllers)/sizeof(RfxCreateControllerFuncptr),
        non_slot_controller);

    RfxSlotRootController *slot_controller;
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        RFX_OBJ_CREATE_EX(slot_controller, RfxSlotRootController, root, (i));
        root->setSlotRootController(i, slot_controller);
        createControllerInternal(s_slot_controllers,
            sizeof(s_slot_controllers)/sizeof(RfxCreateControllerFuncptr),
            slot_controller);
    }

    createControllerInternal(s_non_slot_controllers,
        sizeof(s_non_slot_controllers)/sizeof(RfxCreateControllerFuncptr),
        non_slot_controller);
}

void RfxControllerFactory::createControllerInternal(
    const RfxCreateControllerFuncptr* controller_list,
    int length, RfxObject *parent) {
    for (int i = 0; i < length; i++) {
        if (controller_list[i] != NULL) {
            controller_list[i](parent);
        }
    }
}


