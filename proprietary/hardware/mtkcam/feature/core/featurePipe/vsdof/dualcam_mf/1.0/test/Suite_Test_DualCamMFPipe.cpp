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

#include "Suite_Test_DualCamMFPipe.h"

#define PIPE_MODULE_TAG "DualCamMFUnitTest"
#define PIPE_CLASS_TAG "DCMFPipe"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include "DualCamMF_UTLog.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {

//********************************************
// DCMF Stress Test
//*******************************************
TEST(/*DISABLED_*/DCMF, StressTest)
{
    MY_UTLOG("StressTest +");
    const sp<IDualCamMFPipe> dualCamMFPipe = DualCamMFPipeFactory::createDualCamMFPipe();
    const sp<IDCMFRequestCreater> dcmfRequestCreater = new DCMFRequestCreater();
    const sp<DualCamMFPipeCBHandlerBase> dcmfPipeCBHandler = new StressTestEventCBHandler();

    // parameterw for test
    const MUINT32 requestTestCount = 1;
    const std::chrono::milliseconds requsetInteval(100);
    const std::chrono::milliseconds waitForCompleteTimeOut(std::chrono::seconds(30));

    for(int i = 0; i < requestTestCount; i++)
    {
        std::vector<sp<PipeRequest>> requests = dcmfRequestCreater->createRequest(dcmfPipeCBHandler);
        for(auto request : requests)
        {
            MY_UTLOG("enque request: #%d (%p)", request->getRequestNo(), request.get());
            dualCamMFPipe->enque(request);
        }
        MY_UTLOG("wait for %lld ms for next request.", requsetInteval.count());
        std::this_thread::sleep_for(requsetInteval);
    }

    MUINT32 actualRequestCount = DCMFRequestCreater::getActualRequestCount(requestTestCount);

    MY_UTLOG("total request (%d->%d) enque done", requestTestCount, actualRequestCount);

    MUINT32 requestCompleteCount = dcmfPipeCBHandler->waitForDesiredCount(actualRequestCount, waitForCompleteTimeOut);

    MY_UTLOG("wait complete done", requestTestCount, actualRequestCount);

    EXPECT_TRUE(actualRequestCount == requestCompleteCount) << "test count: " << actualRequestCount << std::endl << "complete count: " << requestCompleteCount;

    dualCamMFPipe->sync();
    dualCamMFPipe->uninit();
    MY_UTLOG("StressTest -");
}
//********************************************
// DCMF Flush Test
//********************************************
TEST(DISABLED_DCMF, FlushTest)
{
    MY_UTLOG("FlushTest +");
    const sp<IDualCamMFPipe> dualCamMFPipe = DualCamMFPipeFactory::createDualCamMFPipe();
    const sp<IDCMFRequestCreater> dcmfRequestCreater = new DCMFRequestCreater();
    const sp<DualCamMFPipeCBHandlerBase> dcmfPipeCBHandler = new FlushTestEventCBHandler();

    // parameters for test
    const MINT32 requestTestCount = 20;
    const std::chrono::milliseconds requsetInteval(100);
    const std::chrono::milliseconds minWaitRandomTime(100);
    const std::chrono::milliseconds maxWaitRandomTime(std::chrono::seconds(2));

    for(int i = 0; i < requestTestCount; i++)
    {
        std::vector<sp<PipeRequest>> requests = dcmfRequestCreater->createRequest(dcmfPipeCBHandler);
        for(auto request : requests)
        {
            MY_UTLOG("enque request: #%d (%p)", request->getRequestNo(), request.get());
            dualCamMFPipe->enque(request);
        }
        MY_UTLOG("wait for %lld ms for next request.", requsetInteval.count());
        std::this_thread::sleep_for(requsetInteval);
    }

    std::chrono::milliseconds waitRandomTime(Utility::getRandomNumber(minWaitRandomTime.count(), maxWaitRandomTime.count()));
    MY_UTLOG("wait radom time %lld ms for flush.", waitRandomTime.count());
    std::this_thread::sleep_for(waitRandomTime);

    MY_UTLOG("do flush after sleep for %lld ms", waitRandomTime.count());
    dualCamMFPipe->flush();

    MUINT32 requestCompleteCount = dcmfPipeCBHandler->getReceivedEventCount();

    MY_UTLOG("get requestCompleteCount: %d", requestCompleteCount);
    MUINT32 actualRequestCount = DCMFRequestCreater::getActualRequestCount(requestTestCount);
    MY_UTLOG("actualRequestCount: %d, requestCompleteCount: %d", actualRequestCount, requestCompleteCount);
    EXPECT_TRUE(actualRequestCount == requestCompleteCount) << "test count: " << actualRequestCount << std::endl << "complete count: " << requestCompleteCount;

    dualCamMFPipe->sync();
    dualCamMFPipe->uninit();
    MY_UTLOG("FlushTest -");
}
//********************************************
// DCMFRequestCreater
//********************************************
std::vector<sp<PipeRequest>> DCMFRequestCreater::createRequest(sp<IDualCamMFPipeCB> cb)
{
    std::vector<sp<PipeRequest>> pipeRequests;
    bool isMainRequest = true;
    for(int i = 0 ; i < sDCMFRequestCount ; i++)
    {
        sp<PipeRequest> pipeRequest = new PipeRequest(mRequestNum++, cb);
        setData(pipeRequest);

        if(isMainRequest)
        {
            pipeRequest->addParam(DualCamMFParamID::PID_IS_MAIN, 1);
            isMainRequest = false;
        }
        else
        {
            pipeRequest->removeImageBuffer(DualCamMFBufferID::BID_POSTVIEW);
        }

        if(i == sDCMFRequestCount - 1){
            pipeRequest->addParam(DualCamMFParamID::PID_IS_LAST, 1);
        }

        MY_UTLOG("create request, #: %d (%p)", pipeRequest->getRequestNo(), pipeRequest.get());
        pipeRequests.push_back(pipeRequest);
    }
    mMFHRRequestNum++;

    return pipeRequests;
}

void DCMFRequestCreater::setData(const sp<PipeRequest>& pipeRequest)
{
    // input buffer
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_INPUT_FSRAW_1, DCMFPipeDataMock::getInstance().mpInFSMain1ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_INPUT_FSRAW_2, DCMFPipeDataMock::getInstance().mpInFSMain2ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_INPUT_RSRAW_1, DCMFPipeDataMock::getInstance().mpInRSMain1ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_INPUT_RSRAW_2, DCMFPipeDataMock::getInstance().mpInRSMain2ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_LCS_1, DCMFPipeDataMock::getInstance().mpInLCSO1ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_LCS_2, DCMFPipeDataMock::getInstance().mpInLCSO2ImgBuf);

    // output buffer
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_FS_YUV_1, DCMFPipeDataMock::getInstance().mpOutFSYUVMain1ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_FS_YUV_2, DCMFPipeDataMock::getInstance().mpOutFSYUVMain2ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_RS_YUV_1, DCMFPipeDataMock::getInstance().mpOutRSYUVMain1ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_RS_YUV_2, DCMFPipeDataMock::getInstance().mpOutRSYUVMain2ImgBuf);
    pipeRequest->addImageBuffer(DualCamMFBufferID::BID_POSTVIEW, DCMFPipeDataMock::getInstance().mpOutYUVPostviewImgBuf);

    // prepare metadatas
    IMetadata* pMetadata = createMetaData();
    pipeRequest->addMetadata(DualCamMFBufferID::BID_META_IN_APP, pMetadata);
    pipeRequest->addMetadata(DualCamMFBufferID::BID_META_IN_HAL, pMetadata);
    pipeRequest->addMetadata(DualCamMFBufferID::BID_META_IN_HAL_2, pMetadata);
}

IMetadata* DCMFRequestCreater::createMetaData()
{
    IMetadata* pMetadata = new IMetadata();
    {
        IMetadata exifMeta;
        IMetadata::IEntry entry_iso(MTK_3A_EXIF_AE_ISO_SPEED);
        entry_iso.push_back(1600, Type2Type< MINT32 >());
        exifMeta.update(entry_iso.tag(), entry_iso);
        trySetMetadata<IMetadata>(pMetadata, MTK_3A_EXIF_METADATA, exifMeta);
    }
    trySetMetadata<MINT32>(pMetadata, MTK_JPEG_ORIENTATION, 270);
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);

    IMetadata::IEntry entry(MTK_EIS_REGION);
    {
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XINT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XFLOAT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YINT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YFLOAT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_WIDTH
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HEIGHT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERX
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_ISFROMRZ
        entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVX
        entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFX
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_EXPTIME
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HWTS
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_LWTS
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MAX_GMV
        entry.push_back(0, Type2Type< MBOOL >());   // EIS_REGION_INDEX_ISFRONTBIN
    }
    pMetadata->update(MTK_EIS_REGION, entry);
    trySetMetadata<MSize>(pMetadata, MTK_P1NODE_RESIZER_SIZE, MSize(1920, 1248));

    mMetaDataVec.push_back(pMetadata);
    return pMetadata;
}

DCMFRequestCreater::~DCMFRequestCreater()
{
    for(IMetadata* pMetaData : mMetaDataVec)
    {
        delete pMetaData;
    }
}
//********************************************

} // NSDCMF
} // NSFeaturePipe
} // NSCamFeature
} // NSCam