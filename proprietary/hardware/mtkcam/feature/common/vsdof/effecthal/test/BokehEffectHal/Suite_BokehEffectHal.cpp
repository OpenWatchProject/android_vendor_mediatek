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
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)
//
#define LOG_TAG "BokehEffectHalUTTest"
//
#include <gtest/gtest.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
#include <utils/RefBase.h>
#include "BokehEffectHalUT.h"
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
//
#include <random>
using namespace std;
using namespace VSDOF::Bokeh::UT;
using namespace StereoHAL;
//************************************************************************
// utility function
//************************************************************************
#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {
  }
}
/*TEST(BokehEffectHalUT, Init)
{
    BokehEffectHalUTParam param;
    param.scenario = FRAME_SCENARIO_PREVIEW;
    BokehEffectHalUT ut;
    EXPECT_EQ(MTRUE, ut.init(param));
    EXPECT_EQ(MTRUE, ut.config());
    EXPECT_EQ(MTRUE, ut.uninit());
}*/
//
TEST(BokehEffectHalUT, GF_UT)
{
    random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 3);
    //
    GeneratorParam param;
    param.scenario = dis(gen);
    param.runPath = GENERAL;
    param.stopMode = VSDOF_STOP_MODE::FLUSH;
    param.is3DNREnable = MTRUE;
    param.gmv_x = 0;
    param.gmv_y = 0;
    param.bokehLevel = 16;
    param.isAFTrigger = MTRUE;
    param.ptAF_x = 165;
    param.ptAF_y = 43;
    sp<BokehEffectHalUT> ut = new BokehEffectHalUT();
    EXPECT_EQ(MTRUE, ut->init(param));
    EXPECT_EQ(MTRUE, ut->config());
    EXPECT_EQ(OK, ut->run());
    Wait(200);
    EXPECT_EQ(MTRUE, ut->uninit());
    ut=nullptr;
}