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

/******************************************************************************
 *
 ******************************************************************************/
// enable this macro can debug if any user double free metadata instance, but may
// decrease performance.
#define IMETADATA_DEBUG_DOUBLE_FREE_DETECT 0

#define LOG_TAG "MtkCam/Metadata"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
//
#include <utils/std/Log.h>
#include <utils/std/common.h>
//
#include <utils/metadata/mtk_metadata_types.h>
//
namespace NSCam {
    typedef int MERROR;
};
//
#include <mutex>
#include <memory>
#include <array>
#include <functional>
#include <unordered_map>
#include <atomic>
using namespace NSCam;
using namespace android;
using namespace std;
using namespace com::mediatek::campostalgo::NSFeaturePipe;

// Debug utils
#if IMETADATA_DEBUG_DOUBLE_FREE_DETECT
// {{{
static std::mutex mx;
static std::unordered_map<uintptr_t, uint8_t> deathbook;
static inline void metadata_debug_new_life(uintptr_t key)
{
    std::lock_guard<std::mutex> lk(mx);
    std::atomic_thread_fence(std::memory_order_acquire);
    // new life, remove the record from deathbook
    auto itr = deathbook.find(key);
    if (itr != deathbook.end()) {
        deathbook.erase(itr);
        std::atomic_thread_fence(std::memory_order_release);
    }
}


static inline bool metadata_debug_is_alive(uintptr_t key)
{
    std::lock_guard<std::mutex> lk(mx);
    std::atomic_thread_fence(std::memory_order_acquire);
    auto itr = deathbook.find(key);
    if (itr != deathbook.end()) { // found
        return true;
    }
    else {
        // not found, add into deathbook
        deathbook[key] = 1;
        std::atomic_thread_fence(std::memory_order_release);
    }
    return false;
}
// }}}
#endif
//
#define get_mtk_metadata_tag_type(x) mType
//
/******************************************************************************
 *
 ******************************************************************************/
#if 0 //(MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
        aee_system_exception( \
            "mtkcam/Metadata", \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String)
#endif
/******************************************************************************
 * IMetadata::Memory
 ******************************************************************************/
size_t
IMetadata::Memory::size() const
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    return this->_data->size();
}


void
IMetadata::Memory::resize(const size_t size, uint8_t default_val /* = 0 */)
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    dup_data_locked();
    this->_data->resize(size, default_val);
}


size_t
IMetadata::Memory::append(const Memory &other)
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    // acquires lockers
    std::lock( this->_data_lock, other._data_lock);
    std::lock_guard<std::mutex> lk1(this->_data_lock, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(other._data_lock, std::adopt_lock);
#endif

    dup_data_locked();

    // append data
    this->_data->insert(
            this->_data->end(),
            other._data->begin(),
            other._data->end()
            );

    return this->_data->size();
}


size_t
IMetadata::Memory::appendVector(const Memory &other)
{
    return this->append(other);
}


size_t
IMetadata::Memory::appendVector(const android::Vector<MUINT8> &v)
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    dup_data_locked();

    // reserve total size of this->_data and v
    const size_t dsize1 = this->_data->size();
    const size_t dsize2 = v.size();

    // enlarge buffer
    // std::vector::resize will keep the original data after invoked resize
    this->_data->resize(dsize1 + dsize2);

    // append data from v to the tail of this->_data
    std::memcpy(
            this->_data->data() + dsize1,
            v.array(),
            dsize2);

    return this->_data->size();
}


const uint8_t*
IMetadata::Memory::array() const
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    return this->_data->data();
}


uint8_t*
IMetadata::Memory::editArray()
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    dup_data_locked();
    return this->_data->data();
}


uint8_t
IMetadata::Memory::itemAt(size_t index) const
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    uint8_t val = this->_data->at(index);
    return val;
}


void
IMetadata::Memory::clear()
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(this->_data_lock);
#endif
    this->_data->clear();
}


IMetadata::Memory::Memory()
{
    this->_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
#if 0
    // error check
    if (__builtin_expect( this->_data.get() == nullptr, false )) {
        assert(0);
    }
#endif
}


IMetadata::Memory::Memory(const Memory &other)
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(other._data_lock);
#endif
    this->_data = other._data;
}


IMetadata::Memory::Memory(Memory &&other)
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    std::lock_guard<std::mutex> lk(other._data_lock);
#endif
    this->_data = std::move(other._data);
}


IMetadata::Memory&
IMetadata::Memory::operator = (const Memory &other)
{
    if (__builtin_expect( this == &other, false )) {
        return *this;
    }
    else {
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
        // acquires lockers
        std::lock( this->_data_lock, other._data_lock);
        std::lock_guard<std::mutex> lk1(this->_data_lock, std::adopt_lock);
        std::lock_guard<std::mutex> lk2(other._data_lock, std::adopt_lock);
#endif
        this->_data = other._data;
    }

    return *this;
}


IMetadata::Memory&
IMetadata::Memory::operator = (Memory &&other)
{
    if (__builtin_expect( this == &other, false )) {
        return *this;
    }
    else {
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
        // acquires lockers
        std::lock( this->_data_lock, other._data_lock);
        std::lock_guard<std::mutex> lk1(this->_data_lock, std::adopt_lock);
        std::lock_guard<std::mutex> lk2(other._data_lock, std::adopt_lock);
#endif
        this->_data = std::move(other._data);
    }

    return *this;
}


bool
IMetadata::Memory::operator == (const Memory &other) const
{
#if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
    // acquires lockers
    std::lock( this->_data_lock, other._data_lock);
    std::lock_guard<std::mutex> lk1(this->_data_lock, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(other._data_lock, std::adopt_lock);
#endif
    const size_t dsize1 = this->_data->size();
    const size_t dsize2 = other._data->size();
    if (dsize1 != dsize2) {
        return false;
    }

    return 0 == std::memcmp(this->_data->data(), other._data->data(), dsize1);
}

bool
IMetadata::Memory::operator != (const Memory &other) const
{
    return !(this->operator == (other));
}


void
IMetadata::Memory::dup_data_locked()
{
    // check if unique, if yes, no need to dup
    if (this->_data.use_count() == 1) {
        return;
    }

    // query size, it might be 0, but it's okay.
    const size_t dataSize = this->_data->size();

    // create a shared_ptr contains std::vector<uint8_t>
    auto spData = std::make_shared<std::vector<uint8_t>>(
            std::vector<uint8_t>(dataSize)
            );

    // copy data if necessary (dataSize may be 0, but it's okay)
    std::memcpy(spData->data(), this->_data->data(), dataSize);

    // owns the unique data
    this->_data = spData;
}


namespace IEntryStorage {
// operation wrapper of android::Vector
struct ops
{
    virtual size_t       _size(const void*)                        { return 0; };
    virtual void         _clear(void*)                             {};
    virtual void*        _new()                                    { return nullptr; };
    virtual void         _delete(void*)                            {};
    virtual const void*  _item_at(void*, size_t)                   { return nullptr; };
    virtual void         _push_back(void*, const void*)            {};
    virtual void         _replace_at(void *, size_t, const void*)  {};
    virtual size_t       _remove_at(void *, size_t)                { return 0; };
    virtual void         _copy(void *, void *)                     {};
    virtual bool         _isempty(void *)                          { return true; };
    virtual size_t       _capacity(void *)                         { return 0; };
    virtual size_t       _set_capacity(void *, size_t)             { return 0; };
    virtual const void*  _array(void *)                            { return nullptr; };

    virtual ~ops() = default;
};

template<class T>
struct opsImp : public ops
{
    typedef android::Vector<T> Container_T;

    virtual size_t _size(const void *ptr)
    {
        return static_cast<const Container_T*>(ptr)->size();
    };

    virtual void _clear(void *ptr)
    {
        static_cast<Container_T*>(ptr)->clear();
    };

    virtual void* _new()
    {
        return new Container_T;
    };

    virtual void _delete(void *ptr)
    {
        delete static_cast<Container_T*>(ptr);
    };

    virtual const void* _item_at(void *ptr, size_t index)
    {
        return static_cast<const void*>(&(static_cast<Container_T*>(ptr)->itemAt(index)));
    };

    virtual void _push_back(void *ptr, const void *item)
    {
        static_cast<Container_T*>(ptr)->push_back(*static_cast<const T*>(item));
    };

    virtual void _replace_at(void *ptr, size_t index, const void *item)
    {
        static_cast<Container_T*>(ptr)->editItemAt(index) = *static_cast<const T*>(item);
    };

    virtual size_t _remove_at(void *ptr, size_t index)
    {
        return static_cast<Container_T*>(ptr)->removeAt(index);
    };

    virtual void _copy(void *lhs, void *rhs)
    {
        *static_cast<Container_T*>(lhs) = *static_cast<Container_T*>(rhs);
    };

    virtual bool _isempty(void *ptr)
    {
        return static_cast<Container_T*>(ptr)->isEmpty();
    };

    virtual size_t _capacity(void *ptr)
    {
        return static_cast<Container_T*>(ptr)->capacity();
    };

    virtual size_t _set_capacity(void *ptr, size_t size)
    {
        return static_cast<Container_T*>(ptr)->setCapacity(size);
    };

    virtual const void* _array(void *ptr)
    {
        return static_cast<const void*>(static_cast<Container_T*>(ptr)->array());
    };

    virtual ~opsImp() = default;
};


// bind operators
static struct ops *operators[] = {
    new opsImp< MUINT8     >,
    new opsImp< MINT32     >,
    new opsImp< MFLOAT     >,
    new opsImp< MINT64     >,
    new opsImp< MDOUBLE    >,
    new opsImp< MRational  >,
    new opsImp< MPoint     >,
    new opsImp< MSize      >,
    new opsImp< MRect      >,
    new opsImp< IMetadata  >,
    new opsImp< IMetadata::Memory >,

    // default ops, do nothing
    new ops
};


}; // namespace IEntryStorage


/******************************************************************************
 *
 ******************************************************************************/
#define IMETADATA_IENTRY_CHECK_TYPE_ID  1
class IMetadata::IEntry::Implementor
{
private:
    Tag_t                           mTag;
    volatile int                    mType;
    mutable std::mutex              mResourceMtx;

public:
    void*                           mStoragePtr;

public:     ////                    Instantiation.
    virtual                         ~Implementor();
                                    Implementor(Tag_t tag);
    Implementor&                    operator=(Implementor const& other);
                                    Implementor(Implementor const& other);

public:     ////                    Accessors.

    /**
     * Return the tag.
     */
    virtual MUINT32                 tag() const;

    /**
     * Return the type.
     */
    virtual MINT32                  type() const;

    /**
     * Return the start address of IEntry's container.
     *  @note The memory chunk of container is always continuous.
     */
    virtual const void*             data() const;

    /**
     * Check to see whether it is empty (no items) or not.
     */
    virtual MBOOL                   isEmpty() const;

    /**
     * Return the number of items.
     */
    virtual MUINT                   count() const;

    /**
     * Return how many items can be stored without reallocating the backing store.
     */
    virtual MUINT                   capacity() const;

    /**
     * Set the capacity.
     */
    virtual MBOOL                   setCapacity(MUINT size);

    /**
     * Return the storageOps, to manipulate mStoragePtr.
     */
    inline virtual IEntryStorage::ops*   StorageOps() const
    {
        if(mType != -1)
            return IEntryStorage::operators[mType];
        else
            return IEntryStorage::operators[NUM_MTYPES];
    }

public:     ////                    Operations.

    /**
     * Clear all items.
     * Note: Copy-on write.
     */
    virtual MVOID                   clear();

    /**
     * Delete an item at a given index.
     * Note: Copy-on write.
     */
    virtual MERROR                  removeAt(MUINT index);


#define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
    virtual MVOID                   push_back(_T const& item, Type2Type<_T>) \
                                    {  \
                                        std::lock_guard<std::mutex> lk(mResourceMtx); \
                                        mType = TYPE_##_T;\
                                        try_init_storage(TYPE_##_T); \
                                        this->StorageOps()->_push_back(mStoragePtr, &item); \
                                    } \
    virtual MVOID                   replaceItemAt(MUINT index, _T const& item, Type2Type<_T>) \
                                    { \
                                        std::lock_guard<std::mutex> lk(mResourceMtx); \
                                        guarantee_type_id(TYPE_##_T); \
                                        this->StorageOps()->_replace_at(mStoragePtr, index, &item);\
                                    } \
    virtual _T                      itemAt(MUINT index, Type2Type<_T>) const { \
                                        std::lock_guard<std::mutex> lk(mResourceMtx); \
                                        return *static_cast<const _T*>(this->StorageOps()->_item_at(mStoragePtr, index)); \
                                    }

    IMETADATA_IENTRY_OPS_DECLARATION(MUINT8)
    IMETADATA_IENTRY_OPS_DECLARATION(MINT32)
    IMETADATA_IENTRY_OPS_DECLARATION(MFLOAT)
    IMETADATA_IENTRY_OPS_DECLARATION(MINT64)
    IMETADATA_IENTRY_OPS_DECLARATION(MDOUBLE)
    IMETADATA_IENTRY_OPS_DECLARATION(MRational)
    IMETADATA_IENTRY_OPS_DECLARATION(MPoint)
    IMETADATA_IENTRY_OPS_DECLARATION(MSize)
    IMETADATA_IENTRY_OPS_DECLARATION(MRect)
    IMETADATA_IENTRY_OPS_DECLARATION(IMetadata)
    IMETADATA_IENTRY_OPS_DECLARATION(Memory)

#undef  IMETADATA_IENTRY_OPS_DECLARATION

private:

#if IMETADATA_IENTRY_CHECK_TYPE_ID
    inline void guarantee_type_id(int typeId)
    {
        if (__builtin_expect( mType != typeId, false )) {
            CAM_LOGE("unexpected type ID, (lhs, rhs)=(%d, %d)", mType, typeId);
            *(volatile uint32_t*)(0x00000000) = mType | (typeId << 16);
            __builtin_unreachable();
        }
    }
#else
    #define guarantee_type_id(x) do {} while(0)
#endif

    void try_init_storage(int typeId)
    {
        if (mStoragePtr)
            return;
        else {
            guarantee_type_id(typeId);
            mStoragePtr  = this->StorageOps()->_new();
        }
    }
};


/******************************************************************************
 *
 ******************************************************************************/
IMetadata::IEntry::Implementor::
Implementor(Tag_t tag)
    : mTag(tag)
    , mType(-1)
    , mStoragePtr(nullptr)
{
}


IMetadata::IEntry::Implementor::
Implementor(IMetadata::IEntry::Implementor const& other)
    : mTag(other.mTag)
    , mType(other.mType)
    , mStoragePtr(nullptr)
{
    *this = other;
}


IMetadata::IEntry::Implementor&
IMetadata::IEntry::Implementor::
operator=(IMetadata::IEntry::Implementor const& other)
{
    if (this != &other)
    {
        std::lock(mResourceMtx, other.mResourceMtx);
        std::lock_guard<std::mutex>(mResourceMtx,       std::adopt_lock);
        std::lock_guard<std::mutex>(other.mResourceMtx, std::adopt_lock);
        mTag = other.mTag;
        mType = other.mType;
        // release self's storage first
        this->StorageOps()->_delete(mStoragePtr);
        // create self's storage by other's operator
        mStoragePtr = other.StorageOps()->_new();
        // use other's ops to copy storage
        other.StorageOps()->_copy(mStoragePtr, other.mStoragePtr);
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


IMetadata::IEntry::Implementor::
~Implementor()
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    this->StorageOps()->_delete(mStoragePtr);
}


MUINT32
IMetadata::IEntry::Implementor::
tag() const
{
    return mTag;
}

MINT32
IMetadata::IEntry::Implementor::
type() const
{
    return mType;
}

const void*
IMetadata::IEntry::Implementor::
data() const
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    return get_mtk_metadata_tag_type(mTag) == -1 ?
        nullptr : this->StorageOps()->_array(mStoragePtr);
}

MBOOL
IMetadata::IEntry::Implementor::
isEmpty() const
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    return get_mtk_metadata_tag_type(mTag) == -1 ?
        MTRUE : this->StorageOps()->_isempty(mStoragePtr);
}


MUINT
IMetadata::IEntry::Implementor::
count() const
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    return this->StorageOps()->_size(mStoragePtr);
}


MUINT
IMetadata::IEntry::Implementor::
capacity() const
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    return get_mtk_metadata_tag_type(mTag) == -1 ?
        0 : this->StorageOps()->_capacity(mStoragePtr);
}


MBOOL
IMetadata::IEntry::Implementor::
setCapacity(MUINT size)
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    MERROR ret = get_mtk_metadata_tag_type(mTag) == -1 ?
        MFALSE : this->StorageOps()->_set_capacity(mStoragePtr, size);
    return ret == NO_MEMORY ? MFALSE : ret;
}


MVOID
IMetadata::IEntry::Implementor::
clear()
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    if (get_mtk_metadata_tag_type(mTag) != -1)
        return this->StorageOps()->_clear(mStoragePtr);
}


MERROR
IMetadata::IEntry::Implementor::
removeAt(MUINT index)
{
    std::lock_guard<std::mutex> lk1(mResourceMtx);
    MERROR ret = get_mtk_metadata_tag_type(mTag) == -1 ?
                  BAD_VALUE : this->StorageOps()->_remove_at(mStoragePtr, index);
    return ret == BAD_VALUE ? BAD_VALUE : OK;
}


/******************************************************************************
 *
 ******************************************************************************/
#define AEE_IF_TAG_ERROR(_TAG_) \
    if (_TAG_ == (uint32_t)-1) \
    { \
        CAM_LOGE("tag(%d) error", _TAG_); \
        Utils::dumpCallStack(__FUNCTION__); \
        AEE_ASSERT("tag error"); \
    }

IMetadata::IEntry::
IEntry(Tag_t tag)
    : mpImp(new Implementor(tag))
{
}


IMetadata::IEntry::
    IEntry(IMetadata::IEntry const& other)
{
    std::lock_guard< std::mutex > lk1(other.mEntryLock);
    mpImp = new Implementor(*(other.mpImp));
}


IMetadata::IEntry::
~IEntry()
{
    std::lock_guard< std::mutex > lk1(mEntryLock);
    if(mpImp) {
        delete mpImp;
        mpImp = nullptr;
    }
}


IMetadata::IEntry&
IMetadata::IEntry::
operator=(IMetadata::IEntry const& other)
{
    if (this != &other) {
        std::lock(mEntryLock, other.mEntryLock);
        std::lock_guard<std::mutex> lk1(mEntryLock,         std::adopt_lock);
        std::lock_guard<std::mutex> lk2(other.mEntryLock,   std::adopt_lock);
        delete mpImp;
        mpImp = new Implementor(*(other.mpImp));
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


MUINT32
IMetadata::IEntry::
tag() const
{
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->tag();
}

MINT32
IMetadata::IEntry::
type() const
{
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->type();
}

const void*
IMetadata::IEntry::
data() const
{
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->data();
}

MBOOL
IMetadata::IEntry::
isEmpty() const
{
    std::lock_guard<std::mutex> lk1(mEntryLock);
    //AEE_IF_TAG_ERROR(tag())
    return mpImp->isEmpty();
}


MUINT
IMetadata::IEntry::
count() const
{
    std::lock_guard<std::mutex> lk1(mEntryLock);
    //AEE_IF_TAG_ERROR(tag())
    return mpImp->count();
}


MUINT
IMetadata::IEntry::
capacity() const
{
    AEE_IF_TAG_ERROR(tag())
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->capacity();
}


MBOOL
IMetadata::IEntry::
setCapacity(MUINT size)
{
    AEE_IF_TAG_ERROR(tag())
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->setCapacity(size);
}


MVOID
IMetadata::IEntry::
clear()
{
    AEE_IF_TAG_ERROR(tag())
    std::lock_guard<std::mutex> lk1(mEntryLock);
    mpImp->clear();
}


MERROR
IMetadata::IEntry::
removeAt(MUINT index)
{
    AEE_IF_TAG_ERROR(tag())
    std::lock_guard<std::mutex> lk1(mEntryLock);
    return mpImp->removeAt(index);
}


#define ASSERT_CHECK(_defaultT, _T) \
      CAM_LOGE_IF( TYPE_##_T != _defaultT, "tag(%x), type(%d) should be (%d)", tag(), TYPE_##_T, _defaultT); \
      if (TYPE_##_T != _defaultT) { \
          Utils::dumpCallStack(); \
          AEE_ASSERT("type mismatch"); \
      }
#undef  ASSERT_CHECK

#define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
MVOID \
IMetadata::IEntry:: \
push_back(_T const& item, Type2Type<_T> type) \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    std::lock_guard<std::mutex> lk1(mEntryLock); \
    mpImp->push_back(item, type); \
} \
MVOID \
IMetadata::IEntry:: \
replaceItemAt(MUINT index, _T const& item, Type2Type<_T> type) \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    std::lock_guard<std::mutex> lk1(mEntryLock); \
    mpImp->replaceItemAt(index, item, type);\
} \
_T \
IMetadata::IEntry:: \
itemAt(MUINT index, Type2Type<_T> type) const \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    std::lock_guard<std::mutex> lk1(mEntryLock); \
    return mpImp->itemAt(index, type); \
}

IMETADATA_IENTRY_OPS_DECLARATION(MUINT8)
IMETADATA_IENTRY_OPS_DECLARATION(MINT32)
IMETADATA_IENTRY_OPS_DECLARATION(MFLOAT)
IMETADATA_IENTRY_OPS_DECLARATION(MINT64)
IMETADATA_IENTRY_OPS_DECLARATION(MDOUBLE)
IMETADATA_IENTRY_OPS_DECLARATION(MRational)
IMETADATA_IENTRY_OPS_DECLARATION(MPoint)
IMETADATA_IENTRY_OPS_DECLARATION(MSize)
IMETADATA_IENTRY_OPS_DECLARATION(MRect)
IMETADATA_IENTRY_OPS_DECLARATION(IMetadata)
IMETADATA_IENTRY_OPS_DECLARATION(IMetadata::Memory)
#undef  IMETADATA_IENTRY_OPS_DECLARATION

#undef  AEE_IF_TAG_ERROR
/******************************************************************************
 *
 ******************************************************************************/
class IMetadata::Implementor
{
public:     ////                        Instantiation.
    virtual                            ~Implementor();
                                        Implementor();
    Implementor&                        operator=(Implementor const& other);
                                        Implementor(Implementor const& other);

public:     ////                        operators
    Implementor&                        operator+=(Implementor const& other);
    Implementor                         operator+(Implementor const& other);

public:     ////                        Accessors.

    /**
     * Check to see whether it is empty (no entries) or not.
     */
    virtual MBOOL                       isEmpty() const;

    /**
     * Return the number of entries.
     */
    virtual MUINT                       count() const;

public:     ////                        Operations.

    /**
     * Clear all entries.
     * Note: Copy-on write.
     */
    virtual MVOID                       clear();

    /**
     * Delete an entry by tag.
     * Note: Copy-on write.
     */
    virtual MERROR                      remove(Tag_t tag);

    /**
     * Sort all entries for faster find.
     * Note: Copy-on write.
     */
    virtual MERROR                      sort();

    /**
     * Update metadata entry. An entry will be created if it doesn't exist already.
     * Note: Copy-on write.
     */
    virtual MERROR                      update(Tag_t tag, IEntry const& entry);

    /**
    * Get metadata entry by tag, with no editing.
    */
    virtual IEntry                      entryFor(Tag_t tag) const;

    /**
     * Get metadata entry by index, with no editing.
     */
    virtual IEntry                      entryAt(MUINT index) const;

    /**
     * Take metadata entry by tag.
     * Complexity: O(log N)
     */
    virtual IEntry                      takeEntryFor(Tag_t tag);

    /**
     * Take metadata entry by index.
     * Complexity: O(1)
     */
    virtual IEntry                      takeEntryAt(MUINT index);

    /**
     * Flatten IMetadata.
     */
    virtual ssize_t                     flatten(char* buf, size_t buf_size) const;

    /**
     * Unflatten IMetadata.
     */
    virtual ssize_t                     unflatten(char* buf, size_t buf_size);

    virtual void                        dump(int layer=0);

protected:
    DefaultKeyedVector<Tag_t, IEntry>   mMap;
    mutable std::mutex                  mResourceMtx;
};


/******************************************************************************
 *
 ******************************************************************************/
IMetadata::Implementor::
Implementor()
    : mMap()
{

}


IMetadata::Implementor::
Implementor(IMetadata::Implementor const& other)
{
    std::lock_guard<std::mutex> lk(other.mResourceMtx);
    mMap = other.mMap;
}


IMetadata::Implementor::
~Implementor()
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    mMap = decltype(mMap)();
}


IMetadata::Implementor&
IMetadata::Implementor::
operator+=(IMetadata::Implementor const& other)
{
    if (this != &other)
    {
        std::lock(mResourceMtx, other.mResourceMtx);
        std::lock_guard<std::mutex>(mResourceMtx,       std::adopt_lock);
        std::lock_guard<std::mutex>(other.mResourceMtx, std::adopt_lock);
        //
        if( mMap.size() >= other.mMap.size() ) {
            for(size_t idx = 0; idx < other.mMap.size(); idx++ )
                mMap.add( other.mMap.keyAt(idx), other.mMap.valueAt(idx) );
        } else {
            auto temp = other.mMap;
            for(size_t idx = 0; idx < mMap.size(); idx++ )
                temp.add( mMap.keyAt(idx), mMap.valueAt(idx) );
            mMap = temp;
        }
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


IMetadata::Implementor
IMetadata::Implementor::
operator+(IMetadata::Implementor const& other)
{
    return Implementor(*this) += other;
}


IMetadata::Implementor&
IMetadata::Implementor::
operator=(IMetadata::Implementor const& other)
{
    if (this != &other)
    {
        std::lock(mResourceMtx, other.mResourceMtx);
        std::lock_guard<std::mutex>(mResourceMtx,       std::adopt_lock);
        std::lock_guard<std::mutex>(other.mResourceMtx, std::adopt_lock);
        //release mMap'storage
        //assign other.mMap's storage pointer to mMap
        //add 1 to storage's sharebuffer
        mMap = other.mMap;
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


MBOOL
IMetadata::Implementor::
isEmpty() const
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.isEmpty();
}


MUINT
IMetadata::Implementor::
count() const
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.size();
}


MVOID
IMetadata::Implementor::
clear()
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    mMap.clear();
}


MERROR
IMetadata::Implementor::
remove(Tag_t tag)
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.removeItem(tag);
}


MERROR
IMetadata::Implementor::
sort()
{

    //keyedVector always sorted.
    return OK;
}


MERROR
IMetadata::Implementor::
update(Tag_t tag, IEntry const& entry)
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.add(tag, entry);

}


IMetadata::IEntry
IMetadata::Implementor::
entryFor(Tag_t tag) const
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.valueFor(tag);
}


IMetadata::IEntry
IMetadata::Implementor::
entryAt(MUINT index) const
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    return mMap.valueAt(index);

}


IMetadata::IEntry
IMetadata::Implementor::
takeEntryFor(Tag_t tag)
{
    IMetadata::IEntry entry;
    //
    std::lock_guard<std::mutex> lk(mResourceMtx);
    //
    ssize_t index = mMap.indexOfKey(tag); // O(log N)
    if (__builtin_expect( index >= 0, true )) {
        entry = mMap.valueAt(index); // O(1), copy-on-write
        mMap.removeItemsAt(index);   // O(1)
    }
    return entry;
}


IMetadata::IEntry
IMetadata::Implementor::
takeEntryAt(MUINT index)
{
    std::lock_guard<std::mutex> lk(mResourceMtx);
    IMetadata::IEntry entry = mMap.valueAt(index); // O(1)
    mMap.removeItemsAt(index); // O(1)
    return entry;
}

#define DUMP_METADATA_STRING(_layer_, _msg_) \
        CAM_LOGD("(L%d) %s", _layer_, _msg_.string());


void
IMetadata::Implementor::
dump(int layer)
{
    for (size_t i = 0; i < mMap.size(); ++i) {
        IMetadata::IEntry entry = mMap.valueAt(i);
        android::String8 msg = String8::format( "[%s] Map(%zu/%zu) tag(0x%x) type(%d) count(%d) ",
            __FUNCTION__, i, mMap.size(), entry.tag(), entry.type(), entry.count());
        //
        if ( TYPE_IMetadata == entry.type() ) {
            for( MUINT j = 0; j < entry.count(); ++j ) {
                IMetadata meta = entry.itemAt(j, Type2Type< IMetadata >());
                msg += String8::format("metadata.. ");
                DUMP_METADATA_STRING(layer, msg);
                meta.dump(layer+1);
            }
        } else {
            switch( entry.type() )
            {
            case TYPE_MUINT8:
                for (MUINT j=0; j<entry.count(); j++)
                    msg += String8::format("%d ", entry.itemAt(j, Type2Type< MUINT8 >() ));
                break;
            case TYPE_MINT32:
                for (MUINT j=0; j<entry.count(); j++)
                    msg += String8::format("%d ", entry.itemAt(j, Type2Type< MINT32 >() ));
                break;
            case TYPE_MINT64:
                for (MUINT j=0; j<entry.count(); j++)
                    msg += String8::format("%" PRId64 " ", entry.itemAt(j, Type2Type< MINT64 >() ));
                break;
            case TYPE_MFLOAT:
                for (MUINT j=0; j<entry.count(); j++)
                    msg += String8::format("%f ", entry.itemAt(j, Type2Type< MFLOAT >() ));
                break;
            case TYPE_MDOUBLE:
                for (MUINT j=0; j<entry.count(); j++)
                    msg += String8::format("%lf ", entry.itemAt(j, Type2Type< MDOUBLE >() ));
                break;
            case TYPE_MSize:
                for (MUINT j=0; j<entry.count(); j++)
                {
                    MSize src_size = entry.itemAt(j, Type2Type< MSize >());
                    msg += String8::format( "size(%d,%d) ", src_size.w, src_size.h );
                }
                break;
            case TYPE_MRect:
                for (MUINT j=0; j<entry.count(); j++)
                {
                    MRect src_rect = entry.itemAt(j, Type2Type< MRect >());
                    msg += String8::format( "rect(%d,%d,%d,%d) ",
                                           src_rect.p.x, src_rect.p.y,
                                           src_rect.s.w, src_rect.s.h );
                }
                break;
            case TYPE_MPoint:
                for (MUINT j=0; j<entry.count(); j++)
                {
                    MPoint src_point = entry.itemAt(j, Type2Type< MPoint >());
                    msg += String8::format( "point(%d,%d) ", src_point.x, src_point.y );
                }
                break;
            case TYPE_MRational:
                for (MUINT j=0; j<entry.count(); j++)
                {
                    MRational src_rational = entry.itemAt(j, Type2Type< MRational >());
                    msg += String8::format( "rational(%d,%d) ", src_rational.numerator, src_rational.denominator );
                }
                break;
            case TYPE_Memory:
                msg += String8::format("Memory type: not dump!");
                break;
            default:
                msg += String8::format("unsupported type(%d)", entry.type());
            }
        }
    DUMP_METADATA_STRING(layer, msg);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
#define _DEBUG_TIME_ESTIMATE_ 0
#if _DEBUG_TIME_ESTIMATE_
static int sFlattenLayer = 0;
static int sUnflattenLayer = 0;
#define CAM_LOGD_TIME(fmt, arg...)     CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#endif

struct metadata_buffer_entry_info {
    MUINT32 tag;
    MUINT8 type;
    MUINT count;
};

struct metadata_buffer_handle {

    metadata_buffer_handle(char* b, size_t s)
        :buffer(b),
         size(s),
         offset(0)
    {}

    MERROR get_entry_count(MUINT32 &entry_count)
    {
        align(sizeof(entry_count));

        entry_count = *((MUINT32*)current());
        move(sizeof(entry_count));

        return OK;
    }

    MERROR set_entry_count(MUINT32 entry_count)
    {
        align(sizeof(entry_count));

        *((MUINT32*)current()) = entry_count;
        move(sizeof(entry_count));

        return OK;
    }

    inline MERROR get_entry_info(metadata_buffer_entry_info& info)
    {
        align(alignof(metadata_buffer_entry_info));

        if (!check_size(sizeof(metadata_buffer_entry_info))) {
            CAM_LOGE("[get_entry_info] out of boundary");
            return NO_MEMORY;
        }

        memcpy(&info, current(), sizeof(metadata_buffer_entry_info));
        move(sizeof(metadata_buffer_entry_info));

        //CAM_LOGD("[get_info] tag:%d type:%d count:%d offset:%zu", info.tag, info.type, info.count, offset);
        return OK;
    }

    inline MERROR set_entry_info(metadata_buffer_entry_info& info)
    {
        align(alignof(metadata_buffer_entry_info));

        if (!check_size(sizeof(metadata_buffer_entry_info))) {
            CAM_LOGE("[set_entry_info] out of boundary");
            return NO_MEMORY;
        }

        memcpy(current(), &info, sizeof(metadata_buffer_entry_info));
        move(sizeof(metadata_buffer_entry_info));

        //CAM_LOGD("[set_info] tag:%d type:%d count:%d offset:%zu", info.tag, info.type, info.count, offset);
        return OK;
    }

    inline MBOOL check_size(size_t data_size)
    {
        if (size < offset + data_size) {
            CAM_LOGE("memory not enough, size=%zu, offset=%zu, data=%zu",
                    size, offset, data_size);
            return MFALSE;
        }
        return MTRUE;
    }

    inline MVOID align(MUINT8 alignment)
    {
        offset += alignment - ((uintptr_t)current() % alignment);
    }

    inline char* current()
    {
        return buffer + offset;
    }

    inline size_t move(size_t m)
    {
        offset += m;
        return offset;
    }

    char* buffer;
    size_t size;
    size_t offset;
};

template<class T>
static MERROR
write_to_buffer(metadata_buffer_handle& handle, const IMetadata::IEntry& entry)
{
    if (!handle.check_size(sizeof(T) * entry.count()))
        return NO_MEMORY;

    for(MUINT i = 0; i < entry.count(); i++) {
        const T t = entry.itemAt(i, Type2Type<T>());
        *((T*)handle.current()) = t;
        handle.move(sizeof(T));
    }

    return OK;
}

template<class T>
static MERROR
read_from_buffer(metadata_buffer_handle& handle, IMetadata::IEntry& entry, MUINT count)
{
    if (!handle.check_size(sizeof(T) * count))
        return NO_MEMORY;

    entry.setCapacity(count);

    T *p;
    for(MUINT i = 0; i < count; i++) {
        p = (T*)handle.current();
        entry.push_back(*p, Type2Type<T>());
        handle.move(sizeof(T));
    }

    return OK;
}

template<>
MERROR
write_to_buffer<IMetadata::Memory>(metadata_buffer_handle& handle, const IMetadata::IEntry& entry)
{
    for(MUINT i = 0; i < entry.count(); i++) {
        const IMetadata::Memory m = entry.itemAt(i, Type2Type<IMetadata::Memory>());

        if (i)
            handle.align(sizeof(MUINT32));

        if (!handle.check_size(sizeof(MUINT32) + sizeof(MUINT8) * m.size()))
            return NO_MEMORY;

        MUINT32 s = m.size();
        *((MUINT32*)handle.current()) = s;
        handle.move(sizeof(s));
        memcpy(handle.current(), m.array(), m.size());
        handle.move(sizeof(MUINT8) * m.size());
    }

    return OK;
}

template<>
MERROR
read_from_buffer<IMetadata::Memory>(metadata_buffer_handle& handle, IMetadata::IEntry& entry, MUINT count)
{
    MUINT32 s;
    for (MUINT i = 0; i < count; i++) {
        if (i)
            handle.align(sizeof(MUINT32));

        if (!handle.check_size(sizeof(MUINT32)))
            return NO_MEMORY;

        s = *((MUINT32*)handle.current());
        handle.move(sizeof(MUINT32));

        if (!handle.check_size(sizeof(MUINT8) * s))
            return NO_MEMORY;

        IMetadata::Memory mMemory;
        mMemory.resize(s);
        memcpy(mMemory.editArray(), handle.current(), sizeof(MUINT8) * s);
        handle.move(s);

        entry.push_back(mMemory, Type2Type<IMetadata::Memory>());
     }

    return OK;
}

template<>
MERROR
write_to_buffer<IMetadata>(metadata_buffer_handle& handle, const IMetadata::IEntry& entry)
{
    for(MUINT i = 0; i < entry.count(); i++) {
        const IMetadata m = entry.itemAt(i, Type2Type<IMetadata>());
        handle.offset += m.flatten(handle.buffer + handle.offset, handle.size - handle.offset);
    }

    return OK;
}

template<>
MERROR
read_from_buffer<IMetadata>(metadata_buffer_handle& handle, IMetadata::IEntry& entry, MUINT count)
{
    for (MUINT i = 0; i < count; i++) {
        IMetadata meta;
        handle.offset += meta.unflatten(handle.buffer + handle.offset, handle.size - handle.offset);
        entry.push_back(meta, Type2Type<IMetadata>());
     }
    return OK;
}


ssize_t
IMetadata::Implementor::
flatten(char* buf, size_t buf_size) const
{
#if _DEBUG_TIME_ESTIMATE_
    MINT64 start_time = static_cast<MINT64>(::systemTime());
    CAM_LOGD_TIME("layer:%d", ++sFlattenLayer);
#endif
    //
    std::lock_guard<std::mutex> lk(mResourceMtx);
    //
    int ret = 0;
    metadata_buffer_handle handle(buf, buf_size);
    metadata_buffer_entry_info info;
    handle.set_entry_count(mMap.size());


    for(size_t i = 0; i < mMap.size(); ++i) {
        const IMetadata::IEntry& entry = mMap.valueAt(i);

        info.tag = entry.tag();
        info.type = entry.type();
        info.count = entry.count();
        handle.set_entry_info(info);

        typedef com::mediatek::campostalgo::NSFeaturePipe::IMetadata IMetadata;
        switch (info.type) {

#define CASE_WRITER(_type_)                                         \
            case TYPE_##_type_ :                                    \
                ret = write_to_buffer<_type_>(handle, entry);       \
                break;

            CASE_WRITER(MUINT8);
            CASE_WRITER(MINT32);
            CASE_WRITER(MFLOAT);
            CASE_WRITER(MINT64);
            CASE_WRITER(MDOUBLE);
            CASE_WRITER(MRational);
            CASE_WRITER(MPoint);
            CASE_WRITER(MSize);
            CASE_WRITER(MRect);
            CASE_WRITER(IMetadata);
            CASE_WRITER(Memory);
#undef CASE_WRITER
            default:
                CAM_LOGE("[flatten] unsupported format:%ul", info.type);
                ret = BAD_VALUE;
                continue;
        }

        if (ret < 0) return ret;
    }

#if _DEBUG_TIME_ESTIMATE_
    MINT64 end_time = static_cast<MINT64>(::systemTime());
    CAM_LOGD_TIME("layer:%d offset:%zu time:%" PRIu64 "us",
            sFlattenLayer--, handle.offset, (end_time - start_time) / 1000);
#endif

    return handle.offset;
}


ssize_t
IMetadata::Implementor::
unflatten(char* buf, size_t buf_size)
{
#if _DEBUG_TIME_ESTIMATE_
    MINT64 start_time = static_cast<MINT64>(::systemTime());
    CAM_LOGD_TIME("layer:%d", ++sUnflattenLayer);
#endif
    //
    std::lock_guard<std::mutex> lk(mResourceMtx);
    //
    int ret = 0;
    metadata_buffer_handle handle(buf, buf_size);
    metadata_buffer_entry_info info;
    MUINT32 entry_count;
    handle.get_entry_count(entry_count);
    mMap.setCapacity(entry_count);

    for (MUINT32 i = 0; i < entry_count; i++) {
        handle.get_entry_info(info);
        IMetadata::IEntry entry(info.tag);

        typedef com::mediatek::campostalgo::NSFeaturePipe::IMetadata IMetadata;
        switch (info.type) {

#define CASE_READER(_type_)                                                 \
            case TYPE_##_type_ :                                            \
                ret = read_from_buffer<_type_>(handle, entry, info.count);  \
                break;

            CASE_READER(MUINT8);
            CASE_READER(MINT32);
            CASE_READER(MFLOAT);
            CASE_READER(MINT64);
            CASE_READER(MDOUBLE);
            CASE_READER(MRational);
            CASE_READER(MPoint);
            CASE_READER(MSize);
            CASE_READER(MRect);
            CASE_READER(IMetadata);
            CASE_READER(Memory);
#undef CASE_READER
            default:
                CAM_LOGE("[unflatten] unsupported format:%ul", info.type);
                ret = BAD_VALUE;
                continue;
        }

        if (ret < 0) return ret;

        mMap.add(info.tag, entry);
    }
#if _DEBUG_TIME_ESTIMATE_
    MINT64 end_time = static_cast<MINT64>(::systemTime());
    CAM_LOGD_TIME("layer:%d offset:%zu time:%" PRIu64 "us",
            sUnflattenLayer--, handle.offset, (end_time - start_time) / 1000);
#endif

    return handle.offset;
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadata::
IMetadata()
    : mpImp(new Implementor())
{
#if IMETADATA_DEBUG_DOUBLE_FREE_DETECT
    metadata_debug_new_life(reinterpret_cast<uintptr_t>(this));
#endif
}


IMetadata::IMetadata(IMetadata const& other)
{
#if IMETADATA_DEBUG_DOUBLE_FREE_DETECT
    metadata_debug_new_life(reinterpret_cast<uintptr_t>(this));
#endif
    std::lock_guard<std::mutex> lk1(other.mMetadataLock);
    mpImp = new Implementor(*(other.mpImp));
}


IMetadata::
~IMetadata()
{
#if IMETADATA_DEBUG_DOUBLE_FREE_DETECT
    const bool is_alive = metadata_debug_is_alive(reinterpret_cast<uintptr_t>(this));
    if (__builtin_expect( is_alive, false )) {
        CAM_LOGE("double free at %p", this);
        *(volatile uintptr_t*)(0x0) = 0xBADC0DE0;
        __builtin_unreachable();
    }
#endif
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    if(mpImp) {
        delete mpImp;
        mpImp = nullptr;
    }
}


IMetadata&
IMetadata::operator=(IMetadata const& other)
{
    if (this != &other) {
        std::lock(mMetadataLock, other.mMetadataLock);
        std::lock_guard<std::mutex> lk1(mMetadataLock,          std::adopt_lock);
        std::lock_guard<std::mutex> lk2(other.mMetadataLock,    std::adopt_lock);
        //
        delete mpImp;
        mpImp = new Implementor(*(other.mpImp));
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


IMetadata&
IMetadata::operator+=(IMetadata const& other)
{
    std::lock(mMetadataLock, other.mMetadataLock);
    std::lock_guard<std::mutex> lk1(mMetadataLock,          std::adopt_lock);
    std::lock_guard<std::mutex> lk2(other.mMetadataLock,    std::adopt_lock);
    //
    *mpImp += *other.mpImp;
    return *this;
}


IMetadata
IMetadata::operator+(IMetadata const& other)
{
    return IMetadata(*this) += other;
}


MBOOL
IMetadata::
isEmpty() const
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->isEmpty();
}


MUINT
IMetadata::
count() const
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->count();
}


MVOID
IMetadata::
clear()
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    mpImp->clear();
}


MERROR
IMetadata::
remove(Tag_t tag)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->remove(tag) >= 0 ? OK : BAD_VALUE;
}


MERROR
IMetadata::
sort()
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->sort();
}


MERROR
IMetadata::
update(Tag_t tag, IMetadata::IEntry const& entry)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    MERROR ret = mpImp->update(tag, entry);  //keyedVector has two possibilities: BAD_VALUE/NO_MEMORY
    return ret >= 0 ? (MERROR)OK : (MERROR)ret;
}


IMetadata::IEntry
IMetadata::
entryFor(Tag_t tag) const
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->entryFor(tag);
}


IMetadata::IEntry
IMetadata::
entryAt(MUINT index) const
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->entryAt(index);
}


IMetadata::IEntry
IMetadata::
takeEntryFor(Tag_t tag)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->takeEntryFor(tag);
}


IMetadata::IEntry
IMetadata::
takeEntryAt(MUINT index)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->takeEntryAt(index);
}


ssize_t
IMetadata::
flatten(void* buf, size_t buf_size) const
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->flatten(static_cast<char*>(buf), buf_size);
}

ssize_t
IMetadata::
unflatten(void* buf, size_t buf_size)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    return mpImp->unflatten(static_cast<char*>(buf), buf_size);
}

void IMetadata::
dump(int layer)
{
    std::lock_guard<std::mutex> lk1(mMetadataLock);
    mpImp->dump(layer);
}
