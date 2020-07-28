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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_CAMERA_P2_NODE_TEST_H_
#define _MTK_CAMERA_P2_NODE_TEST_H_

// std library
#include "gtest/gtest.h"
#include <utils/RefBase.h>
#include <string>
#include <iostream>

#include "DebugControl.h"
#define PIPE_TRACE TRACE_DISPATCH_PROCESSOR
#define PIPE_CLASS_TAG "Suite_DispatchProcessor"
#include <featurePipe/core/include/PipeLog.h>

#include <featurePipe/core/include/ImageBufferPool.h>

#include "../P2_DispatchProcessor.h"
#include "../P2_MWData.h"

using android::sp;
using android::RefBase;

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

extern int g_argc;
extern char** g_argv;

namespace P2
{
//********************************************
// UT log
//********************************************
#define UTLOG(fmt, arg...) printf("[%s][%s] " fmt "\n", LOG_TAG, __FUNCTION__, ##arg)

#define MY_UTLOG(fmt, arg...) FP_DO(MY_LOGD(fmt, ##arg); UTLOG(fmt, ##arg))

#define MY_UTSCOPEDTRACE() UTScopedTrace ___tracer(__FUNCTION__)

class UTScopedTrace
{
public:
    UTScopedTrace(const char* funcName) : mFuncName(funcName)
    {
        printf("[%s][%s] + \n", LOG_TAG, mFuncName.c_str());
        CAM_LOGD("[%s] +", mFuncName.c_str());
    }

    ~UTScopedTrace()
    {
        printf("[%s][%s] - \n", LOG_TAG, mFuncName.c_str());
        CAM_LOGD("[%s] -", mFuncName.c_str());
    }
    std::string mFuncName;
};

//********************************************
// Config
//********************************************
class Config{
public:
    Config()
    : streamSize(MSize(1920,1080))
    , previewSize(MSize(1920,1080))
    , videoSize(MSize(1920,1080))
    , largeSize(MSize(4000,3000))
    , rrzoSize_1(MSize(2000,1500))
    , rrzoSize_2(MSize(1600,1200))
    , imgoSize_1(MSize(4000,3000))
    , imgoSize_2(MSize(4000,3000))
    {};

    void dump()
    {
        MY_UTLOG("streamSize(%dx%d) previewSize(%dx%d) videoSize(%dx%d)",
            streamSize.w, streamSize.h,
            previewSize.w, previewSize.h,
            videoSize.w, videoSize.h
        );
        MY_UTLOG("main1Idx(%d/%d) main2Idx(%d/%d) isDualCam(%d)",
            main1Idx, main2Idx,
            main1DevIdx, main2DevIdx,
            isDualCam
        );
    };
public:
    const MSize rrzoSize_1;
    const MSize rrzoSize_2;
    const MSize imgoSize_1;
    const MSize imgoSize_2;

    const MSize streamSize;
    const MSize previewSize;
    const MSize videoSize;
    const MSize largeSize;

    const int main1Idx = 0;
    const int main2Idx = 2;
    const int main1DevIdx = std::pow(2,main1Idx);
    const int main2DevIdx = std::pow(2,main2Idx);
    const bool isDualCam = false;
};

//********************************************
// Testing Environment
//********************************************
class P2Environment : public testing::Environment
{
public:
    virtual void SetUp() override;
    virtual void TearDown() override;

private:
    IHalSensor* HalSensorInit();
    void        HalSensorUninit();

 private:
    IHalSensor* mpHalSensor;
};

//********************************************
// P2FrameRequestCreater
//********************************************
enum class MWRequestPath{
    eGeneral,
    ePhysic_1,
    ePhysic_2,
    eLarge_1,
    eLarge_2
};

static const char* pathToChar(const MWRequestPath &path)
{
    switch (path)
    {
        case MWRequestPath::eGeneral:
            return "eGeneral";
        case MWRequestPath::ePhysic_1:
            return "ePhysic_1";
        case MWRequestPath::ePhysic_2:
            return "ePhysic_2";
        case MWRequestPath::eLarge_1:
            return "eLarge_1";
        case MWRequestPath::eLarge_2:
            return "eLarge_2";
        default:
            return "invalid";
    }
}

class P2FrameRequestCreater : public virtual android::RefBase
{
public:
    sp<P2FrameRequest> createRequest(const ILog &log, const sp<P2InfoObj> pP2Info, const set<MWRequestPath>& vRequestPaths);
    ~P2FrameRequestCreater();
private:
    MUINT32 mFrameCount = 0;
};

} // namespace P2

#endif