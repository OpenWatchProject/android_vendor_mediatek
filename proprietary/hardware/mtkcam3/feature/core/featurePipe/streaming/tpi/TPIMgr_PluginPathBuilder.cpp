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

#include "TPIMgr_PluginWrapper.h"
#include "TPIMgr_Util.h"

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include "DebugControl.h"
#define PIPE_CLASS_TAG "TPIMGR_Plugin"
#define PIPE_TRACE TRACE_TPI_MGR_PLUGIN
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using namespace NSCam::NSPipelinePlugin;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

static bool isValidFormat(const TPI_BufferInfo_t &buffer)
{
    return buffer.mFormat != NSCam::eImgFmt_UNKNOWN;
}

static bool isValidSize(const TPI_BufferInfo_t &buffer)
{
    return buffer.mSize.w > 0 && buffer.mSize.h > 0;
}

class PluginPathBuilder
{
private:
    using PathType = TPIMgr_PluginWrapper::PathType;
    using T_Selection = TPIMgr_PluginWrapper::T_Selection;
    using T_NodeID = TPIMgr_PluginWrapper::T_NodeID;
    using PluginMap = TPIMgr_PluginWrapper::PluginMap;
    using NodeIDList = TPIMgr_PluginWrapper::NodeIDList;

public:
    static bool build(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, NodeIDList list, NodeIDList list2);
    static void buildPath(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, const NodeIDList &list);

private:
    static unsigned toInNodeID(PathType type);
    static unsigned toOutNodeID(PathType type);
    static T_Selection getSelection(const PluginMap &pluginMap, T_NodeID id);
    static unsigned getCfgOrder(const PluginMap &pluginMap, T_NodeID id);
    static const char* toName(PathType pathType);
    static void print(const char *name, const PluginMap &pluginMap, const NodeIDList &list);
    static bool checkDup(PathType pathType, const PluginMap &pluginMap, const NodeIDList &list);
    static bool toBufferInfo(TPI_BufferInfo &bufferInfo, const NSPipelinePlugin::BufferSelection &sel, bool force = false);
    static TPI_NodeInfo toNodeInfo(unsigned id, PathType pathType, const T_Selection &sel);
    static bool toNodeInfo(TPI_NodeInfo &node, PathType pathType, const T_Selection &sel);
    static void getSrcBufferInfo(const PluginMap &pluginMap, const NodeIDList &list, TPI_BufferInfo &srcYuv, TPI_BufferInfo &srcDepth);
    static bool needOutput(PathType pathType, TPI_NodeInfo &nodeInfo);
    static void addSrcPath(TPI_Session &session, PathType pathType, unsigned lastID, unsigned id);
    static void addEntryNode(TPI_Session &session, PathType pathType, unsigned inNodeID, unsigned outNodeID, const TPI_BufferInfo &srcYuv, const TPI_BufferInfo &srcDepth, unsigned nodeOpt, unsigned customOpt);
};

unsigned PluginPathBuilder::toInNodeID(PathType type)
{
    switch(type)
    {
    case PathType::YUV:   return TPI_NODE_ID_MTK_S_YUV;
    case PathType::ASYNC: return TPI_NODE_ID_MTK_S_ASYNC;
    case PathType::DISP:  return TPI_NODE_ID_MTK_S_DISP_ONLY;
    case PathType::META:  return TPI_NODE_ID_MTK_S_META_ONLY;
    default:              return TPI_NODE_ID_MTK_S_YUV;
    }
}

unsigned PluginPathBuilder::toOutNodeID(PathType type)
{
    switch(type)
    {
    case PathType::YUV:   return TPI_NODE_ID_MTK_S_YUV_OUT;
    case PathType::ASYNC: return TPI_NODE_ID_MTK_S_ASYNC_OUT;
    case PathType::DISP:  return TPI_NODE_ID_MTK_S_DISP_ONLY_OUT;
    case PathType::META:  return TPI_NODE_ID_MTK_S_META_ONLY_OUT;
    default:              return TPI_NODE_ID_MTK_S_YUV_OUT;
    }
}

PluginPathBuilder::T_Selection PluginPathBuilder::getSelection(const PluginMap &pluginMap, T_NodeID id)
{
    T_Selection sel;
    PluginMap::const_iterator it = pluginMap.find(id);
    if( it != pluginMap.end() )
    {
        sel = it->second.mSelection;
    }
    return sel;
}

unsigned PluginPathBuilder::getCfgOrder(const PluginMap &pluginMap, T_NodeID id)
{
    unsigned order = 0;
    T_Selection sel = getSelection(pluginMap, id);
    if( sel != NULL )
    {
        order = sel->mCfgOrder;
    }
    return order;
}

const char* PluginPathBuilder::toName(PathType pathType)
{
    switch(pathType)
    {
    case PathType::YUV:    return "yuv";
    case PathType::ASYNC:  return "async";
    case PathType::DISP:   return "display";
    case PathType::META:   return "meta";
    default:               return "unknown";
    }
}

void PluginPathBuilder::print(const char *name, const PluginMap &pluginMap, const NodeIDList &list)
{
    MY_LOGD("path %s", name);
    for( T_NodeID id : list )
    {
        unsigned order = getCfgOrder(pluginMap, id);
        MY_LOGD("id=%d order=%d", id, order);
    }
}

bool PluginPathBuilder::checkDup(PathType pathType, const PluginMap &pluginMap, const NodeIDList &list)
{
    bool dup = false;
    unsigned last = 0, count = 0;
    for( T_NodeID id : list )
    {
        unsigned curr = getCfgOrder(pluginMap, id);
        dup = dup || (count++ && curr == last);
        last = curr;
    }

    if( dup )
    {
        const char *name = toName(pathType);
        MY_LOGW("duplicated order in %s list, skipping", name);
        print(name, pluginMap, list);
    }

    return dup;
}

bool PluginPathBuilder::toBufferInfo(TPI_BufferInfo &bufferInfo, const NSPipelinePlugin::BufferSelection &sel, bool force)
{
    bufferInfo.mNeed = force;
    if( sel.getRequired() )
    {
        std::vector<MINT> fmts = sel.getFormats();
        bufferInfo.mFormat = fmts.size() ? (NSCam::EImageFormat)fmts[0] : eImgFmt_UNKNOWN;
        bufferInfo.mSize = sel.getSpecifiedSize();
        bufferInfo.mNeed = true;
    }
    return bufferInfo.mNeed;
}

TPI_NodeInfo PluginPathBuilder::toNodeInfo(unsigned id, PathType pathType, const T_Selection &sel)
{
    TPI_NodeInfo node(id);
    toNodeInfo(node, pathType, sel);
    return node;
}

bool PluginPathBuilder::toNodeInfo(TPI_NodeInfo &node, PathType pathType, const T_Selection &sel)
{
    bool ret = false;
    if( sel != NULL )
    {
        ret = true;
        node.mExpectMS = sel->mCfgExpectMS;
        node.mQueueCount = sel->mCfgQueueCount;
        node.mMarginRatio = sel->mCfgMarginRatio;
        if( pathType == PathType::YUV )
        {
            node.mNodeOption |= sel->mCfgInplace ? TPI_NODE_OPT_INPLACE : 0;
            node.mNodeOption |= (sel->mCfgJoinEntry == eJoinEntry_S_DIV_2) ?
                                TPI_NODE_OPT_DIV : 0;
        }
        else if( pathType == PathType::ASYNC )
        {
            node.mNodeOption |= TPI_NODE_OPT_INPLACE;
            node.mNodeOption |= (sel->mCfgAsyncType == eAsyncType_WAITING) ?
                                  TPI_MTK_OPT_ASYNC_WAITING :
                                (sel->mCfgAsyncType == eAsyncType_POLLING) ?
                                  TPI_MTK_OPT_ASYNC_POLLING : 0;
        }
        else if( pathType == PathType::DISP )
        {
            node.mNodeOption |= TPI_NODE_OPT_INPLACE;
        }
    }
    return ret;
}

void PluginPathBuilder::getSrcBufferInfo(const PluginMap &pluginMap, const NodeIDList &list, TPI_BufferInfo &srcYuv, TPI_BufferInfo &srcDepth)
{
    if( list.size() )
    {
        T_Selection sel = getSelection(pluginMap, list.front());
        if( sel != NULL )
        {
            toBufferInfo(srcYuv, sel->mIBufferMain1);
            toBufferInfo(srcDepth, sel->mIBufferDepth);
        }
    }
}

bool PluginPathBuilder::needOutput(PathType pathType, TPI_NodeInfo &nodeInfo)
{
    return (pathType == PathType::YUV) &&
           !(nodeInfo.mNodeOption & TPI_NODE_OPT_INPLACE);
}

void PluginPathBuilder::addSrcPath(TPI_Session &session, PathType pathType, unsigned lastID, unsigned id)
{
    (void)pathType;
    //if( pathType != PathType::META )
    {
        addPathInfo(session, lastID, TPI_BUFFER_ID_MTK_OUT_YUV,
                                id, TPI_BUFFER_ID_MTK_YUV);
        addPathInfo(session, lastID, TPI_BUFFER_ID_MTK_OUT_YUV_2,
                                id, TPI_BUFFER_ID_MTK_YUV_2);
        addPathInfo(session, lastID, TPI_BUFFER_ID_MTK_OUT_DEPTH,
                                id, TPI_BUFFER_ID_MTK_DEPTH);
    }
}

void PluginPathBuilder::buildPath(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, const NodeIDList &list)
{
    unsigned inNodeID = toInNodeID(pathType);
    unsigned outNodeID = toOutNodeID(pathType);
    unsigned lastNodeID = inNodeID;
    bool needOutBuffer = false;
    unsigned entryNodeOpt = 0, entryCustomOpt = 0;
    TPI_BufferInfo srcYuv(TPI_BUFFER_ID_MTK_OUT_YUV);
    TPI_BufferInfo srcDepth(TPI_BUFFER_ID_MTK_OUT_DEPTH);
    TPI_BufferInfo dstYuv, dstDepth;

    getSrcBufferInfo(pluginMap, list, srcYuv, srcDepth);
    dstYuv = srcYuv;
    dstDepth = srcDepth;

    for( T_NodeID nodeID : list )
    {
        T_Selection sel = getSelection(pluginMap, nodeID);
        if( sel != NULL )
        {
            TPI_NodeInfo node = toNodeInfo(nodeID, pathType, sel);
            needOutBuffer = needOutBuffer || needOutput(pathType, node);
            addBufferInfo(node, dstYuv);
            if( node.mNodeOption & TPI_NODE_OPT_DIV )
            {
                TPI_BufferInfo record(TPI_BUFFER_ID_MTK_OUT_RECORD);
                TPI_BufferInfo display(TPI_BUFFER_ID_MTK_OUT_DISPLAY);

                entryNodeOpt |= TPI_NODE_OPT_DIV;
                if( toBufferInfo(record, sel->mOBufferRecord, true) )
                {
                    addBufferInfo(node, record);
                    entryCustomOpt |= TPI_MTK_OPT_STREAMING_DIV_RECORD;
                }
                if( toBufferInfo(display, sel->mOBufferDisplay) )
                {
                    addBufferInfo(node, display);
                    entryCustomOpt |= TPI_MTK_OPT_STREAMING_DIV_DISPLAY;
                }
            }
            if( sel->mCfgCropInput )
            {
                entryCustomOpt |= TPI_MTK_OPT_STREAMING_NEED_CROP_IN;
            }
            if( sel->mCfgSharedInput )
            {
                if( sel->mCfgInplace ||　sel->mIBufferMain1.getSpecifiedSize().size() )
                {
                    MY_LOGW("SHARED INPUTBUFFER IGNORED mCfgInplace=%d mIBufferMain1 Size=%dx%d",
                        sel->mCfgInplace, sel->mIBufferMain1.getSpecifiedSize().w, sel->mIBufferMain1.getSpecifiedSize().h);
                }
                else
                {
                    entryCustomOpt |= TPI_MTK_OPT_STREAMING_SHARED_INPUT;
                }
            }
            //if( dstDepth.mNeed ) addBufferInfo(node, dstDepth);
            addNodeInfo(session, node);
            addSrcPath(session, pathType, lastNodeID, nodeID);
            lastNodeID = nodeID;
        }
    }

    if( lastNodeID != inNodeID )
    {
        addPathInfo(session, lastNodeID, TPI_BUFFER_ID_MTK_OUT_YUV,
                              outNodeID, TPI_BUFFER_ID_MTK_YUV);
    }

    entryNodeOpt |= (!needOutBuffer) ? TPI_NODE_OPT_INPLACE : 0;
    addEntryNode(session, pathType, inNodeID, outNodeID,
                 srcYuv, srcDepth, entryNodeOpt, entryCustomOpt);
}

void PluginPathBuilder::addEntryNode(TPI_Session &session, PathType pathType, unsigned inNodeID, unsigned outNodeID, const TPI_BufferInfo &srcYuv, const TPI_BufferInfo &srcDepth, unsigned nodeOpt, unsigned customOpt)
{
    (void)pathType;
    TPI_NodeInfo inNode(inNodeID);
    TPI_NodeInfo outNode(outNodeID);
    inNode.mNodeOption = nodeOpt;
    inNode.mCustomOption = customOpt;

    addBufferInfo(inNode, srcYuv);
    addBufferInfo(inNode, TPI_BufferInfo(TPI_BUFFER_ID_MTK_OUT_YUV_2, srcYuv));
    addBufferInfo(inNode, srcDepth);
    inNode.mCustomOption |= TPI_MTK_OPT_STREAMING_YUV;
    inNode.mCustomOption |= isValidFormat(srcYuv) ?
                        TPI_MTK_OPT_STREAMING_CUSTOM_FORMAT : 0;
    inNode.mCustomOption |= isValidSize(srcYuv) ?
                        TPI_MTK_OPT_STREAMING_CUSTOM_SIZE : 0;
    inNode.mCustomOption |= srcDepth.mNeed ?
                        TPI_MTK_OPT_STREAMING_DEPTH : 0;

    addNodeInfo(session, inNode);
    addNodeInfo(session, outNode);
}

bool PluginPathBuilder::build(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, NodeIDList list, NodeIDList list2)
{
    bool ret = false;
    auto sortMethod = [&](unsigned lhs, unsigned rhs)
                      { return getCfgOrder(pluginMap, lhs) <
                               getCfgOrder(pluginMap, rhs); };
    list.sort(sortMethod);
    list2.sort(sortMethod);
    bool dup1 = checkDup(pathType, pluginMap, list);
    bool dup2 = checkDup(pathType, pluginMap, list2);

    if( dup1 ) list.clear();
    if( dup2 ) list2.clear();
    list.splice(list.end(), list2);
    if( list.size() )
    {
        buildPath(session, pluginMap, pathType, list);
        ret = true;
    }
    return ret;
}

bool TPIMgr_PluginWrapper::build(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, const NodeIDList &list, const NodeIDList &list2)
{
    return PluginPathBuilder::build(session, pluginMap, pathType, list, list2);
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
