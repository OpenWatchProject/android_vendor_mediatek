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

#include "TPIMgr_Util.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "TPI_MGR"
#define PIPE_TRACE TRACE_TPI_MGR
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const char* toScenarioString(TPI_SCENARIO_TYPE scenario)
{
  switch(scenario)
  {
  case TPI_SCENARIO_STREAMING_SINGLE:       return "streaming single";
  case TPI_SCENARIO_STREAMING_DUAL_BASIC:   return "streaming dual basic";
  case TPI_SCENARIO_STREAMING_DUAL_DEPTH:   return "streaming dual depth";
  case TPI_SCENARIO_STREAMING_DUAL_VSDOF:   return "streaming dual vsdof";
  case TPI_SCENARIO_SLOWMOTION:             return "slowmotion";
  default:                                  return "unknown";
  };
}

const char* toNodeName(TPI_NODE_ID id)
{
  switch(id)
  {
  case TPI_NODE_ID_MTK_S_YUV:                 return "yuv_in";
  case TPI_NODE_ID_MTK_S_YUV_OUT:             return "yuv_out";
  case TPI_NODE_ID_MTK_S_ASYNC:               return "async_in";
  case TPI_NODE_ID_MTK_S_ASYNC_OUT:           return "async_out";
  case TPI_NODE_ID_MTK_S_RAW:                 return "raw_in";
  case TPI_NODE_ID_MTK_S_RAW_OUT:             return "raw_out";
  case TPI_NODE_ID_MTK_S_DISP_ONLY:           return "disp_in";
  case TPI_NODE_ID_MTK_S_DISP_ONLY_OUT:       return "disp_out";
  case TPI_NODE_ID_MTK_S_META_ONLY:           return "meta_in";
  case TPI_NODE_ID_MTK_S_META_ONLY_OUT:       return "meta_out";
  default:                                    return "unknown";
  };
}

const char* toBufferName(TPI_BUFFER_ID id)
{
  switch(id)
  {
  case TPI_BUFFER_ID_MTK_YUV:                 return "in_yuv";
  case TPI_BUFFER_ID_MTK_YUV_2:               return "in_yuv_2";
  case TPI_BUFFER_ID_MTK_DEPTH:               return "in_depth";
  case TPI_BUFFER_ID_MTK_DEPTH_INTENSITY:     return "in_depth_v";
  case TPI_BUFFER_ID_MTK_PURE:                return "in_pure";
  case TPI_BUFFER_ID_MTK_PURE_2:              return "in_pure_2";
  case TPI_BUFFER_ID_MTK_OUT_YUV:             return "out_yuv";
  case TPI_BUFFER_ID_MTK_OUT_YUV_2:           return "out_yuv_2";
  case TPI_BUFFER_ID_MTK_OUT_DEPTH:           return "out_depth";
  case TPI_BUFFER_ID_MTK_OUT_DEPTH_INTENSITY: return "out_depth_v";
  case TPI_BUFFER_ID_MTK_OUT_DISPLAY:         return "out_display";
  case TPI_BUFFER_ID_MTK_OUT_RECORD:          return "out_record";
  case TPI_BUFFER_ID_MTK_Q_RECORD_IN:         return "in_q_record";
  default:                                    return "unknown";
  };
}

const char* toStr(TPI_DATA_QUEUE_CMD cmd)
{
  switch(cmd)
  {
  case TPI_DATA_QUEUE_CMD_NONE:       return "q_cmd_none";
  case TPI_DATA_QUEUE_CMD_PUSH:       return "q_cmd_push";
  case TPI_DATA_QUEUE_CMD_PUSH_POP:   return "q_cmd_push_pop";
  case TPI_DATA_QUEUE_CMD_POP_ALL:    return "q_cmd_pop_all";
  default:                            return "q_cmd_unknown";
  }
}

void print(const TPI_BufferInfo &buffer)
{
  MY_LOGD("Buffer ID:0x%x(%s) fmt:0x%08x size(%dx%d) setting(0x%x/0x%x)",
    buffer.mBufferID, toBufferName(buffer.mBufferID),
    buffer.mFormat, buffer.mSize.w, buffer.mSize.h,
    buffer.mBufferSetting, buffer.mCustomSetting);
}

void print(const TPI_NodeInfo &node)
{
  MY_LOGD("Node ID:0x%x(%s) Opt:(0x%x/0x%x) Buf#%zu",
    node.mNodeID, toNodeName(node.mNodeID), node.mNodeOption, node.mCustomOption,
    node.mBufferInfoListCount);
  for( size_t i = 0; i < node.mBufferInfoListCount; ++i )
  {
    print(node.mBufferInfoList[i]);
  }
}

void print(const TPI_PathInfo &path)
{
  const TPI_Port &src = path.mSrc;
  const TPI_Port &dst = path.mDst;
  MY_LOGD("[0x%x(%s):0x%x(%s)] => [0x%x(%s):0x%x(%s)]",
    src.mNode, toNodeName(src.mNode), src.mPort, toBufferName(src.mPort),
    dst.mNode, toNodeName(dst.mNode), dst.mPort, toBufferName(dst.mPort));
}

void print(const TPI_Session &session)
{
  print(&session);
}

void print(const TPI_Session *session)
{
  if( session )
  {
    MY_LOGD("Session:%d(%p) sensor:%d scenario:%d(%s) cookie:%p",
      session->mSessionID, session, session->mLogicalSensorID,
      session->mScenario, toScenarioString(session->mScenario),
      session->mSessionCookie);

    for( size_t i = 0; i < session->mNodeInfoListCount; ++i )
    {
      print(session->mNodeInfoList[i]);
    }

    for( size_t i = 0; i < session->mPathInfoListCount; ++i )
    {
      print(session->mPathInfoList[i]);
    }
  }
}

void addBufferInfo(TPI_NodeInfo &node, TPI_BUFFER_ID id, TPI_BufferInfo buffer)
{
    buffer.mBufferID = id;
    addBufferInfo(node, buffer);
}

void addBufferInfo(TPI_NodeInfo &node, const TPI_BufferInfo &buffer)
{
  if( node.mBufferInfoListCount < TPI_BUFFER_INFO_LIST_SIZE )
  {
    size_t index = node.mBufferInfoListCount;
    node.mBufferInfoList[index] = buffer;
    node.mBufferInfoListCount++;
  }
}

void addNodeInfo(TPI_Session &session, const TPI_NodeInfo &info)
{
  if( session.mNodeInfoListCount < TPI_NODE_INFO_LIST_SIZE )
  {
    size_t index = session.mNodeInfoListCount;
    session.mNodeInfoList[index] = info;
    session.mNodeInfoListCount++;
  }
}

void addPathInfo(TPI_Session &session, TPI_NODE_ID srcID, TPI_BUFFER_ID srcPort, TPI_NODE_ID dstID, TPI_BUFFER_ID dstPort)
{
  if( session.mPathInfoListCount < TPI_PATH_INFO_LIST_SIZE )
  {
    size_t index = session.mPathInfoListCount;
    session.mPathInfoList[index].mSrc.mNode = srcID;
    session.mPathInfoList[index].mSrc.mPort = srcPort;
    session.mPathInfoList[index].mDst.mNode = dstID;
    session.mPathInfoList[index].mDst.mPort = dstPort;
    session.mPathInfoListCount++;
  }
}

void getBufferInfo(const TPI_NodeInfo &node, TPI_BUFFER_ID id, EImageFormat &fmt, MSize &size)
{
  fmt = eImgFmt_UNKNOWN;
  size = MSize(0,0);
  for( unsigned i = 0; i < node.mBufferInfoListCount; ++i )
  {
    if( node.mBufferInfoList[i].mBufferID == id )
    {
      fmt = node.mBufferInfoList[i].mFormat;
      size = node.mBufferInfoList[i].mSize;
      break;
    }
  }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
