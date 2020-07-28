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
//
#include "../utils/Log.h"
#include <mtkcam3/pipeline/hwnode/JpegNode.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <semaphore.h>
#include <future>
#include <mutex>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <cutils/properties.h>
//
#include <android/hardware/camera/device/3.2/types.h>
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
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <unordered_map>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>

//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

// [Bg service]
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>

// plugin
#include "../plugin/JpgPack.h"

#define JPEG_ENC_WARN_RATIO 0.6

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NSCam::Utils;
using namespace NSIoPipe;
using namespace std;
using namespace NSCam::TuningUtils;
using namespace NSCam::v3::pipeline::prerelease;

using ::android::hardware::camera::device::V3_2::CameraBlob;
using ::android::hardware::camera::device::V3_2::CameraBlobId;

/******************************************************************************
 *
 ******************************************************************************/
#define JPEGTHREAD_NAME ("Cam@Jpeg")
#define THUMBTHREAD_NAME ("Cam@JpegThumb")

//
#define ENABLE_DEBUG_INFO     (1)
#define ENABLE_PRERELEASE     (0)
#define DBG_BOUND_WIDTH       (320)
#define DBG_BOUND_HEIGH       (240)
#define JPEG_DUMP_PATH        "/sdcard/camera_dump/"


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
static char filename_pack[256] = {0}; // for file dump naming

class JpegNodeImp;
class JpegBufPool;

struct jpeg_params
{
    // gps related
    IMetadata::IEntry gpsCoordinates;
    IMetadata::IEntry gpsProcessingMethod;
    IMetadata::IEntry gpsTimestamp;
    //
    MINT32 orientation;
    MUINT8 quality;
    MUINT8 quality_thumbnail;
    MSize size_thumbnail;
    //
    MRect cropRegion;
    //
    MINT32 flipMode;
    MINT32 isFlipped;
    //
    MRect activeArray;
    MBOOL configured;
    MINT32 facing;
    MBOOL rotationEnable;
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
          , isFlipped(0)
          , activeArray()
          , configured(MTRUE)
          , facing(0)
          , rotationEnable(MFALSE)
    {}
};
class encode_frame : public virtual android::RefBase
{
public:
        sp<IPipelineFrame> const mpFrame;
        MBOOL mbHasThumbnail;
        MBOOL mbSkipParseMeta;
        MBOOL mbSuccess;
        MBOOL mbBufValid;
        MINT8 miJpegEncType;
        //
        jpeg_params mParams;
        //
        sp<IImageBuffer> mpJpeg_Main;
        sp<IImageBuffer> mpJpeg_Thumbnail;
        sp<IImageBuffer> mpYUV_Main;
        sp<IImageBuffer> mpYUV_Thumbnail;

        sp<IImageStreamInfo> mpYUV_MainStreamInfo;
        sp<IImageStreamInfo> mpYUV_ThumbStreamInfo;

        //
        StdExif exif;
        sp<IImageStreamBuffer> mpOutImgStreamBuffer;
        sp<IImageBufferHeap> mpOutImgBufferHeap;

        size_t thumbnailMaxSize;
        MUINT32 muJpegSwMode;
        //
        encode_frame(
                sp<IPipelineFrame> const pFrame,
                MBOOL const hasThumbnail,
                MBOOL const skipParse
                )
            : mpFrame(pFrame)
              , mbHasThumbnail(hasThumbnail)
              , mbSkipParseMeta(skipParse)
              , mbSuccess(MTRUE)
              , mbBufValid(MTRUE)
              , miJpegEncType(-1)
              , mpJpeg_Main(NULL)
              , mpJpeg_Thumbnail(NULL)
              , mpYUV_Main(NULL)
              , mpYUV_Thumbnail(NULL)
              , mpYUV_MainStreamInfo(NULL)
              , mpYUV_ThumbStreamInfo(NULL)
              , mpOutImgStreamBuffer(NULL)
              , mpOutImgBufferHeap(NULL)
              , thumbnailMaxSize(0)
              , muJpegSwMode(0)
    {}
};

struct static_info{
    MUINT8 muFacing; // ref: MTK_LENS_FACING_
    MRect mActiveArray;
    MBOOL mJpegRotationEnable;
    MINT32 mLogLevel;
    MBOOL mThumbDoneFlag;
    MBOOL mDbgInfoEnable;
    MUINT32 muDumpBuffer;
    MINT32 mFlip;
    MBOOL mbPackBitStream;
    static_info()
        : muFacing(0),
        mActiveArray(),
        mJpegRotationEnable(MFALSE),
        mLogLevel(0),
        mThumbDoneFlag(MFALSE),
        mDbgInfoEnable(MFALSE),
        muDumpBuffer(0),
        mFlip(0),
        mbPackBitStream(MFALSE){}
};


class ExifUtils final
{
public:
    static MERROR updateStdExifParam(
            MBOOL const& rNeedExifRotate,
            MSize const& rSize,
            IMetadata* const rpAppMeta,
            IMetadata* const rpHalMeta,
            jpeg_params const& rParams,
            ExifParams & rStdParams,
            MINT32 const enableLog);
    static MERROR updateStdExifParam_3A(
            IMetadata const& rMeta,
            IMetadata const& rAppMeta,
            ExifParams & rStdParams,
            MINT32 const enableLog
            );
    static MERROR updateStdExifParam_gps(
            IMetadata::IEntry const& rGpsCoordinates,
            IMetadata::IEntry const& rGpsProcessingMethod,
            IMetadata::IEntry const& rGpsTimestamp,
            ExifParams & rStdParams
            );
    static MERROR updateDebugInfoToExif(
            IMetadata* const rpHalMeta,
            StdExif & exif,
            MINT32 const enableLog
            );
    static MERROR makeExifHeader(
            sp<encode_frame> rpEncodeFrame,
            MINT8 * const pOutExif,
            // [IN/OUT] in: exif buf size, out: exif header size
            size_t& rOutExifSize,
            MBOOL needSOI = MTRUE
            );
};

class Request
{
public:
    Request(sp<IImageStreamInfo>& main,
        sp<IImageStreamInfo>& thumb,
        sp<IImageStreamInfo>& Jpeg,
        sp<IMetaStreamInfo> pInApp,
        sp<IMetaStreamInfo> pInHal,
        sp<IMetaStreamInfo> pOutApp,
        sp<IPipelineFrame> pFrame,
        MINT32 nodeId,
        const char* nodeName,
        static_info info,
        sp<JpegBufPool>& pool,
        MBOOL skipParse,
        MBOOL bAppnOnly,
        MSize heicYuvSize
        ):
        mpInMainImageStreamBuffer(NULL),
        mpInThumbImageStreamBuffer(NULL),
        mpOutJpegImageStreamBuffer(NULL),
        mpOutThumbImageStreamBuffer(NULL),
        mpInHalMetaStreamBuffer(NULL),
        mpInAppMetaStreamBuffer(NULL),
        mpOutAppMetaStreamBuffer(NULL),
        mbIsEncodeJpegError(MFALSE),
        mbIsEncodePreRelease(MFALSE),
        mpMainStreamInfo(main),
        mpThumbStreamInfo(thumb),
        mpJpegStreamInfo(Jpeg),
        mpInMainImageBuffer(NULL),
        mpInThumbImageBuffer(NULL),
        mpOutJpegImageBuffer(NULL),
        mpOutThumbImageBuffer(NULL),
        mpInAppMeta(pInApp),
        mpInHalMeta(pInHal),
        mpOutAppMeta(pOutApp),
        mInAppMeta(NULL),
        mInHalMeta(NULL),
        mOutAppMeta(NULL),
        mpEncodeFrame(NULL),
        mpFrame(pFrame),
        mNodeId(nodeId),
        mNodeName(nodeName),
        mUniqueKey(-1),
        mFrameNumber(-1),
        mRequestNumber(-1),
        mFreeMemoryMBytes(-1),
        mInfo(info),
        mJpegPool(pool),
        mbSkipParseMeta(skipParse),
        mbAppnOnly(bAppnOnly),
        mHeicYuvSize(heicYuvSize)
    {
        mpEncodeFrame =
            new encode_frame(mpFrame, (mpThumbStreamInfo != NULL)/*useThumbnail*/, mbSkipParseMeta);
    }

    ~Request()
    {}

    // get image & meta and lock
    MERROR lockMeta();

    MERROR lockInImg();

    MERROR lockOutImg();

    // unlock image & meta
    MERROR unlock();

    // prepare encode frame & parse the metadata
    MERROR prepareMeta();

    MERROR prepareExif();

    // get Encode frame for Jpeg encode bitStream
    sp<encode_frame> getEncodeFrame()
    {
        return mpEncodeFrame;
    }

    IMetadata* getHalMeta()
    {
        return mInHalMeta;
    }

    IMetadata* getAppMeta()
    {
        return mInAppMeta;
    }

    IMetadata* getResultMeta()
    {
        return mOutAppMeta;
    }

    MVOID updateMetadata(
            jpeg_params& rParams
            );

private:
    MERROR getInImageBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBuffer>& rpImageBuffer,
            sp<IImageStreamInfo>& rpImageStreamInfo
            );
    MERROR getOutJpegImageBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBuffer>& rpImageBuffer,
            sp<encode_frame>& rpEncodeFrame,
            sp<IImageBufferHeap>& rpImageBufferHeap
            );
    MERROR getOutThumbImageBuffer(
            StreamId_T const streamId,
            sp<encode_frame> const& rpEncodeFrame,
            sp<IImageBufferHeap> const& rpImageBufferHeap,
            sp<IImageBuffer>& rpImageBuffer /*out*/
            );
    MERROR getMetadataBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IMetaStreamBuffer>& rpStreamBuffer,
            IMetadata*& rpMetadata
            );
    MERROR returnMetadataBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IMetaStreamBuffer>  rpStreamBuffer,
            IMetadata* rpMetadata,
            MBOOL success = MTRUE
            );
    MERROR returnImageBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBuffer> rpImageBuffer,
            sp<IImageBuffer> rpImageBuffer2 = NULL
            );
    MBOOL isInImageStream(
            StreamId_T const streamId
            ) const;
    MBOOL isInMetaStream(
            StreamId_T const streamId
            ) const;
    MERROR acquireImageBuffer(
            MUINT32 const frameNo,
            StreamId_T const streamId,
            IStreamBufferSet& rStreamBufferSet,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            MBOOL acquire = MTRUE
            );
    MERROR acquireMetaBuffer(
            MUINT32 const frameNo,
            StreamId_T const streamId,
            IStreamBufferSet& rStreamBufferSet,
            sp<IMetaStreamBuffer>& rpStreamBuffer,
            MBOOL acquire = MTRUE
            );
    MVOID getJpegParams(
            IMetadata* pMetadata_request,
            jpeg_params& rParams
            ) const;
    MVOID unlockImage(
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBuffer>& rpImageBuffer
            );
public:
    sp<IImageStreamBuffer> mpInMainImageStreamBuffer;
    sp<IImageStreamBuffer> mpInThumbImageStreamBuffer;
    sp<IImageStreamBuffer> mpOutJpegImageStreamBuffer;
    sp<IImageStreamBuffer> mpOutThumbImageStreamBuffer;
    sp<IMetaStreamBuffer> mpInHalMetaStreamBuffer;
    sp<IMetaStreamBuffer> mpInAppMetaStreamBuffer;
    sp<IMetaStreamBuffer> mpOutAppMetaStreamBuffer;
    MBOOL mbIsEncodeJpegError;
    MBOOL mbIsEncodePreRelease;

private:
    sp<IImageStreamInfo> mpMainStreamInfo;
    sp<IImageStreamInfo> mpThumbStreamInfo;
    sp<IImageStreamInfo> mpJpegStreamInfo;
    sp<IImageBuffer> mpInMainImageBuffer;
    sp<IImageBuffer> mpInThumbImageBuffer;
    sp<IImageBuffer> mpOutJpegImageBuffer;
    sp<IImageBuffer> mpOutThumbImageBuffer;
    sp<IMetaStreamInfo> mpInAppMeta;
    sp<IMetaStreamInfo> mpInHalMeta;
    sp<IMetaStreamInfo> mpOutAppMeta;
    IMetadata* mInAppMeta;
    IMetadata* mInHalMeta;
    IMetadata* mOutAppMeta;
    sp<encode_frame> mpEncodeFrame;
    sp<IPipelineFrame> mpFrame;
    MINT32 mNodeId;
    const char* mNodeName;
    MINT32 mUniqueKey;
    MINT32 mFrameNumber;
    MINT32 mRequestNumber;
    MINT32 mFreeMemoryMBytes;
    static_info mInfo;
    sp<JpegBufPool> mJpegPool;
    MBOOL mbSkipParseMeta;
    MBOOL mbAppnOnly;
    MSize mHeicYuvSize;

};

class JpegBufPool : public virtual android::RefBase
{
public:
    enum BufStatus
    {
        IDLE = 0,
        AVAILABLE,
        ACQUIRED,
        FREE
    };
    struct BufLot
    {
        BufLot()
        :pHeap(NULL),
        status(IDLE),
        fd(0)
        {}

        sp<IImageBufferHeap> pHeap;
        BufStatus status;
        MINT32 fd;
    };
    JpegBufPool(vector<sp<IImageStreamInfo>> vStreamInfo, MINT32 bufCount)
    :
    mBufCount(bufCount),
    mvImageStreamInfo(vStreamInfo)
    {}

    ~JpegBufPool();

    MERROR commitPool();

    sp<IImageBufferHeap> getBufHeap(StreamId_T const streamId);

    MERROR putBufHeap(StreamId_T const streamId, MINT32 ion_fd);

private:
    MINT32 mBufCount;
    std::mutex mPoolLock;
    std::unordered_map<StreamId_T, vector<BufLot>> mPools;
    vector<sp<IImageStreamInfo>> mvImageStreamInfo;

};

class jpeg_frame : public virtual android::RefBase
{
public:


    sp<IPipelineFrame> const    m_pFrame;
    MBOOL                       m_bPackBitStream;
    MBOOL                       m_bHasThumbnail;
    sp<IImageStreamBuffer>      m_pOutJpeg_ImgStreamBuffer;
    sp<IImageBufferHeap>        m_pOutJpeg_ImgBufferHeap;

    sp<IImageStreamBuffer>      m_pInYuv_ImgStreamBuffer;
    sp<IImageStreamBuffer>      m_pInThumbYuv_ImgStreamBuffer;
    sp<IImageStreamBuffer>      m_pIn2Yuv_ImgStreamBuffer;
    sp<IImageStreamBuffer>      m_pInY16_ImgStreamBuffer;
    sp<IMetaStreamBuffer>       m_pInMetaStream_Request;
    sp<IMetaStreamBuffer>       m_pInMetaStream_Hal;
    sp<IMetaStreamBuffer>       m_pOutMetaStream_Result;
    buffer_handle_t             m_GraphicBufferImportHandle;

    sp<IMetaStreamInfo> m_pInHalMeta;

    MINT32                      m_NodeId;

    vector<Request> m_vReqList;
    vector<std::thread> m_vReqEncodeThreads;

                                //
                                jpeg_frame(
                                    sp<IPipelineFrame> const pFrame
                                    )
                                    :
                                      m_pFrame(pFrame)
                                    , m_bPackBitStream(MFALSE)
                                    , m_bHasThumbnail(MFALSE)
                                    , m_pOutJpeg_ImgStreamBuffer(NULL)
                                    , m_pOutJpeg_ImgBufferHeap(NULL)
                                    , m_pInYuv_ImgStreamBuffer(NULL)
                                    , m_pInThumbYuv_ImgStreamBuffer(NULL)
                                    , m_pIn2Yuv_ImgStreamBuffer(NULL)
                                    , m_pInY16_ImgStreamBuffer(NULL)
                                    , m_pInMetaStream_Request(NULL)
                                    , m_pInMetaStream_Hal(NULL)
                                    , m_pOutMetaStream_Result(NULL)
                                    , m_GraphicBufferImportHandle(NULL)
                                    , m_pInHalMeta(NULL)
                                    , m_NodeId(0)
                                   {}


};

static void extractRequestLoop(JpegNodeImp& jpg);
static void onProcessRequest(JpegNodeImp& jpg,
        Request& req);
static void waitJpegEncodeReqDone(JpegNodeImp& jpg);


class JpegNodeImp
    : public BaseNode
    , public JpegNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //// Definitions.
    typedef android::sp<IPipelineFrame> QueNode_T;
    typedef android::List<QueNode_T> Que_T;

public:     ////Operations.

    JpegNodeImp();

    ~JpegNodeImp();

    virtual MERROR config(ConfigParams const& rParams);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  IPipelineNode Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////Operations.

    virtual MERROR init(InitParams const& rParams);

    virtual MERROR uninit();

    virtual MERROR flush();

    virtual MERROR flush(
            android::sp<IPipelineFrame> const &pFrame
            );

    virtual MERROR queue(
            android::sp<IPipelineFrame> pFrame
            );
    MVOID encodeThumbnailJpeg(
            sp<encode_frame>& pEncodeFrame
            );
    MVOID processJpeg(android::sp<IPipelineFrame>& pFrame, JpegNodeImp& jpg);

    MVOID processJpegAppSegments(android::sp<IPipelineFrame>& pFrame, JpegNodeImp& jpg);

protected:  ////Operations.

    MERROR mapRequest(
            android::sp<IPipelineFrame>& rpFrame,
            vector<Request>& reqList,
            static_info& info
            );
    MERROR verifyConfigParams(
            ConfigParams const & rParams
            ) const;

    MVOID waitForRequestDrained();

    MBOOL isInMetaStream(
            StreamId_T const streamId
            ) const;

    MBOOL isInImageStream(
            StreamId_T const streamId
            ) const;

    MVOID encodeMainJpeg(
            sp<encode_frame>& pEncodeFrame
            );
    MVOID encodeExif(
            sp<encode_frame>& pEncodeFrame);


private:    ////to sync main yuv & thumbnail yuv


    typedef android::sp<jpeg_frame>                QueFrameNode_T;
    typedef android::List<QueFrameNode_T>          QueJpegFrame_T;

    friend void extractRequestLoop(JpegNodeImp& jpg);

    friend void onProcessRequest(JpegNodeImp& jpg,
        Request& req);


    // wait jpeg encoder done
    friend void waitJpegEncodeReqDone(JpegNodeImp& jpg);

    MVOID finalizeEncodeFrame(
            android::sp<IPipelineFrame>& rpFrame
            );
    MERROR errorHandle(
            android::sp<IPipelineFrame> const& pFrame
            );
    MERROR  getBufferAndLock(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBufferHeap>& rpImageBufferHeap,
            sp<IImageBuffer>& rpImageBuffer,
            MBOOL isIn
            );
    MERROR returnBuffer(
            android::sp<IPipelineFrame> const& pFrame,
            sp<IImageStreamBuffer>& rpStreamBuffer,
            sp<IImageBuffer>& rpImageBuffer
            );
    MERROR checkBufferAvailable(
            android::sp<IPipelineFrame> const& pFrame,
            StreamId_T const streamId
            );

protected:  ////hw related

    class my_encode_params
    {
        public:
            // buffer
            IImageBuffer* pSrc;
            IImageBuffer* pDst;

            // settings
            MUINT32 transform;
            MRect crop;
            MUINT32 isSOI;
            MUINT32 quality;
            MUINT32 codecType;
        my_encode_params()
        : pSrc(NULL)
        , pDst(NULL)
        , transform(0)
        , crop()
        , isSOI(0)
        , quality(0)
        , codecType(0)
        {}
    };

    MERROR hardwareOps_encode(
            my_encode_params& rParams
            );

protected:  ////Data Members. (Config)
    mutable RWLock mConfigRWLock;
    // meta
    sp<IMetaStreamInfo> mpInAppMeta;
    sp<IMetaStreamInfo> mpInHalMeta_capture;
    sp<IMetaStreamInfo> mpInHalMeta_streaming;
    std::vector<sp<IMetaStreamInfo>> mvInHalMeta;
    std::vector<sp<IMetaStreamInfo>> mvInAppMeta;
    sp<IMetaStreamInfo> mpOutMetaStreamInfo_Result;
    std::unordered_map<int, sp<IMetaStreamInfo>> mHalMetaMap;

    // image
    sp<IImageStreamInfo> mpInYuv_main;
    sp<IImageStreamInfo> mpInYuv_thumbnail;
    sp<IImageStreamInfo> mpInYuv_main2;
    sp<IImageStreamInfo> mpInYuv_Y16;
    sp<IImageStreamInfo> mpOutJpeg;
    sp<IImageStreamInfo> mpOutJpegAppSegments;

protected:
    sp<JpegBufPool> mBufPool;

protected:   //Data Members (Request)
    std::vector<sp<IImageStreamInfo>> mvInYuv_main;
    std::vector<sp<IImageStreamInfo>> mvInYuv_thumbnail;
    sp<IImageStreamInfo> mpInYuv_Y16_r;

    sp<IImageStreamBuffer> mOutputJpegStreamBuf;
    sp<IImageBuffer> mOutputJpegBuffer;
    sp<IImageBufferHeap> mOutputJpegBufferHeap;
    sp<IImageStreamBuffer> mOutputY16StreamBuf;
    sp<IImageBuffer> mOutputY16Buffer;
protected:  ////Data Members. (Request Queue)
    mutable std::mutex mRequestQueueLock;
    sem_t mRequestQueueCond;
    sem_t mbRequestDrainedCond;         // encode current request busy
    Que_T mRequestQueue;
    MBOOL mbRequestDrained;             // TRUE: no encoding request, can flush directly
    MBOOL mbRequestExit;

    MBOOL                mbRequestDequeSuspend;
    sem_t                mbRequestDequeSuspendCond;
    QueJpegFrame_T       mJpegFrameQueue;
    mutable std::mutex   mEncFrameQueueLock;
    MUINT32              mInQueuePreReleaseRequestNum;
    MBOOL                mIsPreReleaseRequest;
    MUINT32              mPropPreReleaseMode;
    MBOOL                mIsPreReleaseEnable;
    MBOOL                mbAppnOnly;
    MSize                mHeicYuvSize;
    sp<jpeg_frame>       mpJpegFrame;

    //for request async encode
    sem_t                mReqNewTriggerEncodeCond;
    sem_t                mReqWaitDoneQueueStillFreeCond;
    sem_t                mReqWaitDoneQueueIdleCond;
    MBOOL                mIsAsyncWaitEncodeReqEnable;
    MUINT32              mJpegAsyncWaitEncodeReqLevel;
    QueJpegFrame_T       mJpegWaitDoneQueue;
    mutable std::mutex   mWaitDoneQueueLock;
    MUINT32              mPropJpegAsyncMode;
    MUINT32              mPropJpegSwMode;

private:   //// Threads
    std::future<void> mWorkerThread;
    std::future<void> mReqEncodeThread;

private:
    mutable std::mutex mEncodeLock;
    sem_t mEncodeCond;

private: // static infos
    static_info info;
private: // plugin
    sp<JpgPack> mpJpgPack;
};

