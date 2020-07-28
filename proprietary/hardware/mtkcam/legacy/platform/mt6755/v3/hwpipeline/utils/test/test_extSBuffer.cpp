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

#define LOG_TAG "MtkCam/utils/test_extSBuffer"
//
#include <vector>
//
#include <utils/String8.h>
#include <mtkcam/utils/std/Log.h>
//
#include <v3/utils/streaminfo/ImageStreamInfo.h>
#include <v3/utils/streambuf/StreamBuffers.h>
//
#include "../ExtStreamBuffers.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#if 1
#define FUNC_START     MY_LOGD_IF(1, "+")
#define FUNC_END       MY_LOGD_IF(1, "-")
#else
#define FUNC_START     MY_LOGD_IF(mLogLevel>=2, "+")
#define FUNC_END       MY_LOGD_IF(mLogLevel>=2, "-")
#endif


/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *
 ******************************************************************************/
namespace {

typedef ExtImageStreamBuffer::              Allocator
                                            ExtImageStreamBufferAllocatorT;
typedef ExtImageStreamBufferAllocatorT::    StreamBufferPoolT
                                            ExtImageStreamBufferPoolT;
/**
 *
 */
static int const
gTestFormat[] = {
    HAL_PIXEL_FORMAT_BLOB,
    HAL_PIXEL_FORMAT_YV12,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,
    HAL_PIXEL_FORMAT_YCbCr_422_I,
#if 0
    HAL_PIXEL_FORMAT_YCbCr_422_SP,
    //
    HAL_PIXEL_FORMAT_Y8,
    HAL_PIXEL_FORMAT_Y16,
#endif
    //
    HAL_PIXEL_FORMAT_YCbCr_420_888,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
    //
    HAL_PIXEL_FORMAT_RGBA_8888,
#if 0
    HAL_PIXEL_FORMAT_RGBX_8888,
    HAL_PIXEL_FORMAT_RGB_888,
    HAL_PIXEL_FORMAT_RGB_565,
    HAL_PIXEL_FORMAT_BGRA_8888,
#endif
    //
    HAL_PIXEL_FORMAT_RAW_OPAQUE,
    HAL_PIXEL_FORMAT_RAW16,
};


/**
 *
 */
static int const
gTestUsage[] = {
#define GRALLOC_USAGE_SW_RW (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN)

#if 0
    0,
    (GRALLOC_USAGE_SW_RW),
#endif

    (                      GRALLOC_USAGE_HW_VIDEO_ENCODER),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_VIDEO_ENCODER),
    //
    (                      GRALLOC_USAGE_HW_TEXTURE),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_TEXTURE),
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_CAMERA_ZSL),
    //
    (GRALLOC_USAGE_SW_RW | GRALLOC_USAGE_HW_CAMERA_ZSL),

};


/**
 *
 */
static int const
gTestResolution[][2] = {
    { 176,  144},
    { 320,  240},
    { 640,  480},
    { 800,  600},
    { 960,  540},
    {1280,  720},
    {1920, 1080},
    {3224, 2448},
};


};  //namespace


// /******************************************************************************
//  *
//  ******************************************************************************/
// static
// void
// test_dump()
// {
//     IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
//     pGrallocHelper->dumpToLog();
// }


// /******************************************************************************
//  *
//  ******************************************************************************/
// static
// void
// test_static()
// {
//     ::printf("+ test_static \n");
//     //
//     IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
//     //
//     for (size_t i = 0; i < ARRAYOF(gTestResolution); i++) {
//         for (size_t j = 0; j < ARRAYOF(gTestFormat); j++) {
//             for (size_t k = 0; k < ARRAYOF(gTestUsage); k++) {
//                 int const format = gTestFormat[j];
//                 int const usage  = gTestUsage[k];
//                 int w = gTestResolution[i][0];
//                 int h = gTestResolution[i][1];
//                 if  ( HAL_PIXEL_FORMAT_BLOB == format ) {
//                     w = w * h;
//                     h = 1;
//                 }
//                 GrallocStaticInfo staticInfo;
//                 GrallocRequest request;
//                 request.usage           = usage;
//                 request.format          = format;
//                 request.widthInPixels   = w;
//                 request.heightInPixels  = h;
//                 int status = pGrallocHelper->query(&request, &staticInfo);
//                 if  ( 0 != status ) {
//                     MY_LOGE(
//                         "[FAILURE] status:%d %dx%d format:%#x(%s) usage:%#x(%s)",
//                         status, w, h,
//                         format, pGrallocHelper->queryPixelFormatName(format).string(),
//                         usage, pGrallocHelper->queryGrallocUsageName(usage).string()
//                     );
//                     continue;
//                 }
//             }
//         }
//     }
//     //
//     ::printf("- test_static \n");
// }


// /******************************************************************************
//  *
//  ******************************************************************************/
// static
// void
// test_dynamic()
// {
//     ::printf("+ test_dynamic \n");
//     //
//     IGrallocHelper const* pGrallocHelper = IGrallocHelper::singleton();
//     //
//     for (size_t i = 0; i < ARRAYOF(gTestResolution); i++) {
//         for (size_t j = 0; j < ARRAYOF(gTestFormat); j++) {
//             for (size_t k = 0; k < ARRAYOF(gTestUsage); k++) {
//                 int const format = gTestFormat[j];
//                 int const usage  = gTestUsage[k];
//                 int w = gTestResolution[i][0];
//                 int h = gTestResolution[i][1];
//                 if  ( HAL_PIXEL_FORMAT_BLOB == format ) {
//                     w = w * h;
//                     h = 1;
//                 }

//                 GrallocStaticInfo staticInfo;
//                 GrallocDynamicInfo dynamicInfo;
//                 GrallocRequest request;
//                 sp<GraphicBuffer> pGB = new GraphicBuffer(w, h, format, usage);
//                 if  ( pGB == 0 || ! pGB->handle ) {
//                     MY_LOGW(
//                         "Bad GraphicBuffer:%p buffer_handle_t:%p %dx%d format:%#x(%s) usage:%#x(%s)",
//                         pGB.get(), pGB->handle, w, h,
//                         format, pGrallocHelper->queryPixelFormatName(format).string(),
//                         usage, pGrallocHelper->queryGrallocUsageName(usage).string()
//                     );
//                     continue;
//                 }
//                 //
//                 int status = pGrallocHelper->query(pGB->handle, &staticInfo, &dynamicInfo);
//                 if  ( 0 != status ) {
//                     MY_LOGE(
//                         "[FAILURE] buffer_handle_t:%p status:%d %dx%d format:%#x(%s) usage:%#x(%s)",
//                         pGB->handle, status, w, h,
//                         format, pGrallocHelper->queryPixelFormatName(format).string(),
//                         usage, pGrallocHelper->queryGrallocUsageName(usage).string()
//                     );
//                     continue;
//                 }
//                 //
//                 MY_LOGD("*****************************************************************************");
//                 MY_LOGD(
//                     "%dx%d usage:0x%08x(%s) format:0x%08x(%s) -->",
//                     w, h,
//                     usage, pGrallocHelper->queryGrallocUsageName(usage).string(),
//                     format, pGrallocHelper->queryPixelFormatName(format).string()
//                 );
//                 MY_LOGD(
//                     "%dx%d usage:0x%08x(%s) format:0x%08x(%s)",
//                     pGB->getWidth(), pGB->getHeight(),
//                     usage, pGrallocHelper->queryGrallocUsageName(usage).string(),
//                     staticInfo.format, pGrallocHelper->queryPixelFormatName(staticInfo.format).string()
//                 );
//                 for (size_t i_plane = 0; i_plane < staticInfo.planes.size(); i_plane++) {
//                     MY_LOGD(
//                         "  [%zu] sizeInBytes:%zu rowStrideInBytes:%zu",
//                         i_plane,
//                         staticInfo.planes[i_plane].sizeInBytes,
//                         staticInfo.planes[i_plane].rowStrideInBytes
//                     );
//                 }
//                 String8 s8IonFds;
//                 s8IonFds.appendFormat("buffer_handle_t:%p #ion_fd=%zu ==> ", pGB->handle, dynamicInfo.ionFds.size());
//                 for (size_t i_ion = 0; i_ion < dynamicInfo.ionFds.size(); i_ion++) {
//                     s8IonFds.appendFormat("%d ", dynamicInfo.ionFds[i_ion]);
//                 }
//                 MY_LOGD("%s", s8IonFds.string());
//             }
//         }
//     }
//     //
//     ::printf("- test_dynamic \n");
// }


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_dump()
{
    ::printf("%s\n", __FUNCTION__);
}

/******************************************************************************
 *
 ******************************************************************************/
static
sp<IImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform = 0
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
static
sp<ExtImageStreamBufferPoolT>
allocatePool(sp<IImageStreamInfo> pStreamInfo)
{
    FUNC_START;
    IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
    }

    sp<ExtImageStreamBufferPoolT>         pPool;

    IImageBufferAllocator::ImgParam const imgParam(
        pStreamInfo->getImgFormat(),
        pStreamInfo->getImgSize(),
        bufStridesInBytes, bufBoundaryInBytes,
        bufPlanes.size()
    );

    pPool = new ExtImageStreamBufferPoolT(
        pStreamInfo->getStreamName(),
        ExtImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
    );

    if  ( pPool == 0 ) {
        MY_LOGE("Fail to new a image pool:%s", pStreamInfo->getStreamName());
        return NULL;
    }
    //
    MERROR err = pPool->initPool(pStreamInfo->getStreamName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if  ( OK != err ) {
        MY_LOGE("%s: initPool err:%d(%s)", pStreamInfo->getStreamName(), err, ::strerror(-err));
        return NULL;
    }
    // if  ( OK != pPool->commitPool(pStreamInfo->getStreamName()) ) {
    //     MY_LOGE("%s: commitPool err:%d(%s)", pStreamInfo->getStreamName(), err, ::strerror(-err));
    //     return NULL;
    // }
    //
    FUNC_END;
    return pPool;
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_image(bool bPool)
{
    FUNC_START;
    ::printf("%s +\n", __FUNCTION__);

    int usage = 0;
    int format = eImgFmt_YUY2;
    MSize const& size = MSize(640,480);
    sp<IImageStreamInfo> pStreamInfo = createImageStreamInfo(
        "Ext:Image:YUV",
        0xFF,
        eSTREAMTYPE_IMAGE_INOUT,
        10, 3,
        usage, format, size
    );

    if( bPool ) {
        sp<ExtImageStreamBufferPoolT> pPool = allocatePool(pStreamInfo);
        std::vector< sp<ExtImageStreamBuffer> > vStreamBuffer;
        ::printf("tart allocate from pool(%p)...\n", pPool.get());
        MY_LOGD("start allocate from pool(%p)...", pPool.get());
        for( int i=0; i<10; i++ ) {
            uint64_t startTime = ::systemTime();
            MY_LOGD("acquireFromPool %d/10 +", i);
            sp<ExtImageStreamBuffer> pStreamBuffer;

            MERROR err = ( !pPool.get() )? NAME_NOT_FOUND :
                         pPool->acquireFromPool(__FUNCTION__, pStreamBuffer, ::s2ns(10));

            if ( err || !pStreamBuffer.get() ) {
                MY_LOGE("[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#" PRIx64 "(%s)",
                        err, ::strerror(-err), pStreamBuffer.get(),
                        pStreamInfo->getStreamId(), pStreamInfo->getStreamName() );
                continue;
            }
            vStreamBuffer.push_back(pStreamBuffer);

            MY_LOGD("acquireFromPool %d/10 : %p:(%" PRId64 " ns) -",
                    i, pStreamBuffer.get(), ::systemTime()-startTime);
        }
        ::printf("start release to pool...\n");
        MY_LOGD("start release to pool...");
        // for(int i=0; i<10 && !vStreamBuffer.empty(); i++ ) {
        for(auto pStreamBuffer : vStreamBuffer) {
            MY_LOGD("releaseToPool +");
            pStreamBuffer->releaseBuffer();
            // pPool->releaseToPool(__FUNCTION__, pStreamBuffer);
            MY_LOGD("releaseToPool : %p -", pStreamBuffer.get());
        }
        vStreamBuffer.clear();
    }

    ::printf("%s -\n", __FUNCTION__);
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
test_metadata()
{

}


/******************************************************************************
 *  Main
 ******************************************************************************/
int
main(int argc , char *argv[])
{
    try {
        MY_LOGD("argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            MY_LOGD("%s", argv[i]);
        }
        //
        ::printf("========================= \n");
        switch  (argc)
        {
        case 2:
            if  ( 0 == ::strcmp(argv[1], "i_p") ) {
                test_image(true);
                test_dump();
            } else if  ( 0 == ::strcmp(argv[1], "i") ) {
                test_image(false);
                test_dump();
            } else if  ( 0 == ::strcmp(argv[1], "m") ) {
                test_metadata();
                test_dump();
            }
            break;
            //
        default:
            test_image(true);
            test_image(false);
            test_metadata();
            test_dump();
            break;
        }
    } catch(std::exception&) {
        ::printf("test_extSBuffer exception\n");
        return 0;
    }
    ::printf("========= END =========== \n");
    //
    return  0;
}

