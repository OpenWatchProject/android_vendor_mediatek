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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_SYNC_UTIL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_SYNC_UTIL_H_

#include "MtkHeader.h"
//#include <mtkcam/common.h>

#include <vector>

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class NotifyCB : public virtual android::RefBase
{
public:
  NotifyCB();
  virtual ~NotifyCB();
  virtual MBOOL onNotify() = 0;
};

class StatusCB : public virtual android::RefBase
{
public:
  StatusCB();
  virtual ~StatusCB();
  virtual MBOOL onUpdate(int status) = 0;
  virtual MINT32 getStatus() = 0;
};

class WaitNotifyCB : public NotifyCB
{
public:
  WaitNotifyCB();
  virtual ~WaitNotifyCB();
  virtual MBOOL onNotify();
  MBOOL wait();

private:
  android::Mutex mMutex;
  android::Condition mCondition;
  MBOOL mDone;
};

class BacktraceNotifyCB : public NotifyCB
{
public:
  BacktraceNotifyCB();
  virtual ~BacktraceNotifyCB();
  virtual MBOOL onNotify();
};

class TimeoutCB : public virtual android::RefBase
{
public:
  TimeoutCB(unsigned timeout = 0);
  virtual ~TimeoutCB();
  unsigned getTimeout();
  unsigned long long getTimeoutNs();
  MBOOL insertCB(const android::sp<NotifyCB> &cb);
  MBOOL onTimeout();

private:
  android::Mutex mMutex;
  unsigned mTimeout;
  std::vector< android::sp<NotifyCB> > mCB;
};

class CountDownLatch : public virtual android::RefBase
{
public:
  CountDownLatch(unsigned total);
  virtual ~CountDownLatch();

  MBOOL registerTimeoutCB(const android::sp<TimeoutCB> &cb);
  void countDown();
  void countBackUp();
  void wait();

private:
  android::Condition mCondition;
  android::Mutex mMutex;
  int mTotal;
  int mDone;
  android::sp<TimeoutCB> mTimeoutCB;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_SYNC_UTIL_H_
