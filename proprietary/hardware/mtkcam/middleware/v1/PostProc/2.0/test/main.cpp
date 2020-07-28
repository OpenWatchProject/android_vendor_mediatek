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
#define LOG_TAG "PostProcTest"
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
#include <mtkcam/pipeline/pipeline/IPipelineDAG.h>
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>

#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/pipeline/utils/streambuf/StreamBufferPool.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/pipeline/hwnode/NodeId.h>
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <hardware/camera3.h> // for template
//
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>

#include <mtkcam/middleware/v1/PostProc/IPostProc.h>

using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;
using namespace NSCam::v3::NSPipelineContext;

using namespace android::NSPostProc;


/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#endif
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+\n");
#define FUNCTION_OUT    MY_LOGD_IF(1, "-\n")
typedef enum
{
  eTEST_CASE_DUMMY,
  eTEST_CASE_BOKEH,
  eTEST_CASE_THIRDPARTY,
  eTEST_CASE_MAX,
} eTestCase ;

#define MAX_TEST_REQ_COUNT  10
#define MAX_TEST_BUFFER_LIST 3

static int gSensorId = 0;
IHalSensor* mpSensorHalObj = NULL;
static int gSensorId2 = 2;
IHalSensor* mpSensorHalObj2 = NULL;

IMetadata gAppMetadata[MAX_TEST_REQ_COUNT][MAX_TEST_BUFFER_LIST];
IMetadata gHalMetadata[MAX_TEST_REQ_COUNT][MAX_TEST_BUFFER_LIST];
MUINT32 width  = 1920;
MUINT32 height = 1080;
MSize const& finalJpegSize = MSize(width,height);
MSize const& thumbnailSize = MSize(160,128);
MRect const& rect = MRect(width, height);

NSShot::ShotParam gShotParam(
                                eImgFmt_YUY2,                     //yuv format
                                width,                            //picutre width
                                height,                           //picture height
                                eImgFmt_YV12,                     //PostviewDisplay format
                                eImgFmt_YV12,                     //PostviewClient format
                                width/2,                          //postview width
                                height/2,                         //postview height
                                eImgFmt_YV12                     //PostviewThumb format
                                );
NSShot::JpegParam gJpegParam(100, 100, 160, 128);

sp<IImageStreamInfo>        gImage_Yuv;

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
static void help(char *pProg)
{
    printf("PostProc test Usage:\n");
    printf("\t\t%s [testCase]\n\n", pProg);
    printf("\t\t testCase:(MUST)\n");
    printf("\t\t\t number of test case\n");
    printf("\t\t\t 0: DummyPostProcessor\n");
    printf("\t\t\t 1: BokehPostProcessor\n");
    printf("\t\t\t 2: ThirdPartyPostProcessor\n");
}

template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL\n");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

template <typename T>
inline MBOOL
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL\n");
        return MFALSE;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
class CTestMW : public IPostProcRequestCB
{
public:
    CTestMW(){};
   ~CTestMW(){};
    virtual status_t onPostProcEvent(
            MINT32  requestNo,
            PostProcRequestCB callbackType,
            MUINT32 streamId,
            MBOOL bError,
            void* params1 = nullptr,
            void* params2 = nullptr);
};

status_t
CTestMW::
onPostProcEvent(
            MINT32  requestNo,
            PostProcRequestCB callbackType,
            MUINT32 /* streamId */,
            MBOOL   /* bError */,
            void*   /* params1 */,
            void*   /* params2 */
)
{
  MY_LOGD("requestNo: %d, callbackType %d\n", requestNo, callbackType);
  return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void prepareStreamInfo()
{
    MSize const& size = MSize(640,480);
    MINT const format = eImgFmt_NV21;
    MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
    gImage_Yuv = createImageStreamInfo(
            "Hal:Image:yuv",
            eSTREAMID_IMAGE_PIPE_YUV_00,
            eSTREAMTYPE_IMAGE_INOUT,
            1, 1,
            usage, format, size
            );
}
/******************************************************************************
 *
 ******************************************************************************/
int prepareImageBuffer(sp<ImagePostProcessData> pData)
{
    FUNCTION_IN
    sp<IImageBufferHeap> pImageBufferHeap[MAX_TEST_BUFFER_LIST];
    sp<IImageBuffer>     pImageBuffer[MAX_TEST_BUFFER_LIST];
    IImageStreamInfo::BufPlanes_t const& bufPlanes = gImage_Yuv->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    for (size_t i = 0; i < bufPlanes.size(); i++)
    {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
    }
    IImageBufferAllocator::ImgParam imgParam(
                                                gImage_Yuv->getImgFormat(),
                                                gImage_Yuv->getImgSize(),
                                                bufStridesInBytes, bufBoundaryInBytes,
                                                bufPlanes.size()
                                            );
    MY_LOGD("format:%x, size:(%d,%d), stride:%zu, boundary:%zu, planes:%zu\n",
            gImage_Yuv->getImgFormat(), gImage_Yuv->getImgSize().w, gImage_Yuv->getImgSize().h,
            bufStridesInBytes[0], bufBoundaryInBytes[0], bufPlanes.size());



    for (int j = 0 ; j < MAX_TEST_BUFFER_LIST ; j++)
    {
        android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> vImage;
        pImageBufferHeap[j] = IIonImageBufferHeap::create(
                                gImage_Yuv->getStreamName(),
                                imgParam,
                                IIonImageBufferHeap::AllocExtraParam(),
                                MFALSE
                            );
        if ( pImageBufferHeap[j].get() )
        {
            pImageBuffer[j] = pImageBufferHeap[j]->createImageBuffer();
            MY_LOGD("pImageBuffer[%d]: %p\n", j, pImageBuffer[j].get());
        }
        else
        {
            MY_LOGE("fail to allocate imageBuffer\n");
        }
        vImage.add(eSTREAMID_IMAGE_PIPE_YUV_00, pImageBuffer[j]);
        pData->mImageQueue.push_back(vImage); // push image buffers for 3 pictures
    }
    FUNCTION_OUT;
    return MAX_TEST_REQ_COUNT;
}
/******************************************************************************
 *
 ******************************************************************************/
int prepareMetadata(sp<ImagePostProcessData> pData, int reqIndx)
{
    FUNCTION_IN;
    for (int j = 0 ; j < MAX_TEST_BUFFER_LIST ; j++)
    {
        android::DefaultKeyedVector<MINT32, IMetadata> vMeta;
        trySetMetadata<MINT32>(&gAppMetadata[reqIndx][j], MTK_JPEG_ORIENTATION, 0);
        trySetMetadata<MUINT8>(&gAppMetadata[reqIndx][j], MTK_JPEG_QUALITY, 100);
        trySetMetadata<MUINT8>(&gAppMetadata[reqIndx][j], MTK_JPEG_THUMBNAIL_QUALITY, 100);
        trySetMetadata<MSize> (&gAppMetadata[reqIndx][j], MTK_JPEG_THUMBNAIL_SIZE, thumbnailSize);
        trySetMetadata<MRect> (&gAppMetadata[reqIndx][j], MTK_SCALER_CROP_REGION, rect);
        // PlugIn mode
        if (pData->mProcessType == PostProcessorType::THIRDPARTY_MFNR)
        {
            trySetMetadata<MINT32>(&gHalMetadata[reqIndx][j], MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_MFNR_3RD_PARTY);
        }
        else if (pData->mProcessType == PostProcessorType::THIRDPARTY_HDR)
        {
            trySetMetadata<MINT32>(&gHalMetadata[reqIndx][j], MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_HDR_3RD_PARTY);
        }
        else if (pData->mProcessType == PostProcessorType::THIRDPARTY_BOKEH)
        {
            trySetMetadata<MINT32>(&gHalMetadata[reqIndx][j], MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_BOKEH_3RD_PARTY);
        }
        pData->mAppMetadataQueue.push_back(gAppMetadata[reqIndx][j]);
        pData->mHalMetadataQueue.push_back(gHalMetadata[reqIndx][j]);
    }
    FUNCTION_OUT;
    return MAX_TEST_REQ_COUNT;
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
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
            break;
        case eImgFmt_BLOB:
            /*
            add 328448 for image size
            standard exif: 1280 bytes
            4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
            max thumbnail size: 64K bytes
            */
            addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
            break;
        default:
            MY_LOGE("format not support yet 0x%x \n", imgFormat);
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
        MY_LOGE("create ImageStream failed, %s, %ld \n",
                streamName, streamId);
    }

    return pStreamInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
void prepareSensor()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();

    mpSensorHalObj = pHalSensorList->createSensor("tester", gSensorId);
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);

    mpSensorHalObj2 = pHalSensorList->createSensor("tester2", gSensorId2);
    sensorArray[0] = {(MUINT32)gSensorId2};
    mpSensorHalObj2->powerOn("tester2", 1, &sensorArray[0]);
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

    sensorArray[0] = {(MUINT32)gSensorId2};
    mpSensorHalObj2->powerOff("tester2", 1, &sensorArray[0]);
    mpSensorHalObj2->destroyInstance("tester2");
    mpSensorHalObj2 = NULL;

}
/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    MY_LOGD("[PostProc] start to test\n");
    int testCase = 0;
    int reqNO = 0;
    int timestamp = 0;
    if (argc < 2)
    {
        help(argv[0]);
        return -1;
    }
    if (argc >= 2)
    {
        testCase = atoi(argv[1]);
        MY_LOGD("[PostProc] set testCase: %d\n", testCase);
        if ( testCase >= eTEST_CASE_MAX)
        {
          MY_LOGD("[PostProc] testCase is too large\n");
          return -1;
        }
    }

    MY_LOGD("[PostProc] :testCase(%d) \n", testCase);

    //sensor
    prepareSensor();
    //[++]static metadata
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    sp<IMetadataProvider> pMetadataProvider2 = IMetadataProvider::create(gSensorId2);
    NSMetadataProviderManager::add(gSensorId2, pMetadataProvider2.get());


    sp<IPostProcRequestCB>   pMW   = new CTestMW();

    // prepare streaminfo
    prepareStreamInfo();

    for (int i = 0; i < MAX_TEST_REQ_COUNT ; i++)
    {
        sp<ImagePostProcessData> pData = new ImagePostProcessData();

        pData->mRequestNo  = reqNO++;
        pData->miTimeStamp = timestamp++;

        if (testCase == eTEST_CASE_DUMMY)
        {
            pData->mProcessType = PostProcessorType::DUMMY;
        }
        else if (testCase == eTEST_CASE_THIRDPARTY)
        {
            pData->mProcessType = PostProcessorType::THIRDPARTY_HDR;
        }
        //prepare Images
        prepareImageBuffer(pData);
        //prepare MetaData
        prepareMetadata(pData, i);

        // prepare ShotParam, JpegParam
        pData->mShotParam =gShotParam;
        pData->mJpegParam =gJpegParam;

        pData->mpCb = pMW;
        MY_LOGD("[PostProc] enque %d, %d, pMW %p\n", pData->mRequestNo, (int) pData->miTimeStamp, pMW.get());
        sleep(1);
        IImagePostProcessManager::getInstance()->enque(pData);
    }
    MY_LOGD("[PostProc] waitUntilDrained\n");
    IImagePostProcessManager::getInstance()->waitUntilDrained();
    sleep(5);
    MY_LOGD("[PostProc] end of test\n");
    closeSensor();
    return 0;
}
