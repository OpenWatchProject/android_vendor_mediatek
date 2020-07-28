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

#include "TestPipe.h"
#include "TestPipeRule.h"
#include <algorithm>
#include <numeric>

#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestPipe"
#define PIPE_TRACE 1
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TestPipe::TestPipe()
  : CamPipe<TestNode>("TestPipe"),
    mNodeA("TestNodeA", &mCamGraph),
    mNodeB("TestNodeB", &mCamGraph),
    mNodeC("TestNodeC", &mCamGraph),
    mNodeD("TestNodeD", &mCamGraph)
{
  TRACE_FUNC_ENTER();
  mNodeA.setDelayNS(NODE_A_DELAY_NS);
  mNodeB.setDelayNS(NODE_B_DELAY_NS);
  mNodeC.setDelayNS(NODE_C_DELAY_NS);
  mNodeD.setDelayNS(NODE_D_DELAY_NS);
  TRACE_FUNC_EXIT();
}

TestPipe::~TestPipe()
{
  TRACE_FUNC_ENTER();
  this->dispose();
  TRACE_FUNC_EXIT();
}

#define NODE_COUNT 4
long long TestPipe::calculateExpectTimeNS(unsigned loop) const
{
  long long delay[NODE_COUNT], max, oneRun, expect;

  delay[0] = mNodeA.getDelayNS();
  delay[1] = mNodeB.getDelayNS();
  delay[2] = mNodeC.getDelayNS();
  delay[3] = mNodeD.getDelayNS();

  max = *std::max_element(delay, delay+NODE_COUNT);
  oneRun = std::accumulate(delay, delay+NODE_COUNT, 0);

  expect = (loop == 0) ? 0 : (loop * max) + (oneRun - max);

  return expect;
}

MBOOL TestPipe::onInit()
{
  TRACE_FUNC_ENTER();

  this->connectData(A2B, A2B, mNodeA, mNodeB, CONNECTION_SEQUENTIAL);
  this->connectData(A2C, A2C, mNodeA, mNodeC, CONNECTION_SEQUENTIAL);
  this->connectData(B2D, B2D, mNodeB, mNodeD);
  this->connectData(C2D, C2D, mNodeC, mNodeD);
  this->connectData(B_OUT, B_OUT, mNodeB, this);
  this->connectData(D_OUT, D_OUT, mNodeD, this);

  this->setRootNode(&mNodeA);

  TRACE_FUNC_EXIT();
  return MTRUE;
}

MVOID TestPipe::onUninit()
{
  TRACE_FUNC_ENTER();

  this->disconnect();

  TRACE_FUNC_EXIT();
}

MBOOL TestPipe::onData(DataID id, const TestRequestPtr &request)
{
  TRACE_FUNC_ENTER();
  if( id == D_OUT )
  {
    request->mDone = true;
  }

  if( request->mCB )
  {
    request->mCB(request);
  }

  TRACE_FUNC_EXIT();
  return MTRUE;
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam
