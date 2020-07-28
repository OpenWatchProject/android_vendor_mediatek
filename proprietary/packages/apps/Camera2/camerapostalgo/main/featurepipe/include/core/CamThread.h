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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_H_

#include <core/common.h>

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>

#include <sched.h>
#include <queue>
#include <vector>
#include <utils/Printer.h>

#include "WaitQueue.h"

#define DEFAULT_CAMTHREAD_PRIORITY  (ANDROID_PRIORITY_FOREGROUND)

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

class CamThread : private WaitHub
{
public:
  CamThread(const char *name);
  CamThread(const char *name, MUINT32 policy, MUINT32 priority);
  virtual ~CamThread();
  virtual const char *getName() const = 0;

public:
  MBOOL startThread();
  MBOOL stopThread();

  MVOID triggerDryRun();
  MBOOL insertCB(const android::sp<NotifyCB> &cb);
  MBOOL insertIdleCB(const android::sp<NotifyCB> &cb);
  MBOOL registerStatusCB(const android::sp<StatusCB> &cb);
  MBOOL waitIdle();

  MVOID incExtThreadDependency();
  MVOID decExtThreadDependency();
  MINT32 getExtThreadDependency();

  MVOID dumpCamThreadInfo(android::Printer &printer) const;

public:
  virtual MBOOL onThreadLoop() = 0;
  virtual MBOOL onThreadStart() = 0;
  virtual MBOOL onThreadStop() = 0;

public: // WaitHub member
  using WaitHub::addWaitQueue;
  using WaitHub::signalEnque;
  using WaitHub::waitAllQueue;
  using WaitHub::waitAnyQueue;
  using WaitHub::waitAllQueueSync;
  using WaitHub::waitCondition;
  using WaitHub::peakAllQueue;
  using WaitHub::peakAnyQueue;
  using WaitHub::peakCondition;
  using WaitHub::flushQueues;
  using WaitHub::registerEnqueCB;
  using WaitHub::dumpWaitQueueInfo;

private:
  MBOOL tryProcessStop(MUINT32 signal);
  MBOOL tryProcessCB(MUINT32 signal);
  MBOOL tryProcessIdleCB(MUINT32 signal);
  MBOOL tryProcessStatusCB(MUINT32 signal);

private: // android::Thread member
  class CamThreadHandle : public android::Thread
  {
  public:
    CamThreadHandle(CamThread *parent);
    virtual ~CamThreadHandle();
    android::status_t readyToRun();
    bool threadLoop();

  private:
    CamThread *mParent;
    MBOOL mIsFirst;
  };

private:
  mutable android::Mutex mThreadMutex;
  android::sp<CamThreadHandle> mHandle;
  MUINT32 mPolicy;
  MUINT32 mPriority;
  std::deque<android::sp<NotifyCB> > mCB;
  std::deque<android::sp<NotifyCB> > mIdleCB;
  android::sp<StatusCB> mStatusCB;
  MINT32 mExtThreadDependency;
}; // class CamThread

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_H_
