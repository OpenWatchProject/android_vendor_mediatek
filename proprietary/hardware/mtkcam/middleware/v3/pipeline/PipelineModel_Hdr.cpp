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

#define LOG_TAG "MtkCam/HwPipelineHdr"
//
#include "MyUtils.h"
//
#include "PipelineModel_Hdr.h"
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/aaa/aaa_hal_common.h>
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/HDRNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/utils/hw/CamManager.h>
//
#include <mtkcam/feature/hdr/HDRDefs2.h>
#include <mtkcam/feature/hdr/Platform.h>

#include <memory>

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::HDR2;
using namespace NSCam::Utils;
using NSCam::v3::Utils::MetaStreamInfo;
using NSCam::v3::Utils::ImageStreamInfo;
using namespace NSCam::v3::NSPipelineContext;
using NSCam::v3::Utils::HalMetaStreamBuffer;
using NSCam::v3::Utils::HalImageStreamBuffer;

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
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
typedef HalMetaStreamBuffer::       Allocator
                                    HalMetaStreamBufferAllocatorT;
/******************************************************************************
 *
 ******************************************************************************/
#define SUPPORT_RECONFIGURE         (0)

#define ENABLE_HDRNODE              (1)

#define ALIGN_2(x)     (((x) + 1) & (~1))

static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}

// utility functions
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

static
MBOOL                   getSensorOutputFmt(
                            SensorStaticInfo const& sensorInfo,
                            MUINT32 bitDepth,
                            MBOOL isFull,
                            MINT* pFmt
                        );

// used to check if applying single or multi-frame capture
static inline bool isSingleCapture(
        const size_t captureFrames, const size_t delayedFrames = 0)
{
    return (captureFrames == (1 + delayedFrames));
}

/******************************************************************************
 *
 ******************************************************************************/
namespace {
class PipelineHdrImp
    : public PipelineModel_Hdr
    , public IPipelineBufferSetFrameControl::IAppCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    class MyProcessedParams
    {
    public:
        MINT32 const                mOpenId;
        /*  input params  */
        PipeConfigParams            mConfigParams;
        // internal setting
        MBOOL                       mSkipJpeg;
        //
        /* static info  */
        MBOOL                       mbSensorInfoQueried;
        SensorStaticInfo            mSensorStaticInfo;
        //
        /*  processed params  */
        // Stream infos
        MBOOL                       mbHasRecording;
        MBOOL                       mbHasRaw;
        MBOOL                       mbHasJpeg;
        MSize                       mMaxStreamSize;
        //
        MBOOL                       mbUseP1Node;
        MBOOL                       mbUseP2Node;
        MBOOL                       mbUseHdrNode;
        MBOOL                       mbUseJpegNode;
        //
        // sensor related
        MUINT                       mSensorMode;
        MSize                       mSensorSize;
        MUINT                       mSensorFps;
        //
        // p1 related
        MUINT32                     mPixelMode;
        MINT                        mFullrawFormat;
        MSize                       mFullrawSize;
        size_t                      mFullrawStride;
        MINT                        mResizedrawFormat;
        MSize                       mResizedrawSize;
        size_t                      mResizedrawStride;
        //
    public:
                                    MyProcessedParams(MINT32 const openId);
        MERROR                      update(
                                        PipeConfigParams const& rParams,
                                        MBOOL skipJpeg
                                        )
                                    {
                                        // update config params
                                        mConfigParams = rParams;
                                        mSkipJpeg     = skipJpeg; //TODO: use this?
                                        //
                                        MERROR err;
                                        if( OK != (err = querySensorStatics())  ||
                                            OK != (err = preprocess())          ||
                                            OK != (err = decideSensor())        ||
                                            OK != (err = decideP1())
                                          )
                                            return err;
                                        return OK;
                                    }
        //
    protected:
        MERROR                      querySensorStatics();
        MERROR                      preprocess();
        MERROR                      decideSensor();
        MERROR                      decideP1();
    };

    struct parsedAppRequest
    {
        // original AppRequest
        AppRequest* const           pRequest;
        //
        IMetadata*                  pAppMetaControl;
        // in
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Yuv;
        // out
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Yuv;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Jpeg;
        //
                                    parsedAppRequest(AppRequest* pRequest)
                                        : pRequest(pRequest)
                                        , pAppMetaControl()
                                    {}
                                    ~parsedAppRequest() {
                                        if( pAppMetaControl )
                                            pRequest->vIMetaBuffers[0]->unlock(
                                                    "parsedAppRequest", pAppMetaControl
                                                    );
                                    }
        //
        MERROR                      parse();
    };

    struct pass1Resource
    {
        enum StreamStatus
        {
            eStatus_Uninit  = 0,
            eStatus_Inited,
            eStatus_Reuse,
            eStatus_NoNeed,
            eNUM_Status
        };
        //
        KeyedVector< StreamId_T, StreamStatus>
                                    vStreamStatus;
        MBOOL                       bStreamReused;

                                    pass1Resource()
                                        : vStreamStatus()
                                        , bStreamReused()
                                    {}

                                    ~pass1Resource()
                                    {}

        MINT32                      setCapacity(size_t size) { return vStreamStatus.setCapacity(size); }

        MVOID                       setReuseFlag(MBOOL flag) { bStreamReused = flag; }

        MBOOL                       getReuseFlag() { return bStreamReused; }

        MVOID                       updateStreamStatus(
                                        StreamId_T const streamId,
                                        StreamStatus eStatus
                                    )
                                    {
                                        ssize_t const index = vStreamStatus.indexOfKey(streamId);
                                        if ( index < 0 )
                                            vStreamStatus.add(streamId, eStatus);
                                        else
                                            vStreamStatus.replaceValueFor(streamId, eStatus);
                                    }

        MBOOL                       checkStreamsReusable()
                                    {
                                        MBOOL bRet = MTRUE;
                                        for ( size_t i=0; i<vStreamStatus.size(); i++) {
                                            if ( vStreamStatus.valueAt(i) < StreamStatus::eStatus_Reuse )
                                                return MFALSE;
                                        }
                                        return MTRUE;
                                    }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    MINT32 const                    mOpenId;
    android::String8 const          mName;
    android::wp<IPipelineModelMgr::IAppCallback>const
                                    mpAppCallback;

    MyProcessedParams               mParams;

    // meta: hal
    android::sp<IMetaStreamInfo>    mpHalMeta_Control;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicHdr;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP2;
    // meta: app
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicHdr;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP2;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicJpeg;
    android::sp<IMetaStreamInfo>    mpAppMeta_Control;
    // image: hal
    android::sp<IImageStreamInfo>   mpHalImage_P1_Raw;
    android::sp<IImageStreamInfo>   mpHalImage_P1_ResizerRaw;
    android::sp<IImageStreamInfo>   mpHalImage_Jpeg_YUV;
    android::sp<IImageStreamInfo>   mpHalImage_Thumbnail_YUV;

    // image: app
    android::KeyedVector <
            StreamId_T, android::sp<IImageStreamInfo>
                    >               mvAppYuvImage;
    android::sp<IImageStreamInfo>   mpAppImage_Jpeg;

    // raw/yuv stream mapping
    MBOOL                           mP1ImageStreamReuse;
    StreamSet                       mvYuvStreams_Fullraw;
    StreamSet                       mvYuvStreams_Resizedraw;
    //android::Vector<StreamId_T>     mvYuvStreams_Fullraw;
    //android::Vector<StreamId_T>     mvYuvStreams_Resizedraw;

    android::KeyedVector < StreamId_T, MINT64 >
                                    mvStreamDurations;
    MBOOL                           mJpegRotationEnable;
    //
    std::vector<HDRCaptureParam>    mvHdrCaptureParams;
    sp< HDRNode >                   mpHdrNode;

protected:
    android::RWLock                 mRWLock;

    MINT32                          mTimestamp;

    sp<PipelineContext>             mpPipelineContext;

    sp<PipelineContext>             mpOldCtx;

    pass1Resource                   mPass1Resource;

    CamManager*                     mpCamMgr;
    sp<CamManager::UsingDeviceHelper>
                                    mpDeviceHelper;

    // CPU performance utility
    std::unique_ptr<IPerf>          mPerf;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual char const*             getName() const { return mName.string(); }
    virtual MINT32                  getOpenId() const { return mOpenId; }

    virtual MERROR                  submitRequest(AppRequest& request);

    virtual MERROR                  beginFlush();

    virtual MVOID                   endFlush();

    virtual MVOID                   endRequesting() {}

    virtual MVOID                   waitDrained();

    virtual sp<PipelineContext>     getContext();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipelineModel_Hdr Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MERROR                  configure(
                                        PipeConfigParams const& rConfigParams,
                                        android::sp<IPipelineModel> pOldPipeline
                                    );
    virtual void                    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    ~PipelineHdrImp();
                                    PipelineHdrImp(
                                        MINT32 const openId,
                                        android::String8 const& name,
                                        wp<IPipelineModelMgr::IAppCallback> pAppCallback
                                    );

private:    ////                    Operations.

    MVOID                           evaluatePreviewSize(
                                        PipeConfigParams const& rConfigParams,
                                        MSize &rSize
                                    );

    MERROR                          setupAppStreamsLocked(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          setupHalStreamsLocked(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          configContextLocked_Streams(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_Nodes(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_Pipeline(sp<PipelineContext> pContext);
    //
    MERROR                          configContextLocked_P1Node(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_P2Node(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_HdrNode(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_JpegNode(sp<PipelineContext> pContext);
    //
    MERROR                          checkPermission();
    MERROR                          configRequestRulesLocked();
    //
    MERROR                          compareParamsLocked_P1Node(
                                        P1Node::InitParams const& initParam1,  P1Node::InitParams const& initParam2,
                                        P1Node::ConfigParams const& cfgParam1, P1Node::ConfigParams const& cfgParam2
                                    ) const;
    struct evaluateRequestResult
    {
        DefaultKeyedVector<
            StreamId_T,
            sp<IImageStreamInfo>
                >                               vUpdatedImageInfos;
        //
        NodeSet                                 roots;
        NodeEdgeSet                             edges;
        //
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapImage;
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapMeta;
        //
        DefaultKeyedVector<StreamId_T, sp<IImageStreamBuffer> >
                                                vAppImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<HalImageStreamBuffer> >
                                                vHalImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<IMetaStreamBuffer> >
                                                vAppMetaBuffers;
        DefaultKeyedVector<StreamId_T, sp<HalMetaStreamBuffer> >
                                                vHalMetaBuffers;
    };

    MERROR                          evaluateRequestLocked(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        const MBOOL bMainReq,
                                        const MBOOL bDelayedFrame = MFALSE
                                    );

    MERROR                          refineRequestMetaStreamBuffersLocked(
                                        evaluateRequestResult& result,
                                        const size_t frameIndex = -1,
                                        const size_t delayedFrames = 0,
                                        const MBOOL bLastReq = MFALSE
                                    );

    MERROR                          createStreamInfoLocked_Thumbnail_YUV(
                                        IMetadata const* pMetadata,
                                        android::sp<IImageStreamInfo>& rpStreamInfo
                                    ) const;

    MERROR                          createStreamInfoLocked_Jpeg_YUV(
                                        IMetadata const* pMetadata,
                                        android::sp<IImageStreamInfo>& rpStreamInfo
                                    ) const;

    MSize                           calcThumbnailYuvSize(
                                        MSize const rPicSize,
                                        MSize const rThumbnailsize
                                    ) const;

    MBOOL                           isFdEnable(
                                        IMetadata const* pMetadata
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   updateFrame(
                                        MUINT32 const frameNo,
                                        MINTPTR const userId,
                                        Result const& result
                                    );

private:
    // utility functions
    sp<ImageStreamInfo>             createRawImageStreamInfo(
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

    MBOOL                           skipStream(
                                        MBOOL skipJpeg,
                                        IImageStreamInfo* pStreamInfo
                                    ) const;
};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
PipelineModel_Hdr*
PipelineModel_Hdr::
create(MINT32 const openId, wp<IPipelineModelMgr::IAppCallback> pAppCallback)
{
    String8 const name = String8::format("%s:%d", magicName(), openId);
    PipelineModel_Hdr* pPipelineModel = new PipelineHdrImp(openId, name, pAppCallback);
    if  ( ! pPipelineModel ) {
        MY_LOGE("fail to new an instance");
        return NULL;
    }
    //
    return pPipelineModel;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineHdrImp::MyProcessedParams::
MyProcessedParams(MINT32 const openId)
    : mOpenId(openId)
    //
    , mConfigParams()
    , mSkipJpeg(MFALSE)
    //
    , mbSensorInfoQueried(MFALSE)
    , mSensorStaticInfo()
    //
    , mbHasRecording()
    , mbHasRaw()
    , mbHasJpeg()
    , mMaxStreamSize()
    //
    , mbUseP1Node()
    , mbUseP2Node()
    , mbUseHdrNode()
    , mbUseJpegNode()
    //
    , mSensorMode()
    , mSensorSize()
    , mSensorFps()
    //
    , mPixelMode()
    , mFullrawFormat()
    , mFullrawSize()
    , mFullrawStride()
    , mResizedrawFormat()
    , mResizedrawSize()
    , mResizedrawStride()
{
    memset(&mSensorStaticInfo, 0, sizeof(SensorStaticInfo));
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::MyProcessedParams::
querySensorStatics()
{
    if( mbSensorInfoQueried ) return OK;
    //
    IHalSensorList* pSensorList = MAKE_HalSensorList();
    if( pSensorList == NULL ) {
        MY_LOGE("pSensorList == NULL");
        return DEAD_OBJECT;
    }
    //
    MUINT32 sensorDev = pSensorList->querySensorDevIdx(mOpenId);
    pSensorList->querySensorStaticInfo(sensorDev, &mSensorStaticInfo);
    //
    mbSensorInfoQueried = MTRUE;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::MyProcessedParams::
preprocess()
{
#define hasUsage(flag, usage) ((flag & usage) == usage)
    MBOOL hasVRConsumer = MFALSE;
    for (size_t i = 0; i < mConfigParams.vImage_Yuv_NonStall.size(); i++) {
        if  ( hasUsage(
                    mConfigParams.vImage_Yuv_NonStall[i]->getUsageForConsumer(),
                    GRALLOC_USAGE_HW_VIDEO_ENCODER
                    ) )
        {
            hasVRConsumer = MTRUE;
            break;
        }
    }
#undef hasUsage
    //
    MSize maxStreamSize;
    {
        struct Log
        {
            static String8
                skippedStream(IImageStreamInfo* pStreamInfo)
                {
                    return String8::format(
                            "skipped stream - format:0x%x type:%x size:%dx%d",
                            pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                            pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                            );
                }

            static String8
                candidateStream(IImageStreamInfo* pStreamInfo)
                {
                    return String8::format(
                            "candidate stream - format:0x%x type:%x size:%dx%d",
                            pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                            pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                            );
                }
        };
        //
        if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Raw.get() ) {
            if  ( pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN ) {
                MY_LOGD("%s", Log::skippedStream(pStreamInfo).string());
            }
            else {
                MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }
        //
        if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Jpeg_Stall.get() ) {
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size() ) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }
        //
        for (size_t i = 0; i < mConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( IImageStreamInfo* pStreamInfo = mConfigParams.vImage_Yuv_NonStall[i].get()) {
                MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
                if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size()) {
                    maxStreamSize = pStreamInfo->getImgSize();
                }
            }
        }
    }
    //
    // update processed params
    mbHasRecording = hasVRConsumer;
    mbHasRaw       = mConfigParams.pImage_Raw.get() ? MTRUE : MFALSE;
    mbHasJpeg      = mConfigParams.pImage_Jpeg_Stall.get() ? MTRUE : MFALSE;
    mMaxStreamSize = maxStreamSize;
    MY_LOGD("max stream(%d, %d), raw(%d), jpeg(%d), hasRecording(%d)",
            mMaxStreamSize.w, mMaxStreamSize.h, mbHasRaw, mbHasJpeg, mbHasRecording);
    //
    mbUseP1Node    = MTRUE;
#if ENABLE_HDRNODE
    mbUseHdrNode   = MTRUE;
#else
    mbUseP2Node    = MTRUE;
#endif
    mbUseJpegNode  = mbHasJpeg;
    //
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::MyProcessedParams::
decideSensor()
{
    struct sensormodeHelper
    {
        // use enum to select sensor mode if have preferred sensor mode.
        enum
        {
            eNORMAL_PREVIEW = 0,
            eNORMAL_VIDEO,
            eNORMAL_CAPTURE,
            eNUM_SENSOR_MODE,
        };
        //
                    sensormodeHelper(
                            SensorStaticInfo const& staticInfo,
                            MyProcessedParams* rpParams
                            )
                        : selectIdx(-1)
                        , pParams(rpParams)
                    {
                    #define addMode(idx, _scenarioId_, _key_)                                           \
                        do {                                                                            \
                            sensorSize[idx] = MSize(staticInfo._key_##Width, staticInfo._key_##Height); \
                            sensorFps [idx] = staticInfo._key_##FrameRate/10;                           \
                            sensorMode[idx] = _scenarioId_;                                             \
                            MY_LOGD("candidate mode %d, size(%d, %d)@%d", idx, staticInfo._key_##Width, \
                            staticInfo._key_##Height, staticInfo._key_##FrameRate/10);                  \
                        } while(0)
                        addMode(eNORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview);
                        addMode(eNORMAL_VIDEO  , SENSOR_SCENARIO_ID_NORMAL_VIDEO  , video);
                        addMode(eNORMAL_CAPTURE, SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture);
                    #undef addMode
                    //
                    #if 1 // preview/video mode, accept by FOV verification
                        NSCamHW::HwTransHelper helper(rpParams->mOpenId);
                        MBOOL acceptPrv = verifyFov(helper, eNORMAL_PREVIEW);
                        MBOOL acceptVdo = verifyFov(helper, eNORMAL_VIDEO);
                    #define altMode(src, dst)                               \
                        do {                                                \
                            sensorSize[src] = sensorSize[dst];              \
                            sensorFps [src] = sensorFps [dst];              \
                            sensorMode[src] = sensorMode[dst];              \
                            MY_LOGD("alt candidate mode %d=>%d", src, dst); \
                        } while(0)
                        if (!acceptPrv) {
                            if (!acceptVdo) {
                                altMode(eNORMAL_VIDEO  , eNORMAL_CAPTURE);
                                altMode(eNORMAL_PREVIEW, eNORMAL_CAPTURE);
                            } else {
                                altMode(eNORMAL_PREVIEW, eNORMAL_VIDEO  );
                            }
                        } else if (!acceptVdo) {
                            altMode(eNORMAL_VIDEO,
                                (sensorFps[eNORMAL_CAPTURE] >= 30 ?
                                eNORMAL_CAPTURE : eNORMAL_PREVIEW));
                        }
                    #undef altMode
                    #endif
                    };
                    ~sensormodeHelper()
                    {
                        if( selectIdx != -1 ) {
                            pParams->mSensorMode = sensorMode[selectIdx];
                            pParams->mSensorSize = sensorSize[selectIdx];
                            pParams->mSensorFps  = sensorFps [selectIdx];
                            //
                            refineFps_MultiOpen(pParams->mSensorMode, pParams->mSensorFps);
                            //
                            MY_LOGD("select mode %d, size(%d, %d)@%d",
                                    pParams->mSensorMode,
                                    pParams->mSensorSize.w, pParams->mSensorSize.h,
                                    pParams->mSensorFps
                                   );
                        } else {
                            MY_LOGW("sensor mode is not selected!");
                            for( int i = 0; i < eNUM_SENSOR_MODE; i++ ) {
                                MY_LOGD("mode %d, size(%d, %d)@%d",
                                        sensorMode[i],
                                        sensorSize[i].w, sensorSize[i].h,
                                        sensorFps[i]
                                       );
                            }
                        }
                    }
        MBOOL       verifyFov(NSCamHW::HwTransHelper helper, MUINT const mode)
                    {
                        #define FOV_DIFF_TOLERANCE (0.002)
                        float dX = 0.0f;
                        float dY = 0.0f;
                        return (helper.calculateFovDifference(
                            sensorMode[mode], &dX, &dY) &&
                            dX < FOV_DIFF_TOLERANCE && dY < FOV_DIFF_TOLERANCE)
                            ? MTRUE : MFALSE;
                    };
        MVOID       refineFps_MultiOpen(MUINT const /*mode*/, MUINT& /*fps*/)
                    {}
                    //
        MSize                       sensorSize[eNUM_SENSOR_MODE];
        MUINT                       sensorFps [eNUM_SENSOR_MODE];
        MUINT                       sensorMode[eNUM_SENSOR_MODE];
        //
        int                         selectIdx;
        MyProcessedParams* const    pParams;
    } aHelper(mSensorStaticInfo, this);
    //
    // 1. Raw stream configured: find sensor mode with raw size.
    if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Raw.get() ) {
        bool hit = false;
        for (int i = 0; i < sensormodeHelper::eNUM_SENSOR_MODE; i++) {
            if  (pStreamInfo->getImgSize() == aHelper.sensorSize[i]) {
                aHelper.selectIdx = i;
                hit = true;
                break;
            }
        }
        if( !hit ) {
            MY_LOGE("Can't find sesnor size that equals to raw size");
            return UNKNOWN_ERROR;
        }
    } else
    // 2. if has VR consumer: sensor video mode is preferred
    if  ( mbHasRecording ) {
        aHelper.selectIdx = sensormodeHelper::eNORMAL_VIDEO;
    }
    else {
        //policy:
        //    find the smallest size that is "larger" than max of stream size
        //    (not the smallest difference)
        bool hit = false;
        for (int i = 0; i < sensormodeHelper::eNUM_SENSOR_MODE; i++) {
            if  ( mMaxStreamSize.w <= aHelper.sensorSize[i].w &&
                  mMaxStreamSize.h <= aHelper.sensorSize[i].h )
            {
                aHelper.selectIdx = i;
                hit = true;
                break;
            }
        }
        if( !hit ) {
            // pick largest one
            MY_LOGW("select capture mode");
            aHelper.selectIdx = sensormodeHelper::eNORMAL_CAPTURE;
        }
    }
    //
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::MyProcessedParams::
decideP1()
{
    struct refine
    {
        static
            MVOID       not_larger_than(MSize& size, MSize const& limit) {
                if( size.w > limit.w ) size.w = limit.w;
                if( size.h > limit.h ) size.h = limit.h;
            }
        static
            MVOID       not_smaller_than(MSize& size, MSize const& limit) {
                if( size.w < limit.w ) size.w = limit.w;
                if( size.h < limit.h ) size.h = limit.h;
            }
        static
            MSize       align_2(MSize const& size) {
#define align2(x)  ((x+1) & (~1))
                return MSize(align2(size.w), align2(size.h));
#undef align2
            }
        static
            MSize       scale_roundup(MSize const& size, int mul, int div) {
                return MSize((size.w * mul + div - 1) / div, (size.h * mul + div - 1) / div);
            }
    };
    //
    struct update_pixel_mode
    {
        MERROR  operator() (MyProcessedParams* pParams) {
            MINT32 const openId    = pParams->mOpenId;
            MUINT const sensorMode = pParams->mSensorMode;
            MUINT const sensorFps  = pParams->mSensorFps;
            MUINT32 pixelMode;
            //
            IHalSensor* pSensorHalObj = NULL;
            IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
            //
            if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return DEAD_OBJECT; }

            pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, openId);
            if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return DEAD_OBJECT; }

            pSensorHalObj->sendCommand(
                    pHalSensorList->querySensorDevIdx(openId),
                    SENSOR_CMD_GET_SENSOR_PIXELMODE,
                    (MUINTPTR)(&sensorMode),
                    (MUINTPTR)(&sensorFps),
                    (MUINTPTR)(&pixelMode));

            pSensorHalObj->destroyInstance(LOG_TAG);

            if( pixelMode != 0 && pixelMode != 1 ) {
                MY_LOGE("Un-supported pixel mode %d", pixelMode);
                return BAD_VALUE;
            }
            //
            pParams->mPixelMode = pixelMode;
            return OK;
        }
    };
    //
    if( OK != update_pixel_mode()(this) )
        return UNKNOWN_ERROR;
    //
    NSCamHW::HwInfoHelper infoHelper(mOpenId);
    MSize const sensorSize = mSensorSize;
    //
#define MIN_RRZO_EIS_W          (1280)
#define EIS_RATIO_10X           (12)  // src/dst = 1.2
#define RRZO_SCALE_RATIO_10X    (4)
#if 0
#define MAX_PREVIEW_W           (2560)
#else
#define MAX_PREVIEW_W           (1920)
#endif
#define MAX_RRZO_HW_W           (MAX_PREVIEW_W*EIS_RATIO_10X/10)
#define CHECK_TARGET_SIZE(_msg_, _size_) \
        MY_LOGD_IF(0, "%s: target size(%dx%d)", _msg_, _size_.w, _size_.h);
    // estimate preview yuv max size
    MSize const max_preview_size = refine::align_2(
            MSize(MAX_PREVIEW_W, MAX_PREVIEW_W * sensorSize.h / sensorSize.w));
    //
    MSize maxYuvStreamSize;
    MSize largeYuvStreamSize;
    for (size_t i = 0; i < mConfigParams.vImage_Yuv_NonStall.size(); i++ )
    {
        MSize const streamSize = mConfigParams.vImage_Yuv_NonStall[i]->getImgSize();
        // if stream's size is suitable to use rrzo
        if( streamSize.w <= max_preview_size.w && streamSize.h <= max_preview_size.h )
            refine::not_smaller_than(maxYuvStreamSize, streamSize);
        else
            refine::not_smaller_than(largeYuvStreamSize, streamSize);
    }
    MY_LOGD_IF( !!maxYuvStreamSize, "max yuv stream size(%dx%d)",
            maxYuvStreamSize.w, maxYuvStreamSize.h);
    MY_LOGD_IF( !!largeYuvStreamSize, "large yuv stream size(%dx%d), burst capture",
                largeYuvStreamSize.w, largeYuvStreamSize.h );
    //
    MBOOL bEis = ( mbHasRecording &&
                   CamManager::getInstance()->getFirstUsingId()==mOpenId )?
                   MTRUE : MFALSE;
    MBOOL bDual =  CamManager::getInstance()->isMultiDevice();
    // use resized raw if
    // 1. raw sensor
    // 2. some streams need this
    if( mSensorStaticInfo.sensorType == SENSOR_TYPE_RAW )
    {
        //
        // currently, should always enable resized raw due to some reasons...
        //
        // initial value
        MSize target_rrzo_size = maxYuvStreamSize;
        CHECK_TARGET_SIZE("max yuv stream", target_rrzo_size);
        // apply limitations
        //  1. lower bounds
        {
            if ( bEis ) {
            MSize const min_rrzo_eis_size = refine::align_2(
                    MSize(MIN_RRZO_EIS_W, MIN_RRZO_EIS_W * sensorSize.h / sensorSize.w));
            //
            refine::not_smaller_than(target_rrzo_size, min_rrzo_eis_size);
            target_rrzo_size = refine::align_2(
                    refine::scale_roundup(target_rrzo_size, EIS_RATIO_10X, 10)
                        );
               CHECK_TARGET_SIZE("eis lower bound limitation", target_rrzo_size);
            }
            // sensor rrz hw limitation
            MSize const min_rrzo_sensor_scale = refine::align_2(
                    refine::scale_roundup(sensorSize, RRZO_SCALE_RATIO_10X, 10)
                    );
            refine::not_smaller_than(target_rrzo_size, min_rrzo_sensor_scale);
            CHECK_TARGET_SIZE("sensor rrz lower bound limitation", target_rrzo_size);
        }
        //  2. upper bounds
        {
            // MSize const max_rrzo_hw_size = refine::align_2(
            //         MSize(MAX_RRZO_HW_W, MAX_RRZO_HW_W * sensorSize.h / sensorSize.w));
            //
            // refine::not_larger_than(target_rrzo_size, max_rrzo_hw_size);
            refine::not_larger_than(target_rrzo_size, max_preview_size);
            CHECK_TARGET_SIZE("preview upper bound limitation", target_rrzo_size);
            refine::not_larger_than(target_rrzo_size, sensorSize);
            CHECK_TARGET_SIZE("sensor size upper bound limitation", target_rrzo_size);
        }
        //
        if ( mPixelMode == 1 && bDual )
        {
            MY_LOGD_IF( 1, "2-pixel mode(%d) sensor(%dx%d)",
                        mPixelMode, sensorSize.w, sensorSize.h);
#define GET_MAX_RRZO_W(_x)      ( (_x)/2 - 32 )
            MSize const max_2pixel_bin_rrzo_size = refine::align_2(
                    MSize( GET_MAX_RRZO_W(sensorSize.w),
                           GET_MAX_RRZO_W(sensorSize.w)*sensorSize.h/sensorSize.w )
                    );
#undef GET_MAX_RRZO_W
            refine::not_larger_than(target_rrzo_size, max_2pixel_bin_rrzo_size);
            CHECK_TARGET_SIZE("2-pixel bin upper bound limitation", target_rrzo_size);
        }
        MY_LOGD_IF(1, "rrzo size(%dx%d)", target_rrzo_size.w, target_rrzo_size.h);
        //
        mResizedrawSize = target_rrzo_size;
        getSensorOutputFmt(mSensorStaticInfo, 10, MFALSE, &mResizedrawFormat);
        // check hw limitation with pixel mode & stride
        if( MTRUE != infoHelper.alignPass1HwLimitation(
                    mPixelMode,
                    mResizedrawFormat,
                    MFALSE,
                    mResizedrawSize,
                    mResizedrawStride
                    )
          )
        {
            MY_LOGE("cannot align to hw limitation: resize");
            return BAD_VALUE;
        }
        MY_LOGD_IF(1, "rrzo size(%dx%d) stride %zu",
                mResizedrawSize.w, mResizedrawSize.h, mResizedrawStride);
    }
#undef CHECK_TARGET_SIZE
    //
    // use full raw, if
    // 1. jpeg stream (&& not met BW limit)
    // 2. raw stream
    // 3. or stream's size is beyond rrzo's limit
    MBOOL useImgo =
        (mbHasJpeg && ! mbHasRecording) ||
        mbHasRaw ||
        !!largeYuvStreamSize;
    if( useImgo )
    {
        mFullrawSize = sensorSize;
        getSensorOutputFmt(mSensorStaticInfo, 10, MTRUE, &mFullrawFormat);
        // check hw limitation with pixel mode & stride
        if( MTRUE != infoHelper.alignPass1HwLimitation(
                    mPixelMode,
                    mFullrawFormat,
                    MTRUE,
                    mFullrawSize,
                    mFullrawStride
                    )
          )
        {
            MY_LOGE("cannot align to hw limitation: full");
            return BAD_VALUE;
        }
        MY_LOGD_IF(1, "imgo size(%dx%d) stride %zu",
                mFullrawSize.w, mFullrawSize.h, mFullrawStride);
    }
    else
    {
        mFullrawSize = MSize(0,0);
    }
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::parsedAppRequest::
parse()
{
    struct categorize_img_stream
    {
        typedef KeyedVector< StreamId_T, sp<IImageStreamBuffer> >       IImageSBMapT;
        typedef KeyedVector< StreamId_T, sp<IImageStreamInfo> >         IImageInfoMapT;
        MERROR      operator()(
                IImageSBMapT& map,
                IImageInfoMapT* pMapRaw,
                IImageInfoMapT* pMapYuv,
                IImageInfoMapT* pMapJpeg
                )
        {
            for(size_t i = 0; i < map.size(); i++ )
            {
                sp<IImageStreamBuffer> buf = map.valueAt(i);
                if ( IImageStreamInfo const* pStreamInfo = buf->getStreamInfo() )
                {
                    IImageInfoMapT* pTargetMap = NULL;
                    switch( pStreamInfo->getImgFormat() )
                    {
                        //case eImgFmt_BAYER10: //TODO: not supported yet
                        //case eImgFmt_BAYER12:
                        //case eImgFmt_BAYER14:
                        case eImgFmt_RAW16:
                            pTargetMap = pMapRaw;
                            break;
                            //
                        case eImgFmt_BLOB:
                            pTargetMap = pMapJpeg;
                            break;
                            //
                        case eImgFmt_YV12:
                        case eImgFmt_NV21:
                        case eImgFmt_YUY2:
                        case eImgFmt_Y8:
                        case eImgFmt_Y16:
                            pTargetMap = pMapYuv;
                            break;
                            //
                        default:
                            MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                            break;
                    }
                    if( pTargetMap == NULL ) {
                        MY_LOGE("cannot get target map");
                        return UNKNOWN_ERROR;
                    }
                    //
                    pTargetMap->add(
                            pStreamInfo->getStreamId(),
                            const_cast<IImageStreamInfo*>(pStreamInfo)
                            );
                }
            }
            return OK;
        }
    };
    //
    pAppMetaControl = pRequest->vIMetaBuffers[0]->tryReadLock("parsedAppRequest");
    if( ! pAppMetaControl ) {
        MY_LOGE("cannot get control meta");
        return UNKNOWN_ERROR;
    }
    //
    CHECK_ERROR(
            categorize_img_stream() (
                pRequest->vIImageBuffers,
                &vIImageInfos_Raw, &vIImageInfos_Yuv, NULL
                )
            );
    CHECK_ERROR(
            categorize_img_stream() (
                pRequest->vOImageBuffers,
                &vOImageInfos_Raw, &vOImageInfos_Yuv, &vOImageInfos_Jpeg
                )
            );
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineHdrImp::
PipelineHdrImp(
    MINT32 const openId,
    android::String8 const& name,
    wp<IPipelineModelMgr::IAppCallback> pAppCallback
)
    : mOpenId(openId)
    , mName(name)
    , mpAppCallback(pAppCallback)
    //
    , mParams(openId)
    //
    , mpHalMeta_DynamicP1()
    , mpAppMeta_DynamicP1()
    , mpAppMeta_DynamicHdr()
    , mpAppMeta_DynamicP2()
    , mpAppMeta_DynamicJpeg()
    , mpAppMeta_Control()
    //
    , mpHalImage_P1_Raw()
    , mpHalImage_P1_ResizerRaw()
    , mpHalImage_Jpeg_YUV()
    , mpHalImage_Thumbnail_YUV()
    //
    , mvAppYuvImage()
    , mpAppImage_Jpeg()
    //
	, mP1ImageStreamReuse()
    , mJpegRotationEnable(MFALSE)
	//
	, mpCamMgr(CamManager::getInstance())
    , mpDeviceHelper()
{
    mTimestamp = NSCam::Utils::TimeTool::getReadableTime();
    //
    int32_t enable = ::property_get_int32("jpeg.rotation.enable", 1);
    mJpegRotationEnable = (enable & 0x1)? MTRUE : MFALSE;
    MY_LOGD("Jpeg Rotation enable: %d", mJpegRotationEnable);
    // enable performance mode
    mPerf = std::unique_ptr<IPerf>(HDRPerfFactory::getIPerf());
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineHdrImp::
~PipelineHdrImp()
{
    FUNC_START;
    // disable performance mode
    mPerf.reset();

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PipelineHdrImp::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
/**
 * Given:
 *      App input meta streams
 *      App in/out image streams
 *
 * Action:
 *      Determine CONFIG stream set
 *      Determine I/O streams of each node
 *      Prepare Hal stream pools
 *      Configure each node (with their streams)
 *
 */
MERROR
PipelineHdrImp::
configure(
    PipeConfigParams const& rConfigParams,
    android::sp<IPipelineModel> pOldPipeline
)
{
    CAM_TRACE_NAME("PipelineHdrImp::configure");
    FUNC_START;
    RWLock::AutoWLock _l(mRWLock);
    //
    mParams.update(rConfigParams, MFALSE);
    //
    MY_LOGD_IF(mpPipelineContext.get(), "strong count %d", mpPipelineContext->getStrongCount());
    mpPipelineContext = PipelineContext::create("HdrPipeline");
    //
    if ( pOldPipeline.get() )
        mpOldCtx = pOldPipeline->getContext();
    MY_LOGD_IF(  mpOldCtx.get(), "old strong count %d", mpOldCtx->getStrongCount());
    //
    if ( mpOldCtx.get() )
    {
#if 1
        CHECK_ERROR(mpOldCtx->waitUntilNodeDrained(0x01));
#else
        CHECK_ERROR(mpOldCtx->waitUntilDrained());
#endif
    }
    //
    mpDeviceHelper = new CamManager::UsingDeviceHelper(mOpenId);
    //
    CHECK_ERROR(mpPipelineContext->beginConfigure(mpOldCtx));
    //
    {
        CAM_TRACE_NAME("PipelineHdrImp::BWC");
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getOpenId());
        IScenarioControl::ControlParam param;
        param.scenario   =
            mParams.mbHasJpeg ?
            IScenarioControl::Scenario_StreamingWithJpeg :
            IScenarioControl::Scenario_NormalPreivew;
        param.sensorSize = mParams.mSensorSize;
        param.sensorFps  = mParams.mSensorFps;

        CHECK_ERROR(pScenarioCtrl->exitScenario());
        CHECK_ERROR(pScenarioCtrl->enterScenario(param));
        CHECK_ERROR(mpPipelineContext->setScenarioControl(pScenarioCtrl));
    }
    // create IStreamInfos
    CHECK_ERROR(setupAppStreamsLocked(rConfigParams));
    CHECK_ERROR(setupHalStreamsLocked(rConfigParams));
    //
    // config stream
    CHECK_ERROR(configContextLocked_Streams(mpPipelineContext));
    // config node
    CHECK_ERROR(configContextLocked_Nodes(mpPipelineContext));
    // config pipeline
    CHECK_ERROR(configContextLocked_Pipeline(mpPipelineContext));
    //
    CHECK_ERROR(mpPipelineContext->endConfigure(true));
    mpOldCtx = NULL;
    //
    CHECK_ERROR(configRequestRulesLocked());
    //
    mpDeviceHelper->configDone();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
submitRequest(AppRequest& request)
{
    CAM_TRACE_CALL();

    parsedAppRequest aParsedRequest(&request);
    CHECK_ERROR( aParsedRequest.parse() );
    //
    RWLock::AutoRLock _l(mRWLock);
    //
#if ENABLE_HDRNODE
    std::vector<HDRCaptureParam> vOrigCaptureParams;

    // get HDR capture parameters
    CHECK_ERROR( HDRNode::getCaptureParamSet(
                getOpenId(), mvHdrCaptureParams, &vOrigCaptureParams) );

    // the number of capture frames
    const std::size_t captureFrames = mvHdrCaptureParams.size();

    // the number of delayed frames
    const std::size_t delayedFrames = vOrigCaptureParams.size();

    // add delay frames if reusing pass1 resource
    if ( MTRUE == mPass1Resource.getReuseFlag() )
    {
        // then append the amount of the original capture parameters
        // at the end of HDR capture parameters as a series of delayed frames
        //
        // even though capture can leverage per-frame control, however,
        // to avoid unstable exposure variation is seen on the display,
        // extra frame dealyed time is needed for AE convergence
        mvHdrCaptureParams.insert(mvHdrCaptureParams.begin(),
                vOrigCaptureParams.begin(), vOrigCaptureParams.end());
    }
    //
    MERROR err = OK;
    for (std::size_t frm_idx = 0; frm_idx < mvHdrCaptureParams.size(); frm_idx++ ) {
        //
        evaluateRequestResult evaluateResult;
        //
        const MBOOL isDelayedFrame = (frm_idx >= captureFrames);
        //
        CHECK_ERROR( evaluateRequestLocked(
                    aParsedRequest, evaluateResult, (frm_idx==0), isDelayedFrame) );
        //
        CHECK_ERROR( refineRequestMetaStreamBuffersLocked(
                    evaluateResult, frm_idx, delayedFrames,
                    (frm_idx+1) == mvHdrCaptureParams.size()) );
        //
        MY_LOGD("hdr req:%zu frm:%zu", mvHdrCaptureParams.size(), frm_idx);
        RequestBuilder builder;
        builder.setRootNode( evaluateResult.roots );
        builder.setNodeEdges( evaluateResult.edges );
        //
        if (frm_idx==0)
        {
            // main request
            for ( size_t i = 0; i < evaluateResult.vUpdatedImageInfos.size(); i++ )
            {
                builder.replaceStreamInfo(
                        evaluateResult.vUpdatedImageInfos.keyAt(i),
                        evaluateResult.vUpdatedImageInfos.valueAt(i)
                        );

                const sp<IImageStreamInfo>& streamInfo(
                        evaluateResult.vUpdatedImageInfos.valueAt(i));
                MY_LOGD("replace streaminfo id(%zu name(%s)",
                        streamInfo->getStreamId(), streamInfo->getStreamName());
            }

            builder.setIOMap(
                    eNODEID_P1Node,
                    IOMapSet().add(
                        IOMap()
                        .addOut(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00)
                        ),
                    IOMapSet().add(
                        IOMap()
                        .addIn(mpAppMeta_Control->getStreamId())
                        .addIn(eSTREAMID_META_PIPE_CONTROL)
                        .addOut(eSTREAMID_META_APP_DYNAMIC_01)
                        .addOut(eSTREAMID_META_PIPE_DYNAMIC_01)
                        )
                    );
            //
            IOMap hdrMap = IOMap();
            hdrMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00);
            for ( size_t i=0; i<request.vOImageBuffers.size(); i++)
            {
                MY_LOGD("hdrnode output streamid: %zu", request.vOImageBuffers.keyAt(i));
                switch( request.vOImageBuffers[i]->getStreamInfo()->getImgFormat() )
                {
                    case eImgFmt_YV12:
                    case eImgFmt_NV21:
                    case eImgFmt_YUY2:
                    case eImgFmt_Y8:
                    case eImgFmt_Y16:
                        MY_LOGD("hdrnode add output streamid");
                        hdrMap.addOut(request.vOImageBuffers.keyAt(i));
                        break;
                        //
                    default:
                        break;
                }
            }
            hdrMap.addOut(eSTREAMID_IMAGE_PIPE_YUV_JPEG_00)
                  .addOut(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00);
            //
            builder.setIOMap(
                        eNODEID_HdrNode,
                        IOMapSet().add(
                            hdrMap
                            ),
                        IOMapSet().add(
                            IOMap()
                            .addIn(mpAppMeta_Control->getStreamId())
                            .addIn(eSTREAMID_META_PIPE_DYNAMIC_01)
                            .addOut(eSTREAMID_META_APP_DYNAMIC_HDR)
                            .addOut(eSTREAMID_META_PIPE_DYNAMIC_HDR)
                            )
                        );
            //
            IOMap jpgMap = IOMap();
            jpgMap.addIn(eSTREAMID_IMAGE_PIPE_YUV_JPEG_00)
                  .addIn(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00);
            for ( size_t i=0; i<request.vOImageBuffers.size(); i++)
            {
                MY_LOGD("jpegnode output streamid: %zu", request.vOImageBuffers.keyAt(i));
                switch( request.vOImageBuffers[i]->getStreamInfo()->getImgFormat() )
                {
                    case eImgFmt_BLOB:
                        MY_LOGD("jpegnode add output streamid");
                        jpgMap.addOut(request.vOImageBuffers.keyAt(i));
                        break;
                        //
                    default:
                        break;
                }
            }
            jpgMap.addOut(mpAppImage_Jpeg->getStreamId());
            //
            builder.setIOMap(
                        eNODEID_JpegNode,
                        IOMapSet().add(
                            jpgMap
                            ),
                        IOMapSet().add(
                            IOMap()
                            .addIn(mpAppMeta_Control->getStreamId())
                            .addIn(eSTREAMID_META_PIPE_DYNAMIC_HDR)
                            .addOut(eSTREAMID_META_APP_DYNAMIC_JPEG)
                            )
                        );
        }
        else
        {
            // sub-request
            builder.setIOMap(
                    eNODEID_P1Node,
                    IOMapSet().add(
                        IOMap()
                        .addOut(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00)
                        ),
                    IOMapSet().add(
                        IOMap()
                        .addIn(mpAppMeta_Control->getStreamId())
                        .addIn(eSTREAMID_META_PIPE_CONTROL)
                        .addOut(eSTREAMID_META_APP_DYNAMIC_01)
                        .addOut(eSTREAMID_META_PIPE_DYNAMIC_01)
                        )
                    );

            if (!isDelayedFrame)
            {
                builder.setIOMap(
                        eNODEID_HdrNode,
                        IOMapSet().add(
                            IOMap()
                            .addIn(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00)
                            ),
                        IOMapSet().add(
                            IOMap()
                            .addIn(mpAppMeta_Control->getStreamId())
                            .addIn(eSTREAMID_META_PIPE_DYNAMIC_01)
                            )
                        );
            }
        }

        // set app and hal stream buffers
        {
#define setStreamBuffers(_sb_type_, _type_, _vStreamBuffer_, _builder_)    \
            do {                                                                   \
                for (size_t i = 0; i < _vStreamBuffer_.size(); i++ )               \
                {                                                                  \
                    StreamId_T streamId                = _vStreamBuffer_.keyAt(i); \
                    sp<_sb_type_> buffer = _vStreamBuffer_.valueAt(i);             \
                    _builder_.set##_type_##StreamBuffer(streamId, buffer);         \
                }                                                                  \
            } while(0)
            //
            if (frm_idx==0)
            {
                setStreamBuffers(
                        IImageStreamBuffer, Image, evaluateResult.vAppImageBuffers, builder);
            }
            setStreamBuffers(
                    HalImageStreamBuffer, Image, evaluateResult.vHalImageBuffers, builder);
            setStreamBuffers(
                    IMetaStreamBuffer, Meta, evaluateResult.vAppMetaBuffers, builder);
            setStreamBuffers(
                    HalMetaStreamBuffer, Meta, evaluateResult.vHalMetaBuffers, builder);
#undef setStreamBuffers
        }
        //
        MY_LOGD("construct hdr req:%zu frm:%zu", mvHdrCaptureParams.size(), frm_idx);
        //
        sp<IPipelineFrame> pFrame = builder
                .updateFrameCallback((frm_idx==0)? this : NULL)
                .build(request.requestNo, mpPipelineContext);
        MY_LOGD("construct hdr done req:%zu frm:%zu", mvHdrCaptureParams.size(), frm_idx);
        if( ! pFrame.get() ) {
            MY_LOGE("build pipeline frame failed");
            return UNKNOWN_ERROR;
        }
        err = mpPipelineContext->queue(pFrame);
        if( OK != err ) {
            MY_LOGE("queue pipeline frame failed(%s)", strerror(-err));
            return err;
        }
    }
    //
    return err;
#else
    RequestBuilder builder;
    //
    evaluateRequestResult evaluateResult;
    //
    CHECK_ERROR( evaluateRequestLocked(aParsedRequest, evaluateResult, MTRUE) );
    //
    CHECK_ERROR( refineRequestMetaStreamBuffersLocked(evaluateResult) );
    //
    builder.setRootNode( evaluateResult.roots );
    builder.setNodeEdges( evaluateResult.edges );
    //
    for( size_t i = 0; i < evaluateResult.vUpdatedImageInfos.size(); i++ )
    {
        builder.replaceStreamInfo(
                evaluateResult.vUpdatedImageInfos.keyAt(i),
                evaluateResult.vUpdatedImageInfos.valueAt(i)
                );
    }
    //
#define try_setIOMap(_nodeId_)                                                        \
    do {                                                                              \
        ssize_t idx_image = evaluateResult.nodeIOMapImage.indexOfKey(_nodeId_);       \
        ssize_t idx_meta  = evaluateResult.nodeIOMapMeta.indexOfKey(_nodeId_);        \
        builder.setIOMap(                                                             \
                _nodeId_,                                                             \
                (0 <= idx_image ) ?                                                   \
                evaluateResult.nodeIOMapImage.valueAt(idx_image) : IOMapSet::empty(), \
                (0 <= idx_meta ) ?                                                    \
                evaluateResult.nodeIOMapMeta.valueAt(idx_meta) : IOMapSet::empty()    \
                );                                                                    \
    } while(0)
    //
    try_setIOMap(eNODEID_P1Node);
    try_setIOMap(eNODEID_P2Node);
    try_setIOMap(eNODEID_JpegNode);
    //
#undef try_setIOMap
    //
#define setStreamBuffers(_sb_type_, _type_, _vStreamBuffer_, _builder_)    \
    do {                                                                   \
        for (size_t i = 0; i < _vStreamBuffer_.size(); i++ )               \
        {                                                                  \
            StreamId_T streamId                = _vStreamBuffer_.keyAt(i); \
            sp<_sb_type_> buffer = _vStreamBuffer_.valueAt(i);             \
            _builder_.set##_type_##StreamBuffer(streamId, buffer);         \
        }                                                                  \
    } while(0)
    //
    setStreamBuffers(IImageStreamBuffer  , Image, evaluateResult.vAppImageBuffers, builder);
    setStreamBuffers(HalImageStreamBuffer, Image, evaluateResult.vHalImageBuffers, builder);
    setStreamBuffers(IMetaStreamBuffer   , Meta , evaluateResult.vAppMetaBuffers , builder);
    setStreamBuffers(HalMetaStreamBuffer , Meta , evaluateResult.vHalMetaBuffers , builder);
#undef setStreamBuffers
    //
    //
    sp<IPipelineFrame> pFrame = builder
        .updateFrameCallback(this)
        .build(request.requestNo, mpPipelineContext);
    if( ! pFrame.get() )
        return UNKNOWN_ERROR;
    //
    return mpPipelineContext->queue(pFrame);
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
beginFlush()
{
    FUNC_START;
    //
    if( mpPipelineContext.get() )
        mpPipelineContext->flush();
    else
        MY_LOGW("no context");
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineHdrImp::
endFlush()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineHdrImp::
waitDrained()
{
    CAM_TRACE_CALL();

    FUNC_START;
    if( mpPipelineContext.get() )
        mpPipelineContext->waitUntilDrained();
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<PipelineContext>
PipelineHdrImp::
getContext()
{
    FUNC_START;
    if( mpPipelineContext.get() )
        return mpPipelineContext;
    FUNC_END;
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineHdrImp::
skipStream(
    MBOOL skipJpeg,
    IImageStreamInfo* pStreamInfo
) const
{
    if  (
            skipJpeg
        &&  pStreamInfo->getImgFormat() == HAL_PIXEL_FORMAT_BLOB
        &&  pStreamInfo->getImgSize().size() >= 1920*1080
        )
    {
 //&& limited mode
        return MTRUE;
    }

    return MFALSE;
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
        default:
            MY_LOGE("format not support yet %d", imgFormat);
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
MVOID
PipelineHdrImp::
evaluatePreviewSize(
    PipeConfigParams const& rConfigParams,
    MSize &rSize
)
{
    sp<IImageStreamInfo> pStreamInfo;
    int consumer_usage = 0;
    int allocate_usage = 0;
    int maxheight = rSize.h;
    int prevwidth = 0;
    int prevheight = 0;
    for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
        if  ( (pStreamInfo = rConfigParams.vImage_Yuv_NonStall[i]) != 0 ) {
            consumer_usage = pStreamInfo->getUsageForConsumer();
            allocate_usage = pStreamInfo->getUsageForAllocator();
            MY_LOGD("consumer : %X, allocate : %X", consumer_usage, allocate_usage);
            if(consumer_usage & GRALLOC_USAGE_HW_TEXTURE) {
                prevwidth = pStreamInfo->getImgSize().w;
                prevheight = pStreamInfo->getImgSize().h;
                break;
            }
            if(consumer_usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) {
                continue;
            }
            prevwidth = pStreamInfo->getImgSize().w;
            prevheight = pStreamInfo->getImgSize().h;
        }
    }
    if(prevwidth == 0 || prevheight == 0)
        return ;
    rSize.h = prevheight * rSize.w / prevwidth;
    if(maxheight < rSize.h) {
        MY_LOGW("Warning!!,  scaled preview height(%d) is larger than max height(%d)", rSize.h, maxheight);
        rSize.h = maxheight;
    }
    MY_LOGD("evaluate preview size : %dx%d", prevwidth, prevheight);
    MY_LOGD("FD buffer size : %dx%d", rSize.w, rSize.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
setupAppStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    mvStreamDurations.clear();

    //App:Meta:Control
    {
        sp<IMetaStreamInfo> pStreamInfo;
        if  ( (pStreamInfo = rConfigParams.pMeta_Control) != 0 )
        {
            mpAppMeta_Control = pStreamInfo;
            pStreamInfo->setMaxBufNum(10);
        }
    }

    //App:dynamic
    if ( mParams.mbUseP1Node )
    {
        //App:Meta:01
        //   pass1 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP1",
                    eSTREAMID_META_APP_DYNAMIC_01,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicP1 = pStreamInfo;
    }

#if ENABLE_HDRNODE
    if ( mParams.mbUseHdrNode )
    {
        //App:Meta:P2
        //   Hdr result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicHdr",
                    eSTREAMID_META_APP_DYNAMIC_HDR,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicHdr = pStreamInfo;
    }
#else
    if ( mParams.mbUseP2Node )
    {
        //App:Meta:P2
        //   P2 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP2",
                    eSTREAMID_META_APP_DYNAMIC_02,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicP2 = pStreamInfo;
    }
#endif

    if ( mParams.mbUseJpegNode )
    {
        //App:Meta:Jpeg
        //   Jpeg result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:Jpeg",
                    eSTREAMID_META_APP_DYNAMIC_JPEG,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicJpeg = pStreamInfo;
    }

    //App:Image
    {
        sp<IImageStreamInfo> pStreamInfo;
        //
        // CHECK
        //App:Image:Raw
        if  ( (pStreamInfo = rConfigParams.pImage_Raw) != 0 ) {
            MY_LOGF("Not supported");
            pStreamInfo->setMaxBufNum(1);
            //
            mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Raw_Duration.minDuration
                    );
        }
        //
        //App:Image:Jpeg:Stall
        if  ( (pStreamInfo = rConfigParams.pImage_Jpeg_Stall) != 0 ) {
            mpAppImage_Jpeg = pStreamInfo;
            pStreamInfo->setMaxBufNum(1);
            //
            mvStreamDurations.add(
                    pStreamInfo->getStreamId(),
                    rConfigParams.mImage_Jpeg_Duration.minDuration
                    );
        }
        //
        //App:Image:Yuv:NotStall
        for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( (pStreamInfo = rConfigParams.vImage_Yuv_NonStall[i]) != 0 ) {
                mvAppYuvImage.add(pStreamInfo->getStreamId(), pStreamInfo);
                pStreamInfo->setMaxBufNum(8);
                //
                if( i >= rConfigParams.vImage_Yuv_Duration.size() ) {
                    MY_LOGE("not enough yuv duration for streams");
                    continue;
                }
                mvStreamDurations.add(
                        pStreamInfo->getStreamId(),
                        rConfigParams.vImage_Yuv_Duration[i].minDuration
                        );
            }
        }
        //
        // dump durations
        String8 durations = String8("durations:");
        for( size_t i = 0; i < mvStreamDurations.size(); i++) {
            durations += String8::format("(stream %#" PRIx64 ": %lld) ",
                    mvStreamDurations.keyAt(i), (long long int)mvStreamDurations.valueAt(i));
        }
        MY_LOGD("%s", durations.string());
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
setupHalStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    //Hal:Meta
    //
    if( 1 )
    {
        //Hal:Meta:Control
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:Control",
                    eSTREAMID_META_PIPE_CONTROL,
                    eSTREAMTYPE_META_IN,
                    10, 1
                    );
        mpHalMeta_Control = pStreamInfo;
    }
    //
    if ( mParams.mbUseP1Node )
    {
        //Hal:Meta:01
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P1:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_01,
                    eSTREAMTYPE_META_INOUT,
                    10, 1
                    );
        mpHalMeta_DynamicP1 = pStreamInfo;
    }
    //
#if ENABLE_HDRNODE
    if ( mParams.mbUseHdrNode )
    {
        //Hal:Meta:Hdr
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:Hdr:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_HDR,
                    eSTREAMTYPE_META_INOUT,
                    10, 1
                    );
        mpHalMeta_DynamicHdr = pStreamInfo;
    }
#else
    //
    if ( mParams.mbUseP2Node )
    {
        //Hal:Meta:02
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P2:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_02,
                    eSTREAMTYPE_META_INOUT,
                    10, 1
                    );
        mpHalMeta_DynamicP2 = pStreamInfo;
    }
#endif
    // p1node image port: imgo/rrzo
    mPass1Resource.setStreamCapacity(2);
    //Hal:Image
    if ( mParams.mbUseP1Node &&
            !! mParams.mFullrawSize.size() )
    {
        sp<IImageStreamInfo> pStreamInfo;
        // p1: fullsize
        MSize const& size = mParams.mFullrawSize;
        MINT const format = mParams.mFullrawFormat;
        size_t const stride = mParams.mFullrawStride;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
        //
        MBOOL bDone = MFALSE;
        if ( mpOldCtx.get() &&
             OK == mpOldCtx->queryStream(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00, pStreamInfo) &&
             pStreamInfo.get() )
        {
            if ( pStreamInfo->getImgSize().w == size.w &&
                 pStreamInfo->getImgSize().h == size.h &&
                 pStreamInfo->getImgFormat() == format &&
                 pStreamInfo->getBufPlanes().itemAt(0).rowStrideInBytes == stride &&
                 pStreamInfo->getUsageForAllocator() == usage )
            {
                MY_LOGD_IF( 1, "stream could be reused:%u" PRIx64 , eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00);
                //mvReuseStream.add(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00);
                mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                                                   pass1Resource::StreamStatus::eStatus_Reuse);
                bDone = MTRUE;
            }
        }
        //
        if ( !bDone )
        {
            pStreamInfo = createRawImageStreamInfo(
                    "Hal:Image:P1:Fullraw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    6, 0,
                    usage, format, size, stride
            );
            //
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                                               pass1Resource::StreamStatus::eStatus_Inited);
        }
        //
        MY_LOGD_IF( 1, "setup streamid(%d):%p",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00, pStreamInfo.get() );
        mpHalImage_P1_Raw = pStreamInfo;
    } else if ( mParams.mbUseP1Node &&
                ! mParams.mFullrawSize.size() )
    {
        sp<IImageStreamInfo> pStreamInfo;
        if ( mpOldCtx.get() &&
             OK == mpOldCtx->queryStream(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00, pStreamInfo) &&
             ! pStreamInfo.get() )
        {
            MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%d" ,
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00);
            //mvReuseStream.add(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00);
            mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                                               pass1Resource::StreamStatus::eStatus_NoNeed);
            mpHalImage_P1_Raw = pStreamInfo;
        }
    }

    if ( mParams.mbUseP1Node &&
            !! mParams.mResizedrawSize.size() )
    {
        sp<IImageStreamInfo> pStreamInfo;
        // p1: resize
        MSize const& size = mParams.mResizedrawSize;
        MINT const format = mParams.mResizedrawFormat;
        size_t const stride = mParams.mResizedrawStride;
        MUINT const usage = 0;
        //
        MBOOL bDone = MFALSE;
        if ( mpOldCtx.get() &&
             OK == mpOldCtx->queryStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00, pStreamInfo) &&
             pStreamInfo.get() )
        {
            if ( pStreamInfo->getImgSize().w == size.w &&
                 pStreamInfo->getImgSize().h == size.h &&
                 pStreamInfo->getImgFormat() == format &&
                 pStreamInfo->getBufPlanes().itemAt(0).rowStrideInBytes == stride &&
                 pStreamInfo->getUsageForAllocator() == usage )
            {
                MY_LOGD_IF( 1, "stream could be reused:%d", eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00);
                //mvReuseStream.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00);
                mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                                                   pass1Resource::StreamStatus::eStatus_Reuse);
                bDone = MTRUE;
            }
        }
        //
        if ( !bDone )
        {
            pStreamInfo = createRawImageStreamInfo(
                    "Hal:Image:P1:Resizeraw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    6, 0,
                    usage, format, size, stride
            );
            //
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                                               pass1Resource::StreamStatus::eStatus_Inited);
            }
        //
        MY_LOGD_IF( 1, "setup streamid(%d):%p",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00, pStreamInfo.get() );
        mpHalImage_P1_ResizerRaw = pStreamInfo;
    } else if ( mParams.mbUseP1Node &&
                ! mParams.mResizedrawSize.size() )
    {
        sp<IImageStreamInfo> pStreamInfo;
        if ( mpOldCtx.get() &&
             OK == mpOldCtx->queryStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00, pStreamInfo) &&
             ! pStreamInfo.get() )
        {
            MY_LOGD_IF( 1, "case of previous and current pipeline have no streamid:%d",
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00);
            //mvReuseStream.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00);
            mPass1Resource.updateStreamStatus( eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                                               pass1Resource::StreamStatus::eStatus_NoNeed);
            mpHalImage_P1_ResizerRaw = pStreamInfo;
        }
    }

    //Hal:Image:YUY2 for jpeg & thumbnail
    if ( mParams.mbUseJpegNode )
    {
        //Hal:Image:YUY2 for jpeg
        {
            MSize const& size = rConfigParams.pImage_Jpeg_Stall->getImgSize();
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = 0;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvJpeg",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 0,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_Jpeg_YUV = pStreamInfo;
        }
        //
        //Hal:Image:YUY2 for thumbnail
        {
            MSize const size(-1L, -1L); //unknown now
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = 0;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvThumbnail",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 0,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_Thumbnail_YUV = pStreamInfo;
            MY_LOGD("streamId:%#" PRIx64 " %s %p", pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pStreamInfo.get());
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_Streams(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();
#define BuildStream(_type_, _IStreamInfo_)                                     \
    do {                                                                       \
        if( _IStreamInfo_.get() ) {                                            \
            MERROR err;                                                        \
            if ( OK != (err = StreamBuilder(_type_, _IStreamInfo_)             \
                    .build(pContext)) )                                        \
            {                                                                  \
                MY_LOGE("StreamBuilder fail stream %d of type %zu", \
                    _type_, _IStreamInfo_->getStreamId());                    \
                return err;                                                    \
            }                                                                  \
        }                                                                      \
    } while(0)

    BuildStream(eStreamType_META_HAL, mpHalMeta_Control);
    BuildStream(eStreamType_META_HAL, mpHalMeta_DynamicP1);
#if ENABLE_HDRNODE
    BuildStream(eStreamType_META_HAL, mpHalMeta_DynamicHdr);
#else
    BuildStream(eStreamType_META_HAL, mpHalMeta_DynamicP2);
#endif
    //
    BuildStream(eStreamType_META_APP, mpAppMeta_DynamicP1);
#if ENABLE_HDRNODE
    BuildStream(eStreamType_META_APP, mpAppMeta_DynamicHdr);
#else
    BuildStream(eStreamType_META_APP, mpAppMeta_DynamicP2);
#endif
    BuildStream(eStreamType_META_APP, mpAppMeta_DynamicJpeg);
    BuildStream(eStreamType_META_APP, mpAppMeta_Control);
    //
#if 1
    if ( mPass1Resource.checkStreamsReusable() )
    {
        if ( mpHalImage_P1_Raw.get() )
            CHECK_ERROR( pContext->reuseStream(mpHalImage_P1_Raw) );
        if ( mpHalImage_P1_ResizerRaw.get() )
            CHECK_ERROR( pContext->reuseStream(mpHalImage_P1_ResizerRaw) );
        //
        MY_LOGD_IF( 1, "Reuse: p1 full raw(%p); resized raw(%p)",
                    mpHalImage_P1_Raw.get(), mpHalImage_P1_ResizerRaw.get());
        mPass1Resource.setReuseFlag(MTRUE);
    }
    else
    {
        BuildStream(eStreamType_IMG_HAL_POOL   , mpHalImage_P1_Raw);
        BuildStream(eStreamType_IMG_HAL_POOL   , mpHalImage_P1_ResizerRaw);
        MY_LOGD_IF( 1, "New: p1 full raw(%p); resized raw(%p)",
                    mpHalImage_P1_Raw.get(), mpHalImage_P1_ResizerRaw.get());
        mPass1Resource.setReuseFlag(MFALSE);
    }
#else
        BuildStream(eStreamType_IMG_HAL_POOL   , mpHalImage_P1_Raw);
        BuildStream(eStreamType_IMG_HAL_POOL   , mpHalImage_P1_ResizerRaw);
#endif
    //
    if ( !mJpegRotationEnable )
        BuildStream(eStreamType_IMG_HAL_POOL   , mpHalImage_Jpeg_YUV);
    else
        BuildStream(eStreamType_IMG_HAL_RUNTIME   , mpHalImage_Jpeg_YUV);
    BuildStream(eStreamType_IMG_HAL_RUNTIME, mpHalImage_Thumbnail_YUV);
    //
    for (size_t i = 0; i < mvAppYuvImage.size(); i++ )
    {
        BuildStream(eStreamType_IMG_APP, mvAppYuvImage[i]);
    }
    BuildStream(eStreamType_IMG_APP, mpAppImage_Jpeg);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_Nodes(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

    if( mParams.mbUseP1Node )
        CHECK_ERROR( configContextLocked_P1Node(pContext) );
#if ENABLE_HDRNODE
    if( mParams.mbUseHdrNode )
        CHECK_ERROR( configContextLocked_HdrNode(pContext) );
#else
    if( mParams.mbUseP2Node )
        CHECK_ERROR( configContextLocked_P2Node(pContext) );
#endif
    if( mParams.mbUseJpegNode )
        CHECK_ERROR( configContextLocked_JpegNode(pContext) );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_Pipeline(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

    NodeSet roots;
    {
        roots.add(eNODEID_P1Node);
    }
    NodeEdgeSet edges;
    {
#if ENABLE_HDRNODE
        if( mParams.mbUseP1Node && mParams.mbUseHdrNode )
            edges.addEdge(eNODEID_P1Node, eNODEID_HdrNode);
        if( mParams.mbUseHdrNode && mParams.mbUseJpegNode )
            edges.addEdge(eNODEID_HdrNode, eNODEID_JpegNode);
#else
        if( mParams.mbUseP1Node && mParams.mbUseP2Node )
            edges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
        if( mParams.mbUseP2Node && mParams.mbUseJpegNode )
            edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);
#endif
    }
    //
    CHECK_ERROR(
            PipelineBuilder()
            .setRootNode(roots)
            .setNodeEdges(edges)
            .build(pContext)
            );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
#define add_stream_to_set(_set_, _IStreamInfo_)                                \
    do {                                                                       \
        if( _IStreamInfo_.get() ) { _set_.add(_IStreamInfo_->getStreamId()); } \
    } while(0)
//
#define setImageUsage( _IStreamInfo_, _usg_ )                                   \
    do {                                                                        \
        if( _IStreamInfo_.get() ) {                                             \
            builder.setImageStreamUsage( _IStreamInfo_->getStreamId(), _usg_ ); \
        }                                                                       \
    } while(0)
/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_P1Node(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

    typedef P1Node                  NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_P1Node;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = getOpenId();
        initParam.nodeId = nodeId;
        initParam.nodeName = "P1Node";
    }
    NodeT::ConfigParams cfgParam;
    {
        NodeT::SensorParams sensorParam(
        /*mode     : */mParams.mSensorMode,
        /*size     : */mParams.mSensorSize,
        /*fps      : */mParams.mSensorFps,
        /*pixelMode: */mParams.mPixelMode
        );
        //
        cfgParam.pInAppMeta        = mpAppMeta_Control;
        cfgParam.pInHalMeta        = mpHalMeta_Control;
        cfgParam.pOutAppMeta       = mpAppMeta_DynamicP1;
        cfgParam.pOutHalMeta       = mpHalMeta_DynamicP1;
        cfgParam.pOutImage_resizer = mpHalImage_P1_ResizerRaw;
        if( mpHalImage_P1_Raw.get() )
            cfgParam.pvOutImage_full.push_back(mpHalImage_P1_Raw);
        cfgParam.sensorParams        = sensorParam;
        cfgParam.pStreamPool_resizer = NULL;
        cfgParam.pStreamPool_full    = NULL;
        // cfgParam.pStreamPool_resizer = mpHalImage_P1_ResizerRaw.get() ?
        //    pContext->queryImageStreamPool(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00) : NULL;
        // cfgParam.pStreamPool_full = mpHalImage_P1_Raw.get() ?
        //    pContext->queryImageStreamPool(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00) : NULL;
        cfgParam.enableEIS = mpDeviceHelper->isFirstUsingDevice() ? MTRUE : MFALSE;
    }
    //
    sp<NodeActorT> pNode;
    sp<NodeActorT> pOldNode;
    MBOOL bHasOldNode = (mpOldCtx.get() && OK == mpOldCtx->queryNodeActor(nodeId, pOldNode));
    if ( MTRUE == mPass1Resource.getReuseFlag() && bHasOldNode )
    {
        NodeT::InitParams oldInitParam;
        pOldNode->getInitParam(oldInitParam);
        NodeT::ConfigParams oldCfgParam;
        pOldNode->getConfigParam(oldCfgParam);
        //
        if ( compareParamsLocked_P1Node(initParam, oldInitParam, cfgParam, oldCfgParam) )
        {
            MERROR err = pContext->reuseNode(nodeId);
            MY_LOGD_IF( 1, "check p1 state Old[%d] New[%d]",
                        mpOldCtx->queryINodeActor(nodeId)->getStatus(),
                        pContext->queryINodeActor(nodeId)->getStatus() );
            return err;
        }
    }
    if(bHasOldNode)
        pOldNode->getNodeImpl()->uninit(); // must uninit old P1 before call new P1 config
    //
    pNode = new NodeActorT( NodeT::createInstance() );
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mpHalMeta_Control);
    //
    add_stream_to_set(outStreamSet, mpHalImage_P1_Raw);
    add_stream_to_set(outStreamSet, mpHalImage_P1_ResizerRaw);
    add_stream_to_set(outStreamSet, mpAppMeta_DynamicP1);
    add_stream_to_set(outStreamSet, mpHalMeta_DynamicP1);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mpHalImage_P1_Raw        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mpHalImage_P1_ResizerRaw , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
compareParamsLocked_P1Node(
    P1Node::InitParams const& initParam1,  P1Node::InitParams const& initParam2,
    P1Node::ConfigParams const& cfgParam1, P1Node::ConfigParams const& cfgParam2
) const
{
    FUNC_START;
    if ( initParam1.openId != initParam2.openId ||
         initParam1.nodeId != initParam2.nodeId ||
         strcmp(initParam1.nodeName, initParam2.nodeName) )
        return MFALSE;
    //
    if ( cfgParam1.sensorParams.mode         != cfgParam2.sensorParams.mode ||
         cfgParam1.sensorParams.size         != cfgParam2.sensorParams.size ||
         cfgParam1.sensorParams.fps          != cfgParam2.sensorParams.fps ||
         cfgParam1.sensorParams.pixelMode    != cfgParam2.sensorParams.pixelMode ||
         cfgParam1.sensorParams.vhdrMode     != cfgParam2.sensorParams.vhdrMode)
        return MFALSE;
    //
    if ( ! cfgParam1.pInAppMeta.get()  || ! cfgParam2.pInAppMeta.get() ||
         ! cfgParam1.pOutAppMeta.get() || ! cfgParam2.pOutAppMeta.get() ||
         ! cfgParam1.pOutHalMeta.get() || ! cfgParam2.pOutHalMeta.get() ||
         cfgParam1.pInAppMeta->getStreamId()  != cfgParam2.pInAppMeta->getStreamId() ||
         cfgParam1.pOutAppMeta->getStreamId() != cfgParam2.pOutAppMeta->getStreamId() ||
         cfgParam1.pOutHalMeta->getStreamId() != cfgParam2.pOutHalMeta->getStreamId() )
        return MFALSE;
    //
    if ( ! cfgParam1.pOutImage_resizer.get() || ! cfgParam2.pOutImage_resizer.get() ||
        cfgParam1.pOutImage_resizer->getStreamId() != cfgParam2.pOutImage_resizer->getStreamId() )
        return MFALSE;
    //
    if ( cfgParam1.pvOutImage_full.size() != cfgParam2.pvOutImage_full.size() )
        return MFALSE;
    //
    for ( size_t i=0; i<cfgParam1.pvOutImage_full.size(); i++ ) {
        MBOOL bMatch = MFALSE;
        for ( size_t j=0; j<cfgParam2.pvOutImage_full.size(); j++ ) {
            if ( cfgParam1.pvOutImage_full.itemAt(i)->getStreamId() == cfgParam2.pvOutImage_full.itemAt(i)->getStreamId() )
            {
                bMatch = MTRUE;
                break;
            }
        }
        if ( !bMatch )
            return MFALSE;
    }
    //
    FUNC_END;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_P2Node(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

    typedef P2Node                  NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_P2Node;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = getOpenId();
        initParam.nodeId = nodeId;
        initParam.nodeName = "P2Node";
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta    = mpAppMeta_Control;
        cfgParam.pInHalMeta    = mpHalMeta_DynamicP1;
        cfgParam.pOutAppMeta   = mpAppMeta_DynamicP2;
        cfgParam.pOutHalMeta   = mpHalMeta_DynamicP2;
        //
        if( mpHalImage_P1_Raw.get() )
            cfgParam.pvInFullRaw.push_back(mpHalImage_P1_Raw);
        //
        cfgParam.pInResizedRaw = mpHalImage_P1_ResizerRaw;
        //
        for (size_t i = 0; i < mvAppYuvImage.size(); i++)
            cfgParam.vOutImage.push_back(mvAppYuvImage[i]);
        //
        if( mpHalImage_Jpeg_YUV.get() )
            cfgParam.vOutImage.push_back(mpHalImage_Jpeg_YUV);
        if( mpHalImage_Thumbnail_YUV.get() )
            cfgParam.vOutImage.push_back(mpHalImage_Thumbnail_YUV);
        //
    }
    //
    sp<NodeActorT> pNode = new NodeActorT( NodeT::createInstance(P2Node::PASS2_STREAM) );
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mpHalMeta_DynamicP1);
    add_stream_to_set(inStreamSet, mpHalImage_P1_Raw);
    add_stream_to_set(inStreamSet, mpHalImage_P1_ResizerRaw);
    //
    add_stream_to_set(outStreamSet, mpAppMeta_DynamicP2);
    add_stream_to_set(outStreamSet, mpHalMeta_DynamicP2);
    add_stream_to_set(outStreamSet, mpHalImage_Jpeg_YUV);
    add_stream_to_set(outStreamSet, mpHalImage_Thumbnail_YUV);
    //
    for (size_t i = 0; i < mvAppYuvImage.size(); i++)
        add_stream_to_set(outStreamSet, mvAppYuvImage[i]);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mpHalImage_P1_Raw        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mpHalImage_P1_ResizerRaw , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    //
    for (size_t i = 0; i < mvAppYuvImage.size(); i++)
        setImageUsage(mvAppYuvImage[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    setImageUsage(mpHalImage_Jpeg_YUV      , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mpHalImage_Thumbnail_YUV , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_HdrNode(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

#if ENABLE_HDRNODE
    typedef HDRNode                 NodeT;
#else
    typedef P2Node                  NodeT;
#endif
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_HdrNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = getOpenId();
        initParam.nodeId = nodeId;
        initParam.nodeName = "HdrNode";
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta    = mpAppMeta_Control;
        cfgParam.pInHalMeta    = mpHalMeta_DynamicP1;
        cfgParam.pOutAppMeta   = mpAppMeta_DynamicHdr;
        cfgParam.pOutHalMeta   = mpHalMeta_DynamicHdr;
        //
        if( mpHalImage_P1_Raw.get() ) {
#if ENABLE_HDRNODE
            cfgParam.vInFullRaw.push_back(mpHalImage_P1_Raw);
#else
            cfgParam.pvInFullRaw.push_back(mpHalImage_P1_Raw);
#endif
            cfgParam.pInResizedRaw = NULL;
        }
        else
            cfgParam.pInResizedRaw = mpHalImage_P1_ResizerRaw;
        //
        for (size_t i = 0; i < mvAppYuvImage.size(); i++)
            cfgParam.vOutImage.push_back(mvAppYuvImage[i]);
        //
#if ENABLE_HDRNODE
        if( mpHalImage_Jpeg_YUV.get() )
            cfgParam.vOutYuvJpeg = mpHalImage_Jpeg_YUV;
        if( mpHalImage_Thumbnail_YUV.get() )
            cfgParam.vOutYuvThumbnail = mpHalImage_Thumbnail_YUV;
#else
        if( mpHalImage_Jpeg_YUV.get() )
            cfgParam.vOutImage.push_back(mpHalImage_Jpeg_YUV);
        if( mpHalImage_Thumbnail_YUV.get() )
            cfgParam.vOutImage.push_back(mpHalImage_Thumbnail_YUV);
#endif
    }
    //
#if ENABLE_HDRNODE
    mpHdrNode = NodeT::createInstance();
    sp<NodeActorT> pNode = new NodeActorT( mpHdrNode );
#else
    sp<NodeActorT> pNode = new NodeActorT( NodeT::createInstance() );
#endif
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mpAppMeta_Control);
    add_stream_to_set(inStreamSet, mpHalMeta_DynamicP1);
    add_stream_to_set(inStreamSet, mpHalImage_P1_Raw);
    add_stream_to_set(inStreamSet, mpHalImage_P1_ResizerRaw);
    //
    add_stream_to_set(outStreamSet, mpAppMeta_DynamicHdr);
    add_stream_to_set(outStreamSet, mpHalMeta_DynamicHdr);
    add_stream_to_set(outStreamSet, mpHalImage_Jpeg_YUV);
    add_stream_to_set(outStreamSet, mpHalImage_Thumbnail_YUV);
    //
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mpHalImage_P1_Raw        , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mpHalImage_P1_ResizerRaw , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    //
    setImageUsage(mpHalImage_Jpeg_YUV      , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(mpHalImage_Thumbnail_YUV , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configContextLocked_JpegNode(sp<PipelineContext> pContext)
{
    CAM_TRACE_CALL();

    typedef JpegNode                NodeT;
    typedef NodeActor< NodeT >      NodeActorT;
    //
    NodeId_T const nodeId = eNODEID_JpegNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = getOpenId();
        initParam.nodeId = nodeId;
        initParam.nodeName = "JpegNode";
    }
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta        = mpAppMeta_Control;
#if ENABLE_HDRNODE
        cfgParam.pInHalMeta        = mpHalMeta_DynamicHdr;
#else
        cfgParam.pInHalMeta        = mpHalMeta_DynamicP2;
#endif
        cfgParam.pOutAppMeta       = mpAppMeta_DynamicJpeg;
        cfgParam.pInYuv_Main       = mpHalImage_Jpeg_YUV;
        cfgParam.pInYuv_Thumbnail  = mpHalImage_Thumbnail_YUV;
        cfgParam.pOutJpeg          = mpAppImage_Jpeg;
    }
    //
    sp<NodeActorT> pNode = new NodeActorT( NodeT::createInstance() );
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, mpAppMeta_Control);
#if ENABLE_HDRNODE
    add_stream_to_set(inStreamSet, mpHalMeta_DynamicHdr);
#else
    add_stream_to_set(inStreamSet, mpHalMeta_DynamicP2);
#endif
    add_stream_to_set(inStreamSet, mpHalImage_Jpeg_YUV);
    add_stream_to_set(inStreamSet, mpHalImage_Thumbnail_YUV);
    //
    add_stream_to_set(outStreamSet, mpAppMeta_DynamicJpeg);
    add_stream_to_set(outStreamSet, mpAppImage_Jpeg);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(mpHalImage_Jpeg_YUV, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mpHalImage_Thumbnail_YUV, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(mpAppImage_Jpeg, eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    MERROR err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
#undef add_stream_to_set
#undef setImageUsage

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
checkPermission()
{
    if ( ! mpCamMgr->getPermission() )
    {
        MY_LOGD("cannot config pipeline ... Permission denied");
        return PERMISSION_DENIED;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
configRequestRulesLocked()
{
    struct categorize_yuv_stream
    {
        MVOID   operator() (
                    sp<const IImageStreamInfo> const pInfo,
                    MSize const& thres,
                    StreamSet& vLarge, StreamSet& vSmall
                )
                {
                    if( ! pInfo.get() ) return;
                    //
                    MSize const size = pInfo->getImgSize();
                    if ( size.w > thres.w || size.h > thres.h )
                        vLarge.add(pInfo->getStreamId());
                    else
                        vSmall.add(pInfo->getStreamId());
                }
    };
    //
    mvYuvStreams_Fullraw.clear();
    mvYuvStreams_Resizedraw.clear();
    //
    if( ! mpHalImage_P1_Raw.get() && ! mpHalImage_P1_ResizerRaw.get() ) {
        MY_LOGE("no available raw stream");
        return UNKNOWN_ERROR;
    }
    //
    MSize const threshold =
        mpHalImage_P1_ResizerRaw.get() ? mpHalImage_P1_ResizerRaw->getImgSize() : MSize(0,0);
    //
    StreamSet& vLarge =
        mpHalImage_P1_Raw.get() ? mvYuvStreams_Fullraw : mvYuvStreams_Resizedraw;
    StreamSet& vSmall =
        mpHalImage_P1_ResizerRaw.get() ? mvYuvStreams_Resizedraw : mvYuvStreams_Fullraw;
    //
    bool haveFullraw = mpHalImage_P1_Raw.get();
    //
    for( size_t i = 0; i < mvAppYuvImage.size(); i++ ) {
        sp<const IImageStreamInfo> pStreamInfo = mvAppYuvImage.valueAt(i);
        // forcing preview stream to rrzo
        if ( GRALLOC_USAGE_HW_COMPOSER & pStreamInfo->getUsageForConsumer() )
            categorize_yuv_stream()(pStreamInfo, threshold, vSmall, vSmall);
        else
            categorize_yuv_stream()(pStreamInfo, threshold, vLarge, vSmall);
    }
    //
    categorize_yuv_stream()(mpHalImage_Jpeg_YUV, MSize(0,0), vLarge, vSmall);
    categorize_yuv_stream()(mpHalImage_Thumbnail_YUV, MSize(0,0), vLarge, vSmall);
    //
#if 1
    // dump raw stream dispatch rule
    for( size_t i = 0; i < mvYuvStreams_Fullraw.size(); i++ ) {
        MY_LOGD("full raw streamId:%#" PRIx64 " -> yuv streamId:%#" PRIx64,
                mpHalImage_P1_Raw->getStreamId(), mvYuvStreams_Fullraw[i]);
    }
    for( size_t i = 0; i < mvYuvStreams_Resizedraw.size(); i++ ) {
        MY_LOGD("resized raw streamId:%#" PRIx64 " -> yuv streamId:%#" PRIx64,
                mpHalImage_P1_ResizerRaw->getStreamId(), mvYuvStreams_Resizedraw[i]);
    }
#endif
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
evaluateRequestLocked(
        parsedAppRequest const& request,
        evaluateRequestResult& result,
        const MBOOL bMainReq,
        const MBOOL bDelayedFrame)
{
    enum ePath
    {
        eImagePathP1,
#if ENABLE_HDRNODE
        eImagePathHdrResized,
        eImagePathHdrFull,
#else
        eImagePathP2Resized,
        eImagePathP2Full,
#endif
        eImagePathJpeg,
        //
        eMetaPathP1,
#if ENABLE_HDRNODE
        eMetaPathHdr,
#else
        eMetaPathP2,
#endif
        eMetaPathJpeg,
        //
        ePathCount,
    };
    //
    struct
    {
        IOMap                       maps[ePathCount];
        DefaultKeyedVector<
            StreamId_T,
            sp<IImageStreamInfo>
                >                   vUpdatedImageInfos;
        //
        IOMap&                      editIOMap(ePath path) { return maps[path]; }
        MVOID                       addIn(ePath path, sp<IStreamInfo> pInfo) {
                                        editIOMap(path).addIn(pInfo->getStreamId());
                                    }
        MVOID                       addOut(ePath path, sp<IStreamInfo> pInfo) {
                                        editIOMap(path).addOut(pInfo->getStreamId());
                                    }
        MBOOL                       isConfigured(ePath path) {
                                        return editIOMap(path).vIn.size() ||
                                            editIOMap(path).vOut.size();
                                    }
        MVOID                       updateStreamInfo(sp<IImageStreamInfo> pInfo) {
                                        vUpdatedImageInfos.add(pInfo->getStreamId(), pInfo);
                                    }
    } aCollector;
    NodeEdgeSet& aEdges = result.edges;
    NodeSet& aRoot      = result.roots;

#define FUNC_ASSERT(exp, msg) \
    do{ if(!(exp)) { MY_LOGE("%s", msg); return INVALID_OPERATION; } } while(0)
    //
    FUNC_ASSERT( request.vIImageInfos_Raw.size() == 0, "[TODO] not supported yet!" );
    FUNC_ASSERT( request.vIImageInfos_Yuv.size() == 0, "[TODO] not supported yet!" );
    FUNC_ASSERT( request.vOImageInfos_Raw.size() <= 1, "[TODO] not supported yet!" );
    FUNC_ASSERT( request.vOImageInfos_Jpeg.size() <= 1, "[TODO] not supported yet!" );
    //
    // set root node
    aRoot.add(eNODEID_P1Node);
    //
    if ( request.vOImageInfos_Raw.size() )
    {
        IImageStreamInfo const* pStreamInfo = request.vOImageInfos_Raw[0].get();
        {
            MY_LOGE("not supported raw output stream %#" PRIx64 ,
                    pStreamInfo->getStreamId());
            return INVALID_OPERATION;
        }
    }
    //
    if( request.vOImageInfos_Yuv.size() && !bDelayedFrame) {
        //
        FUNC_ASSERT(
                mvYuvStreams_Fullraw.size() == 0 || mpHalImage_P1_Raw.get(),
                "wrong fullraw config");
        FUNC_ASSERT(
                mvYuvStreams_Resizedraw.size() == 0 || mpHalImage_P1_ResizerRaw.get(),
                "wrong resizedraw config");
        //
        bool useFull = false;
        bool useResized = false;
        for( size_t i = 0; i < request.vOImageInfos_Yuv.size(); i++ )
        {
            sp<IImageStreamInfo> pInfo = request.vOImageInfos_Yuv.valueAt(i);
            //
            StreamId_T const streamId = pInfo->getStreamId();
            if( 0 <= mvYuvStreams_Fullraw.indexOf(streamId) )
            {
#if ENABLE_HDRNODE
                aCollector.addOut(eImagePathHdrFull, pInfo);
#else
                aCollector.addOut(eImagePathP2Full, pInfo);
#endif
                useFull = MTRUE;
            } else if( 0 <= mvYuvStreams_Resizedraw.indexOf(streamId) )
            {
#if ENABLE_HDRNODE
                aCollector.addOut(eImagePathHdrResized, pInfo);
#else
                aCollector.addOut(eImagePathP2Resized, pInfo);
#endif
                useResized = MTRUE;
            }
            else
            {
                MY_LOGE("cannot find propery raw for stream %s(%#" PRIx64 ")",
                        pInfo->getStreamName(),streamId);
                return UNKNOWN_ERROR;
            }
        }
        //
        if ( useFull ) {
            aCollector.addOut(eImagePathP1, mpHalImage_P1_Raw);
            //
#if ENABLE_HDRNODE
            aCollector.addIn(eImagePathHdrFull, mpHalImage_P1_Raw);
#else
            aCollector.addIn(eImagePathP2Full, mpHalImage_P1_Raw);
#endif
        }
        if ( useResized ) {
            aCollector.addOut(eImagePathP1, mpHalImage_P1_ResizerRaw);
            //
#if ENABLE_HDRNODE
            aCollector.addIn(eImagePathHdrResized, mpHalImage_P1_ResizerRaw);
#else
            aCollector.addIn(eImagePathP2Resized, mpHalImage_P1_ResizerRaw);
#endif
        }
        //
#if ENABLE_HDRNODE
        aEdges.addEdge(eNODEID_P1Node, eNODEID_HdrNode);
#else
        aEdges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
#endif
    }
    //
    if( !bDelayedFrame )
    {
#if ENABLE_HDRNODE
        aEdges.addEdge(eNODEID_P1Node, eNODEID_HdrNode);
#else
        aEdges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
#endif
    }
    //
    if( request.vOImageInfos_Jpeg.size() && bMainReq) {
        //
        sp<IImageStreamInfo> pHalImage_Thumbnail_YUV;
        createStreamInfoLocked_Thumbnail_YUV(request.pAppMetaControl, pHalImage_Thumbnail_YUV);
        //
        if( pHalImage_Thumbnail_YUV.get() )
            aCollector.updateStreamInfo(pHalImage_Thumbnail_YUV);
        //
        sp<IImageStreamInfo> pSourceRaw =
            ( 0 <= mvYuvStreams_Fullraw.indexOf(mpHalImage_Jpeg_YUV->getStreamId())) ?
            mpHalImage_P1_Raw : mpHalImage_P1_ResizerRaw;
        // p2
        {
            aCollector.addOut(eImagePathP1, pSourceRaw);
            //
#if ENABLE_HDRNODE
            aCollector.addIn(eImagePathHdrFull, pSourceRaw);
            aCollector.addOut(eImagePathHdrFull, mpHalImage_Jpeg_YUV);
#else
            aCollector.addIn(eImagePathP2Full, pSourceRaw);
            aCollector.addOut(eImagePathP2Full, mpHalImage_Jpeg_YUV);
#endif
            if( pHalImage_Thumbnail_YUV.get() )
#if ENABLE_HDRNODE
                aCollector.addOut(eImagePathHdrFull, pHalImage_Thumbnail_YUV);
#else
                aCollector.addOut(eImagePathP2Full, pHalImage_Thumbnail_YUV);
#endif
            //
#if ENABLE_HDRNODE
            aEdges.addEdge(eNODEID_P1Node, eNODEID_HdrNode);
            aEdges.addEdge(eNODEID_HdrNode, eNODEID_JpegNode);
#else
            aEdges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
            aEdges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);
#endif
        }

        // jpeg
        sp<IImageStreamInfo> pHalImage_Jpeg_YUV;
        if ( !mJpegRotationEnable ) {
            aCollector.addIn(eImagePathJpeg, mpHalImage_Jpeg_YUV);
        } else {
               createStreamInfoLocked_Jpeg_YUV(request.pAppMetaControl, pHalImage_Jpeg_YUV);
               if ( pHalImage_Jpeg_YUV.get() ) {
                   aCollector.addIn(eImagePathJpeg, pHalImage_Jpeg_YUV);
                   //request.vIHalImage.add(pHalImage_Jpeg_YUV->getStreamId(), pHalImage_Jpeg_YUV);
                   MY_LOGD_IF( 1, "Add new Jpeg_Yuv to map, StreamName:%s StreamId:%#" PRIx64 " ImageSize:%dx%d Transform:%d",
                               pHalImage_Jpeg_YUV->getStreamName(), pHalImage_Jpeg_YUV->getStreamId(),
                               pHalImage_Jpeg_YUV->getImgSize().w,  pHalImage_Jpeg_YUV->getImgSize().h,
                               pHalImage_Jpeg_YUV->getTransform() );
                   aCollector.updateStreamInfo(pHalImage_Jpeg_YUV);
               } else {
                   aCollector.addIn(eImagePathJpeg, mpHalImage_Jpeg_YUV);
                   MY_LOGD_IF( 1, "Add default Jpeg_Yuv to map, StreamName:%s StreamId:%#" PRIx64 " ImageSize:%dx%d Transform:%d",
                               mpHalImage_Jpeg_YUV->getStreamName(), mpHalImage_Jpeg_YUV->getStreamId(),
                               mpHalImage_Jpeg_YUV->getImgSize().w,  mpHalImage_Jpeg_YUV->getImgSize().h,
                               mpHalImage_Jpeg_YUV->getTransform() );
                   aCollector.updateStreamInfo(mpHalImage_Jpeg_YUV);
               }
        }

        if( pHalImage_Thumbnail_YUV.get() )
            aCollector.addIn(eImagePathJpeg, pHalImage_Thumbnail_YUV);
        MY_LOGD_IF( 1, "Add Thumb_Yuv to map, StreamName:%s StreamId:%#" PRIx64 " ImageSize:%dx%d Transform:%d",
                    mpHalImage_Thumbnail_YUV->getStreamName(), mpHalImage_Thumbnail_YUV->getStreamId(),
                    mpHalImage_Thumbnail_YUV->getImgSize().w,  mpHalImage_Thumbnail_YUV->getImgSize().h,
                    mpHalImage_Thumbnail_YUV->getTransform() );
        aCollector.addOut(eImagePathJpeg, mpAppImage_Jpeg);
        //
    }
    //
    {
        // workaround: if p1node is used, config both raw if exists
        if( aCollector.isConfigured(eImagePathP1) ) {
            if( mpHalImage_P1_Raw.get() )
                aCollector.addOut(eImagePathP1, mpHalImage_P1_Raw);
            if( mpHalImage_P1_ResizerRaw.get() )
                aCollector.addOut(eImagePathP1, mpHalImage_P1_ResizerRaw);
        }
    }
    //
    // update meta
    if( aCollector.isConfigured(eImagePathP1) )
    {
        aCollector.addIn(eMetaPathP1, mpAppMeta_Control);
        aCollector.addIn(eMetaPathP1, mpHalMeta_Control);
        aCollector.addOut(eMetaPathP1, mpAppMeta_DynamicP1);
        aCollector.addOut(eMetaPathP1, mpHalMeta_DynamicP1);
    }
#if ENABLE_HDRNODE
    if( aCollector.isConfigured(eImagePathHdrFull) ||
        aCollector.isConfigured(eImagePathHdrResized))
    {
        aCollector.addIn(eMetaPathHdr, mpAppMeta_Control);
        aCollector.addIn(eMetaPathHdr, mpHalMeta_DynamicP1);
        aCollector.addOut(eMetaPathHdr, mpAppMeta_DynamicHdr);
        //
    }
#else
    if( aCollector.isConfigured(eImagePathP2Full) ||
        aCollector.isConfigured(eImagePathP2Resized))
    {
        aCollector.addIn(eMetaPathP2, mpAppMeta_Control);
        aCollector.addIn(eMetaPathP2, mpHalMeta_DynamicP1);
        aCollector.addOut(eMetaPathP2, mpAppMeta_DynamicP2);
        //
    }
#endif
    //
    //
    if( aCollector.isConfigured(eImagePathJpeg) && bMainReq )
    {
        aCollector.addIn(eMetaPathJpeg, mpAppMeta_Control);
#if ENABLE_HDRNODE
        aCollector.addIn(eMetaPathJpeg, mpHalMeta_DynamicHdr);
#else
        aCollector.addIn(eMetaPathJpeg, mpHalMeta_DynamicP2);
#endif
        aCollector.addOut(eMetaPathJpeg, mpAppMeta_DynamicJpeg);
    }
    //
    // update to result
    for( size_t i = 0 ; i < aCollector.vUpdatedImageInfos.size(); i++ )
    {
        result.vUpdatedImageInfos.add(
                aCollector.vUpdatedImageInfos.keyAt(i),
                aCollector.vUpdatedImageInfos.valueAt(i)
                );
    }
    //
#define updateIOMap(_type_, _nodeId_, _path_ )                          \
    do{                                                                 \
        if( aCollector.isConfigured(_path_) ) {                    \
            result.nodeIOMap##_type_.add(                               \
                    _nodeId_,                                           \
                    IOMapSet().add(aCollector.editIOMap(_path_))); \
        }                                                               \
    } while(0)

    updateIOMap(Image, eNODEID_P1Node, eImagePathP1);
    {
        IOMapSet iomaps;
#if ENABLE_HDRNODE
        if( aCollector.isConfigured(eImagePathHdrFull) )
            iomaps.add(aCollector.editIOMap(eImagePathHdrFull));
        if( aCollector.isConfigured(eImagePathHdrResized) )
            iomaps.add(aCollector.editIOMap(eImagePathHdrResized));
        result.nodeIOMapImage.add(eNODEID_HdrNode, iomaps);
#else
        if( aCollector.isConfigured(eImagePathP2Full) )
            iomaps.add(aCollector.editIOMap(eImagePathP2Full));
        if( aCollector.isConfigured(eImagePathP2Resized) )
            iomaps.add(aCollector.editIOMap(eImagePathP2Resized));
        result.nodeIOMapImage.add(eNODEID_P2Node, iomaps);
#endif
    }
    if ( bMainReq )
        updateIOMap(Image, eNODEID_JpegNode  , eImagePathJpeg);
    //
    updateIOMap(Meta , eNODEID_P1Node    , eMetaPathP1);
#if ENABLE_HDRNODE
    updateIOMap(Meta , eNODEID_HdrNode    , eMetaPathHdr);
#else
    updateIOMap(Meta , eNODEID_P2Node    , eMetaPathP2);
#endif
    if ( bMainReq )
        updateIOMap(Meta , eNODEID_JpegNode  , eMetaPathJpeg);
    //
#undef updateIOMap
    //
    if ( bMainReq )
    { // app image
        result.vAppImageBuffers.setCapacity(
                request.pRequest->vIImageBuffers.size() +
                request.pRequest->vOImageBuffers.size()
                );
        for( size_t i = 0; i < request.pRequest->vIImageBuffers.size(); i++ )
            result.vAppImageBuffers.add(
                    request.pRequest->vIImageBuffers.keyAt(i),
                    request.pRequest->vIImageBuffers.valueAt(i)
                    );
        for( size_t i = 0; i < request.pRequest->vOImageBuffers.size(); i++ )
            result.vAppImageBuffers.add(
                    request.pRequest->vOImageBuffers.keyAt(i),
                    request.pRequest->vOImageBuffers.valueAt(i)
                    );
    }
    //
    { // hal image
        result.vHalImageBuffers.clear();
    }
    //
    if ( bMainReq )
    { // app meta
        result.vAppMetaBuffers.setCapacity(request.pRequest->vIMetaBuffers.size());
        for( size_t i = 0; i < request.pRequest->vIMetaBuffers.size(); i++ )
            result.vAppMetaBuffers.add(
                    request.pRequest->vIMetaBuffers.keyAt(i),
                    request.pRequest->vIMetaBuffers.valueAt(i)
                    );
    }
    else
    {
        result.vAppMetaBuffers.setCapacity(request.pRequest->vIMetaBuffers.size());
        for( size_t i = 0; i < request.pRequest->vIMetaBuffers.size(); i++ ) {
            if (i==0) { // duplicate AppMetaControl
                sp<IMetaStreamBuffer> pStreamBuffer = HalMetaStreamBufferAllocatorT(mpAppMeta_Control.get())();
                IMetadata* data_w = pStreamBuffer->tryWriteLock(__FUNCTION__);
                *data_w = *request.pAppMetaControl;
                pStreamBuffer->unlock(__FUNCTION__, data_w);
                result.vAppMetaBuffers.add(
                        request.pRequest->vIMetaBuffers.keyAt(i),
                        pStreamBuffer
                        );
            }
        }
    }
    //
    { // hal meta
        result.vHalMetaBuffers.setCapacity(1);
        sp<HalMetaStreamBuffer> pBuffer =
            HalMetaStreamBufferAllocatorT(mpHalMeta_Control.get())();
        result.vHalMetaBuffers.add(mpHalMeta_Control->getStreamId(), pBuffer);
    }
    //
    return OK;
#undef FUNC_ASSERT
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
refineRequestMetaStreamBuffersLocked(
        evaluateRequestResult& result,
        const size_t frameIndex,
        const size_t delayedFrames,
        const MBOOL bLastReq)
{
    if( ! mpHalMeta_Control.get() ) {
        MY_LOGE("should config hal control meta");
        return UNKNOWN_ERROR;
    }
    //
    {
        sp<IMetaStreamBuffer> pBuf = result.vHalMetaBuffers.valueFor(mpHalMeta_Control->getStreamId());
        if( pBuf.get() )
        {
            IMetadata* pMetadata = pBuf->tryWriteLock(LOG_TAG);

#if ENABLE_HDRNODE
            // update AE capture parameters
            {
                IMetadata::Memory capParams;
                capParams.resize(sizeof(NS3Av3::CaptureParam_T));
                memcpy(capParams.editArray(), &mvHdrCaptureParams.at(frameIndex).exposureParam,
                        sizeof(NS3Av3::CaptureParam_T));

                IMetadata::IEntry entry(MTK_3A_AE_CAP_PARAM);
                entry.push_back(capParams, Type2Type< IMetadata::Memory >());
                pMetadata->update(entry.tag(), entry);
            }

            // pause AF for (N - 1) frames and resume for the last frame
            {
                IMetadata::IEntry entry(MTK_FOCUS_PAUSE);
                entry.push_back(bLastReq ? 0 : 1, Type2Type< MUINT8 >());
                pMetadata->update(entry.tag(), entry);
            }

            // fall back to single capture if HDR only needs single capture frame
            if (isSingleCapture(mvHdrCaptureParams.size(), delayedFrames))
            {
                IMetadata::IEntry entry(MTK_HAL_REQUEST_ERROR_FRAME);
                entry.push_back(1, Type2Type< MUINT8 >());
                pMetadata->update(entry.tag(), entry);
            }
#endif

            // update sensor size
            {
                IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
                entry.push_back(mParams.mSensorSize, Type2Type< MSize >());
                pMetadata->update(entry.tag(), entry);
            }

            if ( mpAppImage_Jpeg.get() &&
                    0 <= result.vAppImageBuffers.indexOfKey(mpAppImage_Jpeg->getStreamId()) )
            {
                MY_LOGD_IF(1, "set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
                IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                entry.push_back(1, Type2Type<MUINT8>());
                pMetadata->update(entry.tag(), entry);
            }

            {
                IMetadata::IEntry entry(MTK_PIPELINE_UNIQUE_KEY);
                entry.push_back( mTimestamp, Type2Type< MINT32 >());
                pMetadata->update(entry.tag(), entry);
            }

            // set "the largest frame duration of streams" as "minimum frame duration"
            {
                MINT64 iMinFrmDuration = 0;
                for ( size_t i=0; i<result.vAppImageBuffers.size(); i++ ) {
                    StreamId_T const streamId = result.vAppImageBuffers.keyAt(i);
                    if( mvStreamDurations.indexOfKey(streamId) < 0 ) {
                        MY_LOGE("Request App stream %#" PRIx64 "have not configured yet", streamId);
                        continue;
                    }
                    iMinFrmDuration = ( mvStreamDurations.valueFor(streamId) > iMinFrmDuration)?
                        mvStreamDurations.valueFor(streamId) : iMinFrmDuration;
                }
                //MY_LOGD_IF(0, "The min frame duration is %lld", iMinFrmDuration);
                IMetadata::IEntry entry(MTK_P1NODE_MIN_FRM_DURATION);
                entry.push_back(iMinFrmDuration, Type2Type<MINT64>());
                pMetadata->update(entry.tag(), entry);
            }

            //
            pBuf->unlock(LOG_TAG, pMetadata);
        }
        else
        {
            MY_LOGE("cannot get hal control meta sb.");
            return UNKNOWN_ERROR;
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
#define max(a,b)  ((a) < (b) ? (b) : (a))
#define min(a,b)  ((a) < (b) ? (a) : (b))
MERROR
PipelineHdrImp::
createStreamInfoLocked_Jpeg_YUV(
    IMetadata const* pMetadata,
    android::sp<IImageStreamInfo>& rpStreamInfo
) const
{
    if ( mpHalImage_Jpeg_YUV == 0 ) {
        MY_LOGW("No config stream: Jpeg_YUV");
        return NO_INIT;
    }
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return NAME_NOT_FOUND;
    }
    //
    MINT32 const jpegOrientation = entryJpegOrientation.itemAt(0, Type2Type<MINT32>());
    MUINT32      jpegTransform   = 0;
    if ( 0==jpegOrientation )
        jpegTransform = 0;
    else if ( 90==jpegOrientation )
        jpegTransform = eTransform_ROT_90;
    else if ( 180==jpegOrientation )
        jpegTransform = eTransform_ROT_180;
    else if ( 270==jpegOrientation )
        jpegTransform = eTransform_ROT_270;
    else
         MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    //
    MUINT32 const imgTransform   = mpHalImage_Jpeg_YUV->getTransform();
    MY_LOGD_IF( 1, "Jpeg orientation from metadata:%d transform current(%d) & previous(%d)",
                jpegOrientation, jpegTransform, imgTransform);
    if ( imgTransform == jpegTransform ) {
        rpStreamInfo = NULL;
        return OK;
    }
    MSize size;
    if ( jpegTransform&eTransform_ROT_90 ) { // pillarbox
        size.w = min(mpHalImage_Jpeg_YUV->getImgSize().w, mpHalImage_Jpeg_YUV->getImgSize().h);
        size.h = max(mpHalImage_Jpeg_YUV->getImgSize().w, mpHalImage_Jpeg_YUV->getImgSize().h);
    } else { // letterbox
        size.w = max(mpHalImage_Jpeg_YUV->getImgSize().w, mpHalImage_Jpeg_YUV->getImgSize().h);
        size.h = min(mpHalImage_Jpeg_YUV->getImgSize().w, mpHalImage_Jpeg_YUV->getImgSize().h);
    }
    MINT const format = mpHalImage_Jpeg_YUV->getImgFormat();
    MUINT const usage = mpHalImage_Jpeg_YUV->getUsageForAllocator();
    sp<ImageStreamInfo>
        pStreamInfo = createImageStreamInfo(
            mpHalImage_Jpeg_YUV->getStreamName(),
            mpHalImage_Jpeg_YUV->getStreamId(),
            mpHalImage_Jpeg_YUV->getStreamType(),
            mpHalImage_Jpeg_YUV->getMaxBufNum(),
            mpHalImage_Jpeg_YUV->getMinInitBufNum(),
            mpHalImage_Jpeg_YUV->getUsageForAllocator(),
            mpHalImage_Jpeg_YUV->getImgFormat(),
            size, jpegTransform
        );
    if( pStreamInfo == NULL ) {
        MY_LOGE(
            "fail to new ImageStreamInfo: %s %#" PRIx64,
            mpHalImage_Jpeg_YUV->getStreamName(),
            mpHalImage_Jpeg_YUV->getStreamId()
        );
        return NO_MEMORY;
    }
    rpStreamInfo = pStreamInfo;
    MY_LOGD_IF(
        1,
        "streamId:%#" PRIx64 " %s %p (%p) yuvsize:%dx%d jpegOrientation:%d",
        rpStreamInfo->getStreamId(),
        rpStreamInfo->getStreamName(),
        rpStreamInfo.get(),
        mpHalImage_Jpeg_YUV.get(),
        rpStreamInfo->getImgSize().w, rpStreamInfo->getImgSize().h, jpegOrientation
    );
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineHdrImp::
createStreamInfoLocked_Thumbnail_YUV(
    IMetadata const* pMetadata,
    android::sp<IImageStreamInfo>& rpStreamInfo
) const
{
    if  ( mpHalImage_Thumbnail_YUV == 0 ) {
        MY_LOGW("No config stream: Thumbnail_YUV");
        return NO_INIT;
    }
    //
    IMetadata::IEntry const& entryThumbnailSize = pMetadata->entryFor(MTK_JPEG_THUMBNAIL_SIZE);
    if  ( entryThumbnailSize.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_THUMBNAIL_SIZE");
        return NAME_NOT_FOUND;
    }
    MSize const& thumbnailSize = entryThumbnailSize.itemAt(0, Type2Type<MSize>());
    if  ( ! thumbnailSize ) {
        MY_LOGW("Bad thumbnail size: %dx%d", thumbnailSize.w, thumbnailSize.h);
        return NOT_ENOUGH_DATA;
    }
    MY_LOGD_IF( 1, "thumbnail size from metadata: %dx%d", thumbnailSize.w, thumbnailSize.h);
    //
    //
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return NAME_NOT_FOUND;
    }
    //
    MSize const yuvthumbnailsize = calcThumbnailYuvSize(
                                        mpHalImage_Jpeg_YUV->getImgSize(),
                                        thumbnailSize
                                        );
    //
    MINT32  jpegOrientation = 0;
    MUINT32 jpegTransform   = 0;
    MSize   thunmbSize      = yuvthumbnailsize; // default thumbnail size
    //
    MINT const format = mpHalImage_Thumbnail_YUV->getImgFormat();
    IImageStreamInfo::BufPlanes_t bufPlanes;
    switch (format)
    {
    case eImgFmt_YUY2:{
        IImageStreamInfo::BufPlane bufPlane;
        bufPlane.rowStrideInBytes = (yuvthumbnailsize.w << 1);
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * yuvthumbnailsize.h;
        bufPlanes.push_back(bufPlane);
        }break;
    default:
        MY_LOGE("not supported format: %#x", format);
        break;
    }
    //
    rpStreamInfo = new ImageStreamInfo(
        mpHalImage_Thumbnail_YUV->getStreamName(),
        mpHalImage_Thumbnail_YUV->getStreamId(),
        mpHalImage_Thumbnail_YUV->getStreamType(),
        mpHalImage_Thumbnail_YUV->getMaxBufNum(),
        mpHalImage_Thumbnail_YUV->getMinInitBufNum(),
        mpHalImage_Thumbnail_YUV->getUsageForAllocator(),
        format,
        thunmbSize,
        bufPlanes,
        jpegTransform
    );
    if  ( rpStreamInfo == 0 ) {
        MY_LOGE(
            "fail to new ImageStreamInfo: %s %#" PRIx64,
            mpHalImage_Thumbnail_YUV->getStreamName(),
            mpHalImage_Thumbnail_YUV->getStreamId()
        );
        return NO_MEMORY;
    }
    //
    MY_LOGD_IF(
        1,
        "streamId:%#" PRIx64 " %s %p %p yuvthumbnailsize:%dx%d jpegOrientation:%d",
        rpStreamInfo->getStreamId(),
        rpStreamInfo->getStreamName(),
        rpStreamInfo.get(),
        mpHalImage_Thumbnail_YUV.get(),
        thunmbSize.w, thunmbSize.h, jpegOrientation
    );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
PipelineHdrImp::
calcThumbnailYuvSize(
    MSize const rPicSize,
    MSize const rThumbnailsize
) const
{
#define align2(x) (((x) + 1) & (~0x1))
    MSize size;
    MUINT32 const val0 = rPicSize.w * rThumbnailsize.h;
    MUINT32 const val1 = rPicSize.h * rThumbnailsize.w;
    if( val0 > val1 ) {
        size.w = align2(val0/rPicSize.h);
        size.h = rThumbnailsize.h;
    }
    else if( val0 < val1 ) {
        size.w = rThumbnailsize.w;
        size.h = align2(val1/rPicSize.w);
    }
    else {
        size = rThumbnailsize;
    }
#undef align2
    MY_LOGD_IF(1, "thumb %dx%d, pic %dx%d -> yuv for thumb %dx%d",
            rThumbnailsize.w, rThumbnailsize.h,
            rPicSize.w, rPicSize.h,
            size.w, size.h
            );
    return size;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineHdrImp::
isFdEnable(
    IMetadata const* pMetadata
)
{
    //  If Face detection is not OFF or scene mode is face priority,
    //  add App:Meta:FD_result stream to Output App Meta Streams.
    IMetadata::IEntry const& entryFdMode = pMetadata->entryFor(MTK_STATISTICS_FACE_DETECT_MODE);
    IMetadata::IEntry const& entryfaceScene = pMetadata->entryFor(MTK_CONTROL_SCENE_MODE);
    IMetadata::IEntry const& entryGdMode = pMetadata->entryFor(MTK_FACE_FEATURE_GESTURE_MODE);
    IMetadata::IEntry const& entrySdMode = pMetadata->entryFor(MTK_FACE_FEATURE_SMILE_DETECT_MODE);
    IMetadata::IEntry const& entryAsdMode = pMetadata->entryFor(MTK_FACE_FEATURE_ASD_MODE);
    //
    return  //(0 != mDebugFdMode) ||
             ( !entryFdMode.isEmpty() && MTK_STATISTICS_FACE_DETECT_MODE_OFF != entryFdMode.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryfaceScene.isEmpty() && MTK_CONTROL_SCENE_MODE_FACE_PRIORITY == entryfaceScene.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryGdMode.isEmpty() && MTK_FACE_FEATURE_GESTURE_MODE_OFF != entryGdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entrySdMode.isEmpty() && MTK_FACE_FEATURE_SMILE_DETECT_MODE_OFF != entrySdMode.itemAt(0, Type2Type<MINT32>())) ||
             ( !entryAsdMode.isEmpty() && MTK_FACE_FEATURE_ASD_MODE_OFF != entryAsdMode.itemAt(0, Type2Type<MINT32>()));
}


/******************************************************************************
 *  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
 ******************************************************************************/
MVOID
PipelineHdrImp::
updateFrame(
    MUINT32 const frameNo,
    MINTPTR const userId,
    Result const& result
)
{
    if ( result.bFrameEnd ) return;

    MY_LOGD("frameNo %d, user %#" PRIxPTR ", AppLeft %zu, appMeta %zu, HalLeft %zu, halMeta %zu",
            frameNo, userId,
            result.nAppOutMetaLeft, result.vAppOutMeta.size(),
            result.nHalOutMetaLeft, result.vHalOutMeta.size()
           );
    sp<IPipelineModelMgr::IAppCallback> pAppCallback;
    pAppCallback = mpAppCallback.promote();
    if ( ! pAppCallback.get() ) {
        MY_LOGE("Have not set callback to device");
        FUNC_END;
        return;
    }
    pAppCallback->updateFrame(frameNo, userId, result.nAppOutMetaLeft, result.vAppOutMeta);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
PipelineHdrImp::
createRawImageStreamInfo(
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
    //
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_FG_BAYER10:
            addBufPlane(bufPlanes , imgSize.h, stride);
            break;
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
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


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
getSensorOutputFmt(
    SensorStaticInfo const& sensorInfo,
    MUINT32 bitDepth,
    MBOOL isFull,
    MINT* pFmt
)
{
    MBOOL ret = MFALSE;
    // sensor fmt
#define case_Format( order_bit, mappedfmt, pFmt) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
        break;

    if( sensorInfo.sensorType == SENSOR_TYPE_YUV )
    {
        switch( sensorInfo.sensorFormatOrder )
        {
            case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, pFmt);
            default:
            MY_LOGE("formatOrder not supported, 0x%x", sensorInfo.sensorFormatOrder);
            goto lbExit;
            break;
        }
        MY_LOGD("sensortype:(0x%x), fmt(0x%x)", sensorInfo.sensorType, *pFmt);
    }
    else if( sensorInfo.sensorType == SENSOR_TYPE_RAW )
    {
        if( isFull ) //imgo
        {
            switch( bitDepth )
            {
                case_Format(  8, eImgFmt_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_BAYER10, pFmt);
                case_Format( 12, eImgFmt_BAYER12, pFmt);
                case_Format( 14, eImgFmt_BAYER14, pFmt);
                default:
                MY_LOGE("bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
                break;
            }
        }
        else // rrzo
        {
            switch( bitDepth )
            {
                case_Format(  8, eImgFmt_FG_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_FG_BAYER10, pFmt);
                case_Format( 12, eImgFmt_FG_BAYER12, pFmt);
                case_Format( 14, eImgFmt_FG_BAYER14, pFmt);
                default:
                MY_LOGE("bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
                break;
            }
        }
        MY_LOGD("sensortype: 0x%x, full(%d), fmt(0x%x), order(%d)",
                sensorInfo.sensorType, isFull, *pFmt, sensorInfo.sensorFormatOrder);
    }
    else
    {
        MY_LOGE("sensorType not supported yet(0x%x)", sensorInfo.sensorType);
        goto lbExit;
    }
    ret = MTRUE;
#undef case_Format

lbExit:
    return ret;
}
