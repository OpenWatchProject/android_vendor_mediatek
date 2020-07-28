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

#include "../include/SyncUtil.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_SYNC_UTIL
#define PIPE_CLASS_TAG "SyncUtil"
#include "../include/PipeLog.h"

#include <mtkcam/utils/std/CallStackLogger.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

#define NS_PER_MS   1000000ULL

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

NotifyCB::NotifyCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

NotifyCB::~NotifyCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

StatusCB::StatusCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

StatusCB::~StatusCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

WaitNotifyCB::WaitNotifyCB()
  : mDone(MFALSE)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

WaitNotifyCB::~WaitNotifyCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL WaitNotifyCB::onNotify()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  mDone = MTRUE;
  mCondition.broadcast();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL WaitNotifyCB::wait()
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock lock(mMutex);
  while(!mDone)
  {
    mCondition.wait(mMutex);
  }
  TRACE_FUNC_EXIT();
  return MTRUE;
}

BacktraceNotifyCB::BacktraceNotifyCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

BacktraceNotifyCB::~BacktraceNotifyCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL BacktraceNotifyCB::onNotify()
{
  TRACE_FUNC_ENTER();
  MY_LOGW("BackTrace Dump");
  NSCam::Utils::CallStackLogger csl;
  csl.logProcess(LOG_TAG, ANDROID_LOG_WARN);
  TRACE_FUNC_EXIT();
  return MTRUE;
}

TimeoutCB::TimeoutCB(unsigned timeout)
  : mTimeout(timeout)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

TimeoutCB::~TimeoutCB()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

unsigned TimeoutCB::getTimeout()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mTimeout;
}

unsigned long long TimeoutCB::getTimeoutNs()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mTimeout*NS_PER_MS;
}

MBOOL TimeoutCB::insertCB(const android::sp<NotifyCB> &cb)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  Mutex::Autolock lock(mMutex);
  if( cb != NULL )
  {
    this->mCB.push_back(cb);
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL TimeoutCB::onTimeout()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MTRUE;
  Mutex::Autolock lock(mMutex);
  for( unsigned i = 0, n = mCB.size(); i < n; ++i )
  {
    if( mCB[i] != NULL && mCB[i]->onNotify() == MFALSE )
    {
     ret = MFALSE;
     break;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

CountDownLatch::CountDownLatch(unsigned total)
    : mTotal(total)
    , mDone(0)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

CountDownLatch::~CountDownLatch()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL CountDownLatch::registerTimeoutCB(const android::sp<TimeoutCB> &cb)
{
  TRACE_FUNC_ENTER();
  mMutex.lock();
  mTimeoutCB = cb;
  mMutex.unlock();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

void CountDownLatch::countDown()
{
  TRACE_FUNC_ENTER();
  mMutex.lock();
  ++mDone;
  mCondition.broadcast();
  mMutex.unlock();
  TRACE_FUNC_EXIT();
}

void CountDownLatch::countBackUp()
{
  TRACE_FUNC_ENTER();
  mMutex.lock();
  --mDone;
  mCondition.broadcast();
  mMutex.unlock();
  TRACE_FUNC_EXIT();
}

void CountDownLatch::wait()
{
  TRACE_FUNC_ENTER();
  mMutex.lock();
  unsigned long long timeout = (mTimeoutCB != NULL) ? mTimeoutCB->getTimeoutNs() : 0;
  while( mDone < mTotal )
  {
    if( timeout > 0 )
    {
      if( mCondition.waitRelative(mMutex, timeout) != OK )
      {
        MY_LOGW("CountDownLatch timeout(%llu) done(%d) total(%d)", timeout, mDone, mTotal);
        if( mTimeoutCB != NULL )
        {
          mTimeoutCB->onTimeout();
        }
      }
    }
    else
    {
      mCondition.wait(mMutex);
    }
  }
  mMutex.unlock();
  TRACE_FUNC_EXIT();
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam
