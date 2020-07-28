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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_T_H_

#include <utils/Mutex.h>
#include "CamGraph.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

#define WATCHDOG_TIMEOUT (3000)//ms

template <typename Node_T>
class CamPipe
{
public:
  typedef typename Node_T::DataID_T DataID_T;
  typedef typename Node_T::Handler_T Handler_T;

public:
  CamPipe(const char *name);
  virtual ~CamPipe();
  MBOOL init();
  MBOOL uninit();
  MBOOL setRootNode(Node_T *root);
  MBOOL connectData(DataID_T id, Node_T &srcNode, Node_T &dstNode, ConnectionType type=CONNECTION_DIRECT);
  MBOOL connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Node_T &dstNode, ConnectionType type=CONNECTION_DIRECT);
  MBOOL connectData(DataID_T id, Node_T &srcNode, Handler_T *handler, ConnectionType type=CONNECTION_DIRECT);
  MBOOL connectData(DataID_T srcID, DataID_T dstID, Node_T &srcNode, Handler_T *handler, ConnectionType type=CONNECTION_DIRECT);
  MBOOL disconnect();
  template <typename BUFFER_T>
  MBOOL enque(DataID_T id, BUFFER_T &buffer);
  MVOID flush(unsigned watchdog_ms = WATCHDOG_TIMEOUT);

  // sync will block until all thread in graph are idle
  // use with caution !!
  MVOID sync(unsigned watchdog_ms = WATCHDOG_TIMEOUT);
  MVOID setFlushOnStop(MBOOL flushOnStop);

  MVOID onDumpStatus(android::Printer &printer) const;

protected:
  MVOID dispose();

protected:
  virtual MBOOL onInit() = 0;
  virtual MVOID onUninit() = 0;

private:
  enum Stage { STAGE_IDLE, STAGE_PREPARE, STAGE_READY, STAGE_DISPOSE };
  Stage mStage;
  android::Mutex mStageLock;

protected:
  // TODO: move to private when CamNode does not need direct access to CamGraph
  CamGraph<Node_T> mCamGraph;
};

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_PIPE_T_H_
