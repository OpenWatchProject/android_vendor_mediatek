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

#ifndef _TEST_EXT_THREAD_H_
#define _TEST_EXT_THREAD_H_

#include "TestTool.h"
#include "TestExtThread_t.h"
#include <stdlib.h>

#include <featurePipe/core/include/PipeLogHeaderBegin.h>
#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestExtThread"
#define PIPE_TRACE 1
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename Data_T>
TestExtThread<Data_T>::TestExtThread()
  : mHandle(NULL)
  , mCB(NULL)
  , mDelayNS(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
TestExtThread<Data_T>::~TestExtThread()
{
  TRACE_FUNC_ENTER();
  this->stop();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::start()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( mHandle == NULL )
  {
    mHandle = new Handle(this);
    mHandle->setCB(mCB);
    mHandle->setDelayNS(mDelayNS);
    mHandle->run("test");
  }
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::stop()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( mHandle != NULL )
  {
    mHandle->stop();
    mHandle->join();
    mHandle = NULL;
  }
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::enque(const Data_T &data)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  if( mHandle != NULL )
  {
    mHandle->enque(data);
  }
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::setCB(CB_T cb)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mCB = cb;
  if( mHandle != NULL )
  {
    mHandle->setCB(mCB);
  }
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::setDelayNS(long ns)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mDelayNS = ns;
  if( mHandle != NULL )
  {
    mHandle->setDelayNS(mDelayNS);
  }
  TRACE_FUNC_EXIT();
}

#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "TestExtThreadHandle"

template <typename Data_T>
TestExtThread<Data_T>::Handle::Handle(TestExtThread *parent)
  : mParent(parent)
  , mStop(false)
  , mCB(NULL)
  , mDelayNS(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
TestExtThread<Data_T>::Handle::~Handle()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
android::status_t TestExtThread<Data_T>::Handle::readyToRun()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return android::NO_ERROR;
}

template <typename Data_T>
bool TestExtThread<Data_T>::Handle::threadLoop()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  while( !mStop && mQueue.empty() )
  {
    mCond.wait(mMutex);
  }
  if( !mStop && !mQueue.empty() )
  {
    Data_T data = mQueue.front();
    mQueue.pop_front();
    CB_T cb = mCB;
    long delay = mDelayNS;
    mMutex.unlock();
    if( delay > 0 )
    {
      delay = rand() % delay;
      SimulateDelayNS(delay);
    }
    if( cb )
    {
      cb(data);
    }
    mMutex.lock();
  }
  if( mStop && !mQueue.empty() )
  {
    MY_LOGE("queue not empty before stop");
    mQueue.clear();
  }

  TRACE_FUNC_EXIT();
  return !mStop;
}

template <typename Data_T>
void TestExtThread<Data_T>::Handle::stop()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mStop = true;
  mCond.broadcast();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::Handle::enque(const Data_T &data)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mQueue.push_back(data);
  mCond.broadcast();
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::Handle::setCB(CB_T cb)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mCB = cb;
  TRACE_FUNC_EXIT();
}

template <typename Data_T>
void TestExtThread<Data_T>::Handle::setDelayNS(long delay)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mDelayNS = delay;
  TRACE_FUNC_EXIT();
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#include <featurePipe/core/include/PipeLogHeaderEnd.h>
#endif // _TEST_EXT_THREAD_H_
