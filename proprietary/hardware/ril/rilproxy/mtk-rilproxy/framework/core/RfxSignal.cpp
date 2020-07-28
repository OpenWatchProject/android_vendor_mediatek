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
 * File name:  rfx_signal.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Event in RIL Proxy Framework is used like a type-safe function pointer.
 *  But you can connect one or more functor to one signal.
 *  Connect a functor into a signal, it will create a signal handler for this
 *  functor and insert it into the list of signal.
 *
 *  When you emit an signal, all connected functors will be called one by one.
 *
 *  A functor may terminate the call sequence by call rfxStopEmit().
 */


/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxSignal.h"

#include "RfxAsyncSignal.h"

#include "RfxObject.h"
#include "RfxClassInfo.h"


/****************************************************************************
 * Class RfxEmitFrame
 *****************************************************************************/

/*
 *  RfxEmitFrame is a emit stack object. The current running emit loop in stack
 *   will be handled by this object.
 */
class RfxEmitFrame {
    static RfxEmitFrame *s_emit_frame_list;
public:
    // Constructor / Distructor
    RfxEmitFrame() : stop(false), prev(NULL) {
    }

// Method
public:
    // Enter this emit stack frame
    void enter() {
        prev = s_emit_frame_list;
        s_emit_frame_list = this;
    }

    // Leave this emit stack frame
    void leave() {
        RFX_ASSERT(s_emit_frame_list == this);
        s_emit_frame_list = prev;
    }

    // Stop emit loop
    static void stopEmit() {
        RFX_ASSERT(s_emit_frame_list);
        s_emit_frame_list->stop = true;
    }

// Variable
public:
    // whether this emit loop should be stop
    bool         stop;
    // The previous emit frame for pop
    RfxEmitFrame    *prev;
};


RfxEmitFrame* RfxEmitFrame::s_emit_frame_list = NULL;


void rfxStopEmit() {
    RfxEmitFrame::stopEmit();
}


/*****************************************************************************
 * Class RfxPostEmitData
 *****************************************************************************/

class RfxPostEmitData {
public:
    RfxPostEmitData() {}

    RfxSlotList                 slots;
    RfxEmitEntryFuncPtr         emit_funcptr;
    RfxSignalArg               *arguments;

    wp<RfxObject>               callback_obj;
    RfxSlotObjMemFuncPtr        callback_funcptr;
};


/*****************************************************************************
 * Class RfxAsyncSignalUtil
 *****************************************************************************/

RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxAsyncSignalUtil);

void RfxAsyncSignalUtil::regPostEmit(
    const RfxBaseSignal         *baseSignal,
    RfxEmitEntryFuncPtr          emit_funcptr,
    RfxSignalArg                *arguments,
    RfxObject                   *callback_obj,
    RfxSlotObjMemFuncPtr        callback_funcptr
)
{
    RfxPostEmitData* emitdata;
    emitdata = new RfxPostEmitData();

    emitdata->slots.assignWithList(baseSignal->m_slot_list);
    emitdata->emit_funcptr      = emit_funcptr;
    emitdata->arguments         = arguments;
    emitdata->callback_obj      = callback_obj;
    emitdata->callback_funcptr  = callback_funcptr;

    rfxPostInvoke(this, &RfxAsyncSignalUtil::processPostEmit, (void*)emitdata);
}


void RfxAsyncSignalUtil::processPostEmit(void *data)
{
    RfxPostEmitData* emitdata = (RfxPostEmitData*)data;

    // invoke signal
    RfxBaseSignal::emitSlots(
        &emitdata->slots,
        emitdata->emit_funcptr,
        emitdata->arguments
    );

    // TODO: how about callback_obj is dead
    // callback if assigned callback
    if (emitdata->callback_obj.promote().get() && emitdata->callback_funcptr)
    {
        (*emitdata->emit_funcptr)(
            emitdata->callback_obj.promote().get(),
            emitdata->callback_funcptr,
            emitdata->arguments);
    }

    // free the argument
    delete(emitdata->arguments);

    // free the post data
    delete(emitdata);
}


/*****************************************************************************
 * Class RfxSlotList
 *****************************************************************************/

RfxSlotList::SlotListEntry *RfxSlotList::findEntry(RfxObject *obj, RfxSlotObjMemFuncPtr memFunc) const
{
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(memFunc != NULL);

    RfxSlot slot_to_find(obj, memFunc);

    /* search if any handler is equal to the given handler */
    SlotListEntry *i;
    for (i = m_list_head; i != NULL; i = i->m_next)
    {
        if (i->m_slot.equalTo(slot_to_find))
        {
            return i;
        }
    }
    return NULL;
}


void RfxSlotList::pushSlot(RfxObject *obj, RfxSlotObjMemFuncPtr memFunc)
{
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(memFunc != NULL);

    removeAllInvalidateSlots();

    SlotListEntry *entry = findEntry(obj, memFunc);
    if (entry != NULL)
    {
        // Do nothing if exist
        return;
    }

    // Cerate a new list entry
    entry = new SlotListEntry(obj, memFunc);

    // Add this slot to the end of list
    if (m_list_head == NULL)
    {
        m_list_head = entry;
    }
    else
    {
        entry->m_next = m_list_head;
        m_list_head = entry;
    }
}


void RfxSlotList::appendSlot(RfxObject *obj, RfxSlotObjMemFuncPtr memFunc)
{
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(memFunc != NULL);

    removeAllInvalidateSlots();

    SlotListEntry *entry = findEntry(obj, memFunc);
    if (entry != NULL)
    {
        // Do nothing if exist
        return;
    }

    // Cerate a new list entry
    entry = new SlotListEntry(obj, memFunc);

    // Add this slot to the end of list
    if (m_list_head == NULL)
    {
        m_list_head = entry;
    }
    else
    {
        SlotListEntry *tmp = m_list_head;
        while (tmp->m_next != NULL)
        {
            tmp = tmp->m_next;
        }
        tmp->m_next = entry;
    }
}


void RfxSlotList::removeSlot(RfxObject *obj, RfxSlotObjMemFuncPtr memFunc)
{
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(memFunc != NULL);

    removeAllInvalidateSlots();

    // Find the slot and remove from the list
    RfxSlot slot_to_find(obj, memFunc);
    SlotListEntry **entryPtr = &m_list_head;
    while (*entryPtr != NULL)
    {
        SlotListEntry *entry = *entryPtr;
        if (entry->m_slot.equalTo(slot_to_find))
        {
            *entryPtr = entry->m_next;
            delete(entry);
            return;
        }
        entryPtr = &(entry->m_next);
    }
}


void RfxSlotList::clear()
{
    SlotListEntry *i = m_list_head;
    while (i != NULL)
    {
        SlotListEntry *next = i->m_next;
        delete(i);
        i = next;
    }
    m_list_head = NULL;
}


int RfxSlotList::getCount() const
{
    int cnt = 0;
    SlotListEntry *i = m_list_head;
    while (i != NULL)
    {
        cnt++;
        i = i->m_next;
    }
    return cnt;
}


void RfxSlotList::assignWithList(const RfxSlotList &other)
{
    // Must be empty list
    RFX_ASSERT(m_list_head == NULL);

    SlotListEntry **entryPtr = &m_list_head;
    SlotListEntry *i;
    for (i = other.m_list_head; i != NULL; i = i->m_next)
    {
        SlotListEntry *entry;
        entry = new SlotListEntry(i->m_slot.m_obj_ptr.promote().get(), i->m_slot.m_callback);
        *entryPtr = entry;
        entryPtr = &(entry->m_next);
    }
}


void RfxSlotList::removeAllInvalidateSlots()
{
    SlotListEntry **entryPtr = &m_list_head;
    while (*entryPtr != NULL)
    {
        SlotListEntry *entry = *entryPtr;
        if (!entry->m_slot.isValid())
        {
            *entryPtr = entry->m_next;
            delete(entry);
        }
        else
        {
            entryPtr = &(entry->m_next);
        }
    }
}


/*****************************************************************************
 * class RfxBaseSignal
 *****************************************************************************/

int RfxBaseSignal::getSlotCount() const
{
    return m_slot_list.getCount();
}


void RfxBaseSignal::emitImpl(RfxEmitEntryFuncPtr emit_funcptr, void *arguments) const
{
    RfxSlotList tmp_slot_list(m_slot_list);
    emitSlots(&tmp_slot_list, emit_funcptr, arguments);
}


void RfxBaseSignal::emitSlots(const RfxSlotList *slot_list, RfxEmitEntryFuncPtr emit_funcptr, void *arguments)
{
    RfxEmitFrame emitFrame;
    emitFrame.enter();

    // for all signal handler in callback list
    RfxSlotList::SlotListEntry *i = slot_list->getHead();
    for (; i != NULL; i = i->m_next)
    {
        if (!i->m_slot.isValid())
        {
            continue;
        }

        // Callback the object slot member method
        RfxObject *obj = i->m_slot.m_obj_ptr.promote().get();
        RfxSlotObjMemFuncPtr callback = (RfxSlotObjMemFuncPtr)i->m_slot.m_callback;

        // Callback by emit function
        (*emit_funcptr)(obj, callback, arguments);

        // break if handler is not return
        if (emitFrame.stop)
        {
            break;
        }
    }
    emitFrame.leave();
}


void RfxBaseSignal::baseConnectFront(RfxObject *obj, RfxSlotObjMemFuncPtr func)
{
    // Ignore NULL object pointer
    if (obj == NULL)
    {
        return;
    }
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(func != NULL);

    m_slot_list.pushSlot(obj, func);
}


void RfxBaseSignal::baseConnect(RfxObject *obj, RfxSlotObjMemFuncPtr func)
{
    // Ignore NULL object pointer
    if (obj == NULL)
    {
        return;
    }
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(func != NULL);

    m_slot_list.appendSlot(obj, func);
}


void RfxBaseSignal::baseDisconnect(RfxObject *obj, RfxSlotObjMemFuncPtr func)
{
    RFX_OBJ_ASSERT_VALID(obj);
    RFX_ASSERT(func != NULL);

    m_slot_list.removeSlot(obj, func);
}

