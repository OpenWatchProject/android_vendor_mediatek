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
 * File name:  rfx_root_controller.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Implementation of root controller class.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxRootController.h"
#include "RfxMainThread.h"
#include "RfxLog.h"
#include <libmtkrilutils.h>

#define RFX_LOG_TAG "RfxRoot"

namespace android {
extern RIL_RadioFunctions s_callbacks;
}

/*****************************************************************************
 * Class RfxRootController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RfxRootController", RfxRootController, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxRootController);

bool RfxRootController::processMessage(const sp<RfxMessage>& message) {
    RfxController *handler = NULL;

    if (message->getId() == RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL
            && (message->getType() == REQUEST || message->getType() == RESPONSE)) {
        bool ret = false;
        ret = processAtciRequest(message);
        if (message->getType() == RESPONSE) {
            handleSendResponseAck(message);
        }
        return ret;
    }

    //RFX_LOG_D(RFX_LOG_TAG, "processMessage() begin");
    // check c2k slot
    if ((message->getSource() == RADIO_TECH_GROUP_C2K) &&
            (message->getType() == URC)) {
        // get c2k slot id from svlte mode controller from constructor
        message->setSlotId(getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0));
    }

    if (message->getType() == REQUEST) {
        handler = findMsgHandler(message->getId(), message->getSlotId(), m_request_list);
    } else if (message->getType() == URC) {
        handler = findMsgHandler(message->getId(), message->getSlotId(), m_urc_list);
    } else if (message->getType() == RESPONSE || message->getType() == REQUEST_ACK) {
        handler = findMsgHandler(message->getId(), message->getSlotId(), m_request_list);
    } else if (message->getType() == STATUS_SYNC) {
        RFX_LOG_D(RFX_LOG_TAG, "Update status key");
        getStatusManager(message->getSlotId())->setValue(message->getStatusKey(),
                message->getStatusValue(),message->getForceNotify(), message->getIsDefault());
        return true;
    }

    bool ret = false;

    if (handler == NULL) {
        //RFX_LOG_D(RFX_LOG_TAG, "processMessage() handler = NULL");
        if (REQUEST == message->getType()) {
            RFX_LOG_D(RFX_LOG_TAG, "processMessage() msg = [%s], handler = NULL ==> request",
                    message->toString().string());
            requestToRild(message);
            ret = true;
        } else if (REQUEST_ACK == message->getType()) {
            RFX_LOG_D(RFX_LOG_TAG, "processMessage() msg = [%s], handler = NULL ==> req_ack",
                                message->toString().string());
            requestAckToRilj(message);
            ret = true;
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "processMessage() msg = [%s], handler = NULL ==> resp/urc",
                    message->toString().string());
            ret = responseToRilj(message);
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "processMessage() msg = [%s], handler = %p, %s",
                message->toString().string(), handler, handler->getClassInfo()->getClassName());
        ret = handler->processMessage(message);
    }

    if (message->getType() == RESPONSE || message->getType() == URC) {
        handleSendResponseAck(message);
    }

    //RFX_LOG_D(RFX_LOG_TAG, "processMessage() end");
    return ret;
}

bool RfxRootController::processAtciRequest(const sp<RfxMessage>& message) {
    RfxController *handler = NULL;
    bool handled = false;

    RFX_LOG_D(RFX_LOG_TAG, "processAtciRequest() begin");
    RFX_LOG_D(RFX_LOG_TAG, "msg = [%s]", message->toString().string());

    // ATCI need the request sender to handle the corresponding response
    if (message->getType() == RESPONSE) {
        int id = message->getId();
        int token = message->getToken();
        int slot = message->getSlotId();
        handler = findMsgHandler(id, slot, m_response_list, DEFAULT, token);
        if (handler != NULL) {
            RFX_LOG_D(RFX_LOG_TAG, "processAtciRequest() find response handler = %p, %s",
                            handler, handler->getClassInfo()->getClassName());
            handler->processAtciMessage(message);
            // response for a special token found, remove the register info
            unregisterInternal(m_response_list, slot, &id, 1, DEFAULT, token);
            handled = true;
        }
    }

    if (handled == false) {
        for (int i = HIGHEST; i >= NORMAL; i--) {
            handler = findMsgHandler(message->getId(), message->getSlotId(), m_request_list, (HANDLER_PRIORITY)i);
            if (handler != NULL) {
                RFX_LOG_D(RFX_LOG_TAG, "processAtciRequest() handler = %p, %s priority = %d",
                                    handler, handler->getClassInfo()->getClassName(), i);
                message->resetParcelDataStartPos();
                if (handler->processAtciMessage(message)) {
                    handled = true;
                    break;
                }
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "processAtciRequest() handler = NULL priority = %d", i);
            }
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "processAtciRequest() end handled = %s", handled ? "true" : "false");
    return handled;
}

void RfxRootController::suspendMessage(RfxController *controller, const sp<RfxMessage>& message) {
    // the messages are recorded in the list in the sequence of
    // they are enqueued into main looper
    RFX_LOG_D(RFX_LOG_TAG, "suspendMessage() controller = %p, %s",
                           controller, controller->getClassInfo()->getClassName());
    m_suspended_msg_list.add(RfxSuspendedMsgEntry(controller, message));
}

void RfxRootController::clearMessages() {
    size_t size = m_suspended_msg_list.size();
    for (size_t i = 0; i < size; i++) {
        const RfxSuspendedMsgEntry& msg_entry = m_suspended_msg_list.itemAt(i);
        // enqueue back to main queue, then it will be ignored
        RfxMainThread::enqueueMessageFront(msg_entry.message);
    }
    m_suspended_msg_list.clear();
    RfxController::clearMessages();
}

void RfxRootController::processSuspendedMessage() {
    size_t i = 0;
    size_t size = m_suspended_msg_list.size();
    // traverse the suspended msg list to requeue the
    // message which should be resumed to the main looper
    while(size > 0 && i < size) {
        const RfxSuspendedMsgEntry& msg_entry = m_suspended_msg_list.itemAt(i);
        RfxController *controller = msg_entry.controller.promote().get();

        if (controller != NULL &&
            controller->checkIfRemoveSuspendedMessage(msg_entry.message)) {
            // just remove the msg if controller need
            m_suspended_msg_list.removeAt(i);
            size = m_suspended_msg_list.size();
        } else if (controller == NULL ||
                    (controller != NULL &&
                     controller->checkIfResumeMessage(msg_entry.message))) {
            // enqueue the msg in the front of main looper
            // so it will be processed firstly
            RfxMainThread::enqueueMessageFront(msg_entry.message);
            m_suspended_msg_list.removeAt(i);
            size = m_suspended_msg_list.size();
        } else {
            i++;
        }
    }
}


void RfxRootController::registerToHandleRequest(RfxController *controller, int slot_id,
        const int *request_id_list, size_t length, HANDLER_PRIORITY priority) {
    registerInternal(m_request_list, controller, slot_id, request_id_list, length, priority);
}

void RfxRootController::registerToHandleUrc(RfxController *controller, int slot_id,
        const int *urc_id_list, size_t length) {
    registerInternal(m_urc_list, controller, slot_id, urc_id_list, length);
}

void RfxRootController::registerToHandleResponse(RfxController *controller, int slot_id,
        const int *response_id_list, size_t length, int token) {
    registerInternal(m_response_list, controller, slot_id, response_id_list, length, DEFAULT, token);
}

void RfxRootController::unregisterToHandleRequest(int slot_id,
        const int *request_id_list, size_t length, HANDLER_PRIORITY priority) {
    unregisterInternal(m_request_list, slot_id, request_id_list, length, priority);
}

void RfxRootController::unregisterToHandleUrc(int slot_id,
        const int *urc_id_list, size_t length) {
    unregisterInternal(m_urc_list, slot_id, urc_id_list, length);
}


void RfxRootController::clearRegistry(RfxController *controller) {
    clearRegistryInternal(m_request_list, controller);
    clearRegistryInternal(m_response_list, controller);
    clearRegistryInternal(m_urc_list, controller);
}

void RfxRootController::clearRegistryInternal(SortedVector<RfxRegistryEntry>& list,
                            RfxController *controller) {
    if (controller == NULL) {
        return;
    }

    size_t i = 0;
    size_t size = list.size();

    while (size > 0 && i < size) {
        const RfxRegistryEntry& entry = list.itemAt(i);
        RfxController *reg_controller = entry.controller.promote().get();
        if (reg_controller == NULL || reg_controller == controller) {
            list.removeAt(i);
            size = list.size();
        } else {
            i++;
        }
    }
}

RfxStatusManager *RfxRootController::getStatusManager(int slot_id) const {
    RFX_ASSERT(slot_id == RFX_SLOT_ID_UNKNOWN || (slot_id >= 0 && slot_id <= RFX_SLOT_COUNT));
    if (slot_id == RFX_SLOT_ID_UNKNOWN) {
        return m_status_managers[RFX_SLOT_COUNT];
    } else {
        return m_status_managers[slot_id];
    }
}

RfxController *RfxRootController::getSlotRootController(int slot_id) const {
    RFX_ASSERT(slot_id == RFX_SLOT_ID_UNKNOWN || (slot_id >= 0 && slot_id <= RFX_SLOT_COUNT));
    if (slot_id == RFX_SLOT_ID_UNKNOWN) {
        return m_slot_root_controllers[RFX_SLOT_COUNT];
    } else {
        return m_slot_root_controllers[slot_id];
    }
}

void RfxRootController::setSlotRootController(int slot_id, RfxController *slot_root) {
    RFX_ASSERT(slot_id == RFX_SLOT_ID_UNKNOWN || (slot_id >= 0 && slot_id <= RFX_SLOT_COUNT));
    if (slot_id == RFX_SLOT_ID_UNKNOWN) {
        slot_id = RFX_SLOT_COUNT; // save non-slot based slot root in last index
    }
    m_slot_root_controllers[slot_id] = slot_root;
}

void RfxRootController::onInit() {
    RfxController::onInit();
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        RFX_OBJ_CREATE_EX(m_status_managers[i], RfxStatusManager, this, (i));
    }
    // create the status manager for non-slot controllers
    // slot id is RFX_SLOT_ID_UNKNOWN
    RFX_OBJ_CREATE_EX(m_status_managers[RFX_SLOT_COUNT], RfxStatusManager,
        this, (RFX_SLOT_ID_UNKNOWN));
}

void RfxRootController::registerInternal(SortedVector<RfxRegistryEntry>& list,
        RfxController *controller, int slot_id, const int *id_list,
        size_t length, HANDLER_PRIORITY priority, int token) {

    wp<RfxController> ptr;

    for (size_t i = 0; i < length; i++) {
        ptr = controller;
        RfxRegistryEntry entry(id_list[i], slot_id, ptr, priority, token);
        size_t old_size = list.size();
        // add the ids to the list
        list.add(entry);
        if (list.size() == old_size) {
            RfxController *dup_controller = findMsgHandler(id_list[i], slot_id, list, priority, token);
            if (controller == dup_controller) {
                continue; // the same registry, framework allows
            }
            RFX_LOG_E(RFX_LOG_TAG, "duplicated register controller = %p, %s, dup_controller = %p, %s, slot = %d id = %d priority = %d token = %d",
                           controller, controller->getClassInfo()->getClassName(),
                           dup_controller, dup_controller ? dup_controller->getClassInfo()->getClassName() : "",
                           slot_id, id_list[i], priority, token);
            // an id for one slot can only be registered once
            RFX_ASSERT(0);
        }
    }
}

void RfxRootController::unregisterInternal(SortedVector<RfxRegistryEntry>& list,
        int slot_id, const int *id_list, size_t length,
        HANDLER_PRIORITY priority, int token) {
    wp<RfxController> ptr;

    for (size_t i = 0; i < length && list.size() > 0; i++) {
        // construct a query entry, id & slot_id are comparision
        // parameters of RfxRegistryEntry, wp<RfxController> is not,
        // so just fill in a dummy ptr
        RfxRegistryEntry query_entry(id_list[i], slot_id, ptr, priority, token);

        ssize_t index = list.indexOf(query_entry);

        if (index >= 0) {
            list.removeAt(index);
        }
    }
}

RfxController *RfxRootController::findMsgHandler(int id, int slot_id,
                   SortedVector<RfxRegistryEntry>& list,
                   HANDLER_PRIORITY priority, int token) {
    wp<RfxController> ptr;
    // construct a query entry, id & slot_id are comparision
    // parameters of RfxRegistryEntry, wp<RfxController> is not,
    // so just fill in a dummy ptr
    RfxRegistryEntry query_entry(id, slot_id, ptr, priority, token);

    ssize_t index = list.indexOf(query_entry);

    if (index >= 0) {
        const RfxRegistryEntry &item = list.itemAt(index);
        return item.controller.promote().get();
    } else {
        return NULL;
    }
}

void RfxRootController::handleSendResponseAck(const sp<RfxMessage>& resp) {
    if (android::s_callbacks.version >= 13) {
        int needSend = 0;

        if (resp == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "handleSendResponseAck fail (resp NULL)");
            return;
        }
        if (resp->getType() == RESPONSE) {
            int type = RESPONSE_SOLICITED;
            Parcel* parcel = resp->getParcel();
            size_t pos = parcel->dataPosition();
            parcel->setDataPosition(0);
            parcel->readInt32(&type);
            if (type == RESPONSE_SOLICITED_ACK_EXP) {
                needSend = 1;
            }
            parcel->setDataPosition(pos);
        } else if (resp->getType() == URC) {
            needSend = 1;
        }

        if (needSend == 1) {
            sp<RfxMessage> respAck = RfxMessage::obtainRequest(resp->getSlotId(),
                    resp->getSource(), RIL_RESPONSE_ACKNOWLEDGEMENT);
            requestToRild(respAck);
            RFX_LOG_D(RFX_LOG_TAG, "send ResponseAck (slotId=%d, dest=%d, reqId=%d)",
                    respAck->getSlotId(), respAck->getDest(), respAck->getId());
        }
    }
}
