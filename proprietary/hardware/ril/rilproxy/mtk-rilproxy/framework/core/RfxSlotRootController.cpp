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
 * File name:  rfx_slot_root_controller.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Implementation of slot root controller class.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxSlotRootController.h"
#include "RfxRilAdapter.h"
#include "RfxLog.h"

/*****************************************************************************
 * Class RfxSlotRootController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RfxSlotRootController", RfxSlotRootController, RfxController);

RfxSlotRootController::RfxSlotRootController(int slot_id) :
    m_cs_requests(NULL),
    m_ps_requests(NULL) {
    m_slot_id = slot_id;
}

RfxSlotRootController::~RfxSlotRootController() {
    if (m_cs_requests) {
        delete(m_cs_requests);
    }

    if (m_ps_requests) {
        delete(m_ps_requests);
    }
}

void RfxSlotRootController::regReqToCsRild(const int *request_id_list, size_t length) {
    if (m_cs_requests == NULL) {
        m_cs_requests = new Vector<int>();
    }
    registerRequestInternal(request_id_list, length, m_cs_requests);
}

void RfxSlotRootController::regReqToPsRild(const int *request_id_list, size_t length) {
    if (m_ps_requests == NULL) {
        m_ps_requests = new Vector<int>();
    }
    registerRequestInternal(request_id_list, length, m_ps_requests);
}


void RfxSlotRootController::registerRequestInternal(const int *request_id_list, size_t length,
                                                    Vector<int> *list) {
    if (list == NULL) {
        RFX_ASSERT(0);
    }

    for (size_t i = 0; i < length; i++) {
        list->add(request_id_list[i]);
    }

    registerToHandleRequest(request_id_list, length);
}

void RfxSlotRootController::unregisterRequestInternal(const int *request_id_list, size_t length,
                                                      Vector<int> *list) {
    if (!list) {
        return;
    }
    size_t i = 0;
    while (i < list->size()) {
        int id = list->itemAt(i);
        bool found = false;
        for (size_t j = 0; j < length; j++) {
            if (id == request_id_list[j]) {
                list->removeAt(i);
                found = true;
                break;
            }
        }
        if (!found) {
            i++;
        }
    }

    unregisterToHandleRequest(request_id_list, length);
}


bool RfxSlotRootController::onHandleRequest(const sp<RfxMessage>& message) {
    int id = message->getId();

    if (m_cs_requests) {
        size_t size = m_cs_requests->size();
        for (size_t i = 0; i < size; i++) {
            if (m_cs_requests->itemAt(i) == id) {
                int cs_type = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_TYPE,
                                                              RADIO_TECH_GROUP_GSM);
                sp<RfxMessage> req_msg;
                if (cs_type == RADIO_TECH_GROUP_C2K) {
                    req_msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, id, message, true);
                } else if (cs_type == RADIO_TECH_GROUP_GSM) {
                    req_msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, id, message, true);
                } else {
                    RFX_ASSERT(0);
                }
                requestToRild(req_msg);
                return true;
            }
        }
    }

    if (m_ps_requests) {
        size_t size = m_ps_requests->size();
        for (size_t i = 0; i < size; i++) {
            if (m_ps_requests->itemAt(i) == id) {
                int ps_type = getStatusManager()->getIntValue(RFX_STATUS_KEY_DATA_TYPE,
                                                              RADIO_TECH_GROUP_GSM);
                sp<RfxMessage> req_msg;
                if (ps_type == RADIO_TECH_GROUP_C2K) {
                    req_msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, id, message, true);
                } else if (ps_type == RADIO_TECH_GROUP_GSM) {
                    req_msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, id, message, true);
                } else {
                    RFX_ASSERT(0);
                }
                requestToRild(req_msg);
                return true;
            }
        }
    }
    RFX_ASSERT(0); // the request not registered
    return false;
}

bool RfxSlotRootController::onHandleResponse(const sp<RfxMessage>& message) {
    int id = message->getId();
    if (m_cs_requests) {
        size_t size = m_cs_requests->size();
        for (size_t i = 0; i < size; i++) {
            if (m_cs_requests->itemAt(i) == id) {
                responseToRilj(message);
                return true;
            }
        }
    }
    if (m_ps_requests) {
        size_t size = m_ps_requests->size();
        for (size_t i = 0; i < size; i++) {
            if (m_ps_requests->itemAt(i) == id) {
                responseToRilj(message);
                return true;
            }
        }
    }
    RFX_ASSERT(0); // the response not registered
    return false;
}

