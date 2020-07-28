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

#include "TestSyncNode.h"
#include "TestPipeRule.h"

#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestSyncNode"
#define PIPE_TRACE 1
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

const char* TestSyncDataHandler::ID2Name(DataID id)
{
  switch(id)
  {
  case ID_INVALID:    return "ID_INVALID";
  case ID_ROOT_ENQUE: return "ID_ROOT_ENQUE";
  case ID_A2B:        return "ID_A2B";
  case ID_B_OUT:      return "ID_B_OUT";
  case ID_NEXT:       return "ID_NEXT";
  default:            return "UNKNOWN";
  };
  return "UNKNOWN";
}

TestSyncDataHandler::~TestSyncDataHandler()
{
}

TestSyncNodeUsage::TestSyncNodeUsage()
{
}

TestSyncNodeUsage::TestSyncNodeUsage(long min, long max, MBOOL ext, long extMin, long extMax)
  : mMin(min)
  , mMax(max)
  , mExt(ext)
  , mExtMin(extMin)
  , mExtMax(extMax)
{
}

TestSyncNode::TestSyncNode(const char *name)
  : CamThreadNode(name)
{
  TRACE_FUNC_ENTER();
  this->addWaitQueue(&mRequests);
  TRACE_FUNC_EXIT();
}

TestSyncNode::~TestSyncNode()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL TestSyncNode::onInit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestSyncNode::onUninit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestSyncNode::onThreadStart()
{
  TRACE_FUNC_ENTER();
  mExtThread.setCB(TestSyncNode::onEnqueCB);
  mExtThread.setDelayNS(mUsage.mExtMin, mUsage.mExtMax);
  mExtThread.start();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestSyncNode::onThreadStop()
{
  TRACE_FUNC_ENTER();
  mExtThread.stop();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestSyncNode::onData(DataID data, MUINT32 num)
{
  MBOOL ret = MFALSE;
  TRACE_FUNC_ENTER();

  switch(data)
  {
  case ID_ROOT_ENQUE:
  case ID_A2B:
    MY_LOGD("[%s] frame %d arrived", ID2Name(data), num);
    mRequests.enque(num);
    ret = MTRUE;
    break;
  default:
    ret = MFALSE;
    break;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MVOID TestSyncNode::configUsage(const TestSyncNodeUsage &usage)
{
    TRACE_FUNC_ENTER();
    mUsage = usage;
    TRACE_FUNC_EXIT();
}

MBOOL TestSyncNode::onThreadLoop()
{
  TRACE_FUNC_ENTER();
  //android::sp<TestRequest> request;
  MUINT32 request;

  MY_LOGD("prepare waitAllQueue()");
  if( !waitAllQueue() )
  {
    // mJobs.abort() called
    MY_LOGD("waitAllQueue failed");
    return MFALSE;
  }

  MY_LOGD("prepare mRequests.deque()");
  if( !mRequests.deque(request) )
  {
    MY_LOGD("mRequests.deque() failed");
    return MFALSE;
  }

  MY_LOGD("frame %d in Node %s", request, getName());

  if( mUsage.mMax )
  {
      SimulateDelayNS(mUsage.mMin, mUsage.mMax);
  }

  if( !mUsage.mExt )
  {
    handleData(ID_NEXT, request);
  }
  else
  {
    this->incExtThreadDependency();
    mExtThread.enque(EnqueData(this, request));
  }

  MY_LOGD("finish loop");
  TRACE_FUNC_EXIT();
  return MTRUE;
}

void TestSyncNode::onEnqueCB(const EnqueData &data)
{
  TRACE_FUNC_ENTER();
  data.mNode->handleData(ID_NEXT, data.mData);
  data.mNode->decExtThreadDependency();
  TRACE_FUNC_EXIT();
}

TestSyncNode::EnqueData::EnqueData(TestSyncNode *node, MUINT32 data)
  : mNode(node)
  , mData(data)
{
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
