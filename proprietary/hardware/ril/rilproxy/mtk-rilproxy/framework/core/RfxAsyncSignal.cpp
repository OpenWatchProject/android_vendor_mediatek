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
 * File name:  rfx_async_signal.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * Implementation of async signal queue
 */

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxBasics.h"
#include "RfxAsyncSignal.h"

/*****************************************************************************
 * Define
 *****************************************************************************/

#define RFX_LOG_TAG "SlotQueueEntry"

#define RFX_ASYNC_SIGNAL_EMIT_LOOP_THRESHOLD    1000
#define RFX_ASYNC_SIGNAL_EMIT_BURST_THRESHOLD   3000

/*****************************************************************************
 * Class RfxAsyncSignalQueue
 *****************************************************************************/

RFX_IMPLEMENT_CLASS(RFX_ASYNC_SIGNAL_QUEUE_CLASS_NAME, RfxAsyncSignalQueue, RfxObject);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxAsyncSignalQueue);

void RfxAsyncSignalQueue::putSlot(RfxObject *obj, RfxAsyncSlotObjMemFuncPtrEx memFunc, void *data) {
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(memFunc != NULL);

    // Cerate a new list entry
    SlotQueueEntry *entry = new SlotQueueEntry(obj, memFunc, data);

    // Add the slot to the tail of list
    if (m_list_tail != NULL) {
        m_list_tail->m_next = entry;
    }
    m_list_tail = entry;

    // Add to the head if the list is empty
    if (m_list_head == NULL) {
        m_list_head = entry;
    }
}

void RfxAsyncSignalQueue::clear() {
    SlotQueueEntry *i = m_list_head;
    while (i != NULL) {
        SlotQueueEntry *next = i->m_next;

        delete(i);

        i = next;
    }
    m_list_head = NULL;
}

void RfxAsyncSignalQueue::processEmit() {
    int loopCountDown = RFX_ASYNC_SIGNAL_EMIT_LOOP_THRESHOLD;
    do {
        // NOTE:
        //  Assert here if it have infinit loop with post emit.
        //  For example, emit a post in callback ltself
        RFX_ASSERT(loopCountDown != 0);
        loopCountDown--;

        int burstCountDown = RFX_ASYNC_SIGNAL_EMIT_BURST_THRESHOLD;
        while (1) {
            // NOTE:
            //  Assert here if there are burst post emits
            //  For example, emit so many post in one loop
            RFX_ASSERT(burstCountDown > 0);
            burstCountDown--;
            SlotQueueEntry *entry = getSlot();
            if (entry == NULL) {
                // No more entry in the queue
                break;
            }

            // The target is invalid, ignore it
            RfxObject *obj = entry->m_target_ptr.promote().get();
            if (obj == NULL) {
                delete(entry);
                continue;
            }

            RFX_OBJ_ASSERT_VALID(obj);

            RfxAsyncSlotObjMemFuncPtrEx callback = entry->m_callback;
            (obj->*callback)(entry->m_data);

            delete(entry);
        } // while (1)
    } while (!isEmpty());

    //VFX_LOG(VFX_INFO3, VFX_MMI_CHECK_UPDATE_END);
}

RfxAsyncSignalQueue::SlotQueueEntry *RfxAsyncSignalQueue::getSlot() {
    SlotQueueEntry *first_entry = m_list_head;
    if (first_entry != NULL) {
        m_list_head = first_entry->m_next;

        if (m_list_head == NULL) {
            // If the queue becomes empty, pointer the last entry to NULL
            m_list_tail = NULL;
        }
    }

    return first_entry;
}

