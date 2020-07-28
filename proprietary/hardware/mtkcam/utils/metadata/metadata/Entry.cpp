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
#define LOG_TAG "Metadata2/Entry"

//
#include <stdio.h>
#include <stdlib.h>
//
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
//
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/common.h>
//
#include <mtkcam/utils/metadata/mtk_metadata_types.h>

#ifdef MTKCAM_METADATA_V2

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

#include <libladder.h>
using namespace NSCam;
using namespace android;
using namespace std;

/******************************************************************************
 * IEntry::_ContentNew
 *      create Content object with initialize header
 *      no thread protection
 ******************************************************************************/
inline IMetadata::ContentSP IMetadata::IEntry::_ContentNew()
{
    auto sp = std::make_shared<IMetadata::Content>();
    META_LOGV("create Content %p",sp.get());

    return sp;
}
inline IMetadata::ContentSP IMetadata::IEntry::_ContentClone(const ContentSP &src)
{
    // copy whole vector directly
    auto sp = std::make_shared<IMetadata::Content>(*src);
    META_LOGV("create Content %p clone from (%p, ref %d)",sp.get(),src.get(), (int)src.use_count());

    return sp;
}
/******************************************************************************
 * IEntry::_ContentIsReadOnly
 *      check read only or not
 *      no thread protection
 ******************************************************************************/
inline MBOOL IMetadata::IEntry::_ContentIsReadOnly(const ContentSP &sp) const
{
    if(!sp) return MFALSE; // nullptr, assume is writable mode, WARNING: caller need handle this case
    return sp->isReadOnly();
}
/******************************************************************************
 * IEntry::_ContentSetReadOnly
 *      no thread protection
 ******************************************************************************/
inline void IMetadata::IEntry::_ContentSetReadOnly(const ContentSP &sp) const
{
    if(!sp) return; // nullptr, assume is writable mode
    sp->setReadOnly();
}
/******************************************************************************
 * IEntry::_ContentType
 *      return type from header
 *      no thread protection
 ******************************************************************************/
inline MINT32 IMetadata::IEntry::_ContentType(const ContentSP &sp) const
{
    if(!sp) return -1; // nullptr, type is -1
    return sp->getType();
}
/******************************************************************************
 * IEntry::_ContentCount
 *      return count from header
 *      no thread protection
 ******************************************************************************/
inline MUINT32 IMetadata::IEntry::_ContentCount(const ContentSP &sp) const
{
    if(!sp) return 0; // nullptr, count is 0
    return sp->getCount();
}
/******************************************************************************
 * IEntry::_ContentRemove
 ******************************************************************************/
inline MBOOL IMetadata::IEntry::_ContentRemove(ContentSP &sp, MINT index)
{
    if(!sp) return MFALSE; // nullptr, can't remove anything

    META_LOGV("ContentSP(%p, ref %d) index %d",sp.get(), (int)sp.use_count(), (int)index);

    return sp->remove(index);
}
/******************************************************************************
 * IEntry::_ContentClear
 ******************************************************************************/
inline MBOOL IMetadata::IEntry::_ContentClear(ContentSP &sp)
{
    if(!sp) // nullptr, already clear
        return MTRUE;

    META_LOGV("ContentSP(%p, ref %d)",sp.get(), (int)sp.use_count());

    return sp->clear();
}
/******************************************************************************
 * IEntry::_ContentUpdate
 *      to handle replace / append
 *      index:  for replace begin position, will be ignore if append mode
 *
 *              index value range:
 *                  index == -1 means append to end
 *                  Otherwise, index should 0 ~ count -1
 *
 *              for handle array replacement, items will be updated between
 *                  [index][index+1]....[index + size-1]
 *              will enlarge Content Size if space not enough
 *
 *      size: number of items (type)  NOT BYTES
 ******************************************************************************/
inline MBOOL IMetadata::IEntry::_ContentUpdate(ContentSP &sp, MINT index, const void* array, size_t size, MINT32 type)
{
    META_ASSERT(sp!=nullptr,"ContentSP is nullptr");

    META_LOGV("ContentSP(%p, ref %d), index %d, array %p,size %d, type %x",
            sp.get(), (int)sp.use_count(), (int)index, array, (int)size, (int)type);

    return sp->update(index, array, size, type);
}
/******************************************************************************
 * IEntry::_ContentGetBasicData
 ******************************************************************************/
inline const void* IMetadata::IEntry::_ContentGetBasicData(ContentSP &sp, MINT index, MINT32 type) const
{
    if(!sp) return nullptr; // nullptr, can't query

    META_LOGV("ContentSP(%p, ref %d), index %d",sp.get(), (int)sp.use_count(), (int)index);

    return sp->getBasicData(index,type);
}

/******************************************************************************
 * IEntry::_ContentGetIMetadata
 ******************************************************************************/
inline IMetadata::StorageSP IMetadata::IEntry::_ContentGetIMetadata(ContentSP &sp, MINT index, MINT32 type) const
{
    if(!sp) return nullptr; // nullptr, can't query

    META_LOGV("ContentSP(%p, ref %d), index %d",sp.get(), (int)sp.use_count(), (int)index);

    return sp->getMetadata(index,type);
}
/******************************************************************************
 * IEntry::_ContentGetMemory
 ******************************************************************************/
inline IMetadata::BasicDataSP IMetadata::IEntry::_ContentGetMemory(ContentSP &sp, MINT index, MINT32 type) const
{
    if(!sp) return nullptr; // nullptr, can't query

    META_LOGV("ContentSP(%p, ref %d), index %d",sp.get(), (int)sp.use_count(), (int)index);

    return sp->getMemory(index,type);
}

/******************************************************************************
 * IEntry::_SwitchWritable()
 *      no thread protection
 ******************************************************************************/
inline void IMetadata::IEntry::_SwitchWritable()
{
    if(!mContentPtr) // prepare data for write
        mContentPtr = _ContentNew();

    if(_ContentIsReadOnly(mContentPtr)) { // current is read only
        if(mContentPtr.use_count() > 1) // multi user
            mContentPtr = _ContentClone(mContentPtr);
        else {// single user , we can reuse it
            META_LOGV("convert readonly back to writable. due to only 1 user");
            mContentPtr->setWritable();
        }
    }
}
/******************************************************************************
 * IEntry constructor /destructor
 *
 ******************************************************************************/

IMetadata::IEntry::IEntry(Tag_t tag)
{
    META_LOGV("%p created. tag %x",this, (int)tag);
    mTag = tag;
    mContentPtr = nullptr;
}
IMetadata::IEntry::IEntry(Tag_t tag,ContentSP contentSP)
{
    META_LOGV("%p created. tag %x, share content(%p, ref:%d)",
            this, (int)tag,contentSP.get(),(int) contentSP.use_count());

    if(contentSP) {
        if(!_ContentIsReadOnly(contentSP)) {
            META_LOGE_CALLSTACK("NOT ReadOnly! %p created. tag %x, share content(%p, ref:%d)",
                this, (int)tag,contentSP.get(),(int) contentSP.use_count());
            // It should assert, here just workaround to switch source to read only mode
            _ContentSetReadOnly(contentSP);
        }
    }

    mTag = tag;
    mContentPtr = contentSP; //reuse same storage
}
IMetadata::IEntry::IEntry(IMetadata::IEntry const& other)
{
    std::lock_guard<std::mutex> lk1(other.mLock);

    // switch source to read only mode
    _ContentSetReadOnly(other.mContentPtr);

    // share other's data
    mTag = other.mTag;
    mContentPtr = other.mContentPtr;

    META_LOGV("%p created. tag %x, share content(%p, ref:%d)",
            this, (int)mTag, mContentPtr.get(),(int) mContentPtr.use_count());
}

IMetadata::IEntry::~IEntry()
{
    META_LOGV("%p destructor",this);
}

/******************************************************************************
 * IEntry assign operation
 ******************************************************************************/
IMetadata::IEntry& IMetadata::IEntry::operator=(IMetadata::IEntry const& other)
{
    if(this != &other) {
        {//LOCK PROTECTION BEGIN
            std::lock(mLock, other.mLock);
            std::lock_guard<std::mutex> lk1(mLock, std::adopt_lock);
            std::lock_guard<std::mutex> lk2(other.mLock, std::adopt_lock);

            // switch source to read only mode
            _ContentSetReadOnly(other.mContentPtr);

            // share other's data
            mTag = other.mTag;
            mContentPtr = other.mContentPtr;

            META_LOGV("%p created. tag %x, share content(%p, ref:%d)",
                    this, (int)mTag, mContentPtr.get(),(int) mContentPtr.use_count());

        }//LOCK PROTECTION END
    }

    return *this;
}
/******************************************************************************
 * IEntry  tag()
 ******************************************************************************/
MUINT32 IMetadata::IEntry::tag() const
{
    return mTag;
}

/******************************************************************************
 * IEntry  type()
 ******************************************************************************/
MINT32 IMetadata::IEntry::type() const
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);
        return _ContentType(mContentPtr);
    }//LOCK PROTECT END
}

/******************************************************************************
 * IEntry   isEmpty()
 ******************************************************************************/
MBOOL IMetadata::IEntry::isEmpty() const
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);
        return _ContentCount(mContentPtr) == 0;
    }//LOCK PROTECT END
}


/******************************************************************************
 * IEntry   count()
 ******************************************************************************/
MUINT IMetadata::IEntry::count() const
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);
        return _ContentCount(mContentPtr);
    }//LOCK PROTECT END
}

/******************************************************************************
 * IEntry   clear()
 ******************************************************************************/
MVOID IMetadata::IEntry::clear()
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);
        if(mContentPtr) { // Only need take care if data exists
            if(_ContentIsReadOnly(mContentPtr)) // read only just release ref
                mContentPtr = nullptr;
            else
                _ContentClear(mContentPtr); // free content
        }
    }//LOCK PROTECT END
}
/******************************************************************************
 * IEntry removeAt(MUINT index)
 ******************************************************************************/
MERROR IMetadata::IEntry::removeAt(MUINT index)
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);

        if(mContentPtr) {
            _SwitchWritable();

            // remove index
            if(_ContentRemove(mContentPtr, index))
                return OK;
        }
        return BAD_VALUE;
    }//LOCK PROTECT END
}
/******************************************************************************
 * IEntry data()
 *  It will switch to read only mode to share same data to multi users
 *  User should not modify the content via pointer return from this function
 *
 *  !!Dangerous API (should remove due to life cycle only bound at IEntry)
 ******************************************************************************/
const void *IMetadata::IEntry::data() const
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);

        if(mContentPtr) {
            _ContentSetReadOnly(mContentPtr); //switch to read only mode
            auto type = _ContentType(mContentPtr);
            if(type < TYPE_Memory && type < TYPE_IMetadata)
                return _ContentGetBasicData(mContentPtr,0,type);
        }
        return nullptr;
    }//LOCK PROTECT END
}
/******************************************************************************
 * IEntry::getContentSP()
 ******************************************************************************/
IMetadata::ContentSP IMetadata::IEntry::getContentSP() const
{
    {//LOCK PROTECTION BEGIN
        std::lock_guard<std::mutex> lk1(mLock);

        _ContentSetReadOnly(mContentPtr); //switch to read only mode
        return mContentPtr; // it maybe nullptr
    }//LOCK PROTECT END
}

/******************************************************************************
 * IEntry push_back
 ******************************************************************************/
#define IMETADATA_IENTRY_OPS_PUSH_BACK(_T,_TT) \
MVOID IMetadata::IEntry::push_back(_T const& item, Type2Type<_T>) \
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    _SwitchWritable(); \
    if(!_ContentUpdate(mContentPtr,-1, (const void*)&item, 1, TYPE_##_TT)) { \
        META_LOGE("Update error, tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        IMetadata::setErrorTypeTag(mTag); \
    } \
}

#define IMETADATA_IENTRY_ARRAY_OPS_PUSH_BACK(_T,_TT) \
MVOID IMetadata::IEntry::push_back(_T const* array,size_t size, Type2Type<_T>) \
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    _SwitchWritable(); \
    if(!_ContentUpdate(mContentPtr,-1, (const void*)array, size, TYPE_##_TT)) { \
        META_LOGE("Update error, tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        IMetadata::setErrorTypeTag(mTag); \
    } \
}

/******************************************************************************
 * IEntry replaceItemAt
 ******************************************************************************/
#define IMETADATA_IENTRY_OPS_REPLACE_ITEM_AT(_T,_TT) \
MVOID IMetadata::IEntry::replaceItemAt(MUINT index, _T const& item, Type2Type<_T>) \
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    _SwitchWritable(); \
    if(!_ContentUpdate(mContentPtr, (MINT)index, (const void*)& item, 1, TYPE_##_TT)) { \
        META_LOGE("Update error, tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        IMetadata::setErrorTypeTag(mTag); \
    } \
}

#define IMETADATA_IENTRY_ARRAY_OPS_REPLACE_ITEM_AT(_T,_TT) \
MVOID IMetadata::IEntry::replaceItemAt(MUINT index, _T const* array, size_t size, Type2Type<_T>) \
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    _SwitchWritable(); \
    if(!_ContentUpdate(mContentPtr, (MINT)index, (const void*)array, size, TYPE_##_TT)) { \
        META_LOGE("Update error, tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        IMetadata::setErrorTypeTag(mTag); \
    } \
}

/******************************************************************************
 * IEntry itemAt
 ******************************************************************************/
#define IMETADATA_IENTRY_OPS_ITEM_AT(_T,_TT) \
_T IMetadata::IEntry::itemAt(MUINT index, Type2Type<_T>) const\
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    auto p = _ContentGetBasicData(mContentPtr,(MINT)index, TYPE_##_TT); \
    if(p) \
        return *static_cast<const _T*>(p); \
    else {\
        META_LOGE("Get data error , tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        IMetadata::setErrorTypeTag(mTag); \
        return _T(); \
    } \
}

#define IMETADATA_IENTRY_ARRAY_OPS_ITEM_AT(_T,_TT) \
MBOOL IMetadata::IEntry::itemAt(MUINT index, _T *array, size_t size, Type2Type<_T>) const\
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    auto p = _ContentGetBasicData(mContentPtr,(MINT)index, TYPE_##_TT); \
    if(p) { \
        auto count = _ContentCount(mContentPtr); \
        if(index + size <= count) { \
            memcpy(array, p, sizeof(_T) * size); \
            return MTRUE; \
        } \
    } \
    META_LOGE("Get data error , tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
    IMetadata::setErrorTypeTag(mTag); \
    return MFALSE; \
}
/******************************************************************************
 * IEntry itemAt (for RawData)
 ******************************************************************************/
#define IMETADATA_IENTRY_OPS_ITEM_AT_RAWDATA(_T,_TT) \
_T IMetadata::IEntry::itemAt(MUINT index, Type2Type<_T>) const\
{ \
    std::lock_guard<std::mutex> lk1(mLock); \
    auto p = _ContentGet##_TT(mContentPtr,(MINT)index, TYPE_##_TT); \
    if(p) \
        return _T(p); \
    else { \
        META_LOGV("data is nullptr, tag %x, mContentPtr %p", (int)mTag, mContentPtr.get()); \
        return _T(); \
    } \
} \

#define IMETADATA_IENTRY_OPS(_T) \
    IMETADATA_IENTRY_OPS_PUSH_BACK(_T,_T) \
    IMETADATA_IENTRY_OPS_REPLACE_ITEM_AT(_T,_T) \
    IMETADATA_IENTRY_OPS_ITEM_AT(_T,_T) \
    IMETADATA_IENTRY_ARRAY_OPS_PUSH_BACK(_T,_T) \
    IMETADATA_IENTRY_ARRAY_OPS_REPLACE_ITEM_AT(_T,_T) \
    IMETADATA_IENTRY_ARRAY_OPS_ITEM_AT(_T,_T) \

#define IMETADATA_IENTRY_OPS_RAWDATA(_T, _TT) \
    IMETADATA_IENTRY_OPS_PUSH_BACK(_T,_TT) \
    IMETADATA_IENTRY_OPS_REPLACE_ITEM_AT(_T,_TT) \
    IMETADATA_IENTRY_OPS_ITEM_AT_RAWDATA(_T,_TT) \

IMETADATA_IENTRY_OPS(MUINT8)
IMETADATA_IENTRY_OPS(MINT32)
IMETADATA_IENTRY_OPS(MFLOAT)
IMETADATA_IENTRY_OPS(MINT64)
IMETADATA_IENTRY_OPS(MDOUBLE)
IMETADATA_IENTRY_OPS(MRational)
IMETADATA_IENTRY_OPS(MPoint)
IMETADATA_IENTRY_OPS(MSize)
IMETADATA_IENTRY_OPS(MRect)

IMETADATA_IENTRY_OPS_RAWDATA(IMetadata::Memory, Memory)
IMETADATA_IENTRY_OPS_RAWDATA(IMetadata, IMetadata)

#endif //MTMCAM_METADATA_V2
