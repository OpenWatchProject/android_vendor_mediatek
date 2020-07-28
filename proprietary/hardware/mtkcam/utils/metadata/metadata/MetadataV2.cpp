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
#define LOG_TAG "Metadata2"
//
#include <stdio.h>
#include <stdlib.h>

#include <cutils/properties.h>
//
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
//
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/common.h>
//
#include <mtkcam/utils/metadata/mtk_metadata_types.h>


#ifdef MTKCAM_METADATA_V2
#include <system/camera_metadata.h>

#include <algorithm>
#include <atomic>
#include <array>
#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <libladder.h>

using namespace NSCam;
using namespace android;
using namespace std;

/******************************************************************************
 * Global static variable
 ******************************************************************************/
size_t              IMetadata::Storage::mStatNum  = 0;
size_t              IMetadata::Storage::mStatSize = 0;

IMetadata::Tag_t    IMetadata::mErrorTypeTag = IMetadata::IEntry::BAD_TAG;

unsigned int        IMetadata::mSerialNum = 0;
int                 IMetadata::mLogLevel = ::property_get_int32("vendor.debug.camera.metadata", 0);

/******************************************************************************
 * IMetadata::Storage readonly/writeable
 ******************************************************************************/
inline void IMetadata::Storage::setWritable()
{
    mReadOnly = MFALSE;
}
inline void IMetadata::Storage::setReadOnly()
{
    mReadOnly = MTRUE;
}
inline MBOOL IMetadata::Storage::isReadOnly() const
{
    return mReadOnly;
}
/******************************************************************************
 * IMetadata::Storage constructor/destructor
 ******************************************************************************/
IMetadata::Storage::Storage()
{
    META_LOGV("Storage cnt:%zu", ++IMetadata::Storage::mStatNum); // only counting when log level enable

    mReadOnly = MFALSE; // default is writable
}

IMetadata::Storage::~Storage()
{
    META_LOGV("Storage cnt:%zu", --IMetadata::Storage::mStatNum); // only counting when log level enable
}
/******************************************************************************
 * IMetadata::Storage::clone
 *      create new Storage for writable
 ******************************************************************************/
inline IMetadata::StorageSP IMetadata::Storage::clone() const
{
    META_ASSERTV(mReadOnly, "only read only can be clone");

    auto sp = std::make_shared<Storage>();
    sp->mItems = mItems;
    return sp;
}

/******************************************************************************
 * IMetadata::Storage::inWhiteList
 * Check tag in white list or not
 *      W: Tag_t array in ascending order
 *      WSize: W array numbers
 *      pos: will remember last time search position
 ******************************************************************************/
inline MBOOL IMetadata::Storage::inWhiteList(const Tag_t tag, const Tag_t *W, const ssize_t WSize, size_t &pos) const
{
    if(WSize == 0) // whitelist available but item numbers is 0. all data will be IGNORE
        return MFALSE;

    if(W == nullptr) // NO whitelist available, all data are valid
        return MTRUE;

    while(pos < WSize) {
        if(tag > W[pos]) // current white list value too small, scan next one
            pos++;
        else if(tag == W[pos]) // find it !
            return MTRUE;
        else // Not found
            break;
    }
    return MFALSE;
}
/******************************************************************************
 * IMetadata::Storage flatten/unflatten
 ******************************************************************************/
#define FLATTEN_PATTERN  ((MUINT32) 0x01245432)
inline ssize_t IMetadata::Storage::unflatten(const void *src, const size_t srcSize)
{
    META_ASSERTV(!mReadOnly, "read only mode can't modify data");

    META_LOGV("src %p, size %d",src, (int)srcSize);

    if(src == nullptr) {
        META_LOGE("flatten nullpointer");
        return -1;
    }

    auto p = (MUINT8*) src;
    auto srcEnd = p+ srcSize;

    // check header pattern
    auto header_pattern = *(MUINT32*)p;                 p+= sizeof(MUINT32);
    if(header_pattern != FLATTEN_PATTERN) {
        META_LOGE("unflatten buffer not correct. pattern %x", header_pattern);
        return -1;
    }

    // read key count
    auto key_count = *(MUINT32*)p;                      p+=sizeof(MUINT32);
    mItems.resize(key_count);

    // decod key table, it should already sorted
    for(int i =0;i<(int)key_count;i++) {
        mItems[i].tag = *(Tag_t*)p;                     p+= sizeof(Tag_t);
        auto size = *(MUINT32*)p;                       p+=sizeof(MUINT32);

        META_LOGV("decoding keyData[%d]. tag %x, size %d",i, (int)mItems[i].tag, (int) size);

        if(size>0) {
            // create Content object with copy content [p .. p+size]
            MBOOL isError = MFALSE;
            mItems[i].contentSP = std::make_shared<Content>(p, (size_t)size, isError); p+=size;

            if(isError)// unflatten content fail
                return -1;
        }
        else {
            mItems[i].contentSP = nullptr;
        }
    }
    if(p > srcEnd) {
        META_LOGE("unflatten buffer overflow!!!!!! %p %p", p, srcEnd);
        return -1;
    }

    auto used = (ssize_t) (p - (MUINT8*)src);
    META_LOGV("used %zd bytes", used);
    return used;
}

ssize_t IMetadata::Storage::flatten(void *dest, const size_t destSize, const Tag_t *W, const ssize_t WSize) const
{
    META_ASSERTV(mReadOnly, "writable mode can't flatten out");

    META_LOGV("Storage %p, dest %p, destSize %d",this, dest, (int)destSize);

    if(dest == nullptr) {
        META_LOGE("flatten nullpointer");
        return -1;
    }

    auto p = (MUINT8*) dest;
    auto destEnd = p+ destSize;


    // write header pattern
    *((MUINT32*)p)  = FLATTEN_PATTERN;                  p+=sizeof(MUINT32);

    // record & reserve key count address
    auto p_key_count = p;                               p+=sizeof(MUINT32);
    MUINT32 key_count = 0;

    // start encoding
    size_t WPos = 0; // search white list from begin position
    for(auto &&item: mItems) {
        if(!inWhiteList(item.tag, W, WSize, WPos)) // check whitelist
            continue;

        // write tag
        key_count ++;
        *(Tag_t*)p  = item.tag;                         p+=sizeof(Tag_t);

        if( item.contentSP) {
            auto sp = item.contentSP;

            // keey size address, update later
            auto p_size = (MUINT32*)p;                  p+=sizeof(MUINT32);

            // provide remainder space for write data
            auto remainderSpace = (size_t)(destEnd - p);
            auto size = sp->flatten(p, remainderSpace); p+=size;
            if(size == -1) {
                META_LOGE("encoding tag %x fail , size %d",(int)item.tag, (int) size);
                return -1;
            }

            // update size
            *p_size = size;

            META_LOGV("encoding tag %x, size %d",(int)item.tag, (int) size);
        }
        else { // nullptr data only need write size = 0
            *(MUINT32*)p= 0;                            p+=sizeof(MUINT32);
            META_LOGV("encoding tag %x, data ptr is nullptr",(int)item.tag);
        }
    }

    // write key count
    *((MUINT32*)p_key_count)  = key_count;

    if(p > destEnd) {
        META_LOGE("flatten buffer overflow!!!!!! %p %p", p, destEnd);
        return -1;
    }

    auto used = (size_t) (p - (MUINT8*)dest);
    META_LOGV("used %zd bytes", used);
    return used;
}
size_t IMetadata::Storage::flattenSize(const Tag_t *W, const ssize_t WSize) const
{
    META_ASSERTV(mReadOnly, "writable mode can't query flatten size");

    // header pattern + key count field
    size_t size = sizeof(MUINT32) + sizeof(MUINT32);

    // loop each item
    size_t WPos = 0; // search white list from begin position
    for(auto &&item: mItems) {

        if(!inWhiteList(item.tag, W, WSize, WPos)) // check whitelist
            continue;

        size += sizeof(Tag_t) + sizeof(MUINT32); //tag , length
        if(item.contentSP) // not nullptr
            size += item.contentSP->flattenSize();
        META_LOGV("current size %d ,tag %x,  item.contentSP %p", (int)size, (int)item.tag, item.contentSP.get());
    }
    META_LOGV("Storage %p , flattenSize %zu",this, size);
    return size;
}
/******************************************************************************
 * IMetadata::Storage::update
 *      append tag or update exist tag
 ******************************************************************************/
inline void IMetadata::Storage::update(Tag_t tag, ContentSP contentSP)
{
    META_ASSERTV(!mReadOnly, "read only mode can't modify data");

    auto &&pos = std::lower_bound(mItems.begin(), mItems.end(),tag); // binary search
    if(pos == mItems.end() || pos->tag != tag) { // new tag
        mItems.insert(pos,{tag, contentSP}); //inserting into sorted vector
    }
    else { //replace
        pos->contentSP = contentSP;
    }
}
/******************************************************************************
 * IMetadata::Storage::append
 *      merge two Storage data
 *      same tag will be overwrite
 ******************************************************************************/
inline void IMetadata::Storage::update(const IMetadata::StorageSP &sp)
{
    if(!sp) return; // no data directly return

    META_ASSERTV(!mReadOnly, "read only mode can't modify data");
    META_ASSERTV(sp->isReadOnly(), "Only can access read only source");

    // merge two data, because can't appear two same tag,
    // we need insert sort one by one
    for(auto &&item: sp->mItems) {
        update(item.tag, item.contentSP);
    }
}
/******************************************************************************
 * IMetadata::Storage::count
 ******************************************************************************/
inline size_t IMetadata::Storage::count() const
{
    return mItems.size();
}

/******************************************************************************
 * IMetadata::Storage::remove
 *      remove tag
 ******************************************************************************/
inline MBOOL IMetadata::Storage::remove(Tag_t tag)
{
    META_ASSERTV(!mReadOnly, "read only mode can't modify data");

    auto &&it = std::lower_bound(mItems.begin(), mItems.end(),tag); // binary search
    if(it != mItems.end() && it->tag == tag) {
        mItems.erase(it); // remove item , it still keep sorted
        META_LOGV("tag %x removed",tag);
        return MTRUE;
    }
    else {
        META_LOGV("tag %x Not Found",tag);
        return MFALSE;
    }
}
/******************************************************************************
 * IMetadata::Storage::getContentSP
 *      Query tag and return ContentSP
 *      return MFALSE if not found
 *      Note: retContentSP maybe nullptr if it is empty IEntry with valid tag
 ******************************************************************************/
inline MBOOL IMetadata::Storage::getContentSP(Tag_t tag, IMetadata::ContentSP &retContentSP, MBOOL isTakeAway) const
{
    auto &&it = std::lower_bound(mItems.begin(), mItems.end(),tag); // binary search
    if(it != mItems.end() && it->tag == tag) {
        META_LOGV("tag %x Found",tag);
        auto sp = it->contentSP;

        if(isTakeAway) {
            META_ASSERTV(!mReadOnly, "read only mode can't modify data");
            mItems.erase(it); // remove item , it still keep sorted
            META_LOGV("tag %x removed",tag);
        }
        retContentSP = sp;
        return MTRUE;
    }
    else {
        META_LOGV("tag %x Not Found",tag);
        retContentSP = nullptr;
        return MFALSE;
    }
}
inline MBOOL IMetadata::Storage::getContentSPByIndex(size_t index, IMetadata::ContentSP &retContentSP, Tag_t &retTag) const
{
    if(index >= (size_t) mItems.size()) {
        META_LOGE("index %zu out of range %d",index, (int)mItems.size());
        retContentSP = nullptr;
        return MFALSE;
    }

    auto &&it = mItems[index];
    retTag = it.tag;
    retContentSP = it.contentSP;
    return MTRUE;
}
/******************************************************************************
 * IMetadata internal function
 *      Due to want to write, it will create new Storage
 *      no thread protection
 ******************************************************************************/
inline void IMetadata::_SwitchWritable() const
{
    if(!mStorage) {
        META_LOGV("create new Storage");
        mStorage = std::make_shared<IMetadata::Storage>();
    }
    else if(mStorage->isReadOnly()) {
        if(mStorage.use_count() >1) {
            META_LOGV("read only, clone new one for writable");
            mStorage = mStorage->clone();
        }
        else {
            META_LOGV("convert from read only to writable due to only 1 user");
            mStorage->setWritable();
        }
    }
}
/******************************************************************************
 * IMetadata internal function
 *      change to read-only
 *      no thread protection
 ******************************************************************************/
inline void IMetadata::_SwitchReadOnly() const
{
    if(mStorage)
        mStorage->setReadOnly();
}
/******************************************************************************
 * IMetadata Constructor/Destructor
 ******************************************************************************/
IMetadata::IMetadata()
{
    mStorage = nullptr;

    mValidNum = mValidNumBackup = (++IMetadata::mSerialNum) |0x80000000; // always !=0

    META_LOGV("id(%x)", mValidNum);
}

IMetadata::IMetadata(const IMetadata& other)
{
    mStorage = other.getStorageSP();

    mValidNum = mValidNumBackup = (++IMetadata::mSerialNum) |0x80000000; // always !=0
    META_LOGV("(&%x) id(%x)", other.mValidNum, mValidNum);
}
IMetadata::IMetadata(const IMetadata::StorageSP& src)
{
    mStorage = src;

    mValidNum = mValidNumBackup = (++IMetadata::mSerialNum) |0x80000000; // always !=0
    META_LOGV("mStorage %p, id(%x)", src.get(), mValidNum);
}
IMetadata::IMetadata(const void *flattenSrc,size_t size)
{

    mStorage = std::make_shared<IMetadata::Storage>();
    if(mStorage->unflatten(flattenSrc,size) == -1) {
        META_LOGE("unflatten error %p, size %zu", flattenSrc, size);
        mStorage = nullptr;
    }

    mValidNum = mValidNumBackup = (++IMetadata::mSerialNum) |0x80000000; // always !=0
    META_LOGV("flattenSrc %p, size %zu id(%x)",flattenSrc, size, mValidNum);
}
IMetadata::~IMetadata()
{
    META_LOGV("mStorage %p(cnt:%d) id(%x)", mStorage.get(),(int) mStorage.use_count(), mValidNum);
    mValidNum = 0; // 0 means this obj should invalid
}

/******************************************************************************
 * IMetadata::getStorageSP()
 *      storage become read only and allow to share to others
 ******************************************************************************/
IMetadata::StorageSP IMetadata::getStorageSP() const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);
        _SwitchReadOnly();
        return mStorage;
    }//LOCK PROTECTION END
}
/******************************************************************************
 * IMetadata::flatten
 *      W: white list array
 *      WSize: white list array size
 ******************************************************************************/
ssize_t IMetadata::flatten(void* buf, size_t buf_size, const Tag_t *W, const ssize_t WSize) const
{
    META_LOGV("buf %p, buf_size %d",buf, (int)buf_size);
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        if(!mStorage) // crreate empty for flatten header
            mStorage = std::make_shared<IMetadata::Storage>();

        _SwitchReadOnly();
        return mStorage->flatten(buf,buf_size, W, WSize);
    }//LOCK PROTECTION END
}
/******************************************************************************
 * IMetadata::flattenSize
 ******************************************************************************/
size_t IMetadata::flattenSize(const Tag_t *W, const ssize_t WSize) const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        if(!mStorage) // crreate empty for flatten header
            mStorage = std::make_shared<IMetadata::Storage>();

        _SwitchReadOnly();
        return mStorage->flattenSize(W,WSize);
    }//LOCK PROTECTION END
}

/******************************************************************************
 * IMetadata::unflatten
 ******************************************************************************/
ssize_t IMetadata::unflatten(void* buf, size_t buf_size)
{
    META_LOGV("buf %p, buf_size %d",buf, (int)buf_size);
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        //create new Storage
        mStorage = std::make_shared<IMetadata::Storage>();

        //unflatten
        return mStorage->unflatten(buf, buf_size);
    }
}

/******************************************************************************
 * IMetadata assignment operation
 ******************************************************************************/
IMetadata& IMetadata::operator=(IMetadata const& other)
{
    META_LOGV("IMetadata::IMetadata = ");
    if(this != &other) {
        {//LOCK PROTECTION BEGIN
            META_LOGE_CALLSTACK_IF(mValidNum==0,
                    "mValidNum invalid(%x) mValidNumBackup(%x) other.mValidNum(%x) other.mValidNumBackup(%x)",
                    mValidNum, mValidNumBackup, other.mValidNum, other.mValidNumBackup);
            std::lock_guard<std::mutex> lk1(mLock);

            mStorage = other.getStorageSP();
        }//LOCK PROTECTION END
    }
    return *this;
}

IMetadata& IMetadata::operator+=(IMetadata const& other)
{
    META_LOGV("this %p, other %p",(void*)this, (void*)&other);
    if(this != &other) {
        auto srcStorage = other.getStorageSP(); // "other" will become readonly

        if(srcStorage) // only need update if source data exists
        {//LOCK PROTECTION BEGIN
            META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
            std::lock_guard<std::mutex> lk1(mLock);

            if(!mStorage)
                mStorage = srcStorage; // just share it
            else {
                _SwitchWritable();
                mStorage->update(srcStorage);
            }
        }//LOCK PROTECTION END
    }
    return *this;
}
IMetadata IMetadata::operator+(IMetadata const& other) //helper function
{
    return IMetadata(*this) += other;
}


/******************************************************************************
 * IMetadata status query
 ******************************************************************************/
MBOOL IMetadata::isEmpty() const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        _SwitchReadOnly();

        return (!mStorage) || (mStorage->count() == 0);
    }//LOCK PROTECTION END
}
MUINT IMetadata::count() const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        if(mStorage) {
            _SwitchReadOnly();
            return mStorage->count();
        }
        return 0;
    }//LOCK PROTECTION END
}
/******************************************************************************
 * IMetadata clear
 ******************************************************************************/
MVOID IMetadata::clear()
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);
        mStorage = nullptr; // just release it
    }//LOCK PROTECTION END
}

/******************************************************************************
 * IMetadata remove one entry via tag
 ******************************************************************************/
android::status_t IMetadata::remove(Tag_t tag)
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        _SwitchWritable();

        if(mStorage->remove(tag))
            return OK;
        return NAME_NOT_FOUND;
    }//LOCK PROTECTION END
}

/******************************************************************************
 * IMetadata sort  (USELESS function, always keep sorted)
 ******************************************************************************/
android::status_t IMetadata::sort()
{
    return OK;
}

/******************************************************************************
 * IMetadata::update
 ******************************************************************************/
android::status_t IMetadata::update(Tag_t tag, IMetadata::IEntry const& entry)
{
    auto etag = entry.tag();

    META_LOGV("mStorage %p, tag %x, entry %p(tag %x)",mStorage.get(), (int)tag,(void*)&entry, (int)etag);

    if(etag != IMetadata::IEntry::BAD_TAG && etag != tag) {
        META_LOGE_CALLSTACK("tag mismatch update tag=%x but entry.tag=%x", (int) tag, (int) entry.tag());
    }

#if !defined(METADATA_USER)
    // Tracking error type tag for debug
    if(IMetadata::getErrorTypeTag() == tag) {
        META_LOGE_CALLSTACK("ErrTypeTag tracking tag(0x%x) entry.type(%d)", (int) tag, (int) entry.type());
    }
#endif

    auto contentSP = entry.getContentSP(); // Entry should not include at IMetadata mutex scope
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        _SwitchWritable();

        mStorage->update(tag,contentSP);

        return OK;
    }//LOCK PROTECTION END
}
android::status_t IMetadata::update(IEntry const& entry)
{
    auto tag = entry.tag();
    if(tag == IMetadata::IEntry::BAD_TAG) {
        META_LOGE("entry without valid tag value");
        META_LOGV_CALLSTACK("entry without valid tag value");
        return BAD_VALUE;
    }

#if !defined(METADATA_USER)
    // Tracking error type tag for debug
    if(IMetadata::getErrorTypeTag() == tag) {
        META_LOGE_CALLSTACK("ErrTypeTag tracking tag(0x%x) entry.type(%d)", (int) tag, (int) entry.type());
    }
#endif

    return update(tag, entry);
} // use entry's tag value

/******************************************************************************
 * IMetadata::entryFor query entry via tag
 ******************************************************************************/
IMetadata::IEntry IMetadata::entryFor(Tag_t tag, MBOOL isTakeAway) const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        if(mStorage) {
            if(isTakeAway)
                _SwitchWritable();

            ContentSP retContentSP;

            if(mStorage->getContentSP(tag, retContentSP, isTakeAway)) {
                META_LOGV("entryFor(tag %x,isTakeAway %d) found",(int)tag, (int) isTakeAway);
                return IMetadata::IEntry(tag, retContentSP);
            }
        }
        META_LOGV("entryFor(tag %x,isTakeAway %d) not found",(int)tag, (int) isTakeAway);
        return IMetadata::IEntry(); // return empty Entry
    }//LOCK PROTECTION END
}

/******************************************************************************
 * IMetadata::TakeEntryFor  query and remove entry via tag
 ******************************************************************************/
IMetadata::IEntry IMetadata::takeEntryFor(Tag_t tag)
{
    return entryFor(tag,MTRUE);
}

/******************************************************************************
 * IMetadata::entryAt  query via index
 ******************************************************************************/
IMetadata::IEntry IMetadata::entryAt(MUINT index) const
{
    {//LOCK PROTECTION BEGIN
        META_LOGE_CALLSTACK_IF(mValidNum==0, "mValidNum invalid(%x) mValidNumBackup(%x)", mValidNum, mValidNumBackup);
        std::lock_guard<std::mutex> lk1(mLock);

        if(mStorage) {
            Tag_t retTag = 0;
            ContentSP retContentSP;

            if(mStorage->getContentSPByIndex((size_t)index, retContentSP, retTag)) {
                META_LOGV("entryAt(index %d) found. tag %x",(int)index, (int)retTag);
                return IMetadata::IEntry(retTag,retContentSP);
            }
        }
        META_LOGV("entryAt(index %d) not found.",(int)index);
        return IMetadata::IEntry(); // return empty Entry
    }//LOCK PROTECTION END
}
/******************************************************************************
 * IMetadata::dump
 *  WARNING! this function is not thread safe! only use for DEBUG
 ******************************************************************************/
void IMetadata::dump(int layer)
{
    if(IMetadata::mLogLevel == 0)
        return;

    META_LOGD("layer %d",layer);
    if(!mStorage) {
        META_LOGD("IMetadata::dump mStorage == nullptr");
        return;
    }

    {// Can't mutex protect due to nest mutex lock
        #define DUMP_METADATA_STRING(_layer_, _msg_) \
            META_LOGD("(L%d) %s", _layer_, _msg_.string());
        META_LOGD("mStorage count %d",(int)mStorage->count());
        for (size_t i = 0; i < mStorage->count(); ++i) {
            IMetadata::IEntry entry = entryAt(i);
            META_LOGD("entry tag %x, type %x, count %d",(int)entry.tag(),(int)entry.type(),(int)entry.count());
            android::String8 msg = String8::format( "[%s] Map(%zu/%zu) tag(0x%x) type(%d) count(%d) ",
                __FUNCTION__, i, mStorage->count(), entry.tag(), entry.type(), entry.count());
            //
            if ( TYPE_IMetadata == entry.type() ) {
                for( size_t j = 0; j < entry.count(); ++j ) {
                    IMetadata meta = entry.itemAt(j, Type2Type< IMetadata >());
                    msg += String8::format("metadata.. ");
                    DUMP_METADATA_STRING(layer, msg);
                    meta.dump(layer+1);
                }
            } else {
                switch( entry.type() )
                {
                case TYPE_MUINT8:
                    for (size_t j=0; j<entry.count(); j++)
                        msg += String8::format("%d ", entry.itemAt(j, Type2Type< MUINT8 >() ));
                    break;
                case TYPE_MINT32:
                    for (size_t j=0; j<entry.count(); j++)
                        msg += String8::format("%d ", entry.itemAt(j, Type2Type< MINT32 >() ));
                    break;
                case TYPE_MINT64:
                    for (size_t j=0; j<entry.count(); j++)
                        msg += String8::format("%" PRId64 " ", entry.itemAt(j, Type2Type< MINT64 >() ));
                    break;
                case TYPE_MFLOAT:
                    for (size_t j=0; j<entry.count(); j++)
                        msg += String8::format("%f ", entry.itemAt(j, Type2Type< MFLOAT >() ));
                    break;
                case TYPE_MDOUBLE:
                    for (size_t j=0; j<entry.count(); j++)
                        msg += String8::format("%lf ", entry.itemAt(j, Type2Type< MDOUBLE >() ));
                    break;
                case TYPE_MSize:
                    for (size_t j=0; j<entry.count(); j++)
                    {
                        MSize src_size = entry.itemAt(j, Type2Type< MSize >());
                        msg += String8::format( "size(%d,%d) ", src_size.w, src_size.h );
                    }
                    break;
                case TYPE_MRect:
                    for (size_t j=0; j<entry.count(); j++)
                    {
                        MRect src_rect = entry.itemAt(j, Type2Type< MRect >());
                        msg += String8::format( "rect(%d,%d,%d,%d) ",
                                               src_rect.p.x, src_rect.p.y,
                                               src_rect.s.w, src_rect.s.h );
                    }
                    break;
                case TYPE_MPoint:
                    for (size_t j=0; j<entry.count(); j++)
                    {
                        MPoint src_point = entry.itemAt(j, Type2Type< MPoint >());
                        msg += String8::format( "point(%d,%d) ", src_point.x, src_point.y );
                    }
                    break;
                case TYPE_MRational:
                    for (size_t j=0; j<entry.count(); j++)
                    {
                        MRational src_rational = entry.itemAt(j, Type2Type< MRational >());
                        msg += String8::format( "rational(%d,%d) ", src_rational.numerator, src_rational.denominator );
                    }
                    break;
                case TYPE_Memory:
                    for (size_t j=0; j<entry.count(); j++)
                    {
                        IMetadata::Memory mem = entry.itemAt(j, Type2Type< IMetadata::Memory >());
                        int s = (int) mem.size();
                        MUINT8 *c = mem.editArray();
                        msg += String8::format( "memory[%d] size %d:", (int)j, s);
                        for(int i=0;i<s && i< 64 ;i++) { // Only output first 64 bytes
                            msg += String8::format( "%02X ", (int) c[i]);
                        }
                    }
                    break;
                default:
                    msg += String8::format("unsupported type(%d)", entry.type());
                }
            }
            DUMP_METADATA_STRING(layer, msg);
        }
        #undef DUMP_METADATA_STRING
    }// Can't mutex protect due to nest mutex lock
}
/******************************************************************************
 *
 ******************************************************************************/
/******************************************************************************
 *
 ******************************************************************************/

#endif //MTKCAM_METADATA_V2
