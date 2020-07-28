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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_V2_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_V2_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/BasicTypes.h>
#include <mtkcam/def/UITypes.h>
#include <mtkcam/def/TypeManip.h>
//
#include <mutex>
#include <vector>
#include <memory>
#include <utility>
//

/******************************************************************************
 * Coding conversion
 ******************************************************************************/
// size_t vs ssize_t type :
//          default use size_t. ssize_t only use for return error
//
// android::Vector: will use memcpy without consider
//          constructor/destructor if content is basic c type





/******************************************************************************
 * NSCam
 ******************************************************************************/
namespace NSCam {
/******************************************************************************
 *  NSCam::IMetadata
 ******************************************************************************/
class IMetadata
{
/******************************************************************************
 *  IMetadata forward decaration
 ******************************************************************************/
public:

    class                                   Content; // Entry internal storage
    typedef std::shared_ptr<Content>        ContentSP;

    struct                                  ContentLayout;
    struct                                  ContentRawData;

    class                                   Storage; // metadata internal storage
    typedef std::shared_ptr<Storage>        StorageSP;

    typedef android::Vector<MUINT8>         BasicData;      // for basic type
    typedef std::shared_ptr<BasicData>      BasicDataSP;

    typedef std::vector<StorageSP>          VecStorage;     // for Meta obj
    typedef std::shared_ptr<VecStorage>     VecStorageSP;   // pointer array of pointer

    typedef std::vector<BasicDataSP>        VecBasicData;   // for Memory obj
    typedef std::shared_ptr<VecBasicData>   VecBasicDataSP; // pointer array of pointer

/******************************************************************************
 *  IMetadata Typedefination
 ******************************************************************************/
public:
    typedef MUINT32                     Tag_t;

/******************************************************************************
 *  IMetadata::Content
 ******************************************************************************/
public:
    class Content
    {
        MINT32                      mType;
        MUINT32                     mCount;
        union
        {
            MUINT8                  v0[sizeof(MINT8)];
            MUINT8                  v1[sizeof(MINT32)];
            MUINT8                  v2[sizeof(MFLOAT)];
            MUINT8                  v3[sizeof(MINT64)];
            MUINT8                  v4[sizeof(MDOUBLE)];
            MUINT8                  v5[sizeof(MRational)];
            MUINT8                  v6[sizeof(MPoint)];
            MUINT8                  v7[sizeof(MSize)];
            MUINT8                  v8[sizeof(MRect)];
            // immediate basic storage without array allocate
        }                           mImmData;
    private:
        BasicDataSP                 mBasicDataSP;
        VecStorageSP                mMetaSP;
        VecBasicDataSP              mMemSP;
        MBOOL                       mReadOnly;

        inline ssize_t               _typeSize() const;
    public:
        Content();
        ~Content();
        Content(const void* flattenSrc, size_t size,MBOOL &isError);
        Content(Content &src);

        size_t                      flattenSize();
        ssize_t                     flatten(void* dest, size_t destSize);
        ssize_t                     unflatten(const void* src, size_t srcSize);
        inline MBOOL                isReadOnly()        {return mReadOnly;}
        inline void                 setReadOnly()       {mReadOnly = MTRUE;}
        inline void                 setWritable()       {mReadOnly = MFALSE;}
        inline MINT32               getType()           {return mType;}
        inline MUINT32              getCount()          {return mCount;}
        MBOOL                       remove(MINT index);
        MBOOL                       clear();
        MBOOL                       update(MINT index, const void* array, size_t size, MINT32 type);
        const void*                 getBasicData(MINT index, MINT32 type) const;
        StorageSP                   getMetadata(MINT index, MINT32 type) const;
        BasicDataSP                 getMemory(MINT index, MINT32 type) const;

    public:
        // global static usage
        static size_t               mStatNum; // obj numbers
        static size_t               mStatSize; // bytes
    };
/******************************************************************************
 * IMetadata::Storage class declaration
 ******************************************************************************/
    class Storage
    {
        struct Item
        {
            Tag_t       tag;
            ContentSP   contentSP;
            bool operator<(const Item &rhs) const { return tag<rhs.tag;}
            bool operator<(const Tag_t   &rtag) const { return tag<rtag;}
        };

        typedef std::vector<Item>       ItemTable;

        mutable ItemTable               mItems; // keep sorted
        MBOOL                           mReadOnly;
    public:
                                        Storage();
                                        ~Storage();
        void                            setWritable();
        void                            setReadOnly();
        MBOOL                           isReadOnly() const;
        MBOOL                           inWhiteList(const Tag_t tag, const Tag_t *W, const ssize_t WSize, size_t &pos) const;
        ssize_t                         unflatten(const void *src, const size_t srcSize);
        ssize_t                         flatten(void *dest, const size_t destSize, const Tag_t *W, const ssize_t WSize) const;
        size_t                          flattenSize(const Tag_t *W, const ssize_t WSize) const;
        IMetadata::StorageSP            clone() const;
        void                            update(Tag_t tag, ContentSP contentSP);
        void                            update(const IMetadata::StorageSP &sp);
        size_t                          count() const;
        MBOOL                           remove(Tag_t tag);
        MBOOL                           getContentSP(Tag_t tag, IMetadata::ContentSP &retContentSP, MBOOL isTakeAway=MFALSE) const;
        MBOOL                           getContentSPByIndex(size_t index, IMetadata::ContentSP &retContentSP, Tag_t &retTag) const;
    public:
        // global static usage
        static size_t                   mStatNum; // obj numbers
        static size_t                   mStatSize; // bytes

    };
/******************************************************************************
 *  IMetadata::Memory
 ******************************************************************************/
public:
    class Memory
    {
    // interfaces
    public:
        size_t                      size() const;
        void                        resize(const size_t size);
        size_t                      append(const Memory &other);
        const uint8_t*              array(size_t &retSize) const;
        const uint8_t*              array() const;
        uint8_t*                    editArray(size_t &retSize);
        uint8_t*                    editArray();
        uint8_t                     itemAt(size_t index) const;
        void                        clear();

        // android support
        size_t                      appendVector(const Memory &other);
        size_t                      appendVector(const android::Vector<MUINT8> &v);

        const BasicDataSP           getDataSP() const;

    public:
        Memory();
        Memory(const Memory &other);
        Memory(Memory &&other);
        Memory(const void *data, size_t size);
        Memory(const BasicDataSP& src);
        ~Memory();

    public:
        Memory& operator = (Memory &&other);
        Memory& operator = (const Memory &other);
        bool operator == (const Memory &other) const;
        bool operator != (const Memory &other) const;

    private:
        mutable BasicDataSP         mDataSP;
    };


/******************************************************************************
 *  IMetadata::IEntry
 ******************************************************************************/
public:
    class IEntry
    {
    // Definitions.
    public:
        enum { BAD_TAG = -1U };

    public:
                                    IEntry(Tag_t tag = BAD_TAG);
                                    IEntry(IEntry const& other);
                                    IEntry(Tag_t tag,std::shared_ptr<IMetadata::Content> contentPtr);
        IEntry&                     operator=(IEntry const& other);
    public:
                                    ~IEntry();
        MUINT32                     tag() const;    // Return the tag
        MINT32                      type() const;   // Return the type
        const void*                 data() const;   // Return the start address of IEntry's container
        MBOOL                       isEmpty() const;// Check to see whether it is empty (no items) or not
        MUINT                       count() const;  // Return the number of items
        MVOID                       clear();        // Clear all items
        android::status_t           removeAt(MUINT index);// Delete an item at a given index
        ContentSP                   getContentSP() const;

        #define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
        MVOID                       push_back(_T const& item, Type2Type<_T>); \
        MVOID                       replaceItemAt(MUINT index, _T const& item, Type2Type<_T>); \
        _T                          itemAt(MUINT index, Type2Type<_T>) const; \

        #define IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(_T) \
        MVOID                       push_back(_T const* array, size_t size, Type2Type<_T>); \
        MVOID                       replaceItemAt(MUINT index, _T const* array, size_t size, Type2Type<_T>); \
        MBOOL                       itemAt(MUINT index, _T *array, size_t size, Type2Type<_T>) const;

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

        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MUINT8)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MINT32)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MFLOAT)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MINT64)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MDOUBLE)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MRational)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MPoint)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MSize)
        IMETADATA_IENTRY_ARRAY_OPS_DECLARATION(MRect)
        #undef  IMETADATA_IENTRY_OPS_DECLARATION
        #undef  IMETADATA_IENTRY_ARRAY_OPS_DECLARATION

    public:
        // Get index of target value in Entry
        template <typename T>
        static int indexOf(IEntry& entry, const T& target)
        {
            auto count = entry.count();
            for(size_t i = 0 ; i < count ; i++)
            {
                if( entry.itemAt(i, Type2Type<T>()) == target)
                {
                    return (int)i;
                }
            }
            return -1;
        }

    private:

        friend class IMetadata;
        mutable std::mutex              mLock;
        Tag_t                           mTag;
        mutable ContentSP               mContentPtr;

        // IEntry internal function
        // _Content use MINT index type for -1 do append
        inline ContentLayout*           _ContentHeader(const ContentSP &sp) const;
        inline void                     _ContentResize(const ContentSP &sp,size_t size);
        inline void                     _ContentResize(const ContentSP &sp,size_t offset, size_t oldItemSize, size_t newItemSize);
        inline ContentSP                _ContentNew(); // new a Content
        inline ContentSP                _ContentClone(const ContentSP &src); // clone a Content
        inline MBOOL                    _ContentIsReadOnly(const ContentSP &sp) const;
        inline void                     _ContentSetReadOnly(const ContentSP &sp) const;
        inline MINT32                   _ContentType(const ContentSP &sp) const;
        inline MUINT32                  _ContentCount(const ContentSP &sp) const;
        inline size_t                   _ContentTypeSize(MINT32 type) const;
        inline MBOOL                    _ContentRemove(ContentSP &sp, MINT index);
        inline MBOOL                    _ContentClear(ContentSP &sp);
        inline MBOOL                    _ContentUpdate(ContentSP &sp, MINT index, const void* array, size_t size, MINT32 type);
        inline const void*              _ContentGetBasicData(ContentSP &sp, MINT index, MINT32 type) const;
        inline StorageSP                _ContentGetIMetadata(ContentSP &sp, MINT index, MINT32 type) const;
        inline BasicDataSP              _ContentGetMemory(ContentSP &sp, MINT index, MINT32 type) const;
        inline ContentRawData*          _ContentRawDataByOffset(const ContentSP &sp, size_t offset) const;
        inline size_t                   _ContentRawDataSize(const ContentRawData *p) const;
        inline size_t                   _ContentRawDataSize(const size_t size) const;

        inline void                     _SwitchWritable();
    };

/******************************************************************************
 *  IMetadata Interfaces
 ******************************************************************************/
public:
    //// Constructor
                                        ~IMetadata();
                                        IMetadata();
                                        IMetadata(const void *flattenSrc,size_t size);
                                        IMetadata(const StorageSP& src);

    //// share storage
    StorageSP                           getStorageSP() const;

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
    MBOOL                               isEmpty() const;

    /**
     * Return the number of entries.
     */
    MUINT                               count() const;

public:     ////                        Operations.

    /**
     * Clear all entries.
     * Note: Copy-on write.
     */
    MVOID                               clear();

    /**
     * Delete an entry by tag.
     * Note: Copy-on write.
     */
    android::status_t                   remove(Tag_t tag);

    /**
     * Sort all entries for faster find.
     * Note: Copy-on write.
     */
    android::status_t                   sort();

    /**
     * Update metadata entry. An entry will be created if it doesn't exist already.
     * Note: Copy-on write.
     */
    android::status_t                   update(Tag_t tag, IEntry const& entry);
    android::status_t                   update(IEntry const& entry); // use entry's tag value

    IEntry                              entryFor(Tag_t tag, MBOOL isTakeAway=MFALSE) const;
    IEntry                              entryAt(MUINT index) const;

    /**
     * Take metadata entry by tag. After invoked this method, the metadata enty of tag
     * in metadata will be removed.
     * Note: Without all element copy.
     * Complexity: O(log N)
     */
    IEntry                              takeEntryFor(Tag_t tag);

    /**
     * Flatten/Unflatten IMetadata.
     * Note: W != nullptr && WSize != -1, means WHITELIST enable. all data will be filtered
     *      WSize = 0 , means NO DATA will be processed, all data be filtered OUT!!
     *
     *      Whitelist MUST sorted in ASCENDING order
     *      Whitelist only apply at level 1 not include nest/internal metadata
     *      Whitelist filter Complexity: O(N)
     */
    ssize_t                             unflatten(void* buf, size_t buf_size);
    ssize_t                             flatten(void* buf, size_t buf_size, const Tag_t *W = nullptr, const ssize_t WSize=-1) const;
    size_t                              flattenSize(const Tag_t *W = nullptr, const ssize_t WSize=-1) const;

    void                                dump(int layer=0);

public:
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
    static android::status_t setEntry(IMetadata* metadata, MUINT32 const tag, T const& val)
    {
        if (nullptr == metadata)
            return -EINVAL;//BAD_VALUE

        IMetadata::IEntry entry(tag);
        entry.push_back(val, Type2Type<T>());
        int err = metadata->update(tag, entry);
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
    static bool getEntry(const IMetadata* metadata, MUINT32 const tag, T& val, size_t index = 0)
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

private: //
    inline void                         _SwitchWritable() const;
    inline void                         _SwitchReadOnly() const;
    mutable std::mutex                  mLock;
    mutable StorageSP                   mStorage;
    mutable unsigned int                mValidNum;
    mutable unsigned int                mValidNumBackup;

public:
    // global tracking error tag
    static Tag_t                        mErrorTypeTag; // tracking error type of tag
    static inline void setErrorTypeTag(Tag_t tag) {if(tag <mErrorTypeTag) mErrorTypeTag = tag;} // only focus smallest tag
    static inline Tag_t getErrorTypeTag(void)  {return mErrorTypeTag;}

    static unsigned int                 mSerialNum; // IMetadata obj counter

    // currently, only for control dump() output
    // Enable via adb shell setprop vendor.debug.camera.metadata 1
    static int                          mLogLevel;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATA_V2_H_

