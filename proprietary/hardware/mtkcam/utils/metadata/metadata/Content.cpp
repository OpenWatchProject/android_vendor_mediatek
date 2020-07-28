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
#define LOG_TAG "Metadata2/Content"

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

#include <mutex>
#include <memory>
#include <array>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <string>
#include <vector>

#include <libladder.h>
using namespace NSCam;
using namespace android;
using namespace std;

// Global
size_t IMetadata::Content::mStatNum  = 0;
size_t IMetadata::Content::mStatSize = 0;

/******************************************************************************
 * Content internal function
 ******************************************************************************/
inline ssize_t IMetadata::Content::_typeSize() const
{
    static const int sizeTable[]= {
        sizeof(MUINT8),
        sizeof(MINT32),
        sizeof(MFLOAT),
        sizeof(MINT64),
        sizeof(MDOUBLE),
        sizeof(MRational),
        sizeof(MPoint),
        sizeof(MSize),
        sizeof(MRect),
        -1, // useless for TYPE_IMetadata
        sizeof(MUINT8), // treat TYPE_Memory as MUINT8  x mCount , Only support one item at Entry
    };

    META_ASSERT( 0<=mType && mType < (int)(sizeof(sizeTable) / sizeof(sizeTable[0])) , "mType invalid %d", (int)mType);
    META_ASSERT( mType != TYPE_IMetadata, "Not support TYPE_IMetadata");

    return sizeTable[mType];
}
/******************************************************************************
 * Content constructor/destructor
 ******************************************************************************/
IMetadata::Content::Content()
{
    META_LOGV("Content cnt:%zu", ++IMetadata::Content::mStatNum); // only counting when log level enable

    mType           = -1;
    mCount          = 0;
    mBasicDataSP    = nullptr;
    mMemSP          = nullptr;
    mMetaSP         = nullptr;
    mReadOnly       = MFALSE; // default is writable
}
IMetadata::Content::Content(const void* flattenSrc, size_t size, MBOOL &isError)
{
    META_LOGV("Content cnt:%zu", ++IMetadata::Content::mStatNum); // only counting when log level enable

    isError         = MFALSE;

    mType           = -1;
    mCount          = 0;
    mBasicDataSP    = nullptr;
    mMemSP          = nullptr;
    mMetaSP         = nullptr;
    mReadOnly       = MFALSE; // default is writable

    if(unflatten(flattenSrc,size) == -1) {
        META_LOGE("unflatten error %p, size %zu", flattenSrc, size);
        isError         = true;

        // reset again
        mType           = -1;
        mCount          = 0;
        mBasicDataSP    = nullptr;
        mMemSP          = nullptr;
        mMetaSP         = nullptr;
        mReadOnly       = MFALSE; // default is writable
    }
}
IMetadata::Content::Content(Content &src)
{
    META_LOGV("Content cnt:%zu", ++IMetadata::Content::mStatNum); // only counting when log level enable

    mType           = src.mType;
    mCount          = src.mCount;
    mBasicDataSP    = nullptr;
    mMemSP          = nullptr;
    mMetaSP         = nullptr;
    mReadOnly       = MFALSE; // default is writable

    if(mCount > 0) {
        if(src.mBasicDataSP) // clone whole vector if available
            mBasicDataSP = std::make_shared<BasicData>(*src.mBasicDataSP);
        else if(src.mMemSP) { // clone Memory pointer table
            META_ASSERT( mType == TYPE_Memory, "mMemSP confuse with type %d", (int)mType);
            mMemSP = std::make_shared<VecBasicData>( *src.mMemSP );
        }
        else if(src.mMetaSP) {// clone Meta   pointer table
            META_ASSERT( mType == TYPE_IMetadata, "mMetaSP confuse with type %d", (int)mType);
            mMetaSP = std::make_shared<VecStorage>( *src.mMetaSP );
        }
        else { // copy immediate value
            auto ts = _typeSize();
            memcpy(&mImmData, &src.mImmData, ts);
        }
    }
}

IMetadata::Content::~Content()
{
    META_LOGV("Content cnt:%zu", --IMetadata::Content::mStatNum); // only counting when log level enable

    META_LOGV("this %p, mCount %d, mType %d, mBasicDataSP %p(ref:%d, size %d), mMetaSP %p(ref:%d, size %d), mMemSP %p(ref:%d, size %d), mReadOnly %d",
            this, (int)mCount, (int)mType,
            mBasicDataSP.get(), (int)mBasicDataSP.use_count(), (int)(mBasicDataSP?mBasicDataSP->size():-1),
            mMetaSP.get(), (int)mMetaSP.use_count(), (int)(mMetaSP?mMetaSP->size():-1),
            mMemSP.get(), (int)mMemSP.use_count(), (int)(mMemSP?mMemSP->size():-1),
            (int)mReadOnly);
}

/******************************************************************************
 * Content flatten/unflatten
 ******************************************************************************/
#define CONTENT_FLATTEN_HEADER      ((MUINT32)0xAB2454CD)
size_t IMetadata::Content::flattenSize()
{
    META_LOGV("this %p, mCount %d, mType %d, mBasicDataSP %p(ref:%d, size %d), mMetaSP %p(ref:%d, size %d), mMemSP %p(ref:%d, size %d), mReadOnly %d",
            this, (int)mCount, (int)mType,
            mBasicDataSP.get(), (int)mBasicDataSP.use_count(), (int)(mBasicDataSP?mBasicDataSP->size():-1),
            mMetaSP.get(), (int)mMetaSP.use_count(), (int)(mMetaSP?mMetaSP->size():-1),
            mMemSP.get(), (int)mMemSP.use_count(), (int)(mMemSP?mMemSP->size():-1),
            (int)mReadOnly);

    size_t size = sizeof(CONTENT_FLATTEN_HEADER) + sizeof(mType) + sizeof(mCount);

    if(mCount > 0) {
        if(mType == TYPE_Memory) {
            META_ASSERT(mMemSP != nullptr, "mMemSP is nullptr but mCount >0");

            for(auto m: *mMemSP) {
                size += sizeof(MUINT32);// size
                if(m)
                    size += m->size(); // content
            }
        }
        else if(mType == TYPE_IMetadata) {
            META_ASSERT(mMetaSP != nullptr, "mMetaSP is nullptr but mCount >0");

            for(auto m: *mMetaSP) {
                size += sizeof(MUINT32);//flatten size
                if(m)
                    size += m->flattenSize(nullptr, -1); // flatten data
            }
        }
        else { // basic type
            size += _typeSize() * mCount;
        }
    }
    META_LOGV("this %p, size %zu", this, size);
    return size;
}

ssize_t IMetadata::Content::flatten(void* dest, size_t destSize)
{
    auto p          = (MUINT8*)dest;
    auto destEnd    = (MUINT8*)dest + destSize;

    META_LOGV("dest %p, destSize %zu, destEnd %p", dest, destSize, destEnd);

    // write header pattern
    *((MUINT32*)p)  = CONTENT_FLATTEN_HEADER;                       p+= sizeof(MUINT32);

    // write basic information
    *((MINT32* )p)  = mType;                                        p+= sizeof(MINT32);
    *((MUINT32*)p)  = mCount;                                       p+= sizeof(MINT32);

    // write data if available
    if(mCount >0) {
        META_ASSERT(mType != -1,"mType should valid");

        if(mType == TYPE_Memory) {
            META_ASSERT(mMemSP != nullptr, "mMemSP is nullptr but mCount >0");

            //write memory one by one
            for(auto m: *mMemSP.get()) {
                if(m) {
                    // write size
                    auto s = (MUINT32)m->size();
                    *((MUINT32*)p)  = s;                            p+= sizeof(MUINT32);
                    // write data
                    memcpy(p, m->array(), s);                       p+= s;
                    META_LOGV("mem %p, size %d", m.get(), (int)s);
                }
                else { // nullptr item, only need write size = 0
                    *((MUINT32*)p)  = 0;                            p+= sizeof(MUINT32);
                }
            }
        }
        else if(mType == TYPE_IMetadata) {
            META_ASSERT(mMetaSP != nullptr, "mMetaSP is nullptr but mCount >0");

            //write metadata one by one
            for(auto m: *mMetaSP.get()) {
                if(m) {
                    auto p_size = p;                                p+= sizeof(MUINT32);
                    // write data
                    auto s = m->flatten(p, (size_t) (destEnd - p),nullptr, -1); p+= s;
                    if(s == -1) {
                        META_LOGE("flatten meta %p error", m.get());
                        return -1;
                    }
                    // write size
                    *((MUINT32*)p_size)  = (MUINT32)s;
                    META_LOGV("flatten meta %p, size %d", m.get(), (int)s);
                }
                else { // nullptr item, only need write size = 0
                    *((MUINT32*)p)  = 0;                            p+= sizeof(MUINT32);
                }
            }
        }
        else { // basic type
            auto ts = _typeSize();

            if(mBasicDataSP) {
                memcpy(p, mBasicDataSP->array(), ts*mCount);
                META_LOGV("basic data. type %d, mCount %d, size %d", (int)mType, (int)mCount, (int)(ts*mCount));
            }
            else {
                META_ASSERT(mCount == 1,"mCount >1 but mBasicDataSP is nullptr");
                memcpy(p, &mImmData, ts);
            }

            p+= ts* mCount;
        }
    }
    if(p>destEnd) {
        META_LOGE("out of buffer %p, %p", p, destEnd);
        return -1;
    }

    auto used = (ssize_t) (p- (MUINT8*)dest);
    META_LOGV("use %zd bytes", used);
    return used;
}
ssize_t IMetadata::Content::unflatten(const void* src, size_t srcSize)
{
    auto p          = (MUINT8*)src;
    auto srcEnd     = (MUINT8*)src + srcSize;

    // check header pattern
    if( *(MUINT32*)p  != CONTENT_FLATTEN_HEADER) {
        META_LOGE("flatten pattern not correct %x", *(MUINT32*)p);
        return -1;
    }
    p+= sizeof(MUINT32);

    // mark as readonly
    mReadOnly = MTRUE;

    // read basic information
    mType   = *((MINT32* )p);                                       p+= sizeof(MINT32);
    mCount  = *((MUINT32*)p);                                       p+= sizeof(MINT32);

    // read data if available
    if(mCount >0) {
        if(mType == -1) {
            META_LOGE("mType hould valid!");
            return -1;
        }

        if(mType == TYPE_Memory) {
            mMemSP = std::make_shared<VecBasicData>(mCount);
            for(int i=0;i< (int)mCount;i++) {
                auto s = *((MUINT32*)p);                            p+= sizeof(MUINT32);
                if(s>0) {
                    // create one Memory
                    auto data = std::make_shared<BasicData>();
                    data->appendArray(p,s);
                    (*mMemSP)[i] = data;                            p+=s;
                }
                META_LOGV("mMemSP[%d] = %p, size %d", i, (*mMemSP)[i].get(), (int)s);
            }
        }
        else if(mType == TYPE_IMetadata) {

            mMetaSP = std::make_shared<std::vector<StorageSP>>(mCount);
            for(int i=0;i< (int)mCount;i++) {
                auto s = *((MUINT32*)p);                            p+= sizeof(MUINT32);
                if(s>0) {
                    // create one Metadata
                    IMetadata m(p, s);                              p+= s;
                    auto data = m.getStorageSP();
                    (*mMetaSP)[i] = data;
                }
                META_LOGV("mMetaSP[%d] = %p, size %d", i, (*mMetaSP)[i].get(), (int)s);
            }
        }
        else { // basic type
            auto ts = _typeSize();
            auto s  = ts*mCount;

            if(mCount == 1){
                memcpy(&mImmData,p, s);
                META_LOGV("basic data , size %d", (int)s);
            }
            else {
                mBasicDataSP = std::make_shared<BasicData>();
                mBasicDataSP->appendArray(p, s);
                META_LOGV("basic data , ts %d, size %d", (int)ts, (int)s);
            }
            p+= s;
        }
    }
    if(p>srcEnd) {
        META_LOGE("out of buffer %p, %p", p, srcEnd);
        return -1;
    }

    auto used = (ssize_t) (p- (MUINT8*)src);
    META_LOGV("use %zd bytes", used);
    return used;
}


/******************************************************************************
 * IMetadata::Content::remove
 ******************************************************************************/
MBOOL IMetadata::Content::remove(MINT index)
{
    if(index <0 || index >= (int)mCount) {
        META_LOGE("out of index remove. cnt %d, index %d", (int)mCount, (int)index);
        return MFALSE; // out of index
    }

    // only 1 item, just release all the thing
    if(mCount == 1) {
        clear();
        return MTRUE;
    }

    if(mMetaSP) {
        if(mType != TYPE_IMetadata) {
            META_LOGE("content confuse. type %d, mMetaSP %p",(int)mType, mMetaSP.get());
            return MFALSE;
        }
        auto iter = mMetaSP->begin();
        mMetaSP->erase( iter + index);
    }
    else if(mMemSP) {
        if(mType != TYPE_Memory) {
            META_LOGE("content confuse. type %d, mMemSP %p",(int)mType, mMemSP.get());
            return MFALSE;
        }

        auto iter = mMemSP->begin();
        mMemSP->erase( iter + index);
    }
    else if(mBasicDataSP) {
        auto ts = _typeSize();
        auto pos = ts* index;

        auto v = mBasicDataSP.get();
        auto d = v->editArray();
        auto remain = mCount - index -1;

        if(remain >0)
            ::memmove(d+pos, d+pos + ts,remain * ts);
        v->resize(ts*(mCount-1));
    }
    mCount --;

    return MTRUE;
}
/******************************************************************************
 * IMetadata::Content::clear
 ******************************************************************************/
MBOOL IMetadata::Content::clear()
{
    mType           = -1;
    mCount          = 0;
    mBasicDataSP    = nullptr; // release data
    mMemSP          = nullptr; // release data
    mMetaSP         = nullptr; // release data
    mReadOnly       = MFALSE; // default is writable
    return MTRUE;
}

/******************************************************************************
 * IMetadata::Content::update
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
MBOOL IMetadata::Content::update(MINT index, const void* array, size_t size, MINT32 type)
{
    META_LOGV("this %p, mCount %d, mType %d, mBasicDataSP %p(ref:%d, size %d), mMetaSP %p(ref:%d, size %d), mMemSP %p(ref:%d, size %d), mReadOnly %d",
            this, (int)mCount, (int)mType,
            mBasicDataSP.get(), (int)mBasicDataSP.use_count(), (int)(mBasicDataSP?mBasicDataSP->size():-1),
            mMetaSP.get(), (int)mMetaSP.use_count(), (int)(mMetaSP?mMetaSP->size():-1),
            mMemSP.get(), (int)mMemSP.use_count(), (int)(mMemSP?mMemSP->size():-1),
            (int)mReadOnly);

    if(index != -1 && (index + size) > (size_t)mCount) { // replace mode, check index value range
        META_LOGE("index (%d) out of range (%d)",(int)index, (int)mCount);
        return MFALSE;
    }

    // first item decide type
    if(index == -1 && mCount == 0)
        mType = type;

    // check type match
    if(mType !=type) {
        META_LOGE_CALLSTACK("type mismatch mType(%x) type(%x)",(int)mType, (int)type);
        return MFALSE;
    }

    auto pos = index;
    if( pos == -1) {// append mode
        pos = mCount;
    }

    // copy data
    switch(type) {
        case TYPE_Memory:
            {
                // append mode , enlarge space
                if(index == -1) {
                    if(!mMemSP)
                        mMemSP = std::make_shared<VecBasicData>(size);
                    else
                        mMemSP->resize( mMemSP->size() + size);

                    META_LOGV("mMemSP %p(ref:%d, size %d)",mMemSP.get(), (int)mMemSP.use_count(), (int)(mMemSP?mMemSP->size():-1));
                    mCount += size;
                }

                // direct replace the data
                for(int i=0;i<(int)size;i++) {
                    auto p = ((IMetadata::Memory*) array)[i].getDataSP();
                    META_LOGV("update mem[%d] %p => %p", pos+i, (*mMemSP)[pos+i].get(), p.get());
                    (*mMemSP)[pos+i] = p;
                }
            }
            break;
        case TYPE_IMetadata:
            {
                // append mode , enlarge space
                if(index == -1) {
                    if(!mMetaSP)
                        mMetaSP = std::make_shared<VecStorage>(size);
                    else
                        mMetaSP->resize( mMetaSP->size() + size);
                    mCount += size;
                }

                // direct replace the data
                for(int i=0;i<(int)size;i++){
                    auto p = ((IMetadata*) array)[i].getStorageSP();
                    META_LOGV("update metadata[%d] %p => %p", pos+i, (*mMetaSP)[pos+i].get(), p.get());
                    (*mMetaSP)[pos+i] = p;
                }
            }
            break;
        default: // basic data type
            {
                auto ts = _typeSize();

                //append mode
                if( index == -1) {
                    // case 1: immediate value not enough and array not created
                    if(mBasicDataSP == nullptr && mCount + size >1 ) {
                        META_LOGV("append mode, case 1");
                        mBasicDataSP = std::make_shared<BasicData>();
                        mBasicDataSP->resize(ts*(mCount + size));

                        auto dest = (MUINT8*) mBasicDataSP->editArray();

                        if(mCount == 1) {// copy first old data if exists
                            memcpy(dest, &mImmData, ts);
                            dest += ts;
                        }

                        memcpy(dest,array, ts* size);
                    } // case 2: array available, just insert data at the end
                    else if(mBasicDataSP) {
                        META_LOGV("append mode, case 2");
                        // avoid to use insert due to performance bad
                        auto d = mBasicDataSP.get();
                        d->resize(ts*(mCount+size));
                        memcpy(d->editArray() + ts*mCount, (MUINT8*)array, ts*size);
                    } // case 3: update to immediate directly
                    else {
                        META_LOGV("append mode, case 3");
                        META_ASSERT( size==1, "size should be 1");
                        memcpy(&mImmData, array, ts);
                    }
                    // update mCount
                    mCount += size;
                }
                else {
                    // replace mode
                    if(mBasicDataSP) {
                        META_LOGV("replace mode, case 1");
                        auto dest = (MUINT8*) mBasicDataSP->editArray();
                        memcpy(dest + pos * ts, array, ts*size);
                    }
                    else {
                        META_LOGV("replace mode, case 2");
                        memcpy(&mImmData, array, ts);
                    }
                }
                break;
            }
            break;
    }
    return MTRUE;
}
/******************************************************************************
 * IMetadata::Content::getBasicData
 ******************************************************************************/
const void* IMetadata::Content::getBasicData(MINT index, MINT32 type) const
{
    if( index <0 || index >= (int)mCount) {
        META_LOGE("index (%d) out of range (%d)",(int)index, (int)mCount);
        return nullptr;
    }
    if(type != mType) {
        META_LOGE_CALLSTACK("type mismatch type(%x) vs mType(%x)",(int)type, (int)mType);
        return nullptr;
    }

    auto ts = _typeSize();
    if(mBasicDataSP) {
        auto dest = (MUINT8*) mBasicDataSP->array();
        return (const void*)(dest + index * ts);
    }
    else {
        META_ASSERT(mCount==1, "mCount >1 but mBasicDataSP is nullptr");
        return (const void*) &mImmData;
    }
}
/******************************************************************************
 * IMetadata::Content::getMetadata
 ******************************************************************************/
IMetadata::StorageSP IMetadata::Content::getMetadata(MINT index, MINT32 type) const
{
    if( index <0 || index >= (int)mCount) {
        META_LOGE("index (%d) out of range (%d)",(int)index, (int)mCount);
        return nullptr;
    }
    if(type != mType) {
        META_LOGE_CALLSTACK("type mismatch type(%x) vs mType(%x)",(int)type, (int)mType);
        return nullptr;
    }

    if(mMetaSP) {
        META_ASSERT( mType == TYPE_IMetadata, "mMetaSP without TYPE_IMetadata");
        return (*mMetaSP)[index];
    }
    return nullptr;
}
/******************************************************************************
 * IMetadata::Content::getMemory
 ******************************************************************************/
IMetadata::BasicDataSP IMetadata::Content::getMemory(MINT index, MINT32 type) const
{
    if( index <0 || index >= (int)mCount) {
        META_LOGE("index (%d) out of range (%d)",(int)index, (int)mCount);
        return nullptr;
    }
    if(type != mType) {
        META_LOGE_CALLSTACK("type mismatch type(%x) vs mType(%x)",(int)type, (int)mType);
        return nullptr;
    }

    if(mMemSP) {
        META_ASSERT( mType == TYPE_Memory, "mMemSP without TYPE_Memory");
        META_LOGV(" index %d, result %p", (int)index, (*mMemSP)[index].get());
        return (*mMemSP)[index];
    }
    return nullptr;
}
#endif //MTMCAM_METADATA_V2
