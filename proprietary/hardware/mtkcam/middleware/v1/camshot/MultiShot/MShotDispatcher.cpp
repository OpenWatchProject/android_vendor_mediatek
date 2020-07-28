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

#define LOG_TAG "MtkCam/MShotDispatcher"
//
#include <utils/RefBase.h>
#include <mtkcam/utils/std/Log.h>
#include "MShotDispatcher.h"
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
MShotDispatcher::
MShotDispatcher(wp<MShotDispatcher::IDispatcherCallback> pCb)
    : mInFlush(MFALSE)
    , mJpegInUse(MFALSE)
    , mpCb(pCb)
{
    mFrameWaiting.mFrame = NULL;
    mFrameWaiting.mStatus = statusNone;
    mFrameWaiting.mNodeId = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MShotDispatcher::
beginFlush()
{
    FUNC_START;
    RWLock::AutoWLock _l(mDispatcherLock);
    mInFlush = MTRUE;
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MShotDispatcher::
endFlush()
{
    FUNC_START;
    RWLock::AutoWLock _l(mDispatcherLock);
    mInFlush = MFALSE;
    if( mFrameWaiting.mFrame != NULL )
    {
        MY_LOGE("waiting frame not flushed");
    }
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MShotDispatcher::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame,
    Pipeline_NodeId_T nodeId
)
{
    RWLock::AutoWLock _l(mDispatcherLock);
    MY_LOGD("[frameNo:%d] from node %#" PRIxPTR, pFrame->getFrameNo(), nodeId);
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == NULL || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return;
    }

    if( nodeId == eNODEID_JpegNode )
    {
        mJpegInUse = MFALSE;
    }

    Vector< FrameInfo > vFrame;
    if( !mInFlush )
    {
        if( nodeId == eNODEID_P2Node )
        {
            sp<MShotDispatcher::IDispatcherCallback> spCb = mpCb.promote();
            if( spCb.get() )
            {
                spCb->onFrameNotify(pFrame->getFrameNo(), nodeId);
            }
            if( !mJpegInUse )
            {
                if( mFrameWaiting.mFrame == NULL )
                {
                    vFrame.push_back( FrameInfo(pFrame, statusToRun, nodeId) );
                    mJpegInUse = MTRUE;
                }
                else
                {
                    MY_LOGE("someone waiting but no one running, should not happen");
                    return;
                }
            }
            else
            {
                if( mFrameWaiting.mFrame == NULL )
                {
                    mFrameWaiting.mFrame = pFrame;
                    mFrameWaiting.mStatus = statusWaiting;
                    mFrameWaiting.mNodeId = nodeId;
                }
                else
                {
                    vFrame.push_back( FrameInfo(mFrameWaiting.mFrame, statusToFlush, mFrameWaiting.mNodeId) );
                    mFrameWaiting.mFrame = pFrame;
                    mFrameWaiting.mStatus = statusWaiting;
                    mFrameWaiting.mNodeId = nodeId;
                }
            }
        }

        if( nodeId == eNODEID_JpegNode )
        {
            vFrame.push_back( FrameInfo(pFrame, statusToRun, nodeId) );
            if( mFrameWaiting.mFrame != NULL )
            {
                vFrame.push_back( FrameInfo(mFrameWaiting.mFrame, statusToRun, mFrameWaiting.mNodeId) );
                mJpegInUse = MTRUE;
                mFrameWaiting.mFrame = NULL;
                mFrameWaiting.mStatus = statusNone;
                mFrameWaiting.mNodeId = 0;
            }
        }
    }
    else
    {
        vFrame.push_back( FrameInfo(pFrame, statusToFlush, nodeId) );
        if( mFrameWaiting.mFrame != NULL )
        {
            vFrame.push_back( FrameInfo(mFrameWaiting.mFrame, statusToFlush, mFrameWaiting.mNodeId) );
            mFrameWaiting.mFrame = NULL;
            mFrameWaiting.mStatus = statusNone;
            mFrameWaiting.mNodeId = 0;
        }
    }

    Vector< FrameInfo >::iterator frameIter;
    for( frameIter = vFrame.begin() ; frameIter != vFrame.end() ; frameIter++ )
    {
        IPipelineDAG::NodeObjSet_T nextNodes;
        MERROR err = frameIter->mFrame->getPipelineDAG().getOutAdjacentNodes(frameIter->mNodeId, nextNodes);
        if  ( ! err && ! nextNodes.empty() )
        {
            for (size_t i = 0; i < nextNodes.size(); i++) {
                sp<IPipelineNode> pNextNode = pPipelineNodeMap->nodeAt(nextNodes[i].val);
                if  ( pNextNode != NULL ) {
                    MY_LOGD("[frameNo:%d] status:%d -> node %#" PRIxPTR, frameIter->mFrame->getFrameNo(), frameIter->mStatus, pNextNode->getNodeId());
                    if ( frameIter->mStatus == statusToFlush ) {
                        pNextNode->flush(frameIter->mFrame);
                    }
                    else if( frameIter->mStatus == statusToRun )
                    {
                        pNextNode->queue(frameIter->mFrame);
                    }
                }
            }
        }
    }
    vFrame.clear();
    return;
}

