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

#define LOG_TAG "MtkCam/JpegNode"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam3/pipeline/hwnode/JpegNode.h>
#include "BaseNode.h"

#include <thread>
#include <chrono>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <cutils/properties.h>
//
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
//
#include <vector>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/custom/ExifFactory.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
//
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <mtkcam/utils/exif/StdExif.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <unordered_map>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>

//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

#if (HWNODE_HAVE_AEE_FEATURE)
#include <aee.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_JPEG_NODE);
#ifdef AEE_ASSERT
#undef AEE_ASSERT
#endif
#define AEE_ASSERT(String) \
    do { \
        CAM_ULOGME("ASSERT("#String") fail"); \
        aee_system_exception( \
            LOG_TAG, \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String)
#endif


using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NSCam::Utils;
using namespace NSIoPipe;
using namespace std;
using namespace NSCam::TuningUtils;

/******************************************************************************
 *
 ******************************************************************************/
#define JPEGTHREAD_NAME ("Cam@Jpeg")
#define THUMBTHREAD_NAME ("Cam@JpegThumb")

#define JPEGTHREAD_POLICY     (SCHED_OTHER)
#define JPEGTHREAD_PRIORITY   (0)
//
#define ENABLE_DEBUG_INFO     (1)
#define ENABLE_PRERELEASE     (0)
#define DBG_BOUND_WIDTH       (320)
#define DBG_BOUND_HEIGH       (240)
#define JPEG_DUMP_PATH        "/sdcard/camera_dump/"

#define MTK_JPEG_NODE_PRE_RELEASE
//#ifdef MTK_JPEG_NODE_PRE_RELEASE
//  static bool kJpegNodePreRelease = true;
//#else
//  static bool kJpegNodePreRelease = false;
//#endif

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT((1), "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

#define EXIFAPP1_MAX_SIZE 65535 // 64K exif appn max data size
#define EXIFHEADER_ALIGN 128

static char filename[256] = {0}; // for file dump naming

/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}


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


/******************************************************************************
 *
 ******************************************************************************/
class JpegNodeImp
    : public BaseNode
    , public JpegNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class EncodeThread
        : public Thread
    {

    public:

                                    EncodeThread(JpegNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~EncodeThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

        JpegNodeImp*                  mpNodeImp;

    };

    class EncodeThumbThread
        : public Thread
    {

    public:

                                    EncodeThumbThread(JpegNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~EncodeThumbThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

        JpegNodeImp*                  mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    JpegNodeImp();

                                    ~JpegNodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &pFrame
                                    );

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.
    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;

    MVOID                           waitForRequestDrained();


    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer
                                    );

    MERROR                          getMetadataAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>& rpStreamBuffer,
                                        IMetadata*& rpOutMetadataResult
                                    );

    MVOID                           returnMetadataAndUnlock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>  rpStreamBuffer,
                                        IMetadata* rpOutMetadataResult,
                                        MBOOL success = MTRUE
                                    );

    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isInImageStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isHwEncodeSupported(int const format) const;

private:    ////                    to sync main yuv & thumbnail yuv

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
        MINT32                      flipMode;
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
                                        , flipMode(0)
                                    {}
    };

    class encode_frame : public virtual android::RefBase
    {
    public:
        sp<IPipelineFrame> const    mpFrame;
        MBOOL                       mbHasThumbnail;
        MBOOL                       mbSuccess;
        MBOOL                       mbBufValid;
        MINT8                      miJpegEncType;
        //
        jpeg_params                 mParams;
        //
        sp<IImageBuffer>            mpJpeg_Main;
        sp<IImageBuffer>            mpJpeg_Thumbnail;
        //
        StdExif                     exif;
        sp<IImageStreamBuffer>      mpOutImgStreamBuffer;
        sp<IImageBufferHeap>        mpOutImgBufferHeap;

        sp<IImageStreamBuffer>      mpInYuvImgStreamBuffer;
        sp<IImageStreamBuffer>      mpInThumbYuvImgStreamBuffer;
        sp<IMetaStreamBuffer>       mpInMetaStream_Request;
        sp<IMetaStreamBuffer>       mpInMetaStream_Hal;
        sp<IMetaStreamBuffer>       mpOutMetaStream_Result;
        buffer_handle_t             mGraphicBufferImportHandle;

        size_t                      thumbnailMaxSize;
                                    //
                                    encode_frame(
                                        sp<IPipelineFrame> const pFrame,
                                        MBOOL const hasThumbnail
                                        )
                                        : mpFrame(pFrame)
                                        , mbHasThumbnail(hasThumbnail)
                                        , mbSuccess(MTRUE)
                                        , mbBufValid(MTRUE)
                                        , miJpegEncType(-1)
                                        , mpJpeg_Main(NULL)
                                        , mpJpeg_Thumbnail(NULL)
                                        , mpOutImgStreamBuffer(NULL)
                                        , mpOutImgBufferHeap(NULL)
                                        , mpInYuvImgStreamBuffer(NULL)
                                        , mpInThumbYuvImgStreamBuffer(NULL)
                                        , mpInMetaStream_Request(NULL)
                                        , mpInMetaStream_Hal(NULL)
                                        , mpOutMetaStream_Result(NULL)
                                        , mGraphicBufferImportHandle(NULL)
                                        , thumbnailMaxSize(0)
                                    {}
    };

    typedef android::sp<encode_frame>                QueFrameNode_T;
    typedef android::List<QueFrameNode_T>            QueEncFrame_T;


    MVOID                           encodeThumbnail(
                                        sp<encode_frame>& pEncodeFrame
                                    );


    MVOID                           finalizeEncodeFrame(
                                        sp<encode_frame>& rpEncodeFrame
                                    );

    MVOID                           getJpegParams(
                                        IMetadata* pMetadata_request,
                                        jpeg_params& rParams
                                    ) const;

    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        sp<encode_frame>& rpEncodeFrame,
                                        sp<IImageBufferHeap>& rpImageBufferHeap
                                    );

    MERROR                          getThumbImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<encode_frame> const& rpEncodeFrame,
                                        sp<IImageBufferHeap> const& rpImageBufferHeap,
                                        sp<IImageBuffer>& rpImageBuffer /*out*/
                                    );

    MVOID                           updateMetadata(
                                        jpeg_params& rParams,
                                        IMetadata* pMetadata_result
                                    ) const;

    MERROR                          makeExifHeader(
                                        sp<encode_frame> rpEncodeFrame,
                                        MINT8 * const pOutExif,
                                        // [IN/OUT] in: exif buf size, out: exif header size
                                        size_t& rOutExifSize
                                    );

    MVOID                           updateStdExifParam(
                                        MBOOL const&              rNeedExifRotate,
                                        MSize const&                rSize,
                                        IMetadata* const            rpAppMeta,
                                        IMetadata* const            rpHalMeta,
                                        jpeg_params const&          rParams,
                                        ExifParams &                rStdParams
                                    ) const;

    MVOID                           updateStdExifParam_3A(
                                        IMetadata const&            rMeta,
                                        IMetadata const&            rAppMeta,
                                        ExifParams &                rStdParams
                                    ) const;

    MVOID                           updateStdExifParam_gps(
                                        IMetadata::IEntry const& rGpsCoordinates,
                                        IMetadata::IEntry const& rGpsProcessingMethod,
                                        IMetadata::IEntry const& rGpsTimestamp,
                                        ExifParams &        rStdParams
                                    ) const;

    MVOID                           updateDebugInfoToExif(
                                        IMetadata* const            rpHalMeta,
                                        StdExif &                   exif
                                    ) const;

    MUINT32                         calcZoomRatio(
                                        MRect      const&   cropRegion,
                                        MSize      const&   rSize
                                    ) const;

    MERROR                           errorHandle(
                                        sp<encode_frame>& rpEncodeFrame
                                    );

    MVOID                           unlockImage(
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        sp<IImageBuffer>& rpImageBuffer1
                                    );
    MERROR                      getStreamInfo(
                                        StreamId_T const streamId,
                                        android::sp<IPipelineFrame> const& pFrame,
                                        sp<IImageStreamInfo>& rpStreamInfo
                                     );

    MVOID                       dumpYUVBuffer(
                                        MUINT32 const frameNo,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MUINT32 const idx
                                    );

protected:  ////                    hw related

    class my_encode_params
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
    };

    MERROR                          hardwareOps_encode(
                                        my_encode_params& rParams
                                    );

protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMeta;
    sp<IMetaStreamInfo>             mpInHalMeta_capture;
    sp<IMetaStreamInfo>             mpInHalMeta_streaming;
    sp<IMetaStreamInfo>             mpInHalMeta;
    sp<IMetaStreamInfo>             mpOutMetaStreamInfo_Result;
    std::unordered_map<int, sp<IMetaStreamInfo>> mHalMetaMap;

    // image
    sp<IImageStreamInfo>            mpInYuv_main;
    sp<IImageStreamInfo>            mpInYuv_thumbnail;
    sp<IImageStreamInfo>            mpOutJpeg;
    sp<encode_frame>                mpEncodeFrame;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;
    MBOOL                           mbRequestDequeSuspend;
    Condition                       mbRequestDequeSuspendCond;
    QueEncFrame_T                   mEncFrameQueue;
    mutable Mutex                   mEncFrameQueueLock;

private:   ////                     Threads
    sp<EncodeThread>                mpEncodeThread;
    sp<EncodeThumbThread>           mpEncodeThumbThread;

private:
    mutable Mutex                   mEncodeLock;
    Condition                       mEncodeCond;
    MBOOL                           mbIsEncoding;
    MUINT32                         muCurFrameNo;
    //
    sp<encode_frame>                mpCurEncFrame;
    MUINT32                         muDumpBuffer;
    MINT32                          mFlip;
    MUINT32                         mInQueuePreReleaseRequestNum;
    MBOOL                           mIsPreReleaseRequest;
    MUINT32                         mPropPreReleaseMode;
    MBOOL                           mIsPreReleaseEnable;

private: // static infos

    MUINT8                          muFacing; // ref: MTK_LENS_FACING_
    MRect                           mActiveArray;
    MBOOL                           mJpegRotationEnable;
    MINT32                          mLogLevel;
    MBOOL                           mThumbDoneFlag;
    MBOOL                           mDbgInfoEnable;
    MINT32                          mUniqueKey;
    MINT32                          mFrameNumber;
    MINT32                          mRequestNumber;
#if ENABLE_PRERELEASE
    sp<ITimeline>                   mpTimeline;
    MUINT16                         mTimelineCounter;
#endif

};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<JpegNode>
JpegNode::
createInstance()
{
    return new JpegNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
JpegNodeImp::
JpegNodeImp()
    : BaseNode()
    , JpegNode()
    //
    , mConfigRWLock()
    , mpInHalMeta(nullptr)
    //
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpEncodeThread(NULL)
    //
    , mbIsEncoding(MFALSE)
    , muCurFrameNo(0)
    //
    , mpCurEncFrame(NULL)
    //
    , muFacing(0)
    , mJpegRotationEnable(MFALSE)
    , mThumbDoneFlag(MFALSE)
    , mUniqueKey(-1)
    , mFrameNumber(-1)
    , mRequestNumber(-1)
    , mInQueuePreReleaseRequestNum(0)
    , mIsPreReleaseRequest(MFALSE)
    , mIsPreReleaseEnable(MFALSE)
    , mbRequestDequeSuspend(MFALSE)
#if ENABLE_PRERELEASE
    , mpTimeline(ITimeline::create("Timeline::Jpeg"))
    , mTimelineCounter()
    , mpEncodeFrame(NULL)
#endif
{
    mNodeName = "JpegNode";//default name
    MINT32 enable = ::property_get_int32("vendor.jpeg.rotation.enable", 1);
    mJpegRotationEnable = (enable & 0x1)? MTRUE : MFALSE;
    MY_LOGD_IF(mJpegRotationEnable, "Jpeg Rotation enable");

    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.JpegNode", 0);
    }
    MINT32 forceDbg;
#if     (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 3)
        forceDbg = 1;  // for ENG build
#elif   (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 2)
        forceDbg = 1;  // for USERDEBUG build
#else
        forceDbg = 0;  // for USER build
#endif
    mDbgInfoEnable= ::property_get_int32("vendor.debug.camera.dbginfo", forceDbg);
    muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.JpegNode", 0);
    mFlip = ::property_get_int32("vendor.debug.camera.Jpeg.flip", 0);
    mPropPreReleaseMode = ::property_get_int32("vendor.debug.camera.bgservice.mode", 0);

    if (mPropPreReleaseMode == 0) {  //depends VendorTag
        mIsPreReleaseEnable = MFALSE;
    } else if (mPropPreReleaseMode == 1) { //force enable
        mIsPreReleaseEnable = MTRUE;
    } else if (mPropPreReleaseMode == 2) { //force disable
        mIsPreReleaseEnable = MFALSE;
    }

}


/******************************************************************************
 *
 ******************************************************************************/
JpegNodeImp::
~JpegNodeImp()
{
    MY_LOGI("");
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
init(InitParams const& rParams)
{
    CAM_TRACE_NAME("JpegNode init");
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpEncodeThread = new EncodeThread(this);
    if( mpEncodeThread->run(JPEGTHREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }
    //
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }

        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( !tryGetMetadata<MRect>(
                    &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                    mActiveArray)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }

        if( !tryGetMetadata<MUINT8>(
                    &static_meta, MTK_SENSOR_INFO_FACING,
                    muFacing)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_FACING");
            return UNKNOWN_ERROR;
        }

        MY_LOGD_IF(1,"active array(%d, %d, %dx%d), facing %d",
                mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
                muFacing);
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
config(ConfigParams const& rParams)
{
    CAM_TRACE_NAME("JpegNode config");
    FUNC_START;
    CHECK_ERROR( verifyConfigParams(rParams) );
    //
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta       = rParams.pInAppMeta;
        mpInHalMeta_capture       = rParams.pInHalMeta_capture;
        mpInHalMeta_streaming     = rParams.pInHalMeta_streaming;
        if (mpInHalMeta_capture) {
            mHalMetaMap.emplace(std::make_pair(mpInHalMeta_capture->getStreamId(), mpInHalMeta_capture));
            MY_LOGD("debug capture InHalMeta streamId : %#" PRIx64 "", mpInHalMeta_capture->getStreamId());
        }
        if (mpInHalMeta_streaming) {
            mHalMetaMap.emplace(std::make_pair(mpInHalMeta_streaming->getStreamId(), mpInHalMeta_streaming));
            MY_LOGD("debug streaming InHalMeta streamId : %#" PRIx64 "", mpInHalMeta_streaming->getStreamId());
        }
        mpOutMetaStreamInfo_Result = rParams.pOutAppMeta;
        MY_LOGD("debug InOutMeta streamId : %#" PRIx64 "", mpOutMetaStreamInfo_Result->getStreamId());
        // image
        mpInYuv_main      = rParams.pInYuv_Main;
        mpInYuv_thumbnail = rParams.pInYuv_Thumbnail;
        mpOutJpeg         = rParams.pOutJpeg;
        //
        if (mPropPreReleaseMode == 0) {  //depends VendorTag
            mIsPreReleaseEnable = rParams.bIsPreReleaseEnable;
        }
        MY_LOGD("mPropPreReleaseMode %d, bIsPreReleaseEnable %d, mIsPreReleaseEnable:%d !",
                    mPropPreReleaseMode, rParams.bIsPreReleaseEnable,mIsPreReleaseEnable);
    }
    if(mpInYuv_main != NULL){
        MY_LOGD("mpInYuv_main:%dx%d", mpInYuv_main->getImgSize().w, mpInYuv_main->getImgSize().h);
    }
    if(mpInYuv_thumbnail != NULL){
        MY_LOGD("mpInYuv_thumbnail:%dx%d", mpInYuv_thumbnail->getImgSize().w, mpInYuv_thumbnail->getImgSize().h);
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpEncodeThread->requestExit();
    // join
    mpEncodeThread->join();
    //
    mpEncodeThread = NULL;
    mpEncodeThumbThread = NULL;
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
flush(
    android::sp<IPipelineFrame> const& pFrame
)
{
    if (mIsPreReleaseEnable) {

        if( ! pFrame.get() ) {
            MY_LOGE("Null frame");
            return BAD_VALUE;
        }

        MY_LOGD("FrameNo : %d, RequestNo : %d", pFrame->getFrameNo(), pFrame->getRequestNo());

        //queue ppFrame if not error
        IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
        sp<IImageStreamBuffer> rpStreamBuffer = rStreamBufferSet.getImageBuffer(mpInYuv_main->getStreamId(), getNodeId());
        if  ( rpStreamBuffer == 0 ) {
            MY_LOGD("[frame:%d node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", pFrame->getFrameNo(), getNodeId(), mpInYuv_main->getStreamId());
            return NAME_NOT_FOUND;
        }
        //  Check buffer status.
        if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
            //  The producer ahead of this user may fail to render this buffer's content.
            MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d", pFrame->getFrameNo(), getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
            //rpStreamBuffer = 0;
            //return BAD_VALUE;
            return BaseNode::flush(pFrame);
        }
        {
            Mutex::Autolock _l(mRequestQueueLock);
            if (mbRequestDequeSuspend == MFALSE) {
                MY_LOGD("flush frame set mbRequestDequeSuspend!");
                mbRequestDequeSuspend = MTRUE;
            }
        }
        return this->queue(pFrame);
    }
    return BaseNode::flush(pFrame);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
flush()
{
    FUNC_START;
#define MARK_USER_STATUS(sBufSet, streamBuffer, nId, status)   \
        do {                                                \
            sBufSet.markUserStatus(                         \
                 streamBuffer->getStreamId(),               \
                 nId,                                       \
                 status                                     \
                 );                                         \
        }while(0)

    if (!mIsPreReleaseEnable) {
        //
        // 1. clear requests
        {
            Mutex::Autolock _l(mRequestQueueLock);
            //
            Que_T::iterator it = mRequestQueue.begin();
            while ( it != mRequestQueue.end() ) {
                BaseNode::flush(*it);
                it = mRequestQueue.erase(it);
            }
        }
        //
        // 2. wait enque thread
        waitForRequestDrained();

    } else {
        //
        // pre-release step
        // 1. suspend deque request thread
        {
            Mutex::Autolock _l(mRequestQueueLock);
            if (mbRequestDequeSuspend == MFALSE) {
                MY_LOGD("flush set mbRequestDequeSuspend!");
                mbRequestDequeSuspend = MTRUE;
            }
        }
        //
        // 2. wait enque thread finish
        waitForRequestDrained();
        //
        // 3. process enque buffer
        // a) get stream buffer
        // b) mark in/out buffer status as used and pre-release
        // c) apply & dispatch pre-release frame
        {
            Mutex::Autolock _l(mRequestQueueLock);

            MUINT32 index = 0;
            Que_T::iterator it = mRequestQueue.begin();
            for (; it != mRequestQueue.end(); index++, it++) {

                if (index < mInQueuePreReleaseRequestNum) {
                    continue;  //skip already dispatch preRelease frame
                }

                auto& pFrame(*it);
                mInQueuePreReleaseRequestNum++;
                MY_LOGD("mark PreRelease FrameNo : %d, RequestNo : %d, PreReleaseNum : %d",
                          pFrame->getFrameNo(), pFrame->getRequestNo(), mInQueuePreReleaseRequestNum);
                //1. get stream buffer

                //2. mark in/out buffer status as pre-release
                IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
                auto status = IUsersManager::UserStatus::PRE_RELEASE;
                              /*IUsersManager::UserStatus::USED | */

                MBOOL useThumbnail = MFALSE;

                {
                    IPipelineFrame::InfoIOMapSet IOMapSet;
                    if(
                            OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
                            || IOMapSet.mImageInfoIOMapSet.size() != 1
                            || IOMapSet.mMetaInfoIOMapSet.size() != 1
                      ) {
                        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %zu/%zu",
                                IOMapSet.mImageInfoIOMapSet.size(),
                                IOMapSet.mMetaInfoIOMapSet.size()
                                );
                        //return OK;
                    }

                    {
                        IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
                        for (size_t i = 0; i < metaIOMap.vIn.size(); i++) {
                            StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
                            auto search = mHalMetaMap.find((int)streamId);
                            if (search != mHalMetaMap.end()) {
                                MY_LOGD("StreamId : %#" PRIx64 "", streamId);
                                mpInHalMeta = search->second;
                                break;
                            }
                        }
                        if (mpInHalMeta == nullptr)
                        {
                            MY_LOGE("PipelineContext doesn't setup input hal meta");
                            //return OK;
                        }
                    }

                    // query if use thumbnail
                    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
                    for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
                    {
                        StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
                        if( isStream(mpInYuv_thumbnail, streamId) ) {
                            useThumbnail = MTRUE;
                            MY_LOGD("need Thumbnail!");
                            break;
                        }
                    }
                }


                sp<encode_frame> pEncodeFrameNode = new encode_frame(pFrame, useThumbnail);

                //enque encode_frame
                mEncFrameQueue.push_back(pEncodeFrameNode);


                // src main YUV
                {
                    sp<IImageStreamBuffer> rpStreamBuffer;
                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInYuv_main->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpInYuvImgStreamBuffer /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureImageBufferAvailable_ (InYUV) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpInYuv_main->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInYuv_main, getNodeId(), status);
                }

                // src thumbnail YUV
                if(useThumbnail == MTRUE) {

                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInYuv_thumbnail->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpInThumbYuvImgStreamBuffer /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureImageBufferAvailable_ (InThumbYUV) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpInYuv_thumbnail->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInYuv_thumbnail, getNodeId(), status);
                }

                // dst bistream buffer
                {
                    auto& pImageBufferHeap(pEncodeFrameNode->mpOutImgBufferHeap);
                    sp<IGraphicImageBufferHeap> pGraphicImageBufferHeap = NULL;
                    auto& graphicBufferHandle(pEncodeFrameNode->mGraphicBufferImportHandle);
                    sp<IImageBufferHeap> pHeap = NULL;
                    MBOOL bIsNeedRepackGraphicBuffer = MFALSE;

                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpOutJpeg->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpOutImgStreamBuffer /*rpStreamBuffer*/
                            );
                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureImageBufferAvailable_ (Out) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                        //return err;
                    }
                    //import buffer handle if graphic buffer
                    pImageBufferHeap = pEncodeFrameNode->mpOutImgStreamBuffer->tryWriteLock(getNodeName());

                    if (pImageBufferHeap != NULL) {
                        pGraphicImageBufferHeap = IGraphicImageBufferHeap::castFrom(pImageBufferHeap.get());
                    }

                    if (pGraphicImageBufferHeap != NULL) {

                        auto helper = IGrallocHelper::singleton();
                        graphicBufferHandle = *const_cast<buffer_handle_t*>(pGraphicImageBufferHeap->getBufferHandlePtr());

                        if (bIsNeedRepackGraphicBuffer == MTRUE) {
                            if  ( !helper
                               || !helper->importBuffer(graphicBufferHandle)
                               )
                            {
                                MY_LOGE_IF( 1, "importBuffer (Out) fail, FrameNo:%d, StreamId %d!!",
                                               pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                            } else {
                                MY_LOGD_IF(mLogLevel, "GraphicImageBuffer importBuffer success, FrameNo:%d, StreamId %d!!",
                                               pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                                {
                                    int acquire_fence = -1;
                                    int release_fence = -1;
                                    std::string bufferName = "";

                                    sp<IImageStreamInfo> pStreamInfo = const_cast<IImageStreamInfo*>(pEncodeFrameNode->mpOutImgStreamBuffer->getStreamInfo());
                                    pHeap = IGraphicImageBufferHeap::create(
                                            (bufferName + ":" + pStreamInfo->getStreamName()).c_str(),
                                            pStreamInfo->getUsageForAllocator(),
                                            pStreamInfo->getImgFormat() == eImgFmt_BLOB ? MSize(pStreamInfo->getAllocBufPlanes()[0].rowStrideInBytes, 1): pStreamInfo->getImgSize(),
                                            pStreamInfo->getImgFormat() == eImgFmt_JPEG ? eImgFmt_JPEG : pStreamInfo->getAllocImgFormat(),
                                            &graphicBufferHandle,
                                            acquire_fence,
                                            release_fence
                                        );

                                   MY_LOGD_IF( 1, "GraphicImageBuffer repack done, FrameNo:%d, StreamId %d!!",
                                                  pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                                }
                            }
                        } else {
                            MY_LOGW_IF( 1, "Out Stream GraphicImageBuffer repack is disable, FrameNo:%d, StreamId %d!!",
                                           pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                        }

                    } else {
                        MY_LOGW_IF( 1, "Out Stream is not GraphicImageBuffer, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                    }


                    pEncodeFrameNode->mpOutImgStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
                    if (pHeap != NULL) {
                        pImageBufferHeap = pHeap;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpOutJpeg, getNodeId(), status);
                }

                // In App metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInAppMeta->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpInMetaStream_Request /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureMetaBufferAvailable_ (InApp) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpInAppMeta->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInAppMeta, getNodeId(), status);
                }

                // In Hal Metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInHalMeta->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpInMetaStream_Hal /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureMetaBufferAvailable_ (InHal) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpInHalMeta->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInHalMeta, getNodeId(), status);
                }

                // Out Metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpOutMetaStreamInfo_Result->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->mpOutMetaStream_Result /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE_IF( 1, "ensureMetaBufferAvailable_ (out) fail, FrameNo:%d, StreamId %d!!",
                                       pFrame->getFrameNo(), mpOutMetaStreamInfo_Result->getStreamId());
                        //return err;
                    }

                    {
                        if  ( pEncodeFrameNode->mpOutMetaStream_Result == NULL ) {
                            MY_LOGE("[frame:%u node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", pFrame->getFrameNo(), getNodeId(),
                                                                                    mpOutMetaStreamInfo_Result->getStreamId());
                            //return NAME_NOT_FOUND;
                        } else {
                            pEncodeFrameNode->mpOutMetaStream_Result->markStatus(STREAM_BUFFER_STATUS::ERROR);
                        }
                    }

                    MARK_USER_STATUS(streamBufferSet, mpOutMetaStreamInfo_Result, getNodeId(), status);
                }

                //3. apply pre-release
                streamBufferSet.applyPreRelease(getNodeId());
                MY_LOGD("applyPreRelease FrameNo : %d, RequestNo : %d, PreReleaseNum : %d",
                          pFrame->getFrameNo(), pFrame->getRequestNo(), mInQueuePreReleaseRequestNum);
                //4. onDispatchFrame
                onDispatchFrame(pFrame);

            }
        }
        //
        // 4. resume deque request thread
        {
            Mutex::Autolock _l(mRequestQueueLock);
            mbRequestDequeSuspend = MFALSE;
            MY_LOGD("flush mbRequestDequeSuspendCond signal!");
            mbRequestDequeSuspendCond.signal();
        }
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    MY_LOGD("FrameNo : %d, RequestNo : %d", pFrame->getFrameNo(), pFrame->getRequestNo());

    Mutex::Autolock _l(mRequestQueueLock);
    // TODO: handle main & thumnail yuvs are not queued in the same time
    //if( !mbIsEncoding || (mbIsEncoding && pFrame->getFrameNo() != muCurFrameNo) {
        //  Make sure the request with a smaller frame number has a higher priority.
        Que_T::iterator it = mRequestQueue.end();
        for (; it != mRequestQueue.begin(); ) {
            --it;
            if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
                ++it;   //insert(): insert before the current node
                break;
            }
        }
        mRequestQueue.insert(it, pFrame);
    //}
    //else {

    //}
    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    FUNC_START;
    Mutex::Autolock _l(mRequestQueueLock);

    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.broadcast();
        //
        MY_LOGD_IF(mLogLevel, "mRequestQueue.size:%zu wait+", mRequestQueue.size());
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        MY_LOGD_IF(mLogLevel, "mRequestQueue.size:%zu wait-", mRequestQueue.size());
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }

    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }

    if (mIsPreReleaseEnable) {

        if (mbRequestDequeSuspend == MTRUE) {
            // set dained flag
            mbRequestDrained = MTRUE;
            MY_LOGD("mbRequestDrainedCond broadcast!");
            mbRequestDrainedCond.broadcast();
            MY_LOGD("mRequestQueue.size:%zu mbRequestDequeSuspendCond wait+", mRequestQueue.size());
            status_t status = mbRequestDequeSuspendCond.wait(mRequestQueueLock);
            MY_LOGD("mRequestQueue.size:%zu mbRequestDequeSuspendCond wait-", mRequestQueue.size());
            if  ( OK != status ) {
                MY_LOGW(
                    "mbRequestDequeSuspendCond wait status:%d:%s, mRequestQueue.size:%zu",
                    status, ::strerror(-status), mRequestQueue.size()
                );
            }
        }

        if ( mInQueuePreReleaseRequestNum > 0 ) {
          mIsPreReleaseRequest = MTRUE;
          mInQueuePreReleaseRequestNum--;

          mpEncodeFrame = *mEncFrameQueue.begin();
          mEncFrameQueue.erase(mEncFrameQueue.begin());

        } else {
          mIsPreReleaseRequest = MFALSE;
        }
    } else {
        mIsPreReleaseRequest = MFALSE;
    }

    //  Here the queue is not empty, take the first request from the queue.
    if (mIsPreReleaseRequest == MFALSE) {
        mbRequestDrained = MFALSE;
    } else {
        // pre-release flow
        // only first flush need to wait current frame done,
        // the rest flush don't need to wait.
        mbRequestDrained = MTRUE;
    }
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
encodeThumbnail(
    sp<encode_frame>& pEncodeFrame
)
{
    if(pEncodeFrame == NULL)
    {
        MY_LOGE("thumb encode frame is null");
        return;
    }
    sp<IPipelineFrame> const pFrame = pEncodeFrame->mpFrame;
    if( pEncodeFrame->mpJpeg_Thumbnail.get() == NULL )
    {
        MY_LOGW("thumb imagebuffer is null");
        return;
    }
    // to encode thumbnail
    // try get yuv for thumb jpeg
    IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
    StreamId_T const       stream_in            = mpInYuv_thumbnail->getStreamId();
    sp<IImageStreamBuffer>& pInImageStreamBuffer = pEncodeFrame->mpInThumbYuvImgStreamBuffer;//NULL;
    sp<IImageBuffer>       pInImageBuffer       = NULL;

    MERROR const err = getImageBufferAndLock(
            pFrame,
            stream_in,
            pInImageStreamBuffer,
            pInImageBuffer
            );
    if( err != OK ) {
        MY_LOGE("getImageBufferAndLock(InImageStreamBuffer) err = %d", err);
        //unlockImage(pEncodeFrame->mpOutImgStreamBuffer, pEncodeFrame->mpJpeg_Main, pEncodeFrame->mpJpeg_Thumbnail);
        //errorHandle(pEncodeFrame);
        //pEncodeFrame = NULL;
        pEncodeFrame->mbBufValid = MFALSE;
        return;
    }
    if(mLogLevel>=2)
        dumpYUVBuffer(pFrame->getFrameNo(), pInImageBuffer, 1);
    //
    MSize thumbsize = pEncodeFrame->mParams.size_thumbnail;
    // do encode
    {
        my_encode_params params;
        params.pSrc = pInImageBuffer.get();
        params.pDst = pEncodeFrame->mpJpeg_Thumbnail.get();
        //MUINT32 transform = pInImageStreamBuffer->getStreamInfo()->getTransform(); //TODO
        if (mpEncodeFrame->mParams.flipMode || mFlip) {
            if( pEncodeFrame->mParams.orientation == 90)
            {
                params.transform = eTransform_ROT_90 | eTransform_FLIP_V;
                thumbsize = MSize(thumbsize.h, thumbsize.w);
            } else if (pEncodeFrame->mParams.orientation == 180)
            {
                params.transform = eTransform_FLIP_V;
            } else if (pEncodeFrame->mParams.orientation == 270)
            {
                params.transform = eTransform_ROT_90 | eTransform_FLIP_H;
                thumbsize = MSize(thumbsize.h, thumbsize.w);
            } else
            {
                params.transform = eTransform_FLIP_H;
            }
        } else {
            if( pEncodeFrame->mParams.orientation == 90)
            {
                params.transform = eTransform_ROT_90;
                thumbsize = MSize(thumbsize.h, thumbsize.w);
            } else if (pEncodeFrame->mParams.orientation == 180)
            {
                params.transform = eTransform_ROT_180;
            } else if (pEncodeFrame->mParams.orientation == 270)
            {
                params.transform = eTransform_ROT_270;
                thumbsize = MSize(thumbsize.h, thumbsize.w);
            } else
            {
                params.transform = 0;
            }
        }

        params.crop = calCropAspect(pInImageBuffer->getImgSize(), thumbsize);
        params.isSOI = 1;
        params.quality = pEncodeFrame->mParams.quality_thumbnail;
        params.codecType = NSSImager::JPEGENC_SW;
        //
        MINT32 quality = params.quality;
        while(quality > 0) {
            MERROR const err = hardwareOps_encode(params);
            if( err != OK ) {
                MY_LOGE("thumb encode fail src %p, fmt 0x%x, dst %zx, fmt 0x%x", params.pSrc, params.pSrc->getImgFormat(), params.pDst->getBufVA(0), params.pDst->getImgFormat());
                pEncodeFrame->mbSuccess = MFALSE;
            } else {
                if (mpEncodeFrame->thumbnailMaxSize < params.pDst->getBitstreamSize()) {
                    if (params.pDst->getBitstreamSize() > (mpEncodeFrame->thumbnailMaxSize + mpEncodeFrame->exif.getDbgExifSize())) {
                        MY_LOGE("Thumbnail over encode! encode bitstreamSize");
                    } else {
                        MY_LOGW("Thumbnail bitStream size is too big, scale down quality and re-encode again!");
                        quality -= 10;
                        if (quality > 0) {
                            params.quality = quality;
                        }
                        continue;
                    }
                }
            }
            break;
        }
        if (quality <= 0 || !pEncodeFrame->mbSuccess) {
            MY_LOGE("Thumbnail encode fail!");
        }
    }
    pInImageBuffer->unlockBuf(getNodeName());
    pInImageStreamBuffer->unlock(getNodeName(), pInImageBuffer->getImageBufferHeap());
    //
    if (mIsPreReleaseRequest == MFALSE) {
        streamBufferSet.markUserStatus(
                pInImageStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    } else {
        pInImageStreamBuffer->markUserStatus(
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    }

}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if(
            OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
            || IOMapSet.mImageInfoIOMapSet.size() != 1
            || IOMapSet.mMetaInfoIOMapSet.size() != 1
      ) {
        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %zu/%zu",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
                );
        return;
    }

    {
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
        for (size_t i = 0; i < metaIOMap.vIn.size(); i++) {
            StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
            auto search = mHalMetaMap.find((int)streamId);
            if (search != mHalMetaMap.end()) {
                MY_LOGD("StreamId : %#" PRIx64 "", streamId);
                mpInHalMeta = search->second;
                break;
            }
        }
        if (mpInHalMeta == nullptr)
        {
            MY_LOGE("PipelineContext doesn't setup input hal meta");
            return;
        }
    }

    {
        MBOOL useThumbnail = MFALSE;

        // query if use thumbnail
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
        for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
            if( isStream(mpInYuv_thumbnail, streamId) ) {
                useThumbnail = MTRUE;
                MY_LOGD("need Thumbnail!");
                break;
            }
        }

        if(mIsPreReleaseRequest == MFALSE) {
            // new frame
            mpEncodeFrame = NULL;
            mpEncodeFrame = new encode_frame(pFrame, useThumbnail);
        }


        if( !mpEncodeFrame.get() )
        {
            MY_LOGE("mpEncodeFrame is NULL");
            return;
        }
        // get jpeg params
        {
            IMetadata* pInMeta_Request = NULL;
            sp<IMetaStreamBuffer>& pInMetaStream_Request = mpEncodeFrame->mpInMetaStream_Request;//NULL;
            MERROR err = getMetadataAndLock(
                    pFrame,
                    mpInAppMeta->getStreamId(),
                    pInMetaStream_Request,
                    pInMeta_Request
                    );

            if( err != OK ) {
                MY_LOGE("getMetadataAndLock err = %d", err);
                errorHandle(mpEncodeFrame);
                mpEncodeFrame = NULL;
                return;
            }

            getJpegParams(pInMeta_Request, mpEncodeFrame->mParams);

            // get HAL meta
            IMetadata* pInMeta_Hal = NULL;
            sp<IMetaStreamBuffer>& pInMetaStream_Hal = mpEncodeFrame->mpInMetaStream_Hal;//NULL;;//NULL;

            err = getMetadataAndLock(
                    pFrame,
                    mpInHalMeta->getStreamId(),
                    pInMetaStream_Hal,
                    pInMeta_Hal
                    );
            //
            if( err != OK ) {
                MY_LOGE("getMetadataAndLock(pInMetaStream_Hal) err = %d", err);
                errorHandle(mpEncodeFrame);
                mpEncodeFrame = NULL;
                return;
            }
            {
                MUINT8 encodeType;
                if( tryGetMetadata<MUINT8>(pInMeta_Hal, MTK_JPG_ENCODE_TYPE, encodeType) ) {
                    mpEncodeFrame->miJpegEncType = encodeType;
                    MY_LOGD("Assign encode type manually.(%d)", mpEncodeFrame->miJpegEncType);
                }
            }
            //determine exif need rotate
            sp<IImageStreamInfo> pYUVStreamInfo = NULL;
            if (mIsPreReleaseRequest == MFALSE) {
                if( OK != getStreamInfo(mpInYuv_main->getStreamId(), pFrame, pYUVStreamInfo) ){
                    errorHandle(mpEncodeFrame);
                    mpEncodeFrame = NULL;
                    MY_LOGE("getStreamInfo fail");
                    return;
                }
            } else {
                pYUVStreamInfo = const_cast<IImageStreamInfo*>(mpEncodeFrame->mpInYuvImgStreamBuffer->getStreamInfo());
            }
            MUINT32 transform = pYUVStreamInfo->getTransform();
            MBOOL needExifRotate = MTRUE;

            if((mpEncodeFrame->mParams.orientation == 90  &&  transform & eTransform_ROT_90)  ||
               (mpEncodeFrame->mParams.orientation == 270  &&  transform & eTransform_ROT_270) ||
               (mpEncodeFrame->mParams.orientation == 180  &&  transform & eTransform_ROT_180)
              )
                needExifRotate = MFALSE;

            MSize imageSize = MSize(pYUVStreamInfo->getImgSize().w, pYUVStreamInfo->getImgSize().h);
            ExifParams stdParams;
            //
            // update standard exif params
            updateStdExifParam(
                    needExifRotate,
                    imageSize,
                    pInMeta_Request,
                    pInMeta_Hal,
                    mpEncodeFrame->mParams,
                    stdParams
                    );

            // check thumbnail size
            if (!mpEncodeFrame->mParams.size_thumbnail.w || !mpEncodeFrame->mParams.size_thumbnail.h) {
                MY_LOGD("App meta is not set thumbnail size, check request streamInfo size");
                sp<IImageStreamInfo> pThumbnailStreamInfo = NULL;
                if (mIsPreReleaseRequest == MFALSE) {
                    if (OK != getStreamInfo(mpInYuv_thumbnail->getStreamId(), pFrame, pThumbnailStreamInfo)) {
                        MY_LOGW("getThumbnailStreamInfo fail, hasThumbnail :%d", mpEncodeFrame->mbHasThumbnail);
                        mpEncodeFrame->mbHasThumbnail = MFALSE;
                    }
                } else { //pre-release flow
                    if (mpEncodeFrame->mpInThumbYuvImgStreamBuffer.get() != NULL) {
                       pThumbnailStreamInfo = const_cast<IImageStreamInfo*>(mpEncodeFrame->mpInThumbYuvImgStreamBuffer->getStreamInfo());
                    }
                }

                if (pThumbnailStreamInfo.get() != NULL) {
                    if (pThumbnailStreamInfo->getImgSize().w && pThumbnailStreamInfo->getImgSize().h) {
                        mpEncodeFrame->mParams.size_thumbnail = pThumbnailStreamInfo->getImgSize();
                        if ( mJpegRotationEnable ) {
                            if( mpEncodeFrame->mParams.orientation == 90 || mpEncodeFrame->mParams.orientation == 270)
                            {
                                MINT32 tmp = mpEncodeFrame->mParams.size_thumbnail.w;
                                mpEncodeFrame->mParams.size_thumbnail.w = mpEncodeFrame->mParams.size_thumbnail.h;
                                mpEncodeFrame->mParams.size_thumbnail.h = tmp;
                            }
                            MY_LOGD_IF(0,"@@getJpegParams thumb size(w,h)=(%dx%d)",
                                    mpEncodeFrame->mParams.size_thumbnail.w, mpEncodeFrame->mParams.size_thumbnail.h);
                        }
                    } else {
                        MY_LOGW("Thumbnail size is not set!");
                        mpEncodeFrame->mbHasThumbnail = MFALSE;
                    }
                } else {
                        MY_LOGW("Thumbnail size is not set!");
                        mpEncodeFrame->mbHasThumbnail = MFALSE;
                }
            }
            //set common exif debug info
            MINT32 uniqueKey = 0, frameNumber = 0, requestNumber = 0, freeMemoryMBytes = 0;
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_FRAME_NUMBER, frameNumber);
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_REQUEST_NUMBER, requestNumber);
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_FEATURE_FREE_MEMORY_MBYTE, freeMemoryMBytes);
            std::map<MUINT32, MUINT32> debugInfoList;
            {
                using namespace dbg_cam_common_param_1;
                debugInfoList[CMN_TAG_VERSION] = CMN_DEBUG_TAG_VERSION_DP;//tag version : sub version(high 2 byte) | major version(low 2 byte)
                debugInfoList[CMN_TAG_PIPELINE_UNIQUE_KEY] = uniqueKey;
                debugInfoList[CMN_TAG_PIPELINE_FRAME_NUMBER] = frameNumber;
                debugInfoList[CMN_TAG_PIPELINE_REQUEST_NUMBER] = requestNumber;
                debugInfoList[CMN_TAG_SYS_FREE_MEMORY_MBYTES] = freeMemoryMBytes;
            }
            IMetadata exifMetadata;
            tryGetMetadata<IMetadata>(pInMeta_Hal, MTK_3A_EXIF_METADATA, exifMetadata);
            if (DebugExifUtils::setDebugExif(
                    DebugExifUtils::DebugExifType::DEBUG_EXIF_CAM,
                    static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_KEY),
                    static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_DATA),
                    debugInfoList,
                    &exifMetadata) == nullptr)
            {
                MY_LOGW("set debug exif to metadata fail");
            }
            //

            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_UNIQUE_KEY, mUniqueKey);
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_FRAME_NUMBER, mFrameNumber);
            tryGetMetadata<MINT32>(pInMeta_Hal, MTK_PIPELINE_REQUEST_NUMBER, mRequestNumber);
            MINT32 bound = DBG_BOUND_WIDTH * DBG_BOUND_HEIGH;
            if(imageSize.w * imageSize.h > bound) {
                mpEncodeFrame->exif.init(stdParams, mDbgInfoEnable);
                if(mDbgInfoEnable)
                    updateDebugInfoToExif(&exifMetadata, mpEncodeFrame->exif);

                MY_LOGD_IF(mLogLevel, "init (%dx%d)",
                    imageSize.w, imageSize.h);
            } else {
                mpEncodeFrame->exif.init(stdParams, 0);
                MY_LOGD_IF(mLogLevel, "skip init (%dx%d)",
                    imageSize.w, imageSize.h);
            }

            if (muDumpBuffer) {
                FILE_DUMP_NAMING_HINT hint;
                hint.UniqueKey          = mUniqueKey;
                hint.FrameNo            = mFrameNumber;
                hint.RequestNo          = mRequestNumber;
                MBOOL res = MTRUE;
                res = extract(&hint, pInMeta_Hal);
                if (!res) {
                    MY_LOGW("[DUMP_JPG] extract with metadata fail (%d)", res);
                }
                genFileName_JPG(filename, sizeof(filename), &hint, nullptr);
                MY_LOGD("enable muDumpBuffer FileName[%s]", filename);
            }

            returnMetadataAndUnlock(
                mpEncodeFrame->mpFrame,
                mpInHalMeta->getStreamId(),
                pInMetaStream_Hal,
                pInMeta_Hal
            );

            returnMetadataAndUnlock(
                pFrame,
                mpInAppMeta->getStreamId(),
                pInMetaStream_Request,
                pInMeta_Request
            );

            //set thumbnail max size & thumbnail size need to be 128 alignment
            size_t &thumbMaxSize = mpEncodeFrame->thumbnailMaxSize;

            if( mpEncodeFrame->mbHasThumbnail )
            {
                thumbMaxSize = (mpEncodeFrame->mParams.size_thumbnail.w) * (mpEncodeFrame->mParams.size_thumbnail.h) * 18 / 10;

                size_t thumbnailSize = 0;
                if ((EXIFAPP1_MAX_SIZE - mpEncodeFrame->exif.getStdExifSize()) < thumbMaxSize) {
                    thumbnailSize = EXIFAPP1_MAX_SIZE - mpEncodeFrame->exif.getStdExifSize();
                    size_t res = thumbnailSize % EXIFHEADER_ALIGN;
                    if( res != 0 )
                        thumbnailSize = thumbnailSize - res;
                } else {
                    thumbnailSize = thumbMaxSize;
                    size_t res = thumbnailSize % EXIFHEADER_ALIGN;
                    if (res != 0) {
                         // prevent it would exceed EXIFAPP1_MAX_SIZE after doing thumbnail size 128 alignemt
                        if(thumbnailSize + EXIFHEADER_ALIGN > EXIFAPP1_MAX_SIZE) {
                            thumbnailSize -= res;
                        } else {
                            thumbnailSize = thumbnailSize + EXIFHEADER_ALIGN -res;
                        }
                    }
                }

                thumbMaxSize = thumbnailSize;

            }

            size_t headerSize = mpEncodeFrame->exif.getStdExifSize() + mpEncodeFrame->exif.getDbgExifSize() + thumbMaxSize;
            if(headerSize % EXIFHEADER_ALIGN != 0)
                MY_LOGW("not aligned header size %zu", headerSize);

            mpEncodeFrame->exif.setMaxThumbnail(thumbMaxSize);

        }
        //get out main imagebuffer
        {
            //IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
            StreamId_T const       stream_in            = mpOutJpeg->getStreamId();
            sp<IImageStreamBuffer>& pOutImgStreamBuffer = mpEncodeFrame->mpOutImgStreamBuffer;
            sp<IImageBufferHeap>&   pImageBufferHeap    = mpEncodeFrame->mpOutImgBufferHeap;
            sp<IImageBuffer>       pOutImageBuffer      = NULL;
            //
            MERROR const err = getImageBufferAndLock(
                    pFrame,
                    stream_in,
                    pOutImgStreamBuffer,
                    pOutImageBuffer,
                    mpEncodeFrame,
                    pImageBufferHeap
                    );
            if( err != OK ) {
                MY_LOGE("getImageBufferAndLock(OutImageBuffer) err = %d", err);
                errorHandle(mpEncodeFrame);
                mpEncodeFrame = NULL;
                return;
            }
            //remember main buffer
            mpEncodeFrame->mpJpeg_Main = pOutImageBuffer;
        }


        //get thumb image buffer and run thumb thread
        if( mpEncodeFrame->mbHasThumbnail )
        {
            //IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
            StreamId_T const       stream_in            = mpOutJpeg->getStreamId();
            sp<IImageBuffer>       pOutImageBuffer      = NULL;
            sp<IImageBufferHeap>&  pImageBufferHeap     = mpEncodeFrame->mpOutImgBufferHeap;

            MERROR const err = getThumbImageBufferAndLock(
                                pFrame,
                                stream_in,
                                mpEncodeFrame,
                                pImageBufferHeap,
                                pOutImageBuffer
                        );

            if( err != OK ) {
                MY_LOGE("getImageBufferAndLock err = %d", err);
                errorHandle(mpEncodeFrame);
                mpEncodeFrame = NULL;
                return;
            }
            //remember main&thumb buffer(sp<IImageBuffer>)
            mpEncodeFrame->mpJpeg_Thumbnail = pOutImageBuffer;
            //check thumb image buffer offset
            MY_LOGD("[out]frame(%d) heapVA:0x%zx, main/thum imgbufferVA: 0x%zx/0x%zx, heap size: %zu",
                            pFrame->getFrameNo(),
                            pImageBufferHeap->getBufVA(0),
                            mpEncodeFrame->mpJpeg_Main->getBufVA(0),
                            mpEncodeFrame->mpJpeg_Thumbnail->getBufVA(0),
                            pImageBufferHeap->getBufSizeInBytes(0)
                            );
            mThumbDoneFlag = MFALSE;
            //
            mpEncodeThumbThread = new EncodeThumbThread(this);
            if( mpEncodeThumbThread->run(THUMBTHREAD_NAME) != OK ) {
                errorHandle(mpEncodeFrame);
                mpEncodeFrame = NULL;
                return;
            }
        }
    }

    // 2. get src buffers & internal dst buffer for bitstream
    while( mpEncodeFrame->mpJpeg_Main.get() != NULL ) {
        // main jpeg is not encoded, try get yuv for main jpeg
        IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
        StreamId_T const       stream_in            = mpInYuv_main->getStreamId();
        sp<IImageStreamBuffer>& pInImageStreamBuffer = mpEncodeFrame->mpInYuvImgStreamBuffer; //NULL;
        sp<IImageBuffer>       pInImageBuffer       = NULL;
        //
        MERROR const err = getImageBufferAndLock(
                pFrame,
                stream_in,
                pInImageStreamBuffer,
                pInImageBuffer
                );
        if( err != OK ) {
            MY_LOGE("getImageBufferAndLock(in main YUV) err = %d", err);
            //unlockImage(mpEncodeFrame->mpOutImgStreamBuffer, mpEncodeFrame->mpJpeg_Main, mpEncodeFrame->mpJpeg_Thumbnail);
            mpEncodeFrame->mbBufValid = MFALSE;
            break;
        }
        if(mLogLevel>=2)
            dumpYUVBuffer(pFrame->getFrameNo(), pInImageBuffer, 0);
        // do encode
        {
            MINT8 iEncTypeCheck = isHwEncodeSupported(pInImageBuffer->getImgFormat()) ?
                    NSSImager::JPEGENC_HW_FIRST : NSSImager::JPEGENC_SW;
            my_encode_params params;
            params.pSrc = pInImageBuffer.get();
            params.pDst = mpEncodeFrame->mpJpeg_Main.get();
            params.transform = 0;
            params.crop = MRect(MPoint(0,0), pInImageBuffer->getImgSize());
            params.isSOI = 0;
            params.quality = mpEncodeFrame->mParams.quality;
            if(-1 != mpEncodeFrame->miJpegEncType &&
                NSSImager::JPEGENC_HW_FIRST == iEncTypeCheck){
                iEncTypeCheck = NSSImager::JPEGENC_HW_ONLY;
            }
            params.codecType = iEncTypeCheck;
            //
            MERROR const err = hardwareOps_encode(params);
            if( err != OK ) {
                mpEncodeFrame->mbSuccess = MFALSE;
            }
        }
        //
        pInImageBuffer->unlockBuf(getNodeName());
        pInImageStreamBuffer->unlock(getNodeName(), pInImageBuffer->getImageBufferHeap());
        //
        if (mIsPreReleaseRequest == MFALSE) {
            streamBufferSet.markUserStatus(
                    pInImageStreamBuffer->getStreamInfo()->getStreamId(),
                    getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        } else {
            pInImageStreamBuffer->markUserStatus(
                    getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        }
        // 3. end
        {
            size_t const totalJpegSize =
                mpEncodeFrame->mpJpeg_Main->getBitstreamSize() + mpEncodeFrame->exif.getHeaderSize();
            mpEncodeFrame->mpJpeg_Main->getImageBufferHeap()->setBitstreamSize(totalJpegSize);
        }
        break;
    }

    // 4. if no thumbnail, copy to dst buffer & release buffers/metadata
    //    else add to pending list to wait for the other src buffer
    if(
            // condition 1: without thumbnail
            (!mpEncodeFrame->mbHasThumbnail && mpEncodeFrame->mpJpeg_Main.get()) ||
            // condition 2: with thumbnail
            (mpEncodeFrame->mbHasThumbnail && mpEncodeFrame->mpJpeg_Main.get() && mpEncodeFrame->mpJpeg_Thumbnail.get() )
      ) {
        {
            Mutex::Autolock _l(mEncodeLock);
            if( mThumbDoneFlag != MTRUE)
            {
                MY_LOGD("waiting thumbnail encoding done+");
                mEncodeCond.wait(mEncodeLock);
                MY_LOGD("waiting thumbnail encoding done-");
            }
            else
                MY_LOGD_IF(mLogLevel, "enc done and go on...");
        }
        if(mpEncodeFrame->mbBufValid != MTRUE)
        {
            unlockImage(mpEncodeFrame->mpOutImgStreamBuffer,
                            mpEncodeFrame->mpJpeg_Main,
                            mpEncodeFrame->mpJpeg_Thumbnail);
            errorHandle(mpEncodeFrame);
        }
        else
        {
            mpEncodeFrame->mpJpeg_Main->unlockBuf(getNodeName());
            if( mpEncodeFrame->mpJpeg_Thumbnail.get() )
                mpEncodeFrame->mpJpeg_Thumbnail->unlockBuf(getNodeName());
            finalizeEncodeFrame(mpEncodeFrame);
        }
        //
        mpEncodeFrame = NULL;
    }


    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
verifyConfigParams(
    ConfigParams const & rParams
) const
{
    if  ( ! rParams.pInAppMeta.get() ) {
        MY_LOGE("no in app meta");
        return BAD_VALUE;
    }
    //if  ( ! rParams.pInHalMeta.get() ) {
    //    MY_LOGE("no in hal meta");
    //    return BAD_VALUE;
    //}
    if  ( ! rParams.pOutAppMeta.get() ) {
        MY_LOGE("no out app meta");
        return BAD_VALUE;
    }
    if  (  NULL == rParams.pInYuv_Main.get() ) {
        MY_LOGE("no in hal main yuv image");
        return BAD_VALUE;
    }
    //if  (  NULL == rParams.pInYuv_Thumbnail.get() ) {
    //    MY_LOGE("no in hal thumbnail yuv image");
    //    return BAD_VALUE;
    //}

    if  ( ! rParams.pOutJpeg.get() ) {
        MY_LOGE("no out hal jpeg image");
        return BAD_VALUE;
    }
    //
    MY_LOGD_IF(1,
            "stream: [meta] in app %#" PRIx64 ", out app %#" PRIx64,
            rParams.pInAppMeta->getStreamId(),
            rParams.pOutAppMeta->getStreamId()
            );
    MY_LOGD_IF( rParams.pInHalMeta_capture.get(),
            "stream: [meta] in hal capture %#" PRIx64,
            rParams.pInHalMeta_capture->getStreamId()
            );
    MY_LOGD_IF( rParams.pInHalMeta_streaming.get(),
            "stream: [meta] in hal streaming %#" PRIx64,
            rParams.pInHalMeta_streaming->getStreamId());
    MY_LOGD_IF(rParams.pInYuv_Main.get(),
            "stream: [img] in main %#" PRIx64,
            rParams.pInYuv_Main->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Thumbnail.get(),
            "stream: [img] in thumbnail %#" PRIx64,
            rParams.pInYuv_Thumbnail->getStreamId()
            );
    MY_LOGD_IF(rParams.pOutJpeg.get(),
            "stream: [img] out jpeg %#" PRIx64,
            rParams.pOutJpeg->getStreamId()
            );
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
waitForRequestDrained()
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    if (rpStreamBuffer.get() == NULL) {
        MERROR const err = ensureImageBufferAvailable_(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );

        if( err != OK )
            return err;
    }

    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());
    MY_LOGD_IF(mLogLevel, "getImageBufferAndLock try R/W Lock + ");
#ifdef MTK_JPEG_NODE_PRE_RELEASE
    pImageBufferHeap = isInImageStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName(), true) :
        rpStreamBuffer->tryWriteLock(getNodeName());
#else
    pImageBufferHeap = isInImageStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName()) :
        rpStreamBuffer->tryWriteLock(getNodeName());
#endif
    MY_LOGD_IF(mLogLevel, "getImageBufferAndLock try R/W Lock - ");
    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        rpStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    MBOOL ret = rpImageBuffer->lockBuf(getNodeName(), groupUsage);
    if (!ret) {
        AEE_ASSERT("lockBuffer failed, please check VA usage!");
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel, "stream buffer: (%#" PRIxPTR ") %p, heap: %p, buffer: %p, usage: %u",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getMetadataAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& rpStreamBuffer,
    IMetadata*& rpMetadata
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    MY_LOGD_IF(mLogLevel>=2, "nodeID %#" PRIxPTR " streamID %#" PRIxPTR " ",getNodeId(), streamId);
    if (rpStreamBuffer.get() == NULL) {
        MERROR const err = ensureMetaBufferAvailable_(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );

        if( err != OK )
        {
            MY_LOGD_IF( rpStreamBuffer == NULL, "streamId(%#" PRIxPTR ") meta streamBuf not exit", streamId);
            return err;
        }
    }
    MY_LOGD_IF(mLogLevel, "getMetadataAndLock try R/W Lock + ");
#ifdef MTK_JPEG_NODE_PRE_RELEASE
    rpMetadata = isInMetaStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName(), true) :
        rpStreamBuffer->tryWriteLock(getNodeName());
#else
    rpMetadata = isInMetaStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName()) :
        rpStreamBuffer->tryWriteLock(getNodeName());
#endif
    MY_LOGD_IF(mLogLevel, "getMetadataAndLock try R/W Lock - ");

    if( rpMetadata == NULL ) {
        MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                pFrame->getFrameNo(), getNodeId(), rpStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel,"stream %#" PRIx64 ": stream buffer %p, metadata: %p",
        streamId, rpStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
returnMetadataAndUnlock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>  rpStreamBuffer,
    IMetadata* rpMetadata,
    MBOOL success
)
{
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    if( rpStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %#" PRIx64 ": rpStreamBuffer == NULL",
                streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( !isInMetaStream(streamId) ) {
        if  ( success ) {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        }
        else {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }
    //
    if( rpMetadata )
        rpStreamBuffer->unlock(getNodeName(), rpMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    if (mIsPreReleaseRequest == MFALSE) {
        rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    } else {
        rpStreamBuffer->markUserStatus(
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta, streamId) || isStream(mpInHalMeta, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isInImageStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    //
    if( isStream(mpInYuv_main, streamId) )
        return MTRUE;
    //
    if( isStream(mpInYuv_thumbnail, streamId) )
        return MTRUE;
    //
    MY_LOGD_IF(1, "stream id %#" PRIx64 " is not in-stream", streamId);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
finalizeEncodeFrame(
    sp<encode_frame>& rpEncodeFrame
)
{
    sp<IPipelineFrame> const& pFrame = rpEncodeFrame->mpFrame;
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();


    // update metadata
    {
        IMetadata* pOutMeta_Result = NULL;
        sp<IMetaStreamBuffer>& pOutMetaStream_Result = rpEncodeFrame->mpOutMetaStream_Result;//NULL;

        MERROR const err __unused = getMetadataAndLock(
                pFrame,
                mpOutMetaStreamInfo_Result->getStreamId(),
                pOutMetaStream_Result,
                pOutMeta_Result
                );

        updateMetadata(rpEncodeFrame->mParams, pOutMeta_Result);

        returnMetadataAndUnlock(
                pFrame,
                mpOutMetaStreamInfo_Result->getStreamId(),
                pOutMetaStream_Result,
                pOutMeta_Result,
                rpEncodeFrame->mbSuccess
                );
    }

    // get out buffer
    {
        sp<IImageStreamBuffer> pOutImgStreamBuffer = rpEncodeFrame->mpOutImgStreamBuffer;//
        sp<IImageBuffer>       pOutImageBuffer     = NULL;

        pOutImageBuffer = rpEncodeFrame->mpOutImgBufferHeap->createImageBuffer_FromBlobHeap(0, rpEncodeFrame->exif.getHeaderSize());

        if (pOutImageBuffer.get() == NULL) {
            MY_LOGE("rpImageBuffer == NULL");
        }
        MUINT const groupUsage = pOutImgStreamBuffer->queryGroupUsage(getNodeId());
        MBOOL ret = pOutImageBuffer->lockBuf(getNodeName(), groupUsage);
        if (!ret) {
            AEE_ASSERT("lockBuffer failed, please check VA usage!");
            return;
        }

        size_t exifSize  = 0;
        MINT8 * pExifBuf = reinterpret_cast<MINT8*>(pOutImageBuffer->getBufVA(0));
        if( pExifBuf == NULL
        || OK != makeExifHeader(rpEncodeFrame, pExifBuf, exifSize)
           ) {
            rpEncodeFrame->mbSuccess = MFALSE;
            MY_LOGE("frame %u make exif header failed: buf %p, size %zu",
                    rpEncodeFrame->mpFrame->getFrameNo(),
                    pExifBuf, exifSize);
        }
        if( pExifBuf )
            pExifBuf = NULL;

        //
        pOutImageBuffer->unlockBuf(getNodeName());
        if (mIsPreReleaseRequest == MFALSE) {
            pOutImgStreamBuffer->unlock(getNodeName(), pOutImageBuffer->getImageBufferHeap());
        }
        pOutImgStreamBuffer->markStatus(
                rpEncodeFrame->mbSuccess ?
                STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                );

        //  Mark this buffer as USED by this user.
        //  Mark this buffer as RELEASE by this user.
        if (mIsPreReleaseRequest == MFALSE) {
            streamBufferSet.markUserStatus(
                    pOutImgStreamBuffer->getStreamInfo()->getStreamId(),
                    getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        } else {
            pOutImgStreamBuffer->markUserStatus(
                    getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        }
    }

    if(muDumpBuffer) {
        sp<IImageStreamBuffer> pStreamBuffer =
                                rpEncodeFrame->mpOutImgStreamBuffer;
        int jpeg_size = rpEncodeFrame->exif.getHeaderSize() +
                                rpEncodeFrame->mpJpeg_Main->getBitstreamSize();
        sp<IImageBuffer> dumpImgBuffer =
                                rpEncodeFrame->mpOutImgBufferHeap->createImageBuffer_FromBlobHeap(0, jpeg_size);
        if(dumpImgBuffer.get() == NULL) {
            MY_LOGE("dumpBuffer == NULL");
            return;
        }

        MUINT groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
        groupUsage |= eBUFFER_USAGE_SW_READ_OFTEN;
        MBOOL ret = dumpImgBuffer->lockBuf(getNodeName(), groupUsage);
        if (!ret) {
            AEE_ASSERT("lockBuffer failed, please check VA usage!");
            return;
        }

        if(!NSCam::Utils::makePath(JPEG_DUMP_PATH, 0660))
            MY_LOGI("makePath[%s] fails", JPEG_DUMP_PATH);

        MBOOL rets = dumpImgBuffer->saveToFile(filename);
        MY_LOGI("[DUMP_JPG] SaveFile[%s]:(%d)", filename, rets);

        dumpImgBuffer->unlockBuf(getNodeName());

    }

    //free graphic buffer
    if (mIsPreReleaseRequest == MTRUE) {
        auto& graphicBufferHandle(rpEncodeFrame->mGraphicBufferImportHandle);

        if (graphicBufferHandle != NULL) {

            if  ( auto helper = IGrallocHelper::singleton()) {

                //buffer_handle_t* bufferHandle = const_cast<buffer_handle_t*>(pGraphicImageBufferHeap->getBufferHandlePtr());
                helper->freeBuffer(graphicBufferHandle);
                MY_LOGD_IF( 1, "GraphicImageBuffer freebuffer !!");
            } else {
                MY_LOGE_IF( 1, "GraphicImageBuffer get helper fail!!");
            }
        } else {
            MY_LOGW_IF( 1, "Out Stream is not GraphicImageBuffer, skip unmap graphic buffer!!");
        }
    }


    //
    // release
    if (mIsPreReleaseRequest == MFALSE) {
        streamBufferSet.applyRelease(getNodeId());
    #if ENABLE_PRERELEASE
        MY_LOGD("jpeg node-release SB");
        mpTimeline->inc(1);
    #endif
        onDispatchFrame(pFrame);
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
getJpegParams(
    IMetadata* pMetadata_request,
    jpeg_params& rParams
) const
{
    if( NULL == pMetadata_request)
    {
        MY_LOGE("pMetadata_request=NULL");
    }
    rParams.gpsCoordinates =
        pMetadata_request->entryFor(MTK_JPEG_GPS_COORDINATES);

    rParams.gpsProcessingMethod =
        pMetadata_request->entryFor(MTK_JPEG_GPS_PROCESSING_METHOD);

    rParams.gpsTimestamp =
        pMetadata_request->entryFor(MTK_JPEG_GPS_TIMESTAMP);

#define getParam(meta, tag, type, param)                \
    do {                                                \
        if( !tryGetMetadata<type>(meta, tag, param) ) { \
            MY_LOGW("no tag: %s", #tag);                \
        }                                               \
    } while(0)
#define getAppParam(tag, type, param) getParam(pMetadata_request, tag, type, param)

    // request from app
    getAppParam(MTK_JPEG_ORIENTATION      , MINT32, rParams.orientation);
    getAppParam(MTK_JPEG_QUALITY          , MUINT8, rParams.quality);
    getAppParam(MTK_JPEG_THUMBNAIL_QUALITY, MUINT8, rParams.quality_thumbnail);
    getAppParam(MTK_JPEG_THUMBNAIL_SIZE   , MSize , rParams.size_thumbnail);
    getAppParam(MTK_SCALER_CROP_REGION    , MRect , rParams.cropRegion);
    getAppParam(MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE  , MINT32, rParams.flipMode);

#undef getAppParam
#undef getParam
    if ( mJpegRotationEnable ) {
        if( rParams.orientation == 90 || rParams.orientation == 270)
        {
            MINT32 tmp = rParams.size_thumbnail.w;
            rParams.size_thumbnail.w = rParams.size_thumbnail.h;
            rParams.size_thumbnail.h = tmp;
        }
        MY_LOGD_IF(0,"@@getJpegParams thumb size(w,h)=(%dx%d)",
            rParams.size_thumbnail.w, rParams.size_thumbnail.h);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
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
JpegNodeImp::
makeExifHeader(
    sp<encode_frame> rpEncodeFrame,
    MINT8 * const pOutExif,
    size_t& rOutExifSize // [IN/OUT] in: exif buf size, out: exif header size
)
{
    MERROR ret;
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
JpegNodeImp::
updateStdExifParam(
    MBOOL const&              rNeedExifRotate,
    MSize const&                rSize,
    IMetadata* const            rpAppMeta,
    IMetadata* const            rpHalMeta,
    jpeg_params const&          rParams,
    ExifParams &                rStdParams
) const
{
    rStdParams.u4ImageWidth  = rSize.w;
    rStdParams.u4ImageHeight = rSize.h;
    //
    // 3A
    if( rpHalMeta && rpAppMeta) {
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
            updateStdExifParam_3A(exifMeta, *rpAppMeta, rStdParams);
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta or app meta");
    }
    // gps
    updateStdExifParam_gps(
            rParams.gpsCoordinates, rParams.gpsProcessingMethod, rParams.gpsTimestamp,
            rStdParams
            );
    // icc profile
    if( rpHalMeta ) {
        MINT32 iccIdx = -1;
        if( !tryGetMetadata<MINT32>(rpHalMeta, MTK_ISP_COLOR_SPACE, iccIdx) )
        {
            MY_LOGW("no tag: MTK_ISP_COLOR_SPACE");
        }
        else {
          if(iccIdx==MTK_ISP_COLOR_SPACE_SRGB)
              rStdParams.u4ICCIdx = EXIF_ICC_PROFILE_SRGB;
          else if (iccIdx==MTK_ISP_COLOR_SPACE_DISPLAY_P3)
              rStdParams.u4ICCIdx = EXIF_ICC_PROFILE_DCI_P3;
          else
              MY_LOGW("not support isp profile in MTK_ISP_COLOR_SPACE %d ", iccIdx);
        }
    }
    // others
    if ( !rNeedExifRotate )
        rStdParams.u4Orientation = 22;
    else
        rStdParams.u4Orientation = rParams.orientation;
    rStdParams.u4ZoomRatio   = calcZoomRatio(rParams.cropRegion, rSize);
    rStdParams.u4Facing      = (muFacing == MTK_LENS_FACING_BACK) ? 0 : 1;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateStdExifParam_3A(
    IMetadata const&            rMeta,
    IMetadata const&            rAppMeta,
    ExifParams &                rStdParams
) const
{
#define getParam(meta, tag, type, param)                      \
            do {                                              \
                type val = -1;                                 \
                if( !tryGetMetadata<type>(meta, tag, val) ) { \
                    MY_LOGW("no tag: %s", #tag);              \
                }                                             \
                param = val;                                  \
            } while(0)
#if 1
            // from result meta of 3A
            // for Hal3 yuv reprocessing [must need!]
            getParam(&rMeta, MTK_3A_EXIF_FNUMBER,             MINT32, rStdParams.u4FNumber); /**/
            if (rStdParams.u4FNumber == -1) {
                MFLOAT fNumber = 0.0f;
                getParam(&rAppMeta, MTK_LENS_APERTURE, MFLOAT, fNumber);
                rStdParams.u4FNumber = fNumber*10;
                MY_LOGD("miss in Hal find APP MTK_LENS_APERTURE : %d", rStdParams.u4FNumber);
            }
            getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH,        MINT32, rStdParams.u4FocalLength); /**/
            if (rStdParams.u4FocalLength == -1) {
                MFLOAT focalLength = 0.0f;
                getParam(&rAppMeta, MTK_LENS_FOCAL_LENGTH, MFLOAT, focalLength);
                rStdParams.u4FocalLength = focalLength*1000;
                MY_LOGD("miss in Hal find APP MTK_LENS_FOCAL_LENGTH : %d", rStdParams.u4FocalLength);
            }
            getParam(&rMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   MINT32, rStdParams.u4CapExposureTime); /**/
            if (rStdParams.u4CapExposureTime == -1) {
                MINT64 capExposure = 0;
                getParam(&rAppMeta, MTK_SENSOR_EXPOSURE_TIME, MINT64, capExposure);
                rStdParams.u4CapExposureTime = (MINT32)(capExposure/1000);
                MY_LOGD("miss in Hal find APP MTK_3A_EXIF_CAP_EXPOSURE_TIME : %" PRId32, rStdParams.u4CapExposureTime);
            }
            getParam(&rMeta, MTK_3A_EXIF_AE_ISO_SPEED,        MINT32, rStdParams.u4AEISOSpeed); /**/
            if (rStdParams.u4AEISOSpeed == -1) {
                getParam(&rAppMeta, MTK_SENSOR_SENSITIVITY, MINT32, rStdParams.u4AEISOSpeed);
                MY_LOGD("miss in Hal find APP MTK_SENSOR_SENSITIVITY : %d", rStdParams.u4AEISOSpeed);
            }

            //
            getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH_35MM,   MINT32, rStdParams.u4FocalLength35mm);
            getParam(&rMeta, MTK_3A_EXIF_AWB_MODE,            MINT32, rStdParams.u4AWBMode);
            getParam(&rMeta, MTK_3A_EXIF_LIGHT_SOURCE,        MINT32, rStdParams.u4LightSource);
            getParam(&rMeta, MTK_3A_EXIF_EXP_PROGRAM,         MINT32, rStdParams.u4ExpProgram);
            getParam(&rMeta, MTK_3A_EXIF_SCENE_CAP_TYPE,      MINT32, rStdParams.u4SceneCapType);
            getParam(&rMeta, MTK_3A_EXIF_FLASH_LIGHT_TIME_US, MINT32, rStdParams.u4FlashLightTimeus);
            getParam(&rMeta, MTK_3A_EXIF_AE_METER_MODE,       MINT32, rStdParams.u4AEMeterMode);
            getParam(&rMeta, MTK_3A_EXIF_AE_EXP_BIAS,         MINT32, rStdParams.i4AEExpBias);
#endif

#undef getParam
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
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
MVOID
JpegNodeImp::
updateDebugInfoToExif(
    IMetadata* const            pExifMeta,
    StdExif &                   exif
) const
{
    if( pExifMeta == NULL )
    {
        MY_LOGW("pExifMeta is NULL, update debug info to exif fail");
        return;
    }
    MUINT32 dbgKey = MTK_3A_EXIF_DEBUGINFO_BEGIN;
    MUINT32 dbgVal = MTK_3A_EXIF_DEBUGINFO_BEGIN + 1;
    while( dbgVal < MTK_3A_EXIF_DEBUGINFO_END ) {
        MINT32 key;
        IMetadata::Memory dbgmem;
        if( tryGetMetadata<MINT32>(pExifMeta, dbgKey, key) &&
                tryGetMetadata<IMetadata::Memory>(pExifMeta, dbgVal, dbgmem)
          )
        {
            MINT32 ID;
            void* data = static_cast<void*>(dbgmem.editArray());
            size_t size = dbgmem.size();
            if( size > 0 ) {
                MY_LOGD_IF(mLogLevel, "key 0x%x, data %p, size %zu", key, data, size);
                exif.sendCommand(CMD_REGISTER, key, reinterpret_cast<MUINTPTR>(&ID));
                exif.sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MUINTPTR>(data), size);
            }
            else {
                MY_LOGW("key 0x%x with size %zu", key, size);
            }
        }
        //
        dbgKey +=2;
        dbgVal +=2;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
JpegNodeImp::
calcZoomRatio(
    MRect      const&   cropRegion,
    MSize      const&   rSize
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    MUINT32 zoomRatio = 100;
    if( ! mpOutJpeg.get() ) {
        MY_LOGW("jpeg stream is not configured");
        return 100;
    }

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
JpegNodeImp::
hardwareOps_encode(
    my_encode_params& rParams
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    MY_LOGD("+ src: %dx%d, dst: %dx%d, format: 0x%x, crop(%d,%d;%d,%d)",rParams.pSrc-> getImgSize().w, rParams.pSrc-> getImgSize().h,
                                                     rParams.pDst-> getImgSize().w, rParams.pDst-> getImgSize().h,
                                                     rParams.pDst->getImgFormat(),
                                                     rParams.crop.p.x, rParams.crop.p.y,
                                                     rParams.crop.s.w, rParams.crop.s.h
                                                     );

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
    if( !ret ) {
        MY_LOGE("encode failed");
        return UNKNOWN_ERROR;
    }
    //
    MY_LOGD("- bistream size %zu. Check bit: 0x%x 0x%x", rParams.pDst->getBitstreamSize(),
                    (*(reinterpret_cast<MUINT8*>(rParams.pDst->getBufVA(0)))),
                    *(reinterpret_cast<MUINT8*>(rParams.pDst->getBufVA(0))+1));
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (JPEGTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, JPEGTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, JPEGTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = JPEGTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, JPEGTHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), JPEGTHREAD_POLICY, JPEGTHREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
JpegNodeImp::EncodeThread::
requestExit()
{
    FUNC_START;
    //TODO: refine this
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
JpegNodeImp::EncodeThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
JpegNodeImp::EncodeThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            !exitPending()
        &&  OK == mpNodeImp->onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        mpNodeImp->mThumbDoneFlag = MTRUE;
        mpNodeImp->onProcessFrame(pFrame);
        return true;
    }

    MY_LOGD("exit encode thread %d", mpNodeImp->mThumbDoneFlag);
    return  false;

}
/******************************************************************************
 *
 ******************************************************************************/
void
JpegNodeImp::EncodeThumbThread::
requestExit()
{
FUNC_START;FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
JpegNodeImp::EncodeThumbThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
JpegNodeImp::EncodeThumbThread::
threadLoop()
{
    mpNodeImp->encodeThumbnail(mpNodeImp->mpEncodeFrame);
    {
        Mutex::Autolock _l(mpNodeImp->mEncodeLock);
        mpNodeImp->mThumbDoneFlag = MTRUE;
        mpNodeImp->mEncodeCond.signal();
    }
    MY_LOGD_IF(mpNodeImp->mLogLevel, "exit thumb encode thread");
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getThumbImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<encode_frame> const& rpEncodeFrame,
    sp<IImageBufferHeap> const& rpImageBufferHeap,
    sp<IImageBuffer>& rpImageBuffer /*out*/
)
{
    sp<IImageStreamInfo>   pStreamInfo = pFrame->getStreamInfoSet().getImageInfoFor(streamId);

    if(rpImageBufferHeap == NULL) {
        MY_LOGE("heap not exist");
        return BAD_VALUE;
    }

    //get thumb IImageBuffer
    size_t thumbnailMaxSize = rpEncodeFrame->thumbnailMaxSize;
    size_t thumbnailOffset  = rpEncodeFrame->exif.getStdExifSize();

    size_t const bufStridesInBytes[3] = {thumbnailMaxSize, 0 ,0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};
    // ref v1 prepare heap & imagebuffer
    IImageBufferAllocator::ImgParam imgParam =
                            IImageBufferAllocator::ImgParam(
                                rpImageBufferHeap->getImgFormat(),//blob
                                MSize(
                                    rpEncodeFrame->mParams.size_thumbnail.w,
                                    rpEncodeFrame->mParams.size_thumbnail.h
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
    /*IImageBuffer* tempBuffer = pHeap->createImageBuffer();
    tempBuffer->incStrong(tempBuffer);
    tempBuffer->lockBuf(
                    LOG_TAG,
                    eBUFFER_USAGE_SW_READ_OFTEN);*/
    //rpImageBuffer = pHeap->createImageBuffer();
    rpImageBuffer = pHeap->createImageBuffer_FromBlobHeap(
                                0,
                                eImgFmt_JPEG,
                                rpEncodeFrame->mParams.size_thumbnail,
                                bufStridesInBytes
                                );
    MUINT const groupUsage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
    MBOOL ret = rpImageBuffer->lockBuf(
                    getNodeName(), groupUsage
                    );
    if (!ret) {
        AEE_ASSERT("lockBuffer failed, please check VA usage!");
        return BAD_VALUE;
    }

    if (NULL == rpImageBuffer.get()) {
        MY_LOGE("rpImageThumbnailBuffer == NULL");
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel, "thumb stream buffer(%#" PRIxPTR "), heap(0x%x): %p, buffer: %p, usage: %x, heapVA: %zx, bufferVA: %zx",
            streamId, rpImageBufferHeap->getImgFormat(), rpImageBufferHeap.get(), rpImageBuffer.get(), groupUsage,
            rpImageBufferHeap->getBufVA(0), rpImageBuffer->getBufVA(0));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getStreamInfo(
    StreamId_T const streamId,
    android::sp<IPipelineFrame> const& pFrame,
    sp<IImageStreamInfo>& rpStreamInfo)
{
    IStreamBufferSet&         rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageStreamBuffer> rpStreamBuffer    = NULL;


    MERROR const err = ensureImageBufferAvailable_(
                    pFrame->getFrameNo(),
                    streamId,
                    rStreamBufferSet,
                    rpStreamBuffer
                    );
    if( err != OK )
        return err;

    rpStreamInfo = const_cast<IImageStreamInfo*>(rpStreamBuffer->getStreamInfo());

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    sp<encode_frame>& rpEncodeFrame,
    sp<IImageBufferHeap>& rpImageBufferHeap
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();

    if(NULL == rpImageBufferHeap.get()) {

        if(NULL == rpStreamBuffer.get()) {
            MERROR const err = ensureImageBufferAvailable_(
                    pFrame->getFrameNo(),
                    streamId,
                    rStreamBufferSet,
                    rpStreamBuffer
                    );
            if( err != OK )
                return err;
        }

        // pre-release
#if ENABLE_PRERELEASE
        if (streamId != 0) {
            MY_LOGD("test prerelease flow start w/ timeline counter: %d", mTimelineCounter);
            //prepare timeline & release fence
            sp<IFence> release_fence;
            int sync_fence_fd = mpTimeline->createFence("RF_Jpeg", ++mTimelineCounter);
            rStreamBufferSet.setUserReleaseFence(streamId, getNodeId(), sync_fence_fd);
            //
            if ( rpStreamBuffer.get() ) {
                rStreamBufferSet.markUserStatus(
                        streamId,
                        getNodeId(),
                        IUsersManager::UserStatus::USED | IUsersManager::UserStatus::PRE_RELEASE
                        );
                rStreamBufferSet.applyPreRelease(getNodeId());
            }
        }
#endif
        MY_LOGD_IF(mLogLevel, "getImageBufferAndLock(out) try R/W Lock + ");
#ifdef MTK_JPEG_NODE_PRE_RELEASE
        rpImageBufferHeap = isInImageStream(streamId) ?
            rpStreamBuffer->tryReadLock(getNodeName(), true) :
            rpStreamBuffer->tryWriteLock(getNodeName());
#else
        rpImageBufferHeap = isInImageStream(streamId) ?
            rpStreamBuffer->tryReadLock(getNodeName()) :
            rpStreamBuffer->tryWriteLock(getNodeName());
#endif
        MY_LOGD_IF(mLogLevel, "getImageBufferAndLock(out) try R/W Lock - ");
        if (NULL == rpImageBufferHeap.get()) {
            MY_LOGE("rpImageBufferHeap is NULL");
            return BAD_VALUE;
        }
    }

    //
    {
        sp<IImageStreamInfo> pYUVStreamInfo = NULL;
        if (mIsPreReleaseRequest == MFALSE) {
            getStreamInfo(mpInYuv_main->getStreamId(), pFrame, pYUVStreamInfo);
        } else { //pre-release flow
            pYUVStreamInfo = const_cast<IImageStreamInfo*>(mpEncodeFrame->mpInYuvImgStreamBuffer->getStreamInfo());
        }
        size_t mainOffset = rpEncodeFrame->exif.getHeaderSize();
        size_t mainMaxSize = rpImageBufferHeap->getBufSizeInBytes(0) - mainOffset;


        MUINT32 transform = pYUVStreamInfo->getTransform();
        MSize imageSize = MSize(pYUVStreamInfo->getImgSize().w, pYUVStreamInfo->getImgSize().h);

        size_t const bufStridesInBytes[3] = {mainMaxSize, 0 ,0};
        rpImageBuffer = rpImageBufferHeap->createImageBuffer_FromBlobHeap(
                mainOffset, eImgFmt_JPEG, imageSize, bufStridesInBytes
                );
        if (NULL == rpImageBuffer.get()) {
            rpStreamBuffer->unlock(getNodeName(), rpImageBufferHeap.get());
            MY_LOGE("rpImageMainBuffer is NULL");
            return BAD_VALUE;
        }
        //  Query the group usage.
        MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());
        MBOOL ret = rpImageBuffer->lockBuf(getNodeName(), groupUsage);
        if (!ret) {
            AEE_ASSERT("lockBuffer failed, please check VA usage!");
            return BAD_VALUE;
        }

    MY_LOGD("stream buffer(%#" PRIxPTR ") %p, heap(0x%x): %p, buffer: %p, usage: %x, trans:%d, ori:%d, heapVA: %zx, bufferVA: %zx",
            streamId, rpStreamBuffer.get(), rpEncodeFrame->mpOutImgBufferHeap->getImgFormat(), rpImageBufferHeap.get(), rpImageBuffer.get(),
            groupUsage,
            transform, rpEncodeFrame->mParams.orientation,
            rpImageBufferHeap->getBufVA(0), rpImageBuffer->getBufVA(0));
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isHwEncodeSupported(int const format) const
{
    if( format == eImgFmt_YUY2 ||
        format == eImgFmt_NV12 ||
        format == eImgFmt_NV21 )
        return MTRUE;
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
errorHandle(
    sp<encode_frame>& rpEncodeFrame
)
{
    MY_LOGE("Discard frameNo=%d", rpEncodeFrame->mpFrame->getRequestNo());

    MERROR err = BaseNode::flush(rpEncodeFrame->mpFrame);

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
unlockImage(
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    sp<IImageBuffer>& rpImageBuffer1
)
{
    if( rpStreamBuffer == NULL || rpImageBuffer == NULL ) {
        MY_LOGE("rpStreamBuffer %p, rpImageBuffer %p should not be NULL",
                rpStreamBuffer.get(), rpImageBuffer.get());
        return;
    }
    rpImageBuffer->unlockBuf(getNodeName());
    if(rpImageBuffer1 != NULL)
        rpImageBuffer1->unlockBuf(getNodeName());
    rpStreamBuffer->unlock(getNodeName(), rpImageBuffer->getImageBufferHeap());
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
dumpYUVBuffer(
    MUINT32 const frameNo,
    sp<IImageBuffer>& rpImageBuffer,
    MUINT32 const idx
)
{
    char filename[256];
    sprintf(filename, "/sdcard/DCIM/Camera/Buffer_frame%d_%dx%d_%d.yuv", frameNo,
        rpImageBuffer->getImgSize().w,
        rpImageBuffer->getImgSize().h,
        idx
        );
    NSCam::Utils::saveBufToFile(filename, (unsigned char*)rpImageBuffer->getBufVA(0), rpImageBuffer->getBufSizeInBytes(0));
}

