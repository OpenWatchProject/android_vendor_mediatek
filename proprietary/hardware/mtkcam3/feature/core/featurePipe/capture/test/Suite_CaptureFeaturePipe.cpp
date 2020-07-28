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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

// Standard C header file
#include <string>
// Android system/core header file
#include <gtest/gtest.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>
// Local header file
#include "CaptureFeaturePipeUT_Environment.h"
#include "CaptureFeaturePipeUT_Common.h"
// Log
#define LOG_TAG "CapturePipeUT/Suite_CaptureFeaturePipe"
#include "CaptureFeaturePipeUT_log.h"


/*******************************************************************************
* TODO
*******************************************************************************/
// 1. use builder pattern to build the request
// 2. add flush test
// 3. add stress test

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {
namespace UnitTest {


/*******************************************************************************
* Alias.
********************************************************************************/
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSCapture;


/*******************************************************************************
* TEST Define
*******************************************************************************/
TEST(PipeTest, InitAndUnint)
{
    SCOPED_TRACER();

    const MUINT32 main1Index = MYEnvironment::getMain1Index();

    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    pipePtr->uninit();
    pipePtr = nullptr;

    SUCCEED();
}

TEST(DISABLED_FeatureTest, BokehBasicTest)
{
    SCOPED_TRACER();

    const char* name = "BokehBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_MAN_OUT_DEPTH, new WorkingBufferHandle(name, eImgFmt_Y8, depthSize)); // NOTE
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_BOKEH);
    requestPtr->addFeature(FID_DEPTH);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;
    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}

TEST(DISABLED_FeatureTest, FusionBasicTest)
{
    SCOPED_TRACER();

    const char* name = "FusionBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    //requestPtr->addBuffer(BID_MAN_OUT_DEPTH, new WorkingBufferHandle(name, eImgFmt_Y8, depthSize));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_FUSION);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;

    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}


TEST(DISABLED_FeatureTest, SWNRBasicTest)
{
    const char* name = "SWNRBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize, sLCSO1Filename));
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_NR);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;

    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}

// int main2(int argc, char **argv)
// {
//     // 1. power on sensor
//     NSCam::IHalSensor *sensor = NULL;
//     MUINT32 index[] = { 0, 1 };
//     const MUINT32 size = sizeof(index)/sizeof(index[0]);
//     sensor = powerOnCamSensor(index, size);



//     // 2. create feature pipe
//     ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
//     sp<ICaptureFeaturePipe> pPipe = ICaptureFeaturePipe::createInstance(0, usage);

//     pPipe->init();

//     sp<RequestCallback> callback = new TestCallback();

//     pPipe->setCallback(callback);
//     sp<ICaptureFeatureRequest> requestPtr;

// #if 0
//     // MFNR Frame 1
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, yuvSize));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, thumbSize));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 0);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // MFNR Frame 2
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 1);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // MFNR Frame 3
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 2);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

// #endif

// #if 1
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, yuvSize));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_YUY2, thumbSize));
//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);
// #endif
// #if 0
//     // 3. test MFNR + SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 3. test RAW-HDR + SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_HDR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // 3. test SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // 4. test BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 5. test MFNR + SWNR

//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_MFNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 6. test MFNR
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

// #endif
//     sleep(1);

//     // x. power off
//     powerOffCamSensor(sensor, index, size);

//     return 0;
// }

} // UnitTest
} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
