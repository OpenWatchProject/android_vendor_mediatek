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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTKCAM_3RDPARTY_CORE_SINGLETONE_CACHE_H_
#define _MTKCAM_3RDPARTY_CORE_SINGLETONE_CACHE_H_

#include <mutex>
#include <string>
#include <unordered_map>

#pragma push_macro("LOG_TAG")
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MtkCam/SingletonCache"
#include <log/log.h>
#include <android/log.h>
#include <mtkcam/utils/std/ULog.h>

namespace NSCam {
namespace NSTPIUtil {

template <typename T>
class SingletonCache
{
public:
    bool attach(unsigned keyID, const char *keyStr, const char *user);
    bool detach();
    bool update(const T &data);
    bool query(T &data) const;

private:
    class Cache
    {
    public:
        bool update(const T &data);
        bool query(T &data) const;
    private:
        mutable std::mutex mMutex;
        T mData;
    };

private:
    static std::shared_ptr<Cache> getCache(unsigned id, const std::string &name, const std::string &user);

private:
    std::shared_ptr<Cache> mCache;
    unsigned mID = 0;
    std::string mName;
    std::string mUser;
};

template <typename T>
bool SingletonCache<T>::attach(unsigned keyID, const char *keyStr, const char *user)
{
    mID = keyID;
    mName = keyStr;
    mUser = user;
    mCache = getCache(mID, mName, mUser);
    return mCache != nullptr;
}

template <typename T>
bool SingletonCache<T>::detach()
{
    mCache.reset();
    return true;
}

template <typename T>
bool SingletonCache<T>::update(const T &data)
{
    return mCache && mCache->update(data);
}

template <typename T>
bool SingletonCache<T>::query(T &data) const
{
    return mCache && mCache->query(data);
}

template <typename T>
const char* t()
{
    const char *name = __PRETTY_FUNCTION__;
    return name;
}

template <typename T>
std::shared_ptr<typename SingletonCache<T>::Cache> SingletonCache<T>::getCache(unsigned id, const std::string &name, const std::string &user)
{
    static std::mutex sCacheTableMutex;
    static std::unordered_map<std::string, std::weak_ptr<Cache>> sCacheTable;

    std::lock_guard<std::mutex> _lock(sCacheTableMutex);
    std::string key = std::to_string(id) + name;
    std::shared_ptr<Cache> cache = sCacheTable[key].lock();
    if( !cache )
    {
        cache = std::make_shared<Cache>();
        sCacheTable[key] = cache;
    }

    CAM_ULOGI(NSCam::Utils::ULog::MOD_STREAMING_TPI_PLUGIN ,"table<%s>=%p getCache(%d:%s)=%p user(%ld)=%s", t<T>(), &sCacheTable, id, name.c_str(), cache.get(), cache.use_count(), user.c_str());

    return cache;
}

template <typename T>
bool SingletonCache<T>::Cache::update(const T &data)
{
    std::lock_guard<std::mutex> _lock(mMutex);
    mData = data;
    return true;
}

template <typename T>
bool SingletonCache<T>::Cache::query(T &data) const
{
    std::lock_guard<std::mutex> _lock(mMutex);
    data = mData;
    return true;
}

} // namespace NSTPIUtil
} // namespace NSCam

#undef LOG_TAG
#pragma pop_macro("LOG_TAG")

#endif // _MTKCAM_3RDPARTY_CORE_SINGLETONE_CACHE_H_
