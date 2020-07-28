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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_SEQ_UTIL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_SEQ_UTIL_H_

#include "SeqUtil_t.h"
#include "VarMap.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_SEQ_UTIL
#define PIPE_CLASS_TAG "SeqUtil"
#include "PipeLog.h"


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

template <typename T, class SeqConverter>
SequentialQueue<T, SeqConverter>::SequentialQueue()
  : mSeq(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename T, class SeqConverter>
SequentialQueue<T, SeqConverter>::SequentialQueue(unsigned seq)
  : mSeq(seq)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename T, class SeqConverter>
SequentialQueue<T, SeqConverter>::~SequentialQueue()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename T, class SeqConverter>
bool SequentialQueue<T, SeqConverter>::empty() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mQueue.empty();
}

template <typename T, class SeqConverter>
size_t SequentialQueue<T, SeqConverter>::size() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mQueue.size();
}

template <typename T, class SeqConverter>
void SequentialQueue<T, SeqConverter>::enque(const T &val)
{
  TRACE_FUNC_ENTER();
  if( mSeq != mSeqConverter(val) )
  {
    MY_LOGD("seq(%d) != enque(%d)", mSeq, mSeqConverter(val));
  }
  TRACE_FUNC_EXIT();
  return mQueue.push(val);
}

template <typename T, class SeqConverter>
bool SequentialQueue<T, SeqConverter>::deque(T &val)
{
  TRACE_FUNC_ENTER();

  bool ret = false;
  if( !mQueue.empty() )
  {
    const T &min = mQueue.top();
    if( mSeqConverter(min) == mSeq )
    {
      val = min;
      mQueue.pop();
      ++mSeq;
      ret = true;
    }
  }

  TRACE_FUNC_EXIT();
  return ret;
}

template <typename T, class SeqConverter>
bool SequentialQueue<T, SeqConverter>::DataGreater::operator()(const T &lhs, const T &rhs) const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mSeqConverter(lhs) > mSeqConverter(rhs);
}

template<typename Handler_T, class Enable>
SequentialHandler<Handler_T, Enable>::SequentialHandler()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T, class Enable>
SequentialHandler<Handler_T, Enable>::SequentialHandler(unsigned)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T, class Enable>
SequentialHandler<Handler_T, Enable>::~SequentialHandler()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T, class Enable>
template<typename DATA_T>
MBOOL SequentialHandler<Handler_T, Enable>::onData(DataID_T id, const DATA_T &data, Handler_T *handler)
{
  TRACE_FUNC_ENTER();
  MY_LOGE("Please define Handler_T::getSeq() before using SequentialHandler");
  TRACE_FUNC_EXIT();
  return handler->onData(id, data);
}

template<typename Handler_T, class Enable>
template<typename DATA_T>
MBOOL SequentialHandler<Handler_T, Enable>::onData(DataID_T id, DATA_T &data, Handler_T *handler)
{
  TRACE_FUNC_ENTER();
  MY_LOGE("Please define Handler_T::getSeq() before using SequentialHandler");
  TRACE_FUNC_EXIT();
  return handler->onData(id, data);
}

template<typename Handler_T, class Enable>
MVOID SequentialHandler<Handler_T, Enable>::clear()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T>
SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::SequentialHandler()
  : mSeq(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T>
SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::SequentialHandler(unsigned seq)
  : mSeq(seq)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T>
SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::~SequentialHandler()
{
  TRACE_FUNC_ENTER();
  clear();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T>
template<typename DATA_T>
MBOOL SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::onData(DataID_T id, const DATA_T &data, Handler_T *handler)
{
  TRACE_FUNC_ENTER();

  SequentialQueue<DATA_T, HandlerSeqConverter>* seqQueue = getSeqQueue<DATA_T, HandlerSeqConverter>(id);
  seqQueue->enque(data);

  DATA_T d;
  while( seqQueue->deque(d) )
  {
    handler->onData(id, d);
  }

  TRACE_FUNC_EXIT();
  return MTRUE;
}

template<typename Handler_T>
template<typename DATA_T>
MBOOL SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::onData(DataID_T id, DATA_T &data, Handler_T *handler)
{
  TRACE_FUNC_ENTER();

  SequentialQueue<DATA_T, HandlerSeqConverter>* seqQueue = getSeqQueue<DATA_T, HandlerSeqConverter>(id);
  seqQueue->enque(data);

  DATA_T d;
  while( seqQueue->deque(d) )
  {
    handler->onData(id, d);
  }

  TRACE_FUNC_EXIT();
  return MTRUE;
}

template<typename Handler_T>
MVOID SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::clear()
{
  TRACE_FUNC_ENTER();
  mQueueMap.clear();
  TRACE_FUNC_EXIT();
}

template<typename Handler_T>
template<typename DATA_T>
unsigned SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::HandlerSeqConverter::operator()(const DATA_T& data) const
{
  return Handler_T::getSeq(data);
}

template<typename Handler_T>
template<typename DATA_T, class SeqConverter>
SequentialQueue<DATA_T, SeqConverter>* SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>::getSeqQueue(DataID_T id)
{
  TRACE_FUNC_ENTER();
  std::string key;
  key = std::string(Handler_T::ID2Name(id)) + std::string(getTypeNameID<DATA_T>());

  typename CONTAINER::const_iterator it = mQueueMap.find(key);

  if( it == mQueueMap.end() )
  {
    mQueueMap[key] = new SequentialQueue<DATA_T, SeqConverter>(mSeq);
  }

  TRACE_FUNC_EXIT();
  return static_cast<SequentialQueue<DATA_T, SeqConverter>*>(mQueueMap[key].get());
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_SYNC_UTIL_H_
