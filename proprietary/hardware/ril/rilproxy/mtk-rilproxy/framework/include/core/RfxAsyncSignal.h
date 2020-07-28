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
 * File name:  RfxAsyncSignal.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Header of async signal implementation
 */

#ifndef __RFX_ASYNC_SIGNAL_H__
#define __RFX_ASYNC_SIGNAL_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxLog.h"
#include "RfxObject.h"
#include "utils/RefBase.h"

using ::android::sp;
using ::android::wp;
using ::android::RefBase;

/*****************************************************************************
 * Define
 *****************************************************************************/

#define RFX_ASYNC_SIGNAL_QUEUE_CLASS_NAME "AsyncSignalQueue"

/*****************************************************************************
 * Class RfxAsyncSignalQueue
 *****************************************************************************/

// Event callback
typedef void (RfxObject::*RfxAsyncSlotObjMemFuncPtrEx)(void *data);
// RfxAsyncSignal0 callbak
typedef void (RfxObject::*RfxAsyncSlotObjMemFuncPtr)();

class RfxAsyncSignalQueue : public RfxObject {

    RFX_DECLARE_CLASS(RfxAsyncSignalQueue);
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RfxAsyncSignalQueue);

// Constructor / Destructor
public:
    // Default constructor
    RfxAsyncSignalQueue() : m_list_head(NULL), m_list_tail(NULL) {
    }

    // Destructor
    virtual ~RfxAsyncSignalQueue() {
        clear();
    }

// Methods
public:
    // Test if the slot list is empty
    bool isEmpty() const {
        return m_list_head == NULL;
    }

    // Add a slot into the tail of slot queue
    void putSlot(RfxObject *obj, RfxAsyncSlotObjMemFuncPtrEx memFunc, void *data);

    // Add a slot into the tail of slot queue (without event)
    void putSlot0(RfxObject *obj, RfxAsyncSlotObjMemFuncPtr memFunc) {
        putSlot(obj, (RfxAsyncSlotObjMemFuncPtrEx)memFunc, NULL);
    }

    // Clear all slots in the slot list
    void clear();

// Framewrok methods
public:
    void processEmit();

// Implementation
private:
    class SlotQueueEntry {

    public:
        SlotQueueEntry(RfxObject *obj, RfxAsyncSlotObjMemFuncPtrEx memFunc, void *data) :
                m_target_ptr(obj), m_callback(memFunc), m_data(data), m_next(NULL) {
        }

    public:
        wp<RfxObject>               m_target_ptr;
        RfxAsyncSlotObjMemFuncPtrEx m_callback;
        void                        *m_data;
        SlotQueueEntry              *m_next;
    };

    SlotQueueEntry *m_list_head; // Pointer to the first slot entry of queue
    SlotQueueEntry *m_list_tail; // Pointer to the last slot entry of queue

    // Get the first slot entry of the queue, and remove it from queue.
    // RETURNS: The first of slot entry of the queue.
    //          Return NULL if the queue is empty.
    SlotQueueEntry *getSlot();
};

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

// A helper invoke post without type safe.
// This function is for internal use by framework.
template <class _MemFunc>
inline
void rfxPostInvoke(RfxObject *obj, _MemFunc func, void *data = NULL) {
    RfxAsyncSignalQueue *queue = RFX_OBJ_GET_INSTANCE(RfxAsyncSignalQueue);
    queue->putSlot(obj, static_cast<RfxAsyncSlotObjMemFuncPtrEx>(func), data);
}

#endif /* __RFX_ASYNC_SIGNAL_H__ */

