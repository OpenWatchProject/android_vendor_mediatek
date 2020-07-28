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
#define LOG_TAG "MtkCam/ExifJpeg"
//
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>  // For property_get().
#include <utils/String8.h>
//#include <utils/Thread.h>
#include <utils/Vector.h>
//
#include <future>
#include <vector>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include "ExifJpegUtils.h"
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
//
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)   do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)   do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)   do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)   do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)   do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)   do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)   do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#if 1
#define FUNC_START      MY_LOGD("+")
#define FUNC_END        MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
//
#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSIoPipe;

#define ENABLE_DEBUG_INFO     (1)

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
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
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/******************************************************************************
 *
 ******************************************************************************/
static
MRect
calCropAspect(MSize const& srcSize, MSize const& dstSize)
{
    MRect crop;
#define align2(x) (((x) + 1) & (~0x1))
    MUINT32 val0 = srcSize.w * dstSize.h;
    MUINT32 val1 = srcSize.h * dstSize.w;
    if( val0 > val1 ) {
        crop.s.w = align2(val1 / dstSize.h);
        crop.s.h = srcSize.h;
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    }
    else if ( val0 < val1 ) {
        crop.s.w = srcSize.w;
        crop.s.h = align2(val0 / dstSize.w);
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    }
    else {
        crop = MRect(MPoint(0,0), srcSize);
    }
#undef align2
    return crop;
}

class ExifJpegUtilsImpl
    : public ExifJpegUtils
{

public:     ////                    Interface.
    virtual MERROR                      execute();
    virtual MVOID                       setRotationMain(MBOOL enable) { mbRotationMain = enable; }
    virtual MVOID                       setRotationThumb(MBOOL enable) { mbRotationThumb = enable; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes .
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual char const*                 getName()    const { return mName.string(); }
    virtual int32_t                     getOpenId()  const { return mi4OpenId; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        ExifJpegUtilsImpl(
                                            String8 const& rName,
                                            MINT32 const cameraId,
                                            IMetadata &rHalMetadata,
                                            IMetadata &rAppMetadata,
                                            sp<IImageBufferHeap> pImgBufferHeapDst,
                                            sp<IImageBuffer> pImgBufferMain,
                                            sp<IImageBuffer> pImgBufferThumb
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual                             ~ExifJpegUtilsImpl() {};

public:     ////                        Instantiation.

public:     ////                        attributes.

public:     ////                        Operations.

    struct jpeg_params
    {
        // gps related
        IMetadata::IEntry           gpsCoordinates;
        IMetadata::IEntry           gpsProcessingMethod;
        IMetadata::IEntry           gpsTimestamp;
        //
        MINT32                      orientation;
        MUINT8                      quality;
        MUINT8                      quality_thumbnail;
        MSize                       size_thumbnail;
        //
        MRect                       cropRegion;
        //
                                    jpeg_params()
                                        : gpsCoordinates()
                                        , gpsProcessingMethod()
                                        , gpsTimestamp()
                                        //
                                        , orientation(0)
                                        , quality(90)
                                        , quality_thumbnail(90)
                                        , size_thumbnail(0,0)
                                        //
                                        , cropRegion()
                                    {}
    };

    class encode_params
        : public virtual android::RefBase
    {
        public:
            // buffer
            IImageBuffer*           pSrc;
            IImageBuffer*           pDst;

            // settings
            MUINT32                 transform;
            MRect                   crop;
            MUINT32                 isSOI;
            MUINT32                 quality;
            MUINT32                 codecType;
            //
                                    encode_params()
                                        : pSrc()
                                        , pDst()
                                        //
                                        , transform()
                                        , crop()
                                        , isSOI()
                                        , quality()
                                        , codecType()
                                    {}
    };

    class encode_frame
        : public virtual android::RefBase
    {
    public:
        MBOOL                       mbHasThumbnail;
        MBOOL                       mbSuccess;
        MINT8                       miJpegEncType;
        //
        jpeg_params                 mParams;
        // jpeg destination imagebuffer heap
        sp<IImageBufferHeap>        mpOutImgBufferHeap;
        // jpeg main destination imagebuffer from heap
        sp<IImageBuffer>            mpJpeg_Main;
        // jpeg thumbnail destination imagebuffer from heap
        sp<IImageBuffer>            mpJpeg_Thumbnail;
        //
        StdExif                     exif;
        IMetadata                   rHalMetadata;
        IMetadata                   rAppMetadata;
        //                          constructor
                                    encode_frame(
                                        MBOOL const hasThumbnail,
                                        sp<IImageBufferHeap> &rpOutHeap
                                        )
                                        : mbHasThumbnail(hasThumbnail)
                                        , mbSuccess(MTRUE)
                                        , miJpegEncType(-1)
                                        , mParams()
                                        , mpOutImgBufferHeap(rpOutHeap)
                                        , mpJpeg_Main(NULL)
                                        , mpJpeg_Thumbnail(NULL)
                                        , rHalMetadata()
                                        , rAppMetadata()
                                    {}
    };

private:
    MERROR                              init();

    MERROR                              getJpegParams(
                                            IMetadata* pMetadata,
                                            jpeg_params& rParams
                                        ) const;

    MERROR                              getOutMainImageBuffer(
                                            sp<IImageBufferHeap> &rpImageBufferHeap,
                                            sp<IImageBuffer> &rpImageBuffer
                                        );

    MERROR                              getOutThumbImageBuffer(
                                            sp<IImageBufferHeap> const& rpImageBufferHeap,
                                            sp<IImageBuffer> &rpImageBuffer
                                        );

    MERROR                              performEncodeThumbnail(
                                            sp<encode_frame>& pEncodeFrame,
                                            encode_params &rParams
                                        );

    MERROR                              performEncodeMain(
                                            sp<encode_frame>& pEncodeFrame,
                                            encode_params &rParams
                                        );

    MERROR                              waitJobsDone();

    MVOID                               finalizeEncodeFrame(
                                            sp<encode_frame>& rpEncodeFrame
                                        );

    MVOID                               updateMetadata(
                                            jpeg_params& rParams,
                                            IMetadata* pMetadata_result
                                        ) const;

    MERROR                              makeExifHeader(
                                            sp<encode_frame> rpEncodeFrame,
                                            MINT8 * const pOutExif,
                                            // [IN/OUT] in: exif buf size, out: exif header size
                                            size_t& rOutExifSize
                                        );

    MBOOL                               isHwEncodeSupported(int const format) const;

    MERROR                              hardwareOps_encode(
                                            encode_params& rParams
                                        );

    MVOID                               updateStdExifParam(
                                            MBOOL const&              rNeedExifRotate,
                                            MSize const&                rSize,
                                            IMetadata* const            rpHalMeta,
                                            jpeg_params const&          rParams,
                                            ExifParams &                rStdParams
                                        ) const;

    MVOID                               updateStdExifParam_3A(
                                            IMetadata const&            rMeta,
                                            ExifParams &                rStdParams
                                        ) const;

    MVOID                               updateStdExifParam_gps(
                                            IMetadata::IEntry const& rGpsCoordinates,
                                            IMetadata::IEntry const& rGpsProcessingMethod,
                                            IMetadata::IEntry const& rGpsTimestamp,
                                            ExifParams &        rStdParams
                                        ) const;

    MVOID                               updateDebugInfoToExif(
                                            IMetadata* const            rpHalMeta,
                                            StdExif &                   exif
                                        ) const;

    MUINT32                             calcZoomRatio(
                                            MRect      const&   cropRegion,
                                            MSize      const&   rSize
                                        ) const;

    MVOID                               setResultMetadata(IMetadata &rResultMetadata);

private:    ////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    String8 const                       mName;
    MINT32                              mLogLevel;
    MINT32                              mi4OpenId;

    // input metadata
    IMetadata                           mHalMetadata;
    IMetadata                           mAppMetadata;
    IMetadata                           mResultMetadata;

    // input/output imagebuffer
    sp<IImageBufferHeap>                mpDstImgBufHeap;
    //
    sp<IImageBuffer>                    mpSrcImgBuf_main;
    sp<IImageBuffer>                    mpSrcImgBuf_thumbnail;

    std::vector< std::future<MERROR> >  mvFutures;

    MBOOL                               mbHasThumb;

    // main params for this execute job
    sp<encode_frame>                    mpEncodeFrame;

    // encode job to use SImager
    encode_params                       mMainJob;
    encode_params                       mThumbnailJob;

    MUINT8                              muFacing; // ref: MTK_LENS_FACING_
    MRect                               mActiveArray;

    MBOOL                               mbRotationMain;
    MBOOL                               mbRotationThumb;
};

/******************************************************************************
 *
 ******************************************************************************/
ExifJpegUtilsImpl::
ExifJpegUtilsImpl(
    String8 const& rName,
    MINT32 const cameraId,
    IMetadata &rHalMetadata,
    IMetadata &rAppMetadata,
    sp<IImageBufferHeap> pImgBufferHeapDst,
    sp<IImageBuffer> pImgBufferMain,
    sp<IImageBuffer> pImgBufferThumb
)
    : ExifJpegUtils()
    , mName(rName)
    , mi4OpenId(cameraId)
    , mHalMetadata(rHalMetadata)
    , mAppMetadata(rAppMetadata)
    , mpDstImgBufHeap(pImgBufferHeapDst)
    , mpSrcImgBuf_main(pImgBufferMain)
    , mpSrcImgBuf_thumbnail(pImgBufferThumb)
    //
    , mbHasThumb()
    , muFacing()
    //
    , mbRotationMain(MFALSE)
    , mbRotationThumb(MTRUE)
{
    FUNC_START;
    //
    mLogLevel = ::property_get_int32("debug.camera.log.exifjpeg", 0);
    //
    {
        sp<IMetadataProvider> pMetadataProvider =
            NSMetadataProviderManager::valueFor( mi4OpenId );
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
        }
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( !tryGetMetadata<MRect>(
                    &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                    mActiveArray)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        }
        //
        if( !tryGetMetadata<MUINT8>(
                    &static_meta, MTK_SENSOR_INFO_FACING,
                    muFacing)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_FACING");
        }
        MY_LOGD_IF(1,"active array(%d, %d, %dx%d), facing %d",
                mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
                muFacing);
    }
    //
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<ExifJpegUtils>
ExifJpegUtils::
createInstance(
    MINT32 const cameraId,
    IMetadata &rHalMetadata,
    IMetadata &rAppMetadata,
    sp<IImageBufferHeap> pImgBufferHeapDst,
    sp<IImageBuffer> pImgBufferMain,
    sp<IImageBuffer> pImgBufferThumb
)
{
    FUNC_START;
    if ( ! pImgBufferHeapDst.get() || ! pImgBufferMain.get() )
        return NULL;
    //
    String8 const& rName = String8(LOG_TAG);
    sp<ExifJpegUtilsImpl> pObj =  new ExifJpegUtilsImpl(
            rName, cameraId, rHalMetadata, rAppMetadata,
            pImgBufferHeapDst, pImgBufferMain, pImgBufferThumb );
    if ( !pObj.get() ) {
        MY_LOGE("Instantiation fail");
        return NULL;
    }
    //
    FUNC_END;
    return pObj;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
init()
{
    FUNC_START;
    //
    if ( !mpDstImgBufHeap.get() || !mpSrcImgBuf_main.get() ) {
        MY_LOGE("error input params");
        return NAME_NOT_FOUND;
    }
    mbHasThumb = ( !mpSrcImgBuf_thumbnail.get() )? MFALSE : MTRUE;
    if ( !mbHasThumb )
        MY_LOGW("no source of thumbnail buffer");
    //
    // new frame
    mpEncodeFrame = new encode_frame(mbHasThumb, mpDstImgBufHeap);
    if ( !mpEncodeFrame.get() )
        return INVALID_OPERATION;
    //
    // get jpeg params from app metadata
    CHECK_ERROR( getJpegParams( &mAppMetadata, mpEncodeFrame->mParams) );
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
setResultMetadata(IMetadata &rResultMetadata)
{
    FUNC_START;
    mResultMetadata = rResultMetadata;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
execute()
{
    FUNC_START;
    CHECK_ERROR( init() );

    // 1. create output main imagebuffer from heap
    CHECK_ERROR( getOutMainImageBuffer( mpEncodeFrame->mpOutImgBufferHeap,
                                        mpEncodeFrame->mpJpeg_Main) );
    mMainJob.pDst = mpEncodeFrame->mpJpeg_Main.get();
    mMainJob.pSrc = mpSrcImgBuf_main.get();
    MY_LOGD_IF( 1, "[out]heapVA:%p, imgbufferVA:%p ",
                    (void *)mpEncodeFrame->mpOutImgBufferHeap->getBufVA(0),
                    (void *)mpEncodeFrame->mpJpeg_Main->getBufVA(0) );

    // 2. create output thumbnail imagebuffer if exists,
    //    encode thumbnail w/ another thread
    if ( mbHasThumb ) {
        CHECK_ERROR( getOutThumbImageBuffer( mpEncodeFrame->mpOutImgBufferHeap,
                                             mpEncodeFrame->mpJpeg_Thumbnail) );
        mThumbnailJob.pDst = mpEncodeFrame->mpJpeg_Thumbnail.get();
        mThumbnailJob.pSrc = mpSrcImgBuf_thumbnail.get();
        //check thumb image buffer offset
        MY_LOGD_IF( 1, "[out]heapVA:%p, imgbufferVA:%p ",
                    (void *)mpEncodeFrame->mpOutImgBufferHeap->getBufVA(0),
                    (void *)mpEncodeFrame->mpJpeg_Thumbnail->getBufVA(0) );
//        mThumbDoneFlag = MFALSE;
        //
        mvFutures.push_back(
            std::async(std::launch::async,
                [ this ]() {
                    MERROR err = OK;
                    err = performEncodeThumbnail(mpEncodeFrame, mThumbnailJob);
                    if ( OK != err )
                        MY_LOGE("thumbnail encode failure");
                    return err;
                }
            )
        );
    }


    // 3. get src buffers & internal dst buffer for bitstream
    while ( mpEncodeFrame->mpJpeg_Main.get() ) {
        CHECK_ERROR( performEncodeMain(mpEncodeFrame, mMainJob) );
        break;
    }

    //
    CHECK_ERROR( waitJobsDone() );

    // if ( OK != waitJobsDone() ) {
    //     MY_LOGD("encode main jpeg fail");
    //     return MFALSE;
    // }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
getJpegParams(
    IMetadata* pMetadata,
    jpeg_params& rParams
) const
{
    FUNC_START;
    if( !pMetadata )
    {
        MY_LOGE("pMetadata=NULL");
        return NAME_NOT_FOUND;
    }
    //
    rParams.gpsCoordinates =
        pMetadata->entryFor(MTK_JPEG_GPS_COORDINATES);
    rParams.gpsProcessingMethod =
        pMetadata->entryFor(MTK_JPEG_GPS_PROCESSING_METHOD);
    rParams.gpsTimestamp =
        pMetadata->entryFor(MTK_JPEG_GPS_TIMESTAMP);

#define getParam(meta, tag, type, param)                \
    do {                                                \
        if( !tryGetMetadata<type>(meta, tag, param) ) { \
            MY_LOGW("no tag: %s", #tag);                \
        }                                               \
    } while(0)
#define getAppParam(tag, type, param) getParam(pMetadata, tag, type, param)

    // request from app
    getAppParam(MTK_JPEG_ORIENTATION      , MINT32, rParams.orientation);
    getAppParam(MTK_JPEG_QUALITY          , MUINT8, rParams.quality);
    getAppParam(MTK_JPEG_THUMBNAIL_QUALITY, MUINT8, rParams.quality_thumbnail);
    getAppParam(MTK_JPEG_THUMBNAIL_SIZE   , MSize , rParams.size_thumbnail);
    getAppParam(MTK_SCALER_CROP_REGION    , MRect , rParams.cropRegion);

#undef getAppParam
#undef getParam
//
    if( mbHasThumb && mpSrcImgBuf_thumbnail != NULL )
    {
        rParams.size_thumbnail = mpSrcImgBuf_thumbnail->getImgSize();
    }
    else
    {
        rParams.size_thumbnail = MSize(0,0);
    }
    MY_LOGD_IF( 1,"@@getJpegParams thumb size(w,h)=(%dx%d)",
                rParams.size_thumbnail.w, rParams.size_thumbnail.h);
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
updateStdExifParam(
    MBOOL const&                rNeedExifRotate __attribute__((__unused__)),
    MSize const&                rSize,
    IMetadata* const            rpHalMeta,
    jpeg_params const&          rParams,
    ExifParams &                rStdParams
) const
{
    rStdParams.u4ImageWidth  = rSize.w;
    rStdParams.u4ImageHeight = rSize.h;
    //
    //HAL1 not supported HEIC yet, so bNeedSOI should always be TRUE
    rStdParams.bNeedSOI = MTRUE;
    //
    // 3A
    if( rpHalMeta ) {
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
            updateStdExifParam_3A(exifMeta, rStdParams);
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta");
    }
    // gps
    updateStdExifParam_gps(
            rParams.gpsCoordinates, rParams.gpsProcessingMethod, rParams.gpsTimestamp,
            rStdParams
            );
    // others
    rStdParams.u4Orientation = 0;

    rStdParams.u4ZoomRatio   = calcZoomRatio(rParams.cropRegion, rSize);
    rStdParams.u4Facing      = (muFacing == MTK_LENS_FACING_BACK) ? 0 : 1;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
updateStdExifParam_3A(
    IMetadata const&            rMeta,
    ExifParams &                rStdParams
) const
{
#define getParam(meta, tag, type, param)                      \
            do {                                              \
                type val = 0;                                 \
                if( !tryGetMetadata<type>(meta, tag, val) ) { \
                    MY_LOGW("no tag: %s", #tag);              \
                }                                             \
                param = val;                                  \
            } while(0)
#if 1
            // from result meta of 3A
            getParam(&rMeta, MTK_3A_EXIF_FNUMBER,             MINT32, rStdParams.u4FNumber);
            getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH,        MINT32, rStdParams.u4FocalLength);
            getParam(&rMeta, MTK_3A_EXIF_AWB_MODE,            MINT32, rStdParams.u4AWBMode);
            getParam(&rMeta, MTK_3A_EXIF_LIGHT_SOURCE,        MINT32, rStdParams.u4LightSource);
            getParam(&rMeta, MTK_3A_EXIF_EXP_PROGRAM,         MINT32, rStdParams.u4ExpProgram);
            getParam(&rMeta, MTK_3A_EXIF_SCENE_CAP_TYPE,      MINT32, rStdParams.u4SceneCapType);
            getParam(&rMeta, MTK_3A_EXIF_FLASH_LIGHT_TIME_US, MINT32, rStdParams.u4FlashLightTimeus);
            getParam(&rMeta, MTK_3A_EXIF_AE_METER_MODE,       MINT32, rStdParams.u4AEMeterMode);
            getParam(&rMeta, MTK_3A_EXIF_AE_EXP_BIAS,         MINT32, rStdParams.i4AEExpBias);
            getParam(&rMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   MINT32, rStdParams.u4CapExposureTime);
            getParam(&rMeta, MTK_3A_EXIF_AE_ISO_SPEED,        MINT32, rStdParams.u4AEISOSpeed);
#endif

#undef getParam
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
updateStdExifParam_gps(
    IMetadata::IEntry const& rGpsCoordinates,
    IMetadata::IEntry const& rGpsProcessingMethod,
    IMetadata::IEntry const& rGpsTimestamp,
    ExifParams &        rStdParams
) const
{
    if( rGpsCoordinates.count() == 3 ) {
        rStdParams.u4GpsIsOn = 1;
        // latitude
        ::snprintf(
                reinterpret_cast<char*>(rStdParams.uGPSLatitude), 32,
                "%f", rGpsCoordinates.itemAt(0, Type2Type<MDOUBLE>())
                );
        // longitude
        ::snprintf(
                reinterpret_cast<char*>(rStdParams.uGPSLongitude), 32,
                "%f", rGpsCoordinates.itemAt(1, Type2Type<MDOUBLE>())
                );
        // altitude
        rStdParams.u4GPSAltitude = (MUINT32)rGpsCoordinates.itemAt(2, Type2Type<MDOUBLE>());

        // timestamp
        if( !rGpsTimestamp.isEmpty() )
            ::snprintf(
                    reinterpret_cast<char*>(rStdParams.uGPSTimeStamp), 32,
                    "%" PRId64 "", rGpsTimestamp.itemAt(0, Type2Type<MINT64>())
                    );
        else
            MY_LOGW("no MTK_JPEG_GPS_TIMESTAMP");

        if( !rGpsProcessingMethod.isEmpty() ) {
            size_t size = rGpsProcessingMethod.count();
            if( size > 64 ) {
                MY_LOGW("gps processing method too long, size %zu", size);
                size = 64;
            }

            for( size_t i = 0; i < size; i++ ) {
                rStdParams.uGPSProcessingMethod[i] =
                    rGpsProcessingMethod.itemAt(i, Type2Type<MUINT8>());
            }
            rStdParams.uGPSProcessingMethod[63] = '\0'; //null-terminating
        }
        else
            MY_LOGW("no MTK_JPEG_GPS_PROCESSING_METHOD");
    }
    else {
        MY_LOGD_IF( 1,
                "no gps data, coordinates count %d",
                rGpsCoordinates.count()
                );
        // no gps data
        rStdParams.u4GpsIsOn = 0;
    }
}



/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ExifJpegUtilsImpl::
calcZoomRatio(
    MRect      const&   cropRegion,
    MSize      const&   rSize
) const
{
    //RWLock::AutoRLock _l(mConfigRWLock);
    MUINT32 zoomRatio = 100;
    // if( ! mpOutJpeg.get() ) {
    //     MY_LOGW("jpeg stream is not configured");
    //     return 100;
    // }

    MRect const cropAspect = calCropAspect(cropRegion.s, rSize);//mpOutJpeg->getImgSize()
    if( ! cropAspect.s ) {
        MY_LOGW("cropRegion(%d, %d, %dx%d), jpeg size %dx%d",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            rSize.w, rSize.h
            );
        return 100;
    }

    {
        MUINT32 val0 = cropAspect.s.w * mActiveArray.s.h;
        MUINT32 val1 = cropAspect.s.h * mActiveArray.s.w;
        if( val0 > val1 )
            zoomRatio = mActiveArray.s.w * 100 / cropAspect.s.w;
        else
            zoomRatio = mActiveArray.s.h * 100 / cropAspect.s.h;
    }

    MY_LOGD_IF(0, "active(%d, %d, %dx%d), cropRegion(%d, %d, %dx%d), zoomRatio %d",
            mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            zoomRatio
            );
    return zoomRatio;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
getOutMainImageBuffer(
    sp<IImageBufferHeap> &rpImageBufferHeap,
    sp<IImageBuffer> &rpImageBuffer
)
{
    FUNC_START;
    if ( ! rpImageBufferHeap.get() )
        return NAME_NOT_FOUND;
    //
    MSize imageSize = mpSrcImgBuf_main->getImgSize();
    //
    if( mbRotationMain &&
        ( mpEncodeFrame->mParams.orientation == 90 ||
          mpEncodeFrame->mParams.orientation == 270 ) )
    {
        MUINT temp = imageSize.w;
        imageSize.w = imageSize.h;
        imageSize.h = temp;
    }
    //
    MBOOL needExifRotate = MTRUE;
    MY_LOGD( "Main Yuv transform:%d orientation(metadata):%d imgSize:%dx%d",
             0, mpEncodeFrame->mParams.orientation,
             imageSize.w, imageSize.h);
    //
    if ( rpImageBufferHeap.get() ) {
        // skip some process of StreamBuffer
        MUINT8 encodeType;
        if( tryGetMetadata<MUINT8>(&mHalMetadata, MTK_JPG_ENCODE_TYPE, encodeType) )
        {
            mpEncodeFrame->miJpegEncType = encodeType;
            MY_LOGD( "Assign encode type manually.(%d)",
                     mpEncodeFrame->miJpegEncType);
        }
        ExifParams stdParams;
        // update standard exif params
        updateStdExifParam(
                needExifRotate,
                imageSize,
                &mHalMetadata,
                mpEncodeFrame->mParams,
                stdParams
                );
        mpEncodeFrame->exif.init(stdParams, ENABLE_DEBUG_INFO);
    }
    else {
        MY_LOGD("have not got main imagebufferheap");
    }

    //  Query the group usage.
    //MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());
    MUINT const groupUsage =
        eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE;

    size_t thumbnailMaxSize = 0;
    if( mpEncodeFrame->mbHasThumbnail )
    {
        thumbnailMaxSize = (mpEncodeFrame->mParams.size_thumbnail.w) *
                           (mpEncodeFrame->mParams.size_thumbnail.h) * 18 / 10;
        size_t res = thumbnailMaxSize % 128;
        if( res != 0 )
            thumbnailMaxSize = thumbnailMaxSize + 128 - res;
    }
    size_t headerSize = 0;
    headerSize = mpEncodeFrame->exif.getStdExifSize() +
                 mpEncodeFrame->exif.getDbgExifSize() + thumbnailMaxSize;
    if(headerSize % 128 != 0)
        MY_LOGW("not aligned header size %zu", headerSize);
    //
    {
        mpEncodeFrame->exif.setMaxThumbnail(thumbnailMaxSize);
        size_t mainOffset  = mpEncodeFrame->exif.getHeaderSize();
        size_t mainMaxSize = rpImageBufferHeap->getBufSizeInBytes(0) - mainOffset;
        MY_LOGD( "heap size = %d, mainOffset = %zu, mainMaxSize = %zu",
                 (int)rpImageBufferHeap->getBufSizeInBytes(0), mainOffset, mainMaxSize);
        //
        size_t const bufStridesInBytes[3] = {mainMaxSize, 0 ,0};
        rpImageBuffer = rpImageBufferHeap->createImageBuffer_FromBlobHeap(
                mainOffset, eImgFmt_JPEG, imageSize, bufStridesInBytes
                );
        if ( ! rpImageBuffer.get() ) {
            MY_LOGE("rpImageMainBuffer is NULL");
            return BAD_VALUE;
        }
        rpImageBuffer->lockBuf(getName(), groupUsage);
    }
    MY_LOGD("image heap: %p, buffer: %p, usage: %d, heap format: 0x%x",
            rpImageBufferHeap.get(), rpImageBuffer.get(),
            groupUsage, mpEncodeFrame->mpOutImgBufferHeap->getImgFormat() );
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
getOutThumbImageBuffer(
    sp<IImageBufferHeap> const& rpImageBufferHeap,
    sp<IImageBuffer> &rpImageBuffer
)
{
    FUNC_START;
    if( ! rpImageBufferHeap.get() ) {
        MY_LOGE("heap not exist");
        return BAD_VALUE;
    }
    //get thumb IImageBuffer
    size_t thumbnailOffset  = mpEncodeFrame->exif.getStdExifSize();

    MY_LOGD( "thumbnail size: params(%dx%d) imagebuffer(%dx%d) exifsize(%zu)",
             mpEncodeFrame->mParams.size_thumbnail.w, mpEncodeFrame->mParams.size_thumbnail.h,
             mpSrcImgBuf_thumbnail->getImgSize().w, mpSrcImgBuf_thumbnail->getImgSize().h,
             thumbnailOffset );
    mpEncodeFrame->mParams.size_thumbnail = mpSrcImgBuf_thumbnail->getImgSize();
    if( mbRotationThumb &&
        ( mpEncodeFrame->mParams.orientation == 90 ||
          mpEncodeFrame->mParams.orientation == 270 ) )
    {
        MUINT temp = mpEncodeFrame->mParams.size_thumbnail.w;
        mpEncodeFrame->mParams.size_thumbnail.w = mpEncodeFrame->mParams.size_thumbnail.h;
        mpEncodeFrame->mParams.size_thumbnail.h = temp;
    }
    size_t thumbnailMaxSize = (mpEncodeFrame->mParams.size_thumbnail.w) * (mpEncodeFrame->mParams.size_thumbnail.h) * 18 / 10;

    size_t res = thumbnailMaxSize % 128;
    if(res != 0) {
        thumbnailMaxSize = thumbnailMaxSize + 128 - res;
    }
    size_t const bufStridesInBytes[3] = {thumbnailMaxSize, 0 ,0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};
    // ref v1 prepare heap & imagebuffer
    IImageBufferAllocator::ImgParam imgParam =
                            IImageBufferAllocator::ImgParam(
                                rpImageBufferHeap->getImgFormat(),//blob
                                MSize(
                                    mpEncodeFrame->mParams.size_thumbnail.w,
                                    mpEncodeFrame->mParams.size_thumbnail.h
                                    ),
                                bufStridesInBytes,
                                bufBoundaryInBytes,
                                Format::queryPlaneCount(rpImageBufferHeap->getImgFormat())
                            );
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                rpImageBufferHeap->getHeapID(),
                                (MUINTPTR)(rpImageBufferHeap->getBufVA(0) + thumbnailOffset)
                                );

    MBOOL mbEnableIImageBufferLog = MTRUE;
    sp<IImageBufferHeap> pHeap = ImageBufferHeap::create(
                                LOG_TAG,
                                imgParam,
                                portBufInfo,
                                mbEnableIImageBufferLog
                                );
    if(pHeap == 0) {
        MY_LOGE("pHeap is NULL");
        return BAD_VALUE;
    }

    rpImageBuffer = pHeap->createImageBuffer_FromBlobHeap(
            0, eImgFmt_JPEG, mpEncodeFrame->mParams.size_thumbnail, bufStridesInBytes );

    MUINT const groupUsage =
        eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE;
    rpImageBuffer->lockBuf( getName(), groupUsage);

    if ( !rpImageBuffer.get() ) {
        MY_LOGE("rpImageThumbnailBuffer == NULL");
        return BAD_VALUE;
    }

    MY_LOGD("thumb(w,h)=(%dx%d) thumbOffset = %zu, thumbMaxSize = %zu",
            mpEncodeFrame->mParams.size_thumbnail.w, mpEncodeFrame->mParams.size_thumbnail.h, thumbnailOffset, thumbnailMaxSize);
    MY_LOGD("thumb heap: %p, buffer: %p",
            rpImageBufferHeap.get(), rpImageBuffer.get());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
performEncodeThumbnail(
    sp<encode_frame>& pEncodeFrame,
    encode_params &rParams
)
{
    if(pEncodeFrame == NULL)
    {
        MY_LOGE("thumb encode frame is null");
        return NAME_NOT_FOUND;
    }
    // to encode thumbnail
    MSize thumbsize = pEncodeFrame->mParams.size_thumbnail;
    MY_LOGD("thumb YUV (w,h)=(%dx%d), thumb jpeg(w,h)=(%dx%d) orientation(%d)",
            rParams.pSrc->getImgSize().w, rParams.pSrc->getImgSize().h,
            rParams.pDst->getImgSize().w, rParams.pDst->getImgSize().h,
            pEncodeFrame->mParams.orientation );
    // do encode
    {
        if(mbRotationThumb)
        {
            switch(pEncodeFrame->mParams.orientation) {
                case 90:
                    rParams.transform = eTransform_ROT_90;
                    break;
                case 180:
                    rParams.transform = eTransform_ROT_180;
                    break;
                case 270:
                    rParams.transform = eTransform_ROT_270;
                    break;
                default:
                    rParams.transform = 0;
                    break;
            }
        }
        else
        {
            rParams.transform = 0;
        }
        //
        rParams.crop = calCropAspect(rParams.pSrc->getImgSize(), thumbsize);
        rParams.isSOI = 1;
        rParams.quality = pEncodeFrame->mParams.quality_thumbnail;
        rParams.codecType = NSSImager::JPEGENC_SW;
        //
        MERROR const err = hardwareOps_encode(rParams);
        if( err != OK ) {
            MY_LOGE( "thumb encode fail src %p, type 0x%x, dst %p, type 0x%x",
                     (void *)rParams.pSrc, rParams.pSrc->getImgFormat(),
                     (void *)rParams.pDst->getBufVA(0), rParams.pDst->getImgFormat());
            pEncodeFrame->mbSuccess = MFALSE;
        }
        MY_LOGD("thumbnail bistream size %zu", mpEncodeFrame->mpJpeg_Thumbnail->getBitstreamSize());
    }
    //
    {
        #if 0
        bool shouldPrint = true;
        if (shouldPrint)
        {
            char filename[256];
            sprintf( filename, "/sdcard/DCIM/Camera/thumbnailYUV_%d_%d.jpg",
                     rParams.pDst->getImgSize().w, rParams.pDst->getImgSize().h );
            NSCam::Utils::saveBufToFile( filename, (unsigned char*)rParams.pDst->getBufVA(0),
                                         rParams.pDst->getBufSizeInBytes(0));
            MUINT const groupUsage =
                eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
            rParams.pSrc->lockBuf(getName(), groupUsage);
            sprintf( filename, "/sdcard/DCIM/Camera/thumbnailYUV_%d_%d.yuv",
                     rParams.pSrc->getImgSize().w, rParams.pSrc->getImgSize().h );
            NSCam::Utils::saveBufToFile( filename, (unsigned char*)rParams.pSrc->getBufVA(0),
                                         rParams.pSrc->getBufSizeInBytes(0));
            rParams.pSrc->unlockBuf(getName());
            shouldPrint = false;
        }
        #endif
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
performEncodeMain(
    sp<encode_frame>& pEncodeFrame __attribute__((__unused__)),
    encode_params &rParams
)
{
    FUNC_START;
    MY_LOGD( "YUV main imagebuffer(w,h)=(%dx%d), main jpeg(w,h)=(%dx%d)",
             rParams.pSrc->getImgSize().w, rParams.pSrc->getImgSize().h,
             rParams.pDst->getImgSize().w, rParams.pDst->getImgSize().h );
    //
    // do encode
    {
        MUINT8 iEncTypeCheck = isHwEncodeSupported(rParams.pSrc->getImgFormat()) ?
                NSSImager::JPEGENC_HW_FIRST : NSSImager::JPEGENC_SW;
        if(mbRotationMain)
        {
            MY_LOGD();
            switch(pEncodeFrame->mParams.orientation) {
                case 90:
                    rParams.transform = eTransform_ROT_90;
                    break;
                case 180:
                    rParams.transform = eTransform_ROT_180;
                    break;
                case 270:
                    rParams.transform = eTransform_ROT_270;
                    break;
                default:
                    rParams.transform = 0;
                    break;
            }
        }
        else
        {
            rParams.transform = 0;
        }
        rParams.crop = MRect(MPoint(0,0), rParams.pSrc->getImgSize());
        rParams.isSOI = 0;
        rParams.quality = mpEncodeFrame->mParams.quality;
        if( -1 != mpEncodeFrame->miJpegEncType &&
            NSSImager::JPEGENC_HW_FIRST == iEncTypeCheck )
            iEncTypeCheck = NSSImager::JPEGENC_HW_ONLY;
        rParams.codecType = iEncTypeCheck;
        //
        MERROR const err = hardwareOps_encode(rParams);
        if( err != OK ) {
            mpEncodeFrame->mbSuccess = MFALSE;
        }
        MY_LOGD("main bistream size %zu", mpEncodeFrame->mpJpeg_Main->getBitstreamSize());
    }
    {
        #if 0
        bool shouldPrint = true;
        if (shouldPrint) {
            char filename[256];
            sprintf(filename, "/sdcard/DCIM/Camera/mainYUV_%d_%d.jpg",
                rParams.pDst->getImgSize().w, rParams.pDst->getImgSize().h );
            NSCam::Utils::saveBufToFile( filename, (unsigned char*)rParams.pDst->getBufVA(0),
                                         rParams.pDst->getBufSizeInBytes(0));
             MUINT const groupUsage =
                eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
            rParams.pSrc->lockBuf(getName(), groupUsage);
            sprintf(filename, "/sdcard/DCIM/Camera/mainYUV_%d_%d.yuv",
                rParams.pSrc->getImgSize().w, rParams.pSrc->getImgSize().h );
            NSCam::Utils::saveBufToFile( filename, (unsigned char*)rParams.pSrc->getBufVA(0),
                                         rParams.pSrc->getBufSizeInBytes(0));
            rParams.pSrc->unlockBuf(getName());
            shouldPrint = false;
        }
        #endif
    }
    //
    {
        size_t const totalJpegSize =
            mpEncodeFrame->mpJpeg_Main->getBitstreamSize() + mpEncodeFrame->exif.getHeaderSize();
        MBOOL bRet =  mpDstImgBufHeap->setBitstreamSize(totalJpegSize);
        if ( MTRUE != bRet )
            return NO_MEMORY;
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
waitJobsDone()
{
    FUNC_START;
    // if no thumbnail, copy to dst buffer & release buffers/metadata
    // else add to pending list to wait for the other src buffer
    if( // condition 1: without thumbnail
        ( !mpEncodeFrame->mbHasThumbnail && mpEncodeFrame->mpJpeg_Main.get() ) ||
        // condition 2: with thumbnail
        (  mpEncodeFrame->mbHasThumbnail && mpEncodeFrame->mpJpeg_Main.get() &&
           mpEncodeFrame->mpJpeg_Thumbnail.get() )
    )
    {
        NSCam::MERROR err = OK;
        for( auto &fut : mvFutures ) {
            NSCam::MERROR result = fut.get();
            if( result != OK ) {
                err = result;
            }
        }
        mvFutures.clear();
        // unlock imagebuffer
        mpEncodeFrame->mpJpeg_Main->unlockBuf( getName() );
        if( mpEncodeFrame->mpJpeg_Thumbnail.get() )
            mpEncodeFrame->mpJpeg_Thumbnail->unlockBuf( getName() );
        finalizeEncodeFrame(mpEncodeFrame);
        //delete mpEncodeFrame;
        mpEncodeFrame = NULL;
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
finalizeEncodeFrame(
    sp<encode_frame>& rpEncodeFrame
)
{
    FUNC_START;
    // update metadata : skip
    {
    }
    // get out buffer
    {
        sp<IImageBuffer> pOutImageBuffer = NULL;
        pOutImageBuffer = mpDstImgBufHeap->
                createImageBuffer_FromBlobHeap(0, rpEncodeFrame->exif.getHeaderSize());
        if ( !pOutImageBuffer.get() )
            MY_LOGE("rpImageBuffer == NULL");
        //MUINT const groupUsage = pOutImgStreamBuffer->queryGroupUsage(getNodeId());
        MUINT const groupUsage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE;
        pOutImageBuffer->lockBuf(getName(), groupUsage);

        size_t exifSize  = rpEncodeFrame->exif.getHeaderSize();
        MINT8 * pExifBuf = reinterpret_cast<MINT8*>(pOutImageBuffer->getBufVA(0));
        if( pExifBuf == NULL ||
            OK != makeExifHeader(rpEncodeFrame, pExifBuf, exifSize) )
        {
            rpEncodeFrame->mbSuccess = MFALSE;
            MY_LOGE( "make exif header failed: buf %p, size %zu",
                     pExifBuf, exifSize);
        }
        if( pExifBuf )
            pExifBuf = NULL;
        //
        pOutImageBuffer->unlockBuf(getName());
    }
    static int count = 0;
    if( mLogLevel > 1) {
        int jpeg_size = rpEncodeFrame->exif.getHeaderSize() +
                                rpEncodeFrame->mpJpeg_Main->getBitstreamSize();
        sp<IImageBuffer> dumpImgBuffer = mpDstImgBufHeap->
                createImageBuffer_FromBlobHeap(0, jpeg_size);

        if(dumpImgBuffer.get() == NULL) {
            MY_LOGE("dumpBuffer == NULL");
        }

        MUINT const groupUsage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE;
        dumpImgBuffer->lockBuf(getName(), groupUsage);

        char filename[256] = {0};
        if(!NSCam::Utils::makePath("/sdcard/camera_dump/", 0660))
            MY_LOGI("makePath[%s] fails", "/sdcard/camera_dump/");
        sprintf(filename, "/sdcard/camera_dump/%d_%dx%d_%d.jpg",
                                count, mpDstImgBufHeap->getImgSize().w, mpDstImgBufHeap->getImgSize().h, jpeg_size );
        count ++;
        NSCam::Utils::saveBufToFile(filename,
                                (unsigned char*)dumpImgBuffer->getBufVA(0), jpeg_size);

        dumpImgBuffer->unlockBuf(getName());
    }
    // release
    FUNC_END;
    //return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
updateMetadata(
    jpeg_params& rParams,
    IMetadata* pMetadata_result
) const
{
#define updateNonEmptyEntry(pMetadata, tag, entry) \
    do{                                            \
        if( !entry.isEmpty() ) {                   \
            pMetadata->update(tag, entry);         \
        }                                          \
    }while(0)

    // gps related
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_COORDINATES       , rParams.gpsCoordinates);
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_PROCESSING_METHOD , rParams.gpsProcessingMethod);
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_TIMESTAMP         , rParams.gpsTimestamp);
    //
    updateEntry<MINT32>(pMetadata_result , MTK_JPEG_ORIENTATION       , rParams.orientation);
    updateEntry<MUINT8>(pMetadata_result , MTK_JPEG_QUALITY           , rParams.quality);
    updateEntry<MUINT8>(pMetadata_result , MTK_JPEG_THUMBNAIL_QUALITY , rParams.quality_thumbnail);
    updateEntry<MSize>(pMetadata_result  , MTK_JPEG_THUMBNAIL_SIZE    , rParams.size_thumbnail);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
makeExifHeader(
    sp<encode_frame> rpEncodeFrame,
    MINT8 * const pOutExif,
    size_t& rOutExifSize // [IN/OUT] in: exif buf size, out: exif header size
)
{
    MERROR ret;

    //IMetadata*            pInMeta_Hal       = NULL;
    //sp<IMetaStreamBuffer> pInMetaStream_Hal = NULL;
    //

    //
    //rpEncodeFrame->exif.setOutputBuffer(pOutExif, rOutExifSize);
    //
    // update debug info
#if ENABLE_DEBUG_INFO
    updateDebugInfoToExif(&mHalMetadata, rpEncodeFrame->exif);
#endif
    //
    ret = rpEncodeFrame->exif.make((MUINTPTR)pOutExif, rOutExifSize);
    //
    rpEncodeFrame->exif.uninit();
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExifJpegUtilsImpl::
updateDebugInfoToExif(
    IMetadata* const            rpHalMeta,
    StdExif &                   exif
) const
{
    if( rpHalMeta ) {
        IMetadata exifMeta;
        if ( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) )
        {
            MUINT32 dbgKey = MTK_3A_EXIF_DEBUGINFO_BEGIN;
            MUINT32 dbgVal = MTK_3A_EXIF_DEBUGINFO_BEGIN + 1;
            while( dbgVal < MTK_3A_EXIF_DEBUGINFO_END )
            {
                MINT32 key;
                IMetadata::Memory dbgmem;
                if( tryGetMetadata<MINT32>(&exifMeta, dbgKey, key) &&
                    tryGetMetadata<IMetadata::Memory>(&exifMeta, dbgVal, dbgmem) )
                {
                    MINT32 ID;
                    void* data = static_cast<void*>(dbgmem.editArray());
                    size_t size = dbgmem.size();
                    if( size > 0 ) {
                        MY_LOGD_IF(1, "key %d, data %p, size %zu", key, data, size);
                        exif.sendCommand(CMD_REGISTER, key, reinterpret_cast<MUINTPTR>(&ID));
                        exif.sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MUINTPTR>(data), size);
                    }
                    else {
                        MY_LOGW("key %d with size %zu", key, size);
                    }
                }
                //
                dbgKey +=2;
                dbgVal +=2;
            }
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ExifJpegUtilsImpl::
isHwEncodeSupported(int const format) const
{
    FUNC_START;
    if( format == eImgFmt_YUY2 ||
        format == eImgFmt_NV12 ||
        format == eImgFmt_NV21 )
        return MTRUE;
    //
    FUNC_END;
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ExifJpegUtilsImpl::
hardwareOps_encode(
    encode_params& rParams
)
{
    FUNC_START;
    MUINT const groupUsage =
        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
    rParams.pSrc->lockBuf(getName(), groupUsage);
    //
    MY_LOGD( "src(%p:%p) %dx%d, dst(%p:%p) %dx%d, format = 0x%x",
             rParams.pSrc, (void *)rParams.pSrc->getBufVA(0),
             rParams.pSrc->getImgSize().w, rParams.pSrc->getImgSize().h,
             rParams.pDst, (void *)rParams.pDst->getBufVA(0),
             rParams.pDst->getImgSize().w, rParams.pDst->getImgSize().h,
             rParams.pDst->getImgFormat() );
    //
    MBOOL ret = MTRUE;
    //
    NSSImager::ISImager* pSImager = NSSImager::ISImager::createInstance(rParams.pSrc);
    if( pSImager == NULL ) {
        MY_LOGE("create SImage failed");
        return UNKNOWN_ERROR;
    }

    ret = pSImager->setTargetImgBuffer(rParams.pDst)

        && pSImager->setTransform(rParams.transform)

        && pSImager->setCropROI(rParams.crop)

        && pSImager->setEncodeParam(
                rParams.isSOI,
                rParams.quality,
                rParams.codecType,
                false
                )
        && pSImager->execute();

    pSImager->destroyInstance();
    pSImager = NULL;
    //
    rParams.pSrc->unlockBuf(getName());
    //
    if( !ret ) {
        MY_LOGE("encode failed");
        return UNKNOWN_ERROR;
    }
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
