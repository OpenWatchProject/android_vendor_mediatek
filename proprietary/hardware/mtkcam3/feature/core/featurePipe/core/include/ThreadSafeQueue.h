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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_THREAD_SAFE_QUEUE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_THREAD_SAFE_QUEUE_H_

#include "ThreadSafeQueue_t.h"


#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_THREAD_SAFE_QUEUE
#define PIPE_CLASS_TAG "ThreadSafeQueue"
#include "PipeLog.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const char *name)
  : mName(name)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("trackThreadSafeQueue: create queue<%s=%p>", mName.c_str(), this);
  TRACE_FUNC_EXIT();
}

template <typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("trackThreadSafeQueue: destroy queue<%s=%p>", mName.c_str(), this);
  TRACE_FUNC_EXIT();
}

template <typename T>
T ThreadSafeQueue<T>::pop()
{
  TRACE_FUNC_ENTER();

  android::Mutex::Autolock lock(mMutex);

  while( mAvailable.empty() )
  {
    mCondition.wait(mMutex);
  }

  T handle = mAvailable.front();
  mAvailable.pop();

  TRACE_FUNC_EXIT();
  return handle;
}


template <typename T>
void ThreadSafeQueue<T>::push(const T &handle)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);

  mAvailable.push(handle);
  mCondition.broadcast();

  TRACE_FUNC_EXIT();
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    TRACE_FUNC_EXIT();
    return mAvailable.size();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    TRACE_FUNC_EXIT();
    return mAvailable.empty();
}

template <typename T>
const char* ThreadSafeQueue<T>::getName() const
{
    return mName;
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_THREAD_SAFE_QUEUE_H_
