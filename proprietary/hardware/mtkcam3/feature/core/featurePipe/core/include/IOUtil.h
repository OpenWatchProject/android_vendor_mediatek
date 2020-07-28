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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_H_

#include "IOUtil_t.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_IO_UTIL
#define PIPE_CLASS_TAG "IOUtil"
#include "PipeLog.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template<typename T>
static inline const char* getName(const T *node)
{
  return node ? node->getName() : "NULL";
}

static inline OutputType streamToType(const StreamType &stream)
{
  switch(stream)
  {
    case STREAMTYPE_PREVIEW:          return OUTPUT_STREAM_PREVIEW;
    case STREAMTYPE_PREVIEW_CALLBACK: return OUTPUT_STREAM_PREVIEW_CALLBACK;
    case STREAMTYPE_RECORD:           return OUTPUT_STREAM_RECORD;
    case STREAMTYPE_FD:               return OUTPUT_STREAM_FD;
    case STREAMTYPE_PHYSICAL:         return OUTPUT_STREAM_PHYSICAL;
    case STREAMTYPE_ASYNC:            return OUTPUT_STREAM_ASYNC;
    default:                          return OUTPUT_INVALID;
  }
}

static inline const char* toName(const StreamType &stream)
{
  switch(stream)
  {
    case STREAMTYPE_PREVIEW:          return "preview";
    case STREAMTYPE_PREVIEW_CALLBACK: return "preview_callback";
    case STREAMTYPE_RECORD:           return "record";
    case STREAMTYPE_FD:               return "fd";
    case STREAMTYPE_PHYSICAL:         return "phy";
    case STREAMTYPE_ASYNC:            return "async";
    default:                          return "unknown";
  }
}

static inline const char* toName(const OutputType &type)
{
  switch(type)
  {
    case OUTPUT_INVALID:                  return "invalid";
    case OUTPUT_STREAM_PREVIEW:           return "preview";
    case OUTPUT_STREAM_PREVIEW_CALLBACK:  return "preview_callback";
    case OUTPUT_STREAM_RECORD:            return "record";
    case OUTPUT_STREAM_FD:                return "fd";
    case OUTPUT_STREAM_PHYSICAL:          return "phy_out";
    case OUTPUT_STREAM_ASYNC:             return "async";
    case OUTPUT_FULL:                     return "full";
    case OUTPUT_NEXT_FULL:                return "next_full";
    default:                              return "unknown";
  }
}

static inline const char* toName(const IOPolicyType &policy)
{
  switch(policy)
  {
    case IOPOLICY_BYPASS:           return "bypass";
    case IOPOLICY_INOUT:            return "inout";
    case IOPOLICY_LOOPBACK:         return "loopback";
    case IOPOLICY_INOUT_EXCLUSIVE:  return "inout_e";
    case IOPOLICY_INOUT_NEXT:       return "inout_n";
    case IOPOLICY_INOUT_QUEUE:      return "inout_q";
    case IOPOLICY_INPLACE:          return "inplace";
    default:                        return "unknown";
  }
}

template <typename Node_T, typename ReqInfo_T>
IOControl<Node_T, ReqInfo_T>::NextInfo::NextInfo(const Node_T *node)
  : mNode(node)
{
}

template <typename Node_T, typename ReqInfo_T>
void IOControl<Node_T, ReqInfo_T>::NextInfo::addType(const OutputType &type)
{
  mOutput.insert(type);
}

template <typename Node_T, typename ReqInfo_T>
void IOControl<Node_T, ReqInfo_T>::NextInfo::setNext(Node_T *next)
{
  mNext = next;
}

template <typename Node_T, typename ReqInfo_T>
bool IOControl<Node_T, ReqInfo_T>::NextInfo::needType(const OutputType &type) const
{
  return mOutput.count(type);
}

template <typename Node_T, typename ReqInfo_T>
Node_T* IOControl<Node_T, ReqInfo_T>::NextInfo::getNext() const
{
  return mOutput.count(OUTPUT_NEXT_FULL) ? mNext : NULL;
}

template <typename Node_T, typename ReqInfo_T>
void IOControl<Node_T, ReqInfo_T>::NextInfo::print() const
{
  TRACE_FUNC_ENTER();
  const char *name = getName(mNode);
  CAM_ULOGD(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "node(%s): #output=%zu, next full=(%s)",
          name, mOutput.size(), getName(mNext));
  for( const auto &out : mOutput )
  {
      CAM_ULOGD(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "node(%s): output[%s]", name, toName(out));
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
IOControl<Node_T, ReqInfo_T>::BufferInfo::BufferInfo(const android::sp<IIBuffer> &buffer, const NextFullInfo &info)
  : mBuffer(buffer)
  , mNextFullInfo(info)
{
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::BufferInfo::get(android::sp<IIBuffer> &buffer, NextFullInfo &info) const
{
  buffer = mBuffer;
  info = mNextFullInfo;
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::BufferInfo::pop(android::sp<IIBuffer> &buffer, NextFullInfo &info)
{
  buffer = mBuffer;
  info = mNextFullInfo;
  mBuffer = NULL;
  mNextFullInfo.mResize = MSize(0,0);
}

template <typename Node_T, typename ReqInfo_T>
IOControl<Node_T, ReqInfo_T>::IOControl()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
IOControl<Node_T, ReqInfo_T>::~IOControl()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::setRoot(Node_T *root)
{
    mRoot = root;
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::addStream(const StreamType &stream, const NodeList &nodes)
{
  TRACE_FUNC_ENTER();
  if( mStreamPathMap.count(stream) )
  {
    CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "StreamMap[%s] redefined", toName(stream));
  }
  mStreamPathMap[stream] = nodes;
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IOControl<Node_T, ReqInfo_T>::prepareMap(const StreamSet &streams, const ReqInfo_T &reqInfo, IORequest &req)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("{%s} stream(%zu)", reqInfo.dump(), streams.size());
  MBOOL ret = MTRUE;
  for( const StreamType &stream : streams )
  {
    if( mStreamPathMap.count(stream) )
    {
      ret = ret && prepareStreamMap(stream, reqInfo, req);
    }
    else
    {
      CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "{%s} Cannot find stream(%s)", reqInfo.dump(), toName(stream));
    }
  }
  allocNextBuf(reqInfo, req.mNextTable, req.mBufferTable);
  allocAsyncBuf(reqInfo, req.mAsyncNext, req.mAsyncBuffer);

  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::dump(const IORequest &req) const
{
    TRACE_FUNC_ENTER();
    dumpInfo(req.mNextTable);
    dumpInfo(req.mBufferTable);
    TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::dumpInfo(const NextTable &nextTable) const
{
    TRACE_FUNC_ENTER();
    for( const auto &stream : mStreamPathMap )
    {
        for( const auto &node : stream.second)
        {
            auto out = nextTable.find(node);
            if( out != nextTable.end() )
            {
                out->second.print();
            }
        }
    }
    TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::dumpInfo(const BufferTable &bufferTable) const
{
    TRACE_FUNC_ENTER();
    for( const auto &stream : mStreamPathMap )
    {
        for( const auto node : stream.second)
        {
            auto buf = bufferTable.find(node);
            if( buf != bufferTable.end() )
            {
                CAM_ULOGD(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "node(%s) has buffer %p", node->getName(), buf->second.mBuffer.get());
            }
        }
    }
    TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IOControl<Node_T, ReqInfo_T>::prepareStreamMap(const StreamType &stream, const ReqInfo_T &reqInfo, IORequest &req)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  PolicyInfoList policys = queryPathPolicy(stream, reqInfo);
  ret = (stream == STREAMTYPE_ASYNC) ?
        prepareAsyncNext(policys, req.mAsyncNext) :
        prepareGeneralMap(stream, reqInfo, policys, req.mNextTable);
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IOControl<Node_T, ReqInfo_T>::prepareGeneralMap(const StreamType &stream, const ReqInfo_T &reqInfo, const PolicyInfoList &policys, NextTable &nextTable)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MTRUE;

  if( forwardCheck(policys) )
  {
    backwardCalc(stream, reqInfo, policys, nextTable);
  }
  else
  {
    CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "{%s} forward check error, stream(%s) policys(%zu)",
            reqInfo.dump(), toName(stream), policys.size());
    ret = MFALSE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IOControl<Node_T, ReqInfo_T>::prepareAsyncNext(const PolicyInfoList &policys, Node_T* &asyncNext)
{
  TRACE_FUNC_ENTER();
  for( const PolicyInfo &info : policys )
  {
    if( info.mNode != NULL && info.mPolicy == IOPOLICY_INOUT_EXCLUSIVE )
    {
      asyncNext = info.mNode;
      break;
    }
  }
  TRACE_FUNC_EXIT();
  return MTRUE;
}

template <typename Node_T, typename ReqInfo_T>
typename IOControl<Node_T, ReqInfo_T>::PolicyInfoList IOControl<Node_T, ReqInfo_T>::queryPathPolicy(const StreamType &stream, const ReqInfo_T &reqInfo) const
{
  TRACE_FUNC_ENTER();
  PolicyInfoList list;
  auto path = mStreamPathMap.find(stream);
  if( path != mStreamPathMap.end() )
  {
    for( Node_T *node : path->second )
    {
      if( !node )
      {
        CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "stream=%s Invalid node=NULL", toName(stream));
        continue;
      }
      list.emplace_back(PolicyInfo(node, node->getIOPolicy(stream, reqInfo)));
    }
  }
  TRACE_FUNC_EXIT();
  return list;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IOControl<Node_T, ReqInfo_T>::forwardCheck(const PolicyInfoList &policyInfos) const
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MTRUE;
  int count = 0;
  IOPolicyType prev = IOPOLICY_BYPASS;
  for( const PolicyInfo &info : policyInfos )
  {
    IOPolicyType curr = info.mPolicy;
    if( curr != IOPOLICY_BYPASS )
    {
      TRACE_FUNC("Lookup %s to %s", toName(prev), toName(curr));
      if( curr == IOPOLICY_LOOPBACK && count != 0 )
      {
          CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "%s to %s is invalid", toName(prev), toName(curr));
          ret = MFALSE;
          break;
      }
      prev = curr;
      ++count;
    }
  }
  ret = ret && count;
  TRACE_FUNC_EXIT();
  return ret;
}

static inline bool useNext(const OutputType &type, const IOPolicyType &policy)
{
  return (type == OUTPUT_NEXT_FULL) &&
         ((policy == IOPOLICY_INPLACE ||
           policy == IOPOLICY_INOUT_NEXT));
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::backwardCalc(const StreamType &stream, const ReqInfo_T &reqInfo, const PolicyInfoList &policyInfos, NextTable &nextTable) const
{
  TRACE_FUNC_ENTER();
  int count = 0;
  Node_T *nextNode = NULL;
  OutputType nextType = streamToType(stream);

  TRACE_FUNC("{%s} list_size(%zu) type(%s)",
             reqInfo.dump(), policyInfos.size(), toName(nextType));
  for( auto it = policyInfos.crbegin(), end = policyInfos.crend(); it != end; ++it )
  {
    Node_T *node = it->mNode;
    IOPolicyType policy = it->mPolicy;
    TRACE_FUNC("{%s} node(%s)[%s][%s] next(%s)[%s]",
               reqInfo.dump(), getName(node), toName(policy), getName(nextNode), toName(nextType));
    if( node != NULL && policy != IOPOLICY_BYPASS )
    {

      nextTable[node].addType(nextType);
      if( policy == IOPOLICY_LOOPBACK )
      {
        nextTable[node].addType(OUTPUT_FULL);
      }
      if( nextType == OUTPUT_NEXT_FULL )
      {
        nextTable[node].setNext(nextNode);
      }
      nextNode = useNext(nextType, policy) ? nextNode : node;
      switch( policy )
      {
      case IOPOLICY_INOUT_QUEUE:      nextType = OUTPUT_NEXT_FULL;  break;
      case IOPOLICY_INOUT_EXCLUSIVE:  nextType = OUTPUT_NEXT_FULL;  break;
      case IOPOLICY_INOUT_NEXT:       nextType = OUTPUT_NEXT_FULL;  break;
      case IOPOLICY_INPLACE:          break;
      default:                        nextType = OUTPUT_FULL;       break;
      // case IOPOLICY_INOUT:            nextType = OUTPUT_FULL;       break;
      // case IOPOLICY_LOOPBACK:         nextType = OUTPUT_FULL;       break;
      // case IOPOLICY_BYPASS:           nextType = OUTPUT_FULL;       break;
      }

      ++count;
    }
  }

  if( !count )
  {
    CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "{%s} Can not found active policy in path!", reqInfo.dump());
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::allocNextBuf(const ReqInfo_T &reqInfo, NextTable &nextTable, BufferTable &bufferTable) const
{
  TRACE_FUNC_ENTER();
  for( const auto &next : nextTable )
  {
    Node_T *node = next.second.getNext();
    if( node != NULL )
    {
      android::sp<IBufferPool> pool;
      NextFullInfo info;
      if( node->getInputBufferPool(reqInfo, pool, info) && pool != NULL )
      {
        TRACE_FUNC("{%s} name(%s) requst buffer type(%s) from name(%s) pool=(%d/%d)", reqInfo.dump(), (nextEntry.first)->getName(), toName(outType), getName(node), pool->peakAvailableSize(), pool->peakPoolSize());
        bufferTable[next.first] = BufferInfo(pool->requestIIBuffer(), info);
      }
      else
      {
        CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_COMMON, "{%s} name(%s) requst next full from name(%s) failed",
                reqInfo.dump(), getName(next.first), getName(node));
      }
    }
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MVOID IOControl<Node_T, ReqInfo_T>::allocAsyncBuf(const ReqInfo_T &reqInfo, Node_T *asyncNext, BufferInfo &asyncBuf) const
{
  TRACE_FUNC_ENTER();
  if( asyncNext != NULL )
  {
    android::sp<IBufferPool> pool;
    NextFullInfo info;
    if( asyncNext->getInputBufferPool(reqInfo, pool, info) &&
        pool != NULL )
    {
      TRACE_FUNC("{%s} request async buffer from (%s) pool=(%d/%d)",
                 reqInfo.dump(), asyncNext->getName(),
                 pool->peakAvailableSize(), pool->peakPoolSize());
      asyncBuf = BufferInfo(pool->requestIIBuffer(), info);
    }
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::isFullBufHold() const
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  for( const auto &buffer : mBufferTable )
  {
    if( buffer.second.mBuffer != NULL )
    {
      ret = MTRUE;
      break;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needPreview(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_STREAM_PREVIEW);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needPreviewCallback(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_STREAM_PREVIEW_CALLBACK);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needRecord(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_STREAM_RECORD);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needFD(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_STREAM_FD);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needPhysicalOut(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_STREAM_PHYSICAL);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needFull(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_FULL);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needNextFull(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return needOutputType(node, OUTPUT_NEXT_FULL);
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needAsync(const Node_T *node) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mAsyncNext != NULL && needPreview(node);
}

template <typename Node_T, typename ReqInfo_T>
android::sp<IIBuffer> IORequest<Node_T, ReqInfo_T>::getNextFullImg(const Node_T *node, NextFullInfo &info)
{
  TRACE_FUNC_ENTER();
  android::sp<IIBuffer> img;
  mBufferTable[node].pop(img, info);
  TRACE_FUNC_EXIT();
  return img;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::getNextFullImgInfo(const Node_T *node, NextFullInfo &info) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  MBOOL get = MFALSE;
  auto it = mBufferTable.find(node);
  if(it != mBufferTable.end())
  {
    info = it->second.mNextFullInfo;
    get = MTRUE;
  }
  return get;
}


template <typename Node_T, typename ReqInfo_T>
android::sp<IIBuffer> IORequest<Node_T, ReqInfo_T>::getAsyncImg(const Node_T *node, NextFullInfo &info)
{
  (void)node;
  TRACE_FUNC_ENTER();
  android::sp<IIBuffer> img;
  mAsyncBuffer.get(img, info);
  TRACE_FUNC_EXIT();
  return img;
}

template <typename Node_T, typename ReqInfo_T>
MBOOL IORequest<Node_T, ReqInfo_T>::needOutputType(const Node_T *node, const OutputType &type) const
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  const auto &entry = mNextTable.find(node);
  ret = entry != mNextTable.end() &&
        entry->second.needType(type);
  TRACE_FUNC_EXIT();
  return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#include "PipeLogHeaderEnd.h"

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_H_
