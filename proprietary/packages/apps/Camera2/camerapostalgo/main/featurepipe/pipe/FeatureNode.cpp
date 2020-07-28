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
 * MediaTek Inc. (C) 2018. All rights reserved.
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


#include "core/DebugControl.h"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_NODE
#define PIPE_CLASS_TAG "Node"
#include <core/PipeLog.h>

#include <pipe/FeatureNode.h>


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

const char* FeatureDataHandler::ID2Name(DataID id)
{
    return PathID2Name(id);
}

NodeSignal::NodeSignal()
    : mSignal(0)
    , mStatus(0)
{
}

NodeSignal::~NodeSignal()
{
}

MVOID NodeSignal::setSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal |= signal;
    mCondition.broadcast();
}

MVOID NodeSignal::clearSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal &= ~signal;
}

MBOOL NodeSignal::getSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    return (mSignal & signal);
}

MVOID NodeSignal::waitSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    while( !(mSignal & signal) )
    {
        mCondition.wait(mMutex);
    }
}

MVOID NodeSignal::setStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus |= status;
}

MVOID NodeSignal::clearStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus &= ~status;
}

MBOOL NodeSignal::getStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    return (mStatus & status);
}

FeatureDataHandler::~FeatureDataHandler()
{
}

FeatureNode::FeatureNode(NodeID_T nid, const char *name, MUINT32 uLogLevel, MINT32 policy, MINT32 priority)
    : CamThreadNode(name, policy, priority)
    , mSensorIndex(-1)
    , mSensorIndex2(-1)
    , mNodeId(nid)
    , mLogLevel(uLogLevel)
{
}

FeatureNode::~FeatureNode()
{
}

MBOOL FeatureNode::onInit()
{
    return MTRUE;
}

MVOID FeatureNode::setLogLevel(MUINT32 uLogLevel)
{
    mLogLevel = uLogLevel;
}

MVOID FeatureNode::setNodeSignal(const android::sp<NodeSignal> &nodeSignal)
{
    mNodeSignal = nodeSignal;
}


MVOID FeatureNode::setUsageHint(const IFeaturePipe::UsageHint &rUsageHint)
{
    mUsageHint = rUsageHint;
}

MVOID FeatureNode::setTaskQueue(const android::sp<FeatureTaskQueue>& pTaskQueue)
{
    mpTaskQueue = pTaskQueue;
}


MVOID FeatureNode::dispatch(const RequestPtr &pRequest, NodeID_T nodeId)
{
    if (nodeId == NULL_NODE)
        nodeId = getNodeID();

    Vector<NodeID_T> vNextNodes = pRequest->getNextNodes(nodeId);

    pRequest->stopTimer(nodeId);
    pRequest->lock();
    pRequest->finishNode_Locked(nodeId);
    for (NodeID_T nextNode : vNextNodes)
    {
        MY_LOGD_IF(mLogLevel, "find path=%s  %s", NodeID2Name(nodeId), NodeID2Name(nextNode));
        PathID_T pathId = FindPath(nodeId, nextNode);
        if (pathId != NULL_PATH) {
            pRequest->startTimer(nextNode);
            pRequest->finishPath_Locked(pathId);
            handleData(pathId, pRequest);
            MY_LOGD_IF(mLogLevel, "Goto %s", PathID2Name(pathId));
        }
    }

    if (vNextNodes.size() == 0 && pRequest->isFinished_Locked())
    {
        handleData(PID_DEQUE, pRequest);
        pRequest->unlock();
        return;
    }
    pRequest->unlock();
}

MBOOL FeatureNode::dumpData(RequestPtr &request, IImageBuffer *buffer, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer && fmt )
    {
        char name[256];
        va_list ap;
        va_start(ap, fmt);
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);
        ret = dumpNamedData(request, buffer, name);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FeatureNode::dumpNamedData(RequestPtr &request, IImageBuffer *buffer, const char *name)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer && name )
    {
        MUINT32 stride, pbpp, ibpp, width, height, size;
        stride = buffer->getBufStridesInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        size = buffer->getBufSizeInBytes(0);
        pbpp = pbpp ? pbpp : 8;
        width = stride * 8 / pbpp;
        width = width ? width : 1;
        ibpp = ibpp ? ibpp : 8;
        height = size / width;
        if( buffer->getPlaneCount() == 1 )
        {
          height = height * 8 / ibpp;
        }

        char path[256];
        snprintf(path, sizeof(path), "/sdcard/dump/%04d_%s_%dx%d.bin", request->getRequestNo(), name, width, height);

        TRACE_FUNC("dump to %s", path);
        buffer->saveToFile(path);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 FeatureNode::dumpData(const char *buffer, MUINT32 size, const char *filename)
{
    uint32_t writeCount = 0;
    int fd = ::open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if( fd < 0 )
    {
        MY_LOGE("Cannot create file [%s]", filename);
    }
    else
    {
        for( int cnt = 0, nw = 0; writeCount < size; ++cnt )
        {
            nw = ::write(fd, buffer + writeCount, size - writeCount);
            if( nw < 0 )
            {
                MY_LOGE("Cannot write to file [%s]", filename);
                break;
            }
            writeCount += nw;
        }
        ::close(fd);
    }
    return writeCount;
}

MBOOL FeatureNode::loadData(IImageBuffer *buffer, const char *filename)
{
    MBOOL ret = MFALSE;
    if( buffer )
    {
        loadData((char*)buffer->getBufVA(0), 0, filename);
        ret = MTRUE;
    }
    return MFALSE;
}

MUINT32 FeatureNode::loadData(char *buffer, size_t size, const char *filename)
{
    uint32_t readCount = 0;
    int fd = ::open(filename, O_RDONLY);
    if( fd < 0 )
    {
        MY_LOGE("Cannot open file [%s]", filename);
    }
    else
    {
        if( size == 0 )
        {
            off_t readSize = ::lseek(fd, 0, SEEK_END);
            size = (readSize < 0) ? 0 : readSize;
            ::lseek(fd, 0, SEEK_SET);
        }
        for( int cnt = 0, nr = 0; readCount < size; ++cnt )
        {
            nr = ::read(fd, buffer + readCount, size - readCount);
            if( nr < 0 )
            {
                MY_LOGE("Cannot read from file [%s]", filename);
                break;
            }
            readCount += nr;
        }
        ::close(fd);
    }
    return readCount;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

