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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_H_

#include "../include/CamThreadNode_t.h"

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_CAM_THREAD_NODE
#define PIPE_CLASS_TAG "CamThreadNode"
#include "PipeLog.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename Handler_T>
CamThreadNode<Handler_T>::CamThreadNode(const char* name)
  : CamNode<Handler_T>(name), CamThread(name)
{
}

template <typename Handler_T>
CamThreadNode<Handler_T>::CamThreadNode(const char* name, int policy, int priority)
  : CamNode<Handler_T>(name), CamThread(name, policy, priority)
{
}

template <typename Handler_T>
CamThreadNode<Handler_T>::~CamThreadNode()
{
}

template <typename Handler_T>
const char* CamThreadNode<Handler_T>::getName() const
{
  return CamNode<Handler_T>::getName();
}

template <typename Handler_T>
CamThreadNode<Handler_T>::FlushWrapper::FlushWrapper(CamThreadNode<Handler_T> *parent, const android::sp<NotifyCB> &cb)
  : mParent(parent)
  , mCB(cb)
{
}

template <typename Handler_T>
CamThreadNode<Handler_T>::FlushWrapper::~FlushWrapper()
{
}

template <typename Handler_T>
MBOOL CamThreadNode<Handler_T>::FlushWrapper::onNotify()
{
  if( mParent )
  {
    mParent->onFlush();
    if( mCB != NULL )
    {
      mCB->onNotify();
    }
  }
  return MTRUE;
}

template <typename Handler_T>
MVOID CamThreadNode<Handler_T>::flush(const android::sp<NotifyCB> &cb)
{
  TRACE_FUNC_ENTER();
  android::sp<NotifyCB>wrapper = new FlushWrapper(this, cb);
  this->insertCB(wrapper);
  TRACE_FUNC_EXIT();
}

template <typename T>
CamThreadNode<T>::SyncCounterCB::SyncCounterCB(const char *name, const android::sp<CountDownLatch> &cb)
    : mName(name)
    , mCB(cb)
    , mIsSync(0)
{
}

template <typename T>
MBOOL CamThreadNode<T>::SyncCounterCB::onUpdate(MINT32 isSync)
{
  TRACE_N_FUNC_ENTER(mName);
  android::Mutex::Autolock lock(mMutex);
  TRACE_N_FUNC(mName, "sync %d => %d", mIsSync, isSync);
  if( isSync && !mIsSync )
  {
    mCB->countDown();
  }
  else if( !isSync && mIsSync )
  {
    mCB->countBackUp();
  }
  mIsSync = isSync;
  TRACE_N_FUNC_EXIT(mName);
  return MTRUE;
}

template <typename T>
MINT32 CamThreadNode<T>::SyncCounterCB::getStatus()
{
  TRACE_N_FUNC_ENTER(mName);
  //android::Mutex::Autolock lock(mMutex);
  TRACE_N_FUNC_EXIT(mName);
  return mIsSync;
}

template <typename T>
MBOOL CamThreadNode<T>::SyncCounterCB::onNotify()
{
  TRACE_N_FUNC_ENTER(mName);
  android::Mutex::Autolock lock(mMutex);
  if( mIsSync )
  {
    // Data enqued, bring node out of sync
    TRACE_N_FUNC(mName, "sync %d => 0");
    mIsSync = 0;
    mCB->countBackUp();
  }
  TRACE_N_FUNC_EXIT(mName);
  return MTRUE;
}

template <typename Handler_T>
MVOID CamThreadNode<Handler_T>::registerSyncCB(const android::sp<CountDownLatch> &cb)
{
  TRACE_FUNC_ENTER();
  android::sp<SyncCounterCB> wrapper;
  if( cb != NULL )
  {
    wrapper = new SyncCounterCB(getName(), cb);
  }
  CamThread::registerStatusCB(wrapper);
  CamThread::registerEnqueCB(wrapper);
  TRACE_FUNC_EXIT();
}

template <typename Handler_T>
MBOOL CamThreadNode<Handler_T>::onStart()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock lock(mMutex);
  ret = this->startThread();
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamThreadNode<Handler_T>::onStop()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  this->stopThread();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

template <typename Handler_T>
MVOID CamThreadNode<Handler_T>::onFlush()
{
  TRACE_FUNC_ENTER();
  this->flushQueues();
  TRACE_FUNC_EXIT();
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_H_
