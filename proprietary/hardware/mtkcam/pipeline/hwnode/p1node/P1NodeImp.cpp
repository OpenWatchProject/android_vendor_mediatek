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

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1NodeImp.h"
#include "P1TaskCtrl.h"
#include "P1DeliverMgr.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::EIS;
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
P1NodeImp::
P1NodeImp()
    : BaseNode()
    , P1Node()
    , mConfigRWLock()
    , mInitLock()
    , mInit(MTRUE)
    //
    , mPowerNotifyLock()
    , mPowerNotify(MFALSE)
    //
    , mLaunchStateLock()
    , mLaunchState(NSP1Node::LAUNCH_STATE_NULL)
    //
    , mpStreamPool_full(NULL)
    , mpStreamPool_resizer(NULL)
    , mpStreamPool_lcso(NULL)
    , mpStreamPool_rsso(NULL)
    , mBurstNum(1)
    , mDepthNum(1)
    , mRawPostProcSupport(MTRUE)
    , mRawProcessed(MFALSE)
    , mRawSetDefType(RAW_DEF_TYPE_AUTO)
    , mRawDefType(EPipe_PURE_RAW)
    , mRawOption(0)
    , mDisableFrontalBinning(MFALSE)
    , mDisableDynamicTwin(MFALSE)
    , mEnableEISO(MFALSE)
    , mForceSetEIS(MFALSE)
    , mPackedEisInfo(0)
    , mEnableLCSO(MFALSE)
    , mEnableRSSO(MFALSE)
    , mEnableFSC(MFALSE)
    , mEnableDualPD(MFALSE)
    , mEnableQuadCode(MFALSE)
    , mEnableUniForcedOn(MFALSE)
    , mEnableSecure(MFALSE)
    , mSecType(SecType::mem_normal)
    , mSecStatus(0)
    , mDisableHLR(MFALSE)
    , mPipeMode(PIPE_MODE_NORMAL)
    , mPipeBit(CAM_Pipeline_12BITS)
    , mCfgAppMeta()
    , mCfgHalMeta()
    , mResizeQuality(RESIZE_QUALITY_UNKNOWN)
    , mTgNum(0)
    //
    , mRawFormat(P1_IMGO_DEF_FMT)
    , mRawStride(0)
    , mRawLength(0)
    //
    , mReceiveMode(REV_MODE_NORMAL)
    , mSensorFormatOrder(SENSOR_FORMAT_ORDER_NONE)
    , mSensorRollingSkewNs(P1_SENSOR_ROLLING_SKEW_UNKNOWN)
    //
    , mQualitySwitchLock()
    , mQualitySwitching(MFALSE)
    //
    //, mImageStorage()
    //
    , m3AProcessedDepth(3)
    , mNumHardwareBuffer(3)
    , mDelayframe(3)
    , mLastNum(1)
    , mLastNumLock()
    , mLastSofIdx(P1SOFIDX_NULL_VAL)
    , mLastSetNum(0)
    , mHardwareLock()
    , mStopSttLock()
    #if (USING_DRV_IO_PIPE_EVENT)
    , mIoPipeEvtState(IO_PIPE_EVT_STATE_NONE)
    , mIoPipeEvtStateLock()
    , mIoPipeEvtWaitLock()
    , mIoPipeEvtWaitCond()
    , mIoPipeEvtWaiting(MFALSE)
    , mIoPipeEvtOpLock()
    , mIoPipeEvtOpAcquired(MFALSE)
    , mIoPipeEvtOpLeaving(MFALSE)
    , mspIoPipeEvtHandleAcquire(NULL)
    , mspIoPipeEvtHandleRelease(NULL)
    #endif
    , mCamIOVersion(0)
    , mpCamIO(NULL)
    , mp3A(NULL)
    #if SUPPORT_VHDR
    , mpVhdr(NULL)
    #endif
    #if SUPPORT_LCS
    , mpLCS(NULL)
    #endif
    #if SUPPORT_RSS
    , mpRSS(NULL)
    #endif
    #if SUPPORT_FSC
    , mpFSC(NULL)
    #endif
    //
    , mCfg()
    //
    , mTimestampSrc(MTK_SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME)
    , mPixelMode(0)
    //
    , mConfigPort(CONFIG_PORT_NONE)
    , mConfigPortNum(0)
    , mCamCfgExp(MFALSE)
    , mIsBinEn(MFALSE)
    , mIsDynamicTwinEn(MFALSE)
    , mIsLegacyStandbyMode(MFALSE)
    , mForceStandbyMode(0)
    //
    , mResizeRatioMax(RESIZE_RATIO_MAX_100X)
    //
    , mCurBinLock()
    , mCurBinSize()
    //
    , mwpPipelineCb(NULL)
    , mPipelineCbLock()
    , mLastFrmReqNumLock()
    , mLastFrmNum(P1_FRM_NUM_NULL)
    , mLastReqNum(P1_REQ_NUM_NULL)
    , mLastCbCnt(0)
    //
    , mMonitorTime(0)
    , mMonitorLock()
    //
    , mStuffBufNumBasis(P1NODE_STUFF_BUFFER_WATER_MARK)
    , mStuffBufNumMax(P1NODE_STUFF_BUFFER_MAX_AMOUNT)
    , mStuffBufMgr()
    //
    , mDebugScanLineMask(0)
    , mpDebugScanLine(NULL)
    //
    , mIvMs(0)
    , mpIndependentVerification(NULL)
    //
    , mRequestQueueLock()
    , mRequestQueue()
    //
    #if USING_CTRL_3A_LIST_PREVIOUS
    , mPreviousCtrlList()
    #endif
    //
    , mFrameSetLock()
    , mFrameSetAlready(MFALSE)
    //
    , mFirstReceived(MFALSE)
    //
    , mStartCaptureLock()
    , mStartCaptureCond()
    , mStartCaptureState(START_CAP_STATE_NONE)
    , mStartCaptureType(E_CAPTURE_NORMAL)
    , mStartCaptureIdx(0)
    , mStartCaptureExp(0)
    //
    //, m3AStorage()
    //
    , mProcessingQueueLock()
    , mProcessingQueueCond()
    , mProcessingQueue()
    //
    , mDropQueueLock()
    , mDropQueue()
    //
    , mTransferJobLock()
    , mTransferJobCond()
    , mTransferJobIdx(P1ACT_ID_NULL)
    , mTransferJobWaiting(MFALSE)
    //
    , mStartLock()
    , mStartCond()
    //
    , mThreadLock()
    , mActiveCond()
    , mReadyCond()
    //
    , mDequeThreadProfile("P1Node::deque", 30000000LL)
    , mInFlightRequestCnt(0)
    //
    , mpDeliverMgr(NULL)
    //
    , mpConnectLMV(NULL)
    //
    , mpConCtrl(NULL)
    //
    , mpHwStateCtrl(NULL)
    //
    , mpTimingCheckerMgr(NULL)
    , mTimingFactor(1)
    //
    , mspSyncHelper(NULL)
    , mSyncHelperLock()
    , mSyncHelperReady(MFALSE)
    //
    , mspResConCtrl(NULL)
    , mResConClient(IResourceConcurrency::CLIENT_HANDLER_NULL)
    , mIsResConGot(MFALSE)
    //
    , mLogInfo()
    //
    , mLogLevel(0)
    , mLogLevelI(0)
    , mSysLevel(P1_SYS_LV_DEFAULT)
    , mMetaLogOp(0)
    , mMetaLogTag(0)
    , mCamDumpEn(0)
    , mEnableDumpRaw(0)
    , mDisableDeliverReleasing(0)
    , mDisableAEEIS(0)
    , mTagReq()
    , mTagSet()
    , mTagEnq()
    , mTagDeq()
    , mTagOut()
    , mTagList()
    //
    , mNoteRelease(P1NODE_FRAME_NOTE_SLOT_SIZE_DEF)
    , mNoteDispatch(P1NODE_FRAME_NOTE_SLOT_SIZE_DEF)
    //
    , mInitReqSet(0)
    , mInitReqNum(0)
    , mInitReqCnt(0)
    , mInitReqOff(MFALSE)
    //
    , mEnableCaptureFlow(MFALSE)
    , mEnableCaptureOff(MFALSE)
    , mEnableFrameSync(MFALSE)
    , mNeedHwReady(MFALSE)
    , mStereoCamMode(MFALSE)
{
    MINT32 cam_log = ::property_get_int32("vendor.debug.camera.log", 0);
    MINT32 p1_log = ::property_get_int32("vendor.debug.camera.log.p1node", 1);
    MINT32 p1_logi = ::property_get_int32("vendor.debug.camera.log.p1nodei", 0);
    MINT32 g_log = ::property_get_int32("persist.vendor.mtk.camera.log_level", 0); // global log level control
    MINT32 g_log_lv = (g_log >= 2) ?  (g_log - 2) : (0); // 2:I:USER 3:D:USERDEBUG 4:V:ENG
    mLogLevel = MAX(cam_log, p1_log);
#if 0 // force to enable all p1 node log
    #warning "[FIXME] force to enable P1Node log"
    if (mLogLevel < 2) {
        mLogLevel = 2;
    }
#endif
    MBOOL buildLogD = MFALSE;
    MBOOL buildLogI = MFALSE;
#if (IS_P1_LOGI)
    //#warning "IS_P1_LOGI build LogI"
    mLogLevelI = (mLogLevel > 0) ? (mLogLevel - 1) : (mLogLevel);
    buildLogI = MTRUE;
#endif
#if (IS_P1_LOGD)
    //#warning "IS_P1_LOGD build LogD"
    mLogLevelI = mLogLevel;
    buildLogD = MTRUE;
#endif
    if (p1_log > 1) {
        mLogLevelI = mLogLevel;
    }
    //
    if (p1_logi > 0) {
        mLogLevelI = p1_logi;
    }
    mLogLevel = MAX(mLogLevel, g_log_lv);
    mLogLevelI = MAX(mLogLevelI, g_log_lv);
    //
    MINT32 g_sys = P1_SYS_LV_DEFAULT;
    MINT32 g_sys_set = 1;
    #if 1 // decide by global-setting
    g_sys_set = ::property_get_int32("vendor.debug.mtkcam.systrace.level",
        MTKCAM_SYSTRACE_LEVEL_DEFAULT);
    #endif
    g_sys = (g_sys_set > 0) ? P1_SYS_LV_DEFAULT : P1_SYS_LV_CRITICAL;
    mSysLevel = g_sys;
    MINT32 p1sys = ::property_get_int32("vendor.debug.camera.log.p1nodesys", 9);
    if (p1sys < 9) { // update by manual-setting
        // =0 : forced-off all P1_TRACE
        // =1 : basic-on P1_TRACE-Lv==1 (P1_SYS_LV_BASIC)
        // =2 : critical-on P1_TRACE-Lv<=2 (P1_SYS_LV_CRITICAL)
        // =3 : default-on P1_TRACE-Lv<=3 (P1_SYS_LV_DEFAULT)
        // >3 : manually-on
        mSysLevel = p1sys;
    };
    //
    MINT32 pMetaLogOp =
        property_get_int32("vendor.debug.camera.log.p1nodemeta", 0);
    MINT32 pMetaLogTag =
        property_get_int32("vendor.debug.camera.log.p1nodemetatag", 0);
    mMetaLogOp = pMetaLogOp;
    mMetaLogTag = pMetaLogTag;
    if (mMetaLogTag != 0) {
        mMetaLogOp = 1;
    }
    //
    mCamDumpEn = property_get_int32("vendor.debug.camera.dump.en", 0);
    //
    mEnableDumpRaw = property_get_int32("vendor.debug.feature.forceEnableIMGO", 0);
    //
    mDisableAEEIS = property_get_int32("vendor.debug.eis.disableae", 0);
    //
    mDebugScanLineMask = ::property_get_int32("vendor.debug.camera.scanline.p1", 0);
    if ( mDebugScanLineMask != 0)
    {
        mpDebugScanLine = DebugScanLine::createInstance();
    }
    //
    mIvMs = property_get_int32(
        "vendor.debug.camera.log.p1independentverification", 0);
    //
#if (SUPPORT_BUFFER_TUNING_DUMP)
    //MY_LOGI("SUPPORT_BUFFER_TUNING_DUMP CamDumpEn(%d)", mCamDumpEn);
#else
    if (mCamDumpEn > 0) {
        MY_LOGI("NOT-SUPPORT_BUFFER_TUNING_DUMP CamDumpEn(%d)", mCamDumpEn);
    }
    mCamDumpEn = 0;
#endif
    //
    MINT32 stuffBufCtrl = ::property_get_int32(
        "vendor.debug.camera.p1stuffbufctrl", 0);
    if (stuffBufCtrl > 0) {
        mStuffBufNumBasis = stuffBufCtrl % 100; // region : 1 ~ 99
        if (stuffBufCtrl >= 100) {
            mStuffBufNumMax = stuffBufCtrl / 100;
        } else { // max amount not set
            mStuffBufNumMax = mStuffBufNumBasis * 2; // default : double
        }
        if (mStuffBufNumMax < mStuffBufNumBasis) {
            mStuffBufNumMax = mStuffBufNumBasis;
        }
    }
    //
    MINT32 disableDeliverReleasing = ::property_get_int32(
        "vendor.debug.camera.p1disabledeliverreleasing", 0);
    mDisableDeliverReleasing = disableDeliverReleasing; // force to disable DeliverReleasing
    //
#if (P1NODE_BUILD_LOG_LEVEL_DEFAULT > 3)
    mTimingFactor = 32;  // for ENG build
#elif (P1NODE_BUILD_LOG_LEVEL_DEFAULT > 2)
    mTimingFactor = 2;  // for USERDEBUG build
#else
    mTimingFactor = 1;  // for USER build
    mIvMs = 0;
#endif
    //
    MY_LOGI("LOGD[%d](%d) LOGI[%d](%d) prop(cam:%d pl:%d pi:%d g:%d:%d) - "
        " SYS[%d-%d:%d](%d) - "
        "MetaLog(p:%d/%d m:%d/x%X) DumpRaw(%d) DataDump(%d) DrawLine(%d) - "
        "StuffBufCtrl(%d)(%d:%d) - DelRelOff(%d) - TF(%d) - IV(%d)",
        buildLogD, mLogLevel, buildLogI, mLogLevelI,
        cam_log, p1_log, p1_logi, g_log, g_log_lv,
        g_sys_set, g_sys, p1sys, mSysLevel,
        pMetaLogOp, pMetaLogTag, mMetaLogOp, mMetaLogTag,
        mEnableDumpRaw, mCamDumpEn, mDebugScanLineMask,
        stuffBufCtrl, mStuffBufNumBasis, mStuffBufNumMax,
        mDisableDeliverReleasing, mTimingFactor, mIvMs);
}


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::
~P1NodeImp()
{
    MY_LOGI0("");
    if( mpDebugScanLine != NULL )
    {
        mpDebugScanLine->destroyInstance();
        mpDebugScanLine = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
init(InitParams const& rParams)
{
    FUNCTION_S_IN;

    P1_TRACE_AUTO(SLG_B, "P1:init");

    setLaunchState(NSP1Node::LAUNCH_STATE_NULL);

    Mutex::Autolock _l(mPublicLock);

    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mOpenId  = rParams.openId;
        mNodeId  = rParams.nodeId;
        mNodeName= rParams.nodeName;
    }
    //
    if (mIvMs > 0) {
        mpIndependentVerification = new IndependentVerification(
            getOpenId(), mLogLevel, mLogLevelI, (MUINT32)mIvMs, this);
        if (mpIndependentVerification == NULL) {
            MY_LOGE("IndependentVerification create fail");
            return NO_MEMORY;
        }
    }
    //  Select CamIO version
    {
        auto pModule = getNormalPipeModule();

        if  ( ! pModule ) {
            MY_LOGE("getNormalPipeModule() fail");
            return UNKNOWN_ERROR;
        }

        MUINT32 const* version = NULL;
        size_t count = 0;
        int err = pModule->get_sub_module_api_version(&version, &count, mOpenId);
        if  ( err < 0 || ! count || ! version ) {
            MY_LOGE(
                "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
                mOpenId, err, count, version
            );
            return UNKNOWN_ERROR;
        }

        mCamIOVersion = *(version + count - 1); //Select max. version
        MY_LOGD0("[%d] count:%zu Selected CamIO Version:%0#x", mOpenId, count, mCamIOVersion);
    }

    #if (USING_DRV_IO_PIPE_EVENT)
    {
        Mutex::Autolock _l(mIoPipeEvtOpLock);
        mIoPipeEvtOpLeaving = MFALSE;
        NSCam::NSIoPipe::IoPipeEventSystem & evtSystem =
            NSCam::NSIoPipe::IoPipeEventSystem::getGlobal();
        if (mspIoPipeEvtHandleAcquire != NULL) {
            mspIoPipeEvtHandleAcquire->unsubscribe();
            mspIoPipeEvtHandleAcquire = NULL;
        }
        mspIoPipeEvtHandleAcquire = evtSystem.subscribe(
            NSCam::NSIoPipe::EVT_IPRAW_P1_ACQUIRING, onEvtCtrlAcquiring, this);
        if (mspIoPipeEvtHandleAcquire == NULL) {
            MY_LOGE("IoPipeEventSystem subscribe EVT_IPRAW_P1_ACQUIRING fail");
            return UNKNOWN_ERROR;
        }
        if (mspIoPipeEvtHandleRelease != NULL) {
            mspIoPipeEvtHandleRelease->unsubscribe();
            mspIoPipeEvtHandleRelease = NULL;
        }
        mspIoPipeEvtHandleRelease = evtSystem.subscribe(
            NSCam::NSIoPipe::EVT_IPRAW_P1_RELEASED, onEvtCtrlReleasing, this);
        if (mspIoPipeEvtHandleRelease == NULL) {
            MY_LOGE("IoPipeEventSystem subscribe EVT_IPRAW_P1_RELEASED fail");
            return UNKNOWN_ERROR;
        }
    }
    #endif

    mStuffBufMgr.setLog(getOpenId(), mLogLevel, mLogLevelI);

    mLongExp.config(getOpenId(), mLogLevel, mLogLevelI);

    mpConCtrl = new ConcurrenceControl(getOpenId(), mLogLevel, mLogLevelI,
        mSysLevel);
    if (mpConCtrl == NULL || mpConCtrl->getStageCtrl() == NULL) {
        MY_LOGE("ConcurrenceControl create fail");
        return NO_MEMORY;
    }

    mpHwStateCtrl = new HardwareStateControl();
    if (mpHwStateCtrl == NULL) {
        MY_LOGE("HardwareStateControl create fail");
        return NO_MEMORY;
    }

    mpConnectLMV = new P1ConnectLMV(getOpenId(), mLogLevel, mLogLevelI,
        mSysLevel);
    if (mpConnectLMV == NULL) {
        MY_LOGE("ConnectLMV create fail Log(%d)(%d) Id(%d)",
            mLogLevel, mLogLevelI, getOpenId());
        return NO_MEMORY;
    }

    mpTimingCheckerMgr = new TimingCheckerMgr(mTimingFactor, getOpenId(), mLogLevel, mLogLevelI);
    if (mpTimingCheckerMgr == NULL) {
        MY_LOGE("TimingCheckerMgr create fail");
        return NO_MEMORY;
    }

    MERROR err = run("P1NodeImp::init");

    mpDeliverMgr = new P1DeliverMgr();
    if (mpDeliverMgr != NULL) {
        mpDeliverMgr->init(this);
    } else {
        MY_LOGE("DeliverMgr create fail");
        return NO_MEMORY;
    }

    mpRegisterNotify = new P1RegisterNotify(this);
    if (mpRegisterNotify != NULL) {
        mpRegisterNotify->init();
    } else {
        MY_LOGE("RegisterNotify create fail");
        return NO_MEMORY;
    }

    mpTaskCtrl = new P1TaskCtrl(this);
    if (mpTaskCtrl == NULL) {
        MY_LOGE("TaskCtrl create fail");
        return NO_MEMORY;
    }

    mpTaskCollector = new P1TaskCollector(mpTaskCtrl);
    if (mpTaskCollector == NULL) {
        MY_LOGE("TaskCollector create fail");
        return NO_MEMORY;
    }

    FUNCTION_S_OUT;

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
uninit()
{
    PUBLIC_APIS_IN;

    P1_TRACE_AUTO(SLG_B, "P1:uninit");

    LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_API_UNINIT_BGN,
        LogInfo::CP_API_UNINIT_END);

    #if (USING_DRV_IO_PIPE_EVENT)
    {
        Mutex::Autolock _l(mIoPipeEvtOpLock);
        mIoPipeEvtOpLeaving = MTRUE;
        if (mspIoPipeEvtHandleAcquire != NULL) {
            mspIoPipeEvtHandleAcquire->unsubscribe();
            mspIoPipeEvtHandleAcquire = NULL;
        }
        if (mspIoPipeEvtHandleRelease != NULL) {
            mspIoPipeEvtHandleRelease->unsubscribe();
            mspIoPipeEvtHandleRelease = NULL;
        }
    }
    #endif

    Mutex::Autolock _l(mPublicLock);

    // flush the left frames if exist
    onHandleFlush(MFALSE);
    setLaunchState(NSP1Node::LAUNCH_STATE_NULL);

    requestExit();

    //mvStreamMeta.clear();
    for (int stream = STREAM_ITEM_START; stream < STREAM_META_NUM; stream++) {
        mvStreamMeta[stream] = NULL;
    }

    //mvStreamImg.clear();
    for (int stream = STREAM_ITEM_START; stream < STREAM_IMG_NUM; stream++) {
        mvStreamImg[stream] = NULL;
    }

    if (mspSyncHelper != NULL) {
        mspSyncHelper = NULL;
    }

    if (mspResConCtrl != NULL) {
        P1NODE_RES_CON_RETURN(mspResConCtrl, mResConClient);
        mspResConCtrl = NULL;
    }

    if (mpDeliverMgr != NULL) {
        mpDeliverMgr->uninit();
        mpDeliverMgr = NULL;
    }

    if (mpRegisterNotify != NULL) {
        mpRegisterNotify->uninit();
        mpRegisterNotify = NULL;
    }

    if (mpTaskCollector != NULL) {
        mpTaskCollector = NULL;
    }

    if (mpTaskCtrl != NULL) {
        mpTaskCtrl = NULL;
    }

    if (mpTimingCheckerMgr != NULL) {
        mpTimingCheckerMgr = NULL;
    }

    if (mpHwStateCtrl != NULL) {
        mpHwStateCtrl = NULL;
    }

    if (mpConCtrl != NULL) {
        mpConCtrl = NULL;
    }

    if (mpIndependentVerification != NULL) {
        delete mpIndependentVerification;
        mpIndependentVerification = NULL;
    }

    PUBLIC_APIS_OUT;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
check_config(ConfigParams const& rParams)
{
    P1_TRACE_AUTO(SLG_S, "P1:check_config");

    if (rParams.pInAppMeta == NULL) {
        MY_LOGE("in app metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pInHalMeta == NULL) {
        MY_LOGE("in hal metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutAppMeta == NULL) {
        MY_LOGE("out app metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutHalMeta == NULL) {
        MY_LOGE("out hal metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pvOutImage_full.size() == 0 &&
        rParams.pOutImage_resizer == NULL) {
        MY_LOGE("image is empty");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_full != NULL &&
        rParams.pvOutImage_full.size() == 0) {
        MY_LOGE("wrong full input");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_resizer != NULL &&
        rParams.pOutImage_resizer == NULL) {
        MY_LOGE("wrong resizer input");
        return BAD_VALUE;
    }
    #if SUPPORT_LCS
    if (rParams.pStreamPool_lcso != NULL &&
        rParams.pOutImage_lcso == NULL) {
        MY_LOGE("wrong lcso input");
        return BAD_VALUE;
    }
    if (rParams.enableLCS == MTRUE &&
        rParams.pOutImage_lcso == NULL) {
        MY_LOGE("LCS enable but no lcso input");
        return BAD_VALUE;
    }
    #endif
    #if SUPPORT_RSS
    if (rParams.pStreamPool_rsso != NULL &&
        rParams.pOutImage_rsso == NULL) {
        MY_LOGE("wrong rsso input");
        return BAD_VALUE;
    }
    if (rParams.enableRSS == MTRUE &&
        rParams.pOutImage_rsso == NULL) {
        MY_LOGE("RSS enable but no rsso input");
        return BAD_VALUE;
    }
    #endif
    //
    if (mpDeliverMgr != NULL && mpDeliverMgr->runningGet()/*isRunning()*/) {
        MY_LOGI0("DeliverMgr thread is running");
        if (mpTimingCheckerMgr != NULL) {
            U_if (mpTimingCheckerMgr->getEnable()) {
                MY_LOGI0("TimingCheckerMgr-cc-stopping");
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
        }
        U_if (!mpDeliverMgr->waitFlush(MTRUE)) {
            MY_LOGW("request not done before exit");
        };
        mpDeliverMgr->requestExit();
        mpDeliverMgr->trigger();
        mpDeliverMgr->join();
        mpDeliverMgr->runningSet(MFALSE);
    }

    // Get sensor format
    IHalSensorList *const pIHalSensorList = MAKE_HalSensorList();
    if (pIHalSensorList) {
        MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(getOpenId());

        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        mSensorFormatOrder = sensorStaticInfo.sensorFormatOrder;
    }

    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        for (int meta = STREAM_ITEM_START; meta < STREAM_META_NUM; meta++) {
            mvStreamMeta[meta] = NULL;
        }
        if (rParams.pInAppMeta != NULL) {
            mvStreamMeta[STREAM_META_IN_APP] = rParams.pInAppMeta;
            //mInAppMeta = rParams.pInAppMeta;
        };
        if (rParams.pInHalMeta != NULL) {
            mvStreamMeta[STREAM_META_IN_HAL] = rParams.pInHalMeta;
            //mInHalMeta = rParams.pInHalMeta;
        };
        if (rParams.pOutAppMeta != NULL) {
            mvStreamMeta[STREAM_META_OUT_APP] = rParams.pOutAppMeta;
            //mOutAppMeta = rParams.pOutAppMeta;
        };
        if (rParams.pOutHalMeta != NULL) {
            mvStreamMeta[STREAM_META_OUT_HAL] = rParams.pOutHalMeta;
            //mOutHalMeta = rParams.pOutHalMeta;
        };
        //
        for (int img = STREAM_ITEM_START; img < STREAM_IMG_NUM; img++) {
            mvStreamImg[img] = NULL;
        }
        //
        if (rParams.pInImage_yuv != NULL) {
            mvStreamImg[STREAM_IMG_IN_YUV] = rParams.pInImage_yuv;
        };
        if (rParams.pInImage_opaque != NULL) {
            mvStreamImg[STREAM_IMG_IN_OPAQUE] = rParams.pInImage_opaque;
        };
        if (rParams.pOutImage_opaque != NULL) {
            mvStreamImg[STREAM_IMG_OUT_OPAQUE] = rParams.pOutImage_opaque;
        };
        #if 1
        for (size_t i  = 0; i < rParams.pvOutImage_full.size(); i++) {
            if (rParams.pvOutImage_full[i] != NULL) { // pick the first item
                mvStreamImg[STREAM_IMG_OUT_FULL] = rParams.pvOutImage_full[i];
                break;
            }
        }
        #else
        if (rParams.pvOutImage_full.size() > 0) {
            mvStreamImg[STREAM_IMG_OUT_FULL] = rParams.pvOutImage_full[0];
            //mvStreamImg[STREAM_IMG_OUT_FULL] = rParams.pOutImage_full;
        };
        #endif
        if (rParams.pOutImage_resizer != NULL) {
            mvStreamImg[STREAM_IMG_OUT_RESIZE] = rParams.pOutImage_resizer;
        };
        #if SUPPORT_LCS
        if (rParams.pOutImage_lcso != NULL) {
            mvStreamImg[STREAM_IMG_OUT_LCS] = rParams.pOutImage_lcso;
        };
        #endif
        #if SUPPORT_RSS
        if (rParams.pOutImage_rsso != NULL) {
            mvStreamImg[STREAM_IMG_OUT_RSS] = rParams.pOutImage_rsso;
        };
        #endif
        //
        mpStreamPool_full = (rParams.pStreamPool_full != NULL) ?
            rParams.pStreamPool_full : NULL;
        mpStreamPool_resizer = (rParams.pStreamPool_resizer != NULL) ?
            rParams.pStreamPool_resizer : NULL;
        #if SUPPORT_LCS
        mpStreamPool_lcso = (rParams.pStreamPool_lcso != NULL) ?
            rParams.pStreamPool_lcso : NULL;
        #endif
        #if SUPPORT_RSS
        mpStreamPool_rsso = (rParams.pStreamPool_rsso != NULL) ?
            rParams.pStreamPool_rsso : NULL;
        #endif
        //
        #if 0
        #warning "[FIXME] force to change p1 not use pool"
        {
            MUINT8 no_pool =
                    ::property_get_int32("vendor.debug.camera.p1nopool", 0);
            if (no_pool > 0) {
                mpStreamPool_full = NULL;
                mpStreamPool_resizer = NULL;
                mpStreamPool_lcso = NULL;
                mpStreamPool_rsso = NULL;
            }
            MY_LOGI0("debug.camera.p1nopool = %d", no_pool);
        }
        #endif
        //
        {
            if (mspSyncHelper != NULL) {
                mspSyncHelper = NULL;
            }
            if (rParams.pSyncHelper != NULL) {
                mspSyncHelper = rParams.pSyncHelper;
            }
        }
        //
        {
            if (mspResConCtrl != NULL) {
                P1NODE_RES_CON_RETURN(mspResConCtrl, mResConClient);
                mspResConCtrl = NULL;
            }
            mspResConCtrl = rParams.pResourceConcurrency;
            if (mspResConCtrl != NULL) {
                mResConClient = IResourceConcurrency::CLIENT_HANDLER_NULL;
                mIsResConGot = MFALSE;
            }
        }
        //
        mBurstNum = MAX(rParams.burstNum, 1);
        #if (ENABLE_CHECK_CONFIG_COMMON_PORPERTY || (0)) // for SMVR IT
        #warning "[FIXME] force to change p1 burst number"
        {
            MUINT8 burst_num =
                    ::property_get_int32("vendor.debug.camera.p1burst", 0);
            if (burst_num > 0) {
                mBurstNum = burst_num;
            }
            MY_LOGI0("debug.camera.p1burst = %d  -  BurstNum = %d",
                burst_num, mBurstNum);
        }
        #endif
        //
        mReceiveMode = rParams.receiveMode;
        #if (ENABLE_CHECK_CONFIG_COMMON_PORPERTY || (0)) // receive mode IT
        #warning "[FIXME] force to change p1 receive mode"
        {
            MUINT8 rev_mode =
                    ::property_get_int32("vendor.debug.camera.p1rev", 0);
            if (rev_mode > 0) {
                mReceiveMode = (REV_MODE)rev_mode;
            }
            MY_LOGI0("debug.camera.p1rev = %d  - RevMode=%d BurstNum=%d",
                rev_mode, mReceiveMode, mBurstNum);
        }
        #endif
        //
        #if (ENABLE_CHECK_CONFIG_COMMON_PORPERTY || (1)) // standby mode
        //#warning "force to change standby mode"
        {
            MINT8 standby_mode =
                    ::property_get_int32("vendor.debug.camera.p1standbymode", 0);
            if (standby_mode > 0) {
                mForceStandbyMode = standby_mode;
                MY_LOGI0("debug.camera.standbymode = %d - ForceStandbyMode = %d",
                    standby_mode, mForceStandbyMode);
            }

        }
        #endif
        //
        android::String8 meta_str("");
        mCfgAppMeta.clear();
        if (rParams.cfgAppMeta.count() > 0) {
            mCfgAppMeta = rParams.cfgAppMeta;
            if (LOGI_LV1) {
                meta_str.appendFormat(" -- ConfigParams.cfgAppMeta[%d] ",
                    rParams.cfgAppMeta.count());
                for (MUINT32 i = 0; i < rParams.cfgAppMeta.count(); i++) {
                    generateMetaInfoStr(rParams.cfgAppMeta.entryAt(i),
                        meta_str);
                }
            }
        }
        mCfgHalMeta.clear();
        if (rParams.cfgHalMeta.count() > 0) {
            mCfgHalMeta = rParams.cfgHalMeta;
            if (LOGI_LV1) {
                meta_str.appendFormat(" -- ConfigParams.cfgHalMeta[%d] ",
                    rParams.cfgHalMeta.count());
                for (MUINT32 i = 0; i < rParams.cfgHalMeta.count(); i++) {
                    generateMetaInfoStr(rParams.cfgHalMeta.entryAt(i),
                        meta_str);
                }
            }
        }
        if (!meta_str.isEmpty()) {
            MY_LOGI1("%s", meta_str.string());
        }
        //
        mSensorParams = rParams.sensorParams;
        //
        mEnableDualPD = rParams.enableDualPD;
        //
        mEnableQuadCode = rParams.enableQuadCode;
        //
        mRawProcessed = rParams.rawProcessed;
        mRawSetDefType = rParams.rawDefType;
        //
        mEnableSecure = rParams.enableSecurity;
        mSecType = rParams.secType;
        mSecStatus = rParams.statusSecHandle;
        //
        mTgNum = rParams.tgNum;
        //
        mPipeMode = rParams.pipeMode;
        //
        mPipeBit = rParams.pipeBit;
        //
        mResizeQuality = rParams.resizeQuality;
        //
        mDisableHLR = rParams.disableHLR;
        //
        mDisableFrontalBinning = rParams.disableFrontalBinning;
        //
        mDisableDynamicTwin = rParams.disableDynamicTwin;
        //
        mEnableUniForcedOn = rParams.enableUNI;
        //
        if (IS_LMV(mpConnectLMV)) {
            mEnableEISO = rParams.enableEIS;
            mForceSetEIS = rParams.forceSetEIS;
            mPackedEisInfo = rParams.packedEisInfo;
        }
        #if SUPPORT_LCS
        mEnableLCSO = rParams.enableLCS;
        #endif
        #if SUPPORT_RSS
        mEnableRSSO = rParams.enableRSS;
        #endif
        mEnableFSC = rParams.enableFSC;
        //
        mEnableCaptureFlow = rParams.enableCaptureFlow;
        mEnableCaptureOff = MFALSE;
        mEnableFrameSync = rParams.enableFrameSync;
        mStereoCamMode = rParams.stereoCamMode;
        if (EN_START_CAP_CFG) { // disable - acquire init buffer from pool
            mpStreamPool_full = NULL;
            mpStreamPool_resizer = NULL;
            mpStreamPool_lcso = NULL;
            mpStreamPool_rsso = NULL;
        }
        //
        {
            mInitReqSet = rParams.initRequest;
            #if (ENABLE_CHECK_CONFIG_COMMON_PORPERTY || (1)) // init request set IT
            //#warning "[FIXME] force to set init request"
            {
                MINT32 init_req =
                    ::property_get_int32("vendor.debug.camera.p1initreqnum", 0);
                if (init_req != 0) {
                    if (init_req > P1NODE_DEF_SHUTTER_DELAY) {
                        mInitReqSet = init_req;
                    } else if (init_req < 0) { // forced-disable
                        mInitReqSet = 0;
                    }
                    MY_LOGI0("debug.camera.p1initreqnum = %d (>%d) - InitReq=%d"
                        " BurstNum=%d", init_req, P1NODE_DEF_SHUTTER_DELAY,
                        mInitReqSet, mBurstNum);
                }
            }
            #endif
            if (EN_INIT_REQ_CFG && mInitReqSet <= P1NODE_DEF_SHUTTER_DELAY) {
                MY_LOGE("INVALID init request value (%d)", mInitReqSet);
                return INVALID_OPERATION;
            }
            mInitReqNum = mInitReqSet * mBurstNum; // the InitReq setting will re-assign as re-configure
            mInitReqCnt = 0;
            mInitReqOff = MFALSE;
            if (EN_INIT_REQ_CFG) {
                MY_LOGI0("InitReq Set:%d Num:%d Cnt:%d Off:%d",
                    mInitReqSet, mInitReqNum, mInitReqCnt, mInitReqOff);
                U_if (mInitReqSet > mStuffBufNumMax) {
                    MY_LOGI0("InitReqSet(%d) > StuffBufNum(%d:%d) replace "
                        "StuffBufNumMax-value with InitReqSet - "
                        "StuffBufNum(%d:%d)", mInitReqSet, mStuffBufNumBasis,
                        mStuffBufNumMax, mStuffBufNumBasis, mInitReqSet);
                    mStuffBufNumMax = mInitReqSet;
                }
            }
        }
        //
        if (IS_BURST_ON) {
            mDepthNum = 2;
        } else if (isRevMode(REV_MODE_CONSERVATIVE)) {
            mDepthNum = 2;
        } else {
            mDepthNum = 1;
        }
        //
        #if 1 // check conflict configuration
        {
            if (((EN_BURST_MODE) &&
                (EN_INIT_REQ_CFG || EN_START_CAP_CFG || EN_REPROCESSING))
                || (EN_INIT_REQ_CFG && EN_START_CAP_CFG)
                ) {
                MY_LOGE("[Check_Config_Conflict] P1Node::ConfigParams:: "
                    "burstNum(%d) enableCaptureFlow(%d) initRequest(%d) "
                    "pInImage_opaque[%#" PRIx64 "] "
                    "pInImage_yuv[%#" PRIx64 "] ",
                    rParams.burstNum, rParams.enableCaptureFlow,
                    rParams.initRequest,
                    ((mvStreamImg[STREAM_IMG_IN_OPAQUE] == NULL) ?
                    (StreamId_T)(-1) :
                    mvStreamImg[STREAM_IMG_IN_OPAQUE]->getStreamId()),
                    ((mvStreamImg[STREAM_IMG_IN_YUV] == NULL) ?
                    (StreamId_T)(-1) :
                    mvStreamImg[STREAM_IMG_IN_YUV]->getStreamId()));
                return INVALID_OPERATION;
            }

            if (mEnableSecure && ((mSecType != SecType::mem_protected) &&
                (mSecType != SecType::mem_secure)))
            {
                MY_LOGE("[Check_Config_Conflict] P1Node::ConfigParams:: "
                    "mEnableSecure(%d) mSecType(0x%x)",
                    mEnableSecure, mSecType);
                return BAD_VALUE;
            }
        }
        #endif
    }
    //
    if (mvStreamImg[STREAM_IMG_OUT_OPAQUE] != NULL) {
        if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL) {
            mRawFormat = mvStreamImg[STREAM_IMG_OUT_FULL]->getImgFormat();
            IImageStreamInfo::BufPlanes_t const & planes =
                mvStreamImg[STREAM_IMG_OUT_FULL]->getBufPlanes();
            L_if (planes.size() > 0) {
                mRawStride = planes.itemAt(0).rowStrideInBytes;
                mRawLength = planes.itemAt(0).sizeInBytes;
            } else {
                MY_LOGE("STREAM_IMG_OUT_FULL getBufPlanes(%zu)", planes.size());
                return BAD_VALUE;
            }
        } else {
            mRawFormat = P1_IMGO_DEF_FMT;
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
            getNormalPipeModule()->query(
                    NSCam::NSIoPipe::PORT_IMGO.index,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                    (EImageFormat)mRawFormat,
                    mSensorParams.size.w,
                    queryRst
                    );
            mRawStride = queryRst.stride_byte;
            mRawLength = mRawStride * mSensorParams.size.h;
        }
    }
    //
    {
        sp<IMetadataProvider> pMetadataProvider =
            NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta =
            pMetadataProvider->getMtkStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta,
            MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray) ) {
            MY_LOGD_IF(mLogLevel > 1, "active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y,
                    mActiveArray.s.w, mActiveArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            #if (P1NODE_USING_MTK_LDVT > 0)
            mActiveArray = MRect(mSensorParams.size.w, mSensorParams.size.h);
            MY_LOGI0("set sensor size to active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y,
                    mActiveArray.s.w, mActiveArray.s.h);
            #else
            return UNKNOWN_ERROR;
            #endif
        }
        if( tryGetMetadata< MUINT8 >(&static_meta,
            MTK_SENSOR_INFO_TIMESTAMP_SOURCE, mTimestampSrc) ) {
            MY_LOGI1("TimestampSrc(%d)", mTimestampSrc);
        } else {
            MY_LOGI0("no static info: MTK_SENSOR_INFO_TIMESTAMP_SOURCE");
        }
    }
    //
    if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL) {
        if (mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize() !=
            mSensorParams.size) {
            MY_LOGE("[Check_Config_Conflict] IMGO_Stream.ImgSize(%dx%d) != "
                "SensorParam.Size(%d,%d) - P1Node::ConfigParams:: "
                "IMGO_StreamID:%#" PRIx64 "_ImgFormat[0x%x]-ImgSize(%dx%d) "
                "SensorParam_mode(%d)_fps(%d)_pixelMode(%d)_vhdrMode(%d)_"
                "size(%dx%d)", mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize().w,
                mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize().h,
                mSensorParams.size.w, mSensorParams.size.h,
                mvStreamImg[STREAM_IMG_OUT_FULL]->getStreamId(),
                mvStreamImg[STREAM_IMG_OUT_FULL]->getImgFormat(),
                mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize().w,
                mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize().h,
                mSensorParams.mode, mSensorParams.fps, mSensorParams.pixelMode,
                mSensorParams.vhdrMode,
                mSensorParams.size.w, mSensorParams.size.h);
            return INVALID_OPERATION;
        }
    }
    //
    {
        MERROR res = checkConstraint();
        if (res != OK) {
            return res;
        }
    }
    //
    mLogInfo.config(getOpenId(), mLogLevel, mLogLevelI, mBurstNum);
    mLogInfo.setActive(MTRUE);
    //
    L_if (mpTimingCheckerMgr != NULL) {
        mpTimingCheckerMgr->setEnable(MTRUE);
    }
    //
    {
        MBOOL deliver_mgr_send = MTRUE;
        #if 0
        #warning "[FIXME] force to change p1 dispatch frame directly"
        {
            MUINT8 dispatch =
                ::property_get_int32("vendor.debug.camera.p1dispatch", 0);
            MY_LOGI0("debug.camera.p1dispatch = %d", dispatch);
            if (dispatch > 0) {
                deliver_mgr_send = MFALSE;
            };
        }
        #endif
        /*
        if (EN_REPROCESSING) {
            deliver_mgr_send = MTRUE; // for reprocessing flow
        };
        */
        MY_LOGD2("USE DeliverMgr Thread Loop : %d", deliver_mgr_send);
        if (deliver_mgr_send) {
            if (mpDeliverMgr != NULL) {
                mpDeliverMgr->config();
                if (NO_ERROR == mpDeliverMgr->run("P1NodeImp::config")) {
                    MY_LOGD2("RUN DeliverMgr Thread OK");
                    mpDeliverMgr->runningSet(MTRUE);
                } else {
                    MY_LOGE("RUN DeliverMgr Thread FAIL");
                    return BAD_VALUE;
                }
            }
        }
    }
    //
    if (mpTaskCtrl != NULL) {
        mpTaskCtrl->config();
    }
    if (mpTaskCollector != NULL) {
        mpTaskCollector->config();
    }
    if (mpTaskCtrl != NULL) {
        mpTaskCtrl->reset(); // reset the act from Collector.settle
    }
    mLastNum = P1_MAGIC_NUM_FIRST;
    if (mpRegisterNotify != NULL) {
        mpRegisterNotify->config();
    }
    //
    {
        MUINT32 queReserve = mBurstNum * P1NODE_DEF_QUEUE_DEPTH;
        {
            Mutex::Autolock _ll(mDropQueueLock);
            mDropQueue.clear();
            mDropQueue.reserve(queReserve);
        }
        {
            Mutex::Autolock _ll(mRequestQueueLock);
            mRequestQueue.clear();
            mRequestQueue.reserve(queReserve);
        }
        {
            Mutex::Autolock _ll(mProcessingQueueLock);
            mProcessingQueue.clear();
            mProcessingQueue.reserve(queReserve);
        }
    }
    //
    //MY_LOGI0("CAM_LOG_LEVEL %d", CAM_LOG_LEVEL);
    #if (IS_P1_LOGI)
    {
        android::String8 strInfo("");
        strInfo += String8::format("Cam::%d ", getOpenId());
        //
        strInfo += String8::format("Param["
            "N:m%d,p%d,q%d,t%d,b%d,i%d,r%d,w%d,v%" PRId64"_"
            "B:p%d,b%d,t%d,h%d,u%d,e%d,l%d,r%d,f%d,d%d,q%d,c%d,f%d,v%d,s%d,SE%d,ST%d,SB%x] ",
            // Param-iNt/eNum
            rParams.pipeMode/*m*/, rParams.pipeBit/*p*/,
            rParams.resizeQuality/*q*/,
            rParams.tgNum/*t*/, rParams.burstNum/*b*/,
            rParams.initRequest/*i*/, rParams.receiveMode/*r*/,
            rParams.rawDefType/*w*/,
            rParams.packedEisInfo/*v*//*EisInfo::getMode(mPackedEisInfo)*/,
            // Param-Bool
            rParams.rawProcessed/*p*/, rParams.disableFrontalBinning/*b*/,
            rParams.disableDynamicTwin/*t*/, rParams.disableHLR/*h*/,
            rParams.enableUNI/*u*/, rParams.enableEIS/*e*/,
            rParams.enableLCS/*l*/, rParams.enableRSS/*r*/, rParams.enableFSC/*f*/,
            rParams.enableDualPD/*d*/, rParams.enableQuadCode/*q*/,
            rParams.enableCaptureFlow/*c*/, rParams.enableFrameSync/*f*/,
            rParams.forceSetEIS/*v*/, rParams.stereoCamMode/*s*/,
            rParams.enableSecurity/*SE*/, (int)rParams.secType/*ST*/,
            rParams.statusSecHandle/*SB*/
            );
        //
        strInfo += String8::format("S(%d,%d,%d,%d,x%x,%dx%d) ",
            mSensorParams.mode, mSensorParams.fps, mSensorParams.pixelMode,
            mSensorParams.vhdrMode, mSensorFormatOrder,
            mSensorParams.size.w, mSensorParams.size.h);
        strInfo += String8::format("R(0x%x-%d-%d,%d-%d-%d,%d-0x%x) ",
            mRawFormat, mRawStride, mRawLength, mRawPostProcSupport,
            mRawProcessed, mRawSetDefType, mRawDefType, mRawOption);
        strInfo += String8::format("D(b%d,t%d,h%d) ",
            mDisableFrontalBinning/*b*/, mDisableDynamicTwin/*t*/,
            mDisableHLR/*h*/);
        strInfo += String8::format("E(e%d,l%d,r%d,u%d,d%d,q%d,c%d,f%d,s%d) ",
            mEnableEISO/*e*/, mEnableLCSO/*l*/, mEnableRSSO/*r*/,
            mEnableUniForcedOn/*u*/, mEnableDualPD/*d*/, mEnableQuadCode/*q*/,
            mEnableCaptureFlow/*c*/, mEnableFrameSync/*f*/,
            mStereoCamMode/*s*/);
        strInfo += String8::format("M(m0x%x,p0x%x,q%d,t%d,b%d,d%d,r%d,i%d,"
            "v%" PRId64 ") ",
            mPipeMode/*m*/, mPipeBit/*p*/, mResizeQuality/*q*/, mTgNum/*t*/,
            mBurstNum/*b*/, mDepthNum/*d*/, mReceiveMode/*r*/, mInitReqSet/*i*/,
            mPackedEisInfo/*v*//*EisInfo::getMode(mPackedEisInfo)*/);
        strInfo += String8::format("Dm(%d) ", mpDeliverMgr->runningGet());
        strInfo += String8::format("Rc(%p) ", mspResConCtrl.get());
        strInfo += String8::format("Sh(%p) ", mspSyncHelper.get());
        strInfo += String8::format("Pool(IMG%p,RRZ%p,LCS%p,RSS%p) ",
            (mpStreamPool_full != NULL) ?
            (mpStreamPool_full.get()) : (NULL),
            (mpStreamPool_resizer != NULL) ?
            (mpStreamPool_resizer.get()) : (NULL),
            (mpStreamPool_lcso != NULL) ?
            (mpStreamPool_lcso.get()) : (NULL),
            (mpStreamPool_rsso != NULL) ?
            (mpStreamPool_rsso.get()) : (NULL));
        strInfo += String8::format(
            "Meta%s_%d:%#" PRIx64 " Meta%s_%d:%#" PRIx64 " "
            "Meta%s_%d:%#" PRIx64 " Meta%s_%d:%#" PRIx64 " ",
            maStreamMetaName[STREAM_META_IN_APP], STREAM_META_IN_APP,
            (mvStreamMeta[STREAM_META_IN_APP] == NULL) ? (StreamId_T)(-1) :
            mvStreamMeta[STREAM_META_IN_APP]->getStreamId(),
            maStreamMetaName[STREAM_META_IN_HAL], STREAM_META_IN_HAL,
            (mvStreamMeta[STREAM_META_IN_HAL] == NULL) ? (StreamId_T)(-1) :
            mvStreamMeta[STREAM_META_IN_HAL]->getStreamId(),
            maStreamMetaName[STREAM_META_OUT_APP], STREAM_META_OUT_APP,
            (mvStreamMeta[STREAM_META_OUT_APP] == NULL) ? (StreamId_T)(-1) :
            mvStreamMeta[STREAM_META_OUT_APP]->getStreamId(),
            maStreamMetaName[STREAM_META_OUT_HAL], STREAM_META_OUT_HAL,
            (mvStreamMeta[STREAM_META_OUT_HAL] == NULL) ? (StreamId_T)(-1) :
            mvStreamMeta[STREAM_META_OUT_HAL]->getStreamId());
        //
        for (int i = STREAM_ITEM_START; i < STREAM_IMG_NUM; i++) {
            if (mvStreamImg[i] != NULL) {
                strInfo += String8::format(
                    "Img%s_%d:%#" PRIx64 "(%dx%d)[0x%x] ",
                    (maStreamImgName[i]),
                    i, mvStreamImg[i]->getStreamId(),
                    mvStreamImg[i]->getImgSize().w,
                    mvStreamImg[i]->getImgSize().h,
                    mvStreamImg[i]->getImgFormat());
            };
        }
        //
        strInfo += String8::format("Meta(APP:%d=%d,HAL:%d=%d) ",
            rParams.cfgAppMeta.count(), mCfgAppMeta.count(),
            rParams.cfgHalMeta.count(), mCfgHalMeta.count());
        //
        if (mvStreamImg[STREAM_IMG_OUT_RESIZE] != NULL) {
            strInfo += String8::format("RR(%d) ", getResizeMaxRatio(
                mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat()));
        };
        //
        strInfo += String8::format("AA(%d,%d-%dx%d) ", mActiveArray.p.x,
            mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
        //
        strInfo += String8::format("TS(%d) ", mTimestampSrc);
        //
        MY_LOGI0("%s", strInfo.string());
    }
    #endif

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
checkConstraint()
{
    auto pModule = getNormalPipeModule();
    if (!pModule) {
        MY_LOGE("getNormalPipeModule() fail");
        return UNKNOWN_ERROR;
    }
    // check raw type
    {
        mRawPostProcSupport = isPostProcRawSupported();
    }
    if (mPipeMode == PIPE_MODE_NORMAL_SV) { // only support pure raw
        mRawDefType = EPipe_PURE_RAW;
        mRawOption = (1 << EPipe_PURE_RAW);
        if (mRawSetDefType == RAW_DEF_TYPE_PROCESSED_RAW) {
            MY_LOGE("INVALID Raw-Default-Type option, "
                "P1Node::ConfigParams::PipeMode(%d) is PIPE_MODE_NORMAL_SV - "
                "it will reject the P1Node::ConfigParams::rawDefType(%d) "
                "A.K.A. RAW_DEF_TYPE_PROCESSED_RAW",
                mPipeMode, mRawSetDefType);
                return INVALID_OPERATION;
        }
        if (mRawProcessed == MTRUE) {
            MY_LOGE("INVALID Raw-Processed option, "
                "P1Node::ConfigParams::PipeMode(%d) is PIPE_MODE_NORMAL_SV - "
                "it will reject the P1Node::ConfigParams::rawProcessed(%d) ",
                mPipeMode, mRawProcessed);
                return INVALID_OPERATION;
        }
    } else if (mRawPostProcSupport) {
        mRawDefType = EPipe_PURE_RAW;
        mRawOption = (1 << EPipe_PURE_RAW);
        if (mRawProcessed == MTRUE) {
            // DualPD, raw type will be selected by driver
            mRawDefType = EPipe_PROCESSED_RAW;
            mRawOption |= (1 << EPipe_PROCESSED_RAW);
        }
        if (mRawSetDefType == RAW_DEF_TYPE_AUTO) {
            // by previous decision
        } else if (mRawSetDefType == RAW_DEF_TYPE_PURE_RAW) {
            mRawDefType = EPipe_PURE_RAW; // accepted
        } else if (mRawSetDefType == RAW_DEF_TYPE_PROCESSED_RAW) {
            if (mRawProcessed == MTRUE) {
                mRawDefType = EPipe_PROCESSED_RAW; // accepted
            } else {
                MY_LOGE("INVALID Raw-Default-Type option, "
                    "P1Node::ConfigParams::rawProcessed(%d) not enabled - "
                    "it will reject the P1Node::ConfigParams::rawDefType(%d) "
                    "A.K.A. RAW_DEF_TYPE_PROCESSED_RAW", mRawProcessed,
                    mRawSetDefType);
                return INVALID_OPERATION;
            }
        } else {
            MY_LOGE("INVALID Raw-Default-Type option, "
                "P1Node::ConfigParams::rawProcessed(%d) - "
                "P1Node::ConfigParams::rawDefType(%d) "
                "UNKNOWN type", mRawProcessed, mRawSetDefType);
            return INVALID_OPERATION;
        }
    } else { // i.e. the platform without HW PostProc raw support
        // ignore mRawProcessed value
        mRawOption = (1 << EPipe_PURE_RAW) | (1 << EPipe_PROCESSED_RAW);
        if (mRawSetDefType == RAW_DEF_TYPE_AUTO ||
            mRawSetDefType == RAW_DEF_TYPE_PROCESSED_RAW) {
            mRawDefType = EPipe_PROCESSED_RAW;  // accepted
        } else if (mRawSetDefType == RAW_DEF_TYPE_PURE_RAW) {
            mRawDefType = EPipe_PURE_RAW;
            MY_LOGW("WARNING Raw-Default-Type option, "
                "use default-pure-raw without post-proc-raw-support - "
                "P1Node::ConfigParams::rawDefType(%d)", mRawSetDefType);
            //return INVALID_OPERATION;
        } else {
            MY_LOGE("INVALID Raw-Default-Type option, "
                "P1Node::ConfigParams::rawDefType(%d) "
                "UNKNOWN type", mRawSetDefType);
            return INVALID_OPERATION;
        }
    }
    //
    // check Burst Mode
    if (mBurstNum > 1) {
        MBOOL ret = MFALSE;
        sCAM_QUERY_BURST_NUM res;
        res.QueryOutput = 0x0;
        ret = pModule->query(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BURST_NUM,
            (MUINTPTR)(&res));
        if (!ret) {
            MY_LOGE("[Cam::%d] Cannot query ENPipeQueryCmd_BURST_NUM",
                getOpenId());
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return BAD_VALUE;
            #endif
        } else if ((res.QueryOutput & mBurstNum) == 0x0) {
            MY_LOGE("[Cam::%d] ENPipeQueryCmd_BURST_NUM - support (0x%X) ,"
                " but BurstNum set as (0x%X)",
                getOpenId(), res.QueryOutput, mBurstNum);
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return INVALID_OPERATION;
            #endif
        }
    }
    //
    // check Raw Pattern
    mCfg.mPattern = eCAM_NORMAL;
    {
        if (mEnableDualPD && mEnableQuadCode) {
            // not support - DualPD and QuadCode at the same time
            MY_LOGE("DualPD(%d) QuadCode(%d) - Not Support",
                mEnableDualPD, mEnableQuadCode);
            return INVALID_OPERATION;
        } else if (mSensorParams.vhdrMode == SENSOR_VHDR_MODE_ZVHDR) {
            // ZVHDR Mode, pass ZVHDR relative enum to P1 driver
            if (mEnableDualPD) {
                mCfg.mPattern = (eCAM_DUAL_PIX_ZVHDR);
            } else if (mEnableQuadCode) {
                mCfg.mPattern = (eCAM_4CELL_ZVHDR);
            } else {
                mCfg.mPattern = (eCAM_ZVHDR);
            }
        } else if (mSensorParams.vhdrMode == SENSOR_VHDR_MODE_IVHDR) {
            // IVHDR Mode, pass IVHDR relative enum to P1 driver
            if (mEnableDualPD) {
                mCfg.mPattern = (eCAM_DUAL_PIX_IVHDR);
            } else if (mEnableQuadCode) {
                mCfg.mPattern = (eCAM_4CELL_IVHDR);
            } else {
                mCfg.mPattern = (eCAM_IVHDR);
            }
        } else if (mEnableDualPD) {
            mCfg.mPattern = (eCAM_DUAL_PIX);//EPipe_Dual_pix
        } else if (mEnableQuadCode) {
            mCfg.mPattern = (eCAM_4CELL);//EPipe_QuadCode
        } else {
            mCfg.mPattern = (eCAM_NORMAL);//EPipe_Normal
        }
    }
    if (mCfg.mPattern != eCAM_NORMAL) {
        MBOOL ret = MFALSE;
        sCAM_QUERY_SUPPORT_PATTERN res;
        res.QueryOutput = 0x0;
        ret = pModule->query(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_SUPPORT_PATTERN,
            (MUINTPTR)(&res));
        if (!ret) {
            MY_LOGE("[Cam::%d] Cannot query ENPipeQueryCmd_SUPPORT_PATTERN",
                getOpenId());
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return BAD_VALUE;
            #endif
        } else if ((res.QueryOutput & (0x1 << mCfg.mPattern)) == 0x0) {
            MY_LOGE("[Cam::%d] ENPipeQueryCmd_SUPPORT_PATTERN - support (0x%X) ,"
                " but Pattern set as (0x%X) - by "
                "DualPD(%d) QuadCode(%d) VhdrMode(%d)",
                getOpenId(), res.QueryOutput, mCfg.mPattern,
                mEnableDualPD, mEnableQuadCode, mSensorParams.vhdrMode);
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return INVALID_OPERATION;
            #endif
        }
    }
    //
    // check IQ Level
    mCfg.mQualityLv = eCamIQ_MAX;
    {
        switch (mResizeQuality) {
            case RESIZE_QUALITY_H:
                mCfg.mQualityLv = eCamIQ_H;
                break;
            case RESIZE_QUALITY_L:
                mCfg.mQualityLv = eCamIQ_L;
                break;
            default:
                break;
        }
    }
    if (mCfg.mQualityLv != eCamIQ_MAX) {
        MBOOL ret = MFALSE;
        sCAM_QUERY_IQ_LEVEL res;
        res.QueryOutput = MFALSE;
        ret = pModule->query(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_IQ_LEVEL,
            (MUINTPTR)(&res));
        if (!ret) {
            MY_LOGE("[Cam::%d] Cannot query ENPipeQueryCmd_IQ_LEVEL",
                getOpenId());
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return BAD_VALUE;
            #endif
        } else if (res.QueryOutput == MFALSE) {
            MY_LOGE("[Cam::%d] ENPipeQueryCmd_IQ_LEVEL - not support ,"
                " but Quality-Level set as (%d)",
                getOpenId(), mCfg.mQualityLv);
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return INVALID_OPERATION;
            #endif
        }
    }
    //
    // check Dynamic Twin
    mCfg.mSupportDynamicTwin = MFALSE;
    #if 1
    {
        MBOOL ret = MFALSE;
        sCAM_QUERY_D_Twin res;
        res.QueryOutput = MFALSE;
        ret = pModule->query(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin,
            (MUINTPTR)(&res));
        if (!ret) {
            MY_LOGE("[Cam::%d] Cannot query ENPipeQueryCmd_D_Twin",
                getOpenId());
            #if USING_DRV_QUERY_CAPABILITY_EXP_SKIP
            MY_LOGW("USING_DRV_QUERY_CAPABILITY_EXP_SKIP go-on");
            #else
            return BAD_VALUE;
            #endif
        }
        mCfg.mSupportDynamicTwin = res.QueryOutput;
    }
    #else
    {
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
        pModule->query(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin,
            0, 0, info);
        mCfg.mSupportDynamicTwin = info.D_TWIN;
    }
    #endif
    //
    mIsLegacyStandbyMode = (mCfg.mSupportDynamicTwin) ? MFALSE : MTRUE;
    mIsDynamicTwinEn = (mCfg.mSupportDynamicTwin && (!mDisableDynamicTwin)) ?
        MTRUE : MFALSE;
    //
    // check Sensor-TG Number
    mCfg.mSensorNum = E_1_SEN;
    switch (mTgNum) {
        case 0:
        case 1:
            mCfg.mSensorNum = E_1_SEN;
            break;
        case 2:
        default:
            mCfg.mSensorNum = E_2_SEN;
            break;
    };
    //
    return OK;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
config(ConfigParams const& rParams)
{
    PUBLIC_APIS_IN;
    P1_TRACE_AUTO(SLG_B, "P1:config");

    Mutex::Autolock _l(mPublicLock);

    MY_LOGI2("CheckCurrent active:%d ready:%d state:%d",
        isActive(), isReady(), getLaunchState());
    if(isActive()) {
        MY_LOGD0("active=%d", isActive());
        onHandleFlush(MFALSE);
    } else {
        abandonRequest(MTRUE); // release previous request
    }
    setLaunchState(NSP1Node::LAUNCH_STATE_IDLE);

    //(1) check
    MERROR err = check_config(rParams);
    if (err != OK) {
        MY_LOGE("Config Param - Check fail (%d)", err);
        return err;
    }
    P1_ATOMIC_RELEASE;

    //(2) configure hardware
    if (mpConCtrl != NULL && (!EN_INIT_REQ_RUN)) { // init-request, no aid-start
        mpConCtrl->setAidUsage(MTRUE);
    }
    //
    if (mpTimingCheckerMgr != NULL) {
        mpTimingCheckerMgr->waitReady();
    }
    //
    mpTaskCtrl->reset();
    mLastNum = P1_MAGIC_NUM_FIRST;
    //
    err = hardwareOps_start();
    if ((!(EN_START_CAP_RUN || EN_INIT_REQ_RUN)) || (err != OK)) {
        if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) {
            mpConCtrl->cleanAidStage();
        }
        if (mpTimingCheckerMgr != NULL) {
            mpTimingCheckerMgr->setEnable(MFALSE);
        }
    }
    if (err != OK) {
        MY_LOGE("Config Param - HW start fail (%d)", err);
        return err;
    }

    PUBLIC_APIS_OUT;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
fetchJob(
    P1QueJob & rOutJob
)
{
    U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        return BAD_VALUE;
    };
    rOutJob.clear();
    MINT cnt = 0;
    mpTaskCtrl->sessionLock();
    cnt = mpTaskCollector->requireJob(rOutJob);
    mTagList.set(cnt);
    if (rOutJob.empty()) {
        MY_LOGI1("using-dummy-request");
        if (LOGI_LV2) {
            mpTaskCollector->dumpRoll();
        }
        //
        P1TaskCollector dummyCollector(mpTaskCtrl);
        for (int i = 0; i < mBurstNum; i++) {
            P1QueAct newAct;
            dummyCollector.enrollAct(newAct);
            createAction(newAct, NULL, REQ_TYPE_DUMMY);
            dummyCollector.verifyAct(newAct);
        }
        dummyCollector.requireJob(rOutJob);
    }
    mpTaskCtrl->sessionUnLock();
    if (!rOutJob.ready()) {
        MY_LOGE("job-not-ready");
        mpTaskCtrl->dumpActPool();
        return BAD_VALUE;
    }
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setRequest(
    MBOOL initial
)
{
    FUNCTION_P1_IN;
    //
    Mutex::Autolock _ll(mFrameSetLock);
    U_if (!initial && !mFrameSetAlready) {
        MY_LOGI0("frame set not init complete");
        return;
    }
    U_if (!isActive()) {
        MY_LOGI0("not-active-return");
        return;
    }
    //
    P1QueJob job(mBurstNum);
    //
    {
        if (OK != fetchJob(job)) {
            MY_LOGE("job-fetch-fail");
            return;
        }
        if (mpHwStateCtrl != NULL &&
            mpHwStateCtrl->isLegacyStandby() &&
            mpHwStateCtrl->checkReceiveRestreaming()) {
            P1Act pAct = GET_ACT_PTR(pAct, job.edit(0), RET_VOID);
            if (pAct->ctrlSensorStatus == SENSOR_STATUS_CTRL_STREAMING) {
                mpHwStateCtrl->checkRestreamingNum(pAct->getNum());
            }
        }
    }
    //
    if (!initial) {
        beckonRequest();
    }
    //
    if (IS_BURST_OFF && // exclude burst mode
        (job.size() >= 1)) { // check control callback
        attemptCtrlSetting(job.edit(0));
    }
    //
    #if USING_CTRL_3A_LIST
    List<MetaSet_T> ctrlList;
    generateCtrlList(&ctrlList, job);
    MY_LOGD3("CtrlList[%zu]", ctrlList.size());
    #else
    std::vector< MetaSet_T* > ctrlQueue;
    ctrlQueue.clear();
    ctrlQueue.reserve(job.size());
    generateCtrlQueue(ctrlQueue, job);
    MY_LOGD3("CtrlQueue[%zu]", ctrlQueue.size());
    #endif
    //
    mLastSetNum = job.getLastNum();
    mTagSet.set(mLastSetNum);
    {
        Mutex::Autolock _l(mRequestQueueLock);
        mRequestQueue.push_back(job);
        P1_ATOMIC_RELEASE;
    }
    P1QueAct * qAct = (job.ready()) ? (job.getLastAct()) : (NULL);
    if (qAct == NULL) {
        MY_LOGW("job-not-ready [%zu] < [%d]", job.size(), job.getMax());
        return;
    }
    P1Act pAct = GET_ACT_PTR(pAct, (*qAct), RET_VOID);
    #if SUPPORT_3A
    if (mp3A) {
        MINT32 p_key = qAct->id();
        MINT32 m_num = pAct->magicNum;
        MINT32 f_num = pAct->frmNum;
        MINT32 r_num = pAct->reqNum;
        if (initial) {
            mLogInfo.setMemo(LogInfo::CP_START_SET_BGN,
                LogInfo::START_SET_GENERAL, m_num);
        }
        mLogInfo.setMemo(LogInfo::CP_SET_BGN, p_key, m_num, f_num, r_num);
        P1_TRACE_F_BEGIN(SLG_I, "P1:3A-set|Pkey:%d Mnum:%d Fnum:%d Rnum:%d",
            p_key, m_num, f_num, r_num);
        MY_LOGD2("mp3A->set[%d](%d) +++", p_key, m_num);
        #if USING_CTRL_3A_LIST
        mp3A->set(ctrlList);
        #else
        mp3A->set(ctrlQueue);
        #endif
        MY_LOGD2("mp3A->set[%d](%d) ---", p_key, m_num);
        P1_TRACE_C_END(SLG_I); // "P1:3A-set"
        mLogInfo.setMemo(LogInfo::CP_SET_END, p_key, m_num, f_num, r_num);
        if (initial) {
            mLogInfo.setMemo(LogInfo::CP_START_SET_END,
                LogInfo::START_SET_GENERAL, m_num);
        }
        mFrameSetAlready = MTRUE;
    }
    if (LOGI_LV1) {
        P1_TRACE_F_BEGIN(SLG_PFL, "P1::SET_LOG|Mnum:%d SofIdx:%d Fnum:%d "
            "Rnum:%d FlushSet:0x%x", pAct->magicNum, pAct->sofIdx, pAct->frmNum,
            pAct->reqNum, pAct->flushSet);
        String8 str("");
        MINT32 num = 0;
        size_t idx = 0;
        #if USING_CTRL_3A_LIST
        size_t size = ctrlList.size();
        List<MetaSet_T>::iterator it = ctrlList.begin();
        for (; it != ctrlList.end(); it++) {
            num = it->MagicNum;
            if ((idx > 0) && (idx % mBurstNum == 0)) {
                str += String8::format(", ");
            }
            str += String8::format("%d ", num);
            idx++;
        }
        #else
        size_t size = ctrlQueue.size();
        std::vector< MetaSet_T* >::iterator it = ctrlQueue.begin();
        for (; it != ctrlQueue.end(); it++) {
            num = ((*it) != NULL) ? ((*it)->MagicNum) : (0);
            str += String8::format("%d ", num);
            idx++;
        }
        #endif
        P1_LOGI(1, "[P1::SET]" P1INFO_ACT_STR " Num[%d] Ctrl[%zu]=[ %s]",
            P1INFO_ACT_VAR(*pAct), num, size, str.string());
        P1_TRACE_C_END(SLG_PFL); // "P1::SET_LOG"
    };
    #endif
    FUNCTION_P1_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
acceptRequest(
    sp<IPipelineFrame> pFrame,
    MUINT32 & rRevResult
)
{
    FUNCTION_P1_IN;
    rRevResult = (MUINT32)REQ_REV_RES_ACCEPT_AVAILABLE;
    #if (USING_DRV_IO_PIPE_EVENT)
    {
        RWLock::AutoRLock _l(mIoPipeEvtStateLock);
        if (mIoPipeEvtState != IO_PIPE_EVT_STATE_NONE) {
            rRevResult = (MUINT32)REQ_REV_RES_REJECT_IO_PIPE_EVT;
            return MFALSE;
        }
    }
    #endif
    if ((!isReady()) || (!mFirstReceived)) {
        return MTRUE;
    }
    //
    #if 0 // check-bypass-request
    if (pFrame != NULL) {
        MBOOL isBypass = pFrame->IsReprocessFrame();
        if (isBypass) {
            rRevResult = (MUINT32)REQ_REV_RES_ACCEPT_BYPASS;
            MY_LOGI0("Num[F:%d,R:%d] - BypassFrame",
                P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
            return MTRUE;
        }
    }
    #else
    MBOOL isBypass = MFALSE;
    MUINT32 bitInSet = 0x0;
    if (pFrame != NULL) {
        IPipelineFrame::InfoIOMapSet rIOMapSet;
        U_if (OK != pFrame->queryInfoIOMapSet(getNodeId(), rIOMapSet)) {
            MY_LOGE("queryInfoIOMap failed");
            rRevResult = (MUINT32)REQ_REV_RES_REJECT_NO_IO_MAP_SET;
            return MFALSE;
        }
        IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet =
                                            rIOMapSet.mImageInfoIOMapSet;
        U_if (imageIOMapSet.isEmpty()) {
            MY_LOGI0("no ImageIOmap in frame");
            // not bypass request, go-on ...
        } else {
            for (size_t i = 0; i < imageIOMapSet.size(); i++) {
                IPipelineFrame::ImageInfoIOMap const& imageIOMap =
                    imageIOMapSet[i];
                if (imageIOMap.vIn.size() > 0) {
                    for (size_t j = 0; j < imageIOMap.vIn.size(); j++) {
                        StreamId_T const streamId = imageIOMap.vIn.keyAt(j);
                        for (MUINT32 s = STREAM_IMG_IN_BGN;
                            s <= STREAM_IMG_IN_END; s++) {
                            if ((mvStreamMeta[s] != NULL) &&
                                (mvStreamImg[s]->getStreamId() == streamId)) {
                                bitInSet |= (0x1 << s);
                                isBypass = MTRUE;
                            }
                        }
                        if (isBypass) {
                            break;
                        }
                    }
                }
            }
        }
    }
    if (isBypass) {
        rRevResult = (MUINT32)REQ_REV_RES_ACCEPT_BYPASS;
        MY_LOGI0("Num[F:%d,R:%d] - BypassFrame - InStreamSet:bit[0x%x]",
            P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame), bitInSet);
        return MTRUE;
    }
    #endif
    //
    MINT cnt = 0;
    MBOOL isAccept = checkReqCnt(cnt);
    MY_LOGI2("Num[F:%d,R:%d] - Cnt(%d) Accept(%d)",
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame), cnt, isAccept);
    if (!isAccept) {
        rRevResult = (MUINT32)REQ_REV_RES_REJECT_NOT_AVAILABLE;
    };
    FUNCTION_P1_OUT;
    return isAccept;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
beckonRequest()
{
    FUNCTION_P1_IN;
    MINT cnt = 0;
    L_if (checkReqCnt(cnt)) {
        MINT32 frmNum = P1_FRM_NUM_NULL;
        MINT32 reqNum = P1_REQ_NUM_NULL;
        MINT32 cnt = lastFrameRequestInfoNotice(frmNum, reqNum, 1);
        MBOOL exeCb = MTRUE;
        {
            Mutex::Autolock _l(mPipelineCbLock);
            android::sp<INodeCallbackToPipeline> spCb = mwpPipelineCb.promote();
            L_if (spCb != NULL) {
                MY_LOGI2("Pipeline_CB (F:%d,R:%d) CbButNotQueCnt:%d +++",
                    frmNum, reqNum, cnt);
                LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_REQ_NOTIFY_BGN,
                    LogInfo::CP_REQ_NOTIFY_END, frmNum, reqNum, cnt);
                INodeCallbackToPipeline::CallBackParams param;
                param.nodeId = getNodeId();
                param.lastFrameNum = frmNum;
                spCb->onCallback(param);
                MY_LOGI2("Pipeline_CB (F:%d,R:%d) CbButNotQueCnt:%d ---",
                    frmNum, reqNum, cnt);
            } else {
                exeCb = MFALSE;
            }
        }
        U_if (!exeCb) {
            cnt = lastFrameRequestInfoNotice(frmNum, reqNum, (-1)); // reset count
            MY_LOGI0("Pipeline_CB not exist (F:%d,R:%d) CbButNotQueCnt:%d",
                frmNum, reqNum, cnt);
        }
        return MTRUE;
    } else {
        MY_LOGI0("not-callback - cnt(%d)", cnt);
    }
    FUNCTION_P1_OUT;
    return MFALSE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
checkReqCnt(MINT32 & cnt)
{
    FUNCTION_P1_IN;
    U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        MY_LOGE("Task Controller or Collector not acceptable");
        return MFALSE;
    };
    //
    MINT depth = mDepthNum;
    MINT cnt_num = depth * mBurstNum;
    MINT que_num = 0;
    MBOOL isAccept = MTRUE;
    mpTaskCtrl->sessionLock();
    if ((que_num = mpTaskCollector->remainder()) >= cnt_num) {
        isAccept = MFALSE;
    }
    mpTaskCtrl->sessionUnLock();
    MY_LOGI2("Que(%d) < Cnt(%d)=(%d*%d) : Accept(%d)",
        que_num, cnt_num, depth, mBurstNum, isAccept);
    cnt = que_num;
    FUNCTION_P1_OUT;
    return isAccept;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
abandonRequest(MBOOL bWaitDone)
{
    FUNCTION_P1S_IN;
    //
    U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        MY_LOGE("Task Controller or Collector not ready");
        return BAD_VALUE;
    };
    //
    {
        mpTaskCtrl->sessionLock();
        MINT cnt = mpTaskCollector->remainder();
        P1_TRACE_F_BEGIN(SLG_E, "P1:abandon(%d)", cnt);
        MY_LOGI1("cnt(%d)", cnt);
        while (cnt > 0) {
            P1QueAct qAct;
            cnt = mpTaskCollector->requireAct(qAct);
            if (qAct.id() > P1ACT_ID_NULL) {
                P1Act act = GET_ACT_PTR(act, qAct, BAD_VALUE);
                MY_LOGI0("ABANDON - " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                onReturnFrame(qAct, FLUSH_ABANDON, MTRUE);
                /* DO NOT use this P1QueAct after onReturnFrame() */
            }
        }
        mTagList.set(cnt);
        P1_TRACE_C_END(SLG_E); // "P1:abandon"
        mpTaskCtrl->sessionUnLock();
    }
    MY_LOGI0("WaitDeliverDrainDone (%d)", bWaitDone);
    if (bWaitDone) {
        U_if (!mpDeliverMgr->waitFlush(MTRUE)) {
            MY_LOGW("request not done after abandon");
        };
    };
    //
    FUNCTION_P1S_OUT;
    //
    return OK;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
recovery(
    MUINT8 state
)
{
    FUNCTION_P1S_IN;
    //
    {   // pre-check before lock got
        MINT current_cnt = 0;
        U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
            MY_LOGE("Task Controller or Collector not acceptable");
            return;
        } else {
            MY_LOGI3("SessionLock+");
            mpTaskCtrl->sessionLock();
            current_cnt = mpTaskCollector->remainder();
            mpTaskCtrl->sessionUnLock();
            MY_LOGI3("SessionLock-");
        };
        if (current_cnt <= 0) {
            MY_LOGI0("NO-Request need to recovery");
            return;
        }
    }
    //
    MY_LOGI3("PublicLock+");
    Mutex::Autolock _l(mPublicLock);
    MY_LOGI3("PublicLock-");
    //
    if (state != getLaunchState()) { // check state after lock got
        MY_LOGI0("LaunchState (%d) => (%d)", state, getLaunchState());
        return;
    }
    //
    MINT que_cnt = 0;
    U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        MY_LOGE("Task Controller or Collector not acceptable");
        return;
    } else {
        MY_LOGI3("SessionLock+");
        mpTaskCtrl->sessionLock();
        que_cnt = mpTaskCollector->remainder();
        mpTaskCtrl->sessionUnLock();
        MY_LOGI3("SessionLock-");
    };
    MY_LOGI0("State(%d) QueCnt(%d) BurstNum(%d) StartCap(%d) InitReq(%d) "
        "NeedReady(%d)", state, que_cnt, mBurstNum,
        EN_START_CAP_RUN, EN_INIT_REQ_RUN, mNeedHwReady);
    if (que_cnt <= 0) {
        MY_LOGI0("NO-Request received need to recovery");
    } else if (mBurstNum > 1) {
        MY_LOGI0("NOT-RECOVERY in BurstMode(%d)", mBurstNum);
    } else if ((!EN_START_CAP_RUN) && (!EN_INIT_REQ_RUN)) {
        MY_LOGI0("NOT-RECOVERY in NON START_CAP(%d) INIT_REQ(%d)",
            EN_START_CAP_RUN, EN_INIT_REQ_RUN);
    } else {
        mLogInfo.inspect(LogInfo::IT_LAUNCH_STATE_TIMEOUT);
        { // set forced-off:TRUE for this time and it will set forced-off with the new value again while onHandleFlush
            if (EN_INIT_REQ_CFG) {
                mInitReqNum = mInitReqSet * mBurstNum;
                mInitReqCnt = 0;
                mInitReqOff = MTRUE;
                if (!EN_INIT_REQ_RUN) {
                    MY_LOGI0("SetOff - InitReq Set:%d Num:%d Cnt:%d Off:%d",
                        mInitReqSet, mInitReqNum, mInitReqCnt, mInitReqOff);
                }
            }
            if (EN_START_CAP_CFG) {
                mEnableCaptureOff = MTRUE;
                if (!EN_START_CAP_RUN) {
                    MY_LOGI0("SetOff - StartCap Set:%d Off:%d",
                        mEnableCaptureFlow, mEnableCaptureOff);
                }
            }
        }
        //
        if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) { // no-matter aid-start on/off, clean usage
            mpConCtrl->cleanAidStage();
            mpConCtrl->initBufInfo_clean();
        }
        //
        if (mNeedHwReady) { // state == LAUNCH_STATE_ACTIVE
            MY_LOGI0("HW ready +++");
            MERROR err = hardwareOps_ready();
            if (mpTimingCheckerMgr != NULL) { // it must be not-EN_START_CAP_RUN and not-EN_INIT_REQ_RUN, disable TimingCheckerMgr
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
            if (err != OK) {
                MY_LOGE("ReactRequest - HW ready fail (%d)", err);
                abandonRequest(MTRUE);
                return;
            }
            MY_LOGI0("HW ready ---");
        } else { // state == LAUNCH_STATE_IDLE
            MY_LOGI0("HW start +++");
            MERROR err = hardwareOps_start();
            if (mpTimingCheckerMgr != NULL) { // it must be not-EN_START_CAP_RUN and not-EN_INIT_REQ_RUN, disable TimingCheckerMgr
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
            if (err != OK) {
                MY_LOGE("ReactRequest - HW start fail (%d)", err);
                abandonRequest(MTRUE);
                return;
            }
            MY_LOGI0("HW start ---");
        }
        //
        mFirstReceived = MTRUE;
        setRequest(MTRUE);
    }
    FUNCTION_P1S_OUT;
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
setNodeCallBack(
    android::wp<INodeCallbackToPipeline> pCallback
)
{
    Mutex::Autolock _l(mPipelineCbLock);
    MY_LOGI1("PipelineNodeCallBack=%p", pCallback.unsafe_get());
    mwpPipelineCb = pCallback;
    return OK;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
queue(
    sp<IPipelineFrame> pFrame
)
{
    PUBLIC_API_IN;
    mLogInfo.setMemo(LogInfo::CP_REQ_ARRIVE,
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    Mutex::Autolock _l(mPublicLock);
    //
    MUINT32 revResult = (MUINT32)REQ_REV_RES_UNKNOWN;
    if (!acceptRequest(pFrame, revResult)) {
        mLogInfo.setMemo(LogInfo::CP_REQ_ACCEPT,
            P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame), MFALSE, revResult);
        PUBLIC_API_OUT;
        return FAILED_TRANSACTION;
    }
    mLogInfo.setMemo(LogInfo::CP_REQ_ACCEPT,
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame), MTRUE, revResult);
    //
    lastFrameRequestInfoUpdate(P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    //
    LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_REQ_REV, LogInfo::CP_REQ_RET,
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    //mLogInfo.setMemo(LogInfo::CP_REQ_REV, P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame)); // use AutoMemo
    P1_TRACE_F_BEGIN(SLG_I, "P1:queue|Fnum:%d Rnum:%d",
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    MY_LOGD2("active=%d", isActive());
    //
    MBOOL isStartSet = MFALSE;
    MINT32 currReqCnt = 0;
    currReqCnt = android_atomic_inc(&mInFlightRequestCnt);
    P1_TRACE_INT(SLG_B, "P1_request_cnt",
        android_atomic_acquire_load(&mInFlightRequestCnt));
    MY_LOGD3("InFlightRequestCount++ (%d) => (%d)", currReqCnt,
        android_atomic_acquire_load(&mInFlightRequestCnt));
    //
    //MBOOL isRestreaming = MFALSE;
    //
    if (EN_INIT_REQ_RUN) {
        if (mInitReqCnt <= (mInitReqNum + mBurstNum)) {
            mInitReqCnt ++;
        }
    }
    //
    MINT cnt = 0;
    if (EN_INIT_REQ_RUN && (mInitReqCnt < mInitReqNum)) {
        P1QueAct newAct;
        mpTaskCtrl->sessionLock();
        mpTaskCollector->enrollAct(newAct);
        createAction(newAct, pFrame);
        cnt = mpTaskCollector->verifyAct(newAct);
        mTagList.set(cnt);
        mpTaskCtrl->sessionUnLock();
    // else if (mInitReqCnt == mInitReqNum) go-on the following flow
    } else { // for REV_MODE_NORMAL/REV_MODE_CONSERVATIVE
        #if 1 // restart while queue ready
        if (!isActive()) {
            MY_LOGI0("HW start +++");
            if (mpConCtrl != NULL && (!EN_INIT_REQ_RUN)) { // init-request, no aid-start
                mpConCtrl->setAidUsage(MTRUE);
            }
            MERROR err = hardwareOps_start();
            if ((!(EN_START_CAP_RUN || EN_INIT_REQ_RUN)) || (err != OK)) {
                if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) {
                    mpConCtrl->cleanAidStage();
                }
                if (mpTimingCheckerMgr != NULL) {
                    mpTimingCheckerMgr->setEnable(MFALSE);
                }
            }
            if (err != OK) {
                MY_LOGE("Queue Frame - HW start fail (%d)", err);
                P1_TRACE_C_END(SLG_I); // "P1:queue"
                return err;
            }
            MY_LOGI0("HW start ---");
        }
        #endif
        if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
            MY_LOGE("Task Controller or Collector not ready");
            P1_TRACE_C_END(SLG_I); // "P1:queue"
            return BAD_VALUE;
        };
        P1QueAct newAct;
        P1QueAct setAct;
        MetaSet_T preSet;
        P1Act pSetAct = NULL;
        mpTaskCtrl->sessionLock();
        //
        mpTaskCollector->enrollAct(newAct);
        createAction(newAct, pFrame);
        cnt = mpTaskCollector->verifyAct(newAct);
        //
        P1Act pAct = GET_ACT_PTR(pAct, newAct, BAD_VALUE);
        if (pAct->ctrlSensorStatus == SENSOR_STATUS_CTRL_STANDBY) {
            MY_LOGI0("receive-standby-control");
        } else if (pAct->ctrlSensorStatus == SENSOR_STATUS_CTRL_STREAMING) {
            MY_LOGI0("receive-streaming-control");
            hardwareOps_streaming();
        }
        //
        if ((mFirstReceived) && (pAct->reqType == REQ_TYPE_YUV)) {
            P1QueAct paddingAct;
            mpTaskCollector->enrollAct(paddingAct);
            createAction(paddingAct, NULL, REQ_TYPE_PADDING);
            cnt = mpTaskCollector->verifyAct(paddingAct);
            MY_LOGI0("add-padding-for-YUV-stall Id:%d Num:%d Type:%d",
                paddingAct.id(), paddingAct.getNum(), paddingAct.getType());
        }
        mTagList.set(cnt);
        if (IS_BURST_OFF && mFirstReceived &&
            pAct->getType() == ACT_TYPE_NORMAL) {
            mpTaskCollector->queryAct(setAct);
            pSetAct = setAct.ptr();//GET_ACT_PTR(pSetAct, firstAct, BAD_VALUE);
            if (pSetAct != NULL) { // for the consideration of session locking peroid with 3A CB and preset, duplicate this MetaSet
                preSet = pSetAct->metaSet;
            } else {
                MY_LOGW("no act ready to PreSet");
            }
        }
        mpTaskCtrl->sessionUnLock();
        if (mp3A && IS_BURST_OFF && mFirstReceived && (pSetAct != NULL)) {
            if (preSet.PreSetKey <= P1_PRESET_KEY_NULL) {
                MY_LOGW("Pre-Set-Meta NOT ready (%d)", preSet.PreSetKey);
            } else {
                if (preSet.Dummy > 0) {
                    MY_LOGI0("Pre-Set-Meta is dummy (%d)", preSet.Dummy);
                }
                MINT32 f_Num = pSetAct->frmNum;
                MINT32 r_Num = pSetAct->reqNum;
                std::vector< MetaSet_T* > ctrlQueue; // only insert once
                ctrlQueue.push_back(&preSet);
                if (mMetaLogOp > 0 && ctrlQueue.size() > 0) {
                    P1_LOG_META(*pSetAct, &(ctrlQueue[0]->appMeta),
                        "3A.PreSet-APP");
                    P1_LOG_META(*pSetAct, &(ctrlQueue[0]->halMeta),
                        "3A.PreSet-HAL");
                }
                mLogInfo.setMemo(LogInfo::CP_PRE_SET_BGN,
                    preSet.PreSetKey, preSet.Dummy, f_Num, r_Num);
                P1_TRACE_F_BEGIN(SLG_I, "P1:3A-preset|Pkey:%d Fnum:%d Rnum:%d",
                    preSet.PreSetKey, f_Num, r_Num);
                MY_LOGD2("mp3A->preset[%d] +++", preSet.PreSetKey);
                mp3A->preset(ctrlQueue);
                MY_LOGD2("mp3A->preset[%d] ---", preSet.PreSetKey);
                P1_TRACE_C_END(SLG_I); // "P1:3A-preset"
                mLogInfo.setMemo(LogInfo::CP_PRE_SET_END,
                    preSet.PreSetKey, preSet.Dummy, f_Num, r_Num);
                if (LOGI_LV1) { // P1_LOGI(1)
                    pAct->msg += String8::format(" | [PreSet][Key:%d] Num(%d) "
                        "Dummy(%d) MetaCnt[APP:%d,HAL:%d]",
                        preSet.PreSetKey, preSet.MagicNum, preSet.Dummy,
                        preSet.appMeta.count(), preSet.halMeta.count());
                }
            }
        }
        //
        if (!mFirstReceived) {
            if (cnt >= mBurstNum) {
                mFirstReceived = MTRUE;
                isStartSet = MTRUE;
            } // else not-start and not-wait, then try to receive more requests
        }
        //
        if (LOGI_LV1) { // P1_LOGI(1)
            P1_TRACE_F_BEGIN(SLG_PFL, "P1::REQ_LOG|Mnum:%d SofIdx:%d Fnum:%d "
                "Rnum:%d FlushSet:0x%x", pAct->magicNum, pAct->sofIdx,
                pAct->frmNum, pAct->reqNum, pAct->flushSet);
            pAct->msg += String8::format(" | [Rev:%d] depth(%d) burst(%d) "
                "Que[%d]", mReceiveMode, mDepthNum, mBurstNum,
                mpTaskCollector->remainder());
            P1_LOGI(1, "%s", pAct->msg.string());
            P1_TRACE_C_END(SLG_PFL); // "P1::REQ_LOG"
        }
    };
    //
#if 1 //SET_REQUEST_BEFORE_FIRST_DONE
    if (isStartSet) {
        if (EN_INIT_REQ_RUN && (!isReady())) {
            MY_LOGI0("HW request +++");
            MERROR err = hardwareOps_request();
            if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) {
                mpConCtrl->cleanAidStage();
            }
            if (mpTimingCheckerMgr != NULL) {
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
            if (err != OK) {
                MY_LOGE("Queue Frame - HW request fail (%d)", err);
                P1_TRACE_C_END(SLG_I); // "P1:queue"
                return err;
            }
            MY_LOGI0("HW request ---");
        } else if (EN_START_CAP_RUN && (!isReady())) {
            MY_LOGI0("HW capture +++");
            MERROR err = hardwareOps_capture();
            if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) {
                mpConCtrl->cleanAidStage();
            }
            if (mpTimingCheckerMgr != NULL) {
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
            if (err != OK) {
                MY_LOGE("Queue Frame - HW capture fail (%d)", err);
                P1_TRACE_C_END(SLG_I); // "P1:queue"
                return err;
            }
            MY_LOGI0("HW capture ---");
        } else {
            //onRequestFrameSet(MTRUE);
            setRequest(MTRUE);
        }
        P1_ATOMIC_RELEASE;
    }
#endif
    //
    if (mpHwStateCtrl != NULL) {
        mpHwStateCtrl->checkRequest();
    }
    //
    inflightMonitoring(IMT_REQ);
    //
    //mLogInfo.setMemo(LogInfo::CP_REQ_RET, P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame)); // use AutoMemo
    P1_TRACE_C_END(SLG_I); // "P1:queue"
    //
    PUBLIC_API_OUT;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
kick()
{
    PUBLIC_API_IN;
    //
    if ((!isActive()) || (!isReady())) {
        MY_LOGI0("return OK - active(%d) ready(%d)", isActive(), isReady());
        return OK;
    }
    if (IS_BURST_ON) {
        MY_LOGI0("return OK - BurstNum(%d)", mBurstNum);
        return OK;
    }
    U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        MY_LOGE("Task Controller or Collector not ready");
        return BAD_VALUE;
    };
    //
    {
        mpTaskCtrl->sessionLock();
        MINT cnt = mpTaskCollector->remainder();
        P1_TRACE_F_BEGIN(SLG_E, "P1:kick(%d)", cnt);
        MY_LOGI1("cnt(%d)", cnt);
        while (cnt > 0) {
            P1QueAct qAct;
            cnt = mpTaskCollector->requireAct(qAct);
            if (qAct.id() > P1ACT_ID_NULL) {
                P1Act act = GET_ACT_PTR(act, qAct, BAD_VALUE);
                if (act->ctrlSensorStatus != SENSOR_STATUS_CTRL_NONE) {
                    MY_LOGI0("Cannot KICK Standby Ctrl Request - "
                        P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                } else {
                    MY_LOGI0("KICK - " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                    onReturnFrame(qAct, FLUSH_KICK, MTRUE);
                    /* DO NOT use this P1QueAct after onReturnFrame() */
                }
            }
        }
        mTagList.set(cnt);
        P1_TRACE_C_END(SLG_E); // "P1:kick"
        mpTaskCtrl->sessionUnLock();
    }
    //
    PUBLIC_API_OUT;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
flush()
{
    PUBLIC_APIS_IN;

    P1_TRACE_AUTO(SLG_B, "P1:flush");

    LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_API_FLUSH_BGN,
        LogInfo::CP_API_FLUSH_END);

    kick();

    Mutex::Autolock _l(mPublicLock);

    onHandleFlush(MFALSE);

    //wait until deque thread going back to waiting state;
    //in case next node receives queue() after flush()

    PUBLIC_APIS_OUT;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
requestExit()
{
    FUNCTION_P1_IN;

    //let deque thread back
    Thread::requestExit();
    MY_LOGI3("ThreadExit");
    {
        Mutex::Autolock _l(mThreadLock);
        mActiveCond.broadcast();
    }
    {
        Mutex::Autolock _l(mThreadLock);
        mReadyCond.broadcast();
    }
    //
    MY_LOGI3("ThreadJoin");
    join();
    MY_LOGI3("ThreadEnd");
    //let cb thread back
    {
        Mutex::Autolock _l(mStartLock);
        mStartCond.broadcast();
    }

    FUNCTION_P1_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P1NodeImp::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)"CAM_P1", 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, P1THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, P1THREAD_PRIORITY);   //  Note: "priority" is nice value.
    //
    ::sched_getparam(0, &sched_p);
    MY_LOGD0(
        "Tid: %d, policy: %d, priority: %d"
        , ::gettid(), ::sched_getscheduler(0)
        , ::getpriority(PRIO_PROCESS, 0)
    );

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P1NodeImp::
threadLoop()
{
    // check if going to leave thread
    P1_TRACE_FUNC(SLG_B);//P1_TRACE_AUTO(SLG_B, "P1:threadLoop");
    //
    MUINT8 active_state = (MUINT8)NSP1Node::LAUNCH_STATE_NULL;
    MUINT8 ready_state = (MUINT8)NSP1Node::LAUNCH_STATE_NULL;
    status_t waitRes = NO_ERROR;
    MUINT32 waitCnt = 0;
    MUINT32 waitMax = P1NODE_LAUNCH_WAIT_CNT_MAX;
    nsecs_t nsTimeout = P1NODE_LAUNCH_WAIT_INV_NS;
    U_if (mTimingFactor > 2) {
        waitMax *= 2; // for ENG
    }
    {
        waitRes = NO_ERROR;
        waitCnt = 0;
        while (!isActive()) {
            {
                Mutex::Autolock _l(mThreadLock);
                P1_TRACE_S_BEGIN(SLG_S, "P1:wait_active");
                MY_LOGI0("wait active + (%d-%d)", waitRes, waitCnt);
                waitRes = mActiveCond.waitRelative(mThreadLock, nsTimeout);
                active_state = getLaunchState();
                if (active_state == (MUINT8)NSP1Node::LAUNCH_STATE_IDLE) {
                    waitCnt ++;
                    if (waitCnt >= (waitMax >> 1)) {
                        MY_LOGI0("WaitActive(%d)-State(%d):(%d*%" PRId64 "ns)",
                            waitRes, active_state, waitCnt, (MINT64)nsTimeout);
                    }
                } else {
                    waitCnt = 0;
                }
                MY_LOGI0("wait active - (%d-%d)", waitRes, waitCnt);
                P1_TRACE_C_END(SLG_S); // "P1:wait_active"
            }
            //
            if (exitPending()) {
                MY_LOGI1("leaving active");
                return false;
            }
            //
            if (active_state == (MUINT8)NSP1Node::LAUNCH_STATE_IDLE &&
                waitCnt >= waitMax) {
                recovery(active_state);
                active_state = (MUINT8)NSP1Node::LAUNCH_STATE_NULL;
                waitCnt = 0;
            }
        };
    }
    //
    if ((isActiveButNotReady()) &&
        mpConCtrl != NULL && mpConCtrl->getAidUsage()) {
        procedureAid_start();
    }
    //
    {
        waitRes = NO_ERROR;
        waitCnt = 0;
        while (isActiveButNotReady()) {
            {
                Mutex::Autolock _l(mThreadLock);
                P1_TRACE_S_BEGIN(SLG_S, "P1:wait_ready");
                MY_LOGI0("wait ready + (%d-%d)", waitRes, waitCnt);
                waitRes = mReadyCond.waitRelative(mThreadLock, nsTimeout);
                ready_state = getLaunchState();
                if (ready_state == (MUINT8)NSP1Node::LAUNCH_STATE_ACTIVE) {
                    waitCnt ++;
                    if (waitCnt >= (waitMax >> 1)) {
                        MY_LOGI0("WaitReady(%d)-State(%d):(%d*%" PRId64 "ns)",
                            waitRes, ready_state, waitCnt, (MINT64)nsTimeout);
                    }
                } else {
                    waitCnt = 0;
                }
                MY_LOGI0("wait ready - (%d-%d)", waitRes, waitCnt);
                P1_TRACE_C_END(SLG_S); // "P1:wait_ready"
            }
            //
            if (exitPending()) {
                MY_LOGI1("leaving ready");
                return false;
            }
            //
            if (ready_state == (MUINT8)NSP1Node::LAUNCH_STATE_ACTIVE &&
                waitCnt >= waitMax) {
                recovery(ready_state);
                ready_state = (MUINT8)NSP1Node::LAUNCH_STATE_NULL;
                waitCnt = 0;
            }
        };
    }
    //
    if (mpHwStateCtrl != NULL) {
        mpHwStateCtrl->checkThreadStandby();
    }

    // deque buffer, and handle frame and metadata
    onProcessDequeFrame();

    if (!isActive()) {
        MY_LOGI_IF(getInit(), "HW stopped , exit init");
        setInit(MFALSE);
    }

    // trigger point for the first time
    {
        if (getInit()) {
            setInit(MFALSE);
        }
    }

    if ((mpDeliverMgr != NULL) && (mpDeliverMgr->runningGet())) {
        onProcessDropFrame(MTRUE);
    };

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setLaunchState(
    MUINT8 state
)
{
    Mutex::Autolock _l(mLaunchStateLock);
    MY_LOGI2("[LaunchState](%d)<-(%d)", state, mLaunchState);
    mLaunchState = state;
    P1_ATOMIC_RELEASE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT8
P1NodeImp::
getLaunchState(
    void
)
{
    Mutex::Autolock _l(mLaunchStateLock);
    MY_LOGI2("[LaunchState](%d)", mLaunchState);
    return mLaunchState;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
isActive(
    void
)
{
    Mutex::Autolock _l(mLaunchStateLock);
    MBOOL ret = MFALSE;
    if (mLaunchState == NSP1Node::LAUNCH_STATE_ACTIVE ||
        mLaunchState == NSP1Node::LAUNCH_STATE_READY) {
        ret = MTRUE;
    };
    MY_LOGI2("[LaunchState](%d)-(%d)", mLaunchState, ret);
    return ret;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
isReady(
    void
)
{
    Mutex::Autolock _l(mLaunchStateLock);
    MBOOL ret = MFALSE;
    if (mLaunchState == NSP1Node::LAUNCH_STATE_READY) {
        ret = MTRUE;
    };
    MY_LOGI2("[LaunchState](%d)-(%d)", mLaunchState, ret);
    return ret;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
isActiveButNotReady(
    void
)
{
    Mutex::Autolock _l(mLaunchStateLock);
    MBOOL ret = MFALSE;
    if (mLaunchState == NSP1Node::LAUNCH_STATE_ACTIVE) {
        ret = MTRUE;
    };
    MY_LOGI2("[LaunchState](%d)-(%d)", mLaunchState, ret);
    return ret;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setInit(
    MBOOL init
)
{
    Mutex::Autolock _l(mInitLock);
    mInit = init;
    P1_ATOMIC_RELEASE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getInit(
    void
)
{
    Mutex::Autolock _l(mInitLock);
    return mInit;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setPowerNotify(
    MBOOL notify
)
{
    Mutex::Autolock _l(mPowerNotifyLock);
    mPowerNotify = notify;
    P1_ATOMIC_RELEASE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getPowerNotify(
    void
)
{
    Mutex::Autolock _l(mPowerNotifyLock);
    return mPowerNotify;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setQualitySwitching(
    MBOOL switching
)
{
    Mutex::Autolock _l(mQualitySwitchLock);
    mQualitySwitching = switching;
    P1_ATOMIC_RELEASE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getQualitySwitching(
    void
)
{
    Mutex::Autolock _l(mQualitySwitchLock);
    return mQualitySwitching;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
setCurrentBinSize(
    MSize size
)
{
    Mutex::Autolock _l(mCurBinLock);
    mCurBinSize = size;
    P1_ATOMIC_RELEASE;
};


/******************************************************************************
 *
 ******************************************************************************/
MSize
P1NodeImp::
getCurrentBinSize(
    void
)
{
    Mutex::Autolock _l(mCurBinLock);
    return mCurBinSize;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
lastFrameRequestInfoUpdate(
    MINT32 const frameNum,
    MINT32 const requestNum
)
{
    Mutex::Autolock _l(mLastFrmReqNumLock);
    mLastFrmNum = frameNum;
    mLastReqNum = requestNum;
    mLastCbCnt = 0;
    P1_ATOMIC_RELEASE;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MINT32
P1NodeImp::
lastFrameRequestInfoNotice(
    MINT32 & frameNum,
    MINT32 & requestNum,
    MINT32 const addCbCnt
)
{
    Mutex::Autolock _l(mLastFrmReqNumLock);
    frameNum = mLastFrmNum;
    requestNum = mLastReqNum;
    if (addCbCnt != 0) {
        mLastCbCnt += (addCbCnt);
    }
    return mLastCbCnt;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
syncHelperStart()
{
    Mutex::Autolock _l(mSyncHelperLock);
    if (!mSyncHelperReady) {
        if (mspSyncHelper != NULL) {
            status_t res = mspSyncHelper->start(getOpenId());
            if (res == OK) {
                mSyncHelperReady = MTRUE;
            }
        }
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
syncHelperStop()
{
    Mutex::Autolock _l(mSyncHelperLock);
    if (mSyncHelperReady) {
        if (mspSyncHelper != NULL) {
            status_t res = mspSyncHelper->stop(getOpenId());
            if (res == OK) {
                mSyncHelperReady = MFALSE;
            }
        }
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
ensureStartReady(
    MUINT8 infoType,
    MINT32 infoNum
)
{
    status_t res = NO_ERROR;
    MUINT32 needRetry = P1NODE_START_READY_WAIT_CNT_MAX;
    if (isActive()) {
        Mutex::Autolock _l(mStartLock);
        while ((isActiveButNotReady()) && (needRetry != 0)) {
            res = mStartCond.waitRelative(mStartLock,
                P1NODE_START_READY_WAIT_INV_NS);
            needRetry --;
            MY_LOGI0("Type(%d) Num(%d) - EnStartCap(%d) EnInitReqRun(%d) - "
                "LaunchState(%d) WaitStatus(%d) NeedRetry(%d)",
                infoType, infoNum, EN_START_CAP_RUN, EN_INIT_REQ_RUN,
                getLaunchState(), res, needRetry);
            if (!isActive()) {
                MY_LOGI0("Not Active");
                break;
            }
            if (res == NO_ERROR) {
                MY_LOGI0("Got Ready");
                break;
            }
        }
    }
    if (isActiveButNotReady()) {
        MY_LOGE("Wait StartReady Timeout (%d*%d ms) - "
            "Type(%d) Num(%d) - EnStartCap(%d) EnInitReqRun(%d) - "
            "LaunchState(%d) WaitStatus(%d) NeedRetry(%d)",
            P1NODE_START_READY_WAIT_CNT_MAX,
            (MUINT32)(P1NODE_START_READY_WAIT_INV_NS / ONE_MS_TO_NS),
            infoType, infoNum, EN_START_CAP_RUN, EN_INIT_REQ_RUN,
            getLaunchState(), res, needRetry);
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onSyncEnd()
{
    FUNCTION_P1_IN;
    //
    MBOOL toSet = MFALSE;
    if (mpHwStateCtrl != NULL) {
        if (mpHwStateCtrl->checkSkipSync()) {
            MY_LOGI0("SyncEND was paused");
            return;
        }
        MBOOL first = mpHwStateCtrl->checkFirstSync();
        MY_LOGI_IF(first, "Got first CB after re-streaming");
        if (first && IS_BURST_ON) {
            toSet = MTRUE;
        }
    }
    //
    {
        Mutex::Autolock _ll(mFrameSetLock);
        if (!mFrameSetAlready) {
            MY_LOGI0("should not callback before first set");
            return;
        }
        if (EN_START_CAP_RUN && (!isReady())) {
            Mutex::Autolock _l(mStartCaptureLock);
            MY_LOGD2("StartCaptureState(%d)", mStartCaptureState);
            if (mStartCaptureState != START_CAP_STATE_READY) {
                MY_LOGI0("should not callback before capture ready (%d)",
                    mStartCaptureState);
                return;
            }
        }
    }
    //
    if (getInit()) {
        MY_LOGI0("sync before frame done");
    }
    //
    if (isActiveButNotReady()) {
        ensureStartReady(IHal3ACb::eID_NOTIFY_VSYNC_DONE);
    }
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:onSyncEnd|TheLastSet-Mnum:%d", mLastSetNum);
    //
    if (IS_BURST_OFF || toSet) {
        //onRequestFrameSet(MFALSE);
        setRequest(MFALSE);
    }
    //
    P1_TRACE_C_END(SLG_I); // "P1:onSyncEnd"
    //
    FUNCTION_P1_OUT;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onSyncBegin(
    MBOOL initial,
    RequestSet_T* reqSet,//MUINT32 magicNum,
    MUINT32 sofIdx,
    CapParam_T* capParam
)
{
    FUNCTION_P1_IN;
    if (mpHwStateCtrl != NULL) {
        if (mpHwStateCtrl->checkSkipSync()) {
            MY_LOGI0("SyncBGN was paused");
            return;
        }
        MBOOL first = mpHwStateCtrl->checkFirstSync();
        MY_LOGI_IF(first, "Got first CB after re-streaming");
    }
    //
    {
        Mutex::Autolock _ll(mFrameSetLock);
        if (!mFrameSetAlready) {
            MY_LOGI0("should not callback before first set");
            return;
        }
        //
        if (EN_START_CAP_RUN && (!isReady())) {
            Mutex::Autolock _l(mStartCaptureLock);
            MY_LOGD2("StartCaptureState(%d)", mStartCaptureState);
            if (mStartCaptureState == START_CAP_STATE_WAIT_CB) {
                if (capParam != NULL) {
                    mStartCaptureType = capParam->u4CapType;
                    mStartCaptureIdx = sofIdx;
                    mStartCaptureExp = MAX(capParam->i8ExposureTime, 0);
                    if (reqSet != NULL && reqSet->vNumberSet.size() > 0 &&
                        IS_BURST_OFF) {
                        mLongExp.set(reqSet->vNumberSet[0], mStartCaptureExp);
                    }
                }
                mStartCaptureState = START_CAP_STATE_READY;
                mStartCaptureCond.broadcast();
                MY_LOGI1("StartCaptureReady @%d init(%d) Cap-Type(%d)-Idx(%d)"
                    "-Exp(%" PRId64 "ns)", sofIdx, getInit(),
                    mStartCaptureType, mStartCaptureIdx, mStartCaptureExp);
                return;
            } else if (mStartCaptureState == START_CAP_STATE_WAIT_REQ) {
                MY_LOGI0("should not callback before capture set (%d)",
                    mStartCaptureState);
                return;
            }
        }
    }
    //
    if (getInit()) {
        MY_LOGI0("sync before frame done");
    }
    //
    MINT32 magicNum = P1_MAGIC_NUM_NULL;
    if (reqSet != NULL && reqSet->vNumberSet.size() > 0) {
        magicNum = reqSet->vNumberSet[0];
    }
    //
    if (isActiveButNotReady()) {
        ensureStartReady(IHal3ACb::eID_NOTIFY_3APROC_FINISH, magicNum);
    }
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:onSyncBegin|"
        "CB Mnum:%d SofIdx:%d Exp(ns):%" PRId64 " Type:%d", magicNum, sofIdx,
        capParam->i8ExposureTime, capParam->u4CapType);
    //
    //(1)
    if((!initial) && (isReady())) {
        P1QueJob job(mBurstNum);
        bool exist = false;
        {
            Mutex::Autolock _l(mRequestQueueLock);
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                if ((*it).getIdx() == magicNum) {
                    job = *it;
                    for (MUINT8 i = 0; i < job.size(); i++) {
                        P1Act act = GET_ACT_PTR(act, job.edit(i), RET_VOID);
                        act->sofIdx = sofIdx;
                        if (capParam != NULL) {
                            act->capType =
                                capParam->u4CapType;
                            act->frameExpDuration =
                                MAX(capParam->i8ExposureTime, 0);
                            if (act->capType == E_CAPTURE_HIGH_QUALITY_CAPTURE
                                /*&& mRawPostProcSupport == MFALSE*/) { // no matter legacy/non-legacy platform, HQC need pure raw
                                if (act->fullRawType != EPipe_PURE_RAW) {
                                    act->isRawTypeChanged = MTRUE;
                                    MY_LOGI0("HQC (%d) - full raw type change"
                                        " (%d => %d)", mRawPostProcSupport,
                                        act->fullRawType, EPipe_PURE_RAW);
                                }
                                act->fullRawType = EPipe_PURE_RAW;
                            }
                            if (IS_BURST_OFF) {
                                mLongExp.set(act->magicNum,
                                    act->frameExpDuration);
                            }
                            MY_LOGI_IF(((capParam->i8ExposureTime >= 400000000)
                                || (capParam->i8ExposureTime <= 0)), "check CB "
                                "num(%d) cap(%d) exp(%" PRId64 "ns)", magicNum,
                                capParam->u4CapType, capParam->i8ExposureTime);
                            if ((act->capType != E_CAPTURE_NORMAL) &&
                                (act->appFrame != NULL)) {
                                MY_LOGI2(
                                    "Job(%d) - Cap(%d)(%" PRId64 "ns) - "
                                    P1INFO_ACT_STR,
                                    job.getIdx(),
                                    capParam->u4CapType,
                                    capParam->i8ExposureTime,
                                    P1INFO_ACT_VAR(*act));
                            }
                        } else {
                            MY_LOGW("cannot find cap param (%d)", magicNum);
                        }
                    }
                    //
                    if (it != mRequestQueue.begin()) {
                        String8 str;
                        str += String8::format("MissingCallback from 3A : "
                            "this CB Mnum(%d) ; current ReqQ[%d] = [ ",
                            magicNum, (int)(mRequestQueue.size()));
                        Que_T::iterator it = mRequestQueue.begin();
                        for(; it != mRequestQueue.end(); it++) {
                            str += String8::format("%d ", (*it).getIdx());
                        }
                        str += String8::format("] @ SOF(%d)", sofIdx);
                        MY_LOGW("%s", str.string());
                    }
                    //
                    mRequestQueue.erase(it);
                    exist = true;
                    break;
                }
            }
            P1_ATOMIC_RELEASE;
        }
        if (exist) {
            {
                Mutex::Autolock _ll(mTransferJobLock);
                mTransferJobIdx = job.getIdx();
            }
            //
            if (OK != onProcessEnqueFrame(job)) {
                MY_LOGE("frame en-queue fail (%d)", magicNum);
                for (MUINT8 i = 0; i < job.size(); i++) {
                    onReturnFrame(job.edit(i), FLUSH_FAIL, MTRUE);
                    /* DO NOT use this P1QueAct after onReturnFrame() */
                }
            } else {
                if (//IS_BURST_OFF && // exclude burst mode
                    (job.size() >= 1)) {
                    P1Act act = GET_ACT_PTR(act, job.edit(0), RET_VOID);
                    if ((act->reqType == REQ_TYPE_NORMAL &&
                        act->appFrame != NULL) &&
                        (capParam != NULL && capParam->metadata.count() > 0)) {
                        requestMetadataEarlyCallback(job.edit(0),
                            STREAM_META_OUT_HAL, &(capParam->metadata));
                    }
                }
                //
                P1Act pAct = GET_ACT_PTR(pAct, job.edit(0), RET_VOID);
                if (mpHwStateCtrl != NULL &&
                    pAct->ctrlSensorStatus == SENSOR_STATUS_CTRL_STANDBY) {
                    MBOOL isAct = MFALSE;
                    //isAct = mpHwStateCtrl->checkSetNum(job.getIdx());
                    isAct = mpHwStateCtrl->checkCtrlStandby(pAct->getNum());
                    // it might call doNotifyDropframe() in DRV->suspend()
                    if ((isAct) && ((mpDeliverMgr != NULL) &&
                        (mpDeliverMgr->runningGet()))) {
                        MY_LOGI0("DRV-suspend executed : check drop-frame");
                        onProcessDropFrame(MTRUE);
                    };
                }
            }
            //
            {
                Mutex::Autolock _ll(mTransferJobLock);
                mTransferJobIdx = P1ACT_ID_NULL;
                U_if (mTransferJobWaiting) {
                    mTransferJobCond.broadcast();
                }
            }
        } else {
            //MY_LOGW_IF(magicNum!=0, "no: %d", magicNum);
            #if (IS_P1_LOGI)
            Mutex::Autolock _l(mRequestQueueLock);
            String8 str;
            str += String8::format("[req(%d)/size(%d)]: ",
                magicNum, (int)(mRequestQueue.size()));
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                str += String8::format("%d ", (*it).getIdx());
            }
            MY_LOGI0("%s", str.string());
            #endif
        }
    }
    //
    if (IS_BURST_ON) {
        MBOOL skip = MFALSE;
        if (mpHwStateCtrl != NULL) {
            skip = mpHwStateCtrl->checkSkipSync();
        }
        if (skip) {
            MY_LOGI0("FrameSet was paused");
        } else {
            //onRequestFrameSet(MFALSE);
            setRequest(MFALSE);
        }
    }
    //
    P1_TRACE_C_END(SLG_I); // "P1:onSyncBegin"
    //
    inflightMonitoring(IMT_ENQ);
    //
    FUNCTION_P1_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessEnqueFrame(
    P1QueJob & job
)
{
    FUNCTION_P1_IN;

    //(1)
    //pass request directly if it's a reprocessing one
    //
    //if( mInHalMeta == NULL) {
    //    onDispatchFrame(pFrame);
    //    return;
    //}

    //(2)
    MERROR status = hardwareOps_enque(job);

    FUNCTION_P1_OUT;
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
P1QueJob
P1NodeImp::
getProcessingFrame_ByNumber(MINT32 magicNum)
{
    FUNCTION_P1_IN;
    P1QueJob job(mBurstNum);

    Mutex::Autolock _l(mProcessingQueueLock);
    if (mProcessingQueue.empty()) {
        MY_LOGE("mProcessingQueue is empty");
        return job;
    }

    #if 1
        Que_T::iterator it = mProcessingQueue.begin();
        for (; it != mProcessingQueue.end(); it++) {
            if ((*it).getIdx() == magicNum) {
                break;
            }
        }
        if (it == mProcessingQueue.end()) {
            MY_LOGI0("cannot find the right act for num: %d", magicNum);
            job.clear();
            return job;
        }
        else {
            job = *it;
            mProcessingQueue.erase(it);
            mProcessingQueueCond.broadcast();
        }
    #else
        job = *mProcessingQueue.begin();
        mProcessingQueue.erase(mProcessingQueue.begin());
        mProcessingQueueCond.broadcast();
    #endif
    P1_ATOMIC_RELEASE;
    FUNCTION_P1_OUT;
    //
    return job;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getProcessingFrame_ByAddr(IImageBuffer* const imgBuffer,
                          MINT32 magicNum,
                          P1QueJob & job
)
{
    FUNCTION_P1_IN;

    MBOOL ret = MFALSE;
    if (imgBuffer == NULL) {
        MY_LOGE("imgBuffer == NULL");
        return ret;
    }

    // get the right act from mProcessingQueue
    MINT32 gotNum = 0;
    std::vector<MINT32> vStoreNum; // not reserve since it will not insert in the most case
    vStoreNum.clear();
    {   //mProcessingQueueLock.lock();
        Mutex::Autolock _l(mProcessingQueueLock);
        if (mProcessingQueue.empty()) {
            MY_LOGE("ProQ is empty");
            return ret;
        }
        //
        Que_T::iterator it = mProcessingQueue.begin();
        for (; it != mProcessingQueue.end(); it++) {
            P1Act act = GET_ACT_PTR(act, (*it).edit(0), MFALSE);
            if (imgBuffer ==
                act->streamBufImg[STREAM_IMG_OUT_FULL].spImgBuf.get() ||
                imgBuffer ==
                act->streamBufImg[STREAM_IMG_OUT_OPAQUE].spImgBuf.get() ||
                imgBuffer ==
                act->streamBufImg[STREAM_IMG_OUT_RESIZE].spImgBuf.get() ||
                imgBuffer ==
                act->streamBufImg[STREAM_IMG_OUT_LCS].spImgBuf.get() ||
                imgBuffer ==
                act->streamBufImg[STREAM_IMG_OUT_RSS].spImgBuf.get()) {
                gotNum = (*it).getIdx();
                if ((*it).getIdx() == magicNum) {
                    ret = MTRUE;
                } else {
                    #if SUPPORT_PERFRAME_CTRL
                    MY_LOGE("magicNum from driver(%d), should(%d)",
                           magicNum, (*it).getIdx());
                    #else
                    if((magicNum & 0x40000000) != 0) {
                        MY_LOGW("magicNum from driver(0x%x) is uncertain",
                              magicNum);
                        ret = MFALSE;
                    } else {
                        ret = MTRUE;
                        MY_LOGW("magicNum from driver(%d), should(%d)",
                              magicNum, (*it).getIdx());
                    }
                    #endif
                    // reset act from 3A info
                    for (size_t i = 0; i < (*it).size(); i++) {
                        P1Act pAct = GET_ACT_PTR(pAct, (*it).edit(i), MFALSE);
                        pAct->capType = E_CAPTURE_NORMAL;
                        pAct->frameExpDuration = 0;
                    }
                }
                break;
            } else {
                continue;
            }
        }
        //
        if (it == mProcessingQueue.end()) {
            MY_LOGE("no act with imagebuf(%p), num(%d)",
                     (void*)imgBuffer, magicNum);
            #if 1 // dump ProcessingQ info
            char const * str[STREAM_IMG_NUM] =
                {"YUV-in", "RAW-in", "OPQ", "IMG", "RRZ", "LCS", "RSS"};
            for (Que_T::iterator j = mProcessingQueue.begin();
                j != mProcessingQueue.end(); j++) {
                for (size_t i = 0; i < (*j).size(); i++) {
                    P1Act act = GET_ACT_PTR(act, (*j).edit(i), MFALSE);
                    MY_LOGW("[ProQ] [%zu] : num(%d)", i, act->magicNum);
                    for (int s = STREAM_ITEM_START; s < STREAM_IMG_NUM; s++) {
                        if (act->streamBufImg[s].bExist &&
                            act->streamBufImg[s].spImgBuf != NULL) {
                            sp<IImageBuffer> pBuf =
                                act->streamBufImg[s].spImgBuf;
                            MY_LOGW("[ProQ] [%zu] : %s(%p)(P:%p)(V:%p)",
                                i, ((str[s] != NULL) ? str[s] : "UNKNOWN"),
                                (void*)pBuf.get(), (void*)pBuf->getBufPA(0),
                                (void*)pBuf->getBufVA(0));
                        };
                    }
                }
            }
            #endif
        } else {
            //
            if (it != mProcessingQueue.begin()) {
                size_t queSize = mProcessingQueue.size();
                MINT queNum = 0;
                Que_T::iterator it_stored = mProcessingQueue.begin();
                for (; it_stored < it; it_stored ++) {
                    for (size_t i = 0; i < (*it_stored).size(); i++) {
                        queNum = ((*it_stored).edit(i).getNum());
                        vStoreNum.push_back(queNum);
                        MY_LOGI0("Non-Dequeued frame(Mnum:%d) in ProcQue[%zu] "
                            "current(%d)", queNum, queSize, gotNum);
                    }
                }
            }
            job = *it;
            mProcessingQueue.erase(it);
            mProcessingQueueCond.broadcast();
            MY_LOGD2("magic: %d", magicNum);
        }
        P1_ATOMIC_RELEASE;
    }   //mProcessingQueueLock.unlock();
    // avoid to execute mpHwStateCtrl functions under mProcessingQueueLock
    MBOOL isPauseDrop = MFALSE;
    if (mpHwStateCtrl != NULL) {
        U_if (gotNum > 0 && mpHwStateCtrl->checkDoneNum(gotNum)) {
            for (size_t idx = 0; idx < vStoreNum.size(); idx++) {
                MY_LOGI0("DropStoreNum[%zu] : %d", idx, vStoreNum[idx]);
                mpHwStateCtrl->setDropNum(vStoreNum[idx]);
            }
            isPauseDrop = MTRUE;
        }
    }
    U_if ((!vStoreNum.empty()) && (!isPauseDrop)) {
        size_t nSize = vStoreNum.size();
        if (nSize > 0 && (vStoreNum[0] + P1NODE_DEF_PROCESS_DEPTH) < gotNum) {
            MY_LOGW("[De-queued Frame Skipped] NonDequeuedFrameCount[%zu]:(%d)"
                " - CurrentDequeuedFrameMnum(%d)"
                " - Please Check the DRV Dequeue/Drop Flow",
                nSize, vStoreNum[0], gotNum);
        }
        for (size_t idx = 0; idx < nSize; idx++) {
            MY_LOGI0("NonDequeued[%zu/%zu] = FrameMnum(%d) - current(%d)",
                idx, nSize, vStoreNum[idx], gotNum);
        }
    }
    //
    FUNCTION_P1_OUT;
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onCheckDropFrame(void)
{
    MUINT cnt = 0;
    MINT32 num = 0;
    if (mpHwStateCtrl != NULL) {
        do
        {
            num = mpHwStateCtrl->getDropNum();
            if (num > 0) {
                Mutex::Autolock _l(mDropQueueLock);
                mDropQueue.push_back(num);
                cnt ++;
            }
        } while (num > 0);
    }
    if ((cnt > 0) && (mpDeliverMgr != NULL) && (mpDeliverMgr->runningGet())) {
        MY_LOGI0("check drop frame (%d)", cnt);
        onProcessDropFrame(MTRUE);
    };
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDropFrame(MBOOL isTrigger)
{
    mDropQueueLock.lock();
    if (mDropQueue.empty()) {
        mDropQueueLock.unlock();
        return OK;
    }
    std::vector< P1QueAct > actQ; // not reserve since it will not insert in the most case
    for(size_t i = 0; i < mDropQueue.size(); i++) {
        P1QueJob job = getProcessingFrame_ByNumber(mDropQueue[i]);
        // if getProcessingFrame_ByNumber can not find the job
        // the job set size is 0
        for (MUINT8 i = 0; i < job.size(); i++) {
            P1QueAct act = job.edit(i);
            actQ.push_back(act);
        }
        MY_LOGI0("drop[%zu/%zu]: %d", i, mDropQueue.size(), mDropQueue[i]);
        P1_LOGI(0, "DropQueue[%zu/%zu] = %d",
            i, mDropQueue.size(), mDropQueue[i]);
    }
    mDropQueue.clear();
    mDropQueueLock.unlock();
    //
    for(size_t i = 0; i < actQ.size(); i++) {
        //
        P1Act pAct = GET_ACT_PTR(pAct, actQ.at(i), BAD_VALUE);
        if (IS_BURST_OFF) {
            mLongExp.reset(pAct->magicNum);
        }
        //
        if (IS_LMV(mpConnectLMV) && (pAct->buffer_eiso != NULL) &&
            isActive()) {
            mpConnectLMV->processDropFrame(pAct->buffer_eiso);
        }
        pAct->exeState = EXE_STATE_DONE;
        onReturnFrame(actQ.at(i), FLUSH_DROP,
            ((isTrigger) && (i == (actQ.size() - 1))) ? MTRUE : MFALSE);
        /* DO NOT use this P1QueAct after onReturnFrame() */
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDequeFrame()
{

#if 0
    // [FIXME]  temp-WA for DRV currently not implement self-signal
    //          the dequeue might be blocked while en-queue empty
    //          it should be removed after DRV self-signal ready
    {
        Mutex::Autolock _ll(mProcessingQueueLock);
        if (mProcessingQueue.empty()) {
            return OK;
        }
    }
#endif

    FUNCTION_P1_IN;

    //P1_TRACE_AUTO(SLG_O, "P1:ProcessDequedFrame");

    MERROR ret= OK;
    QBufInfo deqBuf;
    if(hardwareOps_deque(deqBuf) != OK) {
        return BAD_VALUE;
    }

    if (deqBuf.mvOut.size() == 0) {
        MBOOL normal_case = (!isActive());
        if ((!normal_case) && (mpHwStateCtrl != NULL)) {
            normal_case = (mpHwStateCtrl->checkBufferState());
        }
        MY_LOGI0("DeqBuf Out Size is 0 (act:%d,%d)", isActive(), normal_case);
        return ((normal_case) ? OK : BAD_VALUE);
    }

    MY_LOGI2("HwLockProcessWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI2("HwLockProcessWait ---");

    P1QueJob job(mBurstNum);
    MBOOL match = getProcessingFrame_ByAddr(deqBuf.mvOut[0].mBuffer,
                    deqBuf.mvOut[0].mMetaData.mMagicNum_hal, job);
    {
        Mutex::Autolock _ll(mTransferJobLock);
        status_t res = NO_ERROR;
        MUINT32 needRetry = P1NODE_TRANSFER_JOB_WAIT_CNT_MAX;
        while ((match) && (mTransferJobIdx != P1ACT_ID_NULL) &&
            (mTransferJobIdx == job.getIdx()) && (needRetry != 0)) {
            mTransferJobWaiting = MTRUE;
            res = mTransferJobCond.waitRelative(mTransferJobLock,
                P1NODE_TRANSFER_JOB_WAIT_INV_NS);
            needRetry --;
            MY_LOGI0("TransferJob(%d) ThisJob(%d) - WaitStatus(%d) "
                "NeedRetry(%d)", mTransferJobIdx, job.getIdx(), res, needRetry);
            if (res == NO_ERROR) {
                MY_LOGI0("Got Job");
                break;
            }
        }
        mTransferJobWaiting = MFALSE;
        if ((res != NO_ERROR) && (mTransferJobIdx == job.getIdx())) {
            MY_LOGE("TransferJob(%d) Not-Ready : (%d)", mTransferJobIdx, res);
        }
    }
    onCheckDropFrame(); // must call after getProcessingFrame_ByAddr()
    //
    if (IS_BURST_OFF) {
        mLongExp.reset(deqBuf.mvOut[0].mMetaData.mMagicNum_hal);
    }
    //
    if (!findPortBufIndex(deqBuf, job)) {
        return BAD_VALUE;
    }
    //
    for (MUINT8 i = 0; i < (MUINT8)job.size(); i++) {
        P1QueAct qAct = job.edit(i);
        P1Act act = GET_ACT_PTR(act, qAct, BAD_VALUE);
        MetaSet_T result3A;
        // camera display systrace - DeQ
        if  (act->appFrame != NULL) {
            MINT64 const timestamp = deqBuf.mvOut[i].mMetaData.mTimeStamp;
            P1_TRACE_F_BEGIN(SLG_B, // add information
                "Cam:%d:IspP1:deq|timestamp(ns):%" PRId64
                " duration(ns):%" PRId64
                " request:%d frame:%d",
                getOpenId(), timestamp, ::systemTime() - timestamp,
                act->appFrame->getRequestNo(), act->appFrame->getFrameNo()
            );
            P1_TRACE_C_END(SLG_B); // "IspP1:deq"
            /*
            MINT64 const timestamp = deqBuf.mvOut[i].mMetaData.mTimeStamp;
            String8 const str = String8::format(
                "Cam:%d:IspP1:deq|timestamp(ns):%" PRId64
                " duration(ns):%" PRId64
                " request:%d frame:%d",
                getOpenId(), timestamp, ::systemTime()-timestamp,
                act->appFrame->getRequestNo(), act->appFrame->getFrameNo()
            );
            CAM_TRACE_BEGIN(str.string());
            CAM_TRACE_END();
            */
        }

        MY_LOGD2("job(%d)[%d] = act(%d)",
            job.getIdx(), i, act->magicNum);
        mTagDeq.set(qAct.getNum());
        #if SUPPORT_3A
        {
            Mutex::Autolock _ssl(mStopSttLock);
            if (isActive() && mp3A && act->reqType == REQ_TYPE_NORMAL) {
                MBOOL drop_notify = MFALSE;
                MINT32 ret = 0;
                mp3A->notifyP1Done(act->magicNum);
                if (match && act->capType == E_CAPTURE_HIGH_QUALITY_CAPTURE) {
                    P1_TRACE_F_BEGIN(SLG_I, "P1:3A-getCur|"
                        "Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
                        act->magicNum, act->sofIdx, act->frmNum, act->reqNum);
                    MY_LOGD1("mp3A->getCur(%d) +++", act->magicNum);
                    ret = mp3A->getCur(act->magicNum, result3A);
                    if (ret < 0) { // 0:success
                        drop_notify = MTRUE;
                        MY_LOGI0("drop-frame by 3A GetC(%d) @ (%d)(%d:%d)", ret,
                            act->magicNum, act->frmNum, act->reqNum);
                    }
                    MY_LOGD1("mp3A->getCur(%d) ---", act->magicNum);
                    P1_TRACE_C_END(SLG_I); // "P1:3A-getCur"
                } else {
                    P1_TRACE_F_BEGIN(SLG_I, "P1:3A-get|"
                        "Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
                        act->magicNum, act->sofIdx, act->frmNum, act->reqNum);
                    MY_LOGD2("mp3A->get(%d) +++", act->magicNum);
                    ret = mp3A->get(act->magicNum, result3A);
                    if (ret < 0) { // 0:success
                        drop_notify = MTRUE;
                        MY_LOGI0("drop-frame by 3A Get(%d) @ (%d)(%d:%d)", ret,
                            act->magicNum, act->frmNum, act->reqNum);
                    }
                    MY_LOGD2("mp3A->get(%d) ---", act->magicNum);
                    P1_TRACE_C_END(SLG_I); // "P1:3A-get"
                }
                if (LOGI_LV1) {
                    act->metaCntAaaAPP = result3A.appMeta.count();
                    act->metaCntAaaHAL = result3A.halMeta.count();
                }
                P1_LOG_META(*act, &(result3A.appMeta),
                    "3A.Get-APP");
                P1_LOG_META(*act, &(result3A.halMeta),
                    "3A.Get-HAL");
                if (!match) {
                    act->setFlush(FLUSH_MIS_BUFFER);
                }
                if (drop_notify) {
                    act->setFlush(FLUSH_MIS_RESULT);
                    match = MFALSE;
                }
            }
        }
        #endif

        // check the ReqExpRec
        if (match && (act->expRec != EXP_REC_NONE)) {
            switch (act->reqType) {
                case REQ_TYPE_NORMAL:
                //case REQ_TYPE_REDO:
                //case REQ_TYPE_YUV:
                    #if 0 // flush this frame
                    act->setFlush(FLUSH_MIS_EXP);
                    match = MFALSE;
                    #endif
                    MY_LOGI0("check ExpRec " P1INFO_ACT_STR,
                        P1INFO_ACT_VAR(*act));
                    break;
                default: // REQ_TYPE_INITIAL/REQ_TYPE_DUMMY/REQ_TYPE_PADDING
                    MY_LOGI2("ExpRec " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                    break;
            }
        }
        // check the result of raw type
        if (match) {
            MUINT32 port_index = act->portBufIndex[P1_OUTPUT_PORT_IMGO];
            if (port_index != P1_PORT_BUF_IDX_NONE) {
                MBOOL raw_match = MTRUE;
                MUINT32 res_raw = deqBuf.mvOut[port_index].mMetaData.mRawType;
                MINT64 set_raw = (res_raw == (MUINT32)EPipe_PROCESSED_RAW) ?
                    (MINT64)(eIMAGE_DESC_RAW_TYPE_PROCESSED) :
                    (MINT64)(eIMAGE_DESC_RAW_TYPE_PURE);
                if ((/*IS_OUT(REQ_OUT_FULL_PROC, act->reqOutSet) ||*/
                    (act->fullRawType == EPipe_PROCESSED_RAW)) &&
                    res_raw != (MUINT32)EPipe_PROCESSED_RAW) {
                    raw_match = MFALSE;
                    // only check Processed-Raw with (mRawType == EPipe_PROCESSED_RAW)
                    // in Pure-Raw, it would be EPipe_PURE_RAW or others
                }
                //MY_LOGD2("raw match: %d", raw_match);
                if (!raw_match) {
                    MY_LOGE("RawType mismatch DEQ(%d) REQ(%d)" P1INFO_ACT_STR,
                        res_raw, act->fullRawType, P1INFO_ACT_VAR(*act));
                    #if 1 // flush this frame
                    act->setFlush(FLUSH_MIS_RAW);
                    match = MFALSE;
                    #endif
                } else {
                    sp<IImageBuffer> pBuf = deqBuf.mvOut[port_index].mBuffer;
                    if (pBuf != NULL) {
                        MBOOL res = pBuf->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE,
                            set_raw, MTRUE);
                        MY_LOGD3("ImgBufRawType(%" PRId64 ") %d", set_raw, res);
                        /*
                        MINT64 get_raw = 0;
                        MBOOL ret = pBuf->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, get_raw);
                        MY_LOGD3("ImgBufRawTypeG(%" PRId64 ") %d", get_raw, ret);
                        */
                    }
                }
            }
        }
        //
        #if 0
        if (act->magicNum > 0 && act->magicNum < 15) {
            printf("[%d]act->img_resizer.get() = [%p]\n",
                act->magicNum, act->img_resizer.get());
            if (act->img_resizer.get() != NULL) {
                char filename[256] = {0};
                sprintf(filename, "/sdcard/raw/P1B_%d_%dx%d.raw",
                    act->magicNum,
                    act->img_resizer->getImgSize().w,
                    act->img_resizer->getImgSize().h
                    );
                printf("SAVE BUF [%s]\n", filename);
                act->img_resizer->saveToFile(filename);
            }
        }
        #endif
        act->frameTimeStamp = deqBuf.mvOut[i].mMetaData.mTimeStamp;
        act->frameTimeStampBoot = deqBuf.mvOut[i].mMetaData.mTimeStamp_B;
        act->exeState = EXE_STATE_DONE;
        act->isReadoutReady = MTRUE;

        if (LOGI_LV1) {
            MUINT32 index  = i;
            android::String8 strInfo("");
            strInfo += String8::format("[P1::DEQ]" P1INFO_ACT_STR
                " job(%d/%d) ", P1INFO_ACT_VAR(*act), index, mBurstNum);
            for (size_t n = index; n < deqBuf.mvOut.size(); n += mBurstNum) {
                if (deqBuf.mvOut[n].mPortID.index == PORT_IMGO.index) {
                    strInfo += String8::format("IMG(%s) ", (deqBuf.mvOut[n].
                        mMetaData.mRawType == EPipe_PROCESSED_RAW) ?
                        "proc" : "pure");
                } else if (deqBuf.mvOut[n].mPortID.index == PORT_RRZO.index) {
                    MRect crop_s = deqBuf.mvOut[n].mMetaData.mCrop_s;
                    MRect crop_d = deqBuf.mvOut[n].mMetaData.mCrop_d;
                    MSize size_d = deqBuf.mvOut[n].mMetaData.mDstSize;
                    strInfo += String8::format(
                        "RRZ%d(%d-%d-%dx%d)(%d-%d-%dx%d)(%dx%d) ", mIsBinEn,
                        crop_s.p.x, crop_s.p.y, crop_s.s.w, crop_s.s.h,
                        crop_d.p.x, crop_d.p.y, crop_d.s.w, crop_d.s.h,
                        size_d.w, size_d.h);
               }
            }
            strInfo += String8::format("T-ns(EXP: %" PRId64 ")(Src:%d)"
                "(SOF: m_%" PRId64 " b_%" PRId64 ")(SS: %" PRId64 ") ",
                act->frameExpDuration, mTimestampSrc, act->frameTimeStamp,
                act->frameTimeStampBoot, (((mTimestampSrc ==
                MTK_SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME) &&
                (act->frameTimeStampBoot != 0)) ?
                (act->frameTimeStampBoot - act->frameExpDuration) :
                ((act->frameTimeStamp != 0) ?
                (act->frameTimeStamp - act->frameExpDuration) : (0))));
            act->res.clear();
            act->res += strInfo;
        };

        if (!match || act->getType() == ACT_TYPE_INTERNAL || !isActive()) {
            FLUSH_TYPE type = FLUSH_MIS_UNCERTAIN;
            if (!act->getFlush()) { // if flush type did not set
                if (act->getType() == ACT_TYPE_INTERNAL) {
                    if (act->reqType == REQ_TYPE_INITIAL) {
                        type = FLUSH_INITIAL;
                    } else if (act->reqType == REQ_TYPE_PADDING) {
                        type = FLUSH_PADDING;
                    } else if (act->reqType == REQ_TYPE_DUMMY) {
                        type = FLUSH_DUMMY;
                    } else {
                        type = FLUSH_MIS_UNCERTAIN;
                    }
                } else {
                    type = FLUSH_INACTIVE;
                }
            }
            onReturnFrame(qAct, type, MTRUE);
            /* DO NOT use this P1QueAct after onReturnFrame() */
            ret = BAD_VALUE;
        } else {
            IMetadata resultAppend;
            IMetadata inAPP,inHAL;
            //
            if (IS_LMV(mpConnectLMV) && mp3A != NULL) {
                MBOOL enEIS = IS_PORT(CONFIG_PORT_EISO, mConfigPort);
                MBOOL enRRZ = IS_PORT(CONFIG_PORT_RRZO, mConfigPort);
                MUINT32 idxEIS = act->portBufIndex[P1_OUTPUT_PORT_EISO];
                MUINT32 idxRRZ = act->portBufIndex[P1_OUTPUT_PORT_RRZO];
                if (
                    #if 0
                    ((enEIS && idxEIS != P1_PORT_BUF_IDX_NONE) ||
                    (enRRZ && idxRRZ != P1_PORT_BUF_IDX_NONE)) &&
                    #endif
                    OK == act->frameMetadataGet(STREAM_META_IN_APP, &inAPP) &&
                    OK == act->frameMetadataGet(STREAM_META_IN_HAL, &inHAL)) {
                    MBOOL bIsBinEn = (act->refBinSize == mSensorParams.size) ?
                        MFALSE : MTRUE;
                    mpConnectLMV->processResult(bIsBinEn, enEIS, enRRZ,
                        &inAPP, &inHAL, result3A, mp3A, act->magicNum,
                        act->sofIdx, mLastSofIdx, act->uniSwitchState,
                        deqBuf, idxEIS, idxRRZ, resultAppend);
                } else {
                    //MY_LOGI0("not execute LMV process");
                };
            };
            //
            #if 1 // for RSSO update buffer
            if (IS_OUT(REQ_OUT_RSSO, act->reqOutSet) &&
                (!IS_EXP(EXP_EVT_NOBUF_RSSO, act->expRec))) {
                MUINT32 port_index = act->portBufIndex[P1_OUTPUT_PORT_RSSO];
                sp<IImageBuffer> spImgBuf =
                    act->streamBufImg[STREAM_IMG_OUT_RSS].spImgBuf;
                if (port_index != P1_PORT_BUF_IDX_NONE && spImgBuf != NULL) {
                    MSize size = deqBuf.mvOut[port_index].mMetaData.mDstSize;
                    MY_LOGD3("RSSO data size (%dx%d)", size.w, size.h);

                    IMetadata::IEntry entry(MTK_P1NODE_RSS_SIZE);
                    entry.push_back(size, Type2Type< MSize >());
                    resultAppend.update(MTK_P1NODE_RSS_SIZE, entry);
                }
            }
            #endif

            #if SUPPORT_FSC
            if (mpFSC != NULL && mp3A != NULL) {
                MUINT32 idxRSS = P1_PORT_BUF_IDX_NONE;
                MUINT32 idxRRZ = act->portBufIndex[P1_OUTPUT_PORT_RRZO];
                if (IS_OUT(REQ_OUT_RSSO, act->reqOutSet) && (!IS_EXP(EXP_EVT_NOBUF_RSSO, act->expRec))
                    && act->streamBufImg[STREAM_IMG_OUT_RSS].spImgBuf != NULL) {
                    idxRSS = act->portBufIndex[P1_OUTPUT_PORT_RSSO];
                }
                if (
                    OK == act->frameMetadataGet(STREAM_META_IN_APP, &inAPP) &&
                    OK == act->frameMetadataGet(STREAM_META_IN_HAL, &inHAL)) {
                    MBOOL bIsBinEn = (act->refBinSize == mSensorParams.size) ?
                        MFALSE : MTRUE;
                    mpFSC->processResult(bIsBinEn, &inAPP, &inHAL, result3A, mp3A, act->magicNum,
                    deqBuf, idxRSS, idxRRZ, i, resultAppend);// get FSC resultAppend
                } else {
                    //MY_LOGI0("not execute LMV process");
                };
            }
            #endif
            mLastSofIdx = act->sofIdx;
            onProcessResult(qAct, deqBuf, result3A, resultAppend, i);
            /* DO NOT use this P1QueAct/P1Act after onProcessResult() */
            ret = OK;
        }
    }
    //
    if (IS_PORT(CONFIG_PORT_EISO, mConfigPort) && isActive()) {
        if (IS_LMV(mpConnectLMV)) {
            mpConnectLMV->processDequeFrame(deqBuf);
        }
    }

    // help to trigger GyroCollector.
    NSCam::Utils::GyroCollector::trigger();

    //
    inflightMonitoring(IMT_DEQ);

    FUNCTION_P1_OUT;

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onHandleFlush(
    MBOOL wait,
    MBOOL isNextOff
)
{
    FUNCTION_P1_IN;
    P1_TRACE_S_BEGIN(SLG_S, "P1:HandleFlush");

    P1_ATOMIC_ACQUIRE;

    MY_LOGI1("HW StartCap(%d) InitReq(%d) NeedReady(%d) NextOff(%d)",
        EN_START_CAP_RUN, EN_INIT_REQ_RUN, mNeedHwReady, isNextOff);
    if (EN_START_CAP_RUN || EN_INIT_REQ_RUN) {
        U_if (mNeedHwReady) {
            MY_LOGI0("HW ready +++");
            MERROR err = hardwareOps_ready();
            if (mpConCtrl != NULL && mpConCtrl->getAidUsage() == MTRUE) {
                mpConCtrl->cleanAidStage();
            }
            if (mpTimingCheckerMgr != NULL) {
                mpTimingCheckerMgr->setEnable(MFALSE);
            }
            if (err != OK) {
                MY_LOGE("HW ready fail (%d)", err);
            }
            MY_LOGI0("HW ready ---");
        }
    }

    //wake up cb thread.
    {
        Mutex::Autolock _l(mStartLock);
        mStartCond.broadcast();
    }

    //stop hardware
    L_if (!wait) {
        hardwareOps_stop(); //include hardware and 3A
    }

    //check the state
    U_if (!mFirstReceived) {
        if (EN_START_CAP_RUN) {
            MY_LOGI0("No-Ready-Request-Arrival in start capture flow - "
                "CamCfgExp(%d) - enableCaptureFlow(%d)",
                mCamCfgExp, mEnableCaptureFlow);
            mLogInfo.inspect(LogInfo::IT_STOP_NO_REQ_IN_CAPTURE);
        }
        if (EN_INIT_REQ_RUN) {
            MY_LOGI0("No-Ready-Request-Arrival in initial request flow - "
                "CamCfgExp(%d) - "
                "initRequest(%d) : ReceivedCnt(%d) < RequiredNum(%d)",
                mCamCfgExp, mInitReqSet, mInitReqCnt, mInitReqNum);
            mLogInfo.inspect(LogInfo::IT_STOP_NO_REQ_IN_REQUEST);
        }
        if ((!EN_START_CAP_RUN) && (!EN_INIT_REQ_RUN)) {
            MY_LOGI0("No-Ready-Request-Arrival in normal flow - CamCfgExp(%d)",
                mCamCfgExp);
            mLogInfo.inspect(LogInfo::IT_STOP_NO_REQ_IN_GENERAL);
        }
    }

    { // by flush() or uninit() or eventStreamingOff()
        if (EN_INIT_REQ_CFG) { // in flush() -> queue() flow, for fast switching, it will run InitReq flow again,
            mInitReqNum = mInitReqSet * mBurstNum; // exclude eventStreamingOn case (by mInitReqOff = MTRUE)
            mInitReqCnt = 0;
            mInitReqOff = isNextOff;
            if (!EN_INIT_REQ_RUN) {
                MY_LOGI0("Disable - InitReq Set:%d Num:%d Cnt:%d Off:%d",
                    mInitReqSet, mInitReqNum, mInitReqCnt, mInitReqOff);
            }
        }
        if (EN_START_CAP_CFG) { // in flush() -> queue() flow, for fast switching, it will run StartCap flow again,
            mEnableCaptureOff = isNextOff; // exclude eventStreamingOn case (by mEnableCaptureOff = MTRUE)
            if (!EN_START_CAP_RUN) {
                MY_LOGI0("Disable - StartCap Set:%d Off:%d",
                    mEnableCaptureFlow, mEnableCaptureOff);
            }
        }
    }

    // clear drop frame queue
    onProcessDropFrame();

    // clear collector
    {
        mpTaskCtrl->sessionLock();
        MINT cnt = mpTaskCollector->remainder();
        while (cnt > 0) {
            P1QueAct qAct;
            cnt = mpTaskCollector->requireAct(qAct);
            if (qAct.id() > P1ACT_ID_NULL) {
                onReturnFrame(qAct, FLUSH_COLLECTOR, MFALSE);
                /* DO NOT use this P1QueAct after onReturnFrame() */
            }
        }
        mTagList.set(cnt);
        mpTaskCtrl->sessionUnLock();
    }

    // clear request queue
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //P1_LOGD("Check-RQ (%d)", mRequestQueue.size());
        while(!mRequestQueue.empty()) {
            P1QueJob job = *mRequestQueue.begin();
            mRequestQueue.erase(mRequestQueue.begin());
            for (MUINT8 i = 0; i < job.size(); i++) {
                P1QueAct qAct = job.edit(i);
                onReturnFrame(qAct, FLUSH_REQUESTQ, MFALSE);
                /* DO NOT use this P1QueAct after onReturnFrame() */
            }
        }
        P1_ATOMIC_RELEASE;
    }

    // clear processing queue
    //     wait until processing frame coming out
    U_if (wait) {
        Mutex::Autolock _l(mProcessingQueueLock);
        while(!mProcessingQueue.empty()) {
            mProcessingQueueCond.wait(mProcessingQueueLock);
        }
    } else {
        // must guarantee hardware has been stopped.
        Mutex::Autolock _l(mProcessingQueueLock);
        //P1_LOGD("Check-PQ (%d)", mProcessingQueue.size());
        while(!mProcessingQueue.empty()) {
            P1QueJob job = *mProcessingQueue.begin();
            mProcessingQueue.erase(mProcessingQueue.begin());
            for (MUINT8 i = 0; i < job.size(); i++) {
                P1QueAct qAct = job.edit(i);
                onReturnFrame(qAct, FLUSH_PROCESSQ, MFALSE);
                /* DO NOT use this P1QueAct after onReturnFrame() */
            }
        }
        P1_ATOMIC_RELEASE;
    }

    U_if (mpDeliverMgr != NULL && !mpDeliverMgr->waitFlush(MTRUE)) {
        MY_LOGW("request not done");
    };

    // clear all
    {
        Mutex::Autolock _l(mRequestQueueLock);
        size_t queReqSize = mRequestQueue.size();
        U_if(queReqSize > 0) {
            MY_LOGW("RequestQueue[%zu] not clear ready", queReqSize);
        }
        mRequestQueue.clear(); //suppose already clear
        P1_ATOMIC_RELEASE;
    }
    {
        Mutex::Autolock _l(mProcessingQueueLock);
        size_t queProSize = mProcessingQueue.size();
        U_if(queProSize > 0) {
            MY_LOGW("ProcessingQueue[%zu] not clear ready", queProSize);
        }
        mProcessingQueue.clear(); //suppose already clear
        P1_ATOMIC_RELEASE;
    }
    setLaunchState(NSP1Node::LAUNCH_STATE_IDLE);
    //mImageStorage.uninit();
    //m3AStorage.clear();
    mpTaskCtrl->reset();
    mLastNum = P1_MAGIC_NUM_FIRST;

    P1_TRACE_C_END(SLG_S); // "P1:HandleFlush"
    FUNCTION_P1_OUT;
}


#if 0
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcess3AResult(
    MUINT32 magicNum,
    MUINT32 key,
    MUINT32 val
)
{
    MY_LOGD2("%d", magicNum);

    if(magicNum == 0) return;

    m3AStorage.add(magicNum, key, val);
    if(m3AStorage.isCompleted(magicNum)) {
        sp<IPipelineFrame> spFrame = m3AStorage.valueFor(magicNum).spFrame;
        StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
        IMetadata appMetadata = m3AStorage.valueFor(magicNum).resultVal;
        lock_and_returnMetadata(spFrame, streamId_OutAppMeta, appMetadata);
        m3AStorage.removeItem(magicNum);

        IStreamBufferSet& rStreamBufferSet  = spFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());
    }
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyCb(
    MINT32  _msgType,
    MINTPTR _ext1,
    MINTPTR _ext2,
    MINTPTR _ext3
)
{
    FUNCTION_P1_IN;
    //
    if (_msgType == IHal3ACb::eID_NOTIFY_3APROC_FINISH) {
        MINT32 magicNum = P1_MAGIC_NUM_NULL;
        RequestSet_T* pReqSet = (RequestSet_T*)(_ext1);
        if (pReqSet != NULL && pReqSet->vNumberSet.size() > 0) {
            magicNum = pReqSet->vNumberSet[0];
        }
        MUINT32 sofIdx = (MUINT32)(_ext2);
        mLogInfo.setMemo(LogInfo::CP_CB_PROC_REV, _msgType, magicNum, sofIdx);
    } else if (_msgType == IHal3ACb::eID_NOTIFY_VSYNC_DONE) {
        mLogInfo.setMemo(LogInfo::CP_CB_SYNC_REV, _msgType);
    }
    MY_LOGD2("P1 doNotifyCb(%d) %zd %zd %zd", _msgType, _ext1, _ext2, _ext3);
    //
    U_if (!isActive()) {
        MY_LOGI0("not-active-return");
        if (_msgType == IHal3ACb::eID_NOTIFY_3APROC_FINISH) {
            mLogInfo.setMemo(LogInfo::CP_CB_PROC_RET, _msgType, MTRUE);
        } else if (_msgType == IHal3ACb::eID_NOTIFY_VSYNC_DONE) {
            mLogInfo.setMemo(LogInfo::CP_CB_SYNC_RET, _msgType, MTRUE);
        }
        return;
    }
    switch(_msgType)
    {
        case IHal3ACb::eID_NOTIFY_3APROC_FINISH:
            if (_ext3 == 0) {
                MY_LOGE("CapParam NULL (%d) %zd %zd", _msgType, _ext1, _ext2);
            } else {
                RequestSet_T set = *(RequestSet_T*)(_ext1);
                CapParam_T param = *(CapParam_T*)(_ext3);
                onSyncBegin(MFALSE, &set, (MUINT32)_ext2, &param);
            }
            mLogInfo.setMemo(LogInfo::CP_CB_PROC_RET, _msgType, MFALSE);
            break;
        case IHal3ACb::eID_NOTIFY_CURR_RESULT:
            //onProcess3AResult((MUINT32)_ext1,(MUINT32)_ext2, (MUINT32)_ext3); //magic, key, val
            break;
        case IHal3ACb::eID_NOTIFY_VSYNC_DONE:
            onSyncEnd();
            mLogInfo.setMemo(LogInfo::CP_CB_SYNC_RET, _msgType, MFALSE);
            break;
        default:
            break;
    }
    //
    FUNCTION_P1_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyDropframe(MUINT magicNum, void* cookie)
{
    MY_LOGI("notify drop frame (%d)", magicNum);

    if (cookie == NULL) {
       MY_LOGE("return cookie is NULL");
       return;
    }
    MINT32 mSysLevel = reinterpret_cast<P1NodeImp*>(cookie)->mSysLevel;
    P1_TRACE_F_BEGIN(SLG_E, "P1:DRV-drop(%d)", magicNum);

    {
        Mutex::Autolock _l(reinterpret_cast<P1NodeImp*>(cookie)->mDropQueueLock);
        reinterpret_cast<P1NodeImp*>(cookie)->mDropQueue.push_back(magicNum);
        MY_LOGI("[Cam::%d] receive drop frame (%d)",
            reinterpret_cast<P1NodeImp*>(cookie)->getOpenId(), magicNum);
    }

    if ((reinterpret_cast<P1NodeImp*>(cookie)->mpDeliverMgr != NULL) &&
        (reinterpret_cast<P1NodeImp*>(cookie)->mpDeliverMgr->runningGet())) {
        MY_LOGI("[Cam::%d] process drop frame (%d)",
            reinterpret_cast<P1NodeImp*>(cookie)->getOpenId(), magicNum);
        #if 0 // to simplify the drop callback function
        reinterpret_cast<P1NodeImp*>(cookie)->onProcessDropFrame(MTRUE);
        #else
        reinterpret_cast<P1NodeImp*>(cookie)->mpDeliverMgr->trigger();
        #endif
    };
    P1_TRACE_C_END(SLG_E); // "P1:DRV-drop"
}

#if (USING_DRV_IO_PIPE_EVENT)
/******************************************************************************
 *
 ******************************************************************************/
NSCam::NSIoPipe::IoPipeEventCtrl
P1NodeImp::
onEvtCtrlAcquiring(P1NodeImp * user,
    NSCam::NSIoPipe::IpRawP1AcquiringEvent & evt)
{
    U_if (user == NULL) {
        MY_LOGW("user is NULL");
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
            NSCam::NSIoPipe::IoPipeEvent::RESULT_ERROR);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }
    Mutex::Autolock _l(user->mIoPipeEvtOpLock);
    U_if (user->mIoPipeEvtOpLeaving) {
        MY_LOGI("[Cam::%d] IoPipeEvtOpLeaving return", user->mOpenId);
        return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
    }
    U_if (user->mIoPipeEvtOpAcquired == MTRUE) {
        MY_LOGI("[Cam::%d] IoPipeEvtOpAcquired:1 return", user->mOpenId);
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
            NSCam::NSIoPipe::IoPipeEvent::RESULT_REJECT);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }
    user->eventStreamingOff();
    user->mIoPipeEvtOpAcquired = MTRUE;
    return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
};

/******************************************************************************
 *
 ******************************************************************************/
NSCam::NSIoPipe::IoPipeEventCtrl
P1NodeImp::
onEvtCtrlReleasing(P1NodeImp * user,
    NSCam::NSIoPipe::IpRawP1ReleasedEvent & evt)
{
    U_if (user == NULL) {
        MY_LOGW("user is NULL");
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
            NSCam::NSIoPipe::IoPipeEvent::RESULT_ERROR);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }
    Mutex::Autolock _l(user->mIoPipeEvtOpLock);
    U_if (user->mIoPipeEvtOpLeaving) {
        MY_LOGI("[Cam::%d] IoPipeEvtOpLeaving return", user->mOpenId);
        return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
    }
    U_if (user->mIoPipeEvtOpAcquired == MFALSE) {
        MY_LOGI("[Cam::%d] IoPipeEvtOpAcquired:0 return", user->mOpenId);
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
            NSCam::NSIoPipe::IoPipeEvent::RESULT_REJECT);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }
    user->eventStreamingOn();
    user->mIoPipeEvtOpAcquired = MFALSE;
    return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
};
#endif

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
createStuffBuffer(sp<IImageBuffer> & imageBuffer,
    sp<IImageStreamInfo> const& streamInfo,
    NSCam::MSize::value_type const changeHeight)
{
    std::vector<MUINT32> vStride;
    vStride.clear();
    vStride.reserve(streamInfo->getBufPlanes().size());
    for (size_t i = 0; i < streamInfo->getBufPlanes().size(); i++) {
        vStride.push_back((MUINT32)
            (streamInfo->getBufPlanes()[i].rowStrideInBytes));
    }
    //for (size_t v = 0; v < vStride.size(); v++) MY_LOGI0("stride[%zu/%zu]=%d", v, vStride.size(), vStride[v]);
    //
    MSize size = streamInfo->getImgSize();
    // change the height while changeHeight > 0
    if (changeHeight > 0) {
        size.h = changeHeight;
    }
    //
    return createStuffBuffer(imageBuffer, streamInfo->getStreamName(),
        streamInfo->getImgFormat(), size, vStride, streamInfo->getSecureInfo());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
createStuffBuffer(sp<IImageBuffer> & imageBuffer,
    char const* szName, MINT32 format, MSize size, std::vector<MUINT32> & vStride, MBOOL secureOn)
{
    MBOOL isStreamSecure = MFALSE;
    SecType secType = SecType::mem_normal;
    if (secureOn) {
        isStreamSecure = mEnableSecure;
        secType = mSecType;
    } else {
        isStreamSecure = MFALSE;
        secType = SecType::mem_normal;
    }
    return mStuffBufMgr.acquireStoreBuffer(imageBuffer, szName, format, size,
        vStride, mStuffBufNumBasis, mStuffBufNumMax, mBurstNum,
        (mDebugScanLineMask != 0) ? MTRUE : MFALSE, isStreamSecure, secType);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
destroyStuffBuffer(sp<IImageBuffer> & imageBuffer)
{
    if (imageBuffer == NULL) {
        MY_LOGW("Stuff ImageBuffer not exist");
        return BAD_VALUE;
    }
    return mStuffBufMgr.releaseStoreBuffer(imageBuffer);
}

#if (USING_DRV_IO_PIPE_EVENT)
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
eventStreamingInform()
{
    {
        RWLock::AutoRLock _l(mIoPipeEvtStateLock);
        L_if (mIoPipeEvtState != IO_PIPE_EVT_STATE_ACQUIRING) {
            //MY_LOGI0("Streaming - state(%d)", mIoPipeEvtState);
            return;
        }
    }
    //
    {
        Mutex::Autolock _l(mIoPipeEvtWaitLock);
        L_if (mIoPipeEvtWaiting) {
            if ((mpDeliverMgr != NULL) && (mpDeliverMgr->runningGet()) &&
                (mpDeliverMgr->isActListEmpty())) {
                mIoPipeEvtWaitCond.broadcast();
                MY_LOGI0("action list is empty");
            }
        }
    }
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
eventStreamingOn()
{
    P1_TRACE_AUTO(SLG_E, "P1:eventStreamingOn");
    MY_LOGI0("StreamingOn +");
    Mutex::Autolock _l(mPublicLock);
    {
        RWLock::AutoRLock _l(mIoPipeEvtStateLock);
        U_if (mIoPipeEvtState != IO_PIPE_EVT_STATE_ACQUIRED) {
            MY_LOGI0("StreamingOn return - state(%d)", mIoPipeEvtState);
            return;
        }
    }
    {
        RWLock::AutoWLock _l(mIoPipeEvtStateLock);
        mIoPipeEvtState = IO_PIPE_EVT_STATE_NONE;
    }
    //
    beckonRequest();
    MY_LOGI0("StreamingOn -");
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
eventStreamingOff()
{
    P1_TRACE_AUTO(SLG_E, "P1:eventStreamingOff");
    MY_LOGI0("StreamingOff +");
    Mutex::Autolock _l(mPublicLock);
    {
        RWLock::AutoRLock _l(mIoPipeEvtStateLock);
        U_if (mIoPipeEvtState != IO_PIPE_EVT_STATE_NONE) {
            MY_LOGI0("StreamingOff return - state(%d)", mIoPipeEvtState);
            return;
        }
    }
    {
        RWLock::AutoWLock _l(mIoPipeEvtStateLock);
        mIoPipeEvtState = IO_PIPE_EVT_STATE_ACQUIRING;
    }
    //
    // wait for the last NORMAL/BYPASS action done
    MBOOL bWaitDrain = MTRUE;
    #if 1
    {
        MINT32 nWaitDrain =
            property_get_int32("vendor.debug.camera.p1nodefasthqc", 0);
        if (nWaitDrain > 0) {
            MY_LOGI0("p1node-fast-hqc:%d", nWaitDrain);
            bWaitDrain = MFALSE;
        }
    }
    #endif
    if (bWaitDrain && (mpDeliverMgr != NULL) && (mpDeliverMgr->runningGet())) {
        Mutex::Autolock _l(mIoPipeEvtWaitLock);
        mIoPipeEvtWaiting = MTRUE;
        while (!mpDeliverMgr->isActListEmpty()) {
            status_t res = mIoPipeEvtWaitCond.waitRelative(
                mIoPipeEvtWaitLock, P1NODE_EVT_DRAIN_WAIT_INV_NS);
            if (res == NO_ERROR) {
                MY_LOGI0("all actions done");
                break;
            } else {
                MY_LOGI0("actions not finish - res(%d) empty(%d)", res,
                    mpDeliverMgr->isActListEmpty());
                mpDeliverMgr->dumpInfo();
                mLogInfo.inspect(LogInfo::IT_EVT_WAIT_DRAIN_TIMEOUT);
            }
        }
        mIoPipeEvtWaiting = MFALSE;
    } else {
        MY_LOGI0("stop and flush directly, WaitDrain(%d)", bWaitDrain);
    }
    // In InitReq Flow (EN_INIT_REQ), mInitReqOff re-assign by eventStreamingOff() via onHandleFlush().
    // While eventStreamingOn(), by mInitReqOff to disable the init-request-flow as the next first request arrival.
    onHandleFlush(MFALSE, MTRUE); // disable InitReq flow
    //
    {
        RWLock::AutoWLock _l(mIoPipeEvtStateLock);
        mIoPipeEvtState = IO_PIPE_EVT_STATE_ACQUIRED;
    }
    MY_LOGI0("StreamingOff -");
    return;
};
#endif

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_start()
{
#if SUPPORT_ISP
    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_start");

    mLogInfo.setMemo(LogInfo::CP_OP_START_BGN,
        mBurstNum, (EN_START_CAP_RUN), (EN_INIT_REQ_RUN) ? mInitReqSet : 0);
    //
    MY_LOGI1("HwLockStartWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI1("HwLockStartWait ---");
    //
    mTagReq.clear();
    mTagSet.clear();
    mTagEnq.clear();
    mTagDeq.clear();
    mTagOut.clear();
    mTagList.clear();

    {
        MINT64 currentTime = (MINT64)(::systemTime());
        {
            Mutex::Autolock _l(mMonitorLock);
            mMonitorTime = currentTime;
        }
    }

    setLaunchState(NSP1Node::LAUNCH_STATE_ACTIVE);
    {
        Mutex::Autolock _l(mThreadLock);
        mActiveCond.broadcast();
    }

    #if USING_CTRL_3A_LIST_PREVIOUS
    mPreviousCtrlList.clear();
    #endif
    setInit(MTRUE);
    mLastSofIdx = P1SOFIDX_NULL_VAL;
    mLastSetNum = 0;
    {
        Mutex::Autolock _ll(mTransferJobLock);
        mTransferJobIdx = P1ACT_ID_NULL;
        mTransferJobWaiting = MFALSE;
    }

    #if SUPPORT_VHDR
    mpVhdr = NULL;
    #endif
    #if SUPPORT_LCS
    mpLCS = NULL;
    #endif
    #if SUPPORT_RSS
    mpRSS = NULL;
    #endif
    #if SUPPORT_FSC
    mpFSC = NULL;
    #endif
    mConfigPort = CONFIG_PORT_NONE;
    mConfigPortNum = 0;

    mFirstReceived = MFALSE;
    //
    {
        Mutex::Autolock _ll(mFrameSetLock);
        mFrameSetAlready = MFALSE;
    }
    //
    mDequeThreadProfile.reset();
    //mImageStorage.init(mLogLevel);
    //
    EImageFormat resizer_fmt = eImgFmt_FG_BAYER10;
    //
#ifdef P1_START_INFO_STR
#undef P1_START_INFO_STR
#endif
#define P1_START_INFO_STR "Cam::%d "\
        "Sensor(%dx%d)(%dns) Raw(%d,0x%x)-Proc(%dx%d)-Pure(%dx%d) "\
        "Bin(%dx%d) BinEn=%d TG(%d:%d) DTwin(%d@%d)=%d LSM(%d) QLV(%d) "\
        "Ratio(%d) SensorCfg(i:%d %dx%d s:%d b:%d c:%d, h:%d f:%d t:%d d:%d) "\
        "ConfigPort[%d]:(0x%x) InitParam[R:%d B:%d D:%d Nd:%d Ul:%d "\
        "Pb:%d Dt:%d Iq:%d F(DataPattern:x%x OffBin:x%x SensorNum:x%x RAW:x%x)]"

#ifdef P1_START_INFO_VAR
#undef P1_START_INFO_VAR
#endif
#define P1_START_INFO_VAR getOpenId(),\
        mSensorParams.size.w, mSensorParams.size.h, mSensorRollingSkewNs,\
        mRawDefType, mRawOption,\
        pSizeProc->w, pSizeProc->h, pSizePure->w, pSizePure->h,\
        binInfoSize.w, binInfoSize.h, mIsBinEn, mTgNum, mCfg.mSensorNum,\
        mDisableDynamicTwin, mCfg.mSupportDynamicTwin, mIsDynamicTwinEn,\
        mIsLegacyStandbyMode, mCfg.mQualityLv, mResizeRatioMax,\
        sensorCfg.index, sensorCfg.crop.w, sensorCfg.crop.h,\
        sensorCfg.scenarioId, sensorCfg.isBypassScenario,\
        sensorCfg.isContinuous, sensorCfg.HDRMode, sensorCfg.framerate,\
        sensorCfg.twopixelOn, sensorCfg.debugMode,\
        mConfigPortNum, mConfigPort, halCamIOinitParam.mRawType,\
        halCamIOinitParam.mBitdepth, halCamIOinitParam.m_DynamicRawType,\
        halCamIOinitParam.m_bN3D, halCamIOinitParam.m_UniLinkSel,\
        halCamIOinitParam.m_pipelinebitdepth, halCamIOinitParam.m_DynamicTwin,\
        halCamIOinitParam.m_IQlv,\
        halCamIOinitParam.m_Func.Bits.DATA_PATTERN,\
        halCamIOinitParam.m_Func.Bits.OFF_BIN,\
        halCamIOinitParam.m_Func.Bits.SensorNum,\
        halCamIOinitParam.m_Func.Raw
    //
    if (mspResConCtrl != NULL) {
        P1NODE_RES_CON_ACQUIRE(mspResConCtrl, mResConClient, mIsResConGot);
    }
    //
    {
        MERROR err = OK;
        NSCam::NSIoPipe::NSCamIOPipe::EPipeSelect ps =
            NSCam::NSIoPipe::NSCamIOPipe::EPipeSelect_Normal;
        if (mPipeMode == PIPE_MODE_NORMAL_SV) {
            ps = NSCam::NSIoPipe::NSCamIOPipe::EPipeSelect_NormalSv;
        }
    #if (P1NODE_USING_MTK_LDVT > 0)
        err = getNormalPipeModule()->createSubModule(
            getOpenId(), "iopipeUseTM", mCamIOVersion, (MVOID**)&mpCamIO, ps);
    #else
        err = getNormalPipeModule()->createSubModule(
            getOpenId(), getNodeName(), mCamIOVersion, (MVOID**)&mpCamIO, ps);
    #endif
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_DRV_INIT_BGN,
            LogInfo::CP_OP_START_DRV_INIT_END);
        P1_TIMING_CHECK("P1:DRV-init", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-init");
        MY_LOGI1("mpCamIO->init +++");

        bool secEnable = (mEnableSecure) ? true : false;
        if  ( err < 0 || ! mpCamIO || ! mpCamIO->init(secEnable) ) {
            MY_LOGE("hardware init fail - err:%#x mpCamIO:%p", err, mpCamIO);
            return DEAD_OBJECT;
        }

        MY_LOGI1("mpCamIO->init ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-init"
    }

#if SUPPORT_LCS
    if (mEnableLCSO) {
        P1_TIMING_CHECK("P1:LCS-init", 10, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LCS-init");
        mpLCS = MAKE_LcsHal(LOG_TAG, getOpenId());
        if(mpLCS == NULL)
        {
            MY_LOGE("mpLCS is NULL");
            return DEAD_OBJECT;
        }
        if( mpLCS->Init() != LCS_RETURN_NO_ERROR)
        {
            mpLCS->DestroyInstance(LOG_TAG);
            mpLCS = NULL;
        }
        P1_TRACE_C_END(SLG_S); // "P1:LCS-init"
    }
#endif
#if SUPPORT_VHDR
    {
        P1_TIMING_CHECK("P1:VHDR-init", 10, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:VHDR-init");
        mpVhdr = VHdrHal::CreateInstance(LOG_TAG, getOpenId());
        if(mpVhdr == NULL)
        {
            MY_LOGE("mpVhdr is NULL");
            return DEAD_OBJECT;
        }
        if( mpVhdr->Init(mSensorParams.vhdrMode) != VHDR_RETURN_NO_ERROR)
        {
            //mpVhdr->DestroyInstance(LOG_TAG); // instance always exist until process kill
            mpVhdr = NULL;
        }
        P1_TRACE_C_END(SLG_S); // "P1:VHDR-init"
    }
#endif
    //
    sp<IImageBuffer> pEISOBuf = NULL;
    if (mEnableEISO) {
        P1_TIMING_CHECK("P1:LMV-init", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-init");
        if (IS_LMV(mpConnectLMV)) {
            MINT32 mode = EisInfo::getMode(mPackedEisInfo);
            MINT32 factor = EisInfo::getFactor(mPackedEisInfo);
            if (MFALSE == mpConnectLMV->init(pEISOBuf, mode, factor)) {
                MY_LOGE("ConnectLMV create fail");
                return BAD_VALUE;
            }
        }
        P1_TRACE_C_END(SLG_S); // "P1:LMV-init"
    }
    //
    MUINT32 AETargetMode = MFALSE;  // vHDR OFF mode
#if SUPPORT_VHDR
    if(mpVhdr != NULL)
        AETargetMode = mpVhdr->getCurrentAEMode(mSensorParams.vhdrMode);
    else
        AETargetMode = MFALSE;  // vHDR OFF mode
#endif
    AEInitExpoSetting_T initExpoSetting;
    ::memset(&initExpoSetting, 0, sizeof(initExpoSetting));
    initExpoSetting.u4SensorMode = mSensorParams.mode;
    initExpoSetting.u4AETargetMode = AETargetMode;
    // set shutter/gain 0 as ::memset
    //
    NS3Av3::ConfigInfo_T config;
    NS3Av3::EBitMode_T b = NS3Av3::EBitMode_12Bit;
    switch (mPipeBit) {
        case CAM_Pipeline_10BITS: b = NS3Av3::EBitMode_10Bit; break;
        case CAM_Pipeline_12BITS: b = NS3Av3::EBitMode_12Bit; break;
        case CAM_Pipeline_14BITS: b = NS3Av3::EBitMode_14Bit; break;
        case CAM_Pipeline_16BITS: b = NS3Av3::EBitMode_16Bit; break;
        default:
            MY_LOGW("CANNOT map the pipeline bit mode");
            break;
    };
    config.i4BitMode = b;
    config.i4SubsampleCount = (MINT32)(MAX(mBurstNum, 1));
    config.bIsSecureCam = mEnableSecure;
    config.i4HlrOption = (mDisableHLR) ?
        (NS3Av3::EHlrOption_ForceOff) : (NS3Av3::EHlrOption_Auto);
    config.CfgAppMeta = mCfgAppMeta;
    config.CfgHalMeta = mCfgHalMeta;
#if SUPPORT_3A
    {
        P1_TIMING_CHECK("P1:3A-create-GetAEInitExpoSetting", 10, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-create-GetAEInitExpoSetting");
        mp3A = MAKE_Hal3A(getOpenId(), getNodeName());
        if(mp3A == NULL) {
            MY_LOGE("mp3A is NULL");
            return DEAD_OBJECT;
        }
        mp3A->send3ACtrl(E3ACtrl_GetAEInitExpoSetting,
            (MINTPTR)&initExpoSetting, (MINTPTR)&config);
        MY_LOGI1("GetAEInitExpoSetting: "
            "u4Eposuretime(le:%d/se:%d) u4AfeGain(le:%d/se:%d)",
            initExpoSetting.u4Eposuretime, initExpoSetting.u4Eposuretime_se,
            initExpoSetting.u4AfeGain, initExpoSetting.u4AfeGain_se);
        P1_TRACE_C_END(SLG_S); // "P1:3A-create-GetAEInitExpoSetting"
    }
#endif

#if 1
    IHalSensor::ConfigParam sensorCfg;
    ::memset(&sensorCfg, 0, sizeof(IHalSensor::ConfigParam));
    sensorCfg.index = (MUINT)getOpenId();
    sensorCfg.crop = mSensorParams.size;
    sensorCfg.scenarioId = mSensorParams.mode;
    sensorCfg.isBypassScenario = 0;
    sensorCfg.isContinuous = 1;
    #if SUPPORT_VHDR
    sensorCfg.HDRMode = mSensorParams.vhdrMode;
    #else
    sensorCfg.HDRMode = MFALSE;
    #endif
    #if (P1NODE_USING_MTK_LDVT > 0)
    sensorCfg.framerate = 1;
    #else
    sensorCfg.framerate = mSensorParams.fps;
    #endif
    sensorCfg.twopixelOn = 0;
    sensorCfg.debugMode = 0;
    sensorCfg.exposureTime = initExpoSetting.u4Eposuretime;
    sensorCfg.gain = initExpoSetting.u4AfeGain;
    sensorCfg.exposureTime_se = initExpoSetting.u4Eposuretime_se;
    sensorCfg.gain_se = initExpoSetting.u4AfeGain_se;
    #if SUPPORT_VHDR
    sensorCfg.exposureTime_me = initExpoSetting.u4Eposuretime_me;
    sensorCfg.gain_me = initExpoSetting.u4AfeGain_me;
    sensorCfg.exposureTime_vse = initExpoSetting.u4Eposuretime_vse;
    sensorCfg.gain_vse = initExpoSetting.u4AfeGain_vse;
    #endif
#else
    IHalSensor::ConfigParam sensorCfg =
    {
        (MUINT)getOpenId(),                 /* index            */
        mSensorParams.size,                 /* crop */
        mSensorParams.mode,                 /* scenarioId       */
        0,                                  /* isBypassScenario */
        1,                                  /* isContinuous     */
#if SUPPORT_VHDR
        mSensorParams.vhdrMode,             /* vHDROn mode          */
#else
        MFALSE,                             /* vHDROn mode          */
#endif
        #if (P1NODE_USING_MTK_LDVT > 0)
        1,
        #else
        mSensorParams.fps,                  /* framerate        */
        #endif
        0,                                  /* two pixel on     */
        0,                                  /* debugmode        */
        initExpoSetting.u4Eposuretime,
        initExpoSetting.u4AfeGain,
        initExpoSetting.u4Eposuretime_se,
        initExpoSetting.u4AfeGain_se,
    };
#endif

    std::vector<IHalSensor::ConfigParam> vSensorCfg;
    vSensorCfg.push_back(sensorCfg); // only insert once

    //
    std::vector<portInfo> vPortInfo;
    vPortInfo.clear();
    vPortInfo.reserve(P1_OUTPUT_PORT_TOTAL);
    if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL)
    {
        MINT fmt = mvStreamImg[STREAM_IMG_OUT_FULL]->getImgFormat();
        MBOOL secureOn = mvStreamImg[STREAM_IMG_OUT_FULL]->getSecureInfo();
        IImageStreamInfo::BufPlanes_t const & planes =
            mvStreamImg[STREAM_IMG_OUT_FULL]->getBufPlanes();
        portInfo OutPort(
            PORT_IMGO,
            (EImageFormat)fmt,
            mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize(),
            MRect(MPoint(0,0), mSensorParams.size),
            P1_STRIDE(planes, 0), P1_STRIDE(planes, 1), P1_STRIDE(planes, 2),
            0, // pureraw
            MTRUE/*IS_RAW_FMT_PACK_FULL(fmt)*/,  //packed
            secureOn);
            // by driver request, the PAK should be ON even if un-packed raw
        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_IMGO;
        mConfigPortNum ++;
    } else if (mvStreamImg[STREAM_IMG_OUT_OPAQUE] != NULL) {
        MBOOL secureOn = mvStreamImg[STREAM_IMG_OUT_OPAQUE]->getSecureInfo();
        portInfo OutPort(
            PORT_IMGO,
            (EImageFormat)mRawFormat,
            mSensorParams.size,
            MRect(MPoint(0,0), mSensorParams.size),
            mRawStride, 0/*StrideInByte[1]*/, 0/*StrideInByte[2]*/,
            0, // pureraw
            MTRUE/*IS_RAW_FMT_PACK_FULL(mRawFormat)*/, //packed
            secureOn);
            // by driver request, the PAK should be ON even if un-packed raw
        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_IMGO;
        mConfigPortNum ++;
    }
    //
    if (mvStreamImg[STREAM_IMG_OUT_RESIZE] != NULL)
    {
        MBOOL secureOn = mvStreamImg[STREAM_IMG_OUT_RESIZE]->getSecureInfo();
        IImageStreamInfo::BufPlanes_t const & planes =
            mvStreamImg[STREAM_IMG_OUT_RESIZE]->getBufPlanes();
        portInfo OutPort(
            PORT_RRZO,
            (EImageFormat)mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat(),
            mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgSize(),
            MRect(MPoint(0,0), mSensorParams.size),
            P1_STRIDE(planes, 0), P1_STRIDE(planes, 1), P1_STRIDE(planes, 2),
            0, // pureraw
            MTRUE, //packed
            secureOn);
        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_RRZO;
        mConfigPortNum ++;
        //
        resizer_fmt =
            (EImageFormat)mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat();
    }

    if (mEnableLCSO && mvStreamImg[STREAM_IMG_OUT_LCS] != NULL)
    {
        MBOOL secureOn = mvStreamImg[STREAM_IMG_OUT_LCS]->getSecureInfo();
        IImageStreamInfo::BufPlanes_t const & planes =
            mvStreamImg[STREAM_IMG_OUT_LCS]->getBufPlanes();
        portInfo OutPort(
            PORT_LCSO,
            (EImageFormat)mvStreamImg[STREAM_IMG_OUT_LCS]->getImgFormat(),
            mvStreamImg[STREAM_IMG_OUT_LCS]->getImgSize(),
            MRect(MPoint(0,0),  mvStreamImg[STREAM_IMG_OUT_LCS]->getImgSize()),
            P1_STRIDE(planes, 0), P1_STRIDE(planes, 1), P1_STRIDE(planes, 2),
            0, // pureraw
            MTRUE, //packed
            secureOn);
        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_LCSO;
        mConfigPortNum ++;
    }

    if (mEnableRSSO && mvStreamImg[STREAM_IMG_OUT_RSS] != NULL)
    {
        MBOOL secureOn = mvStreamImg[STREAM_IMG_OUT_RSS]->getSecureInfo();
        IImageStreamInfo::BufPlanes_t const & planes =
            mvStreamImg[STREAM_IMG_OUT_RSS]->getBufPlanes();
        portInfo OutPort(
            PORT_RSSO,
            (EImageFormat)mvStreamImg[STREAM_IMG_OUT_RSS]->getImgFormat(),
            mvStreamImg[STREAM_IMG_OUT_RSS]->getImgSize(),
            MRect(MPoint(0,0),  mvStreamImg[STREAM_IMG_OUT_RSS]->getImgSize()),
            P1_STRIDE(planes, 0), P1_STRIDE(planes, 1), P1_STRIDE(planes, 2),
            0, // pureraw
            MTRUE, //packed
            secureOn);
        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_RSSO;
        mConfigPortNum ++;
    }
    //
    if (mEnableEISO && pEISOBuf != NULL)
    {
        // secure camera EIS buffer can be normal
        MBOOL secureOn = MFALSE;
        portInfo OutPort(
                PORT_EISO,
                (EImageFormat)pEISOBuf->getImgFormat(),
                pEISOBuf->getImgSize(),
                MRect(MPoint(0,0),  pEISOBuf->getImgSize()),
                pEISOBuf->getBufStridesInBytes(0),
                0, //pPortCfg->mStrideInByte[1],
                0, //pPortCfg->mStrideInByte[2],
                0, // pureraw
                MTRUE, // packed
                secureOn);

        vPortInfo.push_back(OutPort);
        mConfigPort |= CONFIG_PORT_EISO;
        mConfigPortNum ++;
    }
    //
    MBOOL bDynamicRawType = MTRUE;  // true:[ON] ; false:[OFF]
    QInitParam halCamIOinitParam(
               0,                           /*sensor test pattern */
               10,                          /* bit depth*/
               vSensorCfg,
               vPortInfo,
               bDynamicRawType);
    halCamIOinitParam.m_Func.Raw = 0; // Clean Bits
    if ((((mRawOption & (1 << EPipe_PROCESSED_RAW)) > 0) && (!mEnableDualPD))
        || (mDisableFrontalBinning)
        ) {
        // In DualPD case, the frontal binning will be decided by driver.
        // therefore, it do not need to adjust for this case.
        // On Legacy platform, the OFF_BIN = 1 will be ignored
        halCamIOinitParam.m_Func.Bits.OFF_BIN = 1;
        //halCamIOinitParam.m_bOffBin = MTRUE;
    }
    //
    halCamIOinitParam.m_Func.Bits.DATA_PATTERN = mCfg.mPattern;
    halCamIOinitParam.m_IQlv = mCfg.mQualityLv;
    halCamIOinitParam.m_Func.Bits.SensorNum = mCfg.mSensorNum;
    halCamIOinitParam.m_pipelinebitdepth = (E_CAM_PipelineBitDepth_SEL)mPipeBit;
    halCamIOinitParam.m_DynamicTwin = mIsDynamicTwinEn;
    halCamIOinitParam.m_DropCB = doNotifyDropframe;
    halCamIOinitParam.m_returnCookie = this;
    // enable frame sync
    if (mEnableFrameSync) {
      MY_LOGI0("P1 node(%d) is in synchroized mode", getOpenId());
      halCamIOinitParam.m_bN3D = MTRUE;
    } else {
      halCamIOinitParam.m_bN3D = MFALSE;
    }

    // [TODO] control with the legacy platform consideration
    // enable EIS
    if(mForceSetEIS){
        MY_LOGI0("force set EIS enable(%d) id(%d)", mEnableEISO, getOpenId());
        if(mEnableEISO)
            halCamIOinitParam.m_UniLinkSel = E_UNI_LINK_SEL::E_UNI_LINK_ON;
        else
            halCamIOinitParam.m_UniLinkSel = E_UNI_LINK_SEL::E_UNI_LINK_OFF;
    }
    else{
        halCamIOinitParam.m_UniLinkSel = (mEnableUniForcedOn) ?
        E_UNI_LINK_ON : E_UNI_LINK_AUTO;
    }

    if (mEnableSecure)
    {
        switch(mSecType)
        {
            case SecType::mem_protected:
                halCamIOinitParam.m_SecureCam.buf_type = E_SEC_LEVEL_PROTECTED;
                halCamIOinitParam.m_SecureCam.chk_handle = mSecStatus;
                break;
            case SecType::mem_secure:
                halCamIOinitParam.m_SecureCam.buf_type = E_SEC_LEVEL_SECURE;
                halCamIOinitParam.m_SecureCam.chk_handle = mSecStatus;
                break;
            default:
                MY_LOGW("Not support secure type : 0x%x", mSecType);
                return BAD_VALUE;
        }
    }

    MY_LOGD1("ConfigPipe-Bit(%d)-N3d(%d)-Uni(%d)-Qlv(%d)-Func(0x%X)",
        halCamIOinitParam.m_pipelinebitdepth, halCamIOinitParam.m_bN3D,
        halCamIOinitParam.m_UniLinkSel, halCamIOinitParam.m_IQlv,
        halCamIOinitParam.m_Func.Raw);
    //
    MSize binInfoSize = mSensorParams.size;
    setCurrentBinSize(mSensorParams.size);
    mIsBinEn = false;
    MSize rawSize[2];
    MSize * pSizeProc = &rawSize[0]; // 0 = EPipe_PROCESSED_RAW
    MSize * pSizePure = &rawSize[1]; // 1 = EPipe_PURE_RAW
    *pSizeProc = MSize(0, 0);
    *pSizePure = MSize(0, 0);
    mCamCfgExp = MFALSE;
    L_if (mpCamIO != NULL) {
        P1_TIMING_CHECK("P1:DRV-configPipe", 500, TC_W);
        mLogInfo.setMemo(LogInfo::CP_OP_START_DRV_CFG_BGN);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-configPipe");
        MY_LOGI1("mpCamIO->configPipe +++");
        MBOOL cfgRes = mpCamIO->configPipe(halCamIOinitParam, mBurstNum);
        U_if (!cfgRes) {
            MY_LOGE("mpCamIO->configPipe fail");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-configPipe"
            mLogInfo.setMemo(LogInfo::CP_OP_START_DRV_CFG_END);
            mCamCfgExp = MTRUE;
            MY_LOGI0("[CamCfgExp] (%d) : return(%d) - " P1_START_INFO_STR,
                mCamCfgExp, cfgRes, P1_START_INFO_VAR);
            return BAD_VALUE;
        } else {
            MY_LOGI1("mpCamIO->configPipe ---");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-configPipe"
            mLogInfo.setMemo(LogInfo::CP_OP_START_DRV_CFG_END);
            P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-GetBinInfo");
            if (mpCamIO->sendCommand(ENPipeCmd_GET_BIN_INFO,
                (MINTPTR)&binInfoSize.w, (MINTPTR)&binInfoSize.h,
                (MINTPTR)NULL)) {
                P1_TRACE_C_END(SLG_S); // "P1:DRV-GetBinInfo"
                if (binInfoSize.w < mSensorParams.size.w ||
                    binInfoSize.h < mSensorParams.size.h) {
                    mIsBinEn = true;
                }
                setCurrentBinSize(binInfoSize);
            } else {
                P1_TRACE_C_END(SLG_S); // "P1:DRV-GetBinInfo"
            }
            //
            {
                MBOOL notSupportProc = MFALSE;
                MBOOL notSupportPure = MFALSE;
                MUINT32 newDefType = mRawDefType;
                MUINT32 newOption = mRawOption;
                P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-GetImgoInfo");
                if (mpCamIO->sendCommand(ENPipeCmd_GET_IMGO_INFO,
                    (MINTPTR)(&rawSize), (MINTPTR)NULL, (MINTPTR)NULL)) {
                    P1_TRACE_C_END(SLG_S); // "P1:DRV-GetImgoInfo"
                    if (pSizeProc->w == 0 || pSizeProc->h == 0) {
                        notSupportProc = MTRUE;
                    }
                    if (pSizePure->w == 0 || pSizePure->h == 0) {
                        notSupportPure = MTRUE;
                    }
                } else {
                    P1_TRACE_C_END(SLG_S); // "P1:DRV-GetImgoInfo"
                }
                if ((!notSupportProc) && (!notSupportPure)) {
                    // both Proc raw and Pure raw are supported
                    // not change the raw type setting
                } else if ((!notSupportProc) && (notSupportPure)) {
                    // only support Proc raw
                    newDefType = EPipe_PROCESSED_RAW;
                    newOption = (1 << EPipe_PROCESSED_RAW);
                } else if ((notSupportProc) && (!notSupportPure)) {
                    // only support Pure raw
                    newDefType = EPipe_PURE_RAW;
                    newOption = (1 << EPipe_PURE_RAW);
                } else {
                    // not support Proc raw and Pure raw
                    MY_LOGE("DualPD(%d) Raw(%d,0x%x) Proc(%dx%d) Pure(%dx%d) "
                    "- Not Support", mEnableDualPD, mRawDefType, mRawOption,
                        pSizeProc->w, pSizeProc->h, pSizePure->w, pSizePure->h);
                    return BAD_VALUE;
                }
                MY_LOGI_IF(((mEnableDualPD) ||
                    (mRawDefType != newDefType) || (mRawOption != newOption)),
                    "[RAW_TYPE] Raw(%d,0x%x) => New(%d,0x%x) : DualPD(%d) "
                    "Proc(%dx%d) Pure(%dx%d)", mEnableDualPD,
                    mRawDefType, mRawOption, newDefType, newOption,
                    pSizeProc->w, pSizeProc->h, pSizePure->w, pSizePure->h);
                //
                mRawDefType = newDefType;
                mRawOption = newOption;
            }
            //
            if (mpRegisterNotify != NULL) {
                MBOOL ret = MFALSE;
                P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-SetRrzCbfp");
                ret = mpCamIO->sendCommand(ENPipeCmd_SET_RRZ_CBFP,
                    (MINTPTR)(mpRegisterNotify->getNotifyCrop()),
                    (MINTPTR)NULL, (MINTPTR)NULL);
                P1_TRACE_C_END(SLG_S); // "P1:DRV-SetRrzCbfp"
                if (!ret) {
                    MY_LOGI0("sendCmd ENPipeCmd_SET_RRZ_CBFP return (%d)", ret);
                    #if USING_DRV_SET_RRZ_CBFP_EXP_SKIP
                    MY_LOGI0("sendCmd ENPipeCmd_SET_RRZ_CBFP return 0 , go-on");
                    #else
                    return BAD_VALUE;
                    #endif
                }
            }
        }
    }

    if ( auto pModule = getNormalPipeModule() ) {
        #if 1 // query height ratio from DRV
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
        pModule->query(
            NSCam::NSIoPipe::PORT_RRZO.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
            resizer_fmt, 0, info);
        mResizeRatioMax = info.bs_ratio;
        MY_LOGI2("ResizeRatioMax = info.bs_ratio(%d)", info.bs_ratio);
        #endif
    }

    queryRollingSkew(getOpenId(), (MUINT32)mSensorParams.mode,
        mSensorRollingSkewNs, mLogLevelI);

#if SUPPORT_3A
    {
        P1_TIMING_CHECK("P1:3A-notifyPwrOn", 10, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-notifyPwrOn");
        mLogInfo.setMemo(LogInfo::CP_OP_START_3A_PWRON_BGN);
        if (!mEnableSecure) {
            if (mp3A->notifyP1PwrOn()) { //CCU DRV power on after ISP configPipe
                setPowerNotify(MTRUE);
            } else {
                MY_LOGI0("3A->notifyP1PwrOn() return FALSE");
            }
        } else {
            MY_LOGW("Dont power on CCU in secure cam.");
        }
        mLogInfo.setMemo(LogInfo::CP_OP_START_3A_PWRON_END);
        P1_TRACE_C_END(SLG_S); // "P1:3A-notifyPwrOn"
    }
    {
        P1_TIMING_CHECK("P1:3A-setSensorMode", 10, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-setSensorMode");
        mp3A->setSensorMode(mSensorParams.mode);
        P1_TRACE_C_END(SLG_S); // "P1:3A-setSensorMode"
    }
#endif

    if (IS_LMV(mpConnectLMV)) {
        P1_TIMING_CHECK("P1:LMV-config", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-config");
        mpConnectLMV->config();
        P1_TRACE_C_END(SLG_S); // "P1:LMV-config"
    }

    #if SUPPORT_RSS
    if (mEnableRSSO) {
        P1_TIMING_CHECK("P1:RSS-register", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:RSS-register");
        mpRSS = RssHal::CreateInstance(LOG_TAG, getOpenId());
        if (mpRSS != NULL && mvStreamImg[STREAM_IMG_OUT_RSS] != NULL) {
            if (mpRSS->Init(mpCamIO, mvStreamImg[STREAM_IMG_OUT_RSS]->getImgSize()) != 0)
                return BAD_VALUE;
        }
        else {
            MY_LOGE("Cannot get RssHal instance");
            return BAD_VALUE;
        }
        P1_TRACE_C_END(SLG_S); // "P1:RSS-register"
    }
    #endif

#if SUPPORT_FSC
    if (mEnableFSC) {
        P1_TIMING_CHECK("P1:FSC-register", 20, TC_W);
        CAM_TRACE_BEGIN("P1:FSC-register");
        mpFSC = FSCHal::CreateInstance(LOG_TAG, getOpenId());
        if (mpFSC == NULL) {
            MY_LOGE("Cannot get FSCHal instance");
            return BAD_VALUE;
        }
        CAM_TRACE_END();
    }
#endif

    #if SUPPORT_VHDR
    if(mpVhdr)
    {
        VHDR_HAL_CONFIG_DATA vhdrConfig;
        vhdrConfig.cameraVer = VHDR_CAMERA_VER_3;
        //
        P1_TIMING_CHECK("P1:VHDR-config", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:VHDR-config");
        mpVhdr->ConfigVHdr(vhdrConfig);
        P1_TRACE_C_END(SLG_S); // "P1:VHDR-config"
    }
    #endif

    #if SUPPORT_LCS
    if(mpLCS)
    {
        LCS_HAL_CONFIG_DATA lcsConfig;
        lcsConfig.cameraVer = LCS_CAMERA_VER_3;
        if (mvStreamImg[STREAM_IMG_OUT_LCS] != NULL) {
            lcsConfig.lcsOutWidth =
                mvStreamImg[STREAM_IMG_OUT_LCS]->getImgSize().w;
            lcsConfig.lcsOutHeight =
                mvStreamImg[STREAM_IMG_OUT_LCS]->getImgSize().h;
        } else {
            MY_LOGI0("LCS enable but no LCS stream info");
            lcsConfig.lcsOutWidth = 0;
            lcsConfig.lcsOutHeight = 0;
        }
        //
        P1_TIMING_CHECK("P1:LCS-config", 20, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LCS-config");
        mpLCS->ConfigLcsHal(lcsConfig);
        P1_TRACE_C_END(SLG_S); // "P1:LCS-config"
    }
    #endif

    if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) { // execute after configPipe checking
        mpConCtrl->getStageCtrl()->done((MUINT32)STAGE_DONE_START, MTRUE);
    }

    #if SUPPORT_3A
    if (mp3A) {
        P1_TIMING_CHECK("P1:3A-config", 300, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-sendCtrl-attachCb");
        if (mEnableDualPD) {
            mp3A ->send3ACtrl(E3ACtrl_SetEnablePBin, 1, 0);
        }
        if ( (EIS_MODE_IS_EIS_ADVANCED_ENABLED(EisInfo::getMode(mPackedEisInfo)) ||
              EIS_MODE_IS_EIS_12_ENABLED(EisInfo::getMode(mPackedEisInfo)))
            && (mSensorParams.vhdrMode == SENSOR_VHDR_MODE_NONE)
            && (!mDisableAEEIS)) {
            mp3A ->send3ACtrl(E3ACtrl_SetAEEISRecording, 1, 0);
            MY_LOGD2("mEisMode:%d => Set EIS AE P-line \n",
                EisInfo::getMode(mPackedEisInfo));
        }
        #if SUPPORT_FSC
        if(mpFSC != NULL) {
            mpFSC->Init(mp3A, mSensorParams.size);// attach 3A CB before 3A->start()
        }
        #endif
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, this);
        P1_TRACE_C_END(SLG_S); // "P1:3A-sendCtrl-attachCb"
        //
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_3A_CFG_BGN,
            LogInfo::CP_OP_START_3A_CFG_END);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-config");
        MY_LOGI1("mp3A->config +++");
        mp3A->config(config);
        //m3AProcessedDepth = mp3A->getCapacity();
        MY_LOGI1("mp3A->config ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-config"
    }
    #endif
    //
    if (mpHwStateCtrl != NULL) {
        MBOOL isLegacy = (mIsLegacyStandbyMode || mForceStandbyMode > 0) ?
            MTRUE : MFALSE;
        mpHwStateCtrl->config(getOpenId(), mLogLevel, mLogLevelI, mSysLevel,
            mBurstNum, mpCamIO, mp3A, isLegacy);
    }
    //
    if (EN_INIT_REQ_RUN) {
        MY_LOGI0("InitRqeFlow return %d %d %d - " P1_START_INFO_STR,
            mInitReqSet, mInitReqNum, mInitReqCnt, P1_START_INFO_VAR);
        mLogInfo.setMemo(LogInfo::CP_OP_START_REQ_WAIT_BGN,
            LogInfo::START_SET_REQUEST);
        mNeedHwReady = MTRUE;
        return OK;
    }
    //
    if (EN_START_CAP_RUN) {
        Mutex::Autolock _l(mStartCaptureLock);
        mStartCaptureState = START_CAP_STATE_WAIT_REQ;
        mStartCaptureType = E_CAPTURE_NORMAL;
        mStartCaptureIdx = 0;
        mStartCaptureExp = 0;
        MY_LOGI0("EnableCaptureFlow(%d) return - " P1_START_INFO_STR,
            EN_START_CAP_RUN, P1_START_INFO_VAR);
        mLogInfo.setMemo(LogInfo::CP_OP_START_REQ_WAIT_BGN,
            LogInfo::START_SET_CAPTURE);
        mNeedHwReady = MTRUE;
        return OK;
    }

    #if SUPPORT_3A
    if (mp3A) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_3A_START_BGN,
            LogInfo::CP_OP_START_3A_START_END);
        P1_TIMING_CHECK("P1:3A-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-start");
        MY_LOGI1("mp3A->start +++");
        mp3A->start();
        MY_LOGI1("mp3A->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-start"
    }
    #endif
    //
    {
        if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) {
            if (mpConCtrl->getAidUsage()) {
                MBOOL success = MFALSE;
                mpConCtrl->getStageCtrl()->wait(
                    (MUINT32)STAGE_DONE_INIT_ITEM, success);
                if (!success) {
                    MY_LOGE("stage - init item fail");
                    return BAD_VALUE;
                }
            } else {
                P1_ATOMIC_ACQUIRE;
                MERROR status = buildInitItem();
                P1_ATOMIC_RELEASE;
                U_if (status != OK) {
                    mpConCtrl->initBufInfo_clean();
                    MY_LOGE("hardware init-buf fail (%d)", status);
                    return status;
                }
            }
        }
        U_if (mProcessingQueue.size() < 1) {
            MY_LOGE("ProcessingQueue empty");
            return BAD_VALUE;
        }
        // before DRV.start , no DEQ/DROP execution and RegisterNotify to change ProcessingQueue
        MERROR status = hardwareOps_enque(
            mProcessingQueue.at(mProcessingQueue.size()-1),
            ENQ_TYPE_INITIAL, MTRUE);
        if (status != OK) {
            MY_LOGE("hardware init-enque fail (%d)", status);
            return status;
        }
    }
    //
    if (mpConCtrl != NULL) {
        mpConCtrl->cleanAidStage();
    }
    //
    #if 1
    if (mpCamIO != NULL) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_DRV_START_BGN,
            LogInfo::CP_OP_START_DRV_START_END);
        P1_TIMING_CHECK("P1:DRV-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-start");
        MY_LOGI1("mpCamIO->start +++");
        if(!mpCamIO->start()) {
            MY_LOGE("mpCamIO->start fail");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
            return BAD_VALUE;
        }
        MY_LOGI1("mpCamIO->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
    }
    #endif
    //
    if (IS_LMV(mpConnectLMV)) {
        P1_TIMING_CHECK("P1:LMV-sensor", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-sensor");
        MY_LOGI1("mpConnectLMV->enableSensor +++");
        mpConnectLMV->enableSensor();
        MY_LOGI1("mpConnectLMV->enableSensor ---");
        P1_TRACE_C_END(SLG_S); // "P1:LMV-sensor"
    }
    //
    {
        Mutex::Autolock _l(mStartLock);
        setLaunchState(NSP1Node::LAUNCH_STATE_READY);
        mStartCond.broadcast();
    }
    {
        Mutex::Autolock _l(mThreadLock);
        mReadyCond.broadcast();
    }
    syncHelperStart();
    MY_LOGI0("End - " P1_START_INFO_STR, P1_START_INFO_VAR);
    #undef P1_START_INFO_STR
    #undef P1_START_INFO_VAR

    mLogInfo.setMemo(LogInfo::CP_OP_START_END,
        mBurstNum, (EN_START_CAP_RUN), (EN_INIT_REQ_RUN) ? mInitReqSet : 0,
        LogInfo::START_SET_GENERAL);

    FUNCTION_P1_OUT;

    return OK;
#else
    return OK;
#endif

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_request()
{
#if SUPPORT_ISP
    FUNCTION_P1S_IN;
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_request");
    //
    mLogInfo.setMemo(LogInfo::CP_OP_START_REQ_WAIT_END,
        LogInfo::START_SET_REQUEST);
    //
    MY_LOGI1("HwLockReqWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI1("HwLockReqWait ---");
    //
    //MINT32 num = 0;
    //
    #if USING_CTRL_3A_LIST
    List<MetaSet_T> ctrlList;
    #else
    std::vector< MetaSet_T* > ctrlQueue;
    ctrlQueue.clear();
    ctrlQueue.reserve(mInitReqNum);
    #endif
    //
    MUINT32 total = mpTaskCollector->remainder();
    MUINT32 initNum = mInitReqNum - 1;
    U_if (total < mInitReqNum) {
        MY_LOGE("init request set is not enough (%d < %d)", total, mInitReqSet);
        return BAD_VALUE;
    }
    // Prepare for DRV
    for (MUINT32 index = 0; index < initNum; index++) {
        P1QueJob job(mBurstNum);
        mpTaskCtrl->sessionLock();
        mpTaskCollector->requireJob(job);
        mpTaskCtrl->sessionUnLock();
        {
            Mutex::Autolock _l(mProcessingQueueLock);
            mProcessingQueue.push_back(job);
        }
        if (job.size() > 0 && job.edit(0).ptr() != NULL) {
            #if USING_CTRL_3A_LIST
            ctrlList.push_back(job.edit(0).ptr()->metaSet);
            #else
            ctrlQueue.push_back(&(job.edit(0).ptr()->metaSet));
            #endif
        }
    }
    // Set to 3A
    {
        P1QueJob job(mBurstNum);
        mpTaskCtrl->sessionLock();
        mpTaskCollector->requireJob(job);
        mpTaskCtrl->sessionUnLock();
        {
            Mutex::Autolock _l(mRequestQueueLock);
            mRequestQueue.push_back(job);
        }
        if (job.size() > 0 && job.edit(0).ptr() != NULL) {
            #if USING_CTRL_3A_LIST
            ctrlList.push_back(job.edit(0).ptr()->metaSet);
            #else
            ctrlQueue.push_back(&(job.edit(0).ptr()->metaSet));
            #endif
        }
        mLastSetNum = job.getLastNum();
        mTagSet.set(mLastSetNum);
        //
        {
            Mutex::Autolock _ll(mFrameSetLock);
            #if SUPPORT_3A
            if (mp3A) {
                P1_TIMING_CHECK("P1:3A-startRequest", 200, TC_W);
                mLogInfo.setMemo(LogInfo::CP_START_SET_BGN,
                    LogInfo::START_SET_REQUEST, mLastSetNum);
                P1_TRACE_S_BEGIN(SLG_S, "P1:3A-startRequest");
                MY_LOGI1("mp3A->startRequestQ +++");
                #if USING_CTRL_3A_LIST
                mp3A->startRequestQ(ctrlList);//mp3A->start();
                #else
                mp3A->startRequestQ(ctrlQueue);//mp3A->start();
                #endif
                MY_LOGI1("mp3A->startRequestQ ---");
                P1_TRACE_C_END(SLG_S); // "P1:3A-startRequest"
                mLogInfo.setMemo(LogInfo::CP_START_SET_END,
                    LogInfo::START_SET_REQUEST, mLastSetNum);
            }
            #endif
            mFrameSetAlready = MTRUE;
        }
    }
    // EnQ to DRV
    {
        for (MUINT32 idx = 0; idx < initNum; idx++) {
            P1QueJob job(mBurstNum);
            {   // clone the QueJob from the ProcessingQueue
                Mutex::Autolock _l(mProcessingQueueLock);
                job = mProcessingQueue.at(idx);
            }
            MY_LOGD0("InitReqEnQ (%d/%d) +++", idx, initNum);
            MERROR status = hardwareOps_enque(job, ENQ_TYPE_INITIAL, MFALSE);
            if (status != OK) {
                MY_LOGE("hardware req-init-enque fail (%d)@(%d)", status, idx);
                return status;
            }
            MY_LOGD0("InitReqEnQ (%d/%d) ---", idx, initNum);
        }
    }
    //
    if (mpConCtrl != NULL) {
        mpConCtrl->cleanAidStage();
    }
    //
    #if 1
    if (mpCamIO) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_DRV_START_BGN,
            LogInfo::CP_OP_START_DRV_START_END);
        P1_TIMING_CHECK("P1:DRV-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-start");
        MY_LOGI1("mpCamIO->start +++");
        if(!mpCamIO->start()) {
            MY_LOGE("hardware start fail");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
            return BAD_VALUE;
        }
        MY_LOGI1("mpCamIO->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
    }
    #endif
    //
    if (IS_LMV(mpConnectLMV)) {
        P1_TIMING_CHECK("P1:LMV-sensor", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-sensor");
        MY_LOGI1("mpConnectLMV->enableSensor +++");
        mpConnectLMV->enableSensor();
        MY_LOGI1("mpConnectLMV->enableSensor ---");
        P1_TRACE_C_END(SLG_S); // "P1:LMV-sensor"
    }
    //
    mNeedHwReady = MFALSE;
    {
        Mutex::Autolock _l(mStartLock);
        setLaunchState(NSP1Node::LAUNCH_STATE_READY);
        mStartCond.broadcast();
    }
    {
        Mutex::Autolock _l(mThreadLock);
        mReadyCond.broadcast();
    }
    syncHelperStart();
    MY_LOGI0("Cam::%d BinEn:%d ConfigPort[%d]:0x%x",
        getOpenId(), mIsBinEn,
        mConfigPortNum, mConfigPort);

    mLogInfo.setMemo(LogInfo::CP_OP_START_END,
        mBurstNum, (EN_START_CAP_RUN), (EN_INIT_REQ_RUN) ? mInitReqSet : 0,
        LogInfo::START_SET_REQUEST);

    FUNCTION_P1S_OUT;

    return OK;
#else
    return OK;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_capture()
{
#if SUPPORT_ISP
    FUNCTION_P1S_IN;
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_capture");
    //
    mLogInfo.setMemo(LogInfo::CP_OP_START_REQ_WAIT_END,
        LogInfo::START_SET_CAPTURE);
    //
    MY_LOGI1("HwLockCapWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI1("HwLockCapWait ---");
    //
    MINT32 num = 0;
    MBOOL isManualCap = MFALSE;
    //
    if (EN_START_CAP_RUN) {
        Mutex::Autolock _l(mStartCaptureLock);
        mStartCaptureState = START_CAP_STATE_WAIT_CB;
    }
    //
    {
        MINT32 type = ESTART_CAP_NORMAL;
        {
            #if 1
            P1QueJob job(mBurstNum);
            mpTaskCtrl->sessionLock();
            mpTaskCollector->requireJob(job);
            mpTaskCtrl->sessionUnLock();
            #endif
            #if USING_CTRL_3A_LIST
            List<MetaSet_T> ctrlList;
            generateCtrlList(&ctrlList, job);
            #else
            std::vector< MetaSet_T* > ctrlQueue;
            ctrlQueue.clear();
            ctrlQueue.reserve(job.size());
            generateCtrlQueue(ctrlQueue, job);
            #endif
            {
                Mutex::Autolock _l(mRequestQueueLock);
                mRequestQueue.push_back(job);
            }
            Mutex::Autolock _ll(mFrameSetLock);
            #if SUPPORT_3A
            if (mp3A) {
                P1_TIMING_CHECK("P1:3A-startCapture", 200, TC_W);
                mLogInfo.setMemo(LogInfo::CP_START_SET_BGN,
                    LogInfo::START_SET_CAPTURE, job.getIdx());
                P1_TRACE_S_BEGIN(SLG_S, "P1:3A-startCapture");
                MY_LOGI1("mp3A->startCapture +++");
                #if USING_CTRL_3A_LIST
                type = mp3A->startCapture(ctrlList);//mp3A->start();
                #else
                type = mp3A->startCapture(ctrlQueue);//mp3A->start();
                #endif
                MY_LOGI1("mp3A->startCapture ---");
                P1_TRACE_C_END(SLG_S); // "P1:3A-startCapture"
                mLogInfo.setMemo(LogInfo::CP_START_SET_END,
                    LogInfo::START_SET_CAPTURE, job.getIdx());
            }
            #endif
            mFrameSetAlready = MTRUE;
            MY_LOGI1("start-capture-type %d", type);
        }
        if (type != ESTART_CAP_NORMAL) {
            isManualCap = MTRUE;
            MY_LOGI0("capture in manual flow %d", type);
        }
    }
    //
    if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) {
        if (mpConCtrl->getAidUsage()) {
            MBOOL success = MFALSE;
            mpConCtrl->getStageCtrl()->wait(
                (MUINT32)STAGE_DONE_INIT_ITEM, success);
            if (!success) {
                MY_LOGE("stage - cap init item fail");
                return BAD_VALUE;
            }
        } else {
            P1_ATOMIC_ACQUIRE;
            MERROR status = buildInitItem();
            P1_ATOMIC_RELEASE;
            U_if (status != OK) {
                mpConCtrl->initBufInfo_clean();
                MY_LOGE("hardware cap-init-buf fail (%d)", status);
                return status;
            }
        }
    }
    U_if (mProcessingQueue.size() < 1) {
        MY_LOGE("Cap ProcessingQueue empty");
        return BAD_VALUE;
    }
    //
    // Normal-Cap : EnQ-Init-Buf => EnQ-First-Buf(wait-3A-ready) => DRV-Start
    // Manual-Cap : EnQ-Init-Buf => DRV-Start => EnQ-First-Buf(wait-3A-ready)
    //
    {   // before DRV.start , no DEQ/DROP execution and RegisterNotify to change ProcessingQueue
        MERROR status = hardwareOps_enque(
            mProcessingQueue.at(mProcessingQueue.size()-1),
            ENQ_TYPE_INITIAL, MTRUE);
        if (status != OK) {
            MY_LOGE("hardware cap-init-enque fail (%d)", status);
            return status;
        }
    }
    //
    if (!isManualCap) {
        P1_TRACE_S_BEGIN(SLG_S, "Cap Normal EnQ");
        P1QueJob job(mBurstNum);
        {
            {
                Mutex::Autolock _l(mRequestQueueLock);
                L_if (mRequestQueue.size() > 0) {
                    Que_T::iterator it = mRequestQueue.begin();
                    job = *it;
                    mRequestQueue.erase(it);
                } else {
                    MY_LOGE("NormalCap RequestQueue is empty");
                    return BAD_VALUE;
                }
            }
            MERROR status = onProcessEnqueFrame(job);
            if (status != OK) {
                MY_LOGE("hardware cap-enque-normal fail (%d)", status);
                return status;
            }
            num = job.edit(0).getNum();
        }
        P1_TRACE_C_END(SLG_S); // "Cap Normal EnQ"
        //
        if (num > 0) {
            mLastSetNum = job.getLastNum();
            mTagSet.set(mLastSetNum);
        }
    }
    //
    if (mpConCtrl != NULL) {
        mpConCtrl->cleanAidStage();
    }
    //
    #if 1
    if (mpCamIO) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_DRV_START_BGN,
            LogInfo::CP_OP_START_DRV_START_END);
        P1_TIMING_CHECK("P1:DRV-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-start");
        MY_LOGI1("mpCamIO->start +++");
        if(!mpCamIO->start()) {
            MY_LOGE("hardware start fail");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
            return BAD_VALUE;
        }
        MY_LOGI1("mpCamIO->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
    }
    #endif
    //
    if (isManualCap) {
        P1_TRACE_S_BEGIN(SLG_S, "Cap Manual EnQ");
        P1QueJob job(mBurstNum);
        {
            {
                Mutex::Autolock _l(mRequestQueueLock);
                L_if (mRequestQueue.size() > 0) {
                    Que_T::iterator it = mRequestQueue.begin();
                    job = *it;
                    mRequestQueue.erase(it);
                } else {
                    MY_LOGE("ManualCap RequestQueue is empty");
                    return BAD_VALUE;
                }
            }
            MERROR status = onProcessEnqueFrame(job);
            if (status != OK) {
                MY_LOGE("hardware cap-enque-manual fail (%d)", status);
                return status;
            }
            num = job.edit(0).getNum();
        }
        P1_TRACE_C_END(SLG_S); // "Cap Manual EnQ"
        //
        if (num > 0) {
            mLastSetNum = job.getLastNum();
            mTagSet.set(mLastSetNum);
        }
    }
    //
    if (IS_LMV(mpConnectLMV)) {
        P1_TIMING_CHECK("P1:LMV-sensor", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-sensor");
        MY_LOGI1("mpConnectLMV->enableSensor +++");
        mpConnectLMV->enableSensor();
        MY_LOGI1("mpConnectLMV->enableSensor ---");
        P1_TRACE_C_END(SLG_S); // "P1:LMV-sensor"
    }
    //
    mNeedHwReady = MFALSE;
    {
        Mutex::Autolock _l(mStartLock);
        setLaunchState(NSP1Node::LAUNCH_STATE_READY);
        mStartCond.broadcast();
    }
    {
        Mutex::Autolock _l(mThreadLock);
        mReadyCond.broadcast();
    }
    syncHelperStart();
    MY_LOGI0("Cam::%d BinEn:%d ConfigPort[%d]:0x%x",
        getOpenId(), mIsBinEn,
        mConfigPortNum, mConfigPort);

    mLogInfo.setMemo(LogInfo::CP_OP_START_END,
        mBurstNum, (EN_START_CAP_RUN), (EN_INIT_REQ_RUN) ? mInitReqSet : 0,
        LogInfo::START_SET_CAPTURE);

    FUNCTION_P1S_OUT;

    return OK;
#else
    return OK;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_ready()
{
    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_ready");
    //
    MY_LOGI1("HwLockReadyWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI1("HwLockReadyWait ---");
    //
    #if SUPPORT_3A
    if (mp3A) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_3A_START_BGN,
            LogInfo::CP_OP_START_3A_START_END);
        P1_TIMING_CHECK("P1:3A-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-start");
        MY_LOGI1("mp3A->start +++");
        mp3A->start();
        MY_LOGI1("mp3A->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-start"
    }
    #endif
    //
    MBOOL initBufReady = MFALSE;
    if (mpConCtrl != NULL && mpConCtrl->getAidUsage()) {
        if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) {
            MBOOL success = MFALSE;
            mpConCtrl->getStageCtrl()->wait(
                (MUINT32)STAGE_DONE_INIT_ITEM, success);
            if (!success) {
                MY_LOGE("stage - init item fail");
                return BAD_VALUE;
            }
            initBufReady = MTRUE;
            MY_LOGI0("stage - init item ready");
        }
    } else { // no AidStart
        if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
            return BAD_VALUE;
        };
        P1QueJob job(mBurstNum);
        mpTaskCtrl->sessionLock();
        P1TaskCollector initCollector(mpTaskCtrl);
        for (int i = 0; i < mBurstNum; i++) {
            P1QueAct initAct;
            initCollector.enrollAct(initAct);
            createAction(initAct, NULL, REQ_TYPE_INITIAL);
            initCollector.verifyAct(initAct);
        }
        initCollector.requireJob(job);
        mpTaskCtrl->sessionUnLock();
        //
        if (!job.ready()) {
            MY_LOGE("init-job-not-ready");
            mpTaskCtrl->dumpActPool();
            return BAD_VALUE;
        } else {
            Mutex::Autolock _l(mProcessingQueueLock);
            mProcessingQueue.push_back(job);
        }
    }
    //
    {
        P1QueJob initJob(mBurstNum);    // prevent ProcessingQueueLock in P1RegisterNotify::doNotifyCrop
        {                               // clone the QueJob from the ProcessingQueue
            Mutex::Autolock _l(mProcessingQueueLock);
            P1_ATOMIC_ACQUIRE;
            U_if (mProcessingQueue.empty()) {
                MY_LOGE("ProcessingQueue-not-ready");
                mpTaskCtrl->dumpActPool();
                return BAD_VALUE;
            } else {
                MY_LOGI0("ProcessingQueue.size(%zu)", mProcessingQueue.size());
            }
            initJob = mProcessingQueue.at(mProcessingQueue.size()-1);
        }
        MERROR status = hardwareOps_enque(initJob, ENQ_TYPE_INITIAL,
            initBufReady);
        if (status != OK) {
            MY_LOGE("hardware ready init-enque fail (%d)", status);
            return status;
        }
    }
    //
    if (mpConCtrl != NULL) {
        mpConCtrl->cleanAidStage();
    }
    //
    #if 1
    if (mpCamIO != NULL) {
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_START_DRV_START_BGN,
            LogInfo::CP_OP_START_DRV_START_END);
        P1_TIMING_CHECK("P1:DRV-start", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-start");
        MY_LOGI1("mpCamIO->start +++");
        if(!mpCamIO->start()) {
            MY_LOGE("mpCamIO->start fail");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
            return BAD_VALUE;
        }
        MY_LOGI1("mpCamIO->start ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-start"
    }
    #endif
    //
    if (IS_LMV(mpConnectLMV)) {
        P1_TIMING_CHECK("P1:LMV-sensor", 100, TC_W);
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-sensor");
        MY_LOGI1("mpConnectLMV->enableSensor +++");
        mpConnectLMV->enableSensor();
        MY_LOGI1("mpConnectLMV->enableSensor ---");
        P1_TRACE_C_END(SLG_S); // "P1:LMV-sensor"
    }
    //
    mNeedHwReady = MFALSE;
    {
        Mutex::Autolock _l(mStartLock);
        setLaunchState(NSP1Node::LAUNCH_STATE_READY);
        mStartCond.broadcast();
    }
    {
        Mutex::Autolock _l(mThreadLock);
        mReadyCond.broadcast();
    }
    syncHelperStart();
    MY_LOGI0("End - ");

    mLogInfo.setMemo(LogInfo::CP_OP_START_END,
        mBurstNum, (EN_START_CAP_RUN), (EN_INIT_REQ_RUN) ? mInitReqSet : 0,
        LogInfo::START_SET_READY);

    FUNCTION_P1_OUT;

    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
procedureAid_start()
{
    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_S, "P1:aid_start");
    MERROR status = OK;
    if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) {
        MBOOL success = MFALSE;
        mpConCtrl->getStageCtrl()->wait(
            (MUINT32)STAGE_DONE_START, success);
        if (!success) {
            MY_LOGE("stage - aid start fail");
            return BAD_VALUE;
        }
    }
    //
    MBOOL init_success = MTRUE;
    P1_ATOMIC_ACQUIRE;
    status = buildInitItem();
    P1_ATOMIC_RELEASE;
    //
    if (OK != status) {
        init_success = MFALSE;
        MY_LOGE("CANNOT build init item");
    }
    if (mpConCtrl != NULL && mpConCtrl->getStageCtrl() != NULL) {
        mpConCtrl->getStageCtrl()->done(
            (MUINT32)STAGE_DONE_INIT_ITEM, init_success);
    }
    //
    FUNCTION_P1_OUT;
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
buildInitItem()
{
    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_S, "P1:reserve_init");
    if (isReady()) {
        MY_LOGW("it should be executed before start ready");
        return BAD_VALUE;
    }
    //
    if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
        return BAD_VALUE;
    };
    P1QueJob job(mBurstNum);
    mpTaskCtrl->sessionLock();
    P1TaskCollector initCollector(mpTaskCtrl);
    for (int i = 0; i < mBurstNum; i++) {
        P1QueAct initAct;
        initCollector.enrollAct(initAct);
        createAction(initAct, NULL, REQ_TYPE_INITIAL);
        initCollector.verifyAct(initAct);
    }
    initCollector.requireJob(job);
    mpTaskCtrl->sessionUnLock();
    //
    if (!job.ready()) {
        MY_LOGE("init-job-not-ready");
        mpTaskCtrl->dumpActPool();
        return BAD_VALUE;
    } else {
        Mutex::Autolock _l(mProcessingQueueLock);
        mProcessingQueue.push_back(job);
    }
    //
    P1QueJob & p_job = mProcessingQueue.at(mProcessingQueue.size()-1);
    QBufInfo* pEnBuf = NULL;
    if (mpConCtrl == NULL || (!mpConCtrl->initBufInfo_create(&pEnBuf))
        || pEnBuf == NULL) {
        MY_LOGE("CANNOT create the initBufInfo");
        return BAD_VALUE;
    }
    for (size_t i = 0; i < p_job.size(); i++) {
        MY_LOGD2("p_job(%d)(%zu/%zu)",
            p_job.getIdx(), i, p_job.size());
        if (OK != setupAction(p_job.edit(i), (*pEnBuf))) {
            MY_LOGE("setup enque act fail");
            return BAD_VALUE;
        }
        P1Act act = GET_ACT_PTR(act, p_job.edit(i), BAD_VALUE);
        act->exeState = EXE_STATE_PROCESSING;
    }
    //
    FUNCTION_P1_OUT;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateAppMeta(P1QueAct & rAct, MetaSet_T const &result3A,
    QBufInfo const &deqBuf, IMetadata &appMetadata, MUINT32 const index)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    if (act->appFrame == NULL) {
        MY_LOGW("pipeline frame is NULL (%d)", act->magicNum);
        return;
    }
    sp<IPipelineFrame> request = act->appFrame;

    //[3A/Flash/sensor section]
    appMetadata = result3A.appMeta;

    MBOOL needOverrideTimestamp = MFALSE;
    if (tryGetMetadata<MBOOL>(&result3A.halMeta, MTK_EIS_NEED_OVERRIDE_TIMESTAMP, needOverrideTimestamp)
        && needOverrideTimestamp) {
        IMetadata::IEntry entry(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP);
        entry.push_back( 1, Type2Type< MUINT8 >()); // Need Override timestamp
        entry.push_back( 0, Type2Type< MUINT8 >()); // timestamp not overrided yet
        appMetadata.update(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP, entry);
    }

    //[request section]
    // android.request.frameCount
    {
        IMetadata::IEntry entry(MTK_REQUEST_FRAME_COUNT);
        entry.push_back( request->getFrameNo(), Type2Type< MINT32 >());
        appMetadata.update(MTK_REQUEST_FRAME_COUNT, entry);
    }
    // android.request.metadataMode
    {
        IMetadata::IEntry entry(MTK_REQUEST_METADATA_MODE);
        entry.push_back(MTK_REQUEST_METADATA_MODE_FULL, Type2Type< MUINT8 >());
        appMetadata.update(MTK_REQUEST_METADATA_MODE, entry);
    }

    //[sensor section]
    // android.sensor.timestamp
    {
        MINT64 frame_duration = 0;
        //IMetadata::IEntry entry(MTK_SENSOR_FRAME_DURATION);
        //should get from control.
        #if 1 // modify timestamp
        frame_duration = act->frameExpDuration;
        #endif
        // ISP SOF : ISP get the first line from sensor
        MINT64 Sof = (deqBuf.mvOut[index].mMetaData.mTimeStamp_B != 0) ?
            deqBuf.mvOut[index].mMetaData.mTimeStamp_B :
            deqBuf.mvOut[index].mMetaData.mTimeStamp;
        if (mTimestampSrc == MTK_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN) {
            Sof = act->frameTimeStamp; // replace by mono
        }
        MINT64 timestamp = (Sof != 0) ? (Sof - frame_duration) : 0;
        IMetadata::IEntry entry(MTK_SENSOR_TIMESTAMP);
        {
            MINT64 expStart = 0;
            android::String8 strInfo("");
            queryStartExpTs(getOpenId(), (MUINT32)mSensorParams.mode,
                frame_duration, Sof, expStart, mLogLevelI);
            if (expStart != 0) {
                timestamp = expStart;
                strInfo += String8::format("Q"); // by sensor Query
            } else {
                strInfo += String8::format("C"); // by local Calculate
            }
            act->expTimestamp = timestamp;
            strInfo += String8::format("(SensorTs:%" PRId64 ")",
                act->expTimestamp);
            act->res += strInfo;
        }
        entry.push_back(timestamp, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_TIMESTAMP, entry);
    }

    //[sensor section]
    // android.sensor.rollingshutterskew
    // query from sensor after configure
    {
        MINT64 skew = (mSensorRollingSkewNs > P1_SENSOR_ROLLING_SKEW_UNKNOWN) ?
            (MINT64)mSensorRollingSkewNs : P1_SENSOR_ROLLING_SKEW_DEFAULT_NS;
        IMetadata::IEntry entry(MTK_SENSOR_ROLLING_SHUTTER_SKEW);
        entry.push_back(skew, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_ROLLING_SHUTTER_SKEW, entry);
    }


}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateAppTagIndex(IMetadata &appMetadata, IMetadata &appTagIndex)
{
    IMetadata::IEntry entryTagIndex(MTK_P1NODE_METADATA_TAG_INDEX);

    for (size_t i = 0; i < appMetadata.count(); ++i) {
        IMetadata::IEntry entry = appMetadata.entryAt(i);
        entryTagIndex.push_back((MINT32)entry.tag(), Type2Type<MINT32>());
    }

    if (OK != appTagIndex.update(entryTagIndex.tag(), entryTagIndex)) {
        MY_LOGE("fail to update index");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateHalMeta(P1QueAct & rAct, MetaSet_T const &result3A,
    QBufInfo const &deqBuf, IMetadata const &resultAppend,
    IMetadata const &inHalMetadata, IMetadata &halMetadata, MUINT32 const index)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    if (deqBuf.mvOut.size() == 0) {
        MY_LOGE("deqBuf is empty");
        return;
    }

    // 3a tuning
    halMetadata = result3A.halMeta;

    // append
    halMetadata += resultAppend;

    // in hal meta
    halMetadata += inHalMetadata;

    {
        IMetadata::IEntry entry(MTK_P1NODE_SENSOR_MODE);
        entry.push_back(mSensorParams.mode, Type2Type< MINT32 >());
        halMetadata.update(MTK_P1NODE_SENSOR_MODE, entry);
    }

    {
        IMetadata::IEntry entry(MTK_P1NODE_SENSOR_VHDR_MODE);
        entry.push_back(mSensorParams.vhdrMode, Type2Type< MINT32 >());
        halMetadata.update(MTK_P1NODE_SENSOR_VHDR_MODE, entry);
    }

    {
        IMetadata::IEntry entry(MTK_PIPELINE_FRAME_NUMBER);
        entry.push_back(act->appFrame->getFrameNo(), Type2Type< MINT32 >());
        halMetadata.update(MTK_PIPELINE_FRAME_NUMBER, entry);
    }

    //rrzo
    MUINT32 port_index = act->portBufIndex[P1_OUTPUT_PORT_RRZO];
    if (port_index != P1_PORT_BUF_IDX_NONE) {
        NSCam::NSIoPipe::NSCamIOPipe::ResultMetadata const * result =
            &(deqBuf.mvOut[port_index].mMetaData);
        if (result == NULL) {
            MY_LOGE("CANNOT get result at (%d) for (%d)", port_index, index);
            return;
        }
        //crop region
        MRect crop = result->mCrop_s;
        MBOOL bIsBinEn = (act->refBinSize == mSensorParams.size) ?
            MFALSE : MTRUE;
        {
            IMetadata::IEntry entry_br(MTK_P1NODE_BIN_CROP_REGION);
            entry_br.push_back(result->mCrop_s, Type2Type< MRect >());
            halMetadata.update(MTK_P1NODE_BIN_CROP_REGION, entry_br);
            IMetadata::IEntry entry_bs(MTK_P1NODE_BIN_SIZE);
            entry_bs.push_back(act->refBinSize, Type2Type< MSize >());
            halMetadata.update(MTK_P1NODE_BIN_SIZE, entry_bs);
            //
            if (bIsBinEn) {
                BIN_REVERT(crop.p.x);
                BIN_REVERT(crop.p.y);
                BIN_REVERT(crop.s.w);
                BIN_REVERT(crop.s.h);
            }
            IMetadata::IEntry entry(MTK_P1NODE_SCALAR_CROP_REGION);
            entry.push_back(crop, Type2Type< MRect >());
            halMetadata.update(MTK_P1NODE_SCALAR_CROP_REGION, entry);
        }
        //
        {
            IMetadata::IEntry entry(MTK_P1NODE_DMA_CROP_REGION);
            entry.push_back(result->mCrop_d, Type2Type< MRect >());
            halMetadata.update(MTK_P1NODE_DMA_CROP_REGION, entry);
        }
        //
        {
            IMetadata::IEntry entry(MTK_P1NODE_RESIZER_SIZE);
            entry.push_back(result->mDstSize, Type2Type< MSize >());
            halMetadata.update(MTK_P1NODE_RESIZER_SIZE, entry);
        }
        //
        MINT32 quality = MTK_P1_RESIZE_QUALITY_LEVEL_UNKNOWN;
        {
            if (result->eIQlv == eCamIQ_L) {
                quality = MTK_P1_RESIZE_QUALITY_LEVEL_L;
            } else if (result->eIQlv == eCamIQ_H) {
                quality = MTK_P1_RESIZE_QUALITY_LEVEL_H;
            }
            IMetadata::IEntry entry(MTK_P1NODE_RESIZE_QUALITY_LEVEL);
            entry.push_back(quality, Type2Type< MINT32 >());
            halMetadata.update(MTK_P1NODE_RESIZE_QUALITY_LEVEL, entry);
        }
        MY_LOGI3("[CropInfo] Bin(%d) Sensor" P1_SIZE_STR "ActRef" P1_SIZE_STR
            "CROP_REGION" P1_RECT_STR "CropS" P1_RECT_STR "CropD" P1_RECT_STR
            "DstSize" P1_SIZE_STR "- [BinQty] QUALITY_LEVEL(%d) IQlv(%d)",
            bIsBinEn, P1_SIZE_VAR(mSensorParams.size),
            P1_SIZE_VAR(act->refBinSize), P1_RECT_VAR(crop),
            P1_RECT_VAR(result->mCrop_s), P1_RECT_VAR(result->mCrop_d),
            P1_SIZE_VAR(result->mDstSize), quality, result->eIQlv);
    }
    //
    {
        MINT64 timestamp =
            deqBuf.mvOut[index].mMetaData.mTimeStamp;
        IMetadata::IEntry entry(MTK_P1NODE_FRAME_START_TIMESTAMP);
        entry.push_back(timestamp, Type2Type< MINT64 >());
        halMetadata.update(MTK_P1NODE_FRAME_START_TIMESTAMP, entry);
    }

    {
        MINT64 timestamp_boot =
            deqBuf.mvOut[index].mMetaData.mTimeStamp_B;
        IMetadata::IEntry entry(MTK_P1NODE_FRAME_START_TIMESTAMP_BOOT);
        entry.push_back(timestamp_boot, Type2Type< MINT64 >());
        halMetadata.update(MTK_P1NODE_FRAME_START_TIMESTAMP_BOOT, entry);
    }
    //
    if ((mIsDynamicTwinEn) && (mpCamIO != NULL)) {
        MBOOL ret = MFALSE;
        MINT32 status = MTK_P1_TWIN_STATUS_NONE;
        NSCam::NSIoPipe::NSCamIOPipe::E_CamHwPathCfg curCfg = eCamHwPathCfg_Num;
        ret = mpCamIO->sendCommand(ENPipeCmd_GET_HW_PATH_CFG,
            (MINTPTR)(&curCfg), (MINTPTR)NULL, (MINTPTR)NULL);
        if (ret) {
            switch (curCfg) {
                case eCamHwPathCfg_One_TG:
                    status = MTK_P1_TWIN_STATUS_TG_MODE_1;
                    break;
                case eCamHwPathCfg_Two_TG:
                    status = MTK_P1_TWIN_STATUS_TG_MODE_2;
                    break;
                //case eCamHwPathCfg_Num:
                default:
                    MY_LOGI0("CamHwPathCfg_Num(%d) not defined", curCfg);
                    break;
            }
            IMetadata::IEntry entry(MTK_P1NODE_TWIN_STATUS);
            entry.push_back(status, Type2Type< MINT32 >());
            halMetadata.update(MTK_P1NODE_TWIN_STATUS, entry);
        } else {
            MY_LOGI0("cannot get ENPipeCmd_GET_HW_PATH_CFG (%d)", ret);
        }
        MY_LOGI3("(%d)=GET_HW_PATH_CFG(%d) TWIN_STATUS[%d] @ (%d)(%d:%d)", ret,
            curCfg, status, act->magicNum, act->frmNum, act->reqNum);
    }
    //
    MINT32 qtyStatus = MTK_P1_RESIZE_QUALITY_STATUS_NONE;
    if (act->qualitySwitchState != QUALITY_SWITCH_STATE_NONE) {
        switch (act->qualitySwitchState) {
            case QUALITY_SWITCH_STATE_DONE_ACCEPT:
                qtyStatus = MTK_P1_RESIZE_QUALITY_STATUS_ACCEPT;
                break;
            case QUALITY_SWITCH_STATE_DONE_IGNORE:
                qtyStatus = MTK_P1_RESIZE_QUALITY_STATUS_IGNORE;
                break;
            case QUALITY_SWITCH_STATE_DONE_REJECT:
                qtyStatus = MTK_P1_RESIZE_QUALITY_STATUS_REJECT;
                break;
            case QUALITY_SWITCH_STATE_DONE_ILLEGAL:
                qtyStatus = MTK_P1_RESIZE_QUALITY_STATUS_ILLEGAL;
                break;
            default:
                break;
        }
        IMetadata::IEntry entry(MTK_P1NODE_RESIZE_QUALITY_STATUS);
            entry.push_back(qtyStatus, Type2Type< MINT32 >());
            halMetadata.update(MTK_P1NODE_RESIZE_QUALITY_STATUS, entry);
    }
    //
    MBOOL qtySwitch = getQualitySwitching();
    {
        IMetadata::IEntry entry(MTK_P1NODE_RESIZE_QUALITY_SWITCHING);
            entry.push_back(qtySwitch, Type2Type< MBOOL >());
            halMetadata.update(MTK_P1NODE_RESIZE_QUALITY_SWITCHING, entry);
    }
    //
    MY_LOGI3("QUALITY_STATUS[%d](%d) - QUALITY_SWITCHING[%d] - " P1NUM_ACT_STR,
        qtyStatus, act->qualitySwitchState, qtySwitch, P1NUM_ACT_VAR(*act));
    //
    U_if (act->isRawTypeChanged) {
        MINT32 rawType = act->fullRawType;
        IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
        entry.push_back(rawType, Type2Type< MINT32 >());
        halMetadata.update(MTK_P1NODE_RAW_TYPE, entry);
        MY_LOGI0("MTK_P1NODE_RAW_TYPE(%d) - full raw type change - "
            P1NUM_ACT_STR, rawType, P1NUM_ACT_VAR(*act));
    }


}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
setupAction(
    P1QueAct & rAct,
    QBufInfo & info
)
{
    FUNCTION_P1_IN;
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
#if SUPPORT_ISP
    MUINT32 out = 0;
    //
    sp<IImageStreamInfo> pImgStreamInfo = NULL;
    sp<IImageBuffer> pImgBuf = NULL;
    //
    NSCam::NSIoPipe::PortID portID = NSCam::NSIoPipe::PortID();
    MSize dstSize = MSize(0, 0);
    MRect cropRect = MRect(MPoint(0, 0), MSize(0, 0));
    MUINT32 rawOutFmt = 0;
    //
    STREAM_IMG streamImg = STREAM_IMG_NUM;
    //
    if ((act->reqType == REQ_TYPE_UNKNOWN) ||
        (act->reqType == REQ_TYPE_REDO) ||
        (act->reqType == REQ_TYPE_YUV)) {
        MY_LOGW("mismatch act type " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
        return BAD_VALUE;
    }
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:setup|Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
        act->magicNum, act->sofIdx, act->frmNum, act->reqNum);
    //
    #if (IS_P1_LOGI)
    android::String8 strInfo("");
    if (LOGI_LV1) {
        strInfo += String8::format("[Exp:%" PRId64 "ns] ",
            act->frameExpDuration);
        strInfo += String8::format("[MetaCtrl]");
        if (act->haveMetaScalerCrop) {
            strInfo += String8::format("[SCALER_CROP(%d) " P1_RECT_STR "]",
                act->haveMetaScalerCrop, P1_RECT_VAR(act->rectMetaScalerCrop));
        } else {
            strInfo += String8::format("[SCALER_CROP(%d)]",
                act->haveMetaScalerCrop);
        }
        if (act->haveMetaSensorCrop) {
            strInfo += String8::format("[SENSOR_CROP(%d) " P1_RECT_STR "]",
                act->haveMetaSensorCrop, P1_RECT_VAR(act->rectMetaSensorCrop));
        } else {
            strInfo += String8::format("[SENSOR_CROP(%d)]",
                act->haveMetaSensorCrop);
        }
        if (act->haveMetaResizerSet) {
            strInfo += String8::format("[RESIZER_SET(%d) " P1_SIZE_STR "]",
                act->haveMetaResizerSet, P1_SIZE_VAR(act->sizeMetaResizerSet));
        } else {
            strInfo += String8::format("[RESIZER_SET(%d)]",
                act->haveMetaResizerSet);
        }
    }
    #endif
    //
    for (out = 0; out < REQ_OUT_MAX; out++) {
        if (!(IS_OUT(out, act->reqOutSet))) {
            continue;
        }
        P1_TRACE_F_BEGIN(SLG_I, "REQ_OUT_%d", out);
        //pBufPool = NULL;
        pImgStreamInfo = NULL;
        //pImgStreamBuf = NULL;
        pImgBuf = NULL;
        streamImg = STREAM_IMG_NUM;
        switch (out) {
            case REQ_OUT_LCSO:
            case REQ_OUT_LCSO_STUFF:
                streamImg = STREAM_IMG_OUT_LCS;
                portID = PORT_LCSO;
                dstSize = mvStreamImg[streamImg]->getImgSize();
                cropRect = MRect(MPoint(0, 0),
                    mvStreamImg[streamImg]->getImgSize());
                rawOutFmt = (EPipe_PROCESSED_RAW);
                if (out == REQ_OUT_LCSO_STUFF) {
                    // not use stuff buffer with height:1
                    cropRect.s = dstSize;
                }
                break;

            case REQ_OUT_RSSO:
            case REQ_OUT_RSSO_STUFF:
                streamImg = STREAM_IMG_OUT_RSS;
                portID = PORT_RSSO;
                dstSize = mvStreamImg[streamImg]->getImgSize();
                cropRect = MRect(MPoint(0, 0),
                    mvStreamImg[streamImg]->getImgSize());
                rawOutFmt = (EPipe_PROCESSED_RAW);
                if (out == REQ_OUT_RSSO_STUFF) {
                    // not use stuff buffer with height:1
                    cropRect.s = dstSize;
                }
                break;

            case REQ_OUT_RESIZER:
            case REQ_OUT_RESIZER_STUFF:
                streamImg = STREAM_IMG_OUT_RESIZE;
                portID = PORT_RRZO;
                dstSize = act->dstSize_resizer;
                cropRect = act->cropRect_resizer;
                rawOutFmt = (EPipe_PROCESSED_RAW);
                if (out == REQ_OUT_RESIZER_STUFF) {
                    // use stuff buffer with height:1
                    dstSize.h = P1_STUFF_BUF_HEIGHT(MTRUE, mConfigPort);
                    cropRect.s = dstSize;
                }
                break;

            case REQ_OUT_FULL_PROC:
            case REQ_OUT_FULL_PURE:
            case REQ_OUT_FULL_OPAQUE:
            case REQ_OUT_FULL_STUFF:
                streamImg = STREAM_IMG_OUT_FULL;
                if (out == REQ_OUT_FULL_OPAQUE || (out == REQ_OUT_FULL_STUFF &&
                    act->streamBufImg[STREAM_IMG_OUT_OPAQUE].bExist)) {
                    streamImg = STREAM_IMG_OUT_OPAQUE;
                } else if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL) {
                    streamImg = STREAM_IMG_OUT_FULL;
                } else if (mvStreamImg[STREAM_IMG_OUT_OPAQUE] != NULL) {
                    streamImg = STREAM_IMG_OUT_OPAQUE;
                };
                portID = PORT_IMGO;
                dstSize = act->dstSize_full;
                cropRect = act->cropRect_full;
                /*
                rawOutFmt = (MUINT32)(((out == REQ_OUT_FULL_PROC) ||
                            ((out == REQ_OUT_FULL_STUFF ||
                            out == REQ_OUT_FULL_OPAQUE) &&
                            (act->fullRawType == EPipe_PROCESSED_RAW))) ?
                            (EPipe_PROCESSED_RAW) : (EPipe_PURE_RAW));
                */
                rawOutFmt = act->fullRawType;
                //
                if (out == REQ_OUT_FULL_STUFF) {
                    if ((mEnableDumpRaw || mCamDumpEn) &&
                        act->reqType == REQ_TYPE_NORMAL) {
                        // If user wants to dump pure raw, Full Raw can not use height 1
                    } else {
                        dstSize.h = P1_STUFF_BUF_HEIGHT(MFALSE, mConfigPort);
                    }
                    cropRect.s = dstSize;
                };
                break;

            //case REQ_OUT_MAX:
            // for this loop, all cases should be listed
            // and the default is an unreachable path
            /*
            default:
                continue;
            */
        };
        //
        if (streamImg < STREAM_IMG_NUM) {
            pImgStreamInfo = mvStreamImg[streamImg];
        } else {
            MY_LOGW("cannot find the StreamImg num:%d out:%d "
                "streamImg:%d", act->magicNum, out, streamImg);
            return BAD_VALUE;
        }
        if (pImgStreamInfo == NULL) {
            MY_LOGW("cannot find the ImgStreamInfo num:%d out:%d "
                "streamImg:%d", act->magicNum, out, streamImg);
            return BAD_VALUE;
        }
        //
        MERROR err = OK;
        if (out == REQ_OUT_FULL_STUFF || out == REQ_OUT_RESIZER_STUFF ||
            out == REQ_OUT_LCSO_STUFF || out == REQ_OUT_RSSO_STUFF) {
            err = act->stuffImageGet(streamImg, dstSize, pImgBuf);
        } else if (act->reqType == REQ_TYPE_INITIAL) {
            // the initial act with the pool, it do not use the stuff buffer
            err = act->poolImageGet(streamImg, pImgBuf);
        } else { // REQ_TYPE_NORMAL
            if (OK != act->frameImageGet(streamImg, pImgBuf)) {
                #if 1 // keep en-queue/de-queue processing
                if (out == REQ_OUT_LCSO || out == REQ_OUT_RSSO ||
                    ((mEnableDumpRaw || mCamDumpEn) &&
                    (out == REQ_OUT_FULL_PURE ||
                    out == REQ_OUT_FULL_PROC || out == REQ_OUT_FULL_OPAQUE))) {
                    MY_LOGI0("keep the output size out:%d", out);
                } else {
                    dstSize.h = P1_STUFF_BUF_HEIGHT(
                        (out == REQ_OUT_RESIZER ? MTRUE : MFALSE), mConfigPort);
                    cropRect.s.h = dstSize.h;
                };
                err = act->stuffImageGet(streamImg, dstSize, pImgBuf);
                if (out == REQ_OUT_RESIZER) {
                    act->expRec |= EXP_REC(EXP_EVT_NOBUF_RRZO);
                } else if (out == REQ_OUT_LCSO) {
                    act->expRec |= EXP_REC(EXP_EVT_NOBUF_LCSO);
                } else if (out == REQ_OUT_RSSO) {
                    act->expRec |= EXP_REC(EXP_EVT_NOBUF_RSSO);
                } else {
                    act->expRec |= EXP_REC(EXP_EVT_NOBUF_IMGO);
                }
                MY_LOGI0("underway-stuff-buffer status(%d) out[%s](%d) "
                    "stream(%#" PRIx64 ") " P1INFO_ACT_STR, err,
                    P1_PORT_TO_STR(portID), out, pImgStreamInfo->getStreamId(),
                    P1INFO_ACT_VAR(*act));
                #else
                MY_LOGE("(%d) frameImageGet failed on StreamId=0x%X",
                    act->magicNum, pImgStreamInfo->getStreamId());
                err = BAD_VALUE;
                #endif
            };
        }
        //
        U_if ((pImgBuf == NULL) || (err != OK)) {
            MY_LOGE("Cannot get ImgBuf status(%d) out[%s](%d)" P1INFO_ACT_STR,
                err, P1_PORT_TO_STR(portID), out, P1INFO_ACT_VAR(*act));
            mLogInfo.inspect(LogInfo::IT_BUFFER_EXCEPTION);
            return BAD_VALUE;
        }

        MBOOL secureOn = mvStreamImg[streamImg]->getSecureInfo();
        //
        if ((out == REQ_OUT_RESIZER || out == REQ_OUT_RESIZER_STUFF) ||
            (out == REQ_OUT_FULL_PURE || out == REQ_OUT_FULL_PROC ||
            out == REQ_OUT_FULL_OPAQUE || out == REQ_OUT_FULL_STUFF)) {
            sp<IImageBufferHeap> pHeap = pImgBuf->getImageBufferHeap();
            if (pHeap != NULL) {
                pHeap->setColorArrangement((MINT32)mSensorFormatOrder);
            }
        }
        //
        #if SUPPORT_CONFIRM_BUF_PA
        U_if ((pImgBuf->getBufPA(0) == 0x0)
            #if SUPPORT_CONFIRM_BUF_PA_VA
            || (pImgBuf->getBufVA(0) == 0x0)
            #endif
        ) {
            MY_LOGE("Cannot get BufAddr out[%s](%d) P(%p) V(%p)" P1INFO_ACT_STR,
                P1_PORT_TO_STR(portID), out, (void*)pImgBuf->getBufPA(0),
                (void*)pImgBuf->getBufVA(0), P1INFO_ACT_VAR(*act));
            mLogInfo.inspect(LogInfo::IT_BUFFER_EXCEPTION);
            return BAD_VALUE;
        }
        #endif
        //
        #if (IS_P1_LOGI)
        if (LOGI_LV1) {
            strInfo += String8::format(
                "[%s][%d](x%x)"
                "(Buf)(%dx%d)(S:%d:%d P:%p V:%p F:0x%x)"
                "(Crop)(%d,%d-%dx%d)(%dx%d) ",
                P1_PORT_TO_STR(portID), out, rawOutFmt,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                (int)pImgBuf->getBufStridesInBytes(0),
                (int)pImgBuf->getBufSizeInBytes(0),
                (void*)pImgBuf->getBufPA(0), (void*)pImgBuf->getBufVA(0),
                pImgBuf->getImgFormat(),
                cropRect.p.x, cropRect.p.y, cropRect.s.w, cropRect.s.h,
                dstSize.w, dstSize.h
                );
        }
        #endif
        NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo(
            portID,
            pImgBuf.get(),
            dstSize,
            cropRect,
            act->magicNum,
            act->sofIdx,
            rawOutFmt);

        if (mEnableSecure) {
            rBufInfo.mSize = pImgBuf->getBufSizeInBytes(0);
            rBufInfo.mVa = pImgBuf->getBufVA(0);
            rBufInfo.mPa = pImgBuf->getBufPA(0);
            rBufInfo.mMemID = pImgBuf->getFD(0);
            rBufInfo.mSecon = secureOn;
            MY_LOGD2("mSize : %d, mVa : %p, mPa : %p, mMemID : %d secureOn: %d",
                rBufInfo.mSize, (void*)rBufInfo.mVa, (void*)rBufInfo.mPa,
                rBufInfo.mMemID, rBufInfo.mSecon);
        }
        info.mvOut.push_back(rBufInfo);
        P1_TRACE_C_END(SLG_I); // "REQ_OUT"
    }; // end of the loop for each out
    //
    {
        //MSize dstSizeNone = MSize(0, 0);
        //MRect cropRectNone = MRect(MPoint(0, 0), MSize(0, 0));
        // EISO
        if (IS_PORT(CONFIG_PORT_EISO, mConfigPort)) {
            sp<IImageBuffer> pImgBuf = NULL;
            if (IS_LMV(mpConnectLMV)) {
                mpConnectLMV->getBuf(pImgBuf);
            }
            if (pImgBuf == NULL) {
                MY_LOGE("(%d) Cannot get LMV buffer", act->magicNum);
                return BAD_VALUE;
            }
            //MY_LOGD1("GetBufLMV: %p ",pImgBuf->getBufVA(0));
            #if SUPPORT_CONFIRM_BUF_PA
            U_if ((pImgBuf->getBufPA(0) == 0x0)
                #if SUPPORT_CONFIRM_BUF_PA_VA
                || (pImgBuf->getBufVA(0) == 0x0)
                #endif
            ) {
                MY_LOGE("Cannot get LMV out[%s](%d) P(%p) V(%p)" P1INFO_ACT_STR,
                    P1_PORT_TO_STR(PORT_EISO), out, (void*)pImgBuf->getBufPA(0),
                    (void*)pImgBuf->getBufVA(0), P1INFO_ACT_VAR(*act));
                return BAD_VALUE;
            }
            #endif
            act->buffer_eiso = pImgBuf;
            #if (IS_P1_LOGI)
            if (LOGI_LV1) {
                if (pImgBuf != NULL) {
                    strInfo += String8::format("[LMV](P:%p V:%p) ",
                        (void*)pImgBuf->getBufPA(0),
                        (void*)pImgBuf->getBufVA(0));
                }
            }
            #endif
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo(
                PORT_EISO,
                pImgBuf.get(),
                pImgBuf->getImgSize(),
                MRect(MPoint(0, 0), pImgBuf->getImgSize()),
                act->magicNum,
                act->sofIdx);
            info.mvOut.push_back(rBufInfo);
        }
    }
    mTagEnq.set(rAct.getNum());
    #if (IS_P1_LOGI)
    if (LOGI_LV1) {
        P1_TRACE_F_BEGIN(SLG_PFL, "P1::ENQ_LOG|Mnum:%d SofIdx:%d Fnum:%d "
            "Rnum:%d FlushSet:0x%x", act->magicNum, act->sofIdx, act->frmNum,
            act->reqNum, act->flushSet);
        P1_LOGI(1, "[P1::ENQ]" P1INFO_ACT_STR " %s",
            P1INFO_ACT_VAR(*act), strInfo.string());
        P1_TRACE_C_END(SLG_PFL); // "P1::ENQ_LOG"
    };
    #endif
    //
    P1_TRACE_C_END(SLG_I); // "P1:setup"
#endif
    FUNCTION_P1_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_enque(
    P1QueJob & job,
    ENQ_TYPE type,
    MBOOL ready, // true : use the ready QBufInfo, without setupAction()
    MINT64 data
)
{
    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_I, "P1:enque");

    if (!isActive()) {
        return BAD_VALUE;
    }
    if (mpCamIO == NULL) {
        MY_LOGE("NormalPipe is NULL");
        return BAD_VALUE;
    }
    MY_LOGD3("EnQ[%d](%" PRId64 ") @ (%d)", type, data, job.getIdx());
    MBOOL toPush = (type == ENQ_TYPE_INITIAL) ? MFALSE : MTRUE;
    MBOOL toSwitchUNI = MFALSE;
    MUINT8 toSwtTgNum = 0;
    MUINT32 toSwitchQuality = QUALITY_SWITCH_STATE_NONE;
#if SUPPORT_ISP
    //
    QBufInfo enBuf;
    QBufInfo* pEnBuf = &enBuf;
    if (ready) { /*((type == ENQ_TYPE_INITIAL) && (!EN_INIT_REQ_RUN))*/
        if (mpConCtrl == NULL || (!mpConCtrl->initBufInfo_get(&pEnBuf))
            || pEnBuf == NULL) {
            MY_LOGE("CANNOT get the initBufInfo");
            return BAD_VALUE;
        }
    } else {
        for (size_t i = 0; i < job.size(); i++) {
            MY_LOGD2("job(%d)(%zu/%zu)", job.getIdx(), i, job.size());
            P1QueAct & rAct = job.edit(i);
            P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
            if (OK != setupAction(rAct, enBuf)) {
                MY_LOGE("setup enque act fail");
                return BAD_VALUE;
            }
            if (i == 0 && act->reqType == REQ_TYPE_NORMAL) {
                if (type == ENQ_TYPE_DIRECTLY) {
                    act->frameExpDuration = data * ONE_US_TO_NS;
                }
                enBuf.mShutterTimeNs = act->frameExpDuration;
            }
            if (act->uniSwitchState == UNI_SWITCH_STATE_REQ) {
                toSwitchUNI = MTRUE;
            }
            if (act->tgSwitchState == TG_SWITCH_STATE_REQ) {
                toSwtTgNum = act->tgSwitchNum;
            }
            if ((act->qualitySwitchState & QUALITY_SWITCH_STATE_REQ_NON) > 0) {
                toSwitchQuality = act->qualitySwitchState;
            }
            act->exeState = EXE_STATE_PROCESSING;
        }
    }
    //
    if (EN_START_CAP_RUN && (!isReady()) && (type == ENQ_TYPE_NORMAL)) {
        Mutex::Autolock _l(mStartCaptureLock);
        MUINT32 cnt = 0;
        status_t res = NO_ERROR;
        MY_LOGD2("StartCaptureState(%d) Cnt(%d)", mStartCaptureState, cnt);
        while (mStartCaptureState == START_CAP_STATE_WAIT_CB) {
            P1_TRACE_F_BEGIN(SLG_S, "StartCapture wait [%d]", cnt);
            res = mStartCaptureCond.waitRelative(
                mStartCaptureLock, P1_CAPTURE_CHECK_INV_NS);
            P1_TRACE_C_END(SLG_S); // "StartCapture wait"
            if (res != NO_ERROR) {
                MY_LOGI0("StartCap(%d) Cnt(%d) Res(%d)", mStartCaptureState,
                    cnt, res);
                mLogInfo.inspect(LogInfo::IT_WAIT_CATURE);
            } else {
                break;
            }
        }
        P1Act act = GET_ACT_PTR(act, job.edit(0), BAD_VALUE);
        act->capType = mStartCaptureType;
        act->frameExpDuration = mStartCaptureExp;
        act->sofIdx = mStartCaptureIdx;
        for (size_t i = 0; i < pEnBuf->mvOut.size(); i++) {
            pEnBuf->mvOut[i].FrameBased.mSOFidx = mStartCaptureIdx;
        }
        pEnBuf->mShutterTimeNs = mStartCaptureExp;
    }
    //
    if (toSwitchUNI) {
        UNI_SWITCH_STATE uniState = UNI_SWITCH_STATE_REQ;
        MUINT32 switchState = 0;
        MBOOL res = MFALSE;
        if (mpCamIO->sendCommand(ENPipeCmd_GET_UNI_SWITCH_STATE,
            (MINTPTR)(&switchState), (MINTPTR)NULL, (MINTPTR)NULL) &&
            switchState == 0) { // DRV: If switch state is NULL, then do switch.
            res = mpCamIO->sendCommand(ENPipeCmd_UNI_SWITCH,
                (MINTPTR)NULL, (MINTPTR)NULL, (MINTPTR)NULL);
            if (res) {
                uniState = UNI_SWITCH_STATE_ACT_ACCEPT;
            } else {
                uniState = UNI_SWITCH_STATE_ACT_IGNORE;
            }
        } else {
            uniState = UNI_SWITCH_STATE_ACT_REJECT;
        }
        //
        for (size_t i = 0; i < job.size(); i++) {
            P1Act act = GET_ACT_PTR(act, job.edit(i), BAD_VALUE);
            if (act->uniSwitchState == UNI_SWITCH_STATE_REQ) {
                act->uniSwitchState = uniState;
                MY_LOGD0("UNI-Switch(%d)(%d,%d) drv(%d,%d):(%d)", act->magicNum,
                    act->frmNum, act->reqNum, switchState, res, uniState);
            }
        }
    }
    //
    if (toSwtTgNum) {
        TG_SWITCH_STATE tgState = TG_SWITCH_STATE_DONE_IGNORE;
        MBOOL res = MFALSE;
        MBOOL ret = MFALSE;
        MBOOL rev = MFALSE;
        MBOOL isOn = MFALSE;
        NSCam::NSIoPipe::NSCamIOPipe::E_CamHwPathCfg curCfg = eCamHwPathCfg_Num;
        NSCam::NSIoPipe::NSCamIOPipe::E_CamHwPathCfg tarCfg = eCamHwPathCfg_Num;
        switch (toSwtTgNum) {
            case 1:
                tarCfg = NSCam::NSIoPipe::NSCamIOPipe::eCamHwPathCfg_One_TG;
                break;
            case 2:
                tarCfg = NSCam::NSIoPipe::NSCamIOPipe::eCamHwPathCfg_Two_TG;
                break;
            default:
                MY_LOGI0("check act TG state num (%d)", toSwtTgNum);
                break;
        }
        if (mpCamIO != NULL) {
            res = mpCamIO->sendCommand (ENPipeCmd_GET_DTwin_INFO,
                (MINTPTR)(&isOn), (MINTPTR)NULL, (MINTPTR)NULL);
            if (res && isOn) {
                ret = mpCamIO->sendCommand(ENPipeCmd_GET_HW_PATH_CFG,
                    (MINTPTR)(&curCfg), (MINTPTR)NULL, (MINTPTR)NULL);
            }
            if (!res) {
                MY_LOGI0("sendCmd ENPipeCmd_GET_DTwin_INFO (%d)", res);
            } else if (!isOn) {
                MY_LOGI0("DynamicTwin not ready (%d)", isOn);
            } else if (!ret) {
                MY_LOGI0("sendCmd ENPipeCmd_GET_HW_PATH_CFG (%d)", ret);
            } else if (curCfg == eCamHwPathCfg_Num) {
                MY_LOGI0("check current num (%d)", curCfg);
            } else if (tarCfg == eCamHwPathCfg_Num) {
                MY_LOGI0("check target num (%d)", tarCfg);
            } else if (curCfg == tarCfg) {
                MY_LOGI0("CamHwPathCfg is ready (%d) == (%d)", curCfg, tarCfg);
            } else {
                rev = mpCamIO->sendCommand(ENPipeCmd_SET_HW_PATH_CFG,
                    (MINTPTR)(tarCfg), (MINTPTR)NULL, (MINTPTR)NULL);
                if (!rev) {
                    MY_LOGI0("sendCmd ENPipeCmd_SET_HW_PATH_CFG (%d)", rev);
                    tgState = TG_SWITCH_STATE_DONE_REJECT;
                } else {
                    tgState = TG_SWITCH_STATE_DONE_ACCEPT;
                }
            }
        }
        //
        for (size_t i = 0; i < job.size(); i++) {
            P1Act act = GET_ACT_PTR(act, job.edit(i), BAD_VALUE);
            if (act->tgSwitchState == TG_SWITCH_STATE_REQ) {
                act->tgSwitchState = tgState;
                act->tgSwitchNum = 0;
                MY_LOGI0("TG(%d)(%d,%d) Drv(%d) Swt(%d)(%d,%d)(%d,%d,%d):%d",
                    act->magicNum, act->frmNum, act->reqNum,
                    isOn, toSwtTgNum, curCfg, tarCfg, res, ret, rev, tgState);
            }
        }
    }
    //
    if (toSwitchQuality != QUALITY_SWITCH_STATE_NONE) {
        QUALITY_SWITCH_STATE switchQuality = QUALITY_SWITCH_STATE_DONE_REJECT;
        MBOOL ret = MFALSE;
        if (mpCamIO != NULL && mpRegisterNotify != NULL) {
            E_CamIQLevel CamLvA = eCamIQ_L;
            E_CamIQLevel CamLvB = eCamIQ_L;
            if ((toSwitchQuality & QUALITY_SWITCH_STATE_REQ_H_A) > 0) {
                CamLvA = eCamIQ_H;
            }
            if ((toSwitchQuality & QUALITY_SWITCH_STATE_REQ_H_B) > 0) {
                CamLvB = eCamIQ_H;
            }
            ret = mpCamIO->sendCommand(ENPipeCmd_SET_QUALITY,
                    (MINTPTR)(mpRegisterNotify->getNotifyQuality()),
                    (MINTPTR)CamLvA, (MINTPTR)CamLvB);
            if (!ret) {
                MY_LOGI0("sendCommand ENPipeCmd_SET_QUALITY fail(%d)", ret);
                switchQuality = QUALITY_SWITCH_STATE_DONE_REJECT;
                setQualitySwitching(MFALSE);
            } else {
                switchQuality = QUALITY_SWITCH_STATE_DONE_ACCEPT;
            }
        }
        for (size_t i = 0; i < job.size(); i++) {
            P1Act act = GET_ACT_PTR(act, job.edit(i), BAD_VALUE);
            if ((act->qualitySwitchState & QUALITY_SWITCH_STATE_REQ_NON) > 0) {
                MY_LOGI0("ResizeQ (%d)(%d,%d) Ret(%d) QualitySwt(%d => %d)",
                    act->magicNum, act->frmNum, act->reqNum,
                    ret, act->qualitySwitchState, switchQuality);
                act->qualitySwitchState = switchQuality;
            }
        }
    }
    //
    if (toPush) {
        Mutex::Autolock _l(mProcessingQueueLock);
        mProcessingQueue.push_back(job);
        MY_LOGD2("Push(%d) to ProQ(%zu)",
            job.getIdx(), mProcessingQueue.size());
        P1_ATOMIC_RELEASE;
    }
    //
    MBOOL isErr = MFALSE;
    P1Act act = GET_ACT_PTR(act, job.edit(0), BAD_VALUE);
    MINT32 numF = act->frmNum;
    MINT32 numR = act->reqNum;
    // check CtrlSync before EnQ
    if ((IS_BURST_OFF) && // exclude burst mode
        (type != ENQ_TYPE_INITIAL) && (job.size() >= 1)) {
        attemptCtrlSync(job.edit(0));
    }
    //
    if ((mspSyncHelper != NULL) && (type != ENQ_TYPE_INITIAL)) {
        IMetadata ctrlMeta;
        if (act->reqType == REQ_TYPE_NORMAL &&
            act->streamBufMeta[STREAM_META_IN_HAL].bExist) {
            act->frameMetadataGet(STREAM_META_IN_HAL, &ctrlMeta);
        } else {
            ctrlMeta.clear(); // use a empty meta for dummy request
        }
        mspSyncHelper->syncEnqHW(getOpenId(), &ctrlMeta);
    }
    //
    if (type == ENQ_TYPE_DIRECTLY) {
        P1_TRACE_F_BEGIN(SLG_E, "P1:DRV-resume|"
            "Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
            act->magicNum, act->sofIdx, numF, numR);
        MY_LOGI0("mpCamIO->resume +++");
        if(!mpCamIO->resume((QBufInfo const *)(pEnBuf))) {
            MY_LOGE("[SUS-RES] DRV resume fail");
            if (mpHwStateCtrl != NULL) {
                mpHwStateCtrl->dump();
            }
            isErr = MTRUE;
        }
        MY_LOGI0("mpCamIO->resume ---");
        P1_TRACE_C_END(SLG_E); // "P1:DRV-resume"
    } else { // ENQ_TYPE_NORMAL / ENQ_TYPE_INITIAL
        mLogInfo.setMemo(LogInfo::CP_ENQ_BGN,
            act->magicNum, numF, numR, act->sofIdx);
        P1_TRACE_F_BEGIN(SLG_I, "P1:DRV-enque|"
            "Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
            act->magicNum, act->sofIdx, numF, numR);
        MY_LOGD2("mpCamIO->enque +++");
        if(!mpCamIO->enque(*pEnBuf)) {
            MY_LOGE("DRV-enque fail");
            isErr = MTRUE;
        }
        MY_LOGD2("mpCamIO->enque ---");
        P1_TRACE_C_END(SLG_I); // "P1:DRV-enque"
        mLogInfo.setMemo(LogInfo::CP_ENQ_END,
            act->magicNum, numF, numR, act->sofIdx);
    }
    //
    if (isErr) {
        if (toPush) {
            Mutex::Autolock _l(mProcessingQueueLock);
            Que_T::iterator it = mProcessingQueue.begin();
            for (; it != mProcessingQueue.end(); it++) {
                if ((*it).getIdx() == job.getIdx()) {
                    break;
                }
            }
            if (it != mProcessingQueue.end()) {
                mProcessingQueue.erase(it);
            }
            MY_LOGD2("Erase(%d) from ProQ(%zu)",
                job.getIdx(), mProcessingQueue.size());
            P1_ATOMIC_RELEASE;
        }
        return BAD_VALUE;
    }
    //
    if (type == ENQ_TYPE_INITIAL) {
        mpConCtrl->initBufInfo_clean();
    }/* else if (type == ENQ_TYPE_DIRECTLY) {
        if (mpHwStateCtrl != NULL) {
            mpHwStateCtrl->checkThreadWeakup();
        }
    }*/
#endif
    FUNCTION_P1_OUT;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_deque(QBufInfo &deqBuf)
{

#if SUPPORT_ISP

    FUNCTION_P1_IN;
    P1_TRACE_AUTO(SLG_I, "P1:deque");

    if (!isActive()) {
        return BAD_VALUE;
    }
    //
    MY_LOGI2("HwLockDeqWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI2("HwLockDeqWait ---");
    //
    if (!isActive()) {
        return BAD_VALUE;
    }

    {
        // deque buffer, and handle frame and metadata
        MY_LOGD2("%" PRId64 ", %f", mDequeThreadProfile.getAvgDuration(), mDequeThreadProfile.getFps());
        QPortID PortID;
        if (IS_PORT(CONFIG_PORT_IMGO, mConfigPort)) {//(mvOutImage_full.size() > 0) {
            PortID.mvPortId.push_back(PORT_IMGO);
        }
        if (IS_PORT(CONFIG_PORT_RRZO, mConfigPort)) {//(mOutImage_resizer != NULL) {
            PortID.mvPortId.push_back(PORT_RRZO);
        }
        if (IS_PORT(CONFIG_PORT_EISO, mConfigPort)) {
            PortID.mvPortId.push_back(PORT_EISO);
        }
        if (IS_PORT(CONFIG_PORT_LCSO, mConfigPort)) {
            PortID.mvPortId.push_back(PORT_LCSO);
        }
        if (IS_PORT(CONFIG_PORT_RSSO, mConfigPort)) {
            PortID.mvPortId.push_back(PORT_RSSO);
        }

        // for mBurstNum: 4 and port: I+R+E+L, the buffer is as
        // [I1][I2][I3][I4][R1][R2][R3][R4][E1][E2][E3][E4][L1][L2][L3][L4]
        mDequeThreadProfile.pulse_down();
        //
        P1_TRACE_F_BEGIN(SLG_I, "P1:DRV-deque@[0x%X]", mConfigPort);
        mLogInfo.setMemo(LogInfo::CP_DEQ_BGN);
        MY_LOGD2("mpCamIO->deque +++");
        if(!mpCamIO->deque(PortID, deqBuf)) {
            if(isActive()) {
                MY_LOGE("DRV-deque fail");
                mLogInfo.setMemo(LogInfo::CP_DEQ_END, -1);
                P1_TRACE_C_END(SLG_I); // "P1:DRV-deque"
            } else {
                MY_LOGW("DRV-deque fail - after stop");
                mLogInfo.setMemo(LogInfo::CP_DEQ_END, -2);
                P1_TRACE_C_END(SLG_I); // "P1:DRV-deque"
                return OK;
            }
            P1_TRACE_C_END(SLG_I); // "P1:DRV-deque"
            AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/P1Node:ISP pass1 deque fail");
            return BAD_VALUE;
        }
        MY_LOGD2("mpCamIO->deque ---");
        mLogInfo.setMemo(LogInfo::CP_DEQ_END, (deqBuf.mvOut.size() > 0) ?
            (deqBuf.mvOut[0].mMetaData.mMagicNum_hal) : 0);
        P1_TRACE_C_END(SLG_I); // "P1:DRV-deque"
        //
        mDequeThreadProfile.pulse_up();
    }
    //
    if( mDebugScanLineMask != 0 &&
        mpDebugScanLine != NULL)
    {
        P1_TRACE_AUTO(SLG_E, "DrawScanLine");
        for(size_t i = 0; i < deqBuf.mvOut.size(); i++)
        {
            if( (   deqBuf.mvOut[i].mPortID.index == PORT_RRZO.index &&
                    mDebugScanLineMask & DRAWLINE_PORT_RRZO  ) ||
                (   deqBuf.mvOut[i].mPortID.index == PORT_IMGO.index &&
                    mDebugScanLineMask & DRAWLINE_PORT_IMGO  )   )
            {
                mpDebugScanLine->drawScanLine(
                                    deqBuf.mvOut[i].mBuffer->getImgSize().w,
                                    deqBuf.mvOut[i].mBuffer->getImgSize().h,
                                    (void*)(deqBuf.mvOut[i].mBuffer->getBufVA(0)),
                                    deqBuf.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                    deqBuf.mvOut[i].mBuffer->getBufStridesInBytes(0));

            }
        }
    }
    static bool shouldPrint = false;
    if (shouldPrint) {
        for(size_t i = 0; i < deqBuf.mvOut.size(); i++) {
            char filename[256] = {0};
            sprintf(filename, "/data/P1_%d_%d_%d.raw", deqBuf.mvOut.at(i).mMetaData.mMagicNum_hal,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().w,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().h);
            P1_TRACE_AUTO(SLG_E, filename);
            NSCam::Utils::saveBufToFile(filename, (unsigned char*)deqBuf.mvOut.at(i).mBuffer->getBufVA(0), deqBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0));
            shouldPrint = false;
        }
    }
    #if 1
    if (mEnableDumpRaw && deqBuf.mvOut.size() > 0) {
        MUINT32 magicNum = deqBuf.mvOut.at(0).mMetaData.mMagicNum_hal;

        /* Record previous "debug.p1.pureraw_dump" prop value.
        * When current prop value is not equal to previous prop value, it will start dump raw.
        * When current prop value is > 0 value, it will dump continuous raw.
        * For example, assume current prop value is 10 ,it will start continuous 10 raw dump.
        */
        static MINT32 prevDumpProp = 0;
        static MUINT32 continueDumpCount = 0;

        /* If current "debug.p1.pureraw_dump" prop value < 0, this variable will save it.
        * This variable is used to continuous magic number dump raws.
        * For example, assume current prop value is -20. When pipeline starts, it will dump frames with magic num < 20.
        */
        static MUINT32 indexRawDump = 0;

        MINT32 currentDumpProp = property_get_int32("vendor.debug.p1.pureraw_dump",0);

        if (prevDumpProp != currentDumpProp)
        {
            if(currentDumpProp == 0 ){
                prevDumpProp = 0;
                indexRawDump = 0;
                continueDumpCount = 0;
            } else if(currentDumpProp < 0){
                indexRawDump = (MUINT32)(-currentDumpProp);
            } else if(currentDumpProp > 0){
                continueDumpCount = (MUINT32)currentDumpProp;
            }
            prevDumpProp = currentDumpProp;
        }

        if ( (magicNum <= indexRawDump) || continueDumpCount > 0 )
        {
            if(continueDumpCount > 0)
                continueDumpCount--;

            for(size_t i = 0; i < deqBuf.mvOut.size(); i++) {
                char filename[256] = {0};
                sprintf(filename, "/sdcard/raw/p1_%u_%d_%04dx%04d_%04d_%d.raw",
                    magicNum,
                    ((deqBuf.mvOut.at(i).mPortID.index == PORT_RRZO.index) ?
                    (0) : (1)),
                    (int)deqBuf.mvOut.at(i).mBuffer->getImgSize().w,
                    (int)deqBuf.mvOut.at(i).mBuffer->getImgSize().h,
                    (int)deqBuf.mvOut.at(i).mBuffer->getBufStridesInBytes(0),
                    (int)mSensorFormatOrder );
                P1_TRACE_AUTO(SLG_E, filename);
                deqBuf.mvOut.at(i).mBuffer->saveToFile(filename);
                MY_LOGI0("save to file : %s", filename);
            }
        }
    }
    #endif


    FUNCTION_P1_OUT;

    return OK;
#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_stop()
{
#if SUPPORT_ISP
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_stop");

    //(1) handle active flag
    if (!isActive()) {
        MY_LOGD0("LaunchState=%d - return", getLaunchState());
        return OK;
    }

    FUNCTION_P1_IN;
    setLaunchState(NSP1Node::LAUNCH_STATE_FLUSH);
    MY_LOGI0("Cam::%d Req=%d Set=%d Enq=%d Deq=%d Out=%d", getOpenId(),
        mTagReq.get(), mTagSet.get(), mTagEnq.get(), mTagDeq.get(),
        mTagOut.get());

    MINT32 frmNum = P1_FRM_NUM_NULL;
    MINT32 reqNum = P1_REQ_NUM_NULL;
    MINT32 cnt = lastFrameRequestInfoNotice(frmNum, reqNum);
    mLogInfo.setMemo(LogInfo::CP_OP_STOP_BGN, frmNum, reqNum, cnt);
    //
    {
        Mutex::Autolock _ll(mFrameSetLock);
        mFrameSetAlready = MFALSE;
    }
    //
    if (getInit()) {
        MY_LOGI0("HwLockInitWait +++");
        Mutex::Autolock _l(mHardwareLock);
        MY_LOGI0("HwLockInitWait ---");
    }

    if (mpHwStateCtrl != NULL) {
        mpHwStateCtrl->reset();
    }

    //(2.2) stop 3A stt
    #if SUPPORT_3A
    L_if (mp3A && (!mCamCfgExp)) {
        Mutex::Autolock _sl(mStopSttLock);
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_STOP_3A_STOPSTT_BGN,
            LogInfo::CP_OP_STOP_3A_STOPSTT_END);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-stopStt");
        MY_LOGI1("mp3A->stopStt +++");
        mp3A->stopStt();
        MY_LOGI1("mp3A->stopStt ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-stopStt"
    } else if (mCamCfgExp) {
        MY_LOGI0("[CamCfgExp] skip 3A.stopStt");
    } else {
        MY_LOGI0("3A.stopStt not exist");
    }
    #endif

    //(2.3) stop isp
    U_if (mpCamIO == NULL) {
        MY_LOGE("hardware CamIO not exist");
        return BAD_VALUE;
    }
    L_if (!mCamCfgExp) {
        //Mutex::Autolock _l(mHardwareLock);
        if (mLongExp.get()) {
            LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_STOP_DRV_STOP_BGN,
                LogInfo::CP_OP_STOP_DRV_STOP_END, MTRUE /* call abort */);
            P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-abort");
            MY_LOGI0("mpCamIO->abort +++");
            if(!mpCamIO->abort()) {
                MY_LOGE("hardware abort fail");
                //return BAD_VALUE;
            }
            MY_LOGI0("mpCamIO->abort ---");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-abort"
        } else {
            LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_STOP_DRV_STOP_BGN,
                LogInfo::CP_OP_STOP_DRV_STOP_END, MFALSE /* not abort */);
            P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-stop");
            MY_LOGI1("mpCamIO->stop +++");
            if(!mpCamIO->stop(MTRUE)) {
                MY_LOGE("hardware stop fail");
                //return BAD_VALUE;
            }
            MY_LOGI1("mpCamIO->stop ---");
            P1_TRACE_C_END(SLG_S); // "P1:DRV-stop"
        }
        /*
        if(IS_PORT(CONFIG_PORT_RRZO, mConfigPort))
            mpCamIO->abortDma(PORT_RRZO,getNodeName());
        if(IS_PORT(CONFIG_PORT_IMGO, mConfigPort))
            mpCamIO->abortDma(PORT_IMGO,getNodeName());
        if(IS_PORT(CONFIG_PORT_EISO, mConfigPort))
            mpCamIO->abortDma(PORT_EISO,getNodeName());
        if(IS_PORT(CONFIG_PORT_LCSO, mConfigPort))
            mpCamIO->abortDma(PORT_LCSO,getNodeName());
        */
    } else {
        MY_LOGI0("[CamCfgExp] skip DRV.stop");
    }

    mLogInfo.setMemo(LogInfo::CP_OP_STOP_HW_LOCK_BGN);
    MY_LOGI1("HwLockStopWait +++");
    Mutex::Autolock _l(mHardwareLock);
    MY_LOGI1("HwLockStopWait ---");
    mLogInfo.setMemo(LogInfo::CP_OP_STOP_HW_LOCK_END);

    //(3.0) stop 3A
    #if SUPPORT_3A
    L_if (mp3A && (!mCamCfgExp)) {
        P1_TRACE_S_BEGIN(SLG_S, "P1:LMV-enableOIS");
        if (IS_LMV(mpConnectLMV)) {
            mpConnectLMV->enableOIS(mp3A);
        }
        P1_TRACE_C_END(SLG_S); // "P1:LMV-enableOIS"
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-sendCtrl-detachCb");
        if (EIS_MODE_IS_EIS_ADVANCED_ENABLED(EisInfo::getMode(mPackedEisInfo)) ||
            EIS_MODE_IS_EIS_12_ENABLED(EisInfo::getMode(mPackedEisInfo)) ) {
            mp3A ->send3ACtrl(E3ACtrl_SetMinMaxFps, 5000, 30000);
            if( (mSensorParams.vhdrMode == SENSOR_VHDR_MODE_NONE) && (!mDisableAEEIS) ) {
                mp3A ->send3ACtrl(E3ACtrl_SetAEEISRecording, 0, 0);
            }
        }
        #if SUPPORT_FSC
        if(mpFSC != NULL)
        {
            mpFSC->Uninit(mp3A);//detach 3A CB before 3A->stop()
        }
        #endif
        //
        mp3A->detachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
        mp3A->detachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
        mp3A->detachCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, this);
        P1_TRACE_C_END(SLG_S); // "P1:3A-sendCtrl-detachCb"
        LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_STOP_3A_STOP_BGN,
            LogInfo::CP_OP_STOP_3A_STOP_END);
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-stop");
        MY_LOGI1("mp3A->stop +++");
        mp3A->stop();
        MY_LOGI1("mp3A->stop ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-stop"
    } else if (mCamCfgExp) {
        MY_LOGI0("[CamCfgExp] skip 3A.stop");
    } else {
        MY_LOGI0("3A.stop not exist");
    }
    #endif

    //(3.1) destroy 3A
    #if SUPPORT_3A
    if (mp3A) {
        if (getPowerNotify()) {
            LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OP_STOP_3A_PWROFF_BGN,
                LogInfo::CP_OP_STOP_3A_PWROFF_END);
            P1_TRACE_S_BEGIN(SLG_S, "P1:3A-notifyPwrOff");
            MY_LOGI1("mp3A->notifyP1PwrOff +++");
            mp3A->notifyP1PwrOff();//CCU DRV power off before ISP uninit.
            MY_LOGI1("mp3A->notifyP1PwrOff ---");
            P1_TRACE_C_END(SLG_S); // "P1:3A-notifyPwrOff"
        } else {
            MY_LOGI0("3A->notifyP1PwrOff() no need");
        }
        setPowerNotify(MFALSE);
        //
        P1_TRACE_S_BEGIN(SLG_S, "P1:3A-destroy");
        MY_LOGI1("mp3A->destroyInstance +++");
        mp3A->destroyInstance(getNodeName());
        MY_LOGI1("mp3A->destroyInstance ---");
        P1_TRACE_C_END(SLG_S); // "P1:3A-destroy"
        mp3A = NULL;
    }
    #endif

    //(3.2) destroy isp
    {
        if (IS_LMV(mpConnectLMV)) {
            mpConnectLMV->uninit();
        }
        //
        #if SUPPORT_VHDR
        if(mpVhdr)
        {
            mpVhdr->Uninit();
            //mpVhdr->DestroyInstance(LOG_TAG); // instance always exist until process kill
            mpVhdr = NULL;
        }
        #endif
        #if SUPPORT_LCS
        if(mpLCS)
        {
            mpLCS->Uninit();
            mpLCS->DestroyInstance(LOG_TAG); // instance always exist until process kill
            mpLCS = NULL;
        }
        #endif
        #if SUPPORT_RSS
        if(mpRSS != NULL)
        {
            mpRSS->Uninit();
            mpRSS = NULL;
        }
        #endif
        #if SUPPORT_FSC
        if(mpFSC != NULL)
        {
            mpFSC = NULL;
        }
        #endif
        //
        mLogInfo.setMemo(LogInfo::CP_OP_STOP_DRV_UNINIT_BGN);
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-uninit");
        MY_LOGI1("mpCamIO->uninit +++");
        if(!mpCamIO->uninit() )
        {
            MY_LOGE("hardware uninit fail");
            //return BAD_VALUE;
        }
        MY_LOGI1("mpCamIO->uninit ---");
        P1_TRACE_C_END(SLG_S); // "P1:DRV-uninit"
        mLogInfo.setMemo(LogInfo::CP_OP_STOP_DRV_UNINIT_END);
        //
        P1_TRACE_S_BEGIN(SLG_S, "P1:DRV-destroyInstance");
        MY_LOGI1("mpCamIO->destroyInstance +++");
        #if (P1NODE_USING_MTK_LDVT > 0)
        mpCamIO->destroyInstance("iopipeUseTM");
        #else
        mpCamIO->destroyInstance(getNodeName());
        #endif
        MY_LOGI1("mpCamIO->destroyInstance ---");
        mpCamIO = NULL;
        P1_TRACE_C_END(SLG_S); // "P1:DRV-destroyInstance"
    }
    //
    syncHelperStop();
    //
    if (mspResConCtrl != NULL) {
        P1NODE_RES_CON_RELEASE(mspResConCtrl, mResConClient, mIsResConGot);
    }
    //
    #if USING_CTRL_3A_LIST_PREVIOUS
    mPreviousCtrlList.clear();
    #endif
    //
    mLogInfo.setMemo(LogInfo::CP_OP_STOP_END, frmNum, reqNum, cnt);
    //
    FUNCTION_P1_OUT;

    return OK;

#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_streaming()
{
    P1_TRACE_AUTO(SLG_B, "P1:hardwareOps_streaming");
    U_if (mpHwStateCtrl == NULL) {
        return BAD_VALUE;
    }
    if (!mpHwStateCtrl->checkReceiveRestreaming()) {
        return BAD_VALUE;
    }
    //
    if (mpHwStateCtrl->isLegacyStandby()) {
        MINT32 nShutterTimeUs = 0;
        mpHwStateCtrl->checkShutterTime(nShutterTimeUs);
        //mpCamIO->resume(nShutterTimeUs);
        //mp3A->resume();
        MBOOL ret = MFALSE;
        P1_TRACE_F_BEGIN(SLG_E, "P1:DRV-Resume(%d)", nShutterTimeUs);
        ret = mpCamIO->resume(nShutterTimeUs);
        P1_TRACE_C_END(SLG_E); // "P1:DRV-Resume"
        if (!ret) {
            MY_LOGE("[SUS-RES] FAIL");
            mpHwStateCtrl->dump();
            mpHwStateCtrl->clean();
            return BAD_VALUE;
        }
        //
        P1_TRACE_S_BEGIN(SLG_E, "P1:3A-Resume");
        mp3A->resume();
        P1_TRACE_C_END(SLG_E); // "P1:3A-Resume"
        //
        MY_LOGI0("[SUS-RES] Recover-Loop-N");
        //
        mpHwStateCtrl->checkThreadWeakup();
    } else {
        U_if (mpTaskCtrl == NULL || mpTaskCollector == NULL) {
            return BAD_VALUE;
        }
        P1QueJob job(mBurstNum);
        mpTaskCollector->requireJob(job);
        if (!job.ready()) {
            MY_LOGE("job-require-fail");
            mpTaskCtrl->dumpActPool();
            return BAD_VALUE;
        }
        P1Act pAct = GET_ACT_PTR(pAct, job.edit(0), BAD_VALUE);
        if (pAct->ctrlSensorStatus != SENSOR_STATUS_CTRL_STREAMING) {
            MY_LOGI0("status-mismatch(%d)@(%d)",
                pAct->ctrlSensorStatus, pAct->getNum());
        }
        MINT32 nShutterTimeUs = 0;
        mpHwStateCtrl->checkShutterTime(nShutterTimeUs);
        mpHwStateCtrl->checkRestreamingNum(pAct->getNum());
        {
            P1_TRACE_F_BEGIN(SLG_E, "P1:3A-resume(%d)", pAct->getNum());
            mp3A->resume(pAct->getNum());
            P1_TRACE_C_END(SLG_E); // "P1:3A-resume"
        }
        MERROR status = hardwareOps_enque(job, ENQ_TYPE_DIRECTLY, MFALSE,
            (MINT64)nShutterTimeUs);
        if (OK != status) {
            MY_LOGE("streaming en-queue fail (%d)@(%d)", status, job.getIdx());
            return BAD_VALUE;
        }
        //
        mpHwStateCtrl->checkThreadWeakup();
        mpHwStateCtrl->checkFirstSync();
    }
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
prepareCropInfo(
    P1QueAct & rAct,
    IMetadata* pAppMetadata,
    IMetadata* pHalMetadata,
    PREPARE_CROP_PHASE phase,
    MBOOL * pCtrlFlush
)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    MSize refSensorSize = getCurrentBinSize();//mSensorParams.size;
    MBOOL bIsBinEn = (refSensorSize == mSensorParams.size) ? MFALSE : MTRUE;
    MBOOL isFullBin = MFALSE;
    if (bIsBinEn && act->reqType == REQ_TYPE_NORMAL &&
        (/*IS_OUT(REQ_OUT_FULL_PROC, act->reqOutSet) ||*/
        act->fullRawType == EPipe_PROCESSED_RAW)) {
        isFullBin = MTRUE;
    }
    MY_LOGI2("[CropInfo][%d] +++ IsBinEn:%d IsFullBin:%d "
        "sensor(%dx%d) ref(%dx%d)", phase, mIsBinEn, isFullBin,
        mSensorParams.size.w, mSensorParams.size.h,
        refSensorSize.w, refSensorSize.h);
    act->refBinSize = refSensorSize;
    if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL) {
        act->dstSize_full = mvStreamImg[STREAM_IMG_OUT_FULL]->getImgSize();
        act->cropRect_full = MRect(MPoint(0, 0),
            (isFullBin) ? refSensorSize : mSensorParams.size);
    } else if (mvStreamImg[STREAM_IMG_OUT_OPAQUE] != NULL) {
        act->dstSize_full = mSensorParams.size;
        act->cropRect_full = MRect(MPoint(0, 0),
            (isFullBin) ? refSensorSize : mSensorParams.size);
    } else {
        act->dstSize_full = MSize(0, 0);
        act->cropRect_full = MRect(MPoint(0, 0), MSize(0, 0));
    }
    if (mvStreamImg[STREAM_IMG_OUT_RESIZE] != NULL) {
        act->dstSize_resizer = mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgSize();
        act->cropRect_resizer = MRect(MPoint(0, 0), refSensorSize);
    } else {
        act->dstSize_resizer= MSize(0, 0);
        act->cropRect_resizer = MRect(MPoint(0, 0), MSize(0, 0));
    }
    MSize calculateSize_full = act->dstSize_full;
    MSize calculateSize_resizer = act->dstSize_resizer;
    //
    if (pCtrlFlush != NULL) {
        *pCtrlFlush = MFALSE;
    }
    //
#if SUPPORT_RRZ_DST_CTRL
    if (mvStreamImg[STREAM_IMG_OUT_RESIZE] != NULL && pHalMetadata != NULL) {
        MSize bufSize = calculateSize_resizer;
        MSize setSize;
        MSize resSize;
        if (tryGetMetadata<MSize>(pHalMetadata, MTK_P1NODE_RESIZER_SET_SIZE,
            setSize)) {
            act->haveMetaResizerSet = MTRUE;
            act->sizeMetaResizerSet = setSize;
            MY_LOGI2("get MTK_P1NODE_RESIZER_SET_SIZE " P1_SIZE_STR,
                P1_SIZE_VAR(setSize));
            if (verifySizeResizer(mSensorParams.pixelMode,
                mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat(),
                mSensorParams.size, bufSize, setSize, resSize, mLogLevelI)) {
                calculateSize_resizer = resSize;
                act->dstSize_resizer = calculateSize_resizer;
                MY_LOGI2("replace DstSizeResizer of this action and calculation"
                    " by SIZE" P1_SIZE_STR, P1_SIZE_VAR(act->dstSize_resizer));
            } else {
                MY_LOGW("MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR
                    "NOT accepted", P1_SIZE_VAR(setSize));
            }
        }
    } // after replace/decide the resizer dst-buf-size, go-on the general crop calculation
#endif
    //
    if ((pAppMetadata != NULL) && (pHalMetadata != NULL)) {
        MRect cropRect_metadata;    // get from metadata
        MRect cropRect_control;     // set to act
        MRect rectApp;              // record the crop of APP-Meta MTK_SCALER_CROP_REGION
        MRect rectHal;              // record the crop of HAL-Meta MTK_P1NODE_SENSOR_CROP_REGION
        MRect rectLMV;              // record the crop of before LMV adjustment
        MBOOL needMoreInfo = MTRUE; // need more info to decide crop setting
        MBOOL needConvert = MFALSE; // need conversion processing
        MBOOL needVerify = MFALSE;  // need verification processing
        MBOOL useAppMeta = MFALSE;  // use the MTK_SCALER_CROP_REGION from AppMeta
        MSize::value_type const min_h = P1_STUFF_BUF_HEIGHT(MTRUE, mConfigPort);
        if (phase == PREPARE_CROP_PHASE_CONTROL_RESIZE) {
            MBOOL resizeFlush = MFALSE;
            if (tryGetMetadata<MBOOL>(pHalMetadata,
                MTK_P1NODE_CTRL_RESIZE_FLUSH, resizeFlush) &&
                (resizeFlush == MTRUE)) {
                if (pCtrlFlush != NULL) {
                    *pCtrlFlush = resizeFlush;
                }
                MSize::value_type crop_h = min_h;
                if (refSensorSize.h < min_h) {
                    MY_LOGI0("RefBinSize" P1_SIZE_STR " < min_h(%d) Port[0x%x]",
                        P1_SIZE_VAR(refSensorSize), min_h, mConfigPort);
                    crop_h = refSensorSize.h;
                }
                cropRect_control.s = MSize(refSensorSize.w, crop_h);
                cropRect_control.p = MPoint(0, (refSensorSize.h - min_h) >> 1);
                needMoreInfo = MFALSE; // crop setting decided
                needConvert = MFALSE; // without conversion
                needVerify = MTRUE; // go to verification
                MY_LOGI2("use <HAL-Meta> MTK_P1NODE_CTRL_FLUSH (%d) FlushCrop"
                    P1_RECT_STR, resizeFlush, P1_RECT_VAR(cropRect_control));
            } // else go-on
        }
        if (needMoreInfo) { // try to get info from AppMeta
            if (!tryGetMetadata<MRect>(pAppMetadata, MTK_SCALER_CROP_REGION,
                cropRect_metadata)) { // MTK_SCALER_CROP_REGION do not exist
                if (phase == PREPARE_CROP_PHASE_CONTROL_RESIZE) { // need Info from HalMeta to decide
                    MY_LOGI3("AppMeta - no MTK_SCALER_CROP_REGION as notify");
                    needMoreInfo = MTRUE; // try to get info from HalMeta
                } else { // in Create Action phase, if the MTK_SCALER_CROP_REGION do not exist, skip the following processing
                    MY_LOGI0("AppMeta - no MTK_SCALER_CROP_REGION, crop size "
                        "set to full(%dx%d) resizer(%dx%d) " P1INFO_ACT_STR,
                        act->dstSize_full.w, act->dstSize_full.h,
                        act->dstSize_resizer.w, act->dstSize_resizer.h,
                        P1INFO_ACT_VAR(*act));
                    needMoreInfo = MFALSE;
                }
                needConvert = MFALSE;
                needVerify = MFALSE;
            } else { // MTK_SCALER_CROP_REGION existing // need Info from HalMeta to decide
                act->haveMetaScalerCrop= MTRUE;
                act->rectMetaScalerCrop = cropRect_metadata;
                rectApp = cropRect_metadata;
                MY_LOGI3("get <APP-Meta> MTK_SCALER_CROP_REGION"
                    P1_RECT_STR, P1_RECT_VAR(rectApp));
                needMoreInfo = MTRUE; // try to get info from HalMeta
                needConvert = MTRUE;
                needVerify = MTRUE;
                useAppMeta = MTRUE;
            }
        }
        if (needMoreInfo) { // try to get info from HalMeta
            if (!tryGetMetadata<MRect>(pHalMetadata, // assign to cropRect_control, without transform(cropRect_metadata)
                MTK_P1NODE_SENSOR_CROP_REGION, cropRect_control)) {
                //MY_LOGD2("cannot get MTK_P1NODE_SENSOR_CROP_REGION, "
                //    "try to use <APP-Meta> MTK_SCALER_CROP_REGION");
                if (!useAppMeta) { // if MTK_SCALER_CROP_REGION is not in AppMeta also, skip the following processing
                    needConvert = MFALSE;
                    needVerify = MFALSE;
                } else { // use AppMeta-MTK_SCALER_CROP_REGION in the following processing
                    needConvert = MTRUE;
                    needVerify = MTRUE;
                }
            } else { // MTK_P1NODE_SENSOR_CROP_REGION existing
                act->haveMetaSensorCrop = MTRUE;
                act->rectMetaSensorCrop = cropRect_control;
                rectHal = cropRect_control;
                MY_LOGI3("get <HAL-Meta> MTK_P1NODE_SENSOR_CROP_REGION"
                    P1_RECT_STR, P1_RECT_VAR(rectHal));
                useAppMeta = MFALSE; // use HalMeta-MTK_P1NODE_SENSOR_CROP_REGION in the following processing
                needConvert = MTRUE;
                needVerify = MTRUE;
            }
        }
        if (needConvert) {
            if (useAppMeta) {
                MY_LOGI2("use <APP-Meta> MTK_SCALER_CROP_REGION"
                    P1_RECT_STR, P1_RECT_VAR(cropRect_metadata)); // in this moment rectApp == cropRect_metadata
                if (bIsBinEn) {
                    BIN_RESIZE(cropRect_metadata.p.x);
                    BIN_RESIZE(cropRect_metadata.p.y);
                    BIN_RESIZE(cropRect_metadata.s.w);
                    BIN_RESIZE(cropRect_metadata.s.h);
                }
                simpleTransform tranActive2Sensor = simpleTransform(
                        MPoint(0,0), mActiveArray.size(), mSensorParams.size);
                cropRect_control.p = transform(tranActive2Sensor,
                                                cropRect_metadata.leftTop());
                cropRect_control.s = transform(tranActive2Sensor,
                                                cropRect_metadata.size());
                MY_LOGI3("bIsBinEn(%d) " "meta" P1_RECT_STR "ctrl" P1_RECT_STR,
                    bIsBinEn, P1_RECT_VAR(cropRect_metadata),
                    P1_RECT_VAR(cropRect_control));
            } else { // not use AppMeta
                MY_LOGI2("use <HAL-Meta> MTK_P1NODE_SENSOR_CROP_REGION"
                    P1_RECT_STR, P1_RECT_VAR(cropRect_control)); // in this moment rectHal == cropRect_control
                if (bIsBinEn) {
                    BIN_RESIZE(cropRect_control.p.x);
                    BIN_RESIZE(cropRect_control.p.y);
                    BIN_RESIZE(cropRect_control.s.w);
                    BIN_RESIZE(cropRect_control.s.h);
                }
                MY_LOGI3("bIsBinEn(%d) " "ctrl" P1_RECT_STR,
                    bIsBinEn, P1_RECT_VAR(cropRect_control));
            }
            //
            if (cropRect_control.s.h < min_h) {
                MY_LOGI0("Ctrl" P1_RECT_STR " < min_h(%d) Port[0x%x] bin(%d)",
                    P1_RECT_VAR(cropRect_control), min_h, mConfigPort,
                    bIsBinEn);
                cropRect_control.s.h = min_h;
            }
        }
        if (needVerify) {
            MBOOL check_return = MFALSE;
            rectLMV = cropRect_control;
            if (IS_LMV(mpConnectLMV)) {
                mpConnectLMV->adjustCropInfo(pAppMetadata, pHalMetadata,
                    cropRect_control, refSensorSize,//mSensorParams.size,
                    mEnableFrameSync, mStereoCamMode);
            };
            if ((cropRect_control.size().w < 0) ||
                (cropRect_control.size().h < 0) ||
                (cropRect_control.leftTop().x < 0) ||
                (cropRect_control.leftTop().y < 0) ||
                (cropRect_control.leftTop().x >= refSensorSize.w) ||
                (cropRect_control.leftTop().y >= refSensorSize.h)) {
                MY_LOGW("Metadata exist - invalid cropRect_control"
                    P1_RECT_STR "refer" P1_SIZE_STR,
                    P1_RECT_VAR(cropRect_control), P1_SIZE_VAR(refSensorSize));
                check_return = MTRUE;
            }
            MY_LOGI_IF((LOGI_LV2 || check_return), "[CropInfo] "
                "meta" P1_RECT_STR "ctrl" P1_RECT_STR "active" P1_RECT_STR
                "sensor" P1_SIZE_STR "refer" P1_SIZE_STR
                "LMV" P1_RECT_STR "APP" P1_RECT_STR "HAL" P1_RECT_STR,
                P1_RECT_VAR(cropRect_metadata), P1_RECT_VAR(cropRect_control),
                P1_RECT_VAR(mActiveArray), P1_SIZE_VAR(mSensorParams.size),
                P1_SIZE_VAR(refSensorSize), P1_RECT_VAR(rectLMV),
                P1_RECT_VAR(rectApp), P1_RECT_VAR(rectHal));
            if (check_return) {
                return;
            }
            MRect curCtrl(cropRect_control);
            if ((cropRect_control.p.x + cropRect_control.s.w) >
                refSensorSize.w) {
                cropRect_control.s.w = refSensorSize.w -
                                        cropRect_control.p.x;
            }
            if ((cropRect_control.p.y + cropRect_control.s.h) >
                refSensorSize.h) {
                cropRect_control.s.h = refSensorSize.h -
                                        cropRect_control.p.y;
            }
            if (cropRect_control != curCtrl) {
                MY_LOGI1("BoundaryRefine refSensor" P1_SIZE_STR
                    "old" P1_RECT_STR "new" P1_RECT_STR
                    "-- Bin:%d APP" P1_RECT_STR "HAL" P1_RECT_STR
                    "LMV" P1_RECT_STR "AA" P1_RECT_STR "sensor" P1_SIZE_STR,
                    P1_SIZE_VAR(refSensorSize), P1_RECT_VAR(curCtrl),
                    P1_RECT_VAR(cropRect_control), bIsBinEn,
                    P1_RECT_VAR(rectApp), P1_RECT_VAR(rectHal),
                    P1_RECT_VAR(rectLMV), P1_RECT_VAR(mActiveArray),
                    P1_SIZE_VAR(mSensorParams.size));
            }
            // calculate the crop region validity
            if (mvStreamImg[STREAM_IMG_OUT_FULL] != NULL) {
                MRect cropRect_full = cropRect_control;
                if (bIsBinEn) { // cropRect_control generating is based on bin-size
                    if (isFullBin) {
                        // the Full path is effected by bin-size
                    } else {
                        // need to revert the cropRect_control
                        BIN_REVERT(cropRect_full.p.x);
                        BIN_REVERT(cropRect_full.p.y);
                        BIN_REVERT(cropRect_full.s.w);
                        BIN_REVERT(cropRect_full.s.h);
                    }
                }
                U_if (MFALSE == calculateCropInfoFull(
                    mSensorParams.pixelMode,
                    (isFullBin) ? refSensorSize : mSensorParams.size,
                    calculateSize_full,
                    (isFullBin) ? cropRect_control : cropRect_full,
                    act->cropRect_full,
                    act->dstSize_full,
                    mLogLevelI)) {
                    MY_LOGI0("[calculateCropInfoFull-not-accept] "
                        P1INFO_ACT_STR " "
                        "PixelMode(%d) isFullBin(%d) "
                        "refSensorSize" P1_SIZE_STR
                        "mSensorParams.size" P1_SIZE_STR
                        "calculateSize_full" P1_SIZE_STR
                        "cropRect_control" P1_RECT_STR
                        "cropRect_full" P1_RECT_STR
                        "act->cropRect_full" P1_RECT_STR
                        "act->dstSize_full" P1_SIZE_STR
                        "MTK_P1NODE_RESIZER_SET_SIZE(%d)" P1_SIZE_STR,
                        P1INFO_ACT_VAR(*act), mSensorParams.pixelMode,
                        isFullBin,
                        P1_SIZE_VAR(refSensorSize),
                        P1_SIZE_VAR(mSensorParams.size),
                        P1_SIZE_VAR(calculateSize_full),
                        P1_RECT_VAR(cropRect_control),
                        P1_RECT_VAR(cropRect_full),
                        P1_RECT_VAR(act->cropRect_full),
                        P1_SIZE_VAR(act->dstSize_full),
                        act->haveMetaResizerSet,
                        P1_SIZE_VAR(act->sizeMetaResizerSet));
                };
            }
            if (mvStreamImg[STREAM_IMG_OUT_RESIZE] != NULL) {
                U_if (MFALSE == calculateCropInfoResizer(
                    mSensorParams.pixelMode,
                    (mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat()),
                    refSensorSize,
                    calculateSize_resizer,
                    cropRect_control,
                    act->cropRect_resizer,
                    act->dstSize_resizer,
                    mLogLevelI)) {
                    MY_LOGI0("[calculateCropInfoResizer-not-accept] "
                        P1INFO_ACT_STR " "
                        "PixelMode(%d) ImgFormat(0x%x) "
                        "refSensorSize" P1_SIZE_STR
                        "calculateSize_resizer" P1_SIZE_STR
                        "cropRect_control" P1_RECT_STR
                        "act->cropRect_resizer" P1_RECT_STR
                        "act->dstSize_resizer" P1_SIZE_STR
                        "MTK_P1NODE_RESIZER_SET_SIZE(%d)" P1_SIZE_STR,
                        P1INFO_ACT_VAR(*act), mSensorParams.pixelMode,
                        (mvStreamImg[STREAM_IMG_OUT_RESIZE]->getImgFormat()),
                        P1_SIZE_VAR(refSensorSize),
                        P1_SIZE_VAR(calculateSize_resizer),
                        P1_RECT_VAR(cropRect_control),
                        P1_RECT_VAR(act->cropRect_resizer),
                        P1_SIZE_VAR(act->dstSize_resizer),
                        act->haveMetaResizerSet,
                        P1_SIZE_VAR(act->sizeMetaResizerSet));
                };
            }
        }
    } else {
        MY_LOGI3("Meta APP(%p) HAL(%p) only check the final size "
            P1INFO_ACT_STR, pAppMetadata, pHalMetadata, P1INFO_ACT_VAR(*act));
    }
    //
    if (act->dstSize_full.w > act->cropRect_full.s.w) {
        MY_LOGI1("cannot-upscale-F DstW(%d) > SrcW(%d)",
            act->dstSize_full.w, act->cropRect_full.s.w);
        act->dstSize_full.w = act->cropRect_full.s.w;
    }
    if (act->dstSize_full.h > act->cropRect_full.s.h) {
        MY_LOGI1("cannot-upscale-F DstH(%d) > SrcH(%d)",
            act->dstSize_full.h, act->cropRect_full.s.h);
        act->dstSize_full.h = act->cropRect_full.s.h;
    }
    if (act->dstSize_resizer.w > act->cropRect_resizer.s.w) {
        MY_LOGI1("cannot-upscale-R DstW(%d) > SrcW(%d)",
            act->dstSize_resizer.w, act->cropRect_resizer.s.w);
        act->dstSize_resizer.w = act->cropRect_resizer.s.w;
    }
    if (act->dstSize_resizer.h > act->cropRect_resizer.s.h) {
        MY_LOGI1("cannot-upscale-R DstH(%d) > SrcH(%d)",
            act->dstSize_resizer.h, act->cropRect_resizer.s.h);
        act->dstSize_resizer.h = act->cropRect_resizer.s.h;
    }
    MY_LOGI2("[CropInfo][%d] --- [F] Src" P1_RECT_STR "Dst" P1_SIZE_STR
        "[R] Src" P1_RECT_STR "Dst" P1_SIZE_STR, phase,
        P1_RECT_VAR(act->cropRect_full), P1_SIZE_VAR(act->dstSize_full),
        P1_RECT_VAR(act->cropRect_resizer), P1_SIZE_VAR(act->dstSize_resizer));
}

#if USING_CTRL_3A_LIST
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateCtrlList(
    List<MetaSet_T> * pList,
    P1QueJob & rJob
)
{
    U_if (pList == NULL) {
        MY_LOGE("List is NULL");
        return;
    }
    #define P1_3A_LIST_INDEX (2)
    size_t total = (P1_3A_LIST_INDEX * mBurstNum);
    #if USING_CTRL_3A_LIST_PREVIOUS // force to save and set previous metadata
    //#warning "using previously padding 3A Control List"
    // add dummy before first request
    while (mPreviousCtrlList.size() < total) {
        MetaSet_T set;
        set.MagicNum = 0;
        set.Dummy = 1;
        mPreviousCtrlList.push_back(set);
    }
    // add this request
    for (size_t j = 0; j < rJob.size(); j++) {
        if (rJob.edit(j).ptr() != NULL) {
            mPreviousCtrlList.push_back(rJob.edit(j).ptr()->metaSet);
        }
    }
    // keep list length in (P1_3A_LIST_INDEX + 1)
    while ((mPreviousCtrlList.size() > (total + mBurstNum))) {
        mPreviousCtrlList.erase(mPreviousCtrlList.begin());
    }
    // copy the mPreviousCtrlList to set-CtrlList
    List<MetaSet_T>::iterator p_it = mPreviousCtrlList.begin();
    for (; p_it != mPreviousCtrlList.end(); p_it++) {
        pList->push_back(*p_it);
    }
    #else
    for (size_t i = 0; i < total; i++) {
        MetaSet_T set;
        pList->push_back(set);
    }
    for (size_t j = 0; j < rJob.size(); j++) {
        if (rJob.edit(j).ptr() != NULL) {
            pList->push_back(rJob.edit(j).ptr()->metaSet);
        }
    }
    #endif
    //
    if (mMetaLogOp > 0 && pList->size() > 0 &&
        pList->size() == (rJob.size() * (P1_3A_LIST_INDEX + 1))) {
        MY_LOGI0("LogMeta List[%zu] Job[%zu]", pList->size(), rJob.size());
        List<MetaSet_T>::iterator it = pList->begin();
        for (size_t j = 0; j < total && it != pList->end(); j++) {
            it++;
        } // shift to (P1_3A_LIST_INDEX * mBurstNum)
        for (size_t i = 0; i < rJob.size() && it != pList->end(); i++, it++) {
            P1Act pAct = GET_ACT_PTR(pAct, rJob.edit((MUINT8) i), RET_VOID);
            P1_LOG_META(*pAct, &(it->appMeta),
                "3A.Set-APP");
            P1_LOG_META(*pAct, &(it->halMeta),
                "3A.Set-HAL");
            //P1_LOG_META(*pAct, &(pAct->metaSet.appMeta), "3A.Act-APP");
            //P1_LOG_META(*pAct, &(pAct->metaSet.halMeta), "3A.Act-HAL");
        }
    }
    return;
};
#endif

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateCtrlQueue(
    std::vector< MetaSet_T* > & rQue,
    P1QueJob & rJob
)
{
    for (size_t j = 0; j < rJob.size(); j++) {
        if (rJob.edit(j).ptr() != NULL) {
            rQue.push_back(&(rJob.edit(j).ptr()->metaSet));
        }
    }
    //
    if (mMetaLogOp > 0 && rQue.size() > 0 && rQue.size() == rJob.size()) {
        MY_LOGI0("LogMeta Que[%zu] Job[%zu]", rQue.size(), rJob.size());
        std::vector< MetaSet_T* >::iterator it = rQue.begin();
        for (size_t i = 0; i < rJob.size() && it != rQue.end(); i++, it++) {
            P1Act pAct = GET_ACT_PTR(pAct, rJob.edit((MUINT8) i), RET_VOID);
            P1_LOG_META(*pAct, &((*it)->appMeta),
                "3A.Set-APP");
            P1_LOG_META(*pAct, &((*it)->halMeta),
                "3A.Set-HAL");
            //P1_LOG_META(*pAct, &(pAct->metaSet.appMeta), "3A.Act-APP");
            //P1_LOG_META(*pAct, &(pAct->metaSet.halMeta), "3A.Act-HAL");
        }
    }
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createAction(P1QueAct & rAct,
    sp<IPipelineFrame> appFrame,
    REQ_TYPE eType)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //create queue act
    //MUINT32 newNum = get_and_increase_magicnum();
    MetaSet_T* metaInfo = &(act->metaSet);;
    IMetadata* pAppMeta = &(act->metaSet.appMeta);
    IMetadata* pHalMeta = &(act->metaSet.halMeta);
    act->metaSet.PreSetKey = rAct.id();
    //
    MINT32 meta_raw_type = (MINT32)mRawDefType;
    MBOOL meta_raw_exist = MFALSE;
    MBOOL meta_zsl_req = MFALSE;
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:create|Fnum:%d Rnum:%d",
        P1GET_FRM_NUM(appFrame), P1GET_REQ_NUM(appFrame));
    MINT32 meta_ZslEn = P1_META_GENERAL_EMPTY_INT;
    MINT32 meta_CapIntent = P1_META_GENERAL_EMPTY_INT;
    MINT32 meta_RawType = P1_META_GENERAL_EMPTY_INT;
    MINT32 meta_TgNum = P1_META_GENERAL_EMPTY_INT;
    MINT32 meta_QualityCtrl = P1_META_GENERAL_EMPTY_INT;
    MINT32 meta_FmtImgo = P1_META_GENERAL_EMPTY_INT; // eImgFmt_UNKNOWN
    MINT32 meta_FmtRrzo = P1_META_GENERAL_EMPTY_INT; // eImgFmt_UNKNOWN
    //
    /*
    // set as constructor
    act->sofIdx = P1SOFIDX_INIT_VAL;
    act->appFrame = NULL;
    act->reqType = REQ_TYPE_UNKNOWN;
    act->reqOutSet = REQ_SET_NONE;
    act->expRec = EXP_REC_NONE;
    act->exeState = EXE_STATE_NULL;
    */
    //
    if (appFrame != NULL) {
        U_if (eType != REQ_TYPE_UNKNOWN) {
            MY_LOGE("Type-Mismatching (%d) on (%d, %d)", eType,
                P1GET_FRM_NUM(appFrame), P1GET_REQ_NUM(appFrame));
            return;
        }
        if (act->appFrame != appFrame) { // act->appFrame == NULL
            act->appFrame = appFrame;
            act->frmNum = appFrame->getFrameNo();
            act->reqNum = appFrame->getRequestNo();
            act->mapFrameStream();
            MY_LOGI2("CreateAct(%d,%d) assign frame", act->frmNum, act->reqNum);
        }
        //
        P1_TRACE_S_BEGIN(SLG_O, "createMeta");
        if (mvStreamMeta[STREAM_META_IN_APP] != NULL) {
            if (OK == act->frameMetadataGet(
                STREAM_META_IN_APP, pAppMeta)) {
                if (LOGI_LV1 && pAppMeta != NULL) {
                    act->metaCntInAPP = pAppMeta->count();
                }
                P1_LOG_META(*act, pAppMeta,
                    "RequestIn-APP");
            } else {
                MY_LOGI0("can not lock the app metadata");
                pAppMeta = NULL;
            }
        }
        if (mvStreamMeta[STREAM_META_IN_HAL] != NULL) {
            if (OK == act->frameMetadataGet(
                STREAM_META_IN_HAL, pHalMeta)) {
                if (LOGI_LV1 && pHalMeta != NULL) {
                    act->metaCntInHAL = pHalMeta->count();
                }
                P1_LOG_META(*act, pHalMeta,
                    "RequestIn-HAL");
            } else {
                MY_LOGI0("can not lock the hal metadata");
                pHalMeta = NULL;
            }
        }
        P1_TRACE_C_END(SLG_O); // "createMeta"
        // check info from APP meta
        if (pAppMeta != NULL) {
            MUINT8 zsl_en = MTK_CONTROL_ENABLE_ZSL_FALSE;
            if (tryGetMetadata<MUINT8>(
                pAppMeta, MTK_CONTROL_ENABLE_ZSL, zsl_en)) {
                meta_ZslEn = zsl_en;
            }
            MUINT8 cap_intent = MTK_CONTROL_CAPTURE_INTENT_CUSTOM;
            if (tryGetMetadata<MUINT8>(
                pAppMeta, MTK_CONTROL_CAPTURE_INTENT, cap_intent)) {
                meta_CapIntent = cap_intent;
            }
            if (zsl_en == MTK_CONTROL_ENABLE_ZSL_TRUE &&
                cap_intent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE
                #if 0 // check-bypass-request
                && appFrame->IsReprocessFrame()
                #endif
                ) {
                meta_zsl_req = MTRUE;
            }
        }
        // check info from HAL meta
        if (pHalMeta != NULL) {
            MINT32 raw_type = meta_raw_type;
            if (mEnableDualPD == MFALSE && // DualPD must be Driver Selected raw
                tryGetMetadata<MINT32>(
                pHalMeta, MTK_P1NODE_RAW_TYPE, raw_type)) {
                meta_RawType = raw_type;
                MY_LOGD2("raw type set from outside %d", raw_type);
                if (meta_raw_type != raw_type) {
                    MY_LOGI2("Metadata-Raw(%d) - Config-Raw(%d)(%d-0x%x)",
                        raw_type, meta_raw_type, mRawDefType, mRawOption);
                }
                if ((mRawOption & (MUINT32)(1 << raw_type)) > 0) {
                    meta_raw_type = raw_type;
                    meta_raw_exist = MTRUE;
                } else {
                    MY_LOGI1("raw type (%d) set from outside, but not accept "
                        "RawOption(0x%x)", raw_type, mRawOption);
                }
            }
            // for UNI Switch control
            if (IS_LMV(mpConnectLMV) &&
                mpConnectLMV->checkSwitchOut(pHalMeta)) {
                act->uniSwitchState = UNI_SWITCH_STATE_REQ;
            }
            // for Twin Switch TG Number control
            if (mIsDynamicTwinEn) {
                MINT32 tg_num = MTK_P1_TWIN_SWITCH_NONE;
                if (tryGetMetadata<MINT32>(
                    pHalMeta, MTK_P1NODE_TWIN_SWITCH, tg_num)) {
                    meta_TgNum = tg_num;
                    if (tg_num != MTK_P1_TWIN_SWITCH_NONE) {
                        act->tgSwitchState = TG_SWITCH_STATE_REQ;
                        switch (tg_num) {
                            case MTK_P1_TWIN_SWITCH_ONE_TG:
                                act->tgSwitchNum = 1;
                                break;
                            case MTK_P1_TWIN_SWITCH_TWO_TG:
                                act->tgSwitchNum = 2;
                                break;
                            default:
                                MY_LOGI0("check MTK_P1NODE_TWIN_SWITCH %d",
                                    tg_num);
                                break;
                        }
                    }
                }
            }
            // for Standby Mode control
            if (mpHwStateCtrl != NULL) {
                act->ctrlSensorStatus =
                    mpHwStateCtrl->checkReceiveFrame(pHalMeta);
            }
            // for Quality Switch control
            if (mpRegisterNotify != NULL) {
                MINT32 quality_ctrl = MTK_P1_RESIZE_QUALITY_SWITCH_NONE;
                act->qualitySwitchState = QUALITY_SWITCH_STATE_NONE;
                if (tryGetMetadata<MINT32>(pHalMeta,
                    MTK_P1NODE_RESIZE_QUALITY_SWITCH, quality_ctrl)) {
                    meta_QualityCtrl = quality_ctrl;
                    if (getQualitySwitching() &&
                        quality_ctrl != MTK_P1_RESIZE_QUALITY_SWITCH_NONE) {
                        act->qualitySwitchState =
                            QUALITY_SWITCH_STATE_DONE_ILLEGAL;
                    } else {
                        switch (quality_ctrl) {
                            case MTK_P1_RESIZE_QUALITY_SWITCH_H_H:
                                act->qualitySwitchState =
                                    QUALITY_SWITCH_STATE_REQ_H_H;
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_H_L:
                                act->qualitySwitchState =
                                    QUALITY_SWITCH_STATE_REQ_H_L;
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_L_H:
                                act->qualitySwitchState =
                                    QUALITY_SWITCH_STATE_REQ_L_H;
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_L_L:
                                act->qualitySwitchState =
                                    QUALITY_SWITCH_STATE_REQ_L_L;
                                break;
                            default:
                                break;
                        }
                    }
                }
                if (act->qualitySwitchState != QUALITY_SWITCH_STATE_NONE) {
                    E_CamIQLevel CamLvA = eCamIQ_MAX;
                    E_CamIQLevel CamLvB = eCamIQ_MAX;
                    if (mpCamIO != NULL &&
                        mpCamIO->sendCommand(ENPipeCmd_GET_QUALITY,
                        (MINTPTR)NULL, (MINTPTR)&CamLvA, (MINTPTR)&CamLvB)) {
                        MBOOL ignore = MFALSE;
                        switch (quality_ctrl) {
                            case MTK_P1_RESIZE_QUALITY_SWITCH_H_H:
                                if (CamLvA == eCamIQ_H && CamLvB == eCamIQ_H) {
                                    ignore = MTRUE;
                                }
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_H_L:
                                if (CamLvA == eCamIQ_H && CamLvB == eCamIQ_L) {
                                    ignore = MTRUE;
                                }
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_L_H:
                                if (CamLvA == eCamIQ_L && CamLvB == eCamIQ_H) {
                                    ignore = MTRUE;
                                }
                                break;
                            case MTK_P1_RESIZE_QUALITY_SWITCH_L_L:
                                if (CamLvA == eCamIQ_L && CamLvB == eCamIQ_L) {
                                    ignore = MTRUE;
                                }
                                break;
                            default:
                                break;
                        }
                        if (ignore) {
                            act->qualitySwitchState =
                                            QUALITY_SWITCH_STATE_DONE_IGNORE;
                        }
                    }
                }
                if ((act->qualitySwitchState & QUALITY_SWITCH_STATE_REQ_NON)
                    > 0) {
                    setQualitySwitching(MTRUE);
                }
            }

            if (tryGetMetadata<MINT32>(pHalMeta, MTK_HAL_REQUEST_IMG_IMGO_FORMAT,
                        act->mReqFmt_Imgo))
            {
                meta_FmtImgo = act->mReqFmt_Imgo;
                MY_LOGI2("MTK_REQUEST_IMG_IMGO_FORMAT : 0x%x", act->mReqFmt_Imgo);
            }

            if (tryGetMetadata<MINT32>(pHalMeta, MTK_HAL_REQUEST_IMG_RRZO_FORMAT,
                        act->mReqFmt_Rrzo))
            {
                meta_FmtRrzo = act->mReqFmt_Rrzo;
                MY_LOGI2("MTK_REQUEST_IMG_RRZO_FORMAT : 0x%x", act->mReqFmt_Rrzo);
            }

        }
        //
        if (meta_zsl_req) {
            act->reqType = REQ_TYPE_ZSL;
        } else if (act->streamBufImg[STREAM_IMG_IN_YUV].bExist) {
            act->reqType = REQ_TYPE_YUV;
        } else if (act->streamBufImg[STREAM_IMG_IN_OPAQUE].bExist) {
            act->reqType = REQ_TYPE_REDO;
        } else {
            act->reqType = REQ_TYPE_NORMAL;
            if (IS_PORT(CONFIG_PORT_IMGO, mConfigPort) &&
                act->streamBufImg[STREAM_IMG_OUT_OPAQUE].bExist) {
                act->reqOutSet |= REQ_SET(REQ_OUT_FULL_OPAQUE);
            }
            if (IS_PORT(CONFIG_PORT_IMGO, mConfigPort) &&
                act->streamBufImg[STREAM_IMG_OUT_FULL].bExist) {
                if (meta_raw_type == EPipe_PROCESSED_RAW) {
                    act->reqOutSet |= REQ_SET(REQ_OUT_FULL_PROC);
                } else {
                    act->reqOutSet |= REQ_SET(REQ_OUT_FULL_PURE);
                }
            }
            if (IS_PORT(CONFIG_PORT_RRZO, mConfigPort) &&
                act->streamBufImg[STREAM_IMG_OUT_RESIZE].bExist) {
                act->reqOutSet |= REQ_SET(REQ_OUT_RESIZER);
            }
            if (IS_PORT(CONFIG_PORT_LCSO, mConfigPort) &&
                act->streamBufImg[STREAM_IMG_OUT_LCS].bExist) {
                act->reqOutSet |= REQ_SET(REQ_OUT_LCSO);
            }
            if (IS_PORT(CONFIG_PORT_RSSO, mConfigPort) &&
                act->streamBufImg[STREAM_IMG_OUT_RSS].bExist) {
                act->reqOutSet |= REQ_SET(REQ_OUT_RSSO);
            }
            //MY_LOGD2("normal (%d) 0x%x raw(%d %d)", act->magicNum,
            //    act->reqOutSet, meta_raw_exist, meta_raw_type);
        }
    } else {
        switch (eType) {
            case REQ_TYPE_INITIAL:
            case REQ_TYPE_PADDING:
            case REQ_TYPE_DUMMY:
                act->reqType = eType;
                break;
            default: //REQ_TYPE_UNKNOWN/REQ_TYPE_NORMAL/REQ_TYPE_REDO/REQ_TYPE_YUV
                MY_LOGE("Type-Mismatching (%d)", eType);
                return;
        }
        pAppMeta = NULL;
        pHalMeta = NULL;
        if (act->reqType == REQ_TYPE_INITIAL) { // using pool buffer only in initial act
            if (IS_PORT(CONFIG_PORT_IMGO, mConfigPort) &&
                mpStreamPool_full != NULL) {
                if (meta_raw_type == EPipe_PROCESSED_RAW) {
                    act->reqOutSet |= REQ_SET(REQ_OUT_FULL_PROC);
                } else {
                    act->reqOutSet |= REQ_SET(REQ_OUT_FULL_PURE);
                }
            }
            if (IS_PORT(CONFIG_PORT_RRZO, mConfigPort) &&
                mpStreamPool_resizer != NULL) {
                act->reqOutSet |= REQ_SET(REQ_OUT_RESIZER);
            }
            if (IS_PORT(CONFIG_PORT_LCSO, mConfigPort) &&
                mpStreamPool_lcso != NULL) {
                act->reqOutSet |= REQ_SET(REQ_OUT_LCSO);
            }
            if (IS_PORT(CONFIG_PORT_RSSO, mConfigPort) &&
                mpStreamPool_rsso != NULL) {
                act->reqOutSet |= REQ_SET(REQ_OUT_RSSO);
            }
        }
    }
    //
    act->fullRawType = meta_raw_type;
    if (act->reqType == REQ_TYPE_NORMAL) {
    #if 1 // add raw type to hal meta
        if (act->reqType == REQ_TYPE_NORMAL && !meta_raw_exist) {
            IMetadata::IEntry entryRawType(MTK_P1NODE_RAW_TYPE);
            entryRawType.push_back(meta_raw_type, Type2Type< MINT32 >());
            metaInfo->halMeta.update(MTK_P1NODE_RAW_TYPE, entryRawType);
        }
    #endif
    }
    //
    if (act->reqType == REQ_TYPE_NORMAL || act->reqType == REQ_TYPE_INITIAL ||
        act->reqType == REQ_TYPE_PADDING || act->reqType == REQ_TYPE_DUMMY) {
        if (IS_PORT(CONFIG_PORT_IMGO, mConfigPort)
            && (0 == (IS_OUT(REQ_OUT_FULL_PROC, act->reqOutSet) ||
                    IS_OUT(REQ_OUT_FULL_PURE, act->reqOutSet) ||
                    IS_OUT(REQ_OUT_FULL_OPAQUE, act->reqOutSet))
                )
            ) {
            act->reqOutSet |= REQ_SET(REQ_OUT_FULL_STUFF);
        }
        if (IS_PORT(CONFIG_PORT_RRZO, mConfigPort)
            && (0 == IS_OUT(REQ_OUT_RESIZER, act->reqOutSet))) {
            act->reqOutSet |= REQ_SET(REQ_OUT_RESIZER_STUFF);
        }
        if (IS_PORT(CONFIG_PORT_LCSO, mConfigPort)
            && (0 == IS_OUT(REQ_OUT_LCSO, act->reqOutSet))) {
            act->reqOutSet |= REQ_SET(REQ_OUT_LCSO_STUFF);
        }
        if (IS_PORT(CONFIG_PORT_RSSO, mConfigPort)
            && (0 == IS_OUT(REQ_OUT_RSSO, act->reqOutSet))) {
            act->reqOutSet |= REQ_SET(REQ_OUT_RSSO_STUFF);
        }
        //
        prepareCropInfo(rAct, pAppMeta, pHalMeta,
            PREPARE_CROP_PHASE_RECEIVE_CREATE);
        act->exeState = EXE_STATE_REQUESTED;
    } else if (act->reqType == REQ_TYPE_REDO || act->reqType == REQ_TYPE_YUV ||
        act->reqType == REQ_TYPE_ZSL) {
        act->exeState = EXE_STATE_DONE;
    }
    //
    //mTagReq.set(rAct.id()); // set number while act register
    //
    if (LOGI_LV1) { // P1_LOGI(1)
        android::String8 info(act->msg);
        act->msg.clear();
        act->msg += String8::format("[P1::REQ]" P1INFO_ACT_STR
            " [META ze:%d ci:%d rt:%d tn:%d qc:%d fi:%d fr:%d] [%s][%d] ",
            P1INFO_ACT_VAR(*act), meta_ZslEn, meta_CapIntent, meta_RawType,
            meta_TgNum, meta_QualityCtrl, meta_FmtImgo, meta_FmtRrzo,
            (appFrame != NULL) ? "New-Request" : "New-Dummy", eType);
        act->msg += info;
        if ((eType != REQ_TYPE_UNKNOWN) || // if the eType is assigned, it should be an internal act,
            (EN_INIT_REQ_RUN && mInitReqCnt < mInitReqNum)) { // print the string since no more message appending
            P1_TRACE_F_BEGIN(SLG_PFL, "P1::REQ_LOG|Mnum:%d SofIdx:%d Fnum:%d "
                "Rnum:%d FlushSet:0x%x", act->magicNum, act->sofIdx,
                act->frmNum, act->reqNum, act->flushSet);
            P1_LOGI(1, "%s", act->msg.string());
            P1_TRACE_C_END(SLG_PFL); // "P1::REQ_LOG"
        }
    };
    //
    P1_TRACE_C_END(SLG_I); // "P1:create"
    //
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessResult(
    P1QueAct & rAct,
    QBufInfo const &deqBuf,
    MetaSet_T const &result3A,
    IMetadata const &resultAppend,
    MUINT32 const index
)
{
    FUNCTION_P1_IN;
    //
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:result|Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
        act->magicNum, act->sofIdx, act->frmNum, act->reqNum);
    //
    if (act->appFrame != 0) {
        if ((mvStreamMeta[STREAM_META_OUT_APP] != NULL) &&
            (mvStreamMeta[STREAM_META_OUT_HAL] != NULL)) {
            // APP out Meta Stream
            IMetadata outAppMetadata;
            generateAppMeta(rAct, result3A, deqBuf, outAppMetadata, index);
            if ((IS_OUT(REQ_OUT_FULL_OPAQUE, act->reqOutSet)) &&
                (act->streamBufImg[STREAM_IMG_OUT_OPAQUE].spImgBuf != NULL) &&
                (!IS_EXP(EXP_EVT_NOBUF_IMGO, act->expRec))) {
                // app metadata index
                IMetadata appTagIndex;
                generateAppTagIndex(outAppMetadata, appTagIndex);
                sp<IImageBufferHeap> pImageBufferHeap =
                    act->streamBufImg[STREAM_IMG_OUT_OPAQUE].spImgBuf->
                    getImageBufferHeap();
                MERROR status = OpaqueReprocUtil::setAppMetadataToHeap(
                    pImageBufferHeap,
                    appTagIndex);
                MY_LOGD2("setAppMetadataToHeap (%d)", status);
            }
            //
            // HAL out Meta Stream
            IMetadata inHalMetadata;
            IMetadata outHalMetadata;
            if (OK != act->frameMetadataGet(STREAM_META_IN_HAL,
                &inHalMetadata)) {
                MY_LOGW("cannot get in-hal-metadata");
            }
            generateHalMeta(rAct, result3A, deqBuf, resultAppend, inHalMetadata,
                outHalMetadata, index);
            if ((IS_OUT(REQ_OUT_FULL_OPAQUE, act->reqOutSet)) &&
                (act->streamBufImg[STREAM_IMG_OUT_OPAQUE].spImgBuf != NULL) &&
                (!IS_EXP(EXP_EVT_NOBUF_IMGO, act->expRec))) {
                sp<IImageBufferHeap> pImageBufferHeap =
                    act->streamBufImg[STREAM_IMG_OUT_OPAQUE].spImgBuf->
                    getImageBufferHeap();
                MERROR status = OpaqueReprocUtil::setHalMetadataToHeap(
                    pImageBufferHeap,
                    outHalMetadata);
                MY_LOGD2("setHalMetadataToHeap (%d)", status);
            }
            //
            MBOOL isChange = MFALSE;
            attemptCtrlReadout(rAct, &outAppMetadata, &outHalMetadata,
                isChange);
            //
            if (mspSyncHelper != NULL) {
                IMetadata ctrlMeta;
                act->frameMetadataGet(STREAM_META_IN_HAL, &ctrlMeta);
                MBOOL res = mspSyncHelper->syncResultCheck(getOpenId(),
                    &ctrlMeta, &outHalMetadata);
                if (!res) {
                    act->setFlush(FLUSH_MIS_SYNC);
                    MY_LOGI0("SyncHelper flush this request (%d)"
                        P1INFO_ACT_STR, res, P1INFO_ACT_VAR(*act));
                }
            }
            //
            if (OK != act->frameMetadataGet(STREAM_META_OUT_APP,
                NULL, MTRUE, &outAppMetadata)) {
                MY_LOGW("cannot write out-app-metadata");
            } else {
                if (LOGI_LV1) {
                    act->metaCntOutAPP = outAppMetadata.count();
                }
                P1_LOG_META(*act, &outAppMetadata,
                    "ResultOut-APP");
            }
            if (OK != act->frameMetadataGet(STREAM_META_OUT_HAL,
                NULL, MTRUE, &outHalMetadata)) {
                MY_LOGW("cannot write out-hal-metadata");
            } else {
                if (LOGI_LV1) {
                    act->metaCntOutHAL = outHalMetadata.count();
                }
                P1_LOG_META(*act, &outHalMetadata,
                    "ResultOut-HAL");
            }
        } else {
            MY_LOGW("STREAM_META_OUT not exist - APP(%d) HAL(%d)",
                (mvStreamMeta[STREAM_META_OUT_APP] != NULL) ? MTRUE : MFALSE,
                (mvStreamMeta[STREAM_META_OUT_HAL] != NULL) ? MTRUE : MFALSE);
        }
    }
    //
    #if 1 // trigger only at the end of this job
    onReturnFrame(rAct, FLUSH_NONEED,
        (IS_BURST_OFF || (index == (MUINT32)(mBurstNum - 1))) ?
        MTRUE : MFALSE);
    /* DO NOT use this P1QueAct after onReturnFrame() */
    #else
    onReturnFrame(rAct, FLUSH_NONEED, MTRUE);
    /* DO NOT use this P1QueAct after onReturnFrame() */
    #endif
    //
    P1_TRACE_C_END(SLG_I); // "P1:result"
    //
    FUNCTION_P1_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
processRedoFrame(
    P1QueAct & rAct
)
{
    FUNCTION_P1_IN;
    //
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //
    if (act->getFlush()) {
        MY_LOGD0("need to flush, skip frame processing");
        return;
    };
    //MBOOL outAppMetaSuccess = MFALSE;
    //MBOOL outHalMetaSuccess = MFALSE;
    IMetadata appMeta;
    IMetadata halMeta;
    sp<IImageBuffer> imgBuf;
    //
    if (OK != act->frameImageGet(STREAM_IMG_IN_OPAQUE, imgBuf)) {
        MY_LOGE("Can not get in-opaque buffer from frame");
    } else {
        sp<IImageBufferHeap> pHeap = imgBuf->getImageBufferHeap();
        IMetadata appMetaTagIndex;
        if (OK == OpaqueReprocUtil::getAppMetadataFromHeap
            (pHeap, appMetaTagIndex)) {
            // get the input of app metadata
            IMetadata metaInApp;
            if (OK != act->frameMetadataGet(STREAM_META_IN_APP,
                &metaInApp)) {
                MY_LOGW("cannot get in-app-metadata");
            }
            // get p1node's tags from opaque buffer
            IMetadata::IEntry entryTagIndex =
                appMetaTagIndex.entryFor(MTK_P1NODE_METADATA_TAG_INDEX);
            for (MUINT i = 0; i < entryTagIndex.count(); i++) {
                MUINT32 tag = entryTagIndex.itemAt(i, Type2Type<MINT32>()); // get Tag from entryTagIndex
                // update entry from metaInApp to appMeta
                IMetadata::IEntry entryInApp = metaInApp.entryFor(tag);
                appMeta.update(tag, entryInApp);
            }
            // Workaround: do not return the duplicated key for YUV reprocessing
            appMeta.remove(MTK_JPEG_THUMBNAIL_SIZE);
            appMeta.remove(MTK_JPEG_ORIENTATION);
            if (OK != act->frameMetadataGet(STREAM_META_OUT_APP,
                NULL, MTRUE, &appMeta)) {
                MY_LOGW("cannot write out-app-metadata");
            }
        } else {
            MY_LOGW("Can not get app meta from in-opaque buffer");
        }
        if (OK == OpaqueReprocUtil::getHalMetadataFromHeap(pHeap, halMeta)) {
            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
            entry.push_back(1, Type2Type<MUINT8>());
            halMeta.update(entry.tag(), entry);
            if (OK != act->frameMetadataGet(STREAM_META_OUT_HAL,
                NULL, MTRUE, &halMeta)) {
                MY_LOGW("cannot write out-hal-metadata");
            }
        } else {
            MY_LOGW("Can not get hal meta from in-opaque buffer");
        }
    }
    //
    FUNCTION_P1_OUT;
    //
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
processYuvFrame(
    P1QueAct & rAct
)
{
    FUNCTION_P1_IN;
    //
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //
    if (act->getFlush()) {
        MY_LOGD0("need to flush, skip frame processing");
        return;
    };
    IMetadata inAppMetadata;
    IMetadata outAppMetadata;
    IMetadata inHalMetadata;
    IMetadata outHalMetadata;
    MINT64 timestamp = 0;
    MFLOAT aperture = (0.0f);
    MFLOAT focallength = (0.0f);
    MINT64 exposure = 0;
    MINT32 iso = 0;
    MINT32 iso_boost = 0;
    MINT64 duration = 0;
    MUINT8 edge = MTK_EDGE_MODE_OFF;
    MUINT8 noise = MTK_NOISE_REDUCTION_MODE_OFF;
    MFLOAT factor = (1.0f);
    // APP in Meta Stream
    if (OK != act->frameMetadataGet(STREAM_META_IN_APP, &inAppMetadata)) {
        MY_LOGW("cannot get in-app-metadata");
    } else {
        //outAppMetadata = inAppMetadata; // copy all from in-app to out-app
        if (tryGetMetadata< MINT64 >(
            &inAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp)) {
            MY_LOGD1("timestamp from in-app %" PRId64 , timestamp);
        } else {
            MY_LOGI0("cannot find timestamp from in-app");
            timestamp = 0;
        }
        //
        if (tryGetMetadata< MFLOAT >(
            &inAppMetadata, MTK_LENS_APERTURE, aperture)) {
            MY_LOGD1("aperture from in-app %f", aperture);
            if (!trySetMetadata< MFLOAT > (
                &outAppMetadata, MTK_LENS_APERTURE, aperture)) {
                MY_LOGW("cannot update MTK_LENS_APERTURE");
            }
        } else {
            MY_LOGI0("cannot find aperture from in-app");
            aperture = (0.0f);
        }
        if (tryGetMetadata< MFLOAT >(
            &inAppMetadata, MTK_LENS_FOCAL_LENGTH, focallength)) {
            MY_LOGD1("focallength from in-app %f", focallength);
            if (!trySetMetadata< MFLOAT > (
                &outAppMetadata, MTK_LENS_FOCAL_LENGTH, focallength)) {
                MY_LOGW("cannot update MTK_LENS_FOCAL_LENGTH");
            }
        } else {
            MY_LOGI0("cannot find focallength from in-app");
            focallength = (0.0f);
        }
        if (tryGetMetadata< MINT64 >(
            &inAppMetadata, MTK_SENSOR_EXPOSURE_TIME, exposure)) {
            MY_LOGD1("exposure from in-app %" PRId64 , exposure);
            if (!trySetMetadata< MINT64 > (
                &outAppMetadata, MTK_SENSOR_EXPOSURE_TIME, exposure)) {
                MY_LOGW("cannot update MTK_SENSOR_EXPOSURE_TIME");
            }
        } else {
            MY_LOGI0("cannot find exposure from in-app");
            exposure = 0;
        }
        if (tryGetMetadata< MINT32 >(
            &inAppMetadata, MTK_SENSOR_SENSITIVITY, iso)) {
            MY_LOGD1("iso from in-app %" PRId32 , iso);
            if (!trySetMetadata< MINT32 > (
                &outAppMetadata, MTK_SENSOR_SENSITIVITY, iso)) {
                MY_LOGW("cannot update MTK_SENSOR_SENSITIVITY");
            }
        } else {
            MY_LOGI0("cannot find iso from in-app");
            iso = 0;
        }
        if (tryGetMetadata< MINT32 >(
            &inAppMetadata, MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST, iso_boost)) {
            MY_LOGD1("iso boost from in-app %" PRId32 , iso_boost);
            if (!trySetMetadata< MINT32 > (
                &outAppMetadata, MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST, iso_boost)) {
                MY_LOGW("cannot update MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST");
            }
        } else {
            MY_LOGI0("cannot find iso boost from in-app");
            iso_boost = 0;
        }
        if (tryGetMetadata< MINT64 >(
            &inAppMetadata, MTK_SENSOR_FRAME_DURATION, duration)) {
            MY_LOGD1("duration from in-app %" PRId64 , duration);
            if (!trySetMetadata< MINT64 > (
                &outAppMetadata, MTK_SENSOR_FRAME_DURATION, duration)) {
                MY_LOGW("cannot update MTK_SENSOR_FRAME_DURATION");
            }
        } else {
            MY_LOGI0("cannot find duration from in-app");
            duration = 0;
        }
        //
        if (tryGetMetadata< MUINT8 >(
            &inAppMetadata, MTK_EDGE_MODE, edge)) {
            MY_LOGD1("MTK_EDGE_MODE from in-app %d" , edge);
            if (!trySetMetadata< MUINT8 > (
                &outAppMetadata, MTK_EDGE_MODE, edge)) {
                MY_LOGW("cannot update MTK_EDGE_MODE");
            }
        } else {
            MY_LOGI0("cannot find MTK_EDGE_MODE from in-app");
            edge = MTK_EDGE_MODE_OFF;
        }
        if (tryGetMetadata< MUINT8 >(
            &inAppMetadata, MTK_NOISE_REDUCTION_MODE, noise)) {
            MY_LOGD1("MTK_NOISE_REDUCTION_MODE from in-app %d" , noise);
            if (!trySetMetadata< MUINT8 > (
                &outAppMetadata, MTK_NOISE_REDUCTION_MODE, noise)) {
                MY_LOGW("cannot update MTK_NOISE_REDUCTION_MODE");
            }
        } else {
            MY_LOGI0("cannot find MTK_NOISE_REDUCTION_MODE from in-app");
            noise = MTK_NOISE_REDUCTION_MODE_OFF;
        }
        if (tryGetMetadata< MFLOAT >(
            &inAppMetadata, MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR, factor)) {
            MY_LOGD1("MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR from in-app %f" , factor);
            if (!trySetMetadata< MFLOAT > (
                &outAppMetadata, MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR, factor)) {
                MY_LOGW("cannot update MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR");
            }
        } else {
            MY_LOGI0("cannot find MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR from in-app");
            factor = (1.0f);
        }
    };
    // APP out Meta Stream
    if (!trySetMetadata< MINT64 > ( // always set sensor-timestamp
        &outAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp)) {
        MY_LOGW("cannot update MTK_SENSOR_TIMESTAMP");
    }
    //
    if (OK != act->frameMetadataGet(STREAM_META_OUT_APP, NULL,
                MTRUE, &outAppMetadata)) {
        MY_LOGW("cannot write out-app-metadata");
    }
    // HAL in/out Meta Stream
    if (OK != act->frameMetadataGet(STREAM_META_IN_HAL, &inHalMetadata)) {
        MY_LOGW("cannot get in-hal-metadata");
    } else {
        outHalMetadata = inHalMetadata;
        if (!trySetMetadata< MINT32 > (
            &outHalMetadata, MTK_P1NODE_SENSOR_MODE, mSensorParams.mode)) {
            MY_LOGW("cannot update MTK_P1NODE_SENSOR_MODE");
        }
        if (!trySetMetadata< MINT32 > (
            &outHalMetadata, MTK_P1NODE_SENSOR_VHDR_MODE, mSensorParams.vhdrMode)) {
            MY_LOGW("cannot update MTK_P1NODE_SENSOR_MODE");
        }
        if (!trySetMetadata< MRect > (
            &outHalMetadata, MTK_P1NODE_SCALAR_CROP_REGION,
            MRect(mSensorParams.size.w, mSensorParams.size.h))) {
            MY_LOGW("cannot update MTK_P1NODE_SCALAR_CROP_REGION");
        }
        if (!trySetMetadata< MRect > (
            &outHalMetadata, MTK_P1NODE_DMA_CROP_REGION,
            MRect(mSensorParams.size.w, mSensorParams.size.h))) {
            MY_LOGW("cannot update MTK_P1NODE_DMA_CROP_REGION");
        }
        if (!trySetMetadata< MSize > (
            &outHalMetadata, MTK_P1NODE_RESIZER_SIZE, mSensorParams.size)) {
            MY_LOGW("cannot update MTK_P1NODE_RESIZER_SIZE");
        }
        if (OK != act->frameMetadataGet(STREAM_META_OUT_HAL, NULL,
                    MTRUE, &outHalMetadata)) {
            MY_LOGW("cannot write out-hal-metadata");
        }
    };
    //
    FUNCTION_P1_OUT;
    //
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
releaseAction(
    P1QueAct & rAct
)
{
    FUNCTION_P1_IN;
    //
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:release|Mnum:%d SofIdx:%d Fnum:%d Rnum:%d "
        "FlushSet:0x%x", act->magicNum, act->sofIdx, act->frmNum, act->reqNum,
        act->flushSet);
    //
    MY_LOGD3(P1INFO_ACT_STR " begin", P1INFO_ACT_VAR(*act));
    //
    if (!act->getFlush()) {
        if (act->reqType == REQ_TYPE_REDO) {
            processRedoFrame(rAct);
        } else if (act->reqType == REQ_TYPE_YUV) {
            processYuvFrame(rAct);
        } else if (act->reqType == REQ_TYPE_NORMAL) {
            if ((act->appFrame != NULL) && act->isReadoutReady) {
                checkBufferDumping(rAct);
            }
        }
    };
    //
    for (int stream = STREAM_ITEM_START; stream < STREAM_META_NUM; stream++) {
        if (act->streamBufMeta[stream].bExist) {
            if (OK != act->frameMetadataPut((STREAM_META)stream)) {
                MY_LOGD0("cannot put metadata stream(%d)", stream);
            }
        };
    };
    //
    for (int stream = STREAM_ITEM_START; stream < STREAM_IMG_NUM; stream++) {
        if ((!act->streamBufImg[stream].bExist) && // for INITIAL act
            (act->streamBufImg[stream].eSrcType == IMG_BUF_SRC_NULL)) {
            continue; // this stream is not existent and no pool/stuff buffer
        }
        switch (act->streamBufImg[stream].eSrcType) {
            case IMG_BUF_SRC_STUFF:
                if (OK != act->stuffImagePut((STREAM_IMG)stream)) {
                    MY_LOGD0("cannot put stuff image stream(%d)", stream);
                };
                break;
            case IMG_BUF_SRC_POOL:
                if (OK != act->poolImagePut((STREAM_IMG)stream)) {
                    MY_LOGD0("cannot put pool image stream(%d)", stream);
                };
                break;
            case IMG_BUF_SRC_FRAME:
            case IMG_BUF_SRC_NULL: // for flush act, buf src is not decided
                if (OK != act->frameImagePut((STREAM_IMG)stream)) {
                    MY_LOGD0("cannot put frame image stream(%d)", stream);
                };
                break;
            default:
                MY_LOGW("act buffer source is not defined");
                MY_LOGW("check act exe "
                    P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                break;
        };
    };
    //
    U_if (act->byDeliverReleasing) {
        if (LOGI_LV1) {
            act->res += String8::format(" [DeliverReleasing]");
        }
    }
    //
    if (LOGI_LV1) {
        act->res += String8::format(
            " [MetaCnt] In(%d,%d) Out(%d,%d) Aaa(%d,%d)",
            act->metaCntInAPP, act->metaCntInHAL,
            act->metaCntOutAPP, act->metaCntOutHAL,
            act->metaCntAaaAPP, act->metaCntAaaHAL);
    }
    //
    if (act->getType() == ACT_TYPE_INTERNAL) {
        mTagOut.set(rAct.getNum());
        if (LOGI_LV1) {
            P1_TRACE_F_BEGIN(SLG_PFL, "P1::DEQ_LOG|Mnum:%d SofIdx:%d Fnum:%d "
                "Rnum:%d FlushSet:0x%x", act->magicNum, act->sofIdx,
                act->frmNum, act->reqNum, act->flushSet);
            P1_LOGI(1, "%s [InternalReturn]", act->res.string());
            P1_TRACE_C_END(SLG_PFL); // "P1::DEQ_LOG"
        }
        MY_LOGD3(P1INFO_ACT_STR " INTERNAL return", P1INFO_ACT_VAR(*act));
        if (mpTaskCtrl != NULL) {
            mpTaskCtrl->releaseAct(rAct);
            /* DO NOT use this P1QueAct after releaseAct() */
        }
        P1_TRACE_C_END(SLG_I); // "P1:release"
        return;
    }
    //
    MY_LOGD3(P1INFO_ACT_STR " applyRelease", P1INFO_ACT_VAR(*act));
    //
    // Apply buffers to release
    IStreamBufferSet& rStreamBufferSet = act->appFrame->getStreamBufferSet();
    //
    if (LOGI_LV1) {
        P1_TRACE_F_BEGIN(SLG_PFL, "P1::DEQ_LOG|Mnum:%d SofIdx:%d Fnum:%d "
            "Rnum:%d FlushSet:0x%x", act->magicNum, act->sofIdx, act->frmNum,
            act->reqNum, act->flushSet);
        android::String8 strInfo("");
        strInfo.appendFormat("%s [ApplyRelease]", act->res.string());
        mNoteRelease.get(&strInfo);
        P1_LOGI(1, "%s", strInfo.string());
        P1_TRACE_C_END(SLG_PFL); // "P1::DEQ_LOG"
    };
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:applyRelease%s",
        (act->byDeliverReleasing) ? "_Deliver" : "");
    rStreamBufferSet.applyRelease(getNodeId());
    P1_TRACE_C_END(SLG_I); // "P1:applyRelease"
    //
    if (LOGI_LV1) {
        mNoteRelease.set(act->frmNum);
    };
    //
    MY_LOGD3(P1INFO_ACT_STR " end", P1INFO_ACT_VAR(*act));

    if (mpTaskCtrl != NULL) {
        mpTaskCtrl->releaseAct(rAct);
        /* DO NOT use this P1QueAct after releaseAct() */
    }

    P1_TRACE_C_END(SLG_I); // "P1:release"
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
releaseFrame(
    P1FrameAct & rFrameAct
)
{
    FUNCTION_P1_IN;
    //
    U_if (rFrameAct.appFrame == NULL) {
        MY_LOGE("PipelineFrame is NULL - "
            P1INFO_ACT_STR, P1INFO_ACT_VAR(rFrameAct));
        return;
    }
    //
    U_if (rFrameAct.byDeliverReleasing) {
        P1QueAct * pQueAct = rFrameAct.getQueAct();
        U_if (pQueAct == NULL) { // must check QueAct ready before use it
            MY_LOGE("QueAct not ready in DeliverReleasing - "
                P1INFO_ACT_STR, P1INFO_ACT_VAR(rFrameAct));
            return;
        }
        releaseAction(*pQueAct);
        MY_LOGD3("delQueAct +++");
        rFrameAct.delQueAct(); // clean QueAct and release its P1Act after releaseAction()
        MY_LOGD3("delQueAct ---");
        /* DO NOT use this P1QueAct after releaseAction() / sendActQueue() */
    }
    //
    #if (USING_DRV_IO_PIPE_EVENT)
    eventStreamingInform();
    #endif
    //
    LogInfo::AutoMemo _m(mLogInfo, LogInfo::CP_OUT_BGN, LogInfo::CP_OUT_END,
        rFrameAct.magicNum, rFrameAct.frmNum, rFrameAct.reqNum);
    //
    MINT32 currReqCnt = 0;
    currReqCnt = android_atomic_dec(&mInFlightRequestCnt);
    P1_TRACE_INT(SLG_B, "P1_request_cnt",
        android_atomic_acquire_load(&mInFlightRequestCnt));
    MY_LOGD3("InFlightRequestCount-- (%d) => (%d)", currReqCnt,
        android_atomic_acquire_load(&mInFlightRequestCnt));
    //
    #if 1
    // camera display systrace - Dispatch
    if (rFrameAct.reqType == REQ_TYPE_NORMAL &&
        rFrameAct.appFrame != NULL &&
        rFrameAct.frameTimeStamp > 0) {
        MINT64 const timestamp = rFrameAct.frameTimeStamp;
        P1_TRACE_F_BEGIN(SLG_B, // add information
            "Cam:%d:IspP1:dispatch|timestamp(ns):%" PRId64
            " duration(ns):%" PRId64
            " request:%d frame:%d",
            getOpenId(), timestamp, ::systemTime() - timestamp,
            rFrameAct.appFrame->getRequestNo(), rFrameAct.appFrame->getFrameNo()
        );
        P1_TRACE_C_END(SLG_B); // "IspP1:dispatch"
        /*
        if  ( ATRACE_ENABLED() ) {
            MINT64 const timestamp = rFrameAct.frameTimeStamp;
            String8 const str = String8::format(
                "Cam:%d:IspP1:dispatch|timestamp(ns):%" PRId64
                " duration(ns):%" PRId64
                " request:%d frame:%d",
                getOpenId(), timestamp, ::systemTime()- timestamp,
                rFrameAct.appFrame->getRequestNo(), rFrameAct.appFrame->getFrameNo()
            );
            CAM_TRACE_BEGIN(str.string());
            CAM_TRACE_END();
        }
        */
    }
    #endif
    //
    L_if (rFrameAct.reqType == REQ_TYPE_NORMAL) {
        mTagOut.set(rFrameAct.magicNum);
    }
    if (LOGI_LV1) {
        P1_TRACE_F_BEGIN(SLG_PFL, "P1::OUT_LOG|Mnum:%d SofIdx:%d Fnum:%d "
            "Rnum:%d FlushSet:0x%x", rFrameAct.magicNum, rFrameAct.sofIdx,
            rFrameAct.frmNum, rFrameAct.reqNum, rFrameAct.flushSet);
        android::String8 strInfo("");
        strInfo.appendFormat("[P1::OUT]" P1INFO_ACT_STR
            " [Release-%d] [DispatchFrame]", P1INFO_ACT_VAR(rFrameAct),
            ((rFrameAct.flushSet == FLUSH_NONEED) ? 0 : 1));
        mNoteDispatch.get(&strInfo);
        P1_LOGI(1, "%s", strInfo.string());
        P1_TRACE_C_END(SLG_PFL); // "P1::OUT_LOG"
    };
    //
    P1_TRACE_F_BEGIN(SLG_I, "onDispatchFrame|Mnum:%d SofIdx:%d Fnum:%d Rnum:%d "
        "FlushSet:0x%x", rFrameAct.magicNum, rFrameAct.sofIdx, rFrameAct.frmNum,
        rFrameAct.reqNum, rFrameAct.flushSet);
    //
    // dispatch to next node
    dispatch(rFrameAct.appFrame);
    //
    MY_LOGI3("[Dispatch-Return] " P1INFO_ACT_STR " (m_%" PRId64 ") "
        "(b_%" PRId64 ")", P1INFO_ACT_VAR(rFrameAct),
        rFrameAct.frameTimeStamp, rFrameAct.frameTimeStampBoot);
    //
    if (LOGI_LV1) {
        mNoteDispatch.set(rFrameAct.frmNum);
    };
    //
    P1_TRACE_C_END(SLG_I); // "onDispatchFrame"
    //
    FUNCTION_P1_OUT;
    //
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
dispatch(
    sp<IPipelineFrame> pFrame
)
{
    FUNCTION_P1_IN;
    //
    // dispatch to next node
    MY_LOGD2("onDispatchFrame +++ FrameNum(%d) RequestNum(%d)",
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    onDispatchFrame(pFrame);
    MY_LOGD2("onDispatchFrame --- FrameNum(%d) RequestNum(%d)",
        P1GET_FRM_NUM(pFrame), P1GET_REQ_NUM(pFrame));
    //
    FUNCTION_P1_OUT;
    //
    return;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
requestMetadataEarlyCallback(
    P1QueAct & rAct, STREAM_META const streamMeta,
    IMetadata * pMetadata
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    //
    P1_CHECK_STREAM_SET(META, streamMeta);
    P1_CHECK_MAP_STREAM(Meta, (*act), streamMeta);
    //
    if (pMetadata == NULL) {
        MY_LOGD1("Result Metadata is Null");
        return BAD_VALUE;
    }
    if (pMetadata->count() == 0) {
        MY_LOGD1("Result Metadata is Empty");
        return OK;
    }
    MY_LOGD2("Meta[%d]=(%d) EarlyCB " P1INFO_ACT_STR,
        streamMeta, pMetadata->count(), P1INFO_ACT_VAR(*act));
    //
    IMetadata outMetadata = *(pMetadata);
    DurationProfile duration("EarlyCB", 5000000LL); // 5ms
    duration.pulse_up();
    P1_TRACE_S_BEGIN(SLG_I, "EarlyCB");
    onEarlyCallback(act->appFrame, mvStreamMeta[streamMeta]->getStreamId(),
        outMetadata);
    P1_TRACE_C_END(SLG_I); // "EarlyCB"
    duration.pulse_down();
    if (duration.isWarning()) {
        MY_LOGI0("EarlyCB Meta[%d]=(%d) " P1INFO_ACT_STR,
        streamMeta, pMetadata->count(), P1INFO_ACT_VAR(*act));
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
notifyCtrlSync(
    P1QueAct & rAct
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    //
    MY_LOGI2("CtrlCb_Sync[%d] sof(%d) cap(%d) exp(%" PRId64 "ns) +++ "
        P1INFO_ACT_STR, IPipelineNodeCallback::eCtrl_Sync,
        act->sofIdx, act->capType, act->frameExpDuration, P1INFO_ACT_VAR(*act));
    DurationProfile duration("CtrlCb_Sync", 3000000LL); // 3ms
    duration.pulse_up();
    P1_TRACE_F_BEGIN(SLG_I, "CtrlCb_Sync[%d]",
        IPipelineNodeCallback::eCtrl_Sync);
    onCtrlSync(act->appFrame, act->sofIdx, act->capType, act->frameExpDuration);
    P1_TRACE_C_END(SLG_I); // "CtrlCb_Sync"
    duration.pulse_down();
    if (duration.isWarning()) {
        MY_LOGI0("CtrlCb_Sync[%d] sof(%d) cap(%d) exp(%" PRId64 "ns) "
            P1INFO_ACT_STR, IPipelineNodeCallback::eCtrl_Sync,
            act->sofIdx, act->capType, act->frameExpDuration,
            P1INFO_ACT_VAR(*act));
    }
    MY_LOGI2("CtrlCb_Sync[%d] sof(%d) cap(%d) exp(%" PRId64 "ns) --- "
        P1INFO_ACT_STR, IPipelineNodeCallback::eCtrl_Sync,
        act->sofIdx, act->capType, act->frameExpDuration, P1INFO_ACT_VAR(*act));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
notifyCtrlMeta(
    IPipelineNodeCallback::eCtrlType eType, P1QueAct & rAct,
    STREAM_META const streamAppMeta, IMetadata * pAppMetadata,
    STREAM_META const streamHalMeta, IMetadata * pHalMetadata,
    MBOOL & rIsChanged
)
{
    rIsChanged = MFALSE;
    nsecs_t nsWarning = 3000000LL; // 3ms
    MBOOL bChangeLog = LOGI_LV0;
    switch (eType) {
        case IPipelineNodeCallback::eCtrl_Resize:
            nsWarning = 2000000LL;
            bChangeLog = LOGI_LV2;
            break;
        case IPipelineNodeCallback::eCtrl_Setting:
        case IPipelineNodeCallback::eCtrl_Readout:
            break;
        default: //IPipelineNodeCallback::eCtrl_Sync
            return OK;
    }
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    MUINT cntApp = 0;
    MUINT cntHal = 0;
    P1_CHECK_STREAM_SET(META, streamAppMeta);
    P1_CHECK_STREAM_SET(META, streamHalMeta);
    P1_CHECK_MAP_STREAM(Meta, (*act), streamAppMeta);
    P1_CHECK_MAP_STREAM(Meta, (*act), streamHalMeta);
    U_if (pAppMetadata == NULL) {
        MY_LOGD1("AppMetadata is Null");
        return BAD_VALUE;
    } else {
        cntApp = pAppMetadata->count();
    }
    U_if (pHalMetadata == NULL) {
        MY_LOGD1("HalMetadata is Null");
        return BAD_VALUE;
    } else {
        cntHal = pHalMetadata->count();
    }
    MY_LOGI2("CtrlCb_Meta[%d] AppMeta[%d]=(%d) HalMeta[%d]=(%d) "
        P1INFO_ACT_STR, eType,
        streamAppMeta, cntApp, streamHalMeta, cntHal, P1INFO_ACT_VAR(*act));
    //
    IMetadata & rAppMetadata = (*(pAppMetadata));
    IMetadata & rHalMetadata = (*(pHalMetadata));
    MBOOL isChanged = MFALSE;
    DurationProfile duration("CtrlCb_Meta", nsWarning);
    duration.pulse_up();
    P1_TRACE_F_BEGIN(SLG_I, "CtrlCb_Meta[%d]", eType);
    if (eType == IPipelineNodeCallback::eCtrl_Setting) {
        onCtrlSetting(act->appFrame,
            mvStreamMeta[streamAppMeta]->getStreamId(), rAppMetadata,
            mvStreamMeta[streamHalMeta]->getStreamId(), rHalMetadata,
            isChanged);
    } else if (eType == IPipelineNodeCallback::eCtrl_Readout) {
        onCtrlReadout(act->appFrame,
            mvStreamMeta[streamAppMeta]->getStreamId(), rAppMetadata,
            mvStreamMeta[streamHalMeta]->getStreamId(), rHalMetadata,
            isChanged);
    } else { // eType == IPipelineNodeCallback::eCtrl_Resize
        onCtrlResize(act->appFrame,
            mvStreamMeta[streamAppMeta]->getStreamId(), rAppMetadata,
            mvStreamMeta[streamHalMeta]->getStreamId(), rHalMetadata,
            isChanged);
    }
    P1_TRACE_C_END(SLG_I); // "CtrlCb_Meta"
    duration.pulse_down();
    if (duration.isWarning() || (isChanged && bChangeLog)) { // for log only
        char str[32] = {0};
        switch (eType) {
            case IPipelineNodeCallback::eCtrl_Setting:
                snprintf(str, sizeof(str), "CtrlCb_Meta[%d]-Setting", eType);
                break;
            case IPipelineNodeCallback::eCtrl_Readout:
                snprintf(str, sizeof(str), "CtrlCb_Meta[%d]-Readout", eType);
                break;
            case IPipelineNodeCallback::eCtrl_Resize:
                snprintf(str, sizeof(str), "CtrlCb_Meta[%d]-Resize", eType);
                break;
            default: //IPipelineNodeCallback::eCtrl_Sync
                return OK;
        }
        if (duration.isWarning()) {
            MY_LOGI0("%s sof(%d) cap(%d) exp(%" PRId64 "ns) " P1INFO_ACT_STR,
                str, act->sofIdx, act->capType,
                act->frameExpDuration, P1INFO_ACT_VAR(*act));
        }
        if (isChanged && bChangeLog) {
            MY_LOGI0("%s change AppMeta[%d]=(%d-%d) HalMeta[%d]=(%d-%d) "
                P1INFO_ACT_STR, str, streamAppMeta, cntApp,
                rAppMetadata.count(), streamHalMeta, cntHal,
                rHalMetadata.count(), P1INFO_ACT_VAR(*act));
        }
    }
    rIsChanged = isChanged;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
attemptCtrlSync(
    P1QueAct & rAct
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    if ((act->appFrame != NULL) &&
        needCtrlCb(act->appFrame, IPipelineNodeCallback::eCtrl_Sync)) {
        notifyCtrlSync(rAct);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
attemptCtrlSetting(
    P1QueAct & rAct
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    MBOOL bIsChanged = MFALSE;
    if ((act->appFrame != NULL) &&
        needCtrlCb(act->appFrame, IPipelineNodeCallback::eCtrl_Setting)) {
        notifyCtrlMeta(IPipelineNodeCallback::eCtrl_Setting, rAct,
            STREAM_META_IN_APP, &(act->metaSet.appMeta),
            STREAM_META_IN_HAL, &(act->metaSet.halMeta), bIsChanged);
    }
    if (bIsChanged) {
        act->metaSet.PreSetKey = P1_PRESET_KEY_NULL;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
attemptCtrlResize(
    P1QueAct & rAct, MBOOL & rIsChanged
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    MBOOL isChanged = MFALSE;
    if ((act->appFrame != NULL) &&
        needCtrlCb(act->appFrame, IPipelineNodeCallback::eCtrl_Resize)) {
        IMetadata revAppMeta;
        IMetadata revHalMeta;
        notifyCtrlMeta(IPipelineNodeCallback::eCtrl_Resize, rAct,
            STREAM_META_IN_APP, &revAppMeta,
            STREAM_META_IN_HAL, &revHalMeta, isChanged);
        if (isChanged) {
            MBOOL ctrlFlush = MFALSE;
            prepareCropInfo(rAct, &revAppMeta, &revHalMeta,
                PREPARE_CROP_PHASE_CONTROL_RESIZE, &ctrlFlush);
            if (ctrlFlush) {
                act->setFlush(FLUSH_MIS_RESIZE);
            }
        }
    }
    rIsChanged = isChanged;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
attemptCtrlReadout(
    P1QueAct & rAct, IMetadata * pAppMetadata, IMetadata * pHalMetadata,
    MBOOL & rIsChanged
)
{
    P1Act act = GET_ACT_PTR(act, rAct, BAD_VALUE);
    MBOOL isChanged = MFALSE;
    if ((act->appFrame != NULL) &&
        needCtrlCb(act->appFrame, IPipelineNodeCallback::eCtrl_Readout)) {
        notifyCtrlMeta(IPipelineNodeCallback::eCtrl_Readout, rAct,
            STREAM_META_OUT_APP, pAppMetadata,
            STREAM_META_OUT_HAL, pHalMetadata, isChanged);
        if (isChanged) {
            MBOOL outFlush = MFALSE;
            if (tryGetMetadata<MINT32>(pHalMetadata,
                MTK_P1NODE_CTRL_READOUT_FLUSH, outFlush) && (outFlush)) {
                act->setFlush(FLUSH_MIS_READOUT);
            }
        }
    }
    rIsChanged = isChanged;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
findPortBufIndex(QBufInfo & deqBuf, P1QueJob & job) {
    size_t job_size = job.size();
    if ((job_size == 0) || (deqBuf.mvOut.size() % job_size > 0)) {
        MY_LOGE("Output size is not match");
        return MFALSE;
    };
    // assume the port order is the same in each de-queue set,
    // it only check the first de-queue set and apply to each act
    P1_OUTPUT_PORT port = P1_OUTPUT_PORT_TOTAL;
    MUINT32 group = 0;
    MUINT32 index = 0;
    for (size_t i = 0; i < (deqBuf.mvOut.size()); i += job_size) {
        index = deqBuf.mvOut[i].mPortID.index;
        port = P1_OUTPUT_PORT_TOTAL;
        if (index == PORT_RRZO.index) {
            port = P1_OUTPUT_PORT_RRZO;
        } else if (index == PORT_IMGO.index) {
            port = P1_OUTPUT_PORT_IMGO;
        } else if (index == PORT_EISO.index) {
            port = P1_OUTPUT_PORT_EISO;
        } else if (index == PORT_LCSO.index) {
            port = P1_OUTPUT_PORT_LCSO;
        } else if (index == PORT_RSSO.index) {
            port = P1_OUTPUT_PORT_RSSO;
        } else {
            MY_LOGE("Output port is not match : [%zu/%zu] index(0x%x) : "
                "IMGO(0x%x) RRZO(0x%x) LCSO(0x%x) RSSO(0x%x) EISO(0x%x)",
                i, (deqBuf.mvOut.size()), index, PORT_IMGO.index,
                PORT_RRZO.index, PORT_LCSO.index,
                PORT_RSSO.index, PORT_EISO.index);
            return MFALSE;
        }
        //
        if (port < P1_OUTPUT_PORT_TOTAL) {
            for (size_t j = 0; j < job_size; j++) {
                P1Act act = GET_ACT_PTR(act, job.edit(j), MFALSE);
                act->portBufIndex[port] =
                    (group * job_size) + j;
            }
        }
        group ++;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
checkBufferDumping(P1QueAct & rAct)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    #if (SUPPORT_BUFFER_TUNING_DUMP)
    if (mCamDumpEn == 0) {
        return;
    } else {
        MINT32 nDumpIMGO = property_get_int32("vendor.debug.camera.dump.p1.imgo", 0);
        if (nDumpIMGO == 0) {
            return;
        } else {
            P1_TRACE_AUTO(SLG_E, "P1:BufferDumping");
            MY_LOGI0("[DUMP_IMGO] " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
            //
            if (act->reqType == REQ_TYPE_NORMAL) {
                IMetadata outHalMetadata;
                if (OK != act->frameMetadataGet(STREAM_META_OUT_HAL,
                    &outHalMetadata)) {
                    MY_LOGW("[DUMP_IMGO] cannot get out-hal-metadata");
                    return;
                }
                //
                sp<IImageBuffer> imgBuf;
                if (mvStreamImg[STREAM_IMG_OUT_FULL] == NULL){
                    MY_LOGW("[DUMP_IMGO] StreamImg FULL not exist");
                    return;
                } else {
                #if 1
                    MY_LOGI0("[DUMP_IMGO] map(%d) type(%d) state(%d) [%p]",
                        act->streamBufImg[STREAM_IMG_OUT_FULL].bExist,
                        act->streamBufImg[STREAM_IMG_OUT_FULL].eSrcType,
                        act->streamBufImg[STREAM_IMG_OUT_FULL].eLockState,
                        act->streamBufImg[STREAM_IMG_OUT_FULL].spImgBuf.get());
                #endif
                    imgBuf = act->streamBufImg[STREAM_IMG_OUT_FULL].spImgBuf;
                }
                if (imgBuf == NULL) {
                    MY_LOGW("[DUMP_IMGO] cannot get ImageBuffer");
                    return;
                }
                //
                NSCam::TuningUtils::FILE_DUMP_NAMING_HINT hint;
                MBOOL res = MTRUE;
                res= extract(&hint, &outHalMetadata);
                if (!res) {
                    MY_LOGW("[DUMP_IMGO] extract with metadata fail (%d)", res);
                    return;
                }
                //
                res= extract(&hint, imgBuf.get());
                if (!res) {
                    MY_LOGW("[DUMP_IMGO] extract with ImgBuf fail (%d)", res);
                    return;
                }
                //
                res= extract_by_SensorOpenId(&hint, getOpenId());
                if (!res) {
                    MY_LOGW("[DUMP_IMGO] extract with OpenId fail (%d)", res);
                    return;
                }
                //
                {
                    char file_name[512];
                    ::memset(file_name, 0, sizeof(file_name));
                    genFileName_RAW(file_name, sizeof(file_name), &hint,
                        NSCam::TuningUtils::RAW_PORT_IMGO);
                    P1_TRACE_AUTO(SLG_E, file_name);
                    MBOOL ret = imgBuf->saveToFile(file_name);
                    MY_LOGI0("[DUMP_IMGO] SaveFile[%s]:(%d)", file_name, ret);
                }
            } else {
                MY_LOGI0("[DUMP_IMGO] not-apply (%d)", act->reqType);
                return;
            }
        }
    }
    #endif
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
inflightMonitoring(INFLIGHT_MONITORING_TIMING timing)
{
    MINT64 currentTime = (MINT64)(::systemTime());
    MBOOL trigger = MFALSE;
    {
        Mutex::Autolock _l(mMonitorLock);
        if (currentTime > (mMonitorTime + (P1_PERIODIC_INSPECT_INV_NS))) {
            mMonitorTime = currentTime;
            trigger = MTRUE;
        }
    }
    if (trigger) {
        char str[128] = {0};
        MINT32 cnt = (MINT32)android_atomic_acquire_load(&mInFlightRequestCnt);
        switch (timing) {
            case IMT_REQ:
                snprintf(str, sizeof(str), "[%d:AfterRequestReceived]"
                    "[Burst=%d Count=%d]", timing, mBurstNum, cnt);
                break;
            case IMT_ENQ:
                snprintf(str, sizeof(str), "[%d:AfterEnQ]"
                    "[Burst=%d Count=%d]", timing, mBurstNum, cnt);
                break;
            case IMT_DEQ:
                snprintf(str, sizeof(str), "[%d:AfterDeQ]"
                    "[Burst=%d Count=%d]", timing, mBurstNum, cnt);
                break;
            default: // IMT_COMMON
                snprintf(str, sizeof(str), "[%d:CommonCase]"
                    "[Burst=%d Count=%d]", timing, mBurstNum, cnt);
                break;
        }
        mLogInfo.inspect(LogInfo::IT_PERIODIC_CHECK, str);
    }
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onReturnFrame(P1QueAct & rAct, FLUSH_TYPE flushType, MBOOL isTrigger)
{
    P1Act act = GET_ACT_PTR(act, rAct, RET_VOID);
    //
    P1_TRACE_F_BEGIN(SLG_I, "P1:return|Mnum:%d SofIdx:%d Fnum:%d Rnum:%d",
        act->magicNum, act->sofIdx, act->frmNum, act->reqNum);
    //
    if (flushType != FLUSH_NONEED) {
        act->setFlush(flushType);
    }
    if (act->getFlush() && isActive()) {
        MY_LOGI1("need flush act " P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
    };
    act->exeState = EXE_STATE_DONE;
    //
    if (LOGI_LV1) {
        U_if (!act->isReadoutReady) {
            android::String8 strInfo("");
            strInfo += String8::format("[P1::DEL]" P1INFO_ACT_STR " Readout(%d)"
                " Bypass(%d) " , P1INFO_ACT_VAR(*act), act->isReadoutReady,
                ((act->getType() == ACT_TYPE_BYPASS) ? MTRUE : MFALSE));
            act->res += strInfo;
        }
    };
    //
    L_if (ACT_TYPE_INTERNAL != rAct.getType()) {
        L_if (act->appFrame != NULL) {
            U_if (mCamDumpEn) { // for NDD case, consider the performance of data dump in de-queue thread
                act->byDeliverReleasing = MTRUE;
                U_if (mDisableDeliverReleasing) { // Disable Deliver Dump Data
                    act->byDeliverReleasing = MFALSE;
                }
            }
            L_if (!act->byDeliverReleasing) {
                releaseAction(rAct);
            } // else the releaseAction() will execute in releaseFrame()
            //
            if (mpDeliverMgr != NULL && mpDeliverMgr->runningGet()) {
                mpDeliverMgr->sendActQueue(rAct, isTrigger); // execute releaseFrame() on DeliverThread
            } else {
                P1FrameAct frameAct(rAct);
                L_if (frameAct.ready()) {
                    releaseFrame(frameAct);
                } else {
                    MY_LOGE("FrameAct not ready to release - "
                        P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
                }
            }
        } else {
            MY_LOGE("PipelineFrame is NULL - "
                P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
        }
    } else { // (ACT_TYPE_INTERNAL == rAct.getType())
        releaseAction(rAct);
    }
    /* DO NOT use this P1QueAct after releaseAction() / sendActQueue() */
    P1_TRACE_C_END(SLG_I); // "P1:return"
    return;
};

#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IndependentVerification Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::IndependentVerification::
exe(void) {
    MBOOL bLoop = MTRUE;
    android::sp<P1NodeImp> spP1NodeImp = mwpP1NodeImp.promote();
    U_if (spP1NodeImp == NULL) {
        MY_LOGI0("[P1_IV] exit");
        bLoop = MFALSE;
        return bLoop;
    };
    //
    #if 0 // only for IO-Pipe-Event verification
    #warning "[FIXME] IO-Pipe-Event verification"
    #if (P1NODE_USING_DRV_IO_PIPE_EVENT > 0)
    {   // the counting number based on IntervalMs:100
        NSCam::NSIoPipe::IpRawP1AcquiringEvent acquiringEvent;
        NSCam::NSIoPipe::IpRawP1ReleasedEvent releasedEvent;
        NSCam::NSIoPipe::IoPipeEventSystem & evtSystem =
            NSCam::NSIoPipe::IoPipeEventSystem::getGlobal();
        MUINT32 cnt = (mCount % 50) + 1;
        if (cnt % 30 == 0) {
            MY_LOGI0("[P1_IV] sendSyncEvent(acquiringEvent) +++");
            evtSystem.sendSyncEvent(acquiringEvent);
            MY_LOGI0("[P1_IV] sendSyncEvent(acquiringEvent) ---");
        }
        if (cnt % 50 == 0) {
            MY_LOGI0("[P1_IV] sendSyncEvent(releasedEvent) +++");
            evtSystem.sendSyncEvent(releasedEvent);
            MY_LOGI0("[P1_IV] sendSyncEvent(releasedEvent) ---");
        }
    }
    #endif
    #endif
    //
    { // for Periodic Inspection
        MY_LOGI0("[P1_IV] InflightMonitoring +++");
        spP1NodeImp->inflightMonitoring(IMT_COMMON);
        MY_LOGI0("[P1_IV] InflightMonitoring ---");
    };
    //
    return bLoop;
};
#endif

};//namespace NSP1Node
};//namespace v3
};//namespace NSCam

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
sp<P1Node>
P1Node::
createInstance()
{
    return new NSCam::v3::NSP1Node::P1NodeImp();
}


