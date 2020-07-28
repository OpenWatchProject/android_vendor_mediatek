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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEDEFAULTIMP_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEDEFAULTIMP_H_
//
#include "MyUtils.h"
//
#include "PipelineModel_Default.h"
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <v3/utils/streaminfo/MetaStreamInfo.h>
#include <v3/utils/streaminfo/ImageStreamInfo.h>
#include <v3/utils/streambuf/StreamBuffers.h>
#include <v3/pipeline/PipelineContext.h>
//
// #include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/hwutils/CamManager.h>
#if MTKCAM_HAVE_ADV_SETTING
#include <mtkcam/feature/advCamSetting/AdvCamSettingMgr.h>
#endif
#define  SUPPORT_IVENDOR                    (0)

#include "zsd/ZsdProcessor.h"
#include "utils/ExtStreamBuffers.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::NSPipelineContext;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

class PipelineDefaultImp
    : public PipelineModel_Default
    , public IPipelineBufferSetFrameControl::IAppCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef Utils::HalMetaStreamBuffer::Allocator
                                    HalMetaStreamBufferAllocatorT;

    struct MyConfigParams
    {
        PipeConfigParams            configParams;
        //
        // internal setting
        MBOOL                       skipJpeg;
    };
    //
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
        /*  processed params  */
        // Stream infos
        MSize                       mVideoSize;
        MBOOL                       mbHasRecording;
        MBOOL                       mb4KRecording;
        MBOOL                       mbHasRaw;
        MBOOL                       mbHasJpeg;
        MSize                       mMaxStreamSize;
        //
        // opaque/yuv reprocessing
        MBOOL                       mbOpaqueReproc;
        MBOOL                       mbYuvReproc;
        //
        // constrained high speed video related
        MUINT32                     mOperation_mode;
        MUINT32                     mDefaultBusrstNum;
        MINT32                      mAeTargetFpsMin;
        MINT32                      mAeTargetFpsMax;
        MINT32                      mAeTargetFpsMin_Req;
        MBOOL                       mResetAeTargetFps_Req;

        MBOOL                       mZsdMode;

        //
        // node related
        MBOOL                       mbUseP1Node;
        MBOOL                       mbUseP2Node;
        MBOOL                       mbUseP2TSNode;
        MBOOL                       mbUseFDNode;
        MBOOL                       mbUseJpegNode;
        MBOOL                       mbUseRaw16Node;
        //
        MBOOL                       mbHasLcso;
        // sensor related
        MUINT                       mSensorMode;
        MSize                       mSensorSize;
        MUINT                       mSensorFps;
        MUINT32                     mVhdrMode;
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
        // NSCamHW::HwInfoHelper       mHwInfoHelper;
#if MTKCAM_HAVE_ADV_SETTING
        sp<AdvCamSetting>           mpAdvSetting;
        MINT32                      mForceEnableIMGO;
#endif
    public:
                                    MyProcessedParams(MINT32 const openId);
        virtual                     ~MyProcessedParams() {}
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
        virtual MERROR              querySensorStatics();
        virtual MERROR              preprocess();
        virtual MERROR              decideSensor();
        virtual MERROR              decideP1();
        virtual MBOOL               isNeedLcso();
    };

    struct parsedAppRequest
    {
        // original AppRequest
        AppRequest* const           pRequest;
        // in
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Opaque;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Yuv;
        // out
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Opaque;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Yuv;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Jpeg;
        //
                                    parsedAppRequest(AppRequest* pRequest)
                                        : pRequest(pRequest)
                                    {}
                                    ~parsedAppRequest() {}
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

    enum ePath
    {
        eImagePathP1,
        eImagePathP2Resized,
        eImagePathP2Full,
        eImagePathP2TS,
        eImagePathRaw16,
        eImagePathJpeg,
        eImagePathFD,
        //
        eMetaPathP1,
        eMetaPathP2,
        eMetaPathP2TS,
        eMetaPathRaw16,
        eMetaPathJpeg,
        eMetaPathFD,
        //
        ePathCount,
    };
    //
    struct IOMapColloctor
    {
                                    IOMapColloctor() {}

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
        size_t                      sizeIn(ePath path) {
                                        return editIOMap(path).sizeIn();
                                    }
        size_t                      sizeOut(ePath path) {
                                        return editIOMap(path).sizeOut();
                                    }
        MBOOL                       isConfigured(ePath path) {
                                        return editIOMap(path).vIn.size() ||
                                            editIOMap(path).vOut.size();
                                    }
        MVOID                       updateStreamInfo(sp<IImageStreamInfo> pInfo) {
                                        vUpdatedImageInfos.add(pInfo->getStreamId(), pInfo);
                                    }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    MINT32 const                    mOpenId;
    MINT32                          mLogLevel;
    android::String8 const          mName;
    android::wp<IPipelineModelMgr::IAppCallback>const
                                    mpAppCallback;
    //
    MyProcessedParams               mParams;
    // meta: hal
    android::sp<IMetaStreamInfo>    mpHalMeta_Control;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP2;
    // meta: app
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP2;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicFD;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicJpeg;
    android::sp<IMetaStreamInfo>    mpAppMeta_Control;
    // image: hal
    android::sp<IImageStreamInfo>   mpHalImage_P1_Raw;
    android::sp<IImageStreamInfo>   mpHalImage_P1_ResizerRaw;
    android::sp<IImageStreamInfo>   mpHalImage_P1_Lcso;
    android::sp<IImageStreamInfo>   mpHalImage_FD_YUV;
    android::sp<IImageStreamInfo>   mpHalImage_Jpeg_YUV;
    android::sp<IImageStreamInfo>   mpHalImage_Thumbnail_YUV;

    // image: app
    android::sp<IImageStreamInfo>   mpAppImage_Yuv_In;
    android::sp<IImageStreamInfo>   mpAppImage_Opaque_In;
    android::sp<IImageStreamInfo>   mpAppImage_Opaque_Out;
    android::KeyedVector <
            StreamId_T, android::sp<IImageStreamInfo>
                    >               mvAppYuvImage;
    android::sp<IImageStreamInfo>   mpAppImage_Jpeg;
    android::sp<IImageStreamInfo>   mpAppImage_RAW16;

    //// raw/yuv stream mapping
    StreamSet                       mvYuvStreams_Fullraw;
    StreamSet                       mvYuvStreams_Resizedraw;
    //android::Vector<StreamId_T>     mvYuvStreams_Fullraw;
    //android::Vector<StreamId_T>     mvYuvStreams_Resizedraw;

    android::KeyedVector < StreamId_T, MINT64 >
                                    mvStreamDurations;
    MBOOL                           mJpegRotationEnable;
protected:
    android::RWLock                 mRWLock;
    //
    sp<PipelineContext>             mpPipelineContext;

    MBOOL                           mPrevFDEn;

    sp<PipelineContext>             mpOldCtx;

    MBOOL                           mP1ImageStreamReuse;

    pass1Resource                   mPass1Resource;

    CamManager*                     mpCamMgr;
    // sp<CamManager::UsingDeviceHelper>
    //                                 mpDeviceHelper;

    struct zsdCommonInfo {
        sp<Zsd::ZsdProcessor>           mProcessor = nullptr;
        int64_t                         mSerialNo  = 0;
        // vec of zsd request: pair of serailNo and refSerailNo
        std::vector< std::pair<int64_t, int64_t> >
                                        mvZsdRequest;
        // pipelineframe requestNo and Zsd serailNo map
        DefaultKeyedVector<MUINT32, int64_t>
                                        mMap;
        // needed if no p1node in zsd flow.
        DefaultKeyedVector<MUINT32, MINT64 >
                                        mShutter;
        DefaultKeyedVector<MUINT32, sp<IMetaStreamBuffer> >
                                        mvMetaSB;
        int32_t                         mInfoCount  = 0;
    };

    android::Mutex                      mZsdLock;
    zsdCommonInfo                       mZsdInfo;


#if SUPPORT_IVENDOR
    sp<NSCam::plugin::IVendorManager>
                                    mpVendorMgr;

    MUINT64                         mUserId;
#endif
#if MTKCAM_HAVE_ADV_SETTING
    AdvCamSettingMgr*               mpAdvSettingMgr;
    MBOOL                           mbFirstReqReceived;
#endif
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
//  PipelineModel_Default Interfaces.
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
                                    ~PipelineDefaultImp();
                                    PipelineDefaultImp(
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
    MERROR                          configContextLocked_Zsd(sp<PipelineContext> pContext);
    MERROR                          configContextLocked_Nodes(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_Pipeline(sp<PipelineContext> pContext);
    //
    MERROR                          checkPermission();
    MERROR                          configRequestRulesLocked();
    MERROR                          configScenarioCtrlLocked();
    MERROR                          reconfigPipelineLocked(parsedAppRequest const& request);
    //
    MERROR                          configContextLocked_P1Node(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_P2Node(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_P2TSNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_FdNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_JpegNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_Raw16Node(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);

    //
    template <class INITPARAM_T, class CONFIGPARAM_T>
    MERROR                          compareParamsLocked_P1Node(
                                        INITPARAM_T const& initParam1,  INITPARAM_T const& initParam2,
                                        CONFIGPARAM_T const& cfgParam1, CONFIGPARAM_T const& cfgParam2
                                    ) const;

    struct reprocessInfo
    {
        bool useYuvIn     = false;
        bool useOpaqueIn  = false;
        bool useOpaqueOut = false;
    };

    struct evaluateRequestResult
    {
        DefaultKeyedVector<
            StreamId_T,
            sp<IImageStreamInfo>
                >                               vUpdatedImageInfos;
        //
        NodeSet                                 roots;
        NodeEdgeSet                             edges;
        bool                                    isTSflow = false;
        bool                                    forceFullRaw = false;
        bool                                    zsdRequest = false;
        int64_t                                 zsdRefNo = -1;
        reprocessInfo                           reprocInfos;
        //
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapImage;
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapMeta;
        //
        DefaultKeyedVector<StreamId_T, sp<IImageStreamBuffer> >
                                                vAppImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<NSCam::v3::Utils::HalImageStreamBuffer> >
                                                vHalImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<IMetaStreamBuffer> >
                                                vAppMetaBuffers;
        DefaultKeyedVector<StreamId_T, sp<NSCam::v3::Utils::HalMetaStreamBuffer> >
                                                vHalMetaBuffers;

        //
        bool                                    isOpaqueReprocOut()
                                                { return reprocInfos.useOpaqueOut; }
        bool                                    isOpaqueReprocIn()
                                                { return reprocInfos.useOpaqueIn; }
        bool                                    isYuvReprocIn()
                                                { return reprocInfos.useYuvIn; }
    };

    struct evaluateSubRequestResult
    {
        MUINT32 subRequetNumber;
        //
        Vector<evaluateRequestResult> subRequestList;
        //
        evaluateSubRequestResult()
            : subRequetNumber(0)
        {}
    };

    MERROR                          evaluateRequestLocked(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    MERROR                          evaluateRequestLocked_Img_Reproc(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Img_Raw16(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Img_Lcso(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Img_Jpeg(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Img_AppYuvs(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Img_FD(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Opt(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_Metadata(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_updateIOMap(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_updateStreamBuffers(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_zsd(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          evaluateRequestLocked_updateStreamBuffers_zsd(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result,
                                        IOMapColloctor& aCollector
                                    );

    MERROR                          refineRequestMetaStreamBuffersLocked(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    sp<IPipelineFrame>              buildPipelineFrameLocked(
                                        MUINT32 requestNo,
                                        evaluateRequestResult&  result
                                    );

#if SUPPORT_IVENDOR
    MBOOL                           refinePluginRequestMetaStreamBuffersLocked(
                                        evaluateRequestResult& result,
                                        evaluateSubRequestResult& subResult
                                    );

    MERROR                          setPluginResult(
                                        MUINT32 startFrameNo,
                                        evaluateRequestResult&    result,
                                        evaluateSubRequestResult& subResult
                                    );

    sp<IPipelineFrame>              buildSubPipelineFrameLocked(
                                        MUINT32 requestNo,
                                        evaluateRequestResult& result
                                    );
#endif

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
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    MBOOL                           isFdEnable(
                                        IMetadata const* pMetadata
                                    );

    MBOOL                           isTimeSharingForJpegSource(
                                        parsedAppRequest const& request
                                    ) const;

    MBOOL                           isZsdRequest(
                                        parsedAppRequest const& request
                                    ) const;

    MVOID                           cacheZsdShutterLocked(
                                        uint32_t frameNo,
                                        android::sp<IMetaStreamBuffer>& pStreamBuffer
                                    );

    MBOOL                           updateZsdShutterLocked(
                                        uint32_t frameNo,
                                        android::sp<IMetaStreamBuffer>& pStreamBuffer
                                    );

    MVOID                           dumpShutterCacheLocked();

    MVOID                           dumpZsdInfoLocked();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   updateFrame(
                                        MUINT32 const frameNo,
                                        MINTPTR const userId,
                                        ssize_t const nOutMetaLeft,
                                        android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
                                    );

protected:
    virtual MVOID                   updateFrameZsd(
                                        MUINT32 const frameNo,
                                        MINTPTR const userId,
                                        ssize_t const nOutMetaLeft,
                                        android::Vector<android::sp<IMetaStreamBuffer> >& rvOutMeta
                                    );

private:
    MBOOL                           skipStream(
                                        MBOOL skipJpeg,
                                        IImageStreamInfo* pStreamInfo
                                    ) const;

    // P2Node::UsageHint               prepareP2Usage(
    //                                     P2Node::ePass2Type type
    //                                 ) const;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam

#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEDEFAULTIMP_H_

