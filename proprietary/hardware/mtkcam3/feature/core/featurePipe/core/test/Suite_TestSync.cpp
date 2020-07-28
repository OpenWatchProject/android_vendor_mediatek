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
#include "TestSyncPipe.h"
#include <iostream>

#define PIPE_TRACE 1
#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestSync"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

using NSCam::NSCamFeature::NSFeaturePipe::TestSyncNodeUsage;

#define TEST_COUNT 1000

void testSync(const TestSyncNodeUsage &u1, const TestSyncNodeUsage &u2)
{
  NSCam::NSCamFeature::NSFeaturePipe::TestSyncPipe pipe(u1, u2);

  pipe.init();
  for(unsigned i = 0; i < TEST_COUNT; ++i )
  {
      pipe.flush();
      for( unsigned j = 0; j < 3; ++j )
      {
          pipe.enque(j);
      }
      pipe.flush();
  }
  pipe.uninit();
}

TEST(TestSync, Basic)
{
  MY_LOGD("Start TestSyncPipe::Sync");

  long d0 = 0 * FPIPE_NS_PER_MS;
  long d1 = 1 * FPIPE_NS_PER_MS;
  long d2 = 2 * FPIPE_NS_PER_MS;
  long d3 = 3 * FPIPE_NS_PER_MS;
  long d4 = 4 * FPIPE_NS_PER_MS;
  long d5 = 5 * FPIPE_NS_PER_MS;

  TestSyncNodeUsage u20(d0, d2, MFALSE, d0, d0);
  TestSyncNodeUsage x25(d0, d2, MTRUE, d0, d5);

  {
      testSync(u20, u20);
      testSync(x25, u20);
  }

  MY_LOGD("Stop TestSyncPipe::Sync");
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
