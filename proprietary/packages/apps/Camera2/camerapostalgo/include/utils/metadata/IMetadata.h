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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_H_
//
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include <utils/def/BuiltinTypes.h>
#include <utils/def/BasicTypes.h>
#include <utils/def/UITypes.h>
#include <utils/def/TypeManip.h>
//
#include <mutex>
#include <vector>
#include <memory>
#include <utility>
//
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

/******************************************************************************
 *  Camera Metadata Interface
 ******************************************************************************/
class IMetadata
{

public:
    // IMetadata::Memory is a buffer chunk module that:
    //
    // 1. dynamic allocation (using heap).
    // 2. continuous, can copy all data chunk via retrieving raw pointers from
    //    array() or editArray(), notice that, array() and editArray() are not
    //    thread-safe method.
    // 3. copy-on-write, caller calls methods whichs are not marked as const,
    //    the internal buffer will be copied immediately.
    // 4. reentraint.
    //
    // optional: thread-safe, except method array() and editArray().
    //           if programmer wants thread-safe support, define
    //           IMETADATA_MEMORY_THREAD_SAFE_SUPPORT to 1
    #define IMETADATA_MEMORY_THREAD_SAFE_SUPPORT   1
    class Memory
    {
    // interfaces
    public:
        size_t                      size() const;
        void                        resize(const size_t size, uint8_t default_val = 0);
        size_t                      append(const Memory &other);
        const uint8_t*              array() const;
        uint8_t*                    editArray();
        uint8_t                     itemAt(size_t index) const;
        void                        clear();

        // android support
        size_t                      appendVector(const Memory &other);
        size_t                      appendVector(const android::Vector<MUINT8> &v);


    public:
        Memory();
        Memory(const Memory &other);
        Memory(Memory &&other);

    public:
        Memory& operator = (Memory &&other);
        Memory& operator = (const Memory &other);
        bool operator == (const Memory &other) const;
        bool operator != (const Memory &other) const;

    private:
        // try to duplicate shared data into an unique new one. if the _data is
        // unique, this function do nothing.
        void dup_data_locked();

    // attributes
    private:
        // we shared _data if without editing Memory (copy-on-write)
        mutable std::shared_ptr< std::vector<uint8_t> > _data;
        #if IMETADATA_MEMORY_THREAD_SAFE_SUPPORT
        mutable std::mutex                              _data_lock;
        #endif
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MUINT32                     Tag_t;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Entry Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    class   IEntry
    {
    public:     ////                    Definitions.
        enum { BAD_TAG = -1U };

    public:     ////                    Instantiation.
        virtual                         ~IEntry();
                                        IEntry(Tag_t tag = BAD_TAG);

        /**
         * Copy constructor and copy assignment.
         */
                                        IEntry(IEntry const& other);
        IEntry&                         operator=(IEntry const& other);

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
        virtual android::status_t       removeAt(MUINT index);

#define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
        virtual MVOID                   push_back(_T const& item, Type2Type<_T>); \
        virtual MVOID                   replaceItemAt(MUINT index, _T const& item, Type2Type<_T>); \
        virtual _T                      itemAt(MUINT index, Type2Type<_T>) const; \

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

    /**
     * Get index of target value in Entry.
     * If the target value doesn't exist, this function returns -1.
     *
     * @param entry [in]    The input entry for look up
     * @param target [in]   Target value in input entry ( Must cast to entry value supported type, eg MDOUBLE)
     * @return              index of item  if target value found. Otherwise, return -1.
     */
    template <typename T>
    static int indexOf(const IEntry& entry, const T& target)
    {
        for(size_t i = 0 ; i < entry.count() ; i++)
        {
            if( entry.itemAt(i, Type2Type<T>()) == target)
            {
                return (int)i;
            }
        }
        return -1;
    }

    public:  ////                    Implementor.
                                        class Implementor;
    protected:
        Implementor*                    mpImp;
        mutable std::mutex              mEntryLock;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
    virtual                             ~IMetadata();
                                        IMetadata();

    /**
     * Copy constructor and copy assignment.
     */
                                        IMetadata(IMetadata const& other);
    IMetadata&                          operator=(IMetadata const& other);

    /**
     * operators
     */
    IMetadata&                          operator+=(IMetadata const& other);
    IMetadata                           operator+(IMetadata const& other);

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
    virtual android::status_t           remove(Tag_t tag);

    /**
     * Sort all entries for faster find.
     * Note: Copy-on write.
     */
    virtual android::status_t           sort();

    /**
     * Update metadata entry. An entry will be created if it doesn't exist already.
     * Note: Copy-on write.
     */
    virtual android::status_t           update(Tag_t tag, IEntry const& entry);

    /**
     * Get metadata entry by tag, with no editing.
     */
    virtual IEntry                      entryFor(Tag_t tag) const;

    /**
     * Get metadata entry by index, with no editing.
     */
    virtual IEntry                      entryAt(MUINT index) const;

    /**
     * Take metadata entry by tag. After invoked this method, the metadata enty of tag
     * in metadata will be removed.
     * Note: Without all element copy.
     * Complexity: O(log N)
     */
    virtual IEntry                      takeEntryFor(Tag_t tag);

    /**
     * Take metadata entry by index. After invoked this method, the index-th metadata
     * entry will be removed.
     * Note: Without all element copy.
     * Complexity: O(1)
     */
    virtual IEntry                      takeEntryAt(MUINT index);

    /**
     * Flatten IMetadata.
     */
    virtual ssize_t                     flatten(void* buf, size_t buf_size) const;

    /**
     * Unflatten IMetadata.
     */
    virtual ssize_t                     unflatten(void* buf, size_t buf_size);

    virtual void                        dump(int layer=0);

public:     ////                        Helpers.

    /**
     * Set metadata with given tag and value.
     * Add a pair a tag with its value into metadata (and replace the one that is there).
     *
     * @param metadata [in,out]    The metadata to be updated
     * @param tag [in]             The tag to update
     * @param val [in]             The value to update
     * @return                     Entry is set or not
     * @retval                     OK on success
     * @retval                     INVALID_OPERATION if metadata is null
     * @retval                     BAD_INDEX if out of range
     * @retval                     NO_MEMORY if out of memory
     */
    template <typename T>
    static
    android::status_t
    setEntry(IMetadata* metadata, MUINT32 const tag, T const& val)
    {
        if (nullptr == metadata)
            return -EINVAL;//BAD_VALUE

        IMetadata::IEntry entry(tag);
        entry.push_back(val, Type2Type<T>());
        int err = metadata->update(entry.tag(), entry);
        return (err >= 0) ? 0 : err;
    }

    /**
     * Get metadata with given tag and value.
     * If the tag doesn't exist, this function returns false.
     *
     * @param metadata [in]    The constant pointer of IMetadata to look up
     * @param tag [in]         The tag to get
     * @param val [out]        Call by reference output if found
     * @param index [in]       Index of item in Entry you want to look up. Default value is 0.
     * @return                 true if the corresponding entry exists
     */
    template <typename T>
    static
    bool
    getEntry(const IMetadata* metadata, MUINT32 const tag, T& val, size_t index = 0)
    {
        if (nullptr == metadata)
            return false;

        IMetadata::IEntry entry = metadata->entryFor(tag);
        if (entry.count() > index)
        {
            val = entry.itemAt(index, Type2Type<T>());
            return true;
        }

        return false;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Bridge.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Implementor.
                                        class Implementor;
    Implementor*                        mpImp;
    mutable std::mutex                  mMetadataLock;
};


/******************************************************************************
 *
 ******************************************************************************/
} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_H_

