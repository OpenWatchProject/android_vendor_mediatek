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
#define LOG_TAG "JpegNodeTest"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam3/pipeline/pipeline/IPipelineDAG.h>
#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>
#include <v3/pipeline/IPipelineNodeMapControl.h>
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
//
#include <mtkcam3/pipeline/utils/streambuf/StreamBufferPool.h>
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam3/pipeline/hwnode/JpegNode.h>
//[++]
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;
using namespace NSCam::v3::NSPipelineContext;
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
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");


static MUINT32 g_u4Transform = 0;
static MUINT32 g_u4TargetWidth = 4096, g_u4TargetHeight = 3072;
//
static MUINT32 g_u4SrcWidth = 4096, g_u4SrcHeight = 3072;
static MUINT32 g_u4SrcWidth_thumb = 176, g_u4SrcHeight_thumb = 132;
/******************************************************************************
 *
 ******************************************************************************/
void help()
{
    printf("JpegNode <test>\n");
}

static
sp<ImageStreamInfo> createImageStreamInfo(
        char const*         streamName,
        StreamId_T          streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        size_t const        stride
        );

static
sp<ImageStreamInfo> createImageStreamInfo(
        char const*         streamName,
        StreamId_T          streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        MUINT32             transform = 0
        );

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    enum STREAM_ID{
        STREAM_ID_IN_YUV_Main = 1,
        STREAM_ID_IN_YUV_Thumbnail,
        STREAM_ID_APPREQUEST,//app metadata in
        STREAM_ID_HALREQUEST,//hal metadata in
        STREAM_ID_APPMETADATA,//app metadata out
        STREAM_ID_OUT_JPEG

    };

    enum NODE_ID{
        NODE_ID_NODE1 = 1,
        NODE_ID_NODE2,
    };


    class AppSimulator
        : public virtual RefBase
    {
    };

    //
    android::sp<AppSimulator>           mpAppSimulator;
    //
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageYuvMain;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageYuvThumbnail;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageJpeg;
    //sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalMetadataRequest;
    //sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_AppMetadataRequest;
    //sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_AppMetadataResult;
    //
    IHalSensor* mpSensorHalObj;
    //
    typedef NSCam::v3::Utils::IStreamInfoSetControl       IStreamInfoSetControlT;
    android::sp<IStreamInfoSetControlT> mpStreamInfoSet;
    android::sp<IPipelineNodeMapControl>mpPipelineNodeMap;
    android::sp<IPipelineDAG>           mpPipelineDAG;
    android::sp<JpegNode>                 mpNode1;
    //
    static int gSensorId = 0;
    static MUINT32 requestTemplate = CAMERA3_TEMPLATE_PREVIEW;
    android::sp<IPipelineContext> gContext;
        // StreamInfos
    sp<IMetaStreamInfo>         gControlMeta_App;
    sp<IMetaStreamInfo>         gControlMeta_Hal;
    sp<IMetaStreamInfo>         gResultMeta_Jpeg_App;

    sp<IImageStreamInfo>        gImage_Yuv;
    sp<IImageStreamInfo>        gImage_Yuv_Thumbnail;
    sp<IImageStreamInfo>        gImage_Jpeg;

}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineNodeMapControl>
getPipelineNodeMapControl()
{
    return mpPipelineNodeMap;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IStreamInfoSet>
getStreamInfoSet()
{
    return mpStreamInfoSet;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineNodeMap>
getPipelineNodeMap()
{
    return mpPipelineNodeMap;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineDAG>
getPipelineDAG()
{
    return mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
void clear_global_var()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void setupMetaStreamInfo()
{
    FUNCTION_IN
    gControlMeta_App =
        new MetaStreamInfo(
                "App Jpeg in Request",
                STREAM_ID_APPREQUEST,
                eSTREAMTYPE_META_IN,
                0
        );
    gControlMeta_Hal =
        new MetaStreamInfo(
                "Hal P2 in Request",
                STREAM_ID_HALREQUEST,
                eSTREAMTYPE_META_IN,
                0
        );
    gResultMeta_Jpeg_App =
        new MetaStreamInfo(
                "App Jpeg out Metadata",
                STREAM_ID_APPMETADATA,
                eSTREAMTYPE_META_OUT,
                0
        );
}


void setupImageStreamInfo()
{
    FUNCTION_IN

    /*{
        MSize const& size = MSize(g_u4TargetWidth, g_u4TargetHeight);
        MINT const format = eImgFmt_BLOB;
        size_t const stride = size.w * size.h * 12 / 10;
        MUINT const usage = eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
        gImage_Jpeg = createImageStreamInfo(
                "App:Image:Jpeg",
                STREAM_ID_OUT_JPEG,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size, stride
                );
    }*/
    {
        StreamId_T const streamId = STREAM_ID_OUT_JPEG;
                MSize const imgSize(g_u4TargetWidth, g_u4TargetHeight);
                MINT const format = eImgFmt_BLOB;//eImgFmt_JPEG
                MUINT const usage = eBUFFER_USAGE_SW_MASK   |
                                    eBUFFER_USAGE_HW_CAMERA_READWRITE
                                    ;

                IImageStreamInfo::BufPlanes_t bufPlanes;
                //[++]
                IImageStreamInfo::BufPlane bufPlane;
                bufPlanes.count = 1;
                bufPlanes.planes[0].rowStrideInBytes = imgSize.w * imgSize.h * 12 / 10;//[++]
                bufPlanes.planes[0].sizeInBytes = bufPlane.rowStrideInBytes;
                //IImageStreamInfo::BufPlane bufPlane = { (unsigned int)(bufPlane.rowStrideInBytes * imgSize.h * 1.2), (unsigned int)(bufPlane.rowStrideInBytes * imgSize.h * 1.2) };
                //
                sp<ImageStreamInfo>
                pStreamInfo = new ImageStreamInfo(
                    "Hal:Image:Jpeg",
                    streamId,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, imgSize, bufPlanes
                );
                gImage_Jpeg = pStreamInfo;
    }
    {
        MSize const& size = MSize(g_u4SrcWidth, g_u4SrcHeight);
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = eBUFFER_USAGE_SW_MASK |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE;
        gImage_Yuv = createImageStreamInfo(
                "Hal:Image YUV_Main",
                STREAM_ID_IN_YUV_Main,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size
                );
    }
    {
        MSize const& size = MSize(g_u4SrcWidth_thumb, g_u4SrcHeight_thumb);
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = eBUFFER_USAGE_SW_MASK |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE;
        gImage_Yuv_Thumbnail = createImageStreamInfo(
                "Hal:Image YUV_Thumbnail",
                STREAM_ID_IN_YUV_Thumbnail,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size
                );
    }

}

/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(idx, planes, height, stride)                                 \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            planes[idx] = { _height * _stride, _stride }; \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
            bufPlanes.count = 3;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w);
            addBufPlane(1, bufPlanes.planes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(2, bufPlanes.planes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            bufPlanes.count = 2;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w);
            addBufPlane(1, bufPlanes.planes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w << 1);
            break;
        default:
            MY_LOGE("format not support yet %p", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    size_t const        stride
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    bufPlanes.count = 1;
    //
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            planes[0] = { _height * _stride, _stride }; \
        }while(0)
#define addBlobBufPlane(planes, height, stride)                                  \
                    do{                                                          \
                        size_t _height = 1;                                      \
                        size_t _stride = (size_t)(stride);                       \
                        planes[0] = { _height * _stride, _stride };              \
                        planes.push_back(bufPlane);                              \
                    }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_FG_BAYER10:
            addBufPlane(bufPlanes.planes , imgSize.h, stride);
            break;
        case eImgFmt_BLOB:
            addBlobBufPlane(bufPlanes.planes , imgSize.h, stride);
            break;
        default:
            MY_LOGE("format not support yet %p", imgFormat);
            break;
    }
#undef addBlobBufPlane
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
void finishPipelineContext()
{
    gContext->waitUntilDrained();
    gContext = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
 void prepareSensor()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    IHalLogicalDeviceList* pHalDeviceList = MAKE_HalLogicalDeviceList();
    pHalDeviceList->searchDevices();
    mpSensorHalObj = pHalSensorList->createSensor("tester", gSensorId);
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);

}

/******************************************************************************
 *
 ******************************************************************************/
void prepareConfiguration()
{
    {
        sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
        NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    }
    {
        ITemplateRequest* obj = NSTemplateRequestManager::valueFor(gSensorId);
        if(obj == NULL) {
            obj = ITemplateRequest::getInstance(gSensorId);
            NSTemplateRequestManager::add(gSensorId, obj);
        }
    }

}
/******************************************************************************
 *
 ******************************************************************************/
void closeSensor()
{
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
    mpSensorHalObj->destroyInstance("tester");
    mpSensorHalObj = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void
write_to_buffer(sp<HalImageStreamBuffer>& rpImageStreamBuffer, StreamId_T const streamId)
{
    //write main yuv to src buffer
    {
        sp<IImageBufferHeap>   pImageBufferHeap = NULL;
        IImageBuffer*                pImageBuffer = NULL;
        pImageBufferHeap = rpImageStreamBuffer->tryWriteLock(LOG_TAG);
        if (pImageBufferHeap == NULL) {
            MY_LOGE("pImageBufferHeap == NULL");
        }

        pImageBuffer = pImageBufferHeap->createImageBuffer();
        if (pImageBuffer == NULL) {
            MY_LOGE("pImageBuffer == NULL");
        }

        char *p = NULL;
        switch(streamId) {
            case STREAM_ID_IN_YUV_Main:
                {
                MY_LOGD("load data from /system/data/mainYUV_391_4096_3072_0.yuv");
                char *filename = "/system/data/mainYUV_391_4096_3072_0.yuv";
                p = filename;
                break;
                }
            case STREAM_ID_IN_YUV_Thumbnail:
                {
                MY_LOGD("load data from /system/data/thumbnailYUV_391_176_132_0.yuv");
                char *filename = "/system/data/thumbnailYUV_391_176_132_0.yuv";
                p = filename;
                break;
                }
            default:
                break;
            }

        printf("load image:%s\n", p);
        pImageBuffer->loadFromFile(p);
        printf("@@BufSize = %zu\n", pImageBuffer->getBufSizeInBytes(0));

        ////pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
        ////MY_LOGD("@@fist byte:%x", *(reinterpret_cast<MINT8*>(pImageBuffer->getBufVA(0))));
        ////pImageBuffer->unlockBuf(LOG_TAG);
        rpImageStreamBuffer->unlock(LOG_TAG, pImageBufferHeap.get());
        //saveToFile("/data/raw16_result.raw", STREAM_ID_IN);
        MY_LOGD("write_to_buffer--");
    }

}

/******************************************************************************
 *
 ******************************************************************************/
void setupPipelineContext()
{
    gContext = IPipelineContextManager::get()->create("test");
    if( !gContext.get() ) {
        MY_LOGE("cannot create context");
        return;
    }
    //
    gContext->beginConfigure();
    //
    // 1. Streams ***************
    //
    // 1.a. check if stream exist
    // 1.b. setup streams
    {
        // Meta
        StreamBuilder(eStreamType_META_APP, gControlMeta_App)
            .build(gContext);MY_LOGD("1");
        StreamBuilder(eStreamType_META_HAL, gControlMeta_Hal)
            .build(gContext);MY_LOGD("2");
        StreamBuilder(eStreamType_META_APP,  gResultMeta_Jpeg_App)
            .build(gContext);MY_LOGD("3");
        // Image
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_Yuv)
            .build(gContext);MY_LOGD("4");
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_Yuv_Thumbnail)
            .build(gContext);MY_LOGD("5");
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_Jpeg)
            .build(gContext);MY_LOGD("6");
    }
    printf("setup streams\n");
    // 2. Nodes   ***************
    //
    // 2.a. check if node exist
    // 2.b. setup nodes
    //
    {
        typedef JpegNode                  NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder Jpeg +");
        NodeT::InitParams initParam;
        {
            initParam.openId = gSensorId;
            initParam.nodeId = eNODEID_JpegNode;
            initParam.nodeName = "JpegNode";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta = gControlMeta_App;
            cfgParam.pInHalMeta = gControlMeta_Hal;
            cfgParam.pOutAppMeta = gResultMeta_Jpeg_App;

            cfgParam.pInYuv_Main = gImage_Yuv;
            cfgParam.pInYuv_Thumbnail = gImage_Yuv_Thumbnail;
            cfgParam.pOutJpeg =  gImage_Jpeg;

        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        MERROR ret =
            NodeBuilder(
                eNODEID_JpegNode,
                pNode
                )
            .addStream(
                    NodeBuilder::eDirection_IN,
                    StreamSet()
                    .add(STREAM_ID_APPREQUEST)
                    .add(STREAM_ID_HALREQUEST)
                    .add(STREAM_ID_IN_YUV_Main)
                    .add(STREAM_ID_IN_YUV_Thumbnail)
                    )
            .addStream(
                    NodeBuilder::eDirection_OUT,
                    StreamSet()
                    .add(STREAM_ID_APPMETADATA)
                    .add(STREAM_ID_OUT_JPEG)
                    )
            .setImageStreamUsage(
                    STREAM_ID_IN_YUV_Main,
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE
                    )
            .setImageStreamUsage(
                    STREAM_ID_IN_YUV_Thumbnail,
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE
                    )
            .setImageStreamUsage(
                    STREAM_ID_OUT_JPEG,
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_CAMERA_READWRITE
                    )
            .build(gContext);
        MY_LOGD("Nodebuilder Jpeg -");

        if( ret != OK ) {
            MY_LOGE("build Jpeg node error");
            return;
        }
    }

    // Test: query p1node from context & do sth.
    /*{
        sp< NodeActor<P1Node> > pNodeActor;
        if( NAME_NOT_FOUND == gContext->queryNodeActor(eNODEID_P1Node, pNodeActor) ) {
            MY_LOGE("cannot find p1node after build p1node");
        } else {
            typedef P1Node                  NodeT;
            NodeT::InitParams initParam;
            NodeT::ConfigParams cfgParam;
            pNodeActor->getInitParam(initParam);
            pNodeActor->getConfigParam(cfgParam);
            // check if param is correct
            {
                if( initParam.openId != gSensorId) MY_LOGE("wrong param");
                if( initParam.nodeId != eNODEID_P1Node) MY_LOGE("wrong param");
                //initParam.nodeName = "P1Node";
            }
            {
                if( cfgParam.pInAppMeta != gControlMeta_App) MY_LOGE("wrong param");
                if( cfgParam.pInHalMeta != gControlMeta_Hal) MY_LOGE("wrong param");
                if( cfgParam.pOutAppMeta != gResultMeta_P1_App) MY_LOGE("wrong param");
                if( cfgParam.pOutHalMeta != gResultMeta_P1_Hal) MY_LOGE("wrong param");
                if( cfgParam.pOutImage_resizer != gImage_RrzoRaw) MY_LOGE("wrong param");
                cfgParam.pvOutImage_full; //N/A
                //cfgParam.sensorParams = gSensorParam;
                if( cfgParam.pStreamPool_resizer != gContext->queryImageStreamPool(STREAM_ID_RAW1)) MY_LOGE("wrong param");
                cfgParam.pStreamPool_full;
            }
            //
            MY_LOGD("do manually init");
            if( OK != pNodeActor->init() )
                MY_LOGE("init failed");
            MY_LOGD("do manually config");
            if( OK != pNodeActor->config() )
                MY_LOGE("config failed");
        }
    }*/
    //
    // 3. Pipeline **************
    //
    {
        MERROR ret = PipelineBuilder()
            .setRootNode(
                    NodeSet().add(eNODEID_JpegNode)
                    )
            .build(gContext);
        if( ret != OK ) {
            MY_LOGE("build pipeline error");
            return;
        }
    }
    //
    gContext->endConfigure();
}

/******************************************************************************
 *
 ******************************************************************************/
IMetaStreamBuffer*
createMetaStreamBuffer(
    android::sp<IMetaStreamInfo> pStreamInfo,
    IMetadata const& rSettings,
    MBOOL const repeating
)
{
    HalMetaStreamBuffer*
    pStreamBuffer =
    HalMetaStreamBuffer::Allocator(pStreamInfo.get())(rSettings);
    //
    pStreamBuffer->setRepeating(repeating);
    //
    return pStreamBuffer;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
get_default_request()
{
    sp<IMetaStreamBuffer> pSBuffer;

    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(gSensorId);
    if(obj == NULL) {
        obj = ITemplateRequest::getInstance(gSensorId);
        NSTemplateRequestManager::add(gSensorId, obj);
    }
    IMetadata meta = obj->getMtkData(requestTemplate);
    //
    pSBuffer = createMetaStreamBuffer(gControlMeta_App, meta, false);
    //
    return pSBuffer;
}

/******************************************************************************
 *
 ******************************************************************************/
void
prepareRequest(android::sp<IPipelineFrame> &pFrame_)
{
    printf("prepare request+\n");

    static int cnt = 0;
    int current_cnt = cnt++;

    sp<IPipelineFrame> pFrame;
    //
    sp<IMetaStreamBuffer> pAppMetaControlSB = get_default_request();
    sp<HalMetaStreamBuffer> pHalMetaControlSB =
        HalMetaStreamBuffer::Allocator(gControlMeta_Hal.get())();

    /*{ // modify hal control metadata
        IMetadata* pMetadata = pHalMetaControlSB->tryWriteLock(LOG_TAG);
        {
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(gSensorParam.size, Type2Type< MSize >());
            pMetadata->update(entry.tag(), entry);
        }
        pHalMetaControlSB->unlock(LOG_TAG, pMetadata);
    }*/
    // write fake image to buffer
    {
        mpPool_HalImageYuvMain = gContext->queryImageStreamPool(STREAM_ID_IN_YUV_Main);
        mpPool_HalImageYuvThumbnail  = gContext->queryImageStreamPool(STREAM_ID_IN_YUV_Thumbnail);
    }
    sp<HalImageStreamBuffer> pHalImageSB_Main;
    sp<HalImageStreamBuffer> pHalImageSB_Thumb;
    MERROR err = mpPool_HalImageYuvMain->acquireFromPool(
                "Tester", pHalImageSB_Main, ::s2ns(30)
            );
    MY_LOGD("[acquireFromPool] - %s %p err:%d", gImage_Yuv->getStreamName(), pHalImageSB_Main.get(), err);
    MY_LOGE_IF(OK!=err || pHalImageSB_Main==0, "pHalImageSB_Main==0");
    write_to_buffer(pHalImageSB_Main, STREAM_ID_IN_YUV_Main);
    pHalImageSB_Main->releaseBuffer();
    err = mpPool_HalImageYuvThumbnail->acquireFromPool(
                "Tester", pHalImageSB_Thumb, ::s2ns(30)
            );
    write_to_buffer(pHalImageSB_Thumb, STREAM_ID_IN_YUV_Thumbnail);
    pHalImageSB_Thumb->releaseBuffer();
    {
            pFrame = RequestBuilder()
                .setIOMap(
                        eNODEID_JpegNode,
                        IOMapSet().add(
                            IOMap()
                            .addIn(STREAM_ID_IN_YUV_Main)
                            .addIn(STREAM_ID_IN_YUV_Thumbnail)
                            .addOut(STREAM_ID_OUT_JPEG)
                            ),
                        IOMapSet().add(
                            IOMap()
                            .addIn(STREAM_ID_APPREQUEST)
                            .addIn(STREAM_ID_HALREQUEST)
                            .addOut(STREAM_ID_APPMETADATA)
                            )
                        )
                .setRootNode(
                        NodeSet().add(eNODEID_JpegNode)
                        )
                .setMetaStreamBuffer(
                        STREAM_ID_APPREQUEST,
                        pAppMetaControlSB
                        )
                .setMetaStreamBuffer(
                        STREAM_ID_HALREQUEST,
                        pHalMetaControlSB
                        )
                ////not imp
                /*.setImageStreamBuffer(
                        STREAM_ID_IN_YUV_Main,
                        pHalImageSB_Main
                        )
                .setImageStreamBuffer(
                        STREAM_ID_IN_YUV_Thumbnail,
                        pHalImageSB_Thumb
                        )*/
                .build(current_cnt, gContext);
            if( ! pFrame.get() ) {
                MY_LOGE("build request failed");
            }
    }


    pFrame_ = pFrame;
    printf("prepare request-\n");
}


/******************************************************************************
 *
 ******************************************************************************/
void
saveToFile(const char *filename, StreamId_T const streamid)
{
    StreamId_T const streamId = streamid;

    sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);

    //acquireFromPool
    mpPool_HalImageJpeg = gContext->queryImageStreamPool(streamid);
    sp<HalImageStreamBuffer> pStreamBuffer;
    MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
    MERROR err = mpPool_HalImageJpeg->acquireFromPool(
                     "Tester", pStreamBuffer, ::s2ns(30)
                 );

    MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
    MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");

    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    IImageBuffer*          pImageBuffer = NULL;
    if (pStreamBuffer == NULL) {
        MY_LOGE("pStreamBuffer == NULL");
    }
    pImageBufferHeap = pStreamBuffer->tryReadLock(LOG_TAG);

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
    }

    pImageBuffer = pImageBufferHeap->createImageBuffer();

    if (pImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
    }
    pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
    MY_LOGD("@@@fist byte:%x", *(reinterpret_cast<MINT8*>(pImageBuffer->getBufVA(0))));

    pImageBuffer->saveToFile(filename);

    pImageBuffer->unlockBuf(LOG_TAG);

    pStreamBuffer->unlock(LOG_TAG, pImageBufferHeap.get());


}
/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    //const char *out_filename = "/data/jpegtest.jpg";
    //saveToFile(out_filename, STREAM_ID_OUT_JPEG);

    MY_LOGD("@@start test");

    //if (argc < 3) return 0;
    //sensor
    prepareSensor();

    prepareConfiguration();

    setupMetaStreamInfo();

    setupImageStreamInfo();

    setupPipelineContext();

    int c = 9;
    while( c++ < 10 )
    {
        android::sp<IPipelineFrame> pFrame;
        prepareRequest(pFrame);
        //usleep(100000);
        MY_LOGD("----------------");
        if( pFrame.get() )
        {
            if( OK != gContext->queue(pFrame) ) {
                MY_LOGE("queue pFrame failed");
            }
        }
        MY_LOGD("request count = %d", c);
        //saveToFile(out_filename, STREAM_ID_OUT_JPEG);
    };
    printf("ya!\n");

    finishPipelineContext();

    closeSensor();
    clear_global_var();

    MY_LOGD("end of test");

    return 0;
}

