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

#include "TestNodeC.h"
#include "TestPipeRule.h"

#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestNodeC"
#define PIPE_TRACE 1
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TestNodeC::TestNodeC(const char *name, Graph_T *graph)
  : TestNode(name, graph)
{
  TRACE_FUNC_ENTER();
  this->addWaitQueue(&mRequests);
  TRACE_FUNC_EXIT();
}

TestNodeC::~TestNodeC()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

MBOOL TestNodeC::onInit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeC::onUninit()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeC::onThreadStart()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeC::onThreadStop()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MTRUE;
}

MBOOL TestNodeC::onData(DataID data, const TestRequestPtr &request)
{
  TRACE_FUNC_ENTER();

  MBOOL ret;
  switch(data)
  {
  case A2C:
    MY_LOGD("frame %d A2C arrived", request->mID);
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

MBOOL TestNodeC::onThreadLoop()
{
  android::sp<TestRequest> request;
  int result;

  MY_LOGD("prepare waitAllQueue()");
  if( !waitAllQueue() )
  {
    MY_LOGD("waitAllQueue failed");
    return MFALSE;
  }

  MY_LOGD("prepare mRequests.deque()");
  if( !mRequests.deque(request) )
  {
    MY_LOGD("mRequests.deque() failed");
    return MFALSE;
  }

  this->simulateDelay();
  result = Func2(request->mData);
  request->mOutC = result;

  TestBuffer buffer(request, result);
  handleData(C2D, buffer);

  MY_LOGD("finish loop");
  return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
