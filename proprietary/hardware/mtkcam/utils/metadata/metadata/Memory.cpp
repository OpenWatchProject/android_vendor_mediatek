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
#define LOG_TAG "Metadata2/Memory"
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
//

#ifdef MTKCAM_METADATA_V2
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

/******************************************************************************
 * IMetadata::Memory constructor/destructor
 ******************************************************************************/
IMetadata::Memory::Memory()
{
    META_LOGV("Memory()");
}


IMetadata::Memory::Memory(const BasicDataSP& src)
{
    META_LOGV("Memory(BasicDataSP)");
    mDataSP = src; // reuse other's data
}

IMetadata::Memory::Memory(const void *data, size_t size)
{
    META_LOGV("Memory(%p, size %d)", data, (int)size);
    if(data !=nullptr && size >0) {
        mDataSP = std::make_shared<BasicData>();
        mDataSP->appendArray((MUINT8*)data, size);
    }
}

IMetadata::Memory::Memory(const Memory &other): Memory(other.getDataSP()) {}
IMetadata::Memory::Memory(Memory &&other) : Memory( (const Memory &)other) { }

IMetadata::Memory::~Memory()
{
}
/******************************************************************************
 * IMetadata::Memory::getDataSP
 *      provide data
 *      become readonly mode
 ******************************************************************************/
const IMetadata::BasicDataSP IMetadata::Memory::getDataSP() const
{
    return mDataSP;
}
/******************************************************************************
 * IMetadata::Memory::size()
 ******************************************************************************/
size_t IMetadata::Memory::size() const
{
    if(!mDataSP) return 0; // no data
    return mDataSP->size();
}


/******************************************************************************
 * IMetadata::Memory::resize()
 ******************************************************************************/
void IMetadata::Memory::resize(const size_t size)
{
    if(!mDataSP)
        mDataSP = std::make_shared<BasicData>(); // create with size
    else if(mDataSP.use_count() >1) // clone
        mDataSP = std::make_shared<BasicData>(*mDataSP);

    mDataSP->resize(size);
}


/******************************************************************************
 * IMetadata::Memory::append()
 ******************************************************************************/
size_t IMetadata::Memory::append(const Memory &other)
{
    auto srcSP = other.getDataSP(); // query other's data and make it readonly

    if(!mDataSP) { // default is no data
        mDataSP = srcSP; // use read only mode to share with others
        return mDataSP->size();
    }
    else {
        if(mDataSP.use_count() >1) // clone
            mDataSP = std::make_shared<BasicData>(*mDataSP);

        auto d = mDataSP.get();
        auto os = d->size();
        auto s = srcSP->size();
        d->resize(os+s);
        memcpy(d->editArray()+os, srcSP->array(), s);
        return os+s;
    }
}

size_t IMetadata::Memory::appendVector(const Memory &other)
{
    return this->append(other);
}


size_t IMetadata::Memory::appendVector(const android::Vector<MUINT8> &v)
{
    // Android::Vector performance better than std::vector


    if(!mDataSP){ // default is no data
        mDataSP = std::make_shared<BasicData>(v); // clone
        return mDataSP->size();
    }
    else {
        if(mDataSP.use_count() >1)// clone
            mDataSP = std::make_shared<BasicData>(*mDataSP);

        return (size_t) mDataSP->appendVector(v);
    }
}


/******************************************************************************
 * IMetadata::Memory::array()
 *      directly read array
 ******************************************************************************/
const uint8_t* IMetadata::Memory::array(size_t &retSize) const
{
    if(mDataSP) {
        retSize = mDataSP->size();
        return mDataSP->array();
    }
    retSize = 0;
    return nullptr;
}
const uint8_t* IMetadata::Memory::array() const
{
    if(mDataSP)
        return mDataSP->array();
    return nullptr;
}

/******************************************************************************
 * IMetadata::Memory::editArray()
 *      directly modify array
 ******************************************************************************/
uint8_t* IMetadata::Memory::editArray(size_t &retSize)
{
    if(mDataSP) {
        if(mDataSP.use_count() > 1) // clone for write
            mDataSP = std::make_shared<BasicData>(*mDataSP);

        retSize = mDataSP->size();
        return mDataSP->editArray();
    }
    retSize = 0;
    return nullptr;
}
uint8_t* IMetadata::Memory::editArray()
{
    if(mDataSP) {
        if(mDataSP.use_count() > 1) // clone for write
            mDataSP = std::make_shared<BasicData>(*mDataSP);
        return mDataSP->editArray();
    }
    return nullptr;
}

/******************************************************************************
 * IMetadata::Memory::itemAt()
 *      get byte data
 ******************************************************************************/
uint8_t IMetadata::Memory::itemAt(size_t index) const
{
    size_t size;
    auto p = array(size);
    if(p && index <size)
        return p[index];
    return 0;
}

/******************************************************************************
 * IMetadata::Memory::clear()
 ******************************************************************************/
void IMetadata::Memory::clear()
{
    mDataSP = nullptr;
}

/******************************************************************************
 * IMetadata::Memory::operator = ()
 *      assignment operation
 ******************************************************************************/
IMetadata::Memory& IMetadata::Memory::operator = (const Memory &other)
{
    if(this != &other)
        mDataSP = other.getDataSP(); // get other's data , make it become readonly

    return *this;
}

IMetadata::Memory& IMetadata::Memory::operator = (Memory &&other) // same implement
{
    if(this != &other)
        mDataSP = other.getDataSP(); // get other's data , make it become readonly
    return *this;
}
/******************************************************************************
 * IMetadata::Memory::operator  compare operation
 ******************************************************************************/
bool IMetadata::Memory::operator == (const Memory &other) const
{
    if(this == &other) return true;

    auto otherSP = other.getDataSP(); // get other's data , make it become readonly

    size_t size1=0,size2=0;
    if(mDataSP) size1 = mDataSP->size();
    if(otherSP) size2 = otherSP->size();

    if(size1 != size2) return false; // size different
    if(size1 == 0) return true;// two are 0 length data, maybe nullptr

    return 0 == std::memcpy((void*)mDataSP->array(),(void*)otherSP->array(),size1);
}

bool IMetadata::Memory::operator != (const Memory &other) const
{
    return !(this->operator == (other));
}


#endif //MTKCAM_METADATA_V2
