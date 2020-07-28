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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_T_H_

#include <core/common.h>

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <functional>
#include <list>
#include <queue>
#include <set>
#include <vector>
#include <utils/Printer.h>
#include "SyncUtil.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

class WaitHub;

class IWaitQueue
{
public:
  class Index
  {
  public:
    unsigned mID;
    unsigned mPriority;

    Index();
    Index(unsigned id, unsigned priority);
    class Less
    {
    public:
      bool operator()(const Index &lhs, const Index &rhs) const;
    };
  };
  typedef std::multiset<unsigned> IDSet;
  typedef std::multiset<Index, Index::Less> IndexSet;

  virtual ~IWaitQueue() {}
  virtual MBOOL isReady() const = 0;
  virtual MVOID setWaitHub(WaitHub *hub) = 0;
  virtual MVOID clear() = 0;
  virtual size_t size() const = 0;
  virtual IDSet getIDSet() const = 0;
  virtual IndexSet getIndexSet() const = 0;
};

class WaitHub
{
public:
  enum Signal {
    SIGNAL_STOP =             (1<<0),
    SIGNAL_CB =               (1<<1),
    SIGNAL_DATA =             (1<<2),
    SIGNAL_IDLE =             (1<<3),
    SIGNAL_IDLE_CB =          (1<<4),
    SIGNAL_SYNC_CB =          (1<<5),
    SIGNAL_NEED_SYNC_BREAK =  (1<<6),
    SIGNAL_DRY_RUN =          (1<<7),
    SIGNAL_DRY_RUN_ONCE =     (1<<8),
  };
  WaitHub(const char *name);
  virtual ~WaitHub();

  MVOID addWaitQueue(IWaitQueue *queue);
  MVOID flushQueues();

  MVOID triggerSignal(MUINT32 signal);
  MVOID resetSignal(MUINT32 signal);
  MVOID resetSignal();
  MVOID shiftSignal(MUINT32 src, MUINT32 dst);

  MVOID signalEnque();
  MVOID registerEnqueCB(const android::sp<NotifyCB> &cb);

  MBOOL waitAllQueue();
  MBOOL waitAnyQueue();
  MBOOL waitAllQueueSync(MUINT32 &id);
  MBOOL peakAllQueue();
  MBOOL peakAnyQueue();

  template <typename F, typename T>
  MBOOL waitCondition(F func, T *data);
  template <typename F, typename T>
  MBOOL peakCondition(F func, T *data);

  MUINT32 waitSignal();
  MBOOL isAllQueueEmpty();

  MVOID dumpWaitQueueInfo(android::Printer &printer) const;

private:
  MBOOL isAllQueueReady();
  MBOOL isAnyQueueReady();
  MBOOL isAllQueueReadySync(MUINT32 *id);

protected:
  char                      mName[128];

private:
  typedef std::vector<IWaitQueue*> CONTAINER;
  mutable android::Mutex    mMutex;
  android::Condition        mCondition;
  CONTAINER                 mQueues;
  android::sp<NotifyCB>     mEnqueCB;
  MUINT32                   mSignal;
};

template <typename T>
class WaitQueue : public IWaitQueue
{
public:
  WaitQueue();
  virtual ~WaitQueue();

public: // queue member
  bool empty() const;
  void enque(const T &val);
  bool deque(T &val);

public: // IWaitQueue member
  virtual MBOOL isReady() const;
  virtual MVOID setWaitHub(WaitHub *hub);
  virtual size_t size() const;
  virtual MVOID clear();
  virtual IWaitQueue::IDSet getIDSet() const;
  virtual IWaitQueue::IndexSet getIndexSet() const;

private:
  mutable android::Mutex mMutex;
  std::queue<T> mQueue;
  WaitHub *mHub;
}; // class WaitQueue

template <typename T, class IndexConverter>
class PriorityWaitQueue : public IWaitQueue
{
public:
  typedef std::vector<T> CONTAINER_TYPE;

  PriorityWaitQueue();
  virtual ~PriorityWaitQueue();

public: // queue member
  bool empty() const;
  void enque(const T &val);
  bool deque(T &val);
  bool deque(unsigned id, T &val);
  CONTAINER_TYPE getContents() const;

public: // IWaitQueue member
  virtual MBOOL isReady() const;
  virtual MVOID setWaitHub(WaitHub *hub);
  virtual size_t size() const;
  virtual MVOID clear();
  virtual IWaitQueue::IDSet getIDSet() const;
  virtual IndexSet getIndexSet() const;

private:
  class DataLess
  {
  public:
    bool operator()(const T &lhs, const T &rhs) const;
  };

private:
  mutable android::Mutex mMutex;
  WaitHub *mHub;
  typedef std::multiset<T, DataLess> DataSet;
  DataSet mDataSet;
  IDSet mIDSet;
  mutable bool mIndexSetValid;
  mutable IndexSet mIndexSet;
}; // class PriorityWaitQueue

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_WAIT_QUEUE_T_H_
