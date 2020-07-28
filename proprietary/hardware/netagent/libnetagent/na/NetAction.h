/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef __NET_ACTION_H__
#define __NET_ACTION_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/RefBase.h>

using ::android::RefBase;
using ::android::sp;

/*****************************************************************************
 * Class NetObject
 *****************************************************************************/
class NetObject : public virtual RefBase {
    public:
        NetObject() {
        }

        virtual ~NetObject() {
        }
};

/*****************************************************************************
 * Class NetBase
 *****************************************************************************/
typedef void (NetObject::*NetObjMemFuncPtr)();
class NetBase {
    public:
        NetBase(NetObject *obj, NetObjMemFuncPtr callCallback) :
            m_obj_ptr(obj),
            m_callback(callCallback) {
        }
        virtual ~NetBase() {
        }

    public:
        NetObject* m_obj_ptr;
        NetObjMemFuncPtr m_callback;
};

/*****************************************************************************
 * Class NetActionArg
 *****************************************************************************/
template <typename... Ts>
class NetActionArg {
    public:
        std::tuple<Ts...> args;

        NetActionArg(Ts... args) : args(std::tuple<Ts...>(args...)) {
        }
        virtual ~NetActionArg() {
        }
};

/*****************************************************************************
 * Class NetPostInvoke
 *****************************************************************************/
template <typename... Ts>
class NetPostInvoke : public NetBase {
    public:
        typedef void (NetObject::*ObjMemFunc)(bool bSuccess, Ts... args);

    public:
        NetActionArg<Ts...> m_args;

    public:
        template <class _MemFunc>
        NetPostInvoke(NetObject *obj, _MemFunc func, Ts... args) :
            NetBase(obj, (NetObjMemFuncPtr)static_cast<ObjMemFunc>(func)), m_args(args...) {
        }
        virtual ~NetPostInvoke() {
        }
};

/*****************************************************************************
 * Class NetAction
 *****************************************************************************/
class NetActionBase : public virtual RefBase {
    public:
        virtual void ack(bool bSuccess = true) = 0;

    public:
        NetActionBase() {
        }
        virtual ~NetActionBase() {
        }
};

/*****************************************************************************
 * Class NetAction
 *****************************************************************************/
template <typename... Ts>
class NetAction : public virtual NetActionBase {
    public:
        template <class _MemFunc>
        NetAction(
                NetObject *obj, // [IN] the callback object
                _MemFunc func, // [IN] the callback object member function
                Ts... args // [IN] n parameters
                ) : m_invoke(obj, func, args...) {}
        virtual ~NetAction() {
        }

    public:
        virtual void ack(bool bSuccess = true) {
            callback(bSuccess, std::index_sequence_for<Ts...>());
        }

    private:
        template <std::size_t... Is>
        void callback(bool bSuccess, std::index_sequence<Is...>) {
            typedef NetPostInvoke<Ts...> PostInvoke;
            NetObject *obj = m_invoke.m_obj_ptr;
            if (obj != NULL) {
                typename PostInvoke::ObjMemFunc memFunc =
                        (typename PostInvoke::ObjMemFunc)m_invoke.m_callback;
                (obj->*memFunc)(bSuccess, std::get<Is>(m_invoke.m_args.args)...);
            }
        }

    private:
        NetPostInvoke<Ts...> m_invoke;
};

#endif /* __NET_ACTION_H__ */
