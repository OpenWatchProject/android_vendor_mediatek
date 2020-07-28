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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_H_

#include "CamGraph_t.h"
#include <cutils/properties.h>
#include <utils/Condition.h>
#include "SyncUtil.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_CAM_GRAPH
#define PIPE_CLASS_TAG "CamGraph"
#include "PipeLog.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

template <typename Node_T>
CamGraph<Node_T>::CamGraph(const char* name)
  : msName(name)
  , mStage(STAGE_IDLE)
  , mRootNode(NULL)
  , mAllowDataFlow(MTRUE)
  , mFlushOnStop(MFALSE)
{
}

template <typename Node_T>
CamGraph<Node_T>::~CamGraph()
{
  android::Mutex::Autolock lock(mMutex);
  if( mStage != STAGE_IDLE || mNodes.size() != 0 )
  {
    MY_LOGE("Error: CamGraph need to be disconnected before destroy");
  }
}

template <typename Node_T>
const char* CamGraph<Node_T>::getName() const
{
  return msName;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::setRootNode(Node_T *rootNode)
{
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_IDLE && rootNode )
  {
    mNodes.insert(rootNode);
    mRootNode = rootNode;
    ret = MTRUE;
  }

  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Node_T &dstNode, ConnectionType type)
{
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_IDLE )
  {
    mNodes.insert(&srcNode);
    mNodes.insert(&dstNode);
    ret = srcNode.connectData(srcID, dstID, &dstNode, type);
    if( ret )
    {
      dstNode.registerInputDataID(dstID);
    }
  }
  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::connectData(DataID_T src, DataID_T dst, Node_T &node, Handler_T *handler, ConnectionType type)
{
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_IDLE )
  {
    mNodes.insert(&node);
    ret = node.connectData(src, dst, handler, type);
  }

  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::disconnect()
{
  TRACE_FUNC_ENTER();

  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_IDLE )
  {
    NODE_SET_ITERATOR it, end;
    for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
    {
      if( !(*it)->disconnect() )
      {
        MY_LOGE("disconnect failed");
      }
    }
    mRootNode = NULL;
    mNodes.clear();
    ret = MTRUE;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::init()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  NODE_SET_ITERATOR it, end;

  if( mStage != STAGE_IDLE )
  {
    MY_LOGE("invalid stage(%d)", mStage);
  }
  else if( !mRootNode )
  {
    MY_LOGE("mRootNode not set");
  }
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    if( !(*it)->init() )
    {
      break;
    }
  }
  if( it != end )
  {
    NODE_SET_ITERATOR begin = mNodes.begin();
    while( it != begin )
    {
      --it;
      (*it)->uninit();
    }
  }
  else
  {
    mStage = STAGE_READY;
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::uninit()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_READY )
  {
    typename NODE_SET::reverse_iterator it, end;
    for( it = mNodes.rbegin(), end = mNodes.rend(); it != end; ++it )
    {
      if( !(*it)->uninit() )
      {
        MY_LOGE("%s uninit failed", (*it)->getName());
      }
    }
    mStage = STAGE_IDLE;
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::start()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);

  if( mStage == STAGE_READY )
  {
    NODE_SET_ITERATOR begin, it, end;
    begin = mNodes.begin();
    end = mNodes.end();
    this->setFlow(mAllowDataFlow);
    for( it = begin; it != end; ++it )
    {
      if( !(*it)->start() )
      {
        break;
      }
    }
    if( it != end )
    {
      while( it != begin )
      {
        --it;
        (*it)->stop();
      }
    }
    else
    {
      mStage = STAGE_RUNNING;
      ret = MTRUE;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::stop()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_RUNNING )
  {
    NODE_SET_ITERATOR it, end;
    if( mFlushOnStop )
    {
      MY_LOGD("flush on stop");
      this->setFlow(MFALSE);
      this->waitFlush();
      this->waitSync();
    }
    else
    {
      MY_LOGD("sync on stop");
      this->waitSync();
      this->setFlow(MFALSE);
    }
    for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
    {
      if( !(*it)->stop() )
      {
        MY_LOGE("%s stop failed", (*it)->getName());
      }
    }
    mStage = STAGE_READY;
    ret = MTRUE;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
template <typename BUFFER_T>
MBOOL CamGraph<Node_T>::enque(DataID_T id, BUFFER_T &buffer)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_RUNNING )
  {
    ret = mRootNode->onData(id, buffer);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
template <typename MSG_T>
MBOOL CamGraph<Node_T>::broadcast(DataID_T id, MSG_T &msg)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_RUNNING )
  {
    NODE_SET_ITERATOR it, end;
    for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
    {
      (*it)->onData(id, msg);
    }
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MVOID CamGraph<Node_T>::setDataFlow(MBOOL allow)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mAllowDataFlow = allow;
  if( mStage == STAGE_RUNNING )
  {
    this->setFlow(mAllowDataFlow);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamGraph<Node_T>::setFlushOnStop(MBOOL flushOnStop)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mFlushOnStop = flushOnStop;
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamGraph<Node_T>::flush(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_RUNNING )
  {
    this->setFlow(MFALSE);
    this->waitFlush(watchdog_ms);
    this->waitSync(watchdog_ms);
    this->setFlow(mAllowDataFlow);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamGraph<Node_T>::sync(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( mStage == STAGE_RUNNING )
  {
    this->waitSync(watchdog_ms);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
CamGraph<Node_T>::Watchdog::Watchdog(CamGraph<Node_T> *parent)
  : mParent(parent)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
CamGraph<Node_T>::Watchdog::~Watchdog()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MBOOL CamGraph<Node_T>::Watchdog::onNotify()
{
  TRACE_FUNC_ENTER();
  if( mParent != NULL )
  {
    android::LogPrinter printer("MtkCam/FeaturePipe", ANDROID_LOG_WARN);
    mParent->onDumpStatus(printer);
  }
  TRACE_FUNC_EXIT();
  return MTRUE;
}

template <typename Node_T>
MVOID CamGraph<Node_T>::setFlow(MBOOL flow)
{
  TRACE_FUNC_ENTER();
  NODE_SET_ITERATOR it, end;
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    (*it)->setDataFlow(flow);
  }
  TRACE_FUNC_EXIT();
}

class CounterCBWrapper : public NotifyCB
{
public:
  CounterCBWrapper(CountDownLatch *counter)
    : mCounter(counter)
  { }

  MBOOL onNotify()
  {
    if( mCounter )
    {
      mCounter->countDown();
    }
    return MTRUE;
  }
private:
  CountDownLatch *mCounter;
};

template <typename Node_T>
MVOID CamGraph<Node_T>::waitFlush(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  NODE_SET_ITERATOR it, end;
  CountDownLatch counter(mNodes.size());
  if( watchdog_ms > 0 )
  {
    android::sp<TimeoutCB> timeoutCB = new TimeoutCB(watchdog_ms);
    timeoutCB->insertCB(new BacktraceNotifyCB());
    timeoutCB->insertCB(new Watchdog(this));
    counter.registerTimeoutCB(timeoutCB);
  }
  android::sp<NotifyCB> flushCB = new CounterCBWrapper(&counter);
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    (*it)->flush(flushCB);
  }
  counter.wait();
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamGraph<Node_T>::waitSync(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  NODE_SET_ITERATOR it, end;
  android::sp<CountDownLatch> counter = new CountDownLatch(mNodes.size());
  if( watchdog_ms > 0 )
  {
    android::sp<TimeoutCB> timeoutCB = new TimeoutCB(watchdog_ms);
    timeoutCB->insertCB(new BacktraceNotifyCB());
    timeoutCB->insertCB(new Watchdog(this));
    counter->registerTimeoutCB(timeoutCB);
  }
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    (*it)->registerSyncCB(counter);
  }
  counter->wait();
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    (*it)->registerSyncCB(NULL);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamGraph<Node_T>::onDumpStatus(android::Printer &printer) const
{
  TRACE_FUNC_ENTER();
  NODE_SET_ITERATOR it, end;
  for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
  {
    (*it)->dumpWaitQueueInfo(printer);
    (*it)->dumpCamThreadInfo(printer);
  }
  TRACE_FUNC_EXIT();
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_H_
