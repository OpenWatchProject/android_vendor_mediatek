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
 * File name:  RfxSignal.h
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

#ifndef __RFX_SIGNAL_H__
#define __RFX_SIGNAL_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <utils/RefBase.h>
#include "RfxAsyncSignal.h"

using ::android::sp;
using ::android::wp;
using ::android::RefBase;

/*****************************************************************************
 * Typedef
 *****************************************************************************/

// Predefined class
class RfxObject;
class RfxBaseSignal;

// Internal used.
typedef void (RfxObject::*RfxSlotObjMemFuncPtr)();
// Internal used.
typedef void (*RfxEmitEntryFuncPtr)(RfxObject *obj,
                                       RfxSlotObjMemFuncPtr callback,
                                       void *arguments);

/*****************************************************************************
 * Define
 *****************************************************************************/

// Null callback object (RfxNullCallback)
#define RFX_CALLBACK_NULL               (RfxNullCallback())

// Internal used. SlotListEntry magic number for a live instance.
#define RFX_SLOT_LIVE_MAGIC_NUMBER 'TSLS'
// Internal used. SlotListEntry magic number for a dead instance.
#define RFX_SLOT_DEAD_MAGIC_NUMBER 'tsls'

// Utility Macro for helping register post emit function
#define RFX_REG_POSTEMIT(_class, _arg, _cb_obj, _cb_func) do { \
    RfxAsyncSignalUtil *async_signal = RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil); \
    async_signal->regPostEmit( \
        this, \
        (RfxEmitEntryFuncPtr)&_class::emitEntry, \
        _arg, \
        _cb_obj, \
        (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(_cb_func)); \
} while(0)

/*****************************************************************************
 * Class RfxSlot
 *****************************************************************************/
// Internal used. RfxSlot is a base class which stores weak reference of
//RfxObject and it's member function.
class RfxSlot {
// Variable
public:
    wp<RfxObject> m_obj_ptr;          // weak reference to RfxObject object
    RfxSlotObjMemFuncPtr m_callback;    // RfxObject object's member funtion pointer

// Constructor / Destructor
public:
    // Construct with object and it's member function
    RfxSlot(RfxObject *obj, RfxSlotObjMemFuncPtr callCallback) :
        m_obj_ptr(obj),
        m_callback(callCallback)
    {
    }

    // Copy constructor
    RfxSlot(const RfxSlot &other) :
        m_obj_ptr(other.m_obj_ptr),
        m_callback(other.m_callback)
    {
    }

// Method
public:
    // Whether target object is valid.
    bool isValid() const {
        return (m_obj_ptr.promote().get() != NULL);
    }

    // Compare with another slot.
    bool equalTo(const RfxSlot &other) const {
        return (m_obj_ptr == other.m_obj_ptr) && (m_callback == other.m_callback);
    }

// Operator
public:
    RfxSlot &operator =(const RfxSlot &other) {
        m_obj_ptr = other.m_obj_ptr;
        m_callback = other.m_callback;
        return *this;
    }

    bool operator == (const RfxSlot &other) {
        return equalTo(other);
    }

    bool operator != (const RfxSlot &other) {
        return !equalTo(other);
    }
};

/*****************************************************************************
 * Class RfxSlotList
 *****************************************************************************/

// Internal used. RfxSlotList is a list container which stores slot objects.
class RfxSlotList {
// Define
public:

    // Internal used. SlotListEntry is an internal list node of RfxSlotList.
    class SlotListEntry {
    public:
        // Consturct with object and it's member function.
        SlotListEntry(RfxObject *obj, RfxSlotObjMemFuncPtr memFunc) :
            m_magic_number(RFX_SLOT_LIVE_MAGIC_NUMBER),
            m_slot(obj, memFunc),
            m_next(NULL) {
        }
        ~SlotListEntry() {
            m_magic_number = RFX_SLOT_DEAD_MAGIC_NUMBER;
        }

    private:
        int m_magic_number;  //for rfx system memory debug
    public:
        RfxSlot         m_slot;     // slot
        SlotListEntry  *m_next;     // next node
    };

// Constructor / Destructor
public:
    // Default constructor
    RfxSlotList() : m_list_head(NULL) {
    }

    // Cioy constructor
    RfxSlotList(const RfxSlotList &other) : m_list_head(NULL) {
        assignWithList(other);
    }

    // Destructor
    ~RfxSlotList() {
        clear();
    }

// Methods
public:
    // Return the head of the slot list
    // RETURNS: The head slot entry of the list. It the slot list is empty,
    //          it will return NULL.
    SlotListEntry *getHead() const {
        return m_list_head;
    }

    // Test if the slot list is empty
    bool isEmpty() const {
        return m_list_head == NULL;
    }

    // Find a slot in the slot list
    // RETURNS: The slot entry to find. If the slot is not in the slot list,
    //          it will return NULL.
    SlotListEntry *findEntry(
        RfxObject *obj,
        RfxSlotObjMemFuncPtr memFunc
    ) const;

    // Add the slot into the head ot slot list
    void pushSlot(
        RfxObject *obj,
        RfxSlotObjMemFuncPtr memFunc
    );

    // Append the slot into the tail ot slot list
    void appendSlot(
        RfxObject *obj,
        RfxSlotObjMemFuncPtr memFunc
    );

    // Remove the given slot from the slot list
    void removeSlot(
        RfxObject *obj,
        RfxSlotObjMemFuncPtr memFunc
    );

    // Clear all slots in the slot list
    void clear();

    // Get number of slot count
    // RETURNS: return number of slot
    int getCount() const;

// Implementation
private:
    SlotListEntry *m_list_head;

    void assignWithList(const RfxSlotList &other);
    void removeAllInvalidateSlots();

    friend class RfxAsyncSignalUtil;
};

/*****************************************************************************
 * Class RfxSignalArg
 *****************************************************************************/

/*
 *  RfxSignalArg is base class that hold arguments for post emit.
 */
class RfxSignalArg {
public:
    RfxSignalArg() {
    }

    // Default virtual destructor,
    //  for RfxSignalArgX member data destructors
    virtual ~RfxSignalArg() {
    }
};

/*****************************************************************************
 * Class RfxSignalArg1
 *****************************************************************************/

// RfxSignalArg1 is a template class which holds one argument for RfxSignal1
template <class _Arg1>
class RfxSignalArg1 : public RfxSignalArg {
public:
    _Arg1 arg1;    // signal argument 1

    // Construct with argument
    RfxSignalArg1(_Arg1 _arg1) : arg1(_arg1) {
    }
};

/*****************************************************************************
 * Class RfxSignalArg2
 *****************************************************************************/

// RfxSignalArg2 is a template class which holds two arguments for RfxSignal2
template <class _Arg1, class _Arg2>
class RfxSignalArg2 : public RfxSignalArg {
public:
    _Arg1 arg1;    // signal argument 1
    _Arg2 arg2;    // signal argument 2

    // Construct with arguments
    RfxSignalArg2(_Arg1 _arg1, _Arg2 _arg2) : arg1(_arg1), arg2(_arg2) {
    }
};

/*****************************************************************************
 * Class RfxSignalArg3
 *****************************************************************************/

// RfxSignalArg3 is a template class which holds three arguments for RfxSignal3
template <class _Arg1, class _Arg2, class _Arg3>
class RfxSignalArg3 : public RfxSignalArg {
public:
    _Arg1 arg1;    // signal argument 1
    _Arg2 arg2;    // signal argument 2
    _Arg3 arg3;    // signal argument 3

    // Construct with arguments
    RfxSignalArg3(_Arg1 _arg1, _Arg2 _arg2, _Arg3 _arg3) : arg1(_arg1), arg2(_arg2), arg3(_arg3) {
    }
};

/*****************************************************************************
 * Class RfxSignalArg4
 *****************************************************************************/

// RfxSignalArg4 is a template class which holds four arguments for RfxSignal4
template <class _Arg1, class _Arg2, class _Arg3, class _Arg4>
class RfxSignalArg4 : public RfxSignalArg {
public:
    _Arg1 arg1;    // signal argument 1
    _Arg2 arg2;    // signal argument 2
    _Arg3 arg3;    // signal argument 3
    _Arg4 arg4;    // signal argument 4

    // Construct with arguments
    RfxSignalArg4(_Arg1 _arg1, _Arg2 _arg2, _Arg3 _arg3, _Arg4 _arg4) :
        arg1(_arg1), arg2(_arg2), arg3(_arg3), arg4(_arg4) {
    }
};

/*****************************************************************************
 * Class RfxCallback
 *****************************************************************************/

// This is an empty class just for have a type to notify the callback to reset its
// content.
// SEE ALSO: RFX_CALLBACK_NULL
class RfxNullCallback {
};

/*****************************************************************************
 * Class RfxCallback0
 *****************************************************************************/

// RfxCallback0 is a callback helper with no function arguments.
// NOTE: the callback object should derived from RfxObject.
//
// EXAMPLE:
// <code>
// // Example 1
// RfxCallback0 callback(this, &RfxControl::onUpdate);
// callback.invoke();
//
// // Example 2
// class RfxMyController : RfxController
// {
//     ......
//     void RfxMyController::onCallback()
//     {
//     }
//
//     void RfxMyController::registerCallback()
//     {
//         RfxCallback0 cb(this, &RfxMyControl::onCallback);
//         RfxNotifier *notifier = RFX_OBJ_GET_INSTANCE(RfxNotifier);
//         notifier->register(cb);
//     }
// };
//
// class RfxNotifier : public RfxObject
// {
//     RFX_DECLARE_CLASS(RfxNotifier);
//     RFX_OBJ_DECLARE_SINGLETON_CLASS(RfxNotifier);
//
// public:
//     RfxCallback0     m_callback;
//
//     void register(const RfxCallback0 &callback)
//     {
//         m_callback = callback;
//     }
//
//     void notify()
//     {
//         m_callback.invoke();
//     }
// };
// </code>
class RfxCallback0 : public RfxSlot {
private:
    typedef void (RfxObject::*MemFunc)();

// Constructor / Distructor
public:
    // Default constructor
    RfxCallback0() : RfxSlot(NULL, NULL) {
    }

    // Constructor
    template <class _func_type>
    RfxCallback0(RfxObject *obj, _func_type func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<MemFunc>(func)) {
    }

    // Copy constructor
    RfxCallback0(const RfxCallback0 &other) :
        RfxSlot(other.m_obj_ptr.promote().get(), other.m_callback) {
    }

    // Helper constructor to init a NULL callback
    RfxCallback0(const RfxNullCallback &nullCallback) : RfxSlot(NULL, NULL) {
        RFX_UNUSED(nullCallback);
    }

    // Invoke registered callback member function
    void invoke() {
        if (isValid()) {
            (m_obj_ptr.promote().get()->*(MemFunc)m_callback)();
        }
    }
};

/*****************************************************************************
 * Class RfxCallback1
 *****************************************************************************/

// RfxCallback1 is a callback helper with 1 function argument
template <typename _Arg1>
class RfxCallback1 : public RfxSlot {
private:
    typedef void (RfxObject::*MemFunc)(_Arg1 arg1);

// Constructor / Distructor
public:
    // Default constructor
    RfxCallback1() : RfxSlot(NULL, NULL) {
    }

    // Constructor
    template <typename _func_type>
    RfxCallback1(RfxObject *obj, _func_type func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<MemFunc>(func)) {
    }

    // Copy constructor
    RfxCallback1(const RfxCallback1 &other) :
        RfxSlot(other.m_obj_ptr.promote().get(), other.m_callback) {
    }

    // Helper constructor to init a NULL callback
    RfxCallback1(const RfxNullCallback &nullCallback) : RfxSlot(NULL, NULL) {
        RFX_UNUSED(nullCallback);
    }

    // Invoke registered callback member function
    void invoke(_Arg1 arg1) {
        if (isValid()) {
            (m_obj_ptr.promote().get()->*(MemFunc)m_callback)(arg1);
        }
    }
};

/*****************************************************************************
 * Class RfxCallback2
 *****************************************************************************/

// RfxCallback2 is a callback helper with 2 function arguments
template <typename _Arg1, typename _Arg2>
class RfxCallback2 : public RfxSlot {
private:
    typedef void (RfxObject::*MemFunc)(_Arg1 arg1, _Arg2 arg2);

// Constructor / Distructor
public:
    // Default constructor
    RfxCallback2() : RfxSlot(NULL, NULL) {
    }

    // Constructor
    template <typename _func_type>
    RfxCallback2(RfxObject *obj, _func_type func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<MemFunc>(func)) {
    }

    // Copy constructor
    RfxCallback2(const RfxCallback2 &other) :
        RfxSlot(other.m_obj_ptr.promote().get(), other.m_callback) {
    }

    // Helper constructor to init a NULL callback
    RfxCallback2(const RfxNullCallback &nullCallback) : RfxSlot(NULL, NULL) {
        RFX_UNUSED(nullCallback);
    }

    // Invoke registered callback member function
    void invoke(_Arg1 arg1, _Arg2 arg2) {
        if (isValid()) {
            (m_obj_ptr.promote().get()->*(MemFunc)m_callback)(arg1, arg2);
        }
    }
};

/*****************************************************************************
 * Class RfxCallback3
 *****************************************************************************/

// RfxCallback3 is a callback helper with 3 function arguments
template <typename _Arg1, typename _Arg2, typename _Arg3>
class RfxCallback3 : public RfxSlot {
private:
    typedef void (RfxObject::*MemFunc)(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3);

// Constructor / Distructor
public:
    // Default constructor
    RfxCallback3() : RfxSlot(NULL, NULL) {
    }

    // Constructor
    template <typename _func_type>
    RfxCallback3(RfxObject *obj, _func_type func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<MemFunc>(func)) {
    }

    // Copy constructor
    RfxCallback3(const RfxCallback3 &other) :
        RfxSlot(other.m_obj_ptr.promote().get(), other.m_callback) {
    }

    // Helper constructor to init a NULL callback
    RfxCallback3(const RfxNullCallback &nullCallback) : RfxSlot(NULL, NULL) {
        RFX_UNUSED(nullCallback);
    }

    // Invoke registered callback member function
    void invoke(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3) {
        if (isValid()) {
            (m_obj_ptr.promote().get()->*(MemFunc)m_callback)(arg1, arg2, arg3);
        }
    }
};

/*****************************************************************************
 * Class RfxCallback4
 *****************************************************************************/

// RfxCallback4 is a callback  helper with 4 function arguments
template <typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4>
class RfxCallback4 : public RfxSlot {
private:
    typedef void (RfxObject::*MemFunc)(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3, _Arg4 arg4);

// Constructor / Distructor
public:
    // Default constructor
    RfxCallback4() : RfxSlot(NULL, NULL) {
    }

    // Constructor
    template <typename _func_type>
    RfxCallback4(RfxObject *obj, _func_type func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<MemFunc>(func)) {
    }

    // Copy constructor
    RfxCallback4(const RfxCallback4 &other) :
        RfxSlot(other.m_obj_ptr.promote().get(), other.m_callback) {
    }

    // Helper constructor to init a NULL callback
    RfxCallback4(const RfxNullCallback &nullCallback) : RfxSlot(NULL, NULL) {
        RFX_UNUSED(nullCallback);
    }

    // Invoke registered callback member function
    void invoke(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3, _Arg4 arg4) {
        if (isValid()) {
            (m_obj_ptr.promote().get()->*(MemFunc)m_callback)(arg1, arg2, arg3, arg4);
        }
    }
};

/*****************************************************************************
 * Class RfxPostInvoke0
 *****************************************************************************/

// Internal used.
class RfxPostInvoke0 : public RfxSlot {
public:
    typedef void (RfxObject::*ObjMemFunc)();

public:

    template <class _MemFunc>
    RfxPostInvoke0(RfxObject *obj, _MemFunc func) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<ObjMemFunc>(func)) {
    }
};

/*****************************************************************************
 * Class RfxPostInvoke1
 *****************************************************************************/

// Internal used.
template <class _Arg1>
class RfxPostInvoke1 : public RfxSlot {
public:
    typedef void (RfxObject::*ObjMemFunc)(_Arg1 arg1);

public:
    RfxSignalArg1 <_Arg1> m_args;

public:

    template <class _MemFunc>
    RfxPostInvoke1(RfxObject *obj, _MemFunc func, _Arg1 a1) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<ObjMemFunc>(func)), m_args(a1) {
    }
};

/*****************************************************************************
 * Class RfxPostInvoke2
 *****************************************************************************/

// Internal used.
template <class _Arg1, class _Arg2>
class RfxPostInvoke2 : public RfxSlot {
public:
    typedef void (RfxObject::*ObjMemFunc)(_Arg1 arg1, _Arg2 arg2);

public:
    RfxSignalArg2 <_Arg1, _Arg2> m_args;

public:

    template <class _MemFunc>
    RfxPostInvoke2(RfxObject *obj, _MemFunc func, _Arg1 a1, _Arg2 a2) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<ObjMemFunc>(func)), m_args(a1, a2) {
    }
};

/*****************************************************************************
 * Class RfxPostInvoke3
 *****************************************************************************/

// Internal used.
template <class _Arg1, class _Arg2, class _Arg3>
class RfxPostInvoke3 : public RfxSlot {
public:
    typedef void (RfxObject::*ObjMemFunc)(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3);

public:
    RfxSignalArg3 <_Arg1, _Arg2, _Arg3> m_args;

public:

    template <class _MemFunc>
    RfxPostInvoke3(RfxObject *obj, _MemFunc func, _Arg1 a1, _Arg2 a2, _Arg3 a3) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<ObjMemFunc>(func)), m_args(a1, a2, a3) {
    }
};

/*****************************************************************************
 * Class RfxPostInvoke4
 *****************************************************************************/

// Internal used.
template <class _Arg1, class _Arg2, class _Arg3, class _Arg4>
class RfxPostInvoke4 : public RfxSlot {
public:
    typedef void (RfxObject::*ObjMemFunc)(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3, _Arg4 arg4);

public:
    RfxSignalArg4 <_Arg1, _Arg2, _Arg3, _Arg4> m_args;

public:

    template <class _MemFunc>
    RfxPostInvoke4(RfxObject *obj, _MemFunc func, _Arg1 a1, _Arg2 a2, _Arg3 a3, _Arg4 a4) :
        RfxSlot(obj, (RfxSlotObjMemFuncPtr)static_cast<ObjMemFunc>(func)), m_args(a1, a2, a3, a4) {
    }
};

/*****************************************************************************
 * Class RfxAsyncSignalUtil
 *****************************************************************************/

// Internal used. RfxAsyncSignalUtil is a singleton class which helps to process asyncronous signals.
class RfxAsyncSignalUtil : public RfxObject {
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RfxAsyncSignalUtil);

// Implementation
private:
    friend class RfxBaseSignal;

// Constructor / Destructor
public:
    // Default constructor
    RfxAsyncSignalUtil() {
    }

// Method
public:
    void regPostEmit(
        const RfxBaseSignal         *baseSignal,
        RfxEmitEntryFuncPtr          emit_funcptr,
        RfxSignalArg                *arguments,
        RfxObject                   *callback_obj,
        RfxSlotObjMemFuncPtr         callback_funcptr
    );

    void processPostEmit(void *data);

    void callback0(void *data) {
        typedef RfxPostInvoke0 PostInvoke;

        PostInvoke *invoke = (PostInvoke *)data;
        RfxObject *obj = invoke->m_obj_ptr.promote().get();
        if (obj != NULL) {
            typename PostInvoke::ObjMemFunc memFunc =
                    (typename PostInvoke::ObjMemFunc)invoke->m_callback;
            (obj->*memFunc)();
        }
        delete(invoke);
    }

    template <class _Arg1>
    void callback1(void *data) {
        typedef RfxPostInvoke1 <_Arg1> PostInvoke;

        PostInvoke *invoke = (PostInvoke *)data;
        RfxObject  *obj = invoke->m_obj_ptr.promote().get();
        if (obj != NULL) {
            typename PostInvoke::ObjMemFunc memFunc =
                    (typename PostInvoke::ObjMemFunc)invoke->m_callback;
            (obj->*memFunc)(invoke->m_args.arg1);
        }
        delete(invoke);
    }

    template <class _Arg1, class _Arg2>
    void callback2(void *data) {
        typedef RfxPostInvoke2 <_Arg1, _Arg2> PostInvoke;

        PostInvoke *invoke = (PostInvoke *)data;
        RfxObject *obj = invoke->m_obj_ptr.promote().get();
        if (obj != NULL) {
            typename PostInvoke::ObjMemFunc memFunc =
                    (typename PostInvoke::ObjMemFunc)invoke->m_callback;
            (obj->*memFunc)(invoke->m_args.arg1, invoke->m_args.arg2);
        }
        delete(invoke);
    }

    template <class _Arg1, class _Arg2, class _Arg3>
    void callback3(void *data) {
        typedef RfxPostInvoke3 <_Arg1, _Arg2, _Arg3> PostInvoke;

        PostInvoke *invoke = (PostInvoke *)data;
        RfxObject *obj = invoke->m_obj_ptr.promote().get();
        if (obj != NULL) {
            typename PostInvoke::ObjMemFunc memFunc =
                    (typename PostInvoke::ObjMemFunc)invoke->m_callback;
            (obj->*memFunc)(invoke->m_args.arg1, invoke->m_args.arg2, invoke->m_args.arg3);
        }
        delete(invoke);
    }

    template <class _Arg1, class _Arg2, class _Arg3, class _Arg4>
    void callback4(void *data) {
        typedef RfxPostInvoke4 <_Arg1, _Arg2, _Arg3, _Arg4> PostInvoke;

        PostInvoke *invoke = (PostInvoke *)data;
        RfxObject *obj = invoke->m_obj_ptr.promote().get();
        if (obj != NULL) {
            typename PostInvoke::ObjMemFunc memFunc =
                    (typename PostInvoke::ObjMemFunc)invoke->m_callback;
            (obj->*memFunc)(invoke->m_args.arg1,
                            invoke->m_args.arg2,
                            invoke->m_args.arg3,
                            invoke->m_args.arg4);
        }
        delete(invoke);
    }
};

/*****************************************************************************
 * Class RfxBaseSignal
 *****************************************************************************/

// Internal used. RfxBaseSignal is a base class for all RfxSignalX.
class RfxBaseSignal {
// Constructor / Distructor
public:
    // Default constructor
    RfxBaseSignal() {
    }

    // Base implement for triggering all connected handlers
    void emitImpl(RfxEmitEntryFuncPtr emit_funcptr, void *arguments) const;

    // Real emit signal in slots
    static void emitSlots(const RfxSlotList* slot_list,
                           RfxEmitEntryFuncPtr emit_funcptr,
                           void* arguments);

    // get number of connected slot
    int getSlotCount() const;

protected:
    // Helper functions
    void baseConnectFront(RfxObject *obj, RfxSlotObjMemFuncPtr func);
    void baseConnect(RfxObject *obj, RfxSlotObjMemFuncPtr func);
    void baseDisconnect(RfxObject *obj, RfxSlotObjMemFuncPtr func);

protected:
    RfxSlotList m_slot_list;

// Implementation
private:
    void cleanInvalidSlots();

    friend class RfxAsyncSignalUtil;
};

/*****************************************************************************
 * Class RfxSignal0
 *****************************************************************************/

/*
 * Signal with no parameter
 */
class RfxSignal0 : public RfxBaseSignal {
    RFX_DECLARE_NO_COPY_CLASS(RfxSignal0);

private:
    typedef void (RfxObject::*obj_mem_funcptr_type)();

// Constructor / Distructor
public:
    // default constructor
    RfxSignal0() {
    }

    // Set the default slot
    template <class _func_type>
    void connectFront(RfxObject *obj, _func_type func) {
        baseConnectFront(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a member-to-function pointer
    template <class _func_type>
    void connect(RfxObject *obj, _func_type func) {
        baseConnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // disconnect from a member-to-function pointer
    template <class _func_type>
    void disconnect(RfxObject *obj, _func_type func) {
        baseDisconnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a callback object
    void connect(RfxCallback0 cb) {
        baseConnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // disconnect from a callback object
    void disconnect(RfxCallback0 cb) {
        baseDisconnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // invoke a single signal function
    static void emitEntry(RfxObject *obj, RfxSlotObjMemFuncPtr callback, void* arguments) {
        RFX_UNUSED(arguments);
        (obj->*(obj_mem_funcptr_type)callback)();
    }

    // trigger this signal
    void emit() const {
        if (!m_slot_list.isEmpty()) {
            RfxSlotList tmpSlotList(m_slot_list);
            emitSlots(&tmpSlotList, (RfxEmitEntryFuncPtr)&RfxSignal0::emitEntry, NULL);
        }
    }

    // post emit a signal
    template <class _func_type>
    void postEmit(
        RfxObject *obj,   // [IN] after emiting the signal, callback to which object
        _func_type func   // [IN] after emiting the signal, callback to which member function
    ) const {
        RFX_REG_POSTEMIT(RfxSignal0, NULL, obj, func);
    }

    // post emit a signal
    void postEmit() const {
        if (!m_slot_list.isEmpty()) {
            RFX_REG_POSTEMIT(RfxSignal0, NULL, NULL, NULL);
        }
    }
};

/*****************************************************************************
 * Class RfxSignal1
 *****************************************************************************/

/*
 * Signal with one parameter
 */
template <class _arg1_type>
class RfxSignal1 : public RfxBaseSignal {
    RFX_DECLARE_NO_COPY_CLASS(RfxSignal1);

private:
    typedef void (RfxObject::*obj_mem_funcptr_type)(_arg1_type arg1);
    typedef RfxSignalArg1 <_arg1_type> Argument;

public:
    // default constructor
    RfxSignal1() {
    }

    // Set the default slot
    template <class _func_type>
    void connectFront(RfxObject *obj, _func_type func) {
        baseConnectFront(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a member-to-function pointer
    template <class _func_type>
    void connect(RfxObject *obj, _func_type func) {
        baseConnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // disconnect from a member-to-function pointer
    template <class _func_type>
    void disconnect(RfxObject *obj, _func_type func) {
        baseDisconnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a callback object
    void connect(RfxCallback1<_arg1_type> cb) {
        baseConnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // disconnect from a callback object
    void disconnect(RfxCallback1<_arg1_type> cb) {
        baseDisconnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // invoke a single signal function
    static void emitEntry(RfxObject *obj, RfxSlotObjMemFuncPtr callback, RfxSignalArg *arguments) {
        Argument* arg = (Argument*) arguments;
        (obj->* obj_mem_funcptr_type(callback))(arg->arg1);
    }

    // trigger this signal
    void emit(_arg1_type arg1) const {
        if (!m_slot_list.isEmpty()) {
            Argument arg(arg1);
            RfxSlotList tmpSlotList(m_slot_list);
            emitSlots(&tmpSlotList, (RfxEmitEntryFuncPtr)&RfxSignal1::emitEntry, (void*)&arg);
        }
    }

    // post emit a signal
    template <class _func_type>
    void postEmit(
        _arg1_type arg1,
        RfxObject *obj,      // [IN] after emiting the signal, callback to which object
        _func_type func      // [IN] after emiting the signal, callback to which member function
    ) const {
        Argument *arg;
        arg = new Argument(arg1);
        RFX_REG_POSTEMIT(RfxSignal1, arg, obj, func);
    }

    // post emit a signal without callback
    void postEmit(_arg1_type arg1) const {
        if (!m_slot_list.isEmpty()) {
            Argument *arg;
            arg = new Argument(arg1);
            RFX_REG_POSTEMIT(RfxSignal1, arg, NULL, NULL);
        }
    }
};

/*****************************************************************************
 * Class RfxSignal2
 *****************************************************************************/

/*
 * Signal with two parameters
 */
template <class _arg1_type, class _arg2_type>
class RfxSignal2 : public RfxBaseSignal {
    RFX_DECLARE_NO_COPY_CLASS(RfxSignal2);

private:
    typedef void (RfxObject::*obj_mem_funcptr_type)(_arg1_type arg1, _arg2_type arg2);
    typedef RfxSignalArg2<_arg1_type, _arg2_type> Argument;

public:
    // default constructor
    RfxSignal2() {
    }

    // Set the default slot
    template <class _func_type>
    void connectFront(RfxObject *obj, _func_type func) {
        baseConnectFront(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a member-to-function pointer
    template <class _func_type>
    void connect(RfxObject *obj, _func_type func) {
        baseConnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // disconnect from a member-to-function pointer
    template <class _func_type>
    void disconnect(RfxObject *obj, _func_type func) {
        baseDisconnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a callback object
    void connect(RfxCallback2<_arg1_type, _arg2_type> cb) {
        baseConnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // disconnect from a callback object
    void disconnect(RfxCallback2<_arg1_type, _arg2_type> cb) {
        baseDisconnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // invoke a single signal function
    static void emitEntry(RfxObject *obj, RfxSlotObjMemFuncPtr callback, void *arguments) {
        Argument* arg = (Argument*) arguments;
        (obj->* obj_mem_funcptr_type(callback))(arg->arg1, arg->arg2);
    }

    // trigger this signal
    void emit(_arg1_type arg1, _arg2_type arg2) const {
        if (!m_slot_list.isEmpty()) {
            Argument arg(arg1, arg2);
            RfxSlotList tmpSlotList(m_slot_list);
            emitSlots(&tmpSlotList, (RfxEmitEntryFuncPtr)&RfxSignal2::emitEntry, (void*)&arg);
        }
    }

    // post emit a signal
    template <class _func_type>
    void postEmit(
        _arg1_type arg1,
        _arg2_type arg2,
        RfxObject *obj,      // [IN] after emiting the signal, callback to which object
        _func_type func      // [IN] after emiting the signal, callback to which member function
    ) const {
        Argument *arg;
        arg = new Argument(arg1, arg2);
        RFX_REG_POSTEMIT(RfxSignal2, arg, obj, func);
    }

    // post emit a signal
    void postEmit(_arg1_type arg1, _arg2_type arg2) const {
        if (!m_slot_list.isEmpty()) {
            Argument *arg;
            arg = new Argument(arg1, arg2);
            RFX_REG_POSTEMIT(RfxSignal2, arg, NULL, NULL);
        }
    }
};

/*****************************************************************************
 * Class RfxSignal3
 *****************************************************************************/

/*
 * Signal with 3 parameters
 */
template <class _arg1_type, class _arg2_type, class _arg3_type>
class RfxSignal3 : public RfxBaseSignal {
    RFX_DECLARE_NO_COPY_CLASS(RfxSignal3);

private:
    typedef void (RfxObject::*obj_mem_funcptr_type)(_arg1_type arg1,
                                                       _arg2_type arg2, _arg3_type arg3);
    typedef RfxSignalArg3<_arg1_type, _arg2_type, _arg3_type> Argument;

public:
    // default constructor
    RfxSignal3() {
    }

    // Set the default slot
    template <class _func_type>
    void connectFront(RfxObject *obj, _func_type func) {
        baseConnectFront(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a member-to-function pointer
    template <class _func_type>
    void connect(RfxObject *obj, _func_type func) {
        baseConnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // disconnect from a member-to-function pointer
    template <class _func_type>
    void disconnect(RfxObject *obj, _func_type func) {
        baseDisconnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a callback object
    void connect(RfxCallback3<_arg1_type, _arg2_type, _arg3_type> cb) {
        baseConnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // disconnect from a callback object
    void disconnect(RfxCallback3<_arg1_type, _arg2_type, _arg3_type> cb) {
        baseDisconnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // invoke a single signal function
    static void emitEntry(RfxObject *obj, RfxSlotObjMemFuncPtr callback, void *arguments) {
        Argument* arg = (Argument*) arguments;
        (obj->* obj_mem_funcptr_type(callback))(arg->arg1, arg->arg2, arg->arg3);
    }

    // trigger this signal
    void emit(_arg1_type arg1, _arg2_type arg2, _arg3_type arg3) const {
        if (!m_slot_list.isEmpty()) {
            Argument arg(arg1, arg2, arg3);
            RfxSlotList tmpSlotList(m_slot_list);
            emitSlots(&tmpSlotList, (RfxEmitEntryFuncPtr)&RfxSignal3::emitEntry, (void*)&arg);
        }
    }

    // post emit a signal
    template <class _func_type>
    void postEmit(
        _arg1_type arg1,
        _arg2_type arg2,
        _arg3_type arg3,
        RfxObject *obj,      // [IN] after emiting the signal, callback to which object
        _func_type func      // [IN] after emiting the signal, callback to which member function
    ) const {
        Argument *arg;
        arg = new Argument(arg1, arg2, arg3);
        RFX_REG_POSTEMIT(RfxSignal3, arg, obj, func);
    }

    // post emit a signal
    void postEmit(_arg1_type arg1, _arg2_type arg2, _arg3_type arg3) const {
        if (!m_slot_list.isEmpty()) {
            Argument *arg;
            arg = new Argument(arg1, arg2, arg3);
            RFX_REG_POSTEMIT(RfxSignal3, arg, NULL, NULL);
        }
    }
};

/*****************************************************************************
 * Class RfxSignal4
 *****************************************************************************/

/*
 * Signal with 4 parameters
 */
template <class _arg1_type, class _arg2_type, class _arg3_type, class _arg4_type>
class RfxSignal4 : public RfxBaseSignal {
    RFX_DECLARE_NO_COPY_CLASS(RfxSignal4);

private:
    typedef void (RfxObject::*obj_mem_funcptr_type)(_arg1_type arg1, _arg2_type arg2,
                                                       _arg3_type arg3, _arg4_type arg4);
    typedef RfxSignalArg4<_arg1_type, _arg2_type, _arg3_type, _arg4_type> Argument;

public:
    // default constructor
    RfxSignal4() {
    }

    // Set the default slot
    template <class _func_type>
    void connectFront(RfxObject *obj, _func_type func) {
        baseConnectFront(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a member-to-function pointer
    template <class _func_type>
    void connect(RfxObject *obj, _func_type func) {
        baseConnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // disconnect from a member-to-function pointer
    template <class _func_type>
    void disconnect(RfxObject *obj, _func_type func) {
        baseDisconnect(obj, (RfxSlotObjMemFuncPtr)static_cast<obj_mem_funcptr_type>(func));
    }

    // connect to a callback object
    void connect(RfxCallback4<_arg1_type, _arg2_type, _arg3_type, _arg4_type> cb) {
        baseConnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // disconnect from a callback object
    void disconnect(RfxCallback4<_arg1_type, _arg2_type, _arg3_type, _arg4_type> cb) {
        baseDisconnect(cb.m_obj_ptr.promote().get(), cb.m_callback);
    }

    // invoke a single signal function
    static void emitEntry(RfxObject *obj, RfxSlotObjMemFuncPtr callback, void *arguments) {
        Argument* arg = (Argument*) arguments;
        (obj->* obj_mem_funcptr_type(callback))(arg->arg1, arg->arg2, arg->arg3, arg->arg4);
    }

    // trigger this signal
    void emit(_arg1_type arg1, _arg2_type arg2, _arg3_type arg3, _arg4_type arg4) const {
        if (!m_slot_list.isEmpty()) {
            Argument arg(arg1, arg2, arg3, arg4);
            RfxSlotList tmpSlotList(m_slot_list);
            emitSlots(&tmpSlotList, (RfxEmitEntryFuncPtr)&RfxSignal4::emitEntry, (void*)&arg);
        }
    }

    // post emit a signal
    template <class _func_type>
    void postEmit(
        _arg1_type arg1,
        _arg2_type arg2,
        _arg3_type arg3,
        _arg4_type arg4,
        RfxObject *obj,      // [IN] after emiting the signal, callback to which object
        _func_type func      // [IN] after emiting the signal, callback to which member function
    ) const {
        Argument *arg;
        arg = new Argument(arg1, arg2, arg3, arg4);
        RFX_REG_POSTEMIT(RfxSignal4, arg, obj, func);
    }

    // post emit a signal
    void postEmit(_arg1_type arg1, _arg2_type arg2, _arg3_type arg3, _arg4_type arg4) const {
        if (!m_slot_list.isEmpty()) {
            Argument *arg;
            arg = new Argument(arg1, arg2, arg3, arg4);
            RFX_REG_POSTEMIT(RfxSignal4, arg, NULL, NULL);
        }
    }
};

/*****************************************************************************
 * Global Function
 *****************************************************************************/

// When a slot want to stop the signal emiting,
//   it can call this API to stop the emit process.
void rfxStopEmit();

// Post invoke a member function without argument
template <class _MemFunc>
void rfxPostInvoke0(
    RfxObject *obj,   // [IN] the callback object
    _MemFunc func     // [IN] the callback object member function
) {
    typedef RfxPostInvoke0 PostInvoke;
    PostInvoke *invoke;
    invoke = new PostInvoke(obj, func);
    rfxPostInvoke(
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil),
        &RfxAsyncSignalUtil::callback0,
        invoke);
}

// Post invoke a member function with 1 arguments
template <class _MemFunc, class _Arg1>
void rfxPostInvoke1(
    RfxObject *obj, // [IN] the callback object
    _MemFunc func,  // [IN] the callback object member function
    _Arg1 arg1      // [IN] the 1'st argument
) {
    typedef RfxPostInvoke1 <_Arg1> PostInvoke;
    PostInvoke *invoke;
    invoke = new PostInvoke(obj, func, arg1);
    rfxPostInvoke(
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil),
        &RfxAsyncSignalUtil::callback1 <_Arg1>,
        invoke);
}

// Post invoke a member function with 2 arguments
template <class _MemFunc, class _Arg1, class _Arg2>
void rfxPostInvoke2(
    RfxObject *obj, // [IN] the callback object
    _MemFunc func,  // [IN] the callback object member function
    _Arg1 arg1,     // [IN] the 1'st argument
    _Arg2 arg2      // [IN] the 2'nd argument
) {
    typedef RfxPostInvoke2 <_Arg1, _Arg2> PostInvoke;
    PostInvoke *invoke;
    invoke = new PostInvoke(obj, func, arg1, arg2);
    rfxPostInvoke(
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil),
        &RfxAsyncSignalUtil::callback2 <_Arg1, _Arg2>,
        invoke);
}

// Post invoke a member function with 3 arguments
template <class _MemFunc, class _Arg1, class _Arg2, class _Arg3>
void rfxPostInvoke3(
    RfxObject *obj, // [IN] the callback object
    _MemFunc func,  // [IN] the callback object member function
    _Arg1 arg1,     // [IN] the 1'st argument
    _Arg2 arg2,     // [IN] the 2'nd argument
    _Arg3 arg3      // [IN] the 3'rd argument
) {
    typedef RfxPostInvoke3 <_Arg1, _Arg2, _Arg3> PostInvoke;
    PostInvoke *invoke;
    invoke = new PostInvoke(obj, func, arg1, arg2, arg3);
    rfxPostInvoke(
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil),
        &RfxAsyncSignalUtil::callback3 <_Arg1, _Arg2, _Arg3>,
        invoke);
}

// Post invoke a member function with 4 arguments
template <class _MemFunc, class _Arg1, class _Arg2, class _Arg3, class _Arg4>
void rfxPostInvoke4(
    RfxObject *obj,  // [IN] the callback object
    _MemFunc func,   // [IN] the callback object member function
    _Arg1 arg1,      // [IN] the 1'st argument
    _Arg2 arg2,      // [IN] the 2'nd argument
    _Arg3 arg3,      // [IN] the 3'rd argument
    _Arg4 arg4       // [IN] the 4'th argument
) {
    typedef RfxPostInvoke4 <_Arg1, _Arg2, _Arg3, _Arg4> PostInvoke;
    PostInvoke *invoke;
    invoke = new PostInvoke(obj, func, arg1, arg2, arg3, arg4);
    rfxPostInvoke(
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil),
        &RfxAsyncSignalUtil::callback4 <_Arg1, _Arg2, _Arg3, _Arg4>,
        invoke);
}

#endif /* __RFX_SIGNAL_H__ */
