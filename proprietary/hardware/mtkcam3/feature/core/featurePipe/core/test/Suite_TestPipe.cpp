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

#include <gtest/gtest.h>
#include "TestPipe.h"
#include "TestPipeRule.h"
#include "TestRequest.h"
#include <iostream>
#include "TestListener.h"

#define PIPE_TRACE 1
#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestPipe"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using android::sp;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

TEST(TestRequest, Initial)
{
  MY_LOGD("Start TestRequest::Initial");
  int i, oldReq, newReq;

  oldReq = TestRequest::genID();
  for( i = 0; i < 10; ++i )
  {
    newReq = TestRequest::genID();
    EXPECT_GT(newReq, oldReq);
    oldReq = newReq;
  }
  MY_LOGD("Stop TestRequest::Initial");
}

TEST(TestPipe, Flow)
{
  MY_LOGD("Start TestPipe::Flow");
  int i;
  NSCam::NSCamFeature::NSFeaturePipe::TestPipe pipe;

  TestRequest::resetID();
  sp<TestRequest> request[TEST_REQUEST_COUNT];
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    request[i] = new TestRequest(i);
    request[i]->mCB = TestListener::CB;
  }

  EXPECT_FALSE(pipe.enque(request[0]));
  ASSERT_TRUE(pipe.init());
  pipe.uninit();

  EXPECT_FALSE(pipe.enque(request[0]));
  ASSERT_TRUE(pipe.init());
  EXPECT_TRUE(pipe.enque(request[0]));
  pipe.uninit();

  EXPECT_FALSE(pipe.enque(request[0]));
  ASSERT_TRUE(pipe.init());
  EXPECT_TRUE(pipe.enque(request[0]));
  pipe.uninit();

  TestListener::resetCounter();
  ASSERT_TRUE(pipe.init());
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    request[i]->mDone = false;
    EXPECT_TRUE(pipe.enque(request[i]));
  }
  EXPECT_TRUE(TestListener::waitRequest(pipe, TEST_REQUEST_COUNT));
  pipe.uninit();

  EXPECT_FALSE(pipe.enque(request[0]));
  MY_LOGD("Stop TestPipe::Flow");
}

TEST(TestPipe, RequestIntegrity)
{
  MY_LOGD("Start TestPipe::RequestIntegrity");
  int i;

  NSCam::NSCamFeature::NSFeaturePipe::TestPipe pipe;
  sp<TestRequest> request[TEST_REQUEST_COUNT];

  TestRequest::resetID();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    request[i] = new TestRequest(i);
    request[i]->mCB = TestListener::CB;
  }

  TestListener::resetCounter();
  pipe.init();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    ASSERT_TRUE(pipe.enque(request[i]));
  }
  EXPECT_TRUE(TestListener::waitRequest(pipe, TEST_REQUEST_COUNT));
  pipe.sync();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    EXPECT_EQ(1, request[i]->getStrongCount());
  }

  pipe.uninit();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    EXPECT_EQ(1, request[i]->getStrongCount());
  }
  MY_LOGD("Stop TestPipe::RequestIntegrity");
}

TEST(TestPipe, Result)
{
  MY_LOGD("Start TestPipe::Result");
  NSCam::NSCamFeature::NSFeaturePipe::TestPipe pipe;
  sp<TestRequest> request[TEST_REQUEST_COUNT];
  int result_b, result_c, result_d;
  int i;

  TestRequest::resetID();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    request[i] = new TestRequest(i);
    request[i]->mCB = TestListener::CB;
  }

  TestListener::resetCounter();
  pipe.init();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    ASSERT_TRUE(pipe.enque(request[i]));
  }
  EXPECT_TRUE(TestListener::waitRequest(pipe, TEST_REQUEST_COUNT));

  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    EXPECT_EQ(request[i]->mData, i);
    result_b = Func1(i);
    result_c = Func2(i);
    result_d = Func3(result_b, result_c);

    EXPECT_EQ(result_b, request[i]->mOutB);
    EXPECT_EQ(result_c, request[i]->mOutC);
    EXPECT_EQ(result_d, request[i]->mOutD);
  }
  pipe.uninit();

#ifdef SHOW_TEST_VERSION
  std::cout << "Finish FeaturePipe gtest: " TEST_VERSION_STR "\n";
#endif
  MY_LOGD("Stop TestPipe::Result");
}

TEST(TestPipe, ByPassRequest)
{
  MY_LOGD("Start TestPipe::ByPassRequest");
  int i;

  NSCam::NSCamFeature::NSFeaturePipe::TestPipe pipe;
  sp<TestRequest> request[TEST_REQUEST_COUNT];

  TestRequest::resetID();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    request[i] = new TestRequest(i);
    request[i]->mCB = TestListener::CB;
    if( i%2 == 0 )
    {
      request[i]->mByPass = true;
    }
  }

  TestListener::resetCounter();
  pipe.init();
  for( i = 0; i < TEST_REQUEST_COUNT; ++i )
  {
    ASSERT_TRUE(pipe.enque(request[i]));
  }
  EXPECT_TRUE(TestListener::waitRequest(pipe, TEST_REQUEST_COUNT));
  pipe.uninit();

  MY_LOGD("Stop TestPipe::ByPassRequest");
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
