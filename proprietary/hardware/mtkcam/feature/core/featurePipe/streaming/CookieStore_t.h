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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_T_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_T_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef void* T_CookieStoreToken;

template <typename T_User>
class CookieStore
{
public:
    typedef CookieStore<T_User> T_Store;
    typedef typename T_User::T_Data T_Data;
    typedef typename T_User::T_Stream T_Stream;
    typedef typename T_User::T_Param T_Param;
    typedef typename T_User::T_Msg T_Msg;
    typedef typename T_User::T_Cookie T_Cookie;
    typedef T_CookieStoreToken T_Token;

    CookieStore();
    ~CookieStore();
    void enque(T_User *user, T_Stream *stream, T_Param param, const T_Data &data);
    void waitAllCallDone();
    static void staticProcessCB(const T_Msg &msg, T_Param param, T_Token token);

private:
    static T_Token store(T_Store *store, T_User *user, const T_Cookie &cookie, const T_Data &data);
    static bool fetch(T_Token token, T_Store* &store, T_User* &user, T_Cookie &cookie, T_Data &data);
    static bool drop(T_Token token);

private:
    void signalEnque();
    void signalDeque();

private:
    android::Mutex mMutex;
    android::Condition mCondition;
    unsigned mStoreCount;

private:
    enum MAGIC {
        MAGIC_VALID = 0xabcd,
        MAGIC_USED =  0xdddd,
        MAGIC_FREED = 0xfaaf,
    };

    class BACKUP_DATA_TYPE
    {
    public:
        BACKUP_DATA_TYPE()
            : mStore(NULL)
            , mUser(NULL)
            , mMagic(MAGIC_VALID)
        {
        }

        BACKUP_DATA_TYPE(T_Store *store, T_User *user, const T_Cookie &cookie, const T_Data &data)
            : mStore(store)
            , mUser(user)
            , mCookie(cookie)
            , mData(data)
            , mMagic(MAGIC_VALID)
        {
        }

        ~BACKUP_DATA_TYPE()
        {
            mMagic = MAGIC_FREED;
        }

        T_Store *mStore;
        T_User *mUser;
        T_Cookie mCookie;
        T_Data mData;
        MAGIC mMagic;
    };
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_COOKIE_STORE_T_H_
