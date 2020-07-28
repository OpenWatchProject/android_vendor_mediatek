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

/**
 * @file DataStorage.h
 * @brief storage for queued buffer
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DATABUFFER_STORAGE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DATABUFFER_STORAGE_H

// Standard C header file
#include <queue>
// Android system/core header file
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
// Local header file
#include "./bufferPoolMgr/BufferPool.h"
#include "DepthMapPipe_Common.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DataStorage"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe_DepthMap{

using namespace android;

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class DataStorage
 * @brief storage queue for data
 */

template <typename T>
class DataStorage : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DataStorage() {}
    ~DataStorage() { while(!mBufferQueue.empty()) mBufferQueue.pop(); }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataStorage Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief push the buffer into the last pos of storage
     * @param [in] buffer buffer to be push
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL push_back(T buffer)
    {
        RWLock::AutoWLock _l(mLock);
        mBufferQueue.push(buffer);
        return MTRUE;
    }
    /**
     * @brief pop the oldest buffer
     * @param [out] buffer popped buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL pop_front(T& buffer)
    {
        RWLock::AutoWLock _l(mLock);
        if(mBufferQueue.empty())
            return MFALSE;
        buffer = mBufferQueue.front();
        mBufferQueue.pop();
        return MTRUE;
    }

    /**
     * @brief pop the oldest buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL pop_front()
    {
        RWLock::AutoWLock _l(mLock);
        if(mBufferQueue.empty())
            return MFALSE;
        mBufferQueue.pop();
        return MTRUE;
    }

    /**
     * @brief get the oldest buffer
     * @param [out] buffer popped buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL front(T& buffer)
    {
        RWLock::AutoRLock _l(mLock);
        if(mBufferQueue.empty())
            return MFALSE;
        buffer = mBufferQueue.front();
        return MTRUE;
    }

    /**
     * @brief get the buffer size
     * @return
     * - buffer size
     */
    MUINT32 size()
    {
        RWLock::AutoRLock _l(mLock);
        return mBufferQueue.size();
    }

    /**
     * @brief get the stored singleton data
     * @param [out] stored data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL getStoredData(T& data)
    {
        RWLock::AutoRLock _l(mLock);
        if(!mbReady)
            return MFALSE;
        data = mSingleData;
        return MTRUE;
    }

    /**
     * @brief set the stored singleton data
     * @param [out] stored data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL setStoredData(T data)
    {
        RWLock::AutoWLock _l(mLock);
        mSingleData = data;
        mbReady = MTRUE;
        return MTRUE;
    }

    /**
     * @brief flush all the data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL flush()
    {
        RWLock::AutoRLock _l(mLock);
        while(!mBufferQueue.empty()) mBufferQueue.pop();
        return MTRUE;
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataStorage Private Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataStorage Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    RWLock mLock;
    // queue to store buffer
    std::queue<T> mBufferQueue;
    // used in single-existance data
    T mSingleData;
    MBOOL mbReady = MFALSE;
};

/**
 * @struct DepthBufferInfo
 * @brief data to be queued during flow type:eDEPTH_FLOW_TYPE_QUEUED_DEPTH
 */

struct DepthBufferInfo
{
    DepthBufferInfo() : miReqIdx(0), mfDistance(0) {}
    ~DepthBufferInfo() {mpDMBGBuffer = nullptr;
                        mpDepthBuffer = nullptr;}

    SmartFatImageBuffer mpDMBGBuffer = nullptr;
    SmartFatImageBuffer mpDepthBuffer = nullptr;
    MUINT32 miReqIdx;
    MFLOAT mfDistance;
};

typedef DataStorage<DepthBufferInfo> DepthInfoStorage;
};
};
};

#endif
