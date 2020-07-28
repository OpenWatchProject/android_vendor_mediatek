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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_SEQ_UTIL_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_SEQ_UTIL_T_H_

#include "MtkHeader.h"
//#include <mtkcam/common.h>

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include <functional>
#include <queue>
#include <map>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class SequentialQueueBase : public virtual android::RefBase
{
public:
  SequentialQueueBase();
  virtual ~SequentialQueueBase();
};

template <typename T, class SeqConverter>
class SequentialQueue : public SequentialQueueBase
{
public:
  SequentialQueue();
  SequentialQueue(unsigned seq);
  virtual ~SequentialQueue();

public: // queue member
  bool empty() const;
  size_t size() const;
  void enque(const T &val);
  bool deque(T &val);
private:
  class DataGreater
  {
  public:
    bool operator()(const T &lhs, const T &rhs) const;
  private:
    SeqConverter mSeqConverter;
  };

private:
  std::priority_queue<T, std::vector<T>, DataGreater> mQueue;
  SeqConverter mSeqConverter;
  unsigned mSeq;
};

typedef android::sp<SequentialQueueBase> SeqQueuePtr;

template<typename Handler_T, class Enable = void> //default
class SequentialHandler
{
public:
  typedef typename Handler_T::DataID DataID_T;
public:
  SequentialHandler();
  SequentialHandler(unsigned seq);
  virtual ~SequentialHandler();

  template<typename DATA_T>
  MBOOL onData(DataID_T id, const DATA_T &data, Handler_T *handler);

  template<typename DATA_T>
  MBOOL onData(DataID_T id, DATA_T &data, Handler_T *handler);

  MVOID clear();
};

template<typename Handler_T>
class SequentialHandler<Handler_T, typename std::enable_if< Handler_T::supportSeq >::type>
{
public:
  typedef typename Handler_T::DataID DataID_T;
public:
  SequentialHandler();
  SequentialHandler(unsigned seq);
  virtual ~SequentialHandler();

  //not thread safe
  template<typename DATA_T>
  MBOOL onData(DataID_T id, const DATA_T &data, Handler_T *handler);

  //not thread safe
  template<typename DATA_T>
  MBOOL onData(DataID_T id, DATA_T &data, Handler_T *handler);

  MVOID clear();

private:
  class HandlerSeqConverter
  {
  public:
    template<typename DATA_T>
    unsigned operator()(const DATA_T& data) const;
  };

  template<typename DATA_T, class SeqConverter>
  SequentialQueue<DATA_T, SeqConverter>* getSeqQueue(DataID_T id);

  typedef std::map<std::string, SeqQueuePtr> CONTAINER;
  CONTAINER mQueueMap;
  const unsigned  mSeq;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_SEQ_UTIL_T_H_
