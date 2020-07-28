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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_H_

#include "CamPipe_t.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_CAM_PIPE
#define PIPE_CLASS_TAG "CamPipe"
#include "PipeLog.h"


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

template <typename Node_T>
CamPipe<Node_T>::CamPipe(const char *name)
  : mStage(STAGE_IDLE)
  , mCamGraph(name)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
CamPipe<Node_T>::~CamPipe()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mStageLock);
  if( mStage != STAGE_DISPOSE )
  {
    MY_LOGE("Error: CamPipe::dispose() not called before destroy");
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::init()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  MBOOL pipeInit, graphInit, graphStart;
  android::Mutex::Autolock lock(mStageLock);

  pipeInit = graphInit = graphStart = MTRUE;
  if( mStage == STAGE_IDLE )
  {
    mStage = STAGE_PREPARE;
    mStageLock.unlock();
    pipeInit = this->onInit();
    mStageLock.lock();

    if( pipeInit )
    {
      if( (graphInit = mCamGraph.init()) )
      {
        graphStart = mCamGraph.start();
      }
    }

    if( pipeInit && graphInit && graphStart )
    {
      mStage = STAGE_READY;
      ret = MTRUE;
    }
    else
    {
      if( graphInit )
      {
        mCamGraph.uninit();
      }
      mStage = STAGE_IDLE;
      if( pipeInit )
      {
        mStageLock.unlock();
        this->onUninit();
        mStageLock.lock();
      }
    }
  }

  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::uninit()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_READY )
  {
    mCamGraph.stop();
    mCamGraph.uninit();
    mStage = STAGE_IDLE;
    mStageLock.unlock();
    this->onUninit();
    mStageLock.lock();
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::setRootNode(Node_T *root)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_IDLE || mStage == STAGE_PREPARE )
  {
    ret = mCamGraph.setRootNode(root);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::connectData(DataID_T id, Node_T &srcNode, Node_T &dstNode, ConnectionType type)
{
  TRACE_FUNC_ENTER();
  MBOOL ret;
  ret = this->connectData(id, id, srcNode, dstNode, type);
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Node_T &dstNode, ConnectionType type)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_IDLE || mStage == STAGE_PREPARE )
  {
    ret = mCamGraph.connectData(srcID, dstID, srcNode, dstNode, type);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::connectData(DataID_T id, Node_T &srcNode, Handler_T *handler, ConnectionType type)
{
  TRACE_FUNC_ENTER();
  MBOOL ret;
  ret = this->connectData(id, id, srcNode, handler, type);
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Handler_T *handler, ConnectionType type)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_IDLE || mStage == STAGE_PREPARE )
  {
    ret = mCamGraph.connectData(srcID, dstID, srcNode, handler, type);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MBOOL CamPipe<Node_T>::disconnect()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_IDLE || mStage == STAGE_PREPARE )
  {
    ret = mCamGraph.disconnect();
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
template <typename BUFFER_T>
MBOOL CamPipe<Node_T>::enque(DataID_T id, BUFFER_T &buffer)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mStageLock);
  MBOOL ret = MFALSE;
  if( mStage == STAGE_READY )
  {
    ret = mCamGraph.enque(id, buffer);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Node_T>
MVOID CamPipe<Node_T>::flush(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_READY )
  {
    mCamGraph.setDataFlow(MFALSE);
    mCamGraph.flush(watchdog_ms);
    mCamGraph.setDataFlow(MTRUE);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamPipe<Node_T>::sync(unsigned watchdog_ms)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_READY )
  {
    mCamGraph.sync(watchdog_ms);
  }
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamPipe<Node_T>::setFlushOnStop(MBOOL flushOnStop)
{
  TRACE_FUNC_ENTER();
  mCamGraph.setFlushOnStop(flushOnStop);
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamPipe<Node_T>::dispose()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mStageLock);
  if( mStage == STAGE_READY )
  {
    mCamGraph.stop();
    mCamGraph.uninit();
    this->onUninit();
  }
  mCamGraph.disconnect();
  mStage = STAGE_DISPOSE;
  TRACE_FUNC_EXIT();
}

template <typename Node_T>
MVOID CamPipe<Node_T>::onDumpStatus(android::Printer &printer) const
{
  TRACE_FUNC_ENTER();
  mCamGraph.onDumpStatus(printer);
  TRACE_FUNC_EXIT();
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_H_
