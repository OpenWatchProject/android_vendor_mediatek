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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_T_H_

#include "MtkHeader.h"
#include "SeqUtil.h"
//#include <mtkcam/common.h>

#include <utils/Mutex.h>

#include <map>
#include <set>
#include "CamNodeULog.h"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


enum ConnectionType { CONNECTION_DIRECT, CONNECTION_SEQUENTIAL };

template <typename Handler_T>
class CamNode : virtual public CamNodeULogHandler
{
public:
  typedef typename Handler_T::DataID        DataID_T;

public:
  CamNode(const char *name);
  virtual ~CamNode();

public:
  virtual const char* getName() const;
  MINT32 getPropValue();
  MINT32 getPropValue(DataID_T id);

public:
  MBOOL connectData(DataID_T src, DataID_T dst, Handler_T *handler, ConnectionType type);
  MBOOL registerInputDataID(DataID_T id);
  MBOOL disconnect();

public: // control flow related
  MVOID setDataFlow(MBOOL allow);

protected: // internal buffer flow related
  template <typename BUFFER_T>
  MBOOL handleData(DataID_T id, const BUFFER_T &buffer);
  template <typename BUFFER_T>
  MBOOL handleData(DataID_T id, BUFFER_T &buffer);

public: // child class related
  virtual MBOOL init();
  virtual MBOOL uninit();
  virtual MBOOL start();
  virtual MBOOL stop();

public:
  virtual MBOOL onInit() = 0;
  virtual MBOOL onUninit() = 0;
  virtual MBOOL onStart() = 0;
  virtual MBOOL onStop() = 0;

protected:
  MBOOL isRunning();

private:
  MVOID updatePropValues();

private:
  enum Stage { STAGE_IDLE, STAGE_READY, STAGE_RUNNING };
  mutable android::Mutex      mNodeLock;
  char                        mName[128] = {0};
  Stage                       mStage;
  MBOOL                       mAllowDataFlow;
  MINT32                      mPropValue;
  std::map<DataID_T, MINT32>  mDataPropValues;

  class HandlerEntry{
  public:
    DataID_T        mSrcID;
    DataID_T        mDstID;
    Handler_T      *mHandler;
    ConnectionType  mType;
    HandlerEntry()
      : mSrcID((DataID_T)0)
      , mDstID((DataID_T)0)
      , mHandler(NULL)
      , mType(CONNECTION_DIRECT)
    { }
    HandlerEntry(DataID_T src, DataID_T dst, Handler_T *handler, ConnectionType type)
      : mSrcID(src), mDstID(dst), mHandler(handler), mType(type)
    { }
  };
  typedef std::map<DataID_T, HandlerEntry>  HANDLER_MAP;
  typedef typename HANDLER_MAP::iterator    HANDLER_MAP_ITERATOR;
  HANDLER_MAP                               mHandlerMap;
  typedef std::set<DataID_T>                SOURCE_SET;
  SOURCE_SET                                mSourceSet;
  SequentialHandler<Handler_T>              mSeqHandler;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_T_H_
