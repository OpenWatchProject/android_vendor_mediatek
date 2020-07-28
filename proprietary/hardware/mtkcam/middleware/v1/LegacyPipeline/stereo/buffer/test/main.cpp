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
#define LOG_TAG "TestSuite_StereoBufferProviders"
//
#include <gtest/gtest.h>
//
#include <sys/prctl.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include <list>
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <utils/StrongPointer.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferPool.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoSelector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferSynchronizer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
//
#include <camutils/IImgBufQueue.h>
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
//
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v1::NSLegacyPipeline;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define FUNCTION_IN                 MY_LOGD_IF(1, "+")
#define FUNCTION_OUT                MY_LOGD_IF(1, "-")

#define MAX_TEST_REQ_COUNT          18
// global vars
namespace {

    #define USER_NAME   "Stereo buffer providers"
    //open sensor ID
    static MINT32 gSensorId = 0;
    static MINT32 gSensorId_Main2 = 2;

    static struct ImageStreamManager::image_stream_info_pool_setting gStereoImgStreamTbl[] =
    {
        { //RRZO
            "Hal:Image:ResizeRaw", eSTREAMID_IMAGE_PIPE_RAW_RESIZER, eSTREAMTYPE_IMAGE_INOUT, 6, 6,
            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
            eImgFmt_BAYER10, MSize(2304, 1296), 4320, 0, eStreamType_IMG_HAL_PROVIDER, NULL
        },
        { //IMGO
            "Hal:Image:FullRaw", eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, eSTREAMTYPE_IMAGE_INOUT, 6, 6,
            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
            eImgFmt_BAYER10, MSize(4176, 3088), 5220, 0, eStreamType_IMG_HAL_PROVIDER, NULL
        },
        { //RRZO Main2
            "Hal:Image:ResizeRaw_Main2", eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, eSTREAMTYPE_IMAGE_INOUT, 6, 6,
            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
            eImgFmt_BAYER10, MSize(1600, 1200), 3000, 0, eStreamType_IMG_HAL_PROVIDER, NULL
        },
        //End
        {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), 0},
    };

    IMetadata metadata[MAX_TEST_REQ_COUNT];
    IMetadata metadata_main2[MAX_TEST_REQ_COUNT];
};// end of global vars
/******************************************************************************
 *
 ******************************************************************************/
int prepareMetadata(){
    FUNCTION_IN;
    printf("prepareMetadata +\n");

    for(int i = 0 ; i < MAX_TEST_REQ_COUNT; i++){
        MINT64 timestamp = (i*33)*1000000; // transform to us
        // printf("req %d,  MTK_SENSOR_TIMESTAMP=%lld\n",i, timestamp);
        printf("req %d,  MTK_SENSOR_TIMESTAMP(ms)=%lld\n",i, timestamp/1000000);
        MY_LOGD("req %d,  MTK_SENSOR_TIMESTAMP(ms)=%lld\n",i, timestamp/1000000);
        IMetadata::IEntry entry(MTK_SENSOR_TIMESTAMP);
        entry.push_back(timestamp, Type2Type< MINT64 >());

        metadata[i].update(MTK_SENSOR_TIMESTAMP, entry);
        metadata_main2[i].update(MTK_SENSOR_TIMESTAMP, entry);
    }

    FUNCTION_OUT;
    printf("prepareMetadata -\n");
    return MAX_TEST_REQ_COUNT;
}
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
void showTimestamp(Vector<ISelector::MetaItemSet> metadata){
    MINT64 timestamp = -1;
    int timestamp_ms;

    // fetch the metadata with timestamp
    for ( size_t i = 0; i < metadata.size(); ++i) {
        // MY_LOGD1("tryGetMetadata bufInfoMain1->metadata %d", i);
        if ( tryGetMetadata<MINT64>(const_cast<IMetadata*>(&metadata[i].meta), MTK_SENSOR_TIMESTAMP, timestamp) ) {
            // MY_LOGD1("tryGetMetadata bufInfoMain1->metadata %d, suceess!", i);
            break;
        }
    }

    timestamp_ms = timestamp/1000000;
    MY_LOGD("timestamp_ms:%09d", timestamp_ms);
    printf("timestamp_ms:%09d\n", timestamp_ms);
}
/******************************************************************************
 *
 ******************************************************************************/
// TEST(StereoBufferProviders, SimpleTest){
//     MY_LOGD("SimpleTest+");
//     printf("SimpleTest+\n");

//     // create ImageStream Manager
//     MY_LOGD("create ImageStream Manager");
//     sp<ImageStreamManager> mspImageStreamMgr    = ImageStreamManager::create(gStereoImgStreamTbl);

//     // main1 rrzo
//     MY_LOGD("create buf provider for main1");
//     sp<IImageStreamInfo> pImage_RrzoRaw = mspImageStreamMgr->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);

//     // create pairMetadata
//     MY_LOGD("create pairMetadata");
//     sp<IResourceContainer> mpResourceContainier = IResourceContainer::getInstance(gSensorId);

//     sp<PairMetadata> pPair;
//     pPair = PairMetadata::createInstance(pImage_RrzoRaw->getStreamName());

//     sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
//     pFactory->setImageStreamInfo(pImage_RrzoRaw);
//     pFactory->setPairRule(pPair, 1);
//     sp<StreamBufferProvider> pProducer = pFactory->create(gSensorId, MTRUE);
//     EXPECT_EQ(true, pProducer!=NULL) << "pFactory->create";

//     // query consumer and set selector
//     MY_LOGD("query consumer and set selector");
//     sp<StreamBufferProvider> consumer_resizer = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
//     EXPECT_EQ(true, consumer_resizer!=NULL) << "query consumer";
//     sp<StereoSelector> mpStereoSelector_RESIZER = new StereoSelector("RESIZER");
//     EXPECT_EQ(OK, consumer_resizer->setSelector(mpStereoSelector_RESIZER)) << "set selector";

//     // deque
//     MY_LOGD("deque from producer");
//     sp<HalImageStreamBuffer> dequedStreamBuffer;
//     int reqNo = 0;
//     EXPECT_EQ(OK, pProducer->dequeStreamBuffer(reqNo, pImage_RrzoRaw, dequedStreamBuffer)) << "dequeStreamBuffer pImage_RrzoRaw reqNo = " << reqNo;

//     // enque
//     MY_LOGD("enque to producer");
//     EXPECT_EQ(OK, pProducer->enqueStreamBuffer(pImage_RrzoRaw, dequedStreamBuffer, STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer pImage_RrzoRaw reqNo = " << reqNo;

//     EXPECT_EQ(OK, OK) << "test done";
//     MY_LOGD("SimpleTest-");
//     printf("SimpleTest-\n");
// }
/******************************************************************************
 *
 ******************************************************************************/
TEST(StereoBufferProviders, selectorTest){
    MY_LOGD("selectorTest+");
    printf("selectorTest+\n");

    /******************************************************************************
     * selector and synchronizer init
     ******************************************************************************/
    // setup selector and synchronizers
    MY_LOGD("create stereoSynchronizer");
    sp<StereoBufferSynchronizer> mpStereoBufferSynchronizer = StereoBufferSynchronizer::createInstance("Synchronizer", 3);
    // MY_LOGE("create stereoSynchronizer with debug mode");
    // mpStereoBufferSynchronizer->setDebugMode(StereoBufferSynchronizer::DEBUG_MODE::SKIP_METADATA_CHECK);
    mpStereoBufferSynchronizer->setZSLDelayCount(3);

    // create StereoSelectors
    MY_LOGD("create StereoSelectors");
    sp<StereoSelector> mpStereoSelector_RESIZER         = new StereoSelector("RESIZER");
    sp<StereoSelector> mpStereoSelector_RESIZER_MAIN2   = new StereoSelector("RESIZER_01");
    sp<StereoSelector> mpStereoSelector_OPAQUE          = new StereoSelector("OPAQUE");

    MY_LOGD("bind stream/selector pair to synchronizer");
    mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,      mpStereoSelector_OPAQUE);
    mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER,     mpStereoSelector_RESIZER);
    mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,  mpStereoSelector_RESIZER_MAIN2);

    MY_LOGD("start synchronizer");
    mpStereoBufferSynchronizer->start();

    // create ImageStream Manager
    MY_LOGD("create ImageStream Manager");
    sp<ImageStreamManager> mspImageStreamMgr    = ImageStreamManager::create(gStereoImgStreamTbl);

    /******************************************************************************
     * create providers (producer/P1)
     ******************************************************************************/
    // create providers
    MY_LOGD("create buf providers");
    sp<IImageStreamInfo> pImage_ImgoRaw       = mspImageStreamMgr->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    sp<IImageStreamInfo> pImage_RrzoRaw       = mspImageStreamMgr->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
    sp<IImageStreamInfo> pImage_RrzoRaw_main2 = mspImageStreamMgr->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);

    // must called before create provider
    sp<IResourceContainer> mpResourceContainier = IResourceContainer::getInstance(gSensorId);
    sp<IResourceContainer> mpResourceContainierMain2 = IResourceContainer::getInstance(gSensorId_Main2);

    // main1 imgo
    MY_LOGD("create buf provider for main1 imgo");
    sp<PairMetadata> pPair_imgo;
    pPair_imgo = PairMetadata::createInstance(pImage_ImgoRaw->getStreamName());

    sp<StreamBufferProviderFactory> pFactory_imgo = StreamBufferProviderFactory::createInstance();
    pFactory_imgo->setImageStreamInfo(pImage_ImgoRaw);
    pFactory_imgo->setPairRule(pPair_imgo, 1);
    sp<StreamBufferProvider> pProducer_imgo = pFactory_imgo->create(gSensorId, MTRUE);
    EXPECT_EQ(true, pProducer_imgo!=NULL) << "pFactory_imgo->create";

    // main1 rrzo
    MY_LOGD("create buf provider for main1 rrzo");
    sp<PairMetadata> pPair_rrzo;
    pPair_rrzo = PairMetadata::createInstance(pImage_RrzoRaw->getStreamName());

    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
    pFactory->setImageStreamInfo(pImage_RrzoRaw);
    pFactory->setPairRule(pPair_rrzo, 1);
    sp<StreamBufferProvider> pProducer_rrzo = pFactory->create(gSensorId, MTRUE);
    EXPECT_EQ(true, pProducer_rrzo!=NULL) << "pFactory->create";

    // main2
    MY_LOGD("create buf provider for main2");
    sp<PairMetadata> pPair_rrzo_main2;
    pPair_rrzo_main2 = PairMetadata::createInstance(pImage_RrzoRaw_main2->getStreamName());

    sp<StreamBufferProviderFactory> pFactory_main2 = StreamBufferProviderFactory::createInstance();
    pFactory_main2->setImageStreamInfo(pImage_RrzoRaw_main2);
    pFactory_main2->setPairRule(pPair_rrzo_main2, 1);
    sp<StreamBufferProvider> pProducer_rrzo_main2 = pFactory_main2->create(gSensorId_Main2, MTRUE);
    EXPECT_EQ(true, pProducer_rrzo_main2!=NULL) << "pFactory_main2->create";

    /******************************************************************************
     * create providers (user/P2)
     ******************************************************************************/
    // create user pool
     MY_LOGD("create user pool");
    sp<StereoBufferPool> mpStereoBufferPool_OPAQUE= StereoBufferPool::createInstance(pImage_ImgoRaw->getStreamName(), pImage_ImgoRaw->getStreamId());
    mpStereoBufferPool_OPAQUE->setSelector(mpStereoSelector_OPAQUE);
    mpStereoBufferPool_OPAQUE->setSynchronizer(mpStereoBufferSynchronizer);

    sp<StereoBufferPool> mpStereoBufferPool_RESIZER= StereoBufferPool::createInstance(pImage_RrzoRaw->getStreamName(), pImage_RrzoRaw->getStreamId());
    mpStereoBufferPool_RESIZER->setSelector(mpStereoSelector_RESIZER);
    mpStereoBufferPool_RESIZER->setSynchronizer(mpStereoBufferSynchronizer);

    sp<StereoBufferPool> mpStereoBufferPool_RESIZER_MAIN2 = StereoBufferPool::createInstance(pImage_RrzoRaw_main2->getStreamName(), pImage_RrzoRaw_main2->getStreamId());
    mpStereoBufferPool_RESIZER_MAIN2->setSelector(mpStereoSelector_RESIZER_MAIN2);
    mpStereoBufferPool_RESIZER_MAIN2->setSynchronizer(mpStereoBufferSynchronizer);

    mpStereoBufferPool_OPAQUE->setReturnMode(StereoBufferPool::RETURN_MODE::TO_SYNCHRONIZER);
    mpStereoBufferPool_RESIZER->setReturnMode(StereoBufferPool::RETURN_MODE::TO_SYNCHRONIZER);
    mpStereoBufferPool_RESIZER_MAIN2->setReturnMode(StereoBufferPool::RETURN_MODE::TO_SYNCHRONIZER);

    // create provider
    android::sp<StreamBufferProvider> ImgoUser = NULL;
    android::sp<StreamBufferProvider> RrzoUser = NULL;
    android::sp<StreamBufferProvider> RrzoUser_Main2 = NULL;
    {
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        pFactory->setImageStreamInfo(pImage_ImgoRaw);
        pFactory->setUsersPool(mpStereoBufferPool_OPAQUE);

        ImgoUser = pFactory->create();
        EXPECT_EQ(true, ImgoUser!=NULL) << "ImgoUser!=NULL";
    }
    {
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        pFactory->setImageStreamInfo(pImage_RrzoRaw);
        pFactory->setUsersPool(mpStereoBufferPool_RESIZER);

        RrzoUser = pFactory->create();
        EXPECT_EQ(true, RrzoUser!=NULL) << "RrzoUser!=NULL";
    }
    {
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        pFactory->setImageStreamInfo(pImage_RrzoRaw_main2);
        pFactory->setUsersPool(mpStereoBufferPool_RESIZER_MAIN2);

        RrzoUser_Main2 = pFactory->create();
        EXPECT_EQ(true, RrzoUser_Main2!=NULL) << "RrzoUser_Main2!=NULL";
    }

    /******************************************************************************
     * binding selectors
     ******************************************************************************/
    // query consumers
    MY_LOGD("query consumers");
    sp<StreamBufferProvider> consumer_opaque        = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    sp<StreamBufferProvider> consumer_resizer       = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
    sp<StreamBufferProvider> consumer_resizer_01    = mpResourceContainierMain2->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
    EXPECT_EQ(true, consumer_opaque!=NULL) << "consumer_opaque!=NULL";
    EXPECT_EQ(true, consumer_resizer!=NULL) << "consumer_resizer!=NULL";
    EXPECT_EQ(true, consumer_resizer_01!=NULL) << "consumer_resizer_01!=NULL";

    // bind selectors to consumers
    MY_LOGD("binding selectors to consumers");
    EXPECT_EQ(OK, consumer_opaque->setSelector(mpStereoSelector_OPAQUE)) << "consumer_opaque->setSelector";
    EXPECT_EQ(OK, consumer_resizer->setSelector(mpStereoSelector_RESIZER)) << "consumer_resizer->setSelector";
    EXPECT_EQ(OK, consumer_resizer_01->setSelector(mpStereoSelector_RESIZER_MAIN2)) << "consumer_resizer_01->setSelector";

    /******************************************************************************
     * start request loop
     ******************************************************************************/
    // prepare metadata
    MY_LOGD("prepare metadata");
    printf("prepare metadata\n");
    int max_req_num = prepareMetadata();
    EXPECT_EQ(true, max_req_num > 0) << "prepareMetadata()";

    for(int reqNo = 0 ; reqNo < max_req_num ; reqNo ++){
    // for(int reqNo = 0 ; reqNo < 3 ; reqNo ++){
        // producer deque
        MY_LOGD("reqNo=%d", reqNo);
        printf("reqNo=%d\n", reqNo);

        MY_LOGD("deque from producer main1");
        printf("deque from producer main1\n");


        sp<HalImageStreamBuffer> dequedStreamBuffer_imgo;
        sp<HalImageStreamBuffer> dequedStreamBuffer_rrzo;
        sp<HalImageStreamBuffer> dequedStreamBuffer_rrzo_main2;
        EXPECT_EQ(OK, pProducer_imgo->dequeStreamBuffer(reqNo,          pImage_ImgoRaw,          dequedStreamBuffer_imgo)) << "dequeStreamBuffer pImage_ImgoRaw reqNo = " << reqNo;
        EXPECT_EQ(OK, pProducer_rrzo->dequeStreamBuffer(reqNo,          pImage_RrzoRaw,          dequedStreamBuffer_rrzo)) << "dequeStreamBuffer pImage_RrzoRaw reqNo = " << reqNo;
        EXPECT_EQ(OK, pProducer_rrzo_main2->dequeStreamBuffer(reqNo,    pImage_RrzoRaw_main2,    dequedStreamBuffer_rrzo_main2)) << "dequeStreamBuffer dequedStreamBuffer pImage_RrzoRaw_main2 reqNo = " << reqNo;

        // producer doTimestampCallback
        MY_LOGD("deque producer doTimestampCallback");
        printf("deque producer doTimestampCallback\n");

        MINT64 dummyTimestamp = reqNo;
        pProducer_imgo->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
        pProducer_rrzo->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
        pProducer_rrzo_main2->doTimestampCallback(  reqNo, MFALSE, dummyTimestamp);

        // pair metadata onResultRecieved
        MY_LOGD("pair metadata onResultRecieved");
        printf("pair metadata onResultRecieved\n");

        pPair_imgo->onResultReceived(reqNo,         eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,        false, metadata[reqNo]);
        pPair_rrzo->onResultReceived(reqNo,         eSTREAMID_IMAGE_PIPE_RAW_RESIZER,       false, metadata[reqNo]);
        pPair_rrzo_main2->onResultReceived(reqNo,   eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,    false, metadata_main2[reqNo]);

        // producer enque
        MY_LOGD("producer enque");
        printf("producer enque\n");

        MY_LOGD("pProducer_imgo");
        EXPECT_EQ(OK, pProducer_imgo->enqueStreamBuffer(pImage_ImgoRaw,             dequedStreamBuffer_imgo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer pImage_ImgoRaw reqNo = " << reqNo;
        MY_LOGD("pProducer_rrzo");
        EXPECT_EQ(OK, pProducer_rrzo->enqueStreamBuffer(pImage_RrzoRaw,             dequedStreamBuffer_rrzo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer pImage_RrzoRaw reqNo = " << reqNo;
        MY_LOGD("pProducer_rrzo_main2");
        EXPECT_EQ(OK, pProducer_rrzo_main2->enqueStreamBuffer(pImage_RrzoRaw_main2, dequedStreamBuffer_rrzo_main2,  STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer pImage_RrzoRaw_main2 reqNo = " << reqNo;

        dequedStreamBuffer_imgo = NULL;
        dequedStreamBuffer_rrzo = NULL;
        dequedStreamBuffer_rrzo_main2 = NULL;

        // user deque & enque
        android::sp<IImageBufferHeap> spHeap_imgo           = NULL;
        android::sp<IImageBufferHeap> spHeap_rrzo           = NULL;
        android::sp<IImageBufferHeap> spHeap_rrzo_main2     = NULL;
        MINT32 reqNo_get_result = -1;
        Vector<ISelector::MetaItemSet> tempMetadata;

        MY_LOGD("mpStereoBufferSynchronizer->lock() +");
        printf("mpStereoBufferSynchronizer->lock() +\n");
        EXPECT_EQ(OK, mpStereoBufferSynchronizer->lock()) << "mpStereoBufferSynchronizer->lock()";
        MY_LOGD("mpStereoBufferSynchronizer->lock() -");
        printf("mpStereoBufferSynchronizer->lock() -\n");

        // get result
        {
            MY_LOGD("get result from selector");
            printf("get result from selector\n");

            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_OPAQUE->getResult(reqNo_get_result, tempMetadata, spHeap_imgo)) << "mpStereoSelector_OPAQUE->getResult";
            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_RESIZER->getResult(reqNo_get_result, tempMetadata, spHeap_rrzo)) << "mpStereoSelector_RESIZER->getResult";
            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_RESIZER_MAIN2->getResult(reqNo_get_result, tempMetadata, spHeap_rrzo_main2)) << "mpStereoSelector_RESIZER_MAIN2->getResult";
        }

        // set buffer
        {
            MY_LOGD("set buffer to buffer pool");
            printf("set buffer to buffer pool\n");

            mpStereoBufferPool_OPAQUE->setBuffer("spHeap_full", spHeap_imgo);
            mpStereoBufferPool_RESIZER->setBuffer("spHeap_resized", spHeap_rrzo);
            mpStereoBufferPool_RESIZER_MAIN2->setBuffer("spHeap_resized_main2", spHeap_rrzo_main2);
        }

        spHeap_imgo           = NULL;
        spHeap_rrzo           = NULL;
        spHeap_rrzo_main2     = NULL;

        // deque  & enque buffer
        {
            sp<HalImageStreamBuffer> user_dequedStreamBuffer_imgo;
            sp<HalImageStreamBuffer> user_dequedStreamBuffer_rrzo;
            sp<HalImageStreamBuffer> user_dequedStreamBuffer_rrzo_main2;

            MY_LOGD("user deque buffer");
            printf("user deque buffer\n");

            EXPECT_EQ(OK, ImgoUser->dequeStreamBuffer(reqNo,        pImage_ImgoRaw,         user_dequedStreamBuffer_imgo)) << "dequeStreamBuffer user_dequedStreamBuffer_imgo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser->dequeStreamBuffer(reqNo,        pImage_RrzoRaw,         user_dequedStreamBuffer_rrzo)) << "dequeStreamBuffer user_dequedStreamBuffer_rrzo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser_Main2->dequeStreamBuffer(reqNo,  pImage_RrzoRaw_main2,   user_dequedStreamBuffer_rrzo_main2)) << "dequeStreamBuffer user_dequedStreamBuffer_rrzo_main2 reqNo = " << reqNo;

            // producer onResultRecieved
            MY_LOGD("user doTimestampCallback");
            printf("user doTimestampCallback\n");

            MINT64 dummyTimestamp = reqNo;
            ImgoUser->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
            RrzoUser->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
            RrzoUser_Main2->doTimestampCallback(  reqNo, MFALSE, dummyTimestamp);

            MY_LOGD("user enque buffer");
            printf("user enque buffer\n");

            EXPECT_EQ(OK, ImgoUser->enqueStreamBuffer(pImage_ImgoRaw,             user_dequedStreamBuffer_imgo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer user_dequedStreamBuffer_imgo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser->enqueStreamBuffer(pImage_RrzoRaw,             user_dequedStreamBuffer_rrzo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer user_dequedStreamBuffer_rrzo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser_Main2->enqueStreamBuffer(pImage_RrzoRaw_main2, user_dequedStreamBuffer_rrzo_main2,  STREAM_BUFFER_STATUS::WRITE_OK)) << "enqueStreamBuffer user_dequedStreamBuffer_rrzo_main2 reqNo = " << reqNo;
        }

        MY_LOGD("mpStereoBufferSynchronizer->unlock() +");
        printf("mpStereoBufferSynchronizer->unlock() +\n");
        EXPECT_EQ(OK, mpStereoBufferSynchronizer->unlock()) << "mpStereoBufferSynchronizer->unlock()";
        MY_LOGD("mpStereoBufferSynchronizer->unlock() -");
        printf("mpStereoBufferSynchronizer->unlock() -\n");

        // lock ZSL buffer
        {
            if(reqNo == 2 || reqNo == 5 || reqNo == 8 || reqNo == 11){
            // if(reqNo == 2){
                MY_LOGD("ZSL do ZSL test + ");
            }else{
                continue;
            }

            MY_LOGD("ZSL mpStereoBufferSynchronizer->waitAndLockCapture() +");
            EXPECT_EQ(OK, mpStereoBufferSynchronizer->waitAndLockCapture()) << "ZSL mpStereoBufferSynchronizer->waitAndLockCapture()";
            MY_LOGD("ZSL mpStereoBufferSynchronizer->waitAndLockCapture() -");

            MY_LOGD("ZSL get result ZSL from selector");
            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_OPAQUE->getResultCapture(reqNo_get_result, tempMetadata, spHeap_imgo)) << "ZSL mpStereoSelector_OPAQUE->getResultCapture";
            showTimestamp(tempMetadata);

            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_RESIZER->getResultCapture(reqNo_get_result, tempMetadata, spHeap_rrzo)) << "ZSL mpStereoSelector_RESIZER->getResultCapture";
            showTimestamp(tempMetadata);

            tempMetadata.clear();
            EXPECT_EQ(OK, mpStereoSelector_RESIZER_MAIN2->getResultCapture(reqNo_get_result, tempMetadata, spHeap_rrzo_main2)) << "ZSL mpStereoSelector_RESIZER_MAIN2->getResultCapture";
            showTimestamp(tempMetadata);

            MY_LOGD("ZSL mpStereoBufferSynchronizer->unlockCapture() +");
            EXPECT_EQ(OK, mpStereoBufferSynchronizer->unlockCapture()) << "ZSL mpStereoBufferSynchronizer->unlockCapture()";
            MY_LOGD("ZSL mpStereoBufferSynchronizer->unlockCapture() -");

            MY_LOGD("ZSL set buffer to buffer pool");
            mpStereoBufferPool_OPAQUE->setBuffer("spHeap_full", spHeap_imgo);
            mpStereoBufferPool_RESIZER->setBuffer("spHeap_resized", spHeap_rrzo);
            mpStereoBufferPool_RESIZER_MAIN2->setBuffer("spHeap_resized_main2", spHeap_rrzo_main2);

            sp<HalImageStreamBuffer> user_dequedStreamBuffer_imgo;
            sp<HalImageStreamBuffer> user_dequedStreamBuffer_rrzo;
            sp<HalImageStreamBuffer> user_dequedStreamBuffer_rrzo_main2;

            MY_LOGD("ZSL user deque buffer");
            EXPECT_EQ(OK, ImgoUser->dequeStreamBuffer(reqNo,        pImage_ImgoRaw,         user_dequedStreamBuffer_imgo)) << "ZSL dequeStreamBuffer user_dequedStreamBuffer_imgo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser->dequeStreamBuffer(reqNo,        pImage_RrzoRaw,         user_dequedStreamBuffer_rrzo)) << "ZSL dequeStreamBuffer user_dequedStreamBuffer_rrzo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser_Main2->dequeStreamBuffer(reqNo,  pImage_RrzoRaw_main2,   user_dequedStreamBuffer_rrzo_main2)) << "ZSL dequeStreamBuffer user_dequedStreamBuffer_rrzo_main2 reqNo = " << reqNo;

            // producer onResultRecieved
            MY_LOGD("ZSL user doTimestampCallback");
            MINT64 dummyTimestamp = reqNo;
            ImgoUser->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
            RrzoUser->doTimestampCallback(        reqNo, MFALSE, dummyTimestamp);
            RrzoUser_Main2->doTimestampCallback(  reqNo, MFALSE, dummyTimestamp);

            MY_LOGD("ZSL user enque buffer");
            EXPECT_EQ(OK, ImgoUser->enqueStreamBuffer(pImage_ImgoRaw,             user_dequedStreamBuffer_imgo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "ZSL enqueStreamBuffer user_dequedStreamBuffer_imgo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser->enqueStreamBuffer(pImage_RrzoRaw,             user_dequedStreamBuffer_rrzo,        STREAM_BUFFER_STATUS::WRITE_OK)) << "ZSL enqueStreamBuffer user_dequedStreamBuffer_rrzo reqNo = " << reqNo;
            EXPECT_EQ(OK, RrzoUser_Main2->enqueStreamBuffer(pImage_RrzoRaw_main2, user_dequedStreamBuffer_rrzo_main2,  STREAM_BUFFER_STATUS::WRITE_OK)) << "ZSL enqueStreamBuffer user_dequedStreamBuffer_rrzo_main2 reqNo = " << reqNo;

            MY_LOGD("ZSL do ZSL test - ");
        }
    }

    // flush synchronizer to return buffers
    MY_LOGD("flush synchronizer to return buffers");
    mpStereoBufferSynchronizer->flush();

    MY_LOGD("selectorTest-");
    printf("selectorTest-\n");
}

// /******************************************************************************
//  *
//  ******************************************************************************/
// namespace TestSuite_StereoBufferProviders
// {
//     sp<StereoBufferSynchronizer> gStereoBufferSynchronizer;
//     sp<StereoSelector>           gStereoSelector_RESIZER;
//     sp<StereoSelector>           gpStereoSelector_RESIZER_MAIN2;
//     sp<StereoSelector>           gStereoSelector_OPAQUE;
// }
// pthread_t               mThreadResized;
// pthread_t               mThreadResizedMain2;
// pthread_t               mThreadFull;
// pthread_t               mThreadP2;
// TEST(StereoBufferProviders, multiThreadTest){
//     MY_LOGD("multiThreadTest+");
//     printf("multiThreadTest+\n");

//     // config

//     // start each thread

//     // clear global var

//     MY_LOGD("multiThreadTest-");
//     printf("multiThreadTest-\n");
// }
/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}