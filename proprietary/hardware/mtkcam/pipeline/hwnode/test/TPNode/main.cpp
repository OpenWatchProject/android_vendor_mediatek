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
#define LOG_TAG "TPNodeTest"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/pipeline/pipeline/IPipelineDAG.h>
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
#include <pipeline/IPipelineNodeMapControl.h>
//
#include <mtkcam/pipeline/utils/streambuf/StreamBufferPool.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/IStreamInfoSetControl.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
#include <mtkcam/pipeline/hwnode/TPNode.h>
//[++]
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;

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
/******************************************************************************
 *
 ******************************************************************************/


typedef IPipelineBufferSetFrameControl          PipelineFrameControlT;

void help()
{
    printf("TP Node <test>\n");
}

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    enum STREAM_ID{
        STREAM_ID_IN_FULL_YUV = 1,
        STREAM_ID_IN_RSZ_YUV,
        STREAM_ID_IN_BIN_YUV,
        STREAM_ID_OUT_YUV_JPEG,
        STREAM_ID_OUT_YUV_THN,
        STREAM_ID_OUT_DEPTH,
        STREAM_ID_OUT_CLEAN,
        STREAM_ID_APP_REQUEST,
        STREAM_ID_HAL_REQUEST,
        STREAM_ID_APP_RESULT,
        STREAM_ID_HAL_RESULT
    };

    enum NODE_ID{
        NODE_ID_NODE1 = 1,
        NODE_ID_NODE2,
    };

    class AppSimulator
        : public virtual RefBase
    {
    };

    class NodeCallback: public virtual IPipelineNodeCallback
        , public virtual NSCam::v3::NSPipelineContext::DispatcherBase
    {
         MVOID                   onDispatchFrame(
                                    android::sp<IPipelineFrame> const& pFrame,
                                    Pipeline_NodeId_T
                                )
                                {
                                    printf("finished frame[%u]\n", pFrame->getFrameNo());
                                }

         MVOID                  onEarlyCallback(
                                    MUINT32,
                                    Pipeline_NodeId_T,
                                    StreamId_T,
                                    IMetadata const&,
                                    MBOOL
                                ) {}
    };
    //
    android::sp<AppSimulator>           mpAppSimulator;
    //
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_InFullYuv;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_InRszYuv;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_InBinYuv;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_OutYuvJpeg;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_OutYuvThn;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_OutDepth;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage_OutClean;
    //
    IHalSensor* mpSensorHalObj;
    //
    typedef NSCam::v3::Utils::IStreamInfoSetControl       IStreamInfoSetControlT;
    android::sp<IStreamInfoSetControlT>     mpStreamInfoSet;
    android::sp<IPipelineNodeMapControl>    mpPipelineNodeMap;
    android::sp<IPipelineNodeCallback>      mpNodeCallback;
    android::sp<IPipelineDAG>               mpPipelineDAG;
    android::sp<TPNode>                     mpNode1;
    //
    static int                              gFrameCounter = 0;
    static int                              gSensorId = 0;
    //
}; // namespace

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
        MY_LOGW("pMetadata == NULL");
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
    mpPipelineNodeMap = NULL;
    mpStreamInfoSet = NULL;
    mpPipelineDAG = NULL;

    mpPool_HalImage_InFullYuv->uninitPool("Tester");
    mpPool_HalImage_InFullYuv = NULL;
    mpPool_HalImage_InRszYuv->uninitPool("Tester");
    mpPool_HalImage_InRszYuv = NULL;
    mpPool_HalImage_InBinYuv->uninitPool("Tester");
    mpPool_HalImage_InBinYuv = NULL;
    mpPool_HalImage_OutYuvJpeg->uninitPool("Tester");
    mpPool_HalImage_OutYuvJpeg = NULL;
    mpPool_HalImage_OutYuvThn->uninitPool("Tester");
    mpPool_HalImage_OutYuvThn = NULL;
    mpPool_HalImage_OutDepth->uninitPool("Tester");
    mpPool_HalImage_OutDepth = NULL;
    mpPool_HalImage_OutClean->uninitPool("Tester");
    mpPool_HalImage_OutClean = NULL;
    mpAppSimulator = NULL;
    mpNodeCallback = NULL;
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
    MUINT32 sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);
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
void prepareConfig(TPNode::ConfigParams &params)
{
    printf("prepareConfig + \n");
    //
    mpStreamInfoSet = IStreamInfoSetControl::create();
    mpPipelineNodeMap = IPipelineNodeMapControl::create();
    mpPipelineDAG = IPipelineDAG::create();
    //
    mpNodeCallback = new NodeCallback();
    mpAppSimulator = new AppSimulator;
    //
    //android::Vector<android::sp<IImageStreamInfo> > pvHalImageYuv;
    //sp<IMetaStreamInfo>  pHalMetaPlatform = 0;
    sp<IMetaStreamInfo> pAppMetaResult   = 0;
    sp<IMetaStreamInfo> pHalMetaResult   = 0;
    sp<IMetaStreamInfo> pAppMetaRequest  = 0;
    sp<IMetaStreamInfo> pHalMetaRequest  = 0;
    sp<ImageStreamInfo> pHalImageInFullYuv  = 0;
    sp<ImageStreamInfo> pHalImageInRszYuv   = 0;
    sp<ImageStreamInfo> pHalImageInBinYuv   = 0;
    sp<ImageStreamInfo> pHalImageOutYuvJpeg = 0;
    sp<ImageStreamInfo> pHalImageOutYuvThn  = 0;
    sp<ImageStreamInfo> pHalImageOutDepth   = 0;
    sp<ImageStreamInfo> pHalImageOutClean   = 0;

    //Hal:Image:InFullYuv
    {
        StreamId_T const streamId = STREAM_ID_IN_FULL_YUV;
        MSize const imgSize(4608, 2592);//[++]
        MINT const format = eImgFmt_NV21;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE |
                            eBUFFER_USAGE_SW_WRITE_OFTEN
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:InFullYuv",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageInFullYuv = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_InFullYuv = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_InFullYuv->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_InFullYuv init fail");
        }
    }

    //Hal:Image:InRszYuv
    {
        StreamId_T const streamId = STREAM_ID_IN_RSZ_YUV;
        MSize const imgSize(2592, 1460);//[++]
        MINT const format = eImgFmt_NV21;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE |
                            eBUFFER_USAGE_SW_WRITE_OFTEN
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:InResizedYuv",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageInRszYuv = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_InRszYuv = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_InRszYuv->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_InRszYuv init fail");
        }
    }

    //Hal:Image:InBinYuv
    {
        StreamId_T const streamId = STREAM_ID_IN_BIN_YUV;
        MSize const imgSize(1296, 730);//[++]
        MINT const format = eImgFmt_NV21;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE |
                            eBUFFER_USAGE_SW_WRITE_OFTEN
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:InBinningYuv",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageInBinYuv = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_InBinYuv = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_InBinYuv->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_InBinYuv init fail");
        }
    }
    //Hal:Image:OutYuvJpeg
    {
        StreamId_T const streamId = STREAM_ID_OUT_YUV_JPEG;
        MSize const imgSize(4608, 2592);
        MINT const format = eImgFmt_NV21;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE |
                            eBUFFER_USAGE_SW_WRITE_OFTEN
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:OutYuvJpeg",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutYuvJpeg = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_OutYuvJpeg = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_OutYuvJpeg->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_OutYuvJpeg init fail");
        }
    }

    //Hal:Image:OutYuvThumbnail
    {
        StreamId_T const streamId = STREAM_ID_OUT_YUV_THN;
        MSize const imgSize(640, 480);
        MINT const format = eImgFmt_YV12;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE |
                            eBUFFER_USAGE_SW_WRITE_OFTEN
                            ;

        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:OutYuvThumbnail",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutYuvThn = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_OutYuvThn = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_OutYuvThn->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_OutYuvThn init fail");
        }
    }

    //Hal:Image:Depth
    {
        StreamId_T const streamId = STREAM_ID_OUT_DEPTH;
        MSize const imgSize(640, 480);
        MINT const format = eImgFmt_Y8;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_SW_WRITE_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE
                            ;

        IImageStreamInfo::BufPlanes_t bufPlanes;
        IImageStreamInfo::BufPlane bufPlane= { (unsigned int)(imgSize.h * imgSize.w), (unsigned int)imgSize.w };
        bufPlanes.push_back(bufPlane);
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:Depth",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutDepth = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_OutDepth = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_OutDepth->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_OutDepth init fail");
        }
    }
    //Hal:Image:Clean
    {
        StreamId_T const streamId = STREAM_ID_OUT_CLEAN;
        MSize const imgSize(4608, 2592);
        MINT const format = eImgFmt_NV21;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_SW_WRITE_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:OutClean",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutClean = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImage_OutClean = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage_OutClean->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage_OutClean init fail");
        }
    }
    //App:Meta:TP Out
    printf("create TP out app metadata\n");
    {
        sp<IMetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App:Meta:TP:Dynamic",
            STREAM_ID_APP_RESULT,
            eSTREAMTYPE_META_OUT,
            5, 1
        );
        pAppMetaResult = pStreamInfo;
    }

    //App:Meta:TP Out
    printf("create TP out app metadata\n");
    {
        sp<IMetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "Hal:Meta:TP:Dynamic",
            STREAM_ID_HAL_RESULT,
            eSTREAMTYPE_META_OUT,
            5, 1
        );
        pHalMetaResult = pStreamInfo;
    }

    //App:Meta:TP In
    printf("create TP in app metadata\n");
    {
        sp<MetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App:Meta:TP:Control",
            STREAM_ID_APP_REQUEST,
            eSTREAMTYPE_META_IN,
            5, 5
        );
        pAppMetaRequest = pStreamInfo;
    }

    //Hal:Meta:TP In
    printf("create TP in app metadata\n");
    {
        sp<MetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "Hal:Meta:TP:Control",
            STREAM_ID_HAL_REQUEST,
            eSTREAMTYPE_META_IN,
            5, 1
        );
        pHalMetaRequest = pStreamInfo;
    }

    params.pInAppMeta       = pAppMetaRequest;
    params.pInHalMeta       = pHalMetaRequest;
    params.pOutAppMeta      = pAppMetaResult;
    params.pOutHalMeta      = pHalMetaResult;
    params.pInFullYuv       = pHalImageInFullYuv;
    params.pInResizedYuv    = pHalImageInRszYuv;
    params.pInBinningYuv    = pHalImageInBinYuv;
    params.pOutYuvJpeg      = pHalImageOutYuvJpeg;
    params.pOutYuvThumbnail = pHalImageOutYuvThn;
    params.pOutYuv00        = NULL;
    params.pOutYuv01        = NULL;
    params.pOutDepth        = pHalImageOutDepth;
    params.pOutClean        = pHalImageOutClean;

    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    printf("add stream to streamInfoset\n");

    mpStreamInfoSet->editHalImage().addStream(pHalImageInFullYuv);
    mpStreamInfoSet->editHalImage().addStream(pHalImageInRszYuv);
    mpStreamInfoSet->editHalImage().addStream(pHalImageInBinYuv);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutYuvJpeg);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutYuvThn);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutDepth);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutClean);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaResult);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaRequest);
    mpStreamInfoSet->editHalMeta().addStream(pHalMetaResult);
    mpStreamInfoSet->editHalMeta().addStream(pHalMetaRequest);

    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    //
    //
    printf("add stream to pipelineNodeMap\n");
    mpPipelineNodeMap->setCapacity(1);
    //
    {
        mpPipelineDAG->addNode(NODE_ID_NODE1);
        ssize_t const tmpNodeIndex = mpPipelineNodeMap->add(NODE_ID_NODE1, mpNode1);
        //
        sp<IStreamInfoSetControl> const&
        rpInpStreams = mpPipelineNodeMap->getNodeAt(tmpNodeIndex)->editInStreams();
        sp<IStreamInfoSetControl> const&
        rpOutStreams = mpPipelineNodeMap->getNodeAt(tmpNodeIndex)->editOutStreams();
        //
        //  [input]
        //      App:Meta:Request
        //      Hal:Image:TP
        //  [output]
        //      App:Meta:Result
        //
        rpInpStreams->editAppMeta().addStream(pAppMetaRequest);
        rpInpStreams->editHalMeta().addStream(pHalMetaRequest);
        rpInpStreams->editHalImage().addStream(pHalImageInFullYuv);
        rpInpStreams->editHalImage().addStream(pHalImageInRszYuv);
        rpInpStreams->editHalImage().addStream(pHalImageInBinYuv);
        //
        rpOutStreams->editAppMeta().addStream(pAppMetaResult);
        rpOutStreams->editHalMeta().addStream(pHalMetaResult);
        rpOutStreams->editHalImage().addStream(pHalImageOutYuvJpeg);
        rpOutStreams->editHalImage().addStream(pHalImageOutYuvThn);
        rpOutStreams->editHalImage().addStream(pHalImageOutDepth);
        rpOutStreams->editHalImage().addStream(pHalImageOutClean);
        //
        // TODO: re-config flow
        //TPNode::ConfigParams cfgParams;
        //mpNode1->config(cfgParams);
    }
    //

    mpPipelineDAG->setRootNode(NodeSet().add(mpNode1->getNodeId()));
    for (size_t i = 0; i < mpPipelineNodeMap->size(); i++)
    {
        mpPipelineDAG->setNodeValue(mpPipelineNodeMap->nodeAt(i)->getNodeId(), i);
    }

    printf("prepareConfig - \n");
}
//************************************************************************
//
//************************************************************************
IImageBuffer*
loadImgFromFile(
    const IImageBufferAllocator::ImgParam imgParam,
    const char* path,
    const char* name,
    MINT usage)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->loadFromFile(path);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}
/******************************************************************************
 *
 ******************************************************************************/
void
prepareRequest(android::sp<PipelineFrameControlT> &pFrame, MUINT32 uRequestNo, MBOOL bDual, MBOOL bCloser)
{
    printf("prepare frame[%u]\n", gFrameCounter);
    MY_LOGD("prepare frame[%u]\n", gFrameCounter);

    pFrame = PipelineFrameControlT::create(uRequestNo, gFrameCounter++, NULL, NULL, mpNodeCallback);
    pFrame->setPipelineNodeMap(getPipelineNodeMap());
    pFrame->setPipelineDAG(getPipelineDAG());
    pFrame->setStreamInfoSet(getStreamInfoSet());

    //TP Node
    {
        IPipelineNode::NodeId_T const nodeId = NODE_ID_NODE1;
        // Pipeline Node Map
        sp<IPipelineNodeMapControl::INode> pPipeNode = getPipelineNodeMapControl()->getNodeFor(nodeId);
        sp<IStreamInfoSet const> pPipeIStream = pPipeNode->getInStreams();
        sp<IStreamInfoSet const> pPipeOStream= pPipeNode->getOutStreams();
        // Frame Node Map
        sp<IPipelineFrameNodeMapControl> pFrameNodeMap = IPipelineFrameNodeMapControl::create();
        pFrameNodeMap->addNode(nodeId);

        sp<IPipelineFrameNodeMapControl::INode> pFrameNode = pFrameNodeMap->getNodeFor(nodeId);
        IPipelineFrame::InfoIOMapSet& rInfoIOMapSet = pFrameNode->editInfoIOMapSet();
        IPipelineFrame::ImageInfoIOMapSet& rImageInfoIOMapSet = rInfoIOMapSet.mImageInfoIOMapSet;
        IPipelineFrame::MetaInfoIOMapSet&  rMetaInfoIOMapSet  = rInfoIOMapSet.mMetaInfoIOMapSet;

        typedef NSCam::v3::Utils::SimpleStreamInfoSetControl StreamInfoSetT;
        sp<StreamInfoSetT> pFrameIStreams = new StreamInfoSetT;
        sp<StreamInfoSetT> pFrameOStreams = new StreamInfoSetT;
        pFrameNode->setIStreams(pFrameIStreams);
        pFrameNode->setOStreams(pFrameOStreams);

        //
        //
        //  Image
        {
            IPipelineFrame::ImageInfoIOMap& rMap =
            rImageInfoIOMapSet.editItemAt(rImageInfoIOMapSet.add());
            //
            //Input
            for (size_t i = 0; i < pPipeIStream->getImageInfoNum(); i++)
            {
                sp<IImageStreamInfo> p = pPipeIStream->getImageInfoAt(i);
                 if (bDual || (p->getStreamId() != STREAM_ID_IN_RSZ_YUV && p->getStreamId() != STREAM_ID_IN_BIN_YUV))
                {
                    rMap.vIn.add(p->getStreamId(), p);
                    pFrameIStreams->editImage().add(p->getStreamId(), p);
                }

            }
            //
            //Output
            if (bCloser)
            {
                for (size_t i = 0; i < pPipeOStream->getImageInfoNum(); i++)
                {
                    sp<IImageStreamInfo> p = pPipeOStream->getImageInfoAt(i);
                    if (bDual ||
                        !(p->getStreamId() == STREAM_ID_OUT_DEPTH ||
                          p->getStreamId() == STREAM_ID_OUT_CLEAN))
                    {
                        rMap.vOut.add(p->getStreamId(), p);
                        pFrameOStreams->editImage().add(p->getStreamId(), p);
                    }
                }
            }
        }
        //
        //  Meta
        {
            IPipelineFrame::MetaInfoIOMap& rMap =
            rMetaInfoIOMapSet.editItemAt(rMetaInfoIOMapSet.add());
            //
            //Input
            for (size_t i = 0; i < pPipeIStream->getMetaInfoNum(); i++)
            {
                sp<IMetaStreamInfo> p = pPipeIStream->getMetaInfoAt(i);
                rMap.vIn.add(p->getStreamId(), p);
                pFrameIStreams->editMeta().add(p->getStreamId(), p);
            }
            //
            //Output
            if (bCloser)
            {
                for (size_t i = 0; i < pPipeOStream->getMetaInfoNum(); i++)
                {
                    sp<IMetaStreamInfo> p = pPipeOStream->getMetaInfoAt(i);
                    rMap.vOut.add(p->getStreamId(), p);
                    pFrameOStreams->editMeta().add(p->getStreamId(), p);
                }
            }
        }
        //
        // must set the node map after the map was configured
        pFrame->setNodeMap(pFrameNodeMap);
    }


    ////////////////////////////////////////////////////////////////////////////
    //
    //  pFrame->setStreamBufferSet(...);
    //
    ////////////////////////////////////////////////////////////////////////////

    //IAppPipeline::AppCallbackParams aAppCallbackParams;
    //aAppCallbackParams.mpBuffersCallback = pAppSimulator;
    //
    //
    {
        //
        StreamId_T const streamId = STREAM_ID_IN_FULL_YUV;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_InFullYuv->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");

        // ======== UT for 3rd party vendor - start ========
        MY_LOGD("read full yuv image start");
        // get IImageBuffer
        IImageBufferHeap*            mpImageBufferHeap;
        IImageBuffer*                fullyuv_buf;
        IImageBuffer*                tmpbuf;
        mpImageBufferHeap = pStreamBuffer->tryWriteLock("TPTester");
        fullyuv_buf = mpImageBufferHeap->createImageBuffer();
        fullyuv_buf->lockBuf("TPTester", eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);

        MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
        MUINT32 MAIN_IMT_STRIDES_fullyuv[3];
        if(bCloser){
            MAIN_IMT_STRIDES_fullyuv[0] = 2592;
            MAIN_IMT_STRIDES_fullyuv[1] = 2592;
            MAIN_IMT_STRIDES_fullyuv[2] = 0;
        }
        else{
            MAIN_IMT_STRIDES_fullyuv[0] = 4608;
            MAIN_IMT_STRIDES_fullyuv[1] = 4608;
            MAIN_IMT_STRIDES_fullyuv[2] = 0;
        }
        const IImageBufferAllocator::ImgParam imgParam_MainImg(
                            eImgFmt_NV21, (bCloser) ? MSize(2592, 1460) : MSize(4608, 2592), MAIN_IMT_STRIDES_fullyuv, FULL_RAW_BOUNDARY, 3);
        #define LOAD_MAIN_IMAGE(IMAGEBUFFER, NAME, PATH)\
        do{\
            IMAGEBUFFER = loadImgFromFile(\
                            imgParam_MainImg,\
                            PATH,\
                            NAME,\
                            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);\
        }while(0)
        if(bCloser){
            // main2
            LOAD_MAIN_IMAGE(tmpbuf, "FullYUV", "/sdcard/vsdof/tpnode_ut/data/Full_YUV_1__2592_1460.yuv");
        }
        else{
            // main1
            LOAD_MAIN_IMAGE(tmpbuf, "FullYUV", "/sdcard/vsdof/tpnode_ut/data/Full_YUV_0__4608_2592.yuv");
        }
        for (size_t i = 0; i < tmpbuf->getPlaneCount(); i++) {
            MUINT32 uBufSize = tmpbuf->getBufSizeInBytes(i);
            //MY_LOGD("1");
            void *pInVa = (void *) (tmpbuf->getBufVA(i));
            //MY_LOGD("2");
            void *pOutVa = (void *) (fullyuv_buf->getBufVA(i));
            //MY_LOGD("3");
            memcpy(pOutVa, pInVa, uBufSize);
        }
        #undef LOAD_MAIN_IMAGE
        tmpbuf->unlockBuf("FullYUV");
        fullyuv_buf->unlockBuf("TPTester");
        pStreamBuffer->unlock("TPTester", mpImageBufferHeap);
        MY_LOGD("read full yuv image end");
        // ======== UT for 3rd party vendor - end ========

        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    if (bDual)
    {
        //
        StreamId_T const streamId = STREAM_ID_IN_RSZ_YUV;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_InRszYuv->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");

        // ======== UT for 3rd party vendor - start ========
        MY_LOGD("read resize yuv image start");
        // get IImageBuffer
        IImageBufferHeap*            mpImageBufferHeap;
        IImageBuffer*                resizeyuv_buf;
        IImageBuffer*                tmpbuf;
        mpImageBufferHeap = pStreamBuffer->tryWriteLock("TPTester");
        resizeyuv_buf = mpImageBufferHeap->createImageBuffer();
        resizeyuv_buf->lockBuf("TPTester", eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);

        MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
        MUINT32 MAIN_IMT_STRIDES_resizeyuv[3];
        // if(bCloser){
            MAIN_IMT_STRIDES_resizeyuv[0] = 2592;
            MAIN_IMT_STRIDES_resizeyuv[1] = 2592;
            MAIN_IMT_STRIDES_resizeyuv[2] = 0;
        // }
        // else{
        //     MAIN_IMT_STRIDES_resizeyuv[0] = 4608;
        //     MAIN_IMT_STRIDES_resizeyuv[1] = 4608;
        //     MAIN_IMT_STRIDES_resizeyuv[2] = 0;
        // }
        const IImageBufferAllocator::ImgParam imgParam_MainImg(
            eImgFmt_NV21, MSize(2592, 1460), MAIN_IMT_STRIDES_resizeyuv, FULL_RAW_BOUNDARY, 3);

        #define LOAD_MAIN_IMAGE(IMAGEBUFFER, NAME, PATH)\
        do{\
            IMAGEBUFFER = loadImgFromFile(\
                            imgParam_MainImg,\
                            PATH,\
                            NAME,\
                            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);\
        }while(0)
        if(bCloser){
            // main2
            LOAD_MAIN_IMAGE(tmpbuf, "ResizeYUV", "/sdcard/vsdof/tpnode_ut/data/Full_YUV_1__2592_1460.yuv");
        }
        else{
            // main1
            LOAD_MAIN_IMAGE(tmpbuf, "ResizeYUV", "/sdcard/vsdof/tpnode_ut/data/Full_YUV_0__4608_2592.yuv");
        }
        #undef LOAD_MAIN_IMAGE
        for (size_t i = 0; i < tmpbuf->getPlaneCount(); i++) {
            MUINT32 uBufSize = tmpbuf->getBufSizeInBytes(i);
            //MY_LOGD("1");
            void *pInVa = (void *) (tmpbuf->getBufVA(i));
            //MY_LOGD("2");
            void *pOutVa = (void *) (resizeyuv_buf->getBufVA(i));
            //MY_LOGD("3");
            memcpy(pOutVa, pInVa, uBufSize);
        }
        tmpbuf->unlockBuf("ResizeYUV");
        resizeyuv_buf->unlockBuf("TPTester");
        pStreamBuffer->unlock("TPTester", mpImageBufferHeap);
        MY_LOGD("read resize yuv image end");
        // ======== UT for 3rd party vendor - end ========

        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    if (bDual)
    {
        //
        StreamId_T const streamId = STREAM_ID_IN_BIN_YUV;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_InBinYuv->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    if (bCloser) {
        //
        StreamId_T const streamId = STREAM_ID_OUT_YUV_JPEG;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_OutYuvJpeg->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    if (bCloser) {
        //
        StreamId_T const streamId = STREAM_ID_OUT_YUV_THN;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_OutYuvThn->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    // Depth
    if (bDual && bCloser)
    {
        //
        StreamId_T const streamId = STREAM_ID_OUT_DEPTH;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_OutDepth->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    // Clean
    if (bDual && bCloser)
    {
        //
        StreamId_T const streamId = STREAM_ID_OUT_CLEAN;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage_OutClean->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    if (bCloser)
    {
        //App:Meta:Result
        StreamId_T const streamId = STREAM_ID_APP_RESULT;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        //alloc without default value
        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())();
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            pUserGraph->addUser(user);
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    if (bCloser)
    {
        //App:Meta:Result
        StreamId_T const streamId = STREAM_ID_HAL_RESULT;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        //alloc without default value
        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())();
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            pUserGraph->addUser(user);
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //TODO: check the target to edit()
        pFrame->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    {
        //APP/Meta/Request
        StreamId_T const streamId = STREAM_ID_APP_REQUEST;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        IMetadata appRequest;
        {
            IMetadata::IEntry entry1(MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
            entry1.push_back(16, Type2Type< MFLOAT >());
            appRequest.update(MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, entry1);

            // bokeh level
            IMetadata::IEntry entry2(MTK_STEREO_FEATURE_DOF_LEVEL);
            entry2.push_back(15, Type2Type<MINT32>());
            appRequest.update(MTK_STEREO_FEATURE_DOF_LEVEL, entry2);

            // AF state
            IMetadata::IEntry entry3(MTK_CONTROL_AF_STATE);
            entry3.push_back(MTK_CONTROL_AF_STATE_FOCUSED_LOCKED, Type2Type<MINT32>());
            appRequest.update(MTK_CONTROL_AF_STATE, entry3);

            // AF ROI
            if (bCloser){
                // main2
                IMetadata::IEntry entry4(MTK_3A_FEATURE_AF_ROI);
                entry4.push_back(2565, Type2Type< MINT32 >());
                entry4.push_back(1433, Type2Type< MINT32 >());
                entry4.push_back(2619, Type2Type< MINT32 >());
                entry4.push_back(1487, Type2Type< MINT32 >());
                appRequest.update(MTK_3A_FEATURE_AF_ROI, entry4);
            }
            else{
                // main1
                IMetadata::IEntry entry4(MTK_3A_FEATURE_AF_ROI);
                entry4.push_back(2256, Type2Type< MINT32 >());
                entry4.push_back(1248, Type2Type< MINT32 >());
                entry4.push_back(2352, Type2Type< MINT32 >());
                entry4.push_back(1344, Type2Type< MINT32 >());
                appRequest.update(MTK_3A_FEATURE_AF_ROI, entry4);
            }
        }

        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())(appRequest);
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            pUserGraph->addUser(user);
            //
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    {
        //HAL/Meta/Request
        StreamId_T const streamId = STREAM_ID_HAL_REQUEST;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        IMetadata halRequest;
        {
            // plugin code
            IMetadata::IEntry entry1(MTK_PLUGIN_MODE);
            if (bDual)
                entry1.push_back(MTK_PLUGIN_MODE_DCMF_3RD_PARTY, Type2Type<MINT32>());
            else
                entry1.push_back(MTK_PLUGIN_MODE_HDR_3RD_PARTY, Type2Type<MINT32>());
            halRequest.update(MTK_PLUGIN_MODE, entry1);
        }
        //
        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())(halRequest);
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            pUserGraph->addUser(user);
            //
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
            pStreamBuffer->finishUserSetup();
        }
        //
        pFrame->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    pFrame->finishConfiguration();
}


/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    MY_LOGD("start test");

    mpNode1 = TPNode::createInstance();
    //
    MUINT32 frameNo = 0;
    //
    //sensor
    prepareSensor();
    //[++]static metadata
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());

    //init
    {
         struct  IPipelineNode::InitParams params;
         params.openId = gSensorId;
         params.nodeName = "TPTester";
         params.nodeId = NODE_ID_NODE1;
         mpNode1->init(params);
    };


    //config
    {
       TPNode::ConfigParams params;
       prepareConfig(params);
       mpNode1->config(params);
    }

    //----------test 1 ------------//
    // {
    //     printf("Test 1: start fixed pattern\n");
    //     android::sp<PipelineFrameControlT> pFrame;
    //     size_t round = (argc > 1) ? atoi(argv[1]) : 2;
    //     size_t pack = (argc > 2) ? atoi(argv[2]) : 4;

    //     for (size_t i = 0 ; i < round; i++) {
    //         for (size_t j = 0; j < pack; j++) {
    //             prepareRequest(pFrame, i, false ,pack - 1 == j);
    //             mpNode1->queue(pFrame);
    //         }
    //     }

    //     usleep(100000);
    //     mpNode1->flush();
    //     printf("Test 1: end\n");
    //     MY_LOGD("Test 1: end\n");
    // }

    //----------test 2 ------------//
    // {
    //     printf("Test 2: start broken pattern\n");
    //     android::sp<PipelineFrameControlT> pFrame;
    //     size_t round = 2;
    //     size_t pack = 4;

    //     for (size_t i = 0 ; i < round; i++) {
    //         for (size_t j = 0; j < pack; j++) {
    //             prepareRequest(pFrame, i, false, i == 0 && pack - 1 == j);
    //             mpNode1->queue(pFrame);
    //         }
    //     }

    //     printf("Test 2: flush +\n");
    //     mpNode1->flush();
    //     printf("Test 2: flush -\n");
    //     printf("Test 2: end------------------------------------------------------\n");
    //     MY_LOGD("Test 2: end------------------------------------------------------\n");
    // }

    //----------test 3 ------------//
    {
        printf("Test 3: start daulcam pattern\n");
        MY_LOGD("Test 3: start daulcam pattern\n");
        android::sp<PipelineFrameControlT> pFrame;
        size_t round = 1;
        size_t pack = 2;

        for (size_t i = 0 ; i < round; i++) {
            for (size_t j = 0; j < pack; j++) {
                prepareRequest(pFrame, i, true ,pack - 1 == j);
                mpNode1->queue(pFrame);
            }
        }

        usleep(1000000);
        mpNode1->flush();
        printf("Test 3: end\n");
        MY_LOGD("Test 3: end===========================================\n");
    }

    //uninit
    printf("uninit\n");
    mpNode1->uninit();

    closeSensor();

    clear_global_var();

    MY_LOGD("end of test");
    return 0;
}
