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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/**
 * @file PipeBufferPoolMgr.cpp
 * @brief BufferPoolMgr for ThirdParty pipe
*/

// Standard C header file

// Android system/core header file
#include <ui/gralloc_extra.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
#include <mtkcam/def/ImageFormat.h>
// Module header file
// Local header file
#include "PipeBufferPoolMgr.h"
#include "PipeBufferHandler.h"

// Logging header file
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "PipeBufferPoolMgr"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using namespace NSIoPipe::NSPostProc;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PipeBufferPoolMgr::
PipeBufferPoolMgr(sp<IspPipeSetting> pPipeSetting)
: mNodeCount(0)
{

}

PipeBufferPoolMgr::
~PipeBufferPoolMgr()
{
    uninit();
}


MVOID
PipeBufferPoolMgr::
setNodeCount(MINT32 nodeCount)
{
    MINT32 old = mNodeCount;
    mNodeCount = nodeCount;
    MY_LOGD("set node count, old:%d, new:%d", old, mNodeCount);
}


MINT32
PipeBufferPoolMgr::
getNodeCount() const
{
    return mNodeCount;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PipeBufferPoolMgr::
uninit()
{
    for(size_t id = 0; id < mBIDtoImgBufPoolMap.size(); id++)
    {
        sp<PipeImgBufferPool>& pool = mBIDtoImgBufPoolMap.editValueAt(id);
        MY_LOGD("Destory buffer pool, id:%d, addr:%p", id, pool.get());
        PipeImgBufferPool::destroy(pool);
    }
    mBIDtoImgBufPoolMap.clear();
    for(size_t id = 0; id < mBIDtoIndividualImgBufPoolMap.size(); id++)
    {
        sp<ImageBufferPool>& pool = mBIDtoIndividualImgBufPoolMap.editValueAt(id);
        MY_LOGD("Destory buffer pool, id:%d, addr:%p", id, pool.get());
        ImageBufferPool::destroy(pool);
    }
    mBIDtoIndividualImgBufPoolMap.clear();
    mBIDtoBufferTyepMap.clear();

    for(size_t id = 0; id < mBIDtoTuningBufferPoollMap.size(); id++)
    {
        sp<TuningBufferPool>& pool = mBIDtoTuningBufferPoollMap.editValueAt(id);

        MY_LOGD("Destory turning buffer pool, id:%d, addr:%p", id, pool.get());
         TuningBufferPool::destroy(pool);
    }
    mBIDtoTuningBufferPoollMap.clear();

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PipeBufferPoolMgr::
addPool(IspPipeBufferID id, sp<PipeImgBufferPool> pool)
{
    if(mBIDtoImgBufPoolMap.indexOfKey(id) >= 0)
    {
        MY_LOGE("Failed to add buffer pool, buffer id(%d)is existing", id);
    }
    else
    {
        mBIDtoImgBufPoolMap.add(id, pool);
        mBIDtoBufferTyepMap.add(id, eBUFFER_IMAGE);
        MY_LOGD("Add buffer pool, buffer, id:%d, addr:%p", id, pool.get());
    }
}

MVOID
PipeBufferPoolMgr::
addPool(IspPipeBufferID id, sp<ImageBufferPool> pool)
{
    if(mBIDtoIndividualImgBufPoolMap.indexOfKey(id) >= 0)
    {
        MY_LOGE("Failed to add buffer pool, buffer id(%d)is existing", id);
    }
    else
    {
        mBIDtoIndividualImgBufPoolMap.add(id, pool);
        mBIDtoBufferTyepMap.add(id, eBUFFER_IMAGE);
        MY_LOGD("Add buffer pool, buffer, id:%d, addr:%p", id, pool.get());
    }
}


MVOID
PipeBufferPoolMgr::
addTBPool(IspPipeBufferID id, sp<TuningBufferPool> pool)
{
    if(mBIDtoTuningBufferPoollMap.indexOfKey(id) >= 0)
    {
        MY_LOGE("Failed to add tuning buffer pool, buffer id(%d)is existing", id);
    }
    else
    {
        mBIDtoTuningBufferPoollMap.add(id, pool);
        MY_LOGD("Add tuning buffer pool, buffer, id:%d, addr:%p", id, pool.get());
    }
}

SmartPipeImgBuffer
PipeBufferPoolMgr::
request(IspPipeBufferID id)
{
    ssize_t index;
    if((index=mBIDtoImgBufPoolMap.indexOfKey(id)) >= 0)
    {
        sp<PipeImgBufferPool> pBufferPool = mBIDtoImgBufPoolMap.valueAt(index);
        return pBufferPool->request();
    }
    else
    {
        MY_LOGD("Cannot find the desired buffer pool, buffer id:%d", id);
        return nullptr;
    }
}

SmartImageBuffer
PipeBufferPoolMgr::
requestIB(IspPipeBufferID id)
{
    ssize_t index;
    if((index=mBIDtoIndividualImgBufPoolMap.indexOfKey(id)) >= 0)
    {
        sp<ImageBufferPool> pBufferPool = mBIDtoIndividualImgBufPoolMap.valueAt(index);
        return pBufferPool->request();
    }
    else
    {
        MY_LOGD("Cannot find the desired buffer pool, buffer id:%d", id);
        return nullptr;
    }
}

SmartGraphicBuffer
PipeBufferPoolMgr::
requestGB(IspPipeBufferID id)
{
    return nullptr;
}

SmartTuningBuffer
PipeBufferPoolMgr::
requestTB(IspPipeBufferID id)
{
    ssize_t index;
    if((index=mBIDtoTuningBufferPoollMap.indexOfKey(id)) >= 0)
    {
        sp<TuningBufferPool> pBufferPool = mBIDtoTuningBufferPoollMap.valueAt(index);
        return pBufferPool->request();
    }
    else
    {
        MY_LOGD("Cannot find the desired tuning buffer pool, buffer id:%d", id);
        return nullptr;
    }
}

sp<PipeBufferHandler>
PipeBufferPoolMgr::
createBufferHandler()
{
    return new PipeBufferHandler(this);
}

IspBufferType
PipeBufferPoolMgr::
queryBufferType(
    IspPipeBufferID bid
)
{
    ssize_t index;
    if((index = mBIDtoBufferTyepMap.indexOfKey(bid)) >= 0)
    {
        return mBIDtoBufferTyepMap.valueAt(index);
    }
    return eBUFFER_INVALID;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

