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
 * File name:  RfxRootController.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of root controller class which
 *  is the class of root object of the controller tree.
 */

#ifndef __RFX_ROOT_CONTROLLER_H__
#define __RFX_ROOT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "utils/SortedVector.h"
#include "utils/RefBase.h"
#include "utils/List.h"
#include "RfxController.h"
#include "RfxStatusManager.h"
#include "RfxRilAdapter.h"


using ::android::sp;
using ::android::wp;
using ::android::SortedVector;
using ::android::Vector;


/*****************************************************************************
 * Class RfxRegistryEntry
 *****************************************************************************/
class RfxRegistryEntry {

public:
    RfxRegistryEntry() :
        id(-1),
        slot_id(RFX_SLOT_ID_UNKNOWN),
        controller(NULL),
        priority(RfxController::HANDLER_PRIORITY::DEFAULT),
        msg_token(-1) {
    }

    RfxRegistryEntry(int _id, int _slot_id,
        wp<RfxController>& _controller,
        RfxController::HANDLER_PRIORITY _priority,
        int token) :
        id(_id),
        slot_id(_slot_id),
        controller(_controller),
        priority(_priority),
        msg_token(token) {
    }

    RfxRegistryEntry(const RfxRegistryEntry &other) :
        id(other.id),
        slot_id(other.slot_id),
        controller(other.controller),
        priority(other.priority),
        msg_token(other.msg_token) {
    }

    bool operator<(const RfxRegistryEntry& rhs) const {
        return (id < rhs.id) ||
               (id == rhs.id && slot_id < rhs.slot_id) ||
               (id == rhs.id && slot_id == rhs.slot_id && priority < rhs.priority) ||
               (id == rhs.id && slot_id == rhs.slot_id &&
                priority == rhs.priority && msg_token < rhs.msg_token);
    }

    bool operator>(const RfxRegistryEntry& rhs) const {
        return (id > rhs.id) ||
               (id == rhs.id && slot_id > rhs.slot_id) ||
               (id == rhs.id && slot_id == rhs.slot_id && priority > rhs.priority) ||
               (id == rhs.id && slot_id == rhs.slot_id &&
                priority == rhs.priority && msg_token > rhs.msg_token);
    }

    bool operator==(const RfxRegistryEntry& rhs) const {
        return id == rhs.id && slot_id == rhs.slot_id &&
               priority == rhs.priority && msg_token == rhs.msg_token;
    }

public:

    int id;
    int slot_id;
    wp<RfxController> controller;
    RfxController::HANDLER_PRIORITY priority;
    int msg_token;
};

/*****************************************************************************
 * Class RfxSuspendedMsgEntry
 *****************************************************************************/
class RfxSuspendedMsgEntry {
public:
    RfxSuspendedMsgEntry() :
        controller(NULL),
        message(NULL) {}

    RfxSuspendedMsgEntry(RfxController *handler,
        const sp<RfxMessage>& msg) :
        controller(handler),
        message(msg) {}

    RfxSuspendedMsgEntry(const RfxSuspendedMsgEntry& other) :
        controller(other.controller),
        message(other.message){}

    RfxSuspendedMsgEntry& operator=(const RfxSuspendedMsgEntry& other) {
        controller = other.controller;
        message = other.message;
        return *this;
    }

public:
    wp<RfxController> controller;
    sp<RfxMessage> message;
};

/***************************************************************************** 
 * Class RfxRootController
 *****************************************************************************/

class RfxRootController : public RfxController
{
    RFX_DECLARE_CLASS(RfxRootController);
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RfxRootController);

// Constructor / Destructor
public:
    // Constructor
    RfxRootController() {}

    virtual ~RfxRootController() {}

// Method
public:

    void processSuspendedMessage();

    virtual bool processMessage(const sp<RfxMessage>& message);

    virtual bool processAtciRequest(const sp<RfxMessage>& message);

    virtual void clearMessages();
// Overridable
public:

    void registerToHandleRequest(RfxController *controller, int slot_id,
        const int *request_id_list, size_t length, HANDLER_PRIORITY priority = DEFAULT);

    void registerToHandleResponse(RfxController *controller, int slot_id,
        const int *response_id_list, size_t length, int token = -1);

    void registerToHandleUrc(RfxController *controller, int slot_id,
        const int *urc_id_list, size_t length);

    void unregisterToHandleRequest(int slot_id, const int *request_id_list,
        size_t length, HANDLER_PRIORITY priority = DEFAULT);

    void unregisterToHandleUrc(int slot_id, const int *urc_id_list, size_t length);

    RfxStatusManager *getStatusManager(int slot_id) const;

    RfxController *getSlotRootController(int slot_id) const;

    void setSlotRootController(int slot_id, RfxController *slot_root);

    void suspendMessage(RfxController *controller, const sp<RfxMessage>& message);

    void clearRegistry(RfxController *controller);

protected:

    virtual void onInit();

private:

    SortedVector<RfxRegistryEntry> m_request_list;
    SortedVector<RfxRegistryEntry> m_response_list;
    SortedVector<RfxRegistryEntry> m_urc_list;
    Vector<RfxSuspendedMsgEntry> m_suspended_msg_list;
    RfxStatusManager *m_status_managers[MAX_RFX_SLOT_COUNT + 1]; // add one more status manage for no-slot controllers
    RfxController *m_slot_root_controllers[MAX_RFX_SLOT_COUNT + 1]; // add one more for append non-slot controllers

private:

    void registerInternal(SortedVector<RfxRegistryEntry>& list, RfxController *controller,
        int slot_id, const int *id_list, size_t length,
        HANDLER_PRIORITY priority = DEFAULT, int token = -1);

    void unregisterInternal(SortedVector<RfxRegistryEntry>& list, int slot_id,
        const int *id_list, size_t length,
        HANDLER_PRIORITY priority = DEFAULT, int token = -1);

    RfxController *findMsgHandler(int id, int slot_id, SortedVector<RfxRegistryEntry>& list,
        HANDLER_PRIORITY priority = DEFAULT, int token = -1);

    void clearRegistryInternal(SortedVector<RfxRegistryEntry>& list, RfxController *controller);

    void handleSendResponseAck(const sp<RfxMessage>& message);
};


#endif /* __RFX_ROOT_CONTROLLER_H__ */

