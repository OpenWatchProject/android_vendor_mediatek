/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_H_

#include "CookieStore_t.h"

#include <featurePipe/core/include/PipeLogHeaderBegin.h>
#include "DebugControl.h"
#define PIPE_TRACE TRACE_COOKIE_STORE
#define PIPE_CLASS_TAG "CookieStore"
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename T_User>
CookieStore<T_User>::CookieStore()
    : mStoreCount(0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

template <typename T_User>
CookieStore<T_User>::~CookieStore()
{
    TRACE_FUNC_ENTER();
    waitAllCallDone();
    TRACE_FUNC_EXIT();
}

template <typename T_User>
void CookieStore<T_User>::enque(T_User *user, T_Stream *stream, T_Param param, const T_Data &data)
{
    TRACE_FUNC_ENTER();
    T_Token token = NULL;
    T_Cookie cookie(param);
    token = CookieStore<T_User>::store(this, user, cookie, data);
    cookie.replace(param, token);
    signalEnque();
    if( !user->onCookieStoreEnque(stream, param) )
    {
        MY_LOGE("onCookieEnque failed, token=%p", token);
        CookieStore<T_User>::drop(token);
        cookie.restore(param);
        user->onCookieStoreCB(T_User::MSG_COOKIE_FAIL, param, data);
        signalDeque();
    }
    TRACE_FUNC_EXIT();
}

template <typename T_User>
void CookieStore<T_User>::waitAllCallDone()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    while( mStoreCount )
    {
        TRACE_FUNC("wait with count=%d", mStoreCount);
        mCondition.wait(mMutex);
    }
    TRACE_FUNC_EXIT();
}

template <typename T_User>
void CookieStore<T_User>::staticProcessCB(const T_Msg &msg, T_Param param, T_Token token)
{
    TRACE_FUNC_ENTER();
    T_User *user = NULL;
    T_Store *store = NULL;
    T_Cookie cookie;
    T_Data data;

    if( !T_Store::fetch(token, store, user, cookie, data) || !user )
    {
        MY_LOGE("Invalid data from token=%p, store=%p, user=%p", token, store, user);
    }
    else
    {
        cookie.restore(param);
        user->onCookieStoreCB(msg, param, data);
        store->signalDeque();
    }
    TRACE_FUNC_EXIT();
}

template <typename T_User>
typename CookieStore<T_User>::T_Token CookieStore<T_User>::store(T_Store *store, T_User *user, const T_Cookie &cookie, const T_Data &data)
{
    TRACE_FUNC_ENTER();
    BACKUP_DATA_TYPE *backup = new BACKUP_DATA_TYPE(store, user, cookie, data);
    if( !backup )
    {
        MY_LOGE("OOM: cannot allocate memory for backup data");
    }
    TRACE_FUNC_EXIT();
    return (T_Token)backup;
}

template <typename T_User>
bool CookieStore<T_User>::fetch(T_Token token, T_Store* &store, T_User* &user, T_Cookie &cookie, T_Data &data)
{
    TRACE_FUNC_ENTER();
    bool ret = false;
    BACKUP_DATA_TYPE *backup = NULL;
    backup = reinterpret_cast<BACKUP_DATA_TYPE*>(token);
    TRACE_FUNC("get token=%p backup=%p", token, backup);
    if( backup )
    {
        if( backup->mMagic != MAGIC_VALID )
        {
            MY_LOGE("Backup data is corrupted: token=%p backup=%p magic=0x%x", token, backup, backup->mMagic);
        }
        else
        {
            backup->mMagic = MAGIC_USED;
            store = backup->mStore;
            user = backup->mUser;
            cookie = backup->mCookie;
            data = backup->mData;
            delete backup;
            backup = NULL;
            ret = true;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

template <typename T_User>
bool CookieStore<T_User>::drop(T_Token token)
{
    TRACE_FUNC_ENTER();
    bool ret = false;
    BACKUP_DATA_TYPE *backup = NULL;
    backup = reinterpret_cast<BACKUP_DATA_TYPE*>(token);
    TRACE_FUNC("get token=%p backup=%p", token, backup);
    if( backup )
    {
        if( backup->mMagic != MAGIC_VALID )
        {
            MY_LOGE("Backup data is corrupted: token=%p backup=%p magic=0x%x", token, backup, backup->mMagic);
        }
        else
        {
            backup->mMagic = MAGIC_USED;
            delete backup;
            backup = NULL;
            ret = true;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

template <typename T_User>
void CookieStore<T_User>::signalEnque()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    ++mStoreCount;
    TRACE_FUNC_EXIT();
}

template <typename T_User>
void CookieStore<T_User>::signalDeque()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    --mStoreCount;
    mCondition.broadcast();
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#include <featurePipe/core/include/PipeLogHeaderEnd.h>
#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_H_
