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

#include "TPIUsage.h"
#include "StreamingFeature_Common.h"
#include "tpi/TPIMgr_Util.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "TPIUsage"
#define PIPE_TRACE TRACE_TPI_USAGE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const char* toName(TPIOEntry entry)
{
    switch(entry)
    {
    case TPIOEntry::YUV:      return "YUV";
    case TPIOEntry::RAW:      return "RAW";
    case TPIOEntry::ASYNC:    return "ASYNC";
    case TPIOEntry::DISP:     return "DISP";
    case TPIOEntry::META:     return "META";
    default:                  return "UNKNOWN";
    };
}

const char* toName(TPIOUse use)
{
    switch(use)
    {
    case TPIOUse::IN_YUV:       return "USE_IN_YUV";
    case TPIOUse::IN_DEPTH:     return "USE_IN_DEPTH";
    case TPIOUse::IN_PURE:      return "USE_IN_PURE";
    case TPIOUse::OUT_DISPLAY:  return "USE_OUT_DISPLAY";
    case TPIOUse::OUT_RECORD:   return "USE_OUT_RECORD";
    case TPIOUse::INPLACE:      return "USE_INPLACE";
    case TPIOUse::DIV:          return "USE_DIV";
    case TPIOUse::INPUT_CROP:   return "USE_INPUT_CROP";
    case TPIOUse::SHARED_INPUT: return "USE_SHARED_INPUT";
    };
}

TPIUsage::TPIUsage()
{
}

MVOID TPIUsage::config(const TPIMgr *mgr)
{
    mSession = TPI_Session();
    mNodeMap.clear();
    mGroupMap.clear();

    if( mgr && mgr->getSessionInfo(mSession) )
    {
        configNodeMap(mSession, mNodeMap);
        configEntry(mSession, mNodeMap, TPIOEntry::YUV,
            TPI_NODE_ID_MTK_S_YUV, TPI_NODE_ID_MTK_S_YUV_OUT);
        configEntry(mSession, mNodeMap, TPIOEntry::DISP,
            TPI_NODE_ID_MTK_S_DISP_ONLY, TPI_NODE_ID_MTK_S_DISP_ONLY_OUT);
        configEntry(mSession, mNodeMap, TPIOEntry::ASYNC,
            TPI_NODE_ID_MTK_S_ASYNC, TPI_NODE_ID_MTK_S_ASYNC_OUT);
        configEntry(mSession, mNodeMap, TPIOEntry::META,
            TPI_NODE_ID_MTK_S_META_ONLY, TPI_NODE_ID_MTK_S_META_ONLY_OUT);
    }

}

MBOOL TPIUsage::useEntry(TPIOEntry entry) const
{
    MBOOL ret = MFALSE;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        ret = MTRUE;
    }
    return ret;
}

TPIO TPIUsage::getTPIO(TPIOEntry entry, MUINT32 index) const
{
    TPIO io;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        if( index < it->second.mNodes.size() )
        {
            io = it->second.mNodes[index];
        }
    }
    return io;
}

MUINT32 TPIUsage::getNodeCount(TPIOEntry entry) const
{
    MUINT32 ret = 0;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        ret = it->second.mNodes.size();
    }
    return ret;
}

MBOOL TPIUsage::use(TPIOEntry entry, TPIOUse use) const
{
    MBOOL ret = MFALSE;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        ret = it->second.mUseSet.count(use);
    }
    return ret;
}

EImageFormat TPIUsage::getCustomFormat(TPIOEntry entry, EImageFormat original) const
{
    EImageFormat fmt = original;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        fmt = toValid(it->second.mFormat, original);
    }
    return fmt;
}

MSize TPIUsage::getCustomSize(TPIOEntry entry, const MSize &original) const
{
    MSize size = original;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        size = toValid(it->second.mSize, original);
    }
    return size;
}

MUINT32 TPIUsage::getNumInBuffer(TPIOEntry entry) const
{
    MUINT32 num = 0;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        num = it->second.mNumInBuffer;
    }
    return num;
}

MUINT32 TPIUsage::getNumOutBuffer(TPIOEntry entry) const
{
    MUINT32 num = 0;
    auto it = mGroupMap.find(entry);
    if( it != mGroupMap.end() && it->second.mEnable )
    {
        num = it->second.mNumOutBuffer;
    }
    return num;
}

MVOID TPIUsage::configNodeMap(const TPI_Session &session, TPIOMap &map)
{
    for( unsigned i = 0; i < session.mNodeInfoListCount; ++i )
    {
        map[session.mNodeInfoList[i].mNodeID] = session.mNodeInfoList[i];
    }
    for( unsigned i = 0; i < session.mPathInfoListCount; ++i )
    {
        TPI_PathInfo path = session.mPathInfoList[i];
        map[path.mDst.mNode].addPrev(path);
        map[path.mSrc.mNode].addNext(path);
    }
}

MVOID TPIUsage::printEntry(TPIOEntry entry, const TPIOGroup &group)
{
    MY_LOGD("entry(%d:%s) enable=%d, node=%zu",
      entry, toName(entry), group.mEnable, group.mNodes.size());
}

MVOID TPIUsage::configEntry(const TPI_Session &session, TPIOMap &map, TPIOEntry entry, TPI_NODE_ID inID, TPI_NODE_ID outID)
{
    if( map.count(inID) && map.count(outID) )
    {
        TPIOGroup &group = mGroupMap[entry];
        group.mEntry = map[inID];
        group.mNodes.reserve(session.mNodeInfoListCount);

        TPI_NODE_ID index = inID;
        while( index != outID )
        {
            auto it = map.find(index);
            MY_LOGD("target=0x%x(%s) find=%d", index, toNodeName(index), it != map.end());
            if( it != map.end() )
            {
                MY_LOGD("travelCount=%d prev=%zu next=%zu", it->second.mTravelCount, it->second.mPrevPortMap.size(), it->second.mNextPortMap.size());
            }
            if( it == map.end() ||
                it->second.mTravelCount ||
                !it->second.mNextPortMap.size() )
            {
                break;
            }
            it->second.mTravelCount += 1;
            if( it->first != inID )
            {
                MY_LOGD("push_back 0x%x", it->first);
                group.mNodes.push_back(it->second);
            }
            if( !it->second.mNextPortMap.begin()->second.size() )
            {
                break;
            }
            index = it->second.mNextPortMap.begin()->second.begin()->mNode;
        }
        if( group.mNodes.size() > MAX_TPI_COUNT )
        {
            MY_LOGW("number of TPI(%zu) exceeds limit(%d), skip all",
                    group.mNodes.size(), MAX_TPI_COUNT);
        }
        else if( index == outID && group.mNodes.size() )
        {
            group.mEnable = MTRUE;
            setUseOption(entry, group);
            setBufferNum(entry, group);
        }
        printEntry(entry, group);
    }
}

MVOID TPIUsage::setUseOption(TPIOEntry entry, TPIOGroup &group)
{
    (void)entry;
    group.mUseSet.clear();
    if( hasNodeOption(group.mEntry, TPI_NODE_OPT_INPLACE) )
    {
        group.mUseSet.insert(TPIOUse::INPLACE);
    }
    if( hasNodeOption(group.mEntry, TPI_NODE_OPT_DIV) )
    {
        group.mUseSet.insert(TPIOUse::DIV);
        if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_DIV_RECORD) )
        {
            group.mUseSet.insert(TPIOUse::OUT_RECORD);
        }
        if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_DIV_DISPLAY) )
        {
            group.mUseSet.insert(TPIOUse::OUT_DISPLAY);
        }
    }
    if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_YUV) )
    {
        group.mUseSet.insert(TPIOUse::IN_YUV);
        EImageFormat fmt = eImgFmt_UNKNOWN;
        MSize size;
        getBufferInfo(group.mEntry.mNodeInfo, TPI_BUFFER_ID_MTK_OUT_YUV, fmt, size);
        if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_CUSTOM_FORMAT) )
        {
            group.mFormat = fmt;
        }
        if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_CUSTOM_SIZE) )
        {
            group.mSize = size;
        }
    }
    if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_DEPTH) )
    {
        group.mUseSet.insert(TPIOUse::IN_DEPTH);
    }
    if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_NEED_CROP_IN) )
    {
        group.mUseSet.insert(TPIOUse::INPUT_CROP);
    }
    if( hasCustomOption(group.mEntry, TPI_MTK_OPT_STREAMING_SHARED_INPUT) )
    {
        group.mUseSet.insert(TPIOUse::SHARED_INPUT);
    }
}

MVOID TPIUsage::setBufferNum(TPIOEntry entry, TPIOGroup &group)
{
    if( entry == TPIOEntry::YUV )
    {
        bool inplace = hasNodeOption(group.mEntry, TPI_NODE_OPT_INPLACE);
        group.mTotalQueueCount = 0;
        for( const TPIO &io : group.mNodes )
        {
            group.mTotalQueueCount += io.mNodeInfo.mQueueCount;
        }
        unsigned numBasic = 3 + group.mNodes.size();
        group.mNumInBuffer = numBasic + group.mTotalQueueCount;
        group.mNumOutBuffer = inplace ? 0 : numBasic;
    }
    else if( entry == TPIOEntry::ASYNC )
    {
        group.mNumInBuffer  = 3;
        group.mNumOutBuffer = 0;
    }
    else if( entry == TPIOEntry::META )
    {
        group.mNumInBuffer  = 0;
        group.mNumOutBuffer = 0;
    }
    else if( entry == TPIOEntry::DISP )
    {
        group.mNumInBuffer = 0;
        group.mNumOutBuffer = 0;
    }
}

MBOOL TPIUsage::hasNodeOption(const TPIO &io, unsigned nodeOption)
{
    return (io.mNodeInfo.mNodeOption & nodeOption);
}

MBOOL TPIUsage::hasCustomOption(const TPIO &io, unsigned customOption)
{
    return (io.mNodeInfo.mCustomOption & customOption);
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam
