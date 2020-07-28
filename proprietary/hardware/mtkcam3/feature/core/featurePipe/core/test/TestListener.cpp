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

#include "TestListener.h"
#include "TestPipeRule.h"
#include <algorithm>
#include <iostream>
#include <time.h>

#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestListener"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using android::sp;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

android::Mutex TestListener::sMutex;
unsigned TestListener::sCBCount = 0;
unsigned TestListener::sDoneCount = 0;
std::vector<unsigned> TestListener::sDoneIDs;

void TestListener::CB(const sp<TestRequest> &request)
{
  android::Mutex::Autolock lock(sMutex);

  ++TestListener::sCBCount;
  MY_LOGD("++CB=%d", TestListener::sCBCount);
  if( request != NULL && request->mDone )
  {
    ++TestListener::sDoneCount;
    MY_LOGD("++Done=%d", TestListener::sDoneCount);

    sDoneIDs.push_back(request->mID);
  }
}

void TestListener::resetCounter()
{
  android::Mutex::Autolock lock(sMutex);

  TestListener::sCBCount = 0;
  TestListener::sDoneCount = 0;
  TestListener::sDoneIDs.clear();
  MY_LOGD("resetCounter()");
}

void printNS(long long ns)
{
  if( ns > FPIPE_NS_PER_SEC )
  {
    std::cerr << ns/FPIPE_NS_PER_SEC << "s "
              << ns%FPIPE_NS_PER_SEC/1000000.0 << "ms\n";
  }
  else
  {
    std::cerr << ns/1000000 << "ms "
              << ns%1000000 << "ns\n";
  }
}

bool TestListener::waitRequest(const TestPipe &pipe, unsigned loop)
{
  timeval t1, t2;
  struct timespec t;
  long waitSec;
  long waitNSec;
  long long expectNS;
  long long usedNS;
  int retry;
  bool result;

  waitSec = TEST_WAIT_PERIOD_NS / FPIPE_NS_PER_SEC;
  waitNSec = TEST_WAIT_PERIOD_NS % FPIPE_NS_PER_SEC;
  expectNS = pipe.calculateExpectTimeNS(loop);

  //std::cerr << "Expect process time for " << loop << " enques: ";
  //printNS(expectNS);

  MY_LOGD("expectNS=%llu", expectNS);
  MY_LOGD("waitSec=%ld", waitSec);
  MY_LOGD("waitNSec=%ld", waitNSec);

  result = false;
  // give 10% extra time
  retry = (expectNS*1.1/ TEST_WAIT_PERIOD_NS) + 1;
  gettimeofday(&t1, NULL);
  do
  {
    {
      android::Mutex::Autolock lock(sMutex);
      if( TestListener::sDoneCount >= loop )
      {
        if( std::is_sorted(TestListener::sDoneIDs.begin(), TestListener::sDoneIDs.end()) )
        {
          result = true;
        }
        break;
      }
    }

    t.tv_sec = waitSec;
    t.tv_nsec = waitNSec;
    if( nanosleep(&t, NULL) != 0 )
    {
      break;
    }
  }while( retry-- );

  gettimeofday(&t2, NULL);
  //std::cerr << "Used process time for " << loop << " enques: ";
  usedNS = (t2.tv_sec-t1.tv_sec) * (long long)1000000000;
  usedNS += (t2.tv_usec-t1.tv_usec) * (long long)1000;
  //printNS(usedNS);

  return result;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
