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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_NODE_IMP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_NODE_IMP_H_

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1Common.h"
#include "P1Utility.h"
#include "P1ConnectLMV.h"
#include "P1TaskCtrl.h"
#include "P1DeliverMgr.h"
#include "P1RegisterNotify.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *
 ******************************************************************************/
class P1NodeImp
    : public BaseNode
    , public P1Node
    , public IHal3ACb
    , protected Thread
{
    friend class P1NodeAct;
    friend class P1QueAct;
    friend class P1QueJob;
    friend class P1TaskCollector;
    friend class P1TaskCtrl;
    friend class P1DeliverMgr;
    friend class P1RegisterNotify;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    //
    typedef std::vector< P1QueJob >  Que_T;
    //
    class Tag {
        public:
                                    Tag()
                                    : mInfo(0)
                                    , mLock()
                                    {
                                    };
                                    ~Tag()
                                    {
                                    };
            void                    clear(void)
                                    {
                                        RWLock::AutoWLock _wl(mLock);
                                        mInfo = 0;
                                    };
            void                    set(MUINT32 info)
                                    {   // for the performance consideration, set/get without locking, only for log usage.
                                        //RWLock::AutoWLock _wl(mLock);
                                        mInfo = info;
                                    };
            MUINT32                 get(void)
                                    {   // for the performance consideration, set/get without locking, only for log usage.
                                        //RWLock::AutoRLock _rl(mLock);
                                        return mInfo;
                                    }
        private:
            MUINT32                 mInfo;
            mutable RWLock          mLock;
    };
    //
    class Cfg {
        public:
                                    Cfg()
                                    : mSupportDynamicTwin(MFALSE)
                                    , mSensorNum(NSCam::NSIoPipe::NSCamIOPipe::E_1_SEN)
                                    , mQualityLv(eCamIQ_MAX)
                                    , mPattern(0)
                                    {
                                    };
                                    ~Cfg()
                                    {
                                    };
        public:
            MBOOL                   mSupportDynamicTwin;
            NSCam::NSIoPipe::NSCamIOPipe::E_SEN
                                    mSensorNum;
            E_CamIQLevel
                                    mQualityLv;
            MUINT32                 mPattern;
    };
    //
    class IndependentVerification
    {
    public:
                        IndependentVerification(
                            MINT32 nOpenId, MINT32 nLogLevel, MINT32 nLogLevelI,
                            MUINT32 ms, wp<P1NodeImp> wpP1NodeImp)
                            : mLock()
                            , mOpenId(nOpenId)
                            , mLogLevel(nLogLevel)
                            , mLogLevelI(nLogLevelI)
                            , mExit(MFALSE)
                            , mCount(0)
                            , mIntervalMs(ms)
                            , mThread()
                            , mwpP1NodeImp(wpP1NodeImp)
                        {
                            MY_LOGI0("[P1_IV][CTR] BGN ms(%d)", mIntervalMs);
                            mThread = std::thread(
                                [this]() mutable {
                                    loop();
                                });
                            MY_LOGI0("[P1_IV][CTR] END ms(%d)", mIntervalMs);
                        };

        virtual         ~IndependentVerification()
                        {
                            MY_LOGI0("[P1_IV][DTR] BGN cnt(%d)", mCount);
                            {
                                Mutex::Autolock _l(mLock);
                                mExit = MTRUE;
                            }
                            MY_LOGD0("[P1_IV][DTR] JOIN cnt(%d)", mCount);
                            if (mThread.joinable()) {
                                mThread.join();
                            };
                            mwpP1NodeImp = NULL;
                            MY_LOGI0("[P1_IV][DTR] END cnt(%d)", mCount);
                        };

    private:
        MVOID           loop(void)
                        {
                            MBOOL run = MFALSE;
                            //MBOOL exit = MFALSE;
                            mCount = 0;
                            char str[32] = {0};
                            do
                            {
                                if (quit()) break;
                                mCount ++;
                                run = exe();
                                if (quit()) break;
                                if (run) {
                                    snprintf(str, sizeof(str), "P1_IV:%d",
                                        mCount);
                                    P1_NOTE_SLEEP(str, mIntervalMs);
                                };
                                if (quit()) break;
                            } while (run);
                            return;
                        };

        MBOOL           quit(void)
                        {
                            Mutex::Autolock _l(mLock);
                            return mExit;
                        };

        MBOOL           exe(void);

    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MBOOL           mExit;
        MUINT32         mCount;
        MUINT32         mIntervalMs;
        std::thread     mThread;
        wp<P1NodeImp>   mwpP1NodeImp;
    };
    friend class P1NodeImp::IndependentVerification;
    //
    #if 0
    enum REQ_STATE
    {
        REQ_STATE_WAITING           = 0,
        REQ_STATE_ONGOING,
        REQ_STATE_RECEIVE,
        REQ_STATE_CREATED
    };
    #endif
    //
    enum START_CAP_STATE
    {
        START_CAP_STATE_NONE        = 0,
        START_CAP_STATE_WAIT_REQ,
        START_CAP_STATE_WAIT_CB,
        START_CAP_STATE_READY
    };
    //
    enum IO_PIPE_EVT_STATE
    {
        IO_PIPE_EVT_STATE_NONE      = 0,
        IO_PIPE_EVT_STATE_ACQUIRING,
        IO_PIPE_EVT_STATE_ACQUIRED  // after acquiring, wait for releasing
    };
    //
protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    mutable Mutex                   mInitLock;
    MBOOL                           mInit;

protected:  ////                    Data Members. (Config)
    mutable Mutex                   mPowerNotifyLock;
    MBOOL                           mPowerNotify;

protected:  ////                    Data Members. (Config)
    mutable Mutex                   mLaunchStateLock;
    MUINT8                          mLaunchState;

    SortedVector<StreamId_T>        mInStreamIds;
    sp<IMetaStreamInfo>             mvStreamMeta[STREAM_META_NUM];
    sp<IImageStreamInfo>            mvStreamImg[STREAM_IMG_NUM];
    char                            maStreamMetaName[STREAM_META_NUM]
                                        [P1_STREAM_NAME_LEN] = {
                                            {"InAPP"},  //STREAM_META_IN_APP
                                            {"InHAL"},  //STREAM_META_IN_HAL
                                            {"OutAPP"}, //STREAM_META_OUT_APP
                                            {"OutHAL"}  //STREAM_META_OUT_HAL
                                        };
    char                            maStreamImgName[STREAM_IMG_NUM]
                                        [P1_STREAM_NAME_LEN] = {
                                            {"InYUV"},  //STREAM_IMG_IN_YUV
                                            {"InOPQ"},  //STREAM_IMG_IN_OPAQUE
                                            {"OutOPQ"}, //STREAM_IMG_OUT_OPAQUE
                                            {"OutIMG"}, //STREAM_IMG_OUT_FULL
                                            {"OutRRZ"}, //STREAM_IMG_OUT_RESIZE
                                            {"OutLCS"}, //STREAM_IMG_OUT_LCS
                                            {"OutRSS"}  //STREAM_IMG_OUT_RSS
                                        };
    //
    SensorParams                    mSensorParams;
    //RawParams                       mRawParams;
    sp<IImageStreamBufferPoolT>     mpStreamPool_full;
    sp<IImageStreamBufferPoolT>     mpStreamPool_resizer;
    sp<IImageStreamBufferPoolT>     mpStreamPool_lcso;
    sp<IImageStreamBufferPoolT>     mpStreamPool_rsso;
    MUINT8                          mBurstNum;
    MUINT8                          mDepthNum;

    MBOOL                           mRawPostProcSupport;
    MBOOL                           mRawProcessed;
    RAW_DEF_TYPE                    mRawSetDefType;

    /**
     * the raw default type, if the request do not set the raw type,
     * it will use this setting to driver
     */
    MUINT32                         mRawDefType;

    /**
     * the raw type option means the capability recorded in each bit,
     * it is decided after the driver configuration
     */
    MUINT32                         mRawOption;
    MBOOL                           mDisableFrontalBinning;
    MBOOL                           mDisableDynamicTwin;

    MBOOL                           mEnableEISO;
    MBOOL                           mForceSetEIS;
    MUINT64                         mPackedEisInfo;
    MBOOL                           mEnableLCSO;
    MBOOL                           mEnableRSSO;
    MBOOL                           mEnableFSC;
    MBOOL                           mEnableDualPD;
    MBOOL                           mEnableQuadCode;
    MBOOL                           mEnableUniForcedOn;

    /**
     * mEnableSecure is SecurityCam purpose.
     * It can notify all 3A module
     */
    MBOOL                           mEnableSecure;

    /**
     * mSecType can identify the secure type (TEE/MTEE)
     */
    SecType                         mSecType;

    /**
     * mSecStatus is represented as secure handle to notify the secure data source
     */
    MUINT32                         mSecStatus;

    MBOOL                           mDisableHLR; // true:force-off false:auto
    PIPE_MODE                       mPipeMode; //(EPipeSelect)
    MUINT32                         mPipeBit; //(E_CAM_PipelineBitDepth_SEL)

    NSCam::IMetadata                mCfgAppMeta;
    NSCam::IMetadata                mCfgHalMeta;

    RESIZE_QUALITY                  mResizeQuality;

    MUINT8                          mTgNum;

    MINT                            mRawFormat;
    MUINT32                         mRawStride;
    MUINT32                         mRawLength;

    REV_MODE                        mReceiveMode;
    MUINT                           mSensorFormatOrder;
    MUINT32                         mSensorRollingSkewNs;

    mutable Mutex                   mQualitySwitchLock;
    MBOOL                           mQualitySwitching;

    LongExposureStatus              mLongExp;
    //Storage                         mImageStorage;

protected:  ////                    Data Members. (System capability)
    static const int                mNumInMeta = 2;
    static const int                mNumOutMeta = 3;
    int                             m3AProcessedDepth;
    int                             mNumHardwareBuffer;
    int                             mDelayframe;

protected:  ////
    MUINT32                         mLastNum;
    mutable Mutex                   mLastNumLock;
    MUINT32                         mLastSofIdx;
    MINT32                          mLastSetNum;

protected:  ////                    Data Members. (Hardware)
    mutable Mutex                   mHardwareLock;
    mutable Mutex                   mStopSttLock;
    //
    #if (USING_DRV_IO_PIPE_EVENT)
    IO_PIPE_EVT_STATE               mIoPipeEvtState;
    mutable RWLock                  mIoPipeEvtStateLock;
    //
    mutable Mutex                   mIoPipeEvtWaitLock;
    Condition                       mIoPipeEvtWaitCond;
    MBOOL                           mIoPipeEvtWaiting;
    //
    mutable Mutex                   mIoPipeEvtOpLock;
    MBOOL                           mIoPipeEvtOpAcquired;
    MBOOL                           mIoPipeEvtOpLeaving;
    android::sp<IoPipeEventHandle>  mspIoPipeEvtHandleAcquire;
    android::sp<IoPipeEventHandle>  mspIoPipeEvtHandleRelease;
    //
    #endif
    MUINT32                         mCamIOVersion;
    INormalPipe*                    mpCamIO;
    IHal3A_T*                       mp3A;
    #if SUPPORT_VHDR
    VHdrHal*                        mpVhdr;
    #endif
    #if SUPPORT_LCS
    LcsHal*                         mpLCS;
    #endif
    #if SUPPORT_RSS
    sp<RssHal>                      mpRSS;
    #endif
    #if SUPPORT_FSC
    sp<FSCHal>                      mpFSC;
    #endif
    //
    Cfg                             mCfg;
    //
    MUINT8                          mTimestampSrc;
    MRect                           mActiveArray;
    MUINT32                         mPixelMode;
    //
    MUINT32                         mConfigPort;
    MUINT32                         mConfigPortNum;
    MBOOL                           mCamCfgExp; // CamIO ConfigPipe Exception
    MBOOL                           mIsBinEn;
    MBOOL                           mIsDynamicTwinEn;
    MBOOL                           mIsLegacyStandbyMode;
    MINT8                           mForceStandbyMode;
    //
    MINT32                          mResizeRatioMax;
    //
    mutable Mutex                   mCurBinLock;
    MSize                           mCurBinSize;
    //
    android::wp<INodeCallbackToPipeline>
                                    mwpPipelineCb;
    mutable Mutex                   mPipelineCbLock;
    mutable Mutex                   mLastFrmReqNumLock;
    MINT32                          mLastFrmNum;
    MINT32                          mLastReqNum;
    MINT32                          mLastCbCnt;
    //
    MINT64                          mMonitorTime;
    mutable Mutex                   mMonitorLock;
    //
    //DefaultKeyedVector< sp<IImageBuffer>, android::String8 >
    //                                mvStuffBufferInfo;
    //mutable Mutex                   mStuffBufferLock;
    //
    MUINT8                          mStuffBufNumBasis;
    MUINT8                          mStuffBufNumMax;
    StuffBufferManager              mStuffBufMgr;
    //
    #define DRAWLINE_PORT_RRZO      0x1
    #define DRAWLINE_PORT_IMGO      0x2
    MUINT32                         mDebugScanLineMask;
    DebugScanLine*                  mpDebugScanLine;
    //
    MUINT32                         mIvMs;
    P1NodeImp::IndependentVerification *
                                    mpIndependentVerification;

protected:  ////                    Data Members. (Queue: Request)
    mutable Mutex                   mRequestQueueLock;
    //Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;

#if USING_CTRL_3A_LIST_PREVIOUS
    List<MetaSet_T>                 mPreviousCtrlList;
#endif
    //
    mutable Mutex                   mFrameSetLock;
    MBOOL                           mFrameSetAlready;
    //
    MBOOL                           mFirstReceived;
    //
    mutable Mutex                   mStartCaptureLock;
    Condition                       mStartCaptureCond;
    START_CAP_STATE                 mStartCaptureState;
    MUINT32                         mStartCaptureType;
    MUINT32                         mStartCaptureIdx;
    MINT64                          mStartCaptureExp;

protected:  ////
    //AAAResult                       m3AStorage;

protected:  ////                    Data Members. (Queue: Processing)
    mutable Mutex                   mProcessingQueueLock;
    Condition                       mProcessingQueueCond;
    Que_T                           mProcessingQueue;

protected:  ////                    Data Members. (Queue: drop)
    mutable Mutex                   mDropQueueLock;
    std::vector<MINT32>             mDropQueue;

protected:  ////                    Data Members.
    mutable Mutex                   mTransferJobLock;
    Condition                       mTransferJobCond;
    MINT32                          mTransferJobIdx;
    MBOOL                           mTransferJobWaiting;

protected:  ////                    Data Members.
    mutable Mutex                   mStartLock;
    Condition                       mStartCond;

protected:  ////                    Data Members.
    mutable Mutex                   mThreadLock;
    Condition                       mActiveCond;
    Condition                       mReadyCond;

protected:  ////                    Data Members.
    DurationProfile                 mDequeThreadProfile;

protected:  ////                    Data Members.
    mutable Mutex                   mPublicLock;

protected:  ////                    Data Members.
    MINT32                          mInFlightRequestCnt;

protected:  ////                    Data Members.
    sp<P1DeliverMgr>                mpDeliverMgr;

protected:  ////                    Data Members.
    sp<P1RegisterNotify>            mpRegisterNotify;

protected:  ////                    Data Members.
    sp<P1TaskCtrl>                  mpTaskCtrl;

protected:  ////                    Data Members.
    sp<P1TaskCollector>             mpTaskCollector;

protected:  ////                    Data Members.
    sp<P1ConnectLMV>                mpConnectLMV;

protected:  ////                    Data Members.
    sp<ConcurrenceControl>          mpConCtrl;

protected:  ////                    Data Members.
    sp<HardwareStateControl>        mpHwStateCtrl;

protected:  ////                    Data Members.
    sp<TimingCheckerMgr>            mpTimingCheckerMgr;
    MUINT32                         mTimingFactor;

protected:  ////                    Data Members.
    sp<NSCam::v3::Utils::Imp::ISyncHelper>
                                    mspSyncHelper;
    mutable Mutex                   mSyncHelperLock;
    MBOOL                           mSyncHelperReady;

protected:  ////                    Data Members.
    sp<IResourceConcurrency>        mspResConCtrl;
    IResourceConcurrency::CLIENT_HANDLER
                                    mResConClient;
    MBOOL                           mIsResConGot;

protected:  ////                    Data Members.
    LogInfo                         mLogInfo;

protected:
    MINT32                          mLogLevel;
    MINT32                          mLogLevelI;
    MINT32                          mSysLevel;
    MINT32                          mMetaLogOp;
    MUINT32                         mMetaLogTag;
    MINT32                          mCamDumpEn;
    MINT32                          mEnableDumpRaw;
    MINT32                          mDisableDeliverReleasing;
    MINT32                          mDisableAEEIS;
    Tag                             mTagReq;
    Tag                             mTagSet;
    Tag                             mTagEnq;
    Tag                             mTagDeq;
    Tag                             mTagOut;
    Tag                             mTagList;
    //
    FrameNote                       mNoteRelease;
    FrameNote                       mNoteDispatch;

protected:  ////                    Data Members.
    MUINT32                         mInitReqSet; // the request set from user
    MUINT32                         mInitReqNum; // the total number need to receive
    MUINT32                         mInitReqCnt; // the currently received count
    MBOOL                           mInitReqOff; // the initial request flow disable

protected:  ////                    Data Members.
    MBOOL                           mEnableCaptureFlow;
    MBOOL                           mEnableCaptureOff;
    MBOOL                           mEnableFrameSync;
    MBOOL                           mNeedHwReady;   // for the specific flow (InitReq, StartCap, ... )
                                                    // it need to check the hardware ready before leave
    MBOOL                           mStereoCamMode;

protected:  ////                    Operations.
    MVOID                           setLaunchState(
                                        MUINT8 state
                                    );

    MUINT8                          getLaunchState(
                                        void
                                    );

    MBOOL                           isActive(
                                        void
                                    );

    MBOOL                           isReady(
                                        void
                                    );

    MBOOL                           isActiveButNotReady(
                                        void
                                    );

    MVOID                           setInit(
                                        MBOOL init
                                    );

    MBOOL                           getInit(
                                        void
                                    );

    MVOID                           setPowerNotify(
                                        MBOOL notify
                                    );

    MBOOL                           getPowerNotify(
                                        void
                                    );

    MVOID                           setQualitySwitching(
                                        MBOOL switching
                                    );

    MBOOL                           getQualitySwitching(
                                        void
                                    );

    MVOID                           setCurrentBinSize(
                                        MSize size
                                    );

    MSize                           getCurrentBinSize(
                                        void
                                    );

    MVOID                           lastFrameRequestInfoUpdate(
                                        MINT32 const frameNum,
                                        MINT32 const requestNum
                                    );

    MINT32                          lastFrameRequestInfoNotice(
                                        MINT32 & frameNum,
                                        MINT32 & requestNum,
                                        MINT32 const addCbCnt = 0
                                    );

    MVOID                           syncHelperStart(
                                        void
                                    );

    MVOID                           syncHelperStop(
                                        void
                                    );

    MVOID                           ensureStartReady(
                                        MUINT8 infoType,
                                        MINT32 infoNum = P1_MAGIC_NUM_INVALID
                                    );

    MVOID                           onRequestFrameSet(
                                        MBOOL initial = MFALSE
                                    );

    MVOID                           recovery(
                                        MUINT8 state
                                    );

    MERROR                          abandonRequest(
                                        MBOOL bWaitDone
                                    );

    MVOID                           setRequest(
                                        MBOOL initial = MFALSE
                                    );

    MBOOL                           acceptRequest(
                                        sp<IPipelineFrame> pFrame,
                                        MUINT32 & rRevResult
                                    );

    MBOOL                           beckonRequest(
                                        void
                                    );

    MBOOL                           checkReqCnt(
                                        MINT32 & cnt
                                    );

    MVOID                           onSyncEnd(
                                        void
                                    );

    MVOID                           onSyncBegin(
                                        MBOOL initial,
                                        RequestSet_T* reqSet = NULL,//MUINT32 magicNum = 0,
                                        MUINT32 sofIdx = P1SOFIDX_INIT_VAL,
                                        CapParam_T* capParam = NULL
                                    );

    MERROR                          fetchJob(
                                        P1QueJob & rOutJob
                                    );

#if 0
    MVOID                           onProcess3AResult(
                                        MUINT32 magicNum,
                                        MUINT32 key,
                                        MUINT32 val
                                    );
#endif

    MERROR                          onProcessEnqueFrame(
                                        P1QueJob & job
                                    );

    MERROR                          onProcessDequeFrame(
                                    );

    MERROR                          onProcessDropFrame(
                                        MBOOL isTrigger = MFALSE
                                    );

    MVOID                           onCheckDropFrame(
                                        void
                                    );

    MBOOL                           getProcessingFrame_ByAddr(
                                        IImageBuffer* const imgBuffer,
                                        MINT32 magicNum,
                                        P1QueJob & job
                                    );

    P1QueJob                        getProcessingFrame_ByNumber(
                                        MINT32 magicNum
                                    );


    MVOID                           onHandleFlush(
                                        MBOOL wait,
                                        MBOOL isNextOff = MFALSE
                                    );

    MVOID                           processRedoFrame(
                                        P1QueAct & rAct
                                    );

    MVOID                           processYuvFrame(
                                        P1QueAct & rAct
                                    );

    MVOID                           onReturnFrame(
                                        P1QueAct & rAct,
                                        FLUSH_TYPE flushType,
                                        MBOOL isTrigger = MTRUE
                                    );

    MVOID                           releaseAction(
                                        P1QueAct & rAct
                                    );

    MVOID                           releaseFrame(
                                        P1FrameAct & rFrameAct
                                    );

    MVOID                           onProcessResult(
                                        P1QueAct & rAct,
                                        QBufInfo const &deqBuf,
                                        MetaSet_T const &result3A,
                                        IMetadata const &resultAppend,
                                        MUINT32 const index = 0
                                    );

    MBOOL                           findPortBufIndex(
                                        QBufInfo & deqBuf,
                                        P1QueJob & job
                                    );

    MVOID                           createAction(
                                        P1QueAct & rAct,
                                        sp<IPipelineFrame> appFrame = NULL,
                                        REQ_TYPE eType = REQ_TYPE_UNKNOWN
                                    );

#if (USING_DRV_IO_PIPE_EVENT)
    MVOID                           eventStreamingInform(
                                    );

    MVOID                           eventStreamingOn(
                                    );

    MVOID                           eventStreamingOff(
                                    );
#endif

protected:  ////                    Hardware Operations.
    MERROR                          hardwareOps_start(
                                    );

    MERROR                          hardwareOps_enque(
                                        P1QueJob & job,
                                        ENQ_TYPE type = ENQ_TYPE_NORMAL,
                                        MBOOL ready = MFALSE,
                                        MINT64 data = 0
                                    );

    MERROR                          hardwareOps_deque(
                                        QBufInfo &deqBuf
                                    );

    MERROR                          hardwareOps_stop(
                                    );

    MERROR                          hardwareOps_request(
                                    );

    MERROR                          hardwareOps_capture(
                                    );

    MERROR                          hardwareOps_ready(
                                    );

#if 0
    MERROR                          hardwareOps_standby(
                                        List<MetaSet_T> *list,
                                        Mutex *listLock
                                    );

    MVOID                           hardwareOps_standby(
                                    );
#endif

    MERROR                          hardwareOps_streaming(
                                    );

    MERROR                          procedureAid_start(
                                    );

    MERROR                          buildInitItem(
                                    );

    MERROR                          setupAction(
                                        P1QueAct & act,
                                        QBufInfo & info
                                    );

    MERROR                          createStuffBuffer(
                                        sp<IImageBuffer> & imageBuffer,
                                        sp<IImageStreamInfo> const& streamInfo,
                                        NSCam::MSize::value_type const
                                            changeHeight = 0
                                    );

    MERROR                          createStuffBuffer(
                                        sp<IImageBuffer> & imageBuffer,
                                        char const * szName,
                                        MINT32 format,
                                        MSize size,
                                        std::vector<MUINT32> & vStride,
                                        MBOOL secureOn = MFALSE
                                    );

    MERROR                          destroyStuffBuffer(
                                        sp<IImageBuffer> & imageBuffer
                                    );

    MVOID                           generateAppMeta(
                                        P1QueAct & act,
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata &appMetadata,
                                        MUINT32 const index = 0
                                    );

    MVOID                           generateAppTagIndex(
                                        IMetadata &appMetadata,
                                        IMetadata &appTagIndex
                                    );

    MVOID                           generateHalMeta(
                                        P1QueAct & act,
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata const &resultAppend,
                                        IMetadata const &inHalMetadata,
                                        IMetadata &halMetadata,
                                        MUINT32 const index = 0
                                    );

#if USING_CTRL_3A_LIST
    MVOID                           generateCtrlList(
                                        List<MetaSet_T> * pList,
                                        P1QueJob & rJob
                                    );
#endif

    MVOID                           generateCtrlQueue(
                                        std::vector< MetaSet_T* > & rQue,
                                        P1QueJob & rJob
                                    );

    MVOID                           prepareCropInfo(
                                        P1QueAct & rAct,
                                        IMetadata* pAppMetadata,
                                        IMetadata* pHalMetadata,
                                        PREPARE_CROP_PHASE phase,
                                        MBOOL * pCtrlFlush = NULL
                                    );

    MERROR                          check_config(
                                        ConfigParams const& rParams
                                    );

    MERROR                          checkConstraint(
                                        void
                                    );

    MERROR                          attemptCtrlSync(
                                        P1QueAct & rAct
                                    );

    MERROR                          attemptCtrlSetting(
                                        P1QueAct & rAct
                                    );

    MERROR                          attemptCtrlResize(
                                        P1QueAct & rAct,
                                        MBOOL & rIsChanged
                                    );

    MERROR                          attemptCtrlReadout(
                                        P1QueAct & rAct,
                                        IMetadata * pAppMetadata,
                                        IMetadata * pHalMetadata,
                                        MBOOL & rIsChanged
                                    );

    MERROR                          notifyCtrlSync(
                                        P1QueAct & rAct
                                    );

    MERROR                          notifyCtrlMeta(
                                        IPipelineNodeCallback::eCtrlType eType,
                                        P1QueAct & rAct,
                                        STREAM_META const streamAppMeta,
                                        IMetadata * pAppMetadata,
                                        STREAM_META const streamHalMeta,
                                        IMetadata * pHalMetadata,
                                        MBOOL & rIsChanged
                                    );

    MERROR                          requestMetadataEarlyCallback(
                                        P1QueAct & act,
                                        STREAM_META const streamMeta,
                                        IMetadata * pMetadata
                                    );

    MVOID                           checkBufferDumping(
                                        P1QueAct & rAct
                                    );

    MVOID                           inflightMonitoring(
                                        INFLIGHT_MONITORING_TIMING
                                            timing = IMT_COMMON
                                    );

    MUINT32                         get_and_increase_magicnum()
                                    {
                                        Mutex::Autolock _l(mLastNumLock);
                                        MUINT32 ret = mLastNum++;
                                        //skip num = 0 as 3A would callback 0 when request stack is empty
                                        //skip -1U as a reserved number to indicate that which would never happen in 3A queue
                                        if(ret==0 || ret==-1U) ret=mLastNum=1;
                                        return ret;
                                    }

    MUINT32                         get_last_magicnum()
                                    {
                                        Mutex::Autolock _l(mLastNumLock);
                                        MUINT32 ret =
                                            (mLastNum > 0) ? (mLastNum - 1) : 0;
                                        return ret;
                                    }

    MBOOL                           isRevMode(REV_MODE mode)
                                    {
                                        return (mode == mReceiveMode) ?
                                            (MTRUE) : (MFALSE);
                                    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MVOID                           dispatch(
                                        sp<IPipelineFrame> pFrame
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

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
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    P1NodeImp();
    virtual                         ~P1NodeImp();
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
                                        sp<IPipelineFrame> pFrame
                                    );

    virtual MERROR                  kick();

    virtual MERROR                  setNodeCallBack(
                                        android::wp<INodeCallbackToPipeline> pCallback
                                    );

public:     ////                    Operations.

    virtual void                    doNotifyCb (
                                        MINT32  _msgType,
                                        MINTPTR _ext1,
                                        MINTPTR _ext2,
                                        MINTPTR _ext3
                                    );

    static void                     doNotifyDropframe(MUINT magicNum, void* cookie);

#if (USING_DRV_IO_PIPE_EVENT)
    static NSCam::NSIoPipe::IoPipeEventCtrl
                                    onEvtCtrlAcquiring(P1NodeImp * user,
                                        NSCam::NSIoPipe::IpRawP1AcquiringEvent & evt);
    static NSCam::NSIoPipe::IoPipeEventCtrl
                                    onEvtCtrlReleasing(P1NodeImp * user,
                                        NSCam::NSIoPipe::IpRawP1ReleasedEvent & evt);
#endif

};

};//namespace NSP1Node
};//namespace v3
};//namespace NSCam

#endif//_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_NODE_IMP_H_

