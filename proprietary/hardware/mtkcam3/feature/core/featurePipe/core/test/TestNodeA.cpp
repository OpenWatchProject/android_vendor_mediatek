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

#include "TestNodeA.h"
#include "TestPipeRule.h"

#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestNodeA"
#define PIPE_TRACE 1
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TestNodeA::TestNodeA(const char *name, Graph_T *graph)
  : TestNode(name, graph)
{
  TRACE_FUNC_ENTER();
  this->addWaitQueue(&mRequests);
  TRACE_FUNC_EXIT();
}

TestNodeA::~TestNodeA()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL TestNodeA::onInit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeA::onUninit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeA::onThreadStart()
{
  TRACE_FUNC_ENTER();
  mExtThread.setCB(TestNodeA::onEnqueCB);
  mExtThread.setDelayNS(getDelayNS());
  mExtThread.start();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeA::onThreadStop()
{
  TRACE_FUNC_ENTER();
  mExtThread.stop();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeA::onData(DataID data, const TestRequestPtr &request)
{
  MBOOL ret;
  TRACE_FUNC_ENTER();

  switch(data)
  {
  case ROOT_ENQUE:
    MY_LOGD("frame %d root_A arrived", request->mID);
    mRequests.enque(request);
    ret = MTRUE;
    break;
  default:
    ret = MFALSE;
    break;
  }

  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL TestNodeA::onThreadLoop()
{
  android::sp<TestRequest> request;

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

  MY_LOGD("frame %d in NodeA", request->mID);

  if( request->mByPass )
  {
    handleData(A2B, request);
    handleData(A2C, request);
  }
  else
  {
    this->incExtThreadDependency();
    mExtThread.enque(EnqueDataA(this, request));
  }

  //this->simulateDelay();
  //this->handleData(A2B, request);
  //this->handleData(A2C, request);

  MY_LOGD("finish loop");
  return MTRUE;
}

void TestNodeA::onEnqueCB(const EnqueDataA &data)
{
  TRACE_FUNC_ENTER();
  data.mNode->handleData(A2B, data.mRequest);
  data.mNode->handleData(A2C, data.mRequest);
  data.mNode->decExtThreadDependency();
  TRACE_FUNC_EXIT();
}

TestNodeA::EnqueDataA::EnqueDataA(TestNodeA *node, TestRequestPtr &request)
  : mNode(node)
  , mRequest(request)
{
}

TestNodeA::EnqueDataA::~EnqueDataA()
{
}

TestNodeA::EnqueDataA::EnqueDataA(const EnqueDataA &src)
{
  mNode = src.mNode;
  mRequest = src.mRequest;
}

TestNodeA::EnqueDataA& TestNodeA::EnqueDataA::operator=(const EnqueDataA &src)
{
  mNode = src.mNode;
  mRequest = src.mRequest;
  return *this;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
