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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_T_H_

#include "CamNode_t.h"
#include <set>
#include <vector>
#include <utils/Mutex.h>
#include "SyncUtil.h"
#include <utils/Printer.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename Node_T>
class CamGraph
{
private:
  typedef typename Node_T::Handler_T Handler_T;
  typedef typename Node_T::Handler_T::DataID DataID_T;

public:
  CamGraph(const char* name);
  ~CamGraph();

public: // cam related
  const char* getName() const;

public: // node related
  MBOOL setRootNode(Node_T *rootNode);
  MBOOL connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Node_T &dstNode, ConnectionType type);
  MBOOL connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Handler_T *handler, ConnectionType type);
  MBOOL disconnect();

  MVOID onDumpStatus(android::Printer &printer) const;

public: // flow control
  MBOOL init();
  MBOOL uninit();
  MBOOL start();
  MBOOL stop();

  template <typename BUFFER_T>
  MBOOL enque(DataID_T id, BUFFER_T &buffer);
  template <typename MSG_T>
  MBOOL broadcast(DataID_T id, MSG_T &msg);

  MVOID setDataFlow(MBOOL allow);
  MVOID setFlushOnStop(MBOOL flushOnStop);
  MVOID flush(unsigned watchdog_ms);
  MVOID sync(unsigned watchdog_ms);

private:
  class Watchdog : public NotifyCB
  {
  public:
    Watchdog(CamGraph<Node_T> *parent);
    virtual ~Watchdog();
    virtual MBOOL onNotify();
  private:
    CamGraph<Node_T> *mParent;
  };
  friend class Watchdog;

  MVOID setFlow(MBOOL flow);
  MVOID waitFlush(unsigned watchdog_ms = 0);
  MVOID waitSync(unsigned watchdog_ms = 0);

private:
  enum Stage { STAGE_IDLE, STAGE_READY, STAGE_RUNNING };
  typedef typename std::set<Node_T*> NODE_SET;
  typedef typename std::set<Node_T*>::iterator NODE_SET_ITERATOR;
  mutable android::Mutex      mMutex;
  const char* const           msName;
  MUINT32                     mStage;
  Node_T                      *mRootNode;
  NODE_SET                    mNodes;
  MBOOL                       mAllowDataFlow;
  MBOOL                       mFlushOnStop;
}; // class CamGraph

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_GRAPH_T_H_
