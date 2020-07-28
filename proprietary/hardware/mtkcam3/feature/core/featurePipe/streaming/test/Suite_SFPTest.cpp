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

#include "DebugControl.h"
#define PIPE_TRACE TRACE_MDPWrapper
#define PIPE_CLASS_TAG "Suite_MDPWrapper_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/log/ILogger.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <mtkcam/utils/std/DebugTimer.h>

#include "TestSFP_Common.h"

#include <cstdlib>
#include <vector>
#include <unordered_map>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;

using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;

using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;

using namespace NSCam::Utils;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

static MBOOL fpipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param)
{
    sp<SFPCBHandlerBase> cb = param.getVar<sp<SFPCBHandlerBase>>("testCB", NULL);
    EXPECT_NE(cb.get(), (SFPCBHandlerBase*)NULL);
    if(cb != NULL)
        cb->onEvent(msg, param);

    return MTRUE;
}

struct SFPTestParam
{
    MUINT32 sensorNum = 1;
    std::vector<MUINT32> genOut;
    std::map<MUINT32, std::vector<MUINT32>> physicalOuts;
    std::map<MUINT32, MBOOL> physicalDifferentTuning;
    std::map<MUINT32, std::vector<MUINT32>> largeOuts;

    MUINT32 requestTestCount = 1;
    MUINT32 reqIntervalMs = 30;
    MUINT32 timeoutSecond = 10;
    MBOOL dumpOut = MFALSE;
};

MVOID runTest(IStreamingFeaturePipe::UsageHint &hint, SFPTestParam &testParam, sp<SFPRequestCreater> creater)
{
    DebugTimer timer;
    timer.start();
    IStreamingFeaturePipe* pPipe = NULL;
    pPipe = IStreamingFeaturePipe::createInstance(0, hint);
    pPipe->init();
    timer.stop();
    printf("SFP init done = %d us\n", timer.getElapsedU());

    const sp<SFPCBHandlerBase> cbHandler = new StressTestEventCBHandler(creater->getMaxPlaneSize());
    cbHandler->mDumpBuffer = testParam.dumpOut;
    cbHandler->mSensorIds = hint.mAllSensorIDs;

    // parameterw for test
    const std::chrono::milliseconds requsetInteval(testParam.reqIntervalMs);
    const std::chrono::milliseconds waitForCompleteTimeOut(std::chrono::seconds(testParam.timeoutSecond));

    bool vendor = false;
    for(MUINT32 i = 0; i < testParam.requestTestCount; i++)
    {
        //if(hint.mVendorMode && i % 5 == 1)
        //    vendor = !vendor;

        FeaturePipeParam param(fpipeCB);
        //if(vendor)
            //ENABLE_VENDOR_V1(param.mFeatureMask);
        param.setVar<sp<SFPCBHandlerBase>>("testCB", cbHandler);
        creater->createRequest(param, testParam.sensorNum, testParam.genOut, testParam.largeOuts,
                testParam.physicalOuts, testParam.physicalDifferentTuning);

        pPipe->enque(param);
        printf("enque %d, wait for %lld ms for next request.\n",i , requsetInteval.count());
        std::this_thread::sleep_for(requsetInteval);

    }

    MUINT32 actualRequestCount = creater->reqNo;

    printf("total request (%d->%d) enque done\n", testParam.requestTestCount, actualRequestCount);

    MUINT32 requestCompleteCount = cbHandler->waitForDesiredCount(actualRequestCount, waitForCompleteTimeOut);

    printf("wait complete done\n");

    EXPECT_TRUE(actualRequestCount == requestCompleteCount) << "test count: " << actualRequestCount << std::endl << "complete count: " << requestCompleteCount;

    pPipe->flush();
    pPipe->uninit(PIPE_CLASS_TAG);
    pPipe->destroyInstance();
    printf("SFP uninit  done\n");
}


TEST(SFPEnque, FD_DispR_Rec)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 2;
    hint.mAllSensorIDs = {0};

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC};
    testParam.requestTestCount = 1;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s

    runTest(hint, testParam, creater);

}

TEST(SFPEnque, FD_DispR_Rec_Extra)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 3;
    hint.mAllSensorIDs = {0};

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC, TEST_EXTRA};
    testParam.requestTestCount = 20;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s

    runTest(hint, testParam, creater);
}

TEST(SFPEnque, FD_DispR_Rec_Extra_VendorSwitch)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 3;
    hint.mAllSensorIDs = {0};
    //hint.mVendorMode = 1;

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC, TEST_EXTRA};
    testParam.requestTestCount = 20;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s

    runTest(hint, testParam, creater);

}

TEST(SFPEnque, FD_DispR_Rec_Large)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 3;
    hint.mOutCfg.mHasLarge = MTRUE;
    hint.mAllSensorIDs = {0};

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC};
    testParam.largeOuts[0] = {TEST_LARGE};
    testParam.requestTestCount = 6;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s

    runTest(hint, testParam, creater);

}

TEST(SFPEnque, FD_DispR_Rec_Phy1_Phy2)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 4;
    hint.mOutCfg.mHasPhysical = MTRUE;
    hint.mAllSensorIDs = {0,1};

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC};
    testParam.physicalOuts[0] = {TEST_PHYSICAL};
    testParam.physicalOuts[1] = {TEST_PHYSICAL_2};
    testParam.requestTestCount = 10;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s
    //testParam.dumpOut = MTRUE;

    runTest(hint, testParam, creater);

}

TEST(SFPEnque, FD_DispR_Rec_Phy1Diff_Phy2Diff)
{
    SFPTestParam testParam;
    IStreamingFeaturePipe::UsageHint hint;
    sp<SFPDataMock> mock = new SFPDataMock();
    sp<SFPRequestCreater> creater = new SFPRequestCreater(mock);

    hint.mStreamingSize = mock->getImgBuf(TEST_RRZO)->getImgSize();
    hint.mOutCfg.mMaxOutNum = 4;
    hint.mOutCfg.mHasPhysical = MTRUE;
    hint.mAllSensorIDs = {0,1};

    testParam.sensorNum = hint.mAllSensorIDs.size();
    testParam.genOut = {TEST_FD, TEST_DISP_R, TEST_REC};
    testParam.physicalOuts[0] = {TEST_PHYSICAL};
    testParam.physicalOuts[1] = {TEST_PHYSICAL_2};
    testParam.physicalDifferentTuning[0] = MTRUE;
    testParam.physicalDifferentTuning[1] = MFALSE;
    testParam.requestTestCount = 10;
    testParam.reqIntervalMs = 30;
    testParam.timeoutSecond = 10; // 10s
    //testParam.dumpOut = MTRUE;

    runTest(hint, testParam, creater);

}




} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
