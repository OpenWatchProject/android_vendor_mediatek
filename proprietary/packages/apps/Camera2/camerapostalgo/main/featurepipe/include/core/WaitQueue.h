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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_H_

#include "WaitQueue_t.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_WAIT_HUB
#define PIPE_CLASS_TAG "WaitHub"
#include "PipeLog.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

template <typename F, typename T>
MBOOL WaitHub::waitCondition(F func, T *data)
{
  TRACE_N_FUNC_ENTER(mName);
  const MUINT32 PRE_BREAK = SIGNAL_STOP | SIGNAL_CB | SIGNAL_DRY_RUN | SIGNAL_DRY_RUN_ONCE;
  const MUINT32 POST_BREAK = PRE_BREAK | SIGNAL_IDLE_CB | SIGNAL_NEED_SYNC_BREAK;
  MBOOL ret = MFALSE;
  MBOOL condResult;
  android::Mutex::Autolock lock(mMutex);
  while( !(mSignal & PRE_BREAK ) )
  {
    do
    {
      mSignal &= ~SIGNAL_DATA;
      mMutex.unlock();
      condResult = func(data);
      mMutex.lock();
    }while( !condResult && (mSignal & SIGNAL_DATA) );

    if( condResult )
    {
      mSignal |= SIGNAL_DATA;
      mSignal &= ~SIGNAL_IDLE;
      if( mSignal & SIGNAL_SYNC_CB )
      {
        mSignal |= SIGNAL_NEED_SYNC_BREAK;
      }
      ret = MTRUE;
      break;
    }
    mSignal &= ~SIGNAL_DATA;
    mSignal |= SIGNAL_IDLE;
    if( mSignal & POST_BREAK )
    {
      break;
    }
    mCondition.wait(mMutex);
  }
  TRACE_N_FUNC(mName, "signal: 0x%03x", mSignal);
  TRACE_N_FUNC_EXIT(mName);
  return ret;
}

template <typename F, typename T>
MBOOL WaitHub::peakCondition(F func, T *data)
{
  TRACE_N_FUNC_ENTER(mName);
  MBOOL result = func(data);
  TRACE_N_FUNC_EXIT(mName);
  return result;
}

template <typename T>
WaitQueue<T>::WaitQueue()
  : mHub(NULL)
{
}

template <typename T>
WaitQueue<T>::~WaitQueue()
{
}

template <typename T>
bool WaitQueue<T>::empty() const
{
  android::Mutex::Autolock lock(mMutex);
  return mQueue.empty();
}

template <typename T>
void WaitQueue<T>::enque(const T &val)
{
  TRACE_FUNC_ENTER();
  // Release lock before trigger signal to avoid deadlock
  mMutex.lock();
  mQueue.push(val);
  mMutex.unlock();
  if( mHub )
  {
    mHub->signalEnque();
  }
  TRACE_FUNC_EXIT();
}

template <typename T>
bool WaitQueue<T>::deque(T &val)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  bool ret = false;
  if( !mQueue.empty() )
  {
    val = mQueue.front();
    mQueue.pop();
    ret = true;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename T>
MBOOL WaitQueue<T>::isReady() const
{
  android::Mutex::Autolock lock(mMutex);
  return !mQueue.empty();
}

template <typename T>
MVOID WaitQueue<T>::setWaitHub(WaitHub *hub)
{
  android::Mutex::Autolock lock(mMutex);
  mHub = hub;
}

template <typename T>
size_t WaitQueue<T>::size() const
{
  android::Mutex::Autolock lock(mMutex);
  return mQueue.size();
}

template <typename T>
MVOID WaitQueue<T>::clear()
{
  android::Mutex::Autolock lock(mMutex);
  while( !mQueue.empty() )
  {
    mQueue.pop();
  }
}

template <typename T>
IWaitQueue::IDSet WaitQueue<T>::getIDSet() const
{
  android::Mutex::Autolock lock(mMutex);
  IWaitQueue::IDSet idSet;
  if( !mQueue.empty() )
  {
    idSet.insert(0);
  }
  return idSet;
}

template <typename T>
IWaitQueue::IndexSet WaitQueue<T>::getIndexSet() const
{
  android::Mutex::Autolock lock(mMutex);
  IWaitQueue::IndexSet index;
  if( !mQueue.empty() )
  {
    index.insert(IWaitQueue::Index());
  }
  return index;
}

template <typename T, class IndexConverter>
PriorityWaitQueue<T, IndexConverter>::PriorityWaitQueue()
  : mHub(NULL)
  , mIndexSetValid(true)
{
}

template <typename T, class IndexConverter>
PriorityWaitQueue<T, IndexConverter>::~PriorityWaitQueue()
{
}

template <typename T, class IndexConverter>
bool PriorityWaitQueue<T, IndexConverter>::empty() const
{
  android::Mutex::Autolock lock(mMutex);
  return mDataSet.empty();
}

template <typename T, class IndexConverter>
void PriorityWaitQueue<T, IndexConverter>::enque(const T &val)
{
  TRACE_FUNC_ENTER();
  // Release lock before trigger signal to avoid deadlock
  {
    android::Mutex::Autolock lock(mMutex);
    mDataSet.insert(val);
    mIDSet.insert(IndexConverter::getID(val));
    if( mIndexSetValid )
    {
      mIndexSet.insert(IndexConverter()(val));
    }
  }
  if( mHub )
  {
    mHub->triggerSignal(WaitHub::SIGNAL_DATA);
  }
  TRACE_FUNC_EXIT();
}

template <typename T, class IndexConverter>
bool PriorityWaitQueue<T, IndexConverter>::deque(T &val)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  bool ret = false;
  if( !mDataSet.empty() )
  {
    val = (*mDataSet.begin());
    mDataSet.erase(mDataSet.begin());
    mIDSet.erase(IndexConverter::getID(val));
    mIndexSetValid = false;
    ret = true;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename T, class IndexConverter>
bool PriorityWaitQueue<T, IndexConverter>::deque(unsigned id, T &val)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  bool ret = false;
  typename DataSet::iterator dataIt, dataEnd;
  typename IndexSet::iterator indexIt, indexEnd;
  for( dataIt = mDataSet.begin(), dataEnd = mDataSet.end(); dataIt != dataEnd; ++dataIt )
  {
    if( id == IndexConverter::getID(*dataIt) )
    {
      break;
    }
  }
  if( dataIt != mDataSet.end() )
  {
    val = *dataIt;
    mDataSet.erase(dataIt);
    mIDSet.erase(IndexConverter::getID(val));
    mIndexSetValid = false;
    ret = true;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename T, class IndexConverter>
typename PriorityWaitQueue<T, IndexConverter>::CONTAINER_TYPE PriorityWaitQueue<T, IndexConverter>::getContents() const
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  CONTAINER_TYPE contents;
  typename CONTAINER_TYPE::iterator it;
  typename DataSet::iterator dataIt, dataEnd;
  contents.resize(mDataSet.size());
  it = contents.begin();
  for( dataIt = mDataSet.begin(), dataEnd = mDataSet.end(); dataIt != dataEnd; ++dataIt, ++it )
  {
      *it = *dataIt;
  }
  TRACE_FUNC_EXIT();
  return contents;
}

template <typename T, class IndexConverter>
MBOOL PriorityWaitQueue<T, IndexConverter>::isReady() const
{
  android::Mutex::Autolock lock(mMutex);
  return !mDataSet.empty();
}

template <typename T, class IndexConverter>
MVOID PriorityWaitQueue<T, IndexConverter>::setWaitHub(WaitHub *hub)
{
  android::Mutex::Autolock lock(mMutex);
  mHub = hub;
}

template <typename T, class IndexConverter>
size_t PriorityWaitQueue<T, IndexConverter>::size() const
{
  android::Mutex::Autolock lock(mMutex);
  return mDataSet.size();
}

template <typename T, class IndexConverter>
MVOID PriorityWaitQueue<T, IndexConverter>::clear()
{
  android::Mutex::Autolock lock(mMutex);
  mDataSet.clear();
  mIDSet.clear();
}

template <typename T, class IndexConverter>
IWaitQueue::IDSet PriorityWaitQueue<T, IndexConverter>::getIDSet() const
{
  android::Mutex::Autolock lock(mMutex);
  return mIDSet;
}

template <typename T, class IndexConverter>
IWaitQueue::IndexSet PriorityWaitQueue<T, IndexConverter>::getIndexSet() const
{
  android::Mutex::Autolock lock(mMutex);
  if( !mIndexSetValid )
  {
    mIndexSet.clear();
    for( typename DataSet::const_iterator it = mDataSet.begin(), end = mDataSet.end(); it != end; ++it )
    {
      mIndexSet.insert(IndexConverter()(*it));
    }
    mIndexSetValid = true;
  }
  return mIndexSet;
}

template <typename T, class IndexConverter>
bool PriorityWaitQueue<T, IndexConverter>::DataLess::operator()(const T &lhs, const T &rhs) const
{
  Index left = IndexConverter()(lhs);
  Index right = IndexConverter()(rhs);
  return IWaitQueue::Index::Less()(left, right);
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_H_
