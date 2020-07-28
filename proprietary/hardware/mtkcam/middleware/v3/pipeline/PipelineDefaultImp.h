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
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/pipeline/extension/IVendorManager.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/feature/eis/EisInfo.h>
#include <mtkcam/utils/hw/CamManager.h>
//
#define  SUPPORT_IVENDOR                    (1)

#include <mtkcam/pipeline/hwnode/P2Node.h>

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
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef Utils::HalMetaStreamBuffer::Allocator
                                    HalMetaStreamBufferAllocatorT;

public:
    class   ConfigHandler;
    class   RequestHandler;

    struct CommonInfo
    {
                                        CommonInfo(MINT32 const openId, android::String8 const& name)
                                            : mOpenId(openId)
                                            , mName(name)
                                            , mLogLevel(0)
                                            , mJpegRotationEnable(MFALSE)
                                        {}
        MINT32 const                    mOpenId;
        android::String8 const          mName;

        MINT32                          mLogLevel;
        MBOOL                           mJpegRotationEnable;
        sp<PipelineContext>             mpPipelineContext = nullptr;
        std::shared_ptr<CameraSettingMgr_Imp>        mCameraSetting;

    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    std::shared_ptr<CommonInfo>     mCommonInfo = nullptr;
    std::shared_ptr<MyProcessedParams>
                                    mParams = nullptr;
    android::RWLock                 mRWLock;

    sp<PipelineDefaultImp::ConfigHandler>
                                    mConfigHandler = nullptr;
    sp<PipelineDefaultImp::RequestHandler>
                                    mRequestHandler = nullptr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual char const*             getName() const { return mCommonInfo->mName.string(); }
    virtual MINT32                  getOpenId() const { return mCommonInfo->mOpenId; }

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
    MBOOL                           skipStream(
                                        MBOOL skipJpeg,
                                        IImageStreamInfo* pStreamInfo
                                    ) const;
};

class PipelineDefaultImp::ConfigHandler
    : public android::RefBase
{
public:
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

        MINT32                      setStreamCapacity(size_t size) { return vStreamStatus.setCapacity(size); }

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

public:
                                    ConfigHandler(
                                        std::shared_ptr<CommonInfo> pCommonInfo,
                                        std::shared_ptr<MyProcessedParams> pParams
                                    );
                                    ~ConfigHandler();

    MERROR                          configureLocked(
                                        PipeConfigParams const& rConfigParams,
                                        android::sp<IPipelineModel> pOldPipeline
                                    );

    MERROR                          reconfigPipelineLocked(parsedAppRequest const& request);

protected:

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
    MERROR                          configContextLocked_Nodes(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_Pipeline(sp<PipelineContext> pContext);
    //
    MERROR                          checkPermission();
    MERROR                          configRequestRulesLocked(MINT32 idx = 0);
    MERROR                          configScenarioCtrlLocked();
    //
    MERROR                          configContextLocked_P1Node(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE, MINT32 idx = 0);
    MERROR                          configContextLocked_P2SNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_P2CNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_FdNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_JpegNode(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);
    MERROR                          configContextLocked_Raw16Node(sp<PipelineContext> pContext, MBOOL isReConfig = MFALSE);

    //
    template <class INITPARAM_T, class CONFIGPARAM_T>
    MERROR                          compareParamsLocked_P1Node(
                                        INITPARAM_T const& initParam1,  INITPARAM_T const& initParam2,
                                        CONFIGPARAM_T const& cfgParam1, CONFIGPARAM_T const& cfgParam2
                                    ) const;

private:
    // // utility functions
    sp<NSCam::v3::Utils::ImageStreamInfo>
                                    createRawImageStreamInfo(
                                        char const*         streamName,
                                        StreamId_T          streamId,
                                        MUINT32             streamType,
                                        size_t              maxBufNum,
                                        size_t              minInitBufNum,
                                        MUINT               usageForAllocator,
                                        MINT                imgFormat,
                                        MSize const&        imgSize,
                                        size_t const        stride,
                                        MINT32              p1Index
                                    ) const;

    P2Common::UsageHint             prepareP2Usage(
                                        P2Node::ePass2Type type
                                    ) const;

protected:
    std::shared_ptr<CommonInfo>     mCommonInfo = nullptr;

    std::shared_ptr<MyProcessedParams>
                                    mParams = nullptr;

    sp<PipelineContext>             mpOldCtx = nullptr;

private:
    std::vector<pass1Resource>      mvPass1Resource;

    CamManager*                     mpCamMgr = nullptr;

    sp<CamManager::UsingDeviceHelper>
                                    mpDeviceHelper = nullptr;

};

class PipelineDefaultImp::RequestHandler
    : public IPipelineBufferSetFrameControl::IAppCallback
{

public:
                                    RequestHandler(
                                        std::shared_ptr<CommonInfo> pCommonInfo,
                                        std::shared_ptr<MyProcessedParams> pParams,
                                        wp<IPipelineModelMgr::IAppCallback> pAppCallback,
                                        sp<PipelineDefaultImp::ConfigHandler> pConfigHandler
                                    );
                                    ~RequestHandler();

    MERROR                          submitRequestLocked(AppRequest& request);

protected:

    static inline MBOOL             isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
                                    {
                                        return pStreamInfo.get() && pStreamInfo->getStreamId()==streamId;
                                    }

    MERROR                          parse(AppRequest* pRequest, parsedAppRequest &ParsedRequest);

    MERROR                          evaluateRequestLocked(
                                        parsedAppRequest const& request,
                                        std::vector<evaluateRequestResult> &vresult
                                    );

    MERROR                          evaluateRequestLocked_updateStreamBuffers(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    MERROR                          evaluateRequestLocked_updateHalBuffers(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    MERROR                          refineRequestMetaStreamBuffersLocked(
                                        parsedAppRequest const& request,
                                        evaluateRequestResult& result
                                    );

    sp<IPipelineFrame>              buildPipelineFrameLocked(
                                        MUINT32 requestNo,
                                        evaluateRequestResult&  result
                                    );

    MBOOL                           isTimeSharingForJpegSource(
                                        parsedAppRequest const& request
                                    ) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   updateFrame(
                                        MUINT32 const frameNo,
                                        MINTPTR const userId,
                                        Result const& result
                                    );

protected:
    android::wp<IPipelineModelMgr::IAppCallback> const
                                    mpAppCallback;

    std::shared_ptr<CommonInfo>     mCommonInfo = nullptr;

    std::shared_ptr<MyProcessedParams>
                                    mParams = nullptr;

    sp<PipelineDefaultImp::ConfigHandler>
                                    mConfigHandler = nullptr;

    MBOOL                           mPrevFDEn;

};

#if 0
class PipelineDefaultImp::VendorHandler
    : public android::RefBase
{
public: // structure
                                    VendorHandler(
                                        std::shared_ptr<CommonInfo> pCommonInfo,
                                        std::shared_ptr<MyProcessedParams> pParams
                                    );
                                    ~VendorHandler();

    MVOID                           getVendorCfg(
                                        android::sp<NSCam::plugin::IVendorManager>& pVendor,
                                        MUINT64& userId
                                    );


    MBOOL                           refinePluginRequestMetaStreamBuffersLocked(
                                        evaluateRequestResult& result,
                                        evaluateSubRequestResult& subResult
                                    );

    MERROR                          setPluginResult(
                                        MUINT32 startFrameNo,
                                        evaluateRequestResult&    result,
                                        evaluateSubRequestResult& subResult
                                    );

    sp<IPipelineFrame>              buildPipelineFrameLocked(
                                        MUINT32 requestNo,
                                        evaluateRequestResult&  result
                                    );

    sp<IPipelineFrame>              buildSubPipelineFrameLocked(
                                        MUINT32 requestNo,
                                        evaluateRequestResult& result
                                    );

protected:
    MERROR                          updateCombination(
                                        plugin::InputInfo&     input,
                                        evaluateRequestResult& result
                                    );
protected:
#if SUPPORT_IVENDOR
    sp<NSCam::plugin::IVendorManager>
                                    mpVendorMgr = nullptr;

    MUINT64                         mUserId;
#endif
    std::shared_ptr<CommonInfo>     mCommonInfo = nullptr;

    std::shared_ptr<MyProcessedParams>
                                    mParams = nullptr;

};
#endif

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam

#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEDEFAULTIMP_H_
