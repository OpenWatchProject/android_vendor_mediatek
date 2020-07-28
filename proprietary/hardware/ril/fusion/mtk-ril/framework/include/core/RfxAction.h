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
 * File name:  RfxAction.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of action.
 */

#ifndef __RFX_ACTION_H__
#define __RFX_ACTION_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include "RfxSignal.h"
#include "RfxDebugInfo.h"
#include "RfxDefs.h"
#include "RfxLog.h"

using ::android::RefBase;
using ::android::Vector;

/*****************************************************************************
 * Class RfxAction
 *****************************************************************************/

class RfxAction : public virtual RefBase
                #ifdef RFX_OBJ_DEBUG
                  , public IRfxDebugLogger
                #endif
{
public:

    virtual void act() = 0;

#ifdef RFX_OBJ_DEBUG
public:
    RfxAction();

    virtual ~RfxAction();

    virtual void dump(int level = 0) const = 0;

    static void dumpActionList();

    static void dumpActionConstructionCallStack(void *action_address);

private:
    RfxDebugInfo *m_debug_info;
    static Vector<RfxDebugInfo*> *s_root_action_debug_info;
#endif //#ifdef RFX_OBJ_DEBUG

};

class RfxAction0 : public RfxAction {
public:

    template <class _MemFunc>
    RfxAction0(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func     // [IN] the callback object member function
                ) : m_invoke(obj, func) {}

public:

    virtual void act() {
        RfxPostInvoke0 *invoke;
        invoke = new RfxPostInvoke0(m_invoke.m_obj_ptr.promote().get(), m_invoke.m_callback);
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil)->callback0(invoke);
    }

private:

    RfxPostInvoke0 m_invoke;

#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxAction0 = %p, obj = %p, %s",
                                this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

class RfxPostAction0 : public RfxAction {
public:

    template <class _MemFunc>
    RfxPostAction0(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func     // [IN] the callback object member function
                ) : m_invoke(obj, func) {}

public:

    virtual void act() {
        rfxPostInvoke0(m_invoke.m_obj_ptr.promote().get(), m_invoke.m_callback);
    }

private:

    RfxPostInvoke0 m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxPostAction0 = %p, obj = %p, %s",
                                    this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1>
class RfxAction1 : public RfxAction {
public:

    template <class _MemFunc>
    RfxAction1(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func,      // [IN] the callback object member function
                _Arg1 arg1          // [IN] the 1'st argument
                ) : m_invoke(obj, func, arg1) {}

public:

    virtual void act() {
        typedef RfxPostInvoke1<_Arg1> PostInvoke;
        PostInvoke *invoke;
        invoke = new PostInvoke(m_invoke.m_obj_ptr.promote().get(),
                                (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                                m_invoke.m_args.arg1);
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil)->callback1<_Arg1>(invoke);
    }

private:

    RfxPostInvoke1<_Arg1> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxAction1 = %p, obj = %p, %s",
                                this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1>
class RfxPostAction1 : public RfxAction {
public:

    template <class _MemFunc>
    RfxPostAction1(
                    RfxObject *obj,     // [IN] the callback object
                    _MemFunc func,      // [IN] the callback object member function
                   _Arg1 arg1          // [IN] the 1'st argument
                    ) : m_invoke(obj, func, arg1) {}

public:

    virtual void act() {
        typedef RfxPostInvoke1<_Arg1> PostInvoke;
        rfxPostInvoke1(m_invoke.m_obj_ptr.promote().get(),
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                        m_invoke.m_args.arg1);
    }

private:

    RfxPostInvoke1<_Arg1> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxPostAction1 = %p, obj = %p, %s",
                                    this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};


template <class _Arg1, class _Arg2>
class RfxAction2 : public RfxAction {
public:

    template <class _MemFunc>
    RfxAction2(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func,      // [IN] the callback object member function
                _Arg1 arg1,         // [IN] the 1'st argument
                _Arg2 arg2          // [IN] the 2'nd argument
                ) : m_invoke(obj, func, arg1, arg2) {}

public:

    virtual void act() {
        typedef RfxPostInvoke2<_Arg1, _Arg2> PostInvoke;
        PostInvoke *invoke;
        invoke = new PostInvoke(m_invoke.m_obj_ptr.promote().get(),
                                (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                                m_invoke.m_args.arg1,
                                m_invoke.m_args.arg2);
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil)->callback2<_Arg1, _Arg2>(invoke);
    }

private:

    RfxPostInvoke2<_Arg1, _Arg2> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxAction2 = %p, obj = %p, %s",
                                this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1, class _Arg2>
class RfxPostAction2 : public RfxAction {
public:

    template <class _MemFunc>
    RfxPostAction2(
                    RfxObject *obj,     // [IN] the callback object
                    _MemFunc func,      // [IN] the callback object member function
                    _Arg1 arg1,         // [IN] the 1'st argument
                    _Arg2 arg2          // [IN] the 2'nd argument
                    ) : m_invoke(obj, func, arg1, arg2) {}

public:

    virtual void act() {
        typedef RfxPostInvoke2<_Arg1, _Arg2> PostInvoke;
        rfxPostInvoke2(m_invoke.m_obj_ptr.promote().get(),
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                        m_invoke.m_args.arg1,
                        m_invoke.m_args.arg2);
    }

private:

    RfxPostInvoke2<_Arg1, _Arg2> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxPostAction2 = %p, obj = %p, %s",
                                    this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1, class _Arg2, class _Arg3>
class RfxAction3 : public RfxAction {
public:

    template <class _MemFunc>
    RfxAction3(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func,      // [IN] the callback object member function
                _Arg1 arg1,         // [IN] the 1'st argument
                _Arg2 arg2,         // [IN] the 2'nd argument
                _Arg3 arg3          // [IN] the 3'rd argument
                ) : m_invoke(obj, func, arg1, arg2, arg3) {}

public:

    virtual void act() {
        typedef RfxPostInvoke3<_Arg1, _Arg2, _Arg3> PostInvoke;
        PostInvoke *invoke;
        invoke = new PostInvoke(
                        m_invoke.m_obj_ptr.promote().get(),
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                        m_invoke.m_args.arg1,
                        m_invoke.m_args.arg2,
                        m_invoke.m_args.arg3);
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil)->callback3<_Arg1, _Arg2, _Arg3>(invoke);
    }

private:

    RfxPostInvoke3<_Arg1, _Arg2, _Arg3> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxAction3 = %p, obj = %p, %s",
                                this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1, class _Arg2, class _Arg3>
class RfxPostAction3 : public RfxAction {
public:

    template <class _MemFunc>
    RfxPostAction3(
                    RfxObject *obj,     // [IN] the callback object
                    _MemFunc func,      // [IN] the callback object member function
                    _Arg1 arg1,         // [IN] the 1'st argument
                    _Arg2 arg2,         // [IN] the 2'nd argument
                    _Arg3 arg3          // [IN] the 3'rd argument
                    ) : m_invoke(obj, func, arg1, arg2, arg3) {}

public:

    virtual void act() {
        typedef RfxPostInvoke3<_Arg1, _Arg2, _Arg3> PostInvoke;
        rfxPostInvoke3(m_invoke.m_obj_ptr.promote().get(),
                       (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                       m_invoke.m_args.arg1,
                       m_invoke.m_args.arg2,
                       m_invoke.m_args.arg3);
    }

private:

    RfxPostInvoke3<_Arg1, _Arg2, _Arg3> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxPostAction3 = %p, obj = %p, %s",
                                    this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1, class _Arg2, class _Arg3, class _Arg4>
class RfxAction4 : public RfxAction {
public:

    template <class _MemFunc>
    RfxAction4(
                RfxObject *obj,     // [IN] the callback object
                _MemFunc func,      // [IN] the callback object member function
                _Arg1 arg1,         // [IN] the 1'st argument
                _Arg2 arg2,         // [IN] the 2'nd argument
                _Arg3 arg3,         // [IN] the 3'rd argument
                _Arg4 arg4          // [IN] the 4'th argument
                ) : m_invoke(obj, func, arg1, arg2, arg3, arg4) {}

public:

    virtual void act() {
        typedef RfxPostInvoke4<_Arg1, _Arg2, _Arg3, _Arg4> PostInvoke;
        PostInvoke *invoke;
        invoke = new PostInvoke(
                        m_invoke.m_obj_ptr.promote().get(),
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                        m_invoke.m_args.arg1,
                        m_invoke.m_args.arg2,
                        m_invoke.m_args.arg3,
                        m_invoke.m_args.arg4);
        RFX_OBJ_GET_INSTANCE(RfxAsyncSignalUtil)->callback4<_Arg1, _Arg2, _Arg3, _Arg4>(invoke);
    }

private:

    RfxPostInvoke4<_Arg1, _Arg2, _Arg3, _Arg4> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxAction4 = %p, obj = %p, %s",
                                this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

template <class _Arg1, class _Arg2, class _Arg3, class _Arg4>
class RfxPostAction4 : public RfxAction {
public:

    template <class _MemFunc>
    RfxPostAction4(
                    RfxObject *obj,     // [IN] the callback object
                    _MemFunc func,      // [IN] the callback object member function
                    _Arg1 arg1,         // [IN] the 1'st argument
                    _Arg2 arg2,         // [IN] the 2'nd argument
                    _Arg3 arg3,         // [IN] the 3'rd argument
                    _Arg4 arg4          // [IN] the 4'th argument
                    ) : m_invoke(obj, func, arg1, arg2, arg3, arg4) {}

public:

    virtual void act() {
        typedef RfxPostInvoke4<_Arg1, _Arg2, _Arg3, _Arg4> PostInvoke;
        rfxPostInvoke4(m_invoke.m_obj_ptr.promote().get(),
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback,
                        m_invoke.m_args.arg1,
                        m_invoke.m_args.arg2,
                        m_invoke.m_args.arg3,
                        m_invoke.m_args.arg4);
    }

private:

    RfxPostInvoke4<_Arg1, _Arg2, _Arg3, _Arg4> m_invoke;
#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const {
        RfxObject *obj = m_invoke.m_obj_ptr.promote().get();
        RFX_LOG_D(RFX_DEBUG_INFO_TAG, "RfxPostAction4 = %p, obj = %p, %s",
                                    this, obj, obj == NULL ? "" : obj->getClassInfo()->getClassName());
        RFX_UNUSED(level);
    }
#endif //#ifdef RFX_OBJ_DEBUG
};

#endif /* __RFX_ACTION_H__ */

