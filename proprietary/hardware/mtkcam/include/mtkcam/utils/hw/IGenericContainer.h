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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IGENERICCONTAINER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IGENERICCONTAINER_H_

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include <utils/Printer.h>

#include <mtkcam/def/common.h>

namespace NSCam {


/**
 * The manager of generic containers.
 */
class IGenericContainerManager
{
public:

    struct Key
    {
        /**
         * The main key. It's usually a timestamp.
         */
        uint64_t k0;

        /**
         * The sub key. It's usually related to sizeof(T).
         */
        uint64_t k1;
    };

    /**
     * Perform the lock/unlock operations.
     */
    virtual auto    lock() -> void                                          = 0;
    virtual auto    unlock() -> void                                        = 0;

    /**
     * Try to query a generic container according to a given key.
     *
     * @param key The key value of the element to search for.
     *
     * @param userName The user name (for debugging).
     *
     * @Notes This call must be performed before lock().
     */
    virtual auto    queryLocked(
                        Key const& key,
                        std::string const& userName
                    ) -> std::shared_ptr<void>                              = 0;

    /**
     * Insert a generic container.
     *
     * @param key The key of the element to find
     *
     * @param container The container to insert.
     *
     * @param userName The user name (for debugging).
     *
     * @Notes This call must be performed before lock().
     */
    virtual auto    insertLocked(
                        Key const& key,
                        std::shared_ptr<void>const& container,
                        std::string const& userName
                    ) -> bool                                               = 0;

    /**
     * Clean up all of the containers which have the given main key (i.e. Key::k0).
     *
     * @param key The key of the element to find
     */
    virtual auto    cleanup(Key const& key) -> void                         = 0;

    /**
     * Dump the state to the log.
     */
    virtual auto    dumpState(android::Printer& printer) -> void            = 0;

public:
    virtual         ~IGenericContainerManager() = default;

    /**
     * Get the singleton instance of the container manager.
     */
    static auto     get() -> std::shared_ptr<IGenericContainerManager>;

};


/**
 * The (sub) key used to identify a generic container.
 */
enum class IGenericContainerKey : uint32_t
{
    /**************************************************************************
     * 0x00000000 - 0x0FFFFFFF
     *
     * This range is reserved for custom or 3rd-party defined use cases.
     **************************************************************************/
    VENDOR_START = 0x0,

    /**************************************************************************
     * 0x10000000 - 0x1FFFFFFF
     *
     * This range is reserved for MTK defined use cases.
     **************************************************************************/
    MTK_START = 0x10000000,

};


/**
 * The class template of a container of generic-type element.
 *
 * @param T The type of the elements.
 */
template <class T>
class IGenericContainer
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGenericContainer.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    std::atomic_size_t  mCapacity{1};
    std::list<T>        mQueue;
    mutable std::mutex  mLock;

    auto            reserve(size_t capacity) -> void { mCapacity = capacity; }
    auto            capacity() const -> size_t { return mCapacity.load(); }

    auto            size() const -> size_t
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        return mQueue.size();
                    }

    auto            clear() -> void
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        mQueue.clear();
                    }

    auto            push_back(const T& value) -> void
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        mQueue.push_back(value);
                        while ( mQueue.size() > mCapacity ) {
                            mQueue.pop_front();
                        }
                    }

    auto            cloneFront(T& out) -> bool
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        if ( ! mQueue.empty() ) {
                            out = mQueue.front();
                            return true;
                        }
                        return false;
                    }

    auto            cloneBack(T& out) -> bool
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        if ( ! mQueue.empty() ) {
                            out = mQueue.back();
                            return true;
                        }
                        return false;
                    }

    auto            clone(std::list<T>& out) -> bool
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        out = mQueue;
                        return ! out.empty();
                    }

protected:
    static  auto    get_or_make_container(
                        std::string const& userName,
                        uint64_t mainKey,
                        uint32_t subKey
                    ) -> std::shared_ptr<IGenericContainer>
                    {
                        const IGenericContainerManager::Key key{
                                .k0 = mainKey,
                                .k1 = static_cast<uint64_t>(subKey) | (sizeof(T)<<32),};
                        auto manager = IGenericContainerManager::get();
                        if ( manager != nullptr ) {
                            manager->lock();
                            auto c = manager->queryLocked(key, userName);   // get the existed one
                            if ( c == nullptr ) {
                                c = std::make_shared<IGenericContainer>();  // make a new one
                                if ( c != nullptr ) {
                                    manager->insertLocked(key, c, userName);
                                }
                            }
                            manager->unlock();
                            return std::static_pointer_cast<IGenericContainer>(c);
                        }
                        return nullptr;
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Reader/Writer Users Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * The reader of the container.
     */
    class Reader
    {
    protected:
        std::shared_ptr<IGenericContainer>
                    mContainer;
        std::string mUserName;

    public:

        /**
         * Return the number of elements that the container has currently allocated space for.
         */
        auto        capacity() const -> size_t { return mContainer->capacity(); }

        /**
         * Return the number of elements in the container.
         */
        auto        size() const -> size_t { return mContainer->size(); }

        /**
         * Clone the first element in the container.
         *
         * @param out The result to retrieve the element.
         * @return true if the first element is retrieved; false otherwise.
         */
        auto        cloneFront(T& out) -> bool { return mContainer->cloneFront(out); }

        /**
         * Clone the last element in the container.
         *
         * @param out The result to retrieve the element.
         * @return true if the last element is retrieved; false otherwise.
         */
        auto        cloneBack(T& out) -> bool { return mContainer->cloneBack(out); }

        /**
         * Clone the whole contests of the container.
         *
         * @param out The result to retrieve the element(s).
         * @return true if any element(s) is retrieved; false otherwise.
         */
        auto        clone(std::list<T>& out) -> bool { return mContainer->clone(out); }

                    Reader(std::shared_ptr<IGenericContainer>const& c, const char* name)
                        : mContainer(c), mUserName(name)
                        {}
    };

    /**
     * The writer of the container.
     */
    class Writer : public Reader
    {
    public:

        /**
         * Set the capacity of the container.
         *
         * @param capacity The new capacity of the vector.
         */
        auto        reserve(size_t capacity) -> void { this->mContainer->reserve(capacity); }

        /**
         * Erase all elements from the container. After this call, size() returns zero.
         */
        auto        clear() -> void { return this->mContainer->clear(); }

        /**
         * Append the given element value to the end of the container.
         * If the new size() is greater than capacity() then the oldest data are discarded.
         *
         * @param value The value of the element to append.
         */
        auto        push_back(const T& value) -> void { this->mContainer->push_back(value); }

                    Writer(std::shared_ptr<IGenericContainer>const& c, const char* name)
                        : Reader(c, name)
                        {}
    };

    /**
     * Make a reader or a writer of a container based on a given main key.
     *
     * @param userName The user name (for debugging).
     *
     * @param key The main key used to associate with the exsited container or
     *  create a new one if not existed.
     *
     * @return A newly-created reader or writer associated with a container.
     */
    template <class UserT>
    static  auto    makeUser(char const* userName, uint64_t mainKey, uint32_t subKey) -> std::shared_ptr<UserT>
                    {
                        auto c = get_or_make_container(userName, mainKey, subKey);
                        return ( c == nullptr ) ? nullptr : std::make_shared<UserT>(c, userName);
                    }

    static  auto    makeReader(char const* userName, uint64_t mainKey, uint32_t subKey = 0) -> std::shared_ptr<Reader>
                    {
                        return makeUser<Reader>(userName, mainKey, subKey);
                    }

    static  auto    makeWriter(char const* userName, uint64_t mainKey, uint32_t subKey = 0) -> std::shared_ptr<Writer>
                    {
                        return makeUser<Writer>(userName, mainKey, subKey);
                    }

};


}; // namespace NSCam
#endif//_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IGENERICCONTAINER_H_
