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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_DCMFSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_DCMFSHOT_H_

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>

#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <mtkcam/utils/std/Semaphore.h>

#include <future>
#include <vector>
#include <atomic>
#include <list>
#include <set>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <utils/Thread.h>

// need refactor
// TODO: change to correct inlude path
#include "../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
#include "../SmartShot.h"

#define CCP_NOT_SUPPORT 1

using namespace std;
using namespace NSShot::NSSmartShot;
using namespace NSCam::plugin;

namespace android {
namespace NSShot {
namespace NSDCMFShot {

// TODO: find a proper id
static constexpr const int DCMF_PIPE_ID         = 1648;
static constexpr const char* DCMF_PURE_ZSD_KEY  = "vendor.fc.dcmf.purezsd";
static constexpr const char* DUMP_KEY           = "vendor.di.dcmfshot.dump";
static constexpr const char* DUMP_PATH          = "/sdcard/dcmfshot";
static constexpr const char* DEBUG_LOG          = "vendor.dg.dcmfshot.log";
static constexpr const char* DUMP_CALI_KEY      = "vendor.di.camera.cali.dump_img";
static constexpr const char* HDR_FRAME_SYNC     = "vendor.dcmfshot.hdrsync";
static constexpr const char* FG_POSTVIEW_KEY    = "vendor.dcmfshot.fgpostview";

typedef android::NSPostProc::ImagePostProcessData PPData;

struct CaptureRequest
{
    MUINT32                                 mCapReqNo = -1;
    MUINT32                                 mPPReqNo = -1;
    std::map<StreamId, sp<IImageBuffer> >   vDstStreams;
    sp<ImageStreamManager>                  spImgStreamMgr = nullptr;
    sp<CallbackImageBufferManager>          spCbImgBufMgr = nullptr;
    sp<BufferCallbackHandler>               spCbHandler = nullptr;
};

class CaptureCBParcel
{
public:
    MUINT32                                 mReqNo = -1;
    std::map<StreamId, sp<IImageBuffer> >   mvDstStreamsCollect;
    std::map<StreamId, IMetadata >          mvMetadataCollect;
public:
    const char* getDumpStr();

    MBOOL isFinished() const;

    android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> getPPInputData_Image() const;

    IMetadata getPPInputData_AppMeta() const;

    IMetadata getPPInputData_HalMeta() const;
};

struct StoredSelectorData
{
    Vector<ISelector::MetaItemSet>    metaSet;
    Vector<ISelector::BufferItemSet>  bufferSet;
};

class ScopeLogger
{
public:
    ScopeLogger(const int tid, const int openid, const bool isDebugLog, const bool debugFlag, const char* shotName, const char* text)
    : mTid(tid)
    , mOpenId(openid)
    , mIsDebugLog(isDebugLog)
    , mDebugFlag(debugFlag)
    , mShotName(shotName)
    , mText(text)
    {
        if( mIsDebugLog ){
            CAM_LOGD_IF(mDebugFlag, "(%d)(%d)(%s)[%s] +", mTid, mOpenId, mShotName, mText);
        }else{
            CAM_LOGI("(%d)(%d)(%s)[%s] +", mTid, mOpenId, mShotName, mText);
        }
    };
    ~ScopeLogger()
    {
        if( mIsDebugLog ){
            CAM_LOGD_IF(mDebugFlag, "(%d)(%d)(%s)[%s] -", mTid, mOpenId, mShotName, mText);
        }else{
            CAM_LOGI("(%d)(%d)(%s)[%s] -", mTid, mOpenId, mShotName, mText);
        }
    };
private:
    const int   mTid        = 0;
    const int   mOpenId     = 0;
    const bool  mIsDebugLog = false;
    const int   mDebugFlag  = 0;
    const char* mShotName   = nullptr;
    const char* mText       = nullptr;
};

#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT && CCP_NOT_SUPPORT
/******************************************************************************
 *
 ******************************************************************************/
class CallbackListener
{
public:
    virtual         ~CallbackListener() {}
public:
    virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    )                                       = 0;
    virtual MVOID   onDataReceived(
                        MUINT32 const requestNo,
                        StreamId_T const streamId,
                        MBOOL const errorResult,
                        android::sp<IImageBuffer>& pBuffer
                    )                                       = 0;
};

/*******************************************************************************
*
********************************************************************************/
class ImageCallback
    : public IImageCallback
{
public:
                                        ImageCallback(
                                            CallbackListener* pListener,
                                            MUINT32 const data
                                            )
                                            : mpListener(pListener)
                                            , mData(data)
                                        {}
public:   ////    interface of IImageCallback
    /**
     *
     * Received result buffer.
     *
     * @param[in] RequestNo : request number.
     *
     * @param[in] pBuffer : IImageBuffer.
     *
     */
    virtual MERROR                      onResultReceived(
                                            MUINT32 const              RequestNo,
                                            StreamId_T const           streamId,
                                            MBOOL   const              errorBuffer,
                                            android::sp<IImageBuffer>& pBuffer
                                        ) {
                                            if( mpListener )
                                                mpListener->onDataReceived(
                                                    RequestNo, streamId, errorBuffer, pBuffer
                                                    );
                                            return OK;
                                        }
protected:
    CallbackListener*                   mpListener;
    MUINT32 const                       mData;
};

/*******************************************************************************
*
********************************************************************************/
class MetadataListener
    : public ResultProcessor::IListener
{
public:
                                        MetadataListener(
                                            CallbackListener* pListener
                                            )
                                            : mpListener(pListener)
                                        {}
public:   ////    interface of IListener
    virtual void                        onResultReceived(
                                            MUINT32         const requestNo,
                                            StreamId_T      const streamId,
                                            MBOOL           const errorResult,
                                            IMetadata       const result
                                        ) {
                                            if( mpListener )
                                                mpListener->onMetaReceived(
                                                    requestNo, streamId, errorResult, result
                                                    );
                                        }

    virtual void                       onFrameEnd(
                                            MUINT32         const /*requestNo*/
                                        ) {};

    virtual String8                     getUserName() { return String8(LOG_TAG); }

protected:
    CallbackListener*                   mpListener;
};
#endif // MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT && CCP_NOT_SUPPORT

/******************************************************************************
 *
 ******************************************************************************/
class DCMFHdrSync
{
public:
    DCMFHdrSync();
   ~DCMFHdrSync();
    static DCMFHdrSync *getInstance();
    void   notifyMySelectResult(int const openId, bool const selectResult);
    bool   getDualSelectResult(int const openId);

private:
    mutable Mutex     mLock;
    mutable Condition mCond;
    std::unordered_map<MUINT32, MBOOL> mSelectResultMap;
    bool              mDualSelectResult = false;
};

/******************************************************************************
 * Dual Cam Multi-Frame Shot
 ******************************************************************************/
class DCMFShot
    : public SmartShot
    , public Thread
    , public NSPostProc::IPostProcRequestCB
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT && CCP_NOT_SUPPORT
    , public CallbackListener
#endif
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Static Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static DCMFShot* createInstance(
                        char const*const pszShotName,
                        uint32_t const   u4ShotMode,
                        int32_t const    i4OpenId,
                        bool const       isZsd
                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~DCMFShot();

private:
                                    DCMFShot(
                                        char const*const pszShotName,
                                        uint32_t const   u4ShotMode,
                                        int32_t const    i4OpenId,
                                        bool const       isZsd
                                    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    //// thread interface
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    //virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IShot Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
public:
    virtual void                    onDestroy();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPostProcRequestCB Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual android::status_t       onPostProcEvent(
                                        MINT32  requestNo,
                                        NSPostProc::PostProcRequestCB callbackType,
                                        MUINT32 streamId,
                                        MBOOL bError,
                                        void* params1 = nullptr,
                                        void* params2 = nullptr
                                    ) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  INotifyCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
virtual android::status_t   onMsgReceived(
                                    MINT32 cmd, /*NOTIFY_MSG*/
                                    MINT32 arg1, MINT32 arg2,
                                    void*  arg3
                                ) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SmartShot Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_capture() override;
    virtual void                    onCmd_cancel() override;

protected:  //// handle setting
    virtual MBOOL                   createStreams() override;

    virtual MBOOL                   createPipeline() override;

    virtual MVOID                   endCapture() override;

    virtual MBOOL                   constructCapturePipeline() override;

    virtual status_t                getSelectorData(IMetadata& rAppSetting,
                                        IMetadata& rHalSetting,
                                        android::sp<IImageBuffer>& pBuffer) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DCMFShot Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            MBOOL                   returnCapReqBuffers(MUINT32 ppReqNo);

            MBOOL                   handlePostView(
                                        IImageBuffer *pImgBuffer,
                                        MUINT32 const requestNo
                                    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SCallbackListener Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    ) override;

virtual MVOID   onDataReceived(
                    MUINT32 const requestNo,
                    StreamId_T const streamId,
                    MBOOL const errorResult,
                    android::sp<IImageBuffer>& pBuffer
                    ) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  CaptureCBHandler:
    //  To collect multiple capture callbacks and merge into one postProc request
    //  e.g. 3 capture requests for 1 postProc request
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class CaptureCBHandler
        : public virtual RefBase
    {
        public:
            CaptureCBHandler(
                MUINT32 const mainReqNo, MINT64 const timestamp,
                ShotParam const &shotParam, JpegParam const &jpegParam,
                MUINT32 const openIdMain1, MUINT32 const openIdMain2
            );

            virtual ~CaptureCBHandler();

            MBOOL registerCapReq(MUINT32 const openId, CaptureRequest& rCapReq, Vector<PipelineImageParam>& vDstStreams, Vector<StreamId>& vDstMetas);

            MBOOL isBelong(MUINT32 const openId, MUINT32 const reqNo);

            MUINT32 getMainReqNo() const {return mMainReqNo;};

            const std::vector<MINT32>& getAllBelongReq(MUINT32 const openId){ return mvAllBelongReq[openId];};

            MBOOL isAllPPSubmitted() const;

            MBOOL onBufFinished(MUINT32 const openId, MUINT32 const reqNo, StreamId const id, android::sp<IImageBuffer>& pBuffer);

            MBOOL onMetaFinished(MUINT32 const openId, MUINT32 const reqNo, StreamId const id, IMetadata const meta);

            MBOOL preparePostProcess();

            MBOOL triggerPostProcess();

            MVOID setIPostProcCB(MUINT32 const openId, sp<NSPostProc::IPostProcRequestCB> pCB) { mvIPostProcCB[openId] = pCB; };

            MVOID setIShotCB(sp<IShotCallback> pCB) { mpIShotCallback = pCB; };

            const std::vector<MINT32>& getPPReqNo(){return mvPendingPPReq;};

            MINT64 getTimestamp() const {return mTimestamp;};

            MVOID setBGServiceSupport(MBOOL val) { mbSupportBGService = val; };

            MVOID setForegroundPostviewSupport(MBOOL val) { mbSupportForegroundPostview = val; };

            MBOOL getForegroundPostviewSupport() const {return mbSupportForegroundPostview;};

        private:
            MUINT32 getOpenId() const {return -1;};

            MBOOL   reMapImageStreams(sp<PPData> rPPData);

            char const* getShotName() const;

            MBOOL   isFinalPP() const;

        private:
            const MUINT32                                       mMain1OpenId;
            const MUINT32                                       mMain2OpenId;

            const MUINT32                                       mMainReqNo;
            const MINT64                                        mTimestamp;
            const ShotParam                                     mShotParam;
            const JpegParam                                     mJpegParam;

            sp<IShotCallback>                                   mpIShotCallback = nullptr;

            MBOOL                                               mbSupportBGService          = MFALSE;
            MBOOL                                               mbSupportForegroundPostview = MFALSE;

            mutable Mutex                                       mLock;
            // one CaptureCBParcel list for one openId
            // In our case,
            // mCapReqCollect[Main1OpenId] is for Master sensor(main1)
            // mCapReqCollect[Main2OpenId] is for Slave sensor(main2)
            std::unordered_map<MUINT32, std::vector<CaptureCBParcel> >    mCapReqCollect;

            std::unordered_map<MUINT32, sp<NSPostProc::IPostProcRequestCB> > mvIPostProcCB;

            std::vector< sp<PPData> >                                     mvPendingPP;
            std::vector< MINT32 >                                         mvPendingPPReq;

            std::unordered_map<MUINT32, std::vector<MINT32> >             mvAllBelongReq;
    };
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  DualOperation:
    //  To provide unifined entry for all operations that is triggered by one shot and
    //  and need to be performed to both shot instances
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class DualOperation
    {
        public:
            enum class eOp
            {
                RETURN_CAP_BUFFERS
            };

        public:
            typedef MBOOL (DCMFShot::*Op)(MUINT32);
            typedef std::map<DualOperation::eOp, Op> OpTable;

        public:
            DualOperation();

            ~DualOperation();

            MBOOL registerShotInstance(sp<DCMFShot> pShot);

            MBOOL unRegisterShotInstance(sp<DCMFShot> pShot);

            MBOOL perform(DualOperation::eOp opName, MUINT32 arg);

        private:
            OpTable                            mOptable;

            Mutex                              mLock;
            std::map<MUINT32, wp<DCMFShot> >   mvShots;
    };

private:
    MBOOL           doCreateStreams();

    MBOOL           colletGlobalSettings();

    MBOOL           configVendor(plugin::InputInfo& inputInfo);

    MBOOL           getVendorInfo(plugin::InputInfo& inputInfo, plugin::OutputInfo& outputInfo);

    MBOOL           setVendorInputSetting(
                        plugin::OutputInfo& outputInfo,
                        plugin::InputSetting& inputSetting,
                        MINT32& rShotCount
                    );

    MBOOL           applyRawBufferSettings_dcmf(
                        Vector<SettingSet> vSettings,
                        MINT32 shotCount,
                        MINT32 delayCount
                    );

    MBOOL           submitRequestToPreviewPipeline(plugin::InputSetting& inputSetting);

    MBOOL           prepareCaptureRequests(
                        const MUINT32 mainReqNo,
                        const MUINT32 shotIdx,
                        IMetadata& shotAppSetting,
                        IMetadata& selectorAppSetting,
                        IMetadata& selectorHalSetting,
                        Vector<PipelineImageParam>& rvImageParam
                    );

    MBOOL           prepareImgBufProvider_Src(
                        Vector<PipelineImageParam>& rvImageParam,
                        sp<BufferCallbackHandler> pCbHandler
                    );

    MBOOL           prepareImgBufProvider_Dst(
                        Vector<PipelineImageParam>& rvImageParam,
                        sp<BufferCallbackHandler> pCbHandler,
                        MBOOL isMainFrame
                    );

    sp<ImageStreamManager>
                    createImgStreamMgr(const Vector<PipelineImageParam>& vImageParam);

    MBOOL           isMaster() const;

    MBOOL           isPureZsd() const {return mPureZSD;};

    MBOOL           getTimestamp(
                        IMetadata* metadata,
                        MINT64 &timestamp
                    );

    MERROR          submitCaptureRequest(
                        MBOOL     mainFrame,
                        IMetadata appSetting,
                        IMetadata halSetting,
                        const Vector<PipelineImageParam>& vImageParam
                    );

    MVOID           beforeCapture(Vector<PipelineImageParam>& rvImageParam);

    MVOID           afterCapture();

    MSize           getRRZOSize() const;

    MSize           getIMGOSize();

    MSize           getSMSize();

    MVOID           handleCaptureDoneCallback();

    MVOID           updateAppMeta(
                        const IMetadata* const appMeta_src,
                        const IMetadata* const halMeta_src,
                        IMetadata* const appMeta_dst
                    ) const;

    MBOOL           supportForegroundPostview(
                        const IMetadata* const appMeta
                    ) const;

    MVOID           updateHalMeta(IMetadata* const halMeta, MUINT32 const requestNo) const;

    MVOID           enterHighPerf();

    MVOID           exitHighPerf();

    MVOID           handleP2Done();

    MVOID           updateVendorFeatureTag(plugin::InputInfo& inputInfo) const;

    MVOID           updateVendorFeatureTag(IMetadata* const halMeta) const;

    MVOID           collectOneShotInfo();

    MVOID           returnStoredSelectorData();

    MVOID           handleDummyCallbacks();

    MBOOL           getSelectorDataAndStore(MINT32 const shotCount);

    status_t        getStoredSelectorData(IMetadata& rAppSetting,
                                          IMetadata& rHalSetting,
                                          android::sp<IImageBuffer>& pBuffer);
    sp<CaptureCBHandler>
                    getBelongCBHandlerLocked(const MUINT32 reqNo) const;

    MVOID           prepareAndTriggerPostProc(const MUINT32 reqNo);

private:
    list<MINT64>                                              mvPlugInKey;
    list<MINT64>                                              mvPlugIn_P2_Comb;

    MBOOL                                                     mPureZSD     = MFALSE;
    MBOOL                                                     mbBGService  = MFALSE;
    MBOOL                                                     mbDumpCali   = MFALSE;

    MINT32                                                    mMain1OpenId = -1;
    MINT32                                                    mMain2OpenId = -1;

    mutable Mutex                                             mAPILock;

    static  Mutex                                             gValidInsLock;
    static  MINT32                                            gValidIns;
    MBOOL                                                     mbVaild = MTRUE;

    mutable Mutex                                             mCapReqLock;
    mutable Condition                                         mCondCapReqLock;
    mutable Condition                                         mCondCapReqErased;
    std::map<MINT32, CaptureRequest>                          mvCaptureRequests;
    MUINT32                                                   mWorkerTriggerCnt = 0;

    MUINT32                                                   mDumpFlag = -1;
    MUINT32                                                   mDebugLog = 0;
    MUINT32                                                   mHDRSync  = 0;

    // Only one queue,  serve for both shot instance, one mainReqNo for one CaptureCBHandler
    static Mutex                                              gCapCBHandlerLock;
    static Condition                                          gCondCapCBHandlerLock;
    static std::map<MINT32, sp<DCMFShot::CaptureCBHandler> >  gvCapCBHandlers;

    MINT64                                                    mVendorMode = 0;

    std::set<StreamId>                                        mVCurrentUsingDst;

    static DCMFShot::DualOperation                            gDualOperation;

    mutable Mutex                                             mOnGoingPPLock;
    Condition                                                 mCondOnGoingPPLock;

    std::set<MINT32>                                          mvOnGoingPP;
    std::unordered_map<MINT32, MINT32 >                       mvPPReqMapping;
    MINT32                                                    mP2DoneCnt = 0;

    sp<IImageStreamInfo>                                      mpInfo_YuvSmall;

    MRect                                                     mFovCrop = MRect(0,0);

    MINT32                                                    mStereoWarning = 0;

    std::future<MBOOL>                                        mFuture;

    std::vector< std::vector<MUINT32> >                       mvCapShotBundle;

    std::list< StoredSelectorData >                           mvStoredSelectorData;
    MINT32                                                    mDebugTryCount = 0;

    // The fov cropping in capture may be different from preview,
    // so we have to compensate this value to results calculated from preview,
    // such as AF ROI, etc. The offset is in Active Array Domain.
    MSize                                                     mFOVOffset = MSize(0,0);
    MRect                                                     mActiveArray = MRect(0,0);

#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT && CCP_NOT_SUPPORT
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>          mpPipeline;
    sp<ImageCallback>                                         mpImageCallback;
    sp<BufferCallbackHandler>                                 mpCallbackHandler;
    sp<MetadataListener>                                      mpMetadataListener;
    sp<MetadataListener>                                      mpMetadataListenerFull;
#endif
};


/******************************************************************************
 *
 ******************************************************************************/
}; // NSDCMFShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_DCMFSHOT_H_

