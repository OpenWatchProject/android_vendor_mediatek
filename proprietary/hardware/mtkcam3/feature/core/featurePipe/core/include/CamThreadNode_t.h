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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_T_H_

#include <queue>
#include "CamNode.h"
#include "CamThread.h"
#include "SyncUtil.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename Handler_T>
class CamThreadNode : public CamNode<Handler_T>, public CamThread
{
public:
  CamThreadNode(const char *name);
  CamThreadNode(const char *name, int policy, int priority);
  virtual ~CamThreadNode();

public: // CamNode members
  virtual const char* getName() const;
  MVOID flush(const android::sp<NotifyCB> &cb);
  MVOID registerSyncCB(const android::sp<CountDownLatch> &cb);

public: // CamNode members for child class
  virtual MBOOL onInit() = 0;
  virtual MBOOL onUninit() = 0;

public: // CamThread members for child class
  virtual MBOOL onThreadLoop() = 0;
  virtual MBOOL onThreadStart() = 0;
  virtual MBOOL onThreadStop() = 0;
  virtual MVOID onFlush();

private:
  virtual MBOOL onStart();
  virtual MBOOL onStop();

private:
  android::Mutex mMutex;

private:
  class FlushWrapper : virtual public NotifyCB
  {
  public:
    FlushWrapper(CamThreadNode<Handler_T> *parent, const android::sp<NotifyCB> &cb);
    virtual ~FlushWrapper();
    MBOOL onNotify();
  private:
    CamThreadNode<Handler_T> *mParent;
    android::sp<NotifyCB> mCB;
  };

  class SyncCounterCB : virtual public StatusCB, virtual public NotifyCB
  {
  public:
    SyncCounterCB(const char *name, const android::sp<CountDownLatch> &cb);

    // from CamThread::onSyncCB
    MBOOL onUpdate(MINT32 isSync);
    MINT32 getStatus();
    // from WaitHub::onEnque
    MBOOL onNotify();

  private:
    const char *mName;
    android::sp<CountDownLatch> mCB;
    android::Mutex mMutex;
    MBOOL mIsSync;
  };

};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_THREAD_NODE_T_H_
