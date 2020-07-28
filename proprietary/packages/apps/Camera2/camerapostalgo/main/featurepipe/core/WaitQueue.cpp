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

#include "core/WaitQueue.h"
#include <algorithm>
#include <functional>

#include "core/DebugControl.h"
#define PIPE_TRACE TRACE_WAIT_HUB
#define PIPE_CLASS_TAG "WaitHub"
#include "core/PipeLog.h"

typedef android::Mutex::Autolock Autolock;

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

static bool isQueueReady(IWaitQueue* queue)
{
  return queue->isReady();
}

#if __cplusplus <= 199711L
template<class InputIterator, class UnaryPredicate>
bool any_of (InputIterator first, InputIterator last, UnaryPredicate pred)
{
  while (first!=last) {
    if (pred(*first)) return true;
    ++first;
  }
  return false;
}
#endif

IWaitQueue::Index::Index()
  : mID(0)
  , mPriority(0)
{
}

IWaitQueue::Index::Index(unsigned id, unsigned priority)
  : mID(id)
  , mPriority(priority)
{
}

bool IWaitQueue::Index::Less::operator()(const Index &lhs, const Index &rhs) const
{
  return (lhs.mPriority < rhs.mPriority) ||
         (lhs.mPriority == rhs.mPriority && lhs.mID < rhs.mID);
}

WaitHub::WaitHub(const char* name)
  : mSignal(0)
{
  snprintf(mName, sizeof(mName), "%s", (name != NULL) ? name : "NA");
}

WaitHub::~WaitHub()
{
}

MVOID WaitHub::addWaitQueue(IWaitQueue *queue)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  if( queue )
  {
    queue->setWaitHub(this);
    mQueues.push_back(queue);
    if( queue->isReady() )
    {
      mSignal |= SIGNAL_DATA;
      mCondition.broadcast();
    }
  }
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::flushQueues()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  std::vector<IWaitQueue*>::iterator it, begin, end;
  for( it = mQueues.begin(), end = mQueues.end(); it != end; ++it )
  {
    (*it)->clear();
  }
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::signalEnque()
{
  TRACE_N_FUNC_ENTER(mName);
  mMutex.lock();
  if( mEnqueCB != NULL )
  {
    android::sp<NotifyCB> cb = mEnqueCB;
    mMutex.unlock();
    cb->onNotify();
    mMutex.lock();
  }
  mSignal |= WaitHub::SIGNAL_DATA;
  mSignal &= ~(WaitHub::SIGNAL_IDLE);
  if( mSignal & WaitHub::SIGNAL_SYNC_CB )
  {
    mSignal |= WaitHub::SIGNAL_NEED_SYNC_BREAK;
  }
  mCondition.broadcast();
  mMutex.unlock();
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::registerEnqueCB(const android::sp<NotifyCB> &cb)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  mEnqueCB = cb;
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::triggerSignal(MUINT32 signal)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  TRACE_N_FUNC(mName, "signal: 0x%03x + 0x%03x => 0x%03x", mSignal, signal, mSignal|signal);
  mSignal |= signal;
  mCondition.broadcast();
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::resetSignal(MUINT32 signal)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  TRACE_N_FUNC(mName, "signal: 0x%03x - 0x%03x => 0x%03x", mSignal, signal, mSignal&(~signal));
  mSignal &= ~signal;
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::resetSignal()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  mSignal = 0;
  TRACE_N_FUNC_EXIT(mName);
}

MVOID WaitHub::shiftSignal(MUINT32 src, MUINT32 dst)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  if( mSignal & src )
  {
    mSignal &= ~src;
    mSignal |= dst;
    //mCondition.broadcast();
  }
  else
  {
    mSignal &= ~dst;
  }
  TRACE_N_FUNC_EXIT(mName);
}

MBOOL WaitHub::waitAllQueue()
{
  return waitCondition(std::mem_fun(&WaitHub::isAllQueueReady), this);
}

MBOOL WaitHub::waitAnyQueue()
{
  return waitCondition(std::mem_fun(&WaitHub::isAnyQueueReady), this);
}

MBOOL WaitHub::waitAllQueueSync(MUINT32 &id)
{
  return waitCondition(bind1st(std::mem_fun(&WaitHub::isAllQueueReadySync), this), &id);
}

MBOOL WaitHub::peakAllQueue()
{
  return peakCondition(std::mem_fun(&WaitHub::isAllQueueReady), this);
}

MBOOL WaitHub::peakAnyQueue()
{
  return peakCondition(std::mem_fun(&WaitHub::isAnyQueueReady), this);
}

MUINT32 WaitHub::waitSignal()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  const MUINT32 TRIGGER = SIGNAL_STOP | SIGNAL_CB | SIGNAL_DATA | SIGNAL_IDLE_CB | SIGNAL_SYNC_CB | SIGNAL_NEED_SYNC_BREAK | SIGNAL_DRY_RUN | SIGNAL_DRY_RUN_ONCE;

  if( !(mSignal & WaitHub::SIGNAL_DATA) &&
      any_of(mQueues.begin(), mQueues.end(), isQueueReady) )
  {
    mSignal |= WaitHub::SIGNAL_DATA;
  }

  while( !(mSignal & TRIGGER) )
  {
    mCondition.wait(mMutex);
    if( any_of(mQueues.begin(), mQueues.end(), isQueueReady) )
    {
      mSignal |= WaitHub::SIGNAL_DATA;
    }
  }
  TRACE_N_FUNC(mName, "signal: 0x%03x", mSignal);
  TRACE_N_FUNC_EXIT(mName);
  return mSignal;
}

MBOOL WaitHub::isAllQueueEmpty()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  MBOOL ret = !any_of(mQueues.begin(), mQueues.end(), isQueueReady);
  TRACE_N_FUNC_EXIT(mName);
  return ret;
}

MVOID WaitHub::dumpWaitQueueInfo(android::Printer &printer) const
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  unsigned n = mQueues.size();
  unsigned size = 0;
  for( unsigned i = 0; i < n; ++i )
  {
    size = mQueues[i]->size();
    printer.printFormatLine("[FPipeStatus] %s queue(%u/%u) size(%u)", mName, i, n, size);
  }
  printer.printFormatLine("[FPipeStatus] %s mSignal(0x%08X)", mName, mSignal);
  TRACE_N_FUNC_EXIT(mName);
  return;
}

MBOOL WaitHub::isAllQueueReady()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  MBOOL ret = MFALSE;
  unsigned ready;
  ready = std::count_if(mQueues.begin(), mQueues.end(), isQueueReady);
  ret = ready && (ready == mQueues.size());
  TRACE_N_FUNC_EXIT(mName);
  return ret;
}

MBOOL WaitHub::isAnyQueueReady()
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  MBOOL ret = any_of(mQueues.begin(), mQueues.end(), isQueueReady);
  TRACE_N_FUNC_EXIT(mName);
  return ret;
}

MBOOL WaitHub::isAllQueueReadySync(MUINT32 *id)
{
  TRACE_N_FUNC_ENTER(mName);
  Autolock lock(mMutex);
  MBOOL found = MFALSE;
  IWaitQueue::IndexSet indexSet;
  std::vector<IWaitQueue::IDSet> idSets;
  unsigned size = mQueues.size();

  if( !id )
  {
    MY_LOGE("Invalid id result holder");
  }
  else if( size > 0 )
  {
    idSets.resize(size);
    for( unsigned i = 0; i < size; ++i )
    {
      idSets[i] = mQueues[i]->getIDSet();
    }

    indexSet = mQueues[0]->getIndexSet();
    IWaitQueue::IndexSet::iterator it, end;
    for( it = indexSet.begin(), end = indexSet.end(); it != end; ++it )
    {
      found  = MTRUE;
      for( unsigned i = 1; i < size; ++i )
      {
        if( idSets[i].count(it->mID) <= 0 )
        {
          found = MFALSE;
          break;
        }
      }
      if( found )
      {
        *id = it->mID;
        break;
      }
    }
  }
  TRACE_N_FUNC_EXIT(mName);
  return found;
}
} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

