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

#include "StreamingFeaturePipe.h"

#define PIPE_CLASS_TAG "Pipe"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_PIPE
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>

#define NORMAL_STREAM_NAME "StreamingFeature"

#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);
using namespace NSCam::Utils::ULog;

using namespace NSCam::NSIoPipe::NSPostProc;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

StreamingFeaturePipe::StreamingFeaturePipe(MUINT32 sensorIndex, const UsageHint &usageHint)
    : CamPipe<StreamingFeatureNode>("StreamingFeaturePipe")
    , mForceOnMask(0)
    , mForceOffMask(~0)
    , mSensorIndex(sensorIndex)
    , mPipeUsage(usageHint, sensorIndex)
    , mCounter(0)
    , mRecordCounter(0)
    , mDebugDump(0)
    , mDebugDumpCount(1)
    , mDebugDumpByRecordNo(MFALSE)
    , mForceIMG3O(MFALSE)
    , mForceWarpPass(MFALSE)
    , mForceGpuOut(NO_FORCE)
    , mForceGpuRGBA(MFALSE)
    , mUsePerFrameSetting(MFALSE)
    , mForcePrintIO(MFALSE)
    , mEarlyInited(MFALSE)
    , mRootNode("fpipe.root")
    , mP2A("fpipe.p2a")
    , mP2AMDP("fpipe.p2amdp")
    , mP2SM("fpipe.p2sm")
    , mP2NR("fpipe.p2nr")
    , mVNR("fpipe.vnr")
    , mAsync("fpipe.async")
    , mDisp("fpipe.disp")
    , mWarp("fpipe.warp")
    , mDepth("fpipe.depth")
    , mBokeh("fpipe.bokeh")
    , mEIS("fpipe.eis")
    , mRSC("fpipe.rsc")
    , mHelper("fpipe.helper")
    , mVMDP("fpipe.vmdp")
{
    TRACE_FUNC_ENTER();

    MY_LOGI("create pipe(%p): SensorIndex=%d sensorNum(%d) dualMode(%d),UsageMode=%d EisMode=0x%x 3DNRMode=%d DSDN(mode/m,d)=(%d/%d,%d) tsq=%d \
         StreamingSize=%dx%d Out(max/phy/large/fd/video)=(%d/%d/%d/%dx%d/%dx%d), secType=%d", this, mSensorIndex, mPipeUsage.getNumSensor(),
        mPipeUsage.getDualMode(), mPipeUsage.getMode(), mPipeUsage.getEISMode(), mPipeUsage.get3DNRMode(),
        usageHint.mDSDNParam.mMode, usageHint.mDSDNParam.mMaxRatioMultiple, usageHint.mDSDNParam.mMaxRatioDivider, usageHint.mUseTSQ,
        usageHint.mStreamingSize.w, usageHint.mStreamingSize.h,
        usageHint.mOutCfg.mMaxOutNum, usageHint.mOutCfg.mHasPhysical, usageHint.mOutCfg.mHasLarge, usageHint.mOutCfg.mFDSize.w, usageHint.mOutCfg.mFDSize.h,
        usageHint.mOutCfg.mVideoSize.w, usageHint.mOutCfg.mVideoSize.h, usageHint.mSecType);

    mAllSensorIDs = mPipeUsage.getAllSensorIDs();
    mNodeSignal = new NodeSignal();
    if( mNodeSignal == NULL )
    {
        MY_LOGE("OOM: cannot create NodeSignal");
    }

    for( int i = 0; i < P2CamContext::SENSOR_INDEX_MAX; i++ )
    {
        mContextCreated[i] = MFALSE;
    }

    mEarlyInited = earlyInit();
    TRACE_FUNC_EXIT();
}

StreamingFeaturePipe::~StreamingFeaturePipe()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("destroy pipe(%p): SensorIndex=%d", this, mSensorIndex);
    lateUninit();
    // must call dispose to free CamGraph
    this->dispose();
    TRACE_FUNC_EXIT();
}

void StreamingFeaturePipe::setSensorIndex(MUINT32 sensorIndex)
{
    TRACE_FUNC_ENTER();
    this->mSensorIndex = sensorIndex;
    TRACE_FUNC_EXIT();
}

MBOOL StreamingFeaturePipe::init(const char *name)
{
    TRACE_FUNC_ENTER();
    (void)name;
    MBOOL ret = MFALSE;

    mTPIMgr = TPIMgr::createInstance();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI init_config");
    mTPIMgr->createSession(mSensorIndex, mAllSensorIDs, mPipeUsage.getNumSensor(), mPipeUsage.getTPMask(), mPipeUsage.getTPMarginRatio(), mPipeUsage.getStreamingSize(), mPipeUsage.getMaxOutSize(), mPipeUsage.getAppSessionMeta(), mPipeUsage.getTPICustomConfig());
    mTPIMgr->initSession();
    mPipeUsage.config(mTPIMgr);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    initNodes();
    mEISQControl.init(mPipeUsage);

    ret = PARENT_PIPE::init();

    mFPDebugee = new FeaturePipeDebugee<StreamingFeaturePipe>(this);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::uninit(const char *name)
{
    TRACE_FUNC_ENTER();
    (void)name;
    MBOOL ret;
    ret = PARENT_PIPE::uninit();

    uninitNodes();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TPI uninit");
    mTPIMgr->uninitSession();
    mTPIMgr->destroySession();
    TPIMgr::destroyInstance(mTPIMgr);
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::enque(const FeaturePipeParam &param)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( !param.mP2Pack.isValid() )
    {
        MY_LOGE("Invalid P2Pack, directly assert.");
        return MFALSE;
    }
    this->prepareFeatureRequest(param);
    RequestPtr request;
    CAM_ULOG_SUBREQS(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, param.mP2Pack.mLog.getLogFrameID(), REQ_STR_FPIPE_REQUEST, mCounter);
    CAM_ULOG_ENTER(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, mCounter);
    request = new StreamingFeatureRequest(mPipeUsage, param, mCounter, mRecordCounter, mEISQControl.getCurrentState());
    if(request == NULL)
    {
        MY_LOGE("OOM: Cannot allocate StreamingFeatureRequest");
        CAM_ULOG_DISCARD(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, mCounter);
    }
    else
    {
        request->calSizeInfo();
        request->setDisplayFPSCounter(&mDisplayFPSCounter);
        request->setFrameFPSCounter(&mFrameFPSCounter);
        if( mTPIMgr != NULL &&
            (mPipeUsage.supportTPI(TPIOEntry::YUV) ||
             mPipeUsage.supportTPI(TPIOEntry::ASYNC) ) &&
            request->hasGeneralOutput() )
        {
            mTPIMgr->genFrame(request->mTPIFrame, request->getAppMeta());
        }
        if( mPipeUsage.supportTPI(TPIOEntry::YUV) &&
            request->mTPIFrame.needEntry(TPIOEntry::YUV) )
        {
            ENABLE_TPI_YUV(request->mFeatureMask);
        }
        if( mPipeUsage.supportTPI(TPIOEntry::ASYNC) &&
            request->hasDisplayOutput() &&
            request->mTPIFrame.needEntry(TPIOEntry::ASYNC) &&
            mAsync.queryFrameEnable() )
        {
            ENABLE_TPI_ASYNC(request->mFeatureMask);
        }
        if( mUsePerFrameSetting )
        {
            this->prepareDebugSetting();
        }
        this->applyMaskOverride(request);
        this->applyVarMapOverride(request);
        mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
        prepareIORequest(request);
        ret = CamPipe::enque(ID_ROOT_ENQUE, request);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID StreamingFeaturePipe::notifyFlush()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL StreamingFeaturePipe::flush()
{
    TRACE_FUNC_ENTER();
    mNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    if( mPipeUsage.supportEIS_Q() )
    {
        MY_LOGD("Notify EIS: flush begin");
        mEIS.triggerDryRun();
    }
    CamPipe::sync();
    if (mPipeUsage.supportDPE())
    {
        MY_LOGD("Notify DepthNodeL: Flush");
        mDepth.onFlush();
    }
    mEISQControl.reset();
    mWarp.clearTSQ();
    mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
    if( mPipeUsage.supportEIS_Q() )
    {
        MY_LOGD("Notify EIS: flush end");
        mEIS.triggerDryRun();
    }
    if( mPipeUsage.supportSMP2() )
    {
        mP2SM.flush();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::setJpegParam(NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd, int arg1, int arg2)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mDIPStream != NULL )
    {
        ret = mDIPStream->setJpegParam(cmd, arg1, arg2);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::setFps(MINT32 fps)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mDIPStream != NULL )
    {
        ret = mDIPStream->setFps(fps);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 StreamingFeaturePipe::getRegTableSize()
{
    TRACE_FUNC_ENTER();
    MUINT32 ret = 0;
    if( mDIPStream != NULL )
    {
        ret = mDIPStream->getRegTableSize();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::sendCommand(NSCam::NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mDIPStream != NULL )
    {
        ret = mDIPStream->sendCommand(cmd, arg1, arg2, arg3);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::addMultiSensorID(MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    if( sensorID < P2CamContext::SENSOR_INDEX_MAX )
    {
        android::Mutex::Autolock lock(mContextMutex);
        if( !mContextCreated[sensorID] )
        {
            P2CamContext::createInstance(sensorID, mPipeUsage);
            mContextCreated[sensorID] = MTRUE;
            ret = MTRUE;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID StreamingFeaturePipe::sync()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Sync start");
    CamPipe::sync();
    MY_LOGD("Sync finish");
    TRACE_FUNC_EXIT();
}

MBOOL StreamingFeaturePipe::onInit()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    MY_LOGI("+");
    MBOOL ret;
    ret = mEarlyInited &&
          this->prepareDebugSetting() &&
          this->prepareNodeSetting() &&
          this->prepareNodeConnection() &&
          this->prepareIOControl() &&
          this->prepareBuffer() &&
          this->prepareCamContext();

    MY_LOGI("-");
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID StreamingFeaturePipe::onUninit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("+");
    this->releaseCamContext();
    this->releaseBuffer();
    this->releaseNodeSetting();
    MY_LOGI("-");
    TRACE_FUNC_EXIT();
}

MBOOL StreamingFeaturePipe::onData(DataID, const RequestPtr &)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::earlyInit()
{
    return this->prepareGeneralPipe();
}

MVOID StreamingFeaturePipe::lateUninit()
{
    this->releaseGeneralPipe();
}

MBOOL StreamingFeaturePipe::initNodes()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    mNodes.push_back(&mRootNode);

    if( mPipeUsage.supportDepthP2() )
    {
        mNodes.push_back(&mDepth);
        if( mPipeUsage.supportBokeh() )     mNodes.push_back(&mBokeh);
    }
    else if( mPipeUsage.supportSMP2() )
    {
        mNodes.push_back(&mP2SM);
    }
    else
    {
        mNodes.push_back(&mP2A);
        mNodes.push_back(&mP2AMDP);
    }

    if( mPipeUsage.supportDSDN20() )
    {
        mNodes.push_back(&mP2NR);
        mNodes.push_back(&mVNR);
    }

    if( mPipeUsage.supportWarpNode() )  mNodes.push_back(&mWarp);
    if( mPipeUsage.supportRSCNode() )   mNodes.push_back(&mRSC);
    if( mPipeUsage.supportEISNode() )   mNodes.push_back(&mEIS);
    if( mPipeUsage.supportTPI(TPIOEntry::YUV) )
    {
        MUINT32 count = mPipeUsage.getTPINodeCount(TPIOEntry::YUV);
        mTPIs.clear();
        mTPIs.reserve(count);
        char name[32];
        for( MUINT32 i = 0; i < count; ++i )
        {
            snprintf(name, sizeof(name), "fpipe.tpi.%d", i);
            TPINode *tpi = new TPINode(name, i);
            tpi->setTPIMgr(mTPIMgr);
            mTPIs.push_back(tpi);
            mNodes.push_back(tpi);
        }
    }
    if( mPipeUsage.supportTPI(TPIOEntry::DISP) )
    {
        mDisp.setTPIMgr(mTPIMgr);
        mNodes.push_back(&mDisp);
    }
    if( mPipeUsage.supportTPI(TPIOEntry::ASYNC) )
    {
        mAsync.setTPIMgr(mTPIMgr);
        mNodes.push_back(&mAsync);
    }
    if( mPipeUsage.supportTPI(TPIOEntry::META) )
    {
        mHelper.setTPIMgr(mTPIMgr);
    }

    if( mPipeUsage.supportVMDPNode() )
    {
        mNodes.push_back(&mVMDP);
    }

    mNodes.push_back(&mHelper);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::uninitNodes()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    for( unsigned i = 0, n = mTPIs.size(); i < n; ++i )
    {
        delete mTPIs[i];
        mTPIs[i] = NULL;
    }
    mTPIs.clear();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareDebugSetting()
{
    TRACE_FUNC_ENTER();

    mForceOnMask = 0;
    mForceOffMask = ~0;

    #define CHECK_DEBUG_MASK(name)                                          \
    {                                                                       \
        MINT32 prop = getPropertyValue(KEY_FORCE_##name, VAL_FORCE_##name); \
        if( prop == FORCE_ON )    ENABLE_##name(mForceOnMask);              \
        if( prop == FORCE_OFF )   DISABLE_##name(mForceOffMask);            \
    }
    CHECK_DEBUG_MASK(EIS);
    CHECK_DEBUG_MASK(EIS_30);
    CHECK_DEBUG_MASK(EIS_QUEUE);
    CHECK_DEBUG_MASK(3DNR);
    CHECK_DEBUG_MASK(VHDR);
    CHECK_DEBUG_MASK(DSDN20);
    CHECK_DEBUG_MASK(TPI_YUV);
    CHECK_DEBUG_MASK(TPI_ASYNC);
    #undef CHECK_DEBUG_SETTING

    mDebugDump = getPropertyValue(KEY_DEBUG_DUMP, VAL_DEBUG_DUMP);
    mDebugDumpCount = getPropertyValue(KEY_DEBUG_DUMP_COUNT, VAL_DEBUG_DUMP_COUNT);
    mDebugDumpByRecordNo = getPropertyValue(KEY_DEBUG_DUMP_BY_RECORDNO, VAL_DEBUG_DUMP_BY_RECORDNO);
    mForceIMG3O = getPropertyValue(KEY_FORCE_IMG3O, VAL_FORCE_IMG3O);
    mForceWarpPass = getPropertyValue(KEY_FORCE_WARP_PASS, VAL_FORCE_WARP_PASS);
    mForceGpuOut = getPropertyValue(KEY_FORCE_GPU_OUT, VAL_FORCE_GPU_OUT);
    mForceGpuRGBA = getPropertyValue(KEY_FORCE_GPU_RGBA, VAL_FORCE_GPU_RGBA);
    mUsePerFrameSetting = getPropertyValue(KEY_USE_PER_FRAME_SETTING, VAL_USE_PER_FRAME_SETTING);
    mForcePrintIO = getPropertyValue(KEY_FORCE_PRINT_IO, VAL_FORCE_PRINT_IO);

    if( !mPipeUsage.support3DNR() )
    {
        DISABLE_3DNR(mForceOffMask);
    }
    if( !mPipeUsage.supportDSDN20() )
    {
        DISABLE_DSDN20(mForceOffMask);
    }
    if( !mPipeUsage.supportEISNode() )
    {
        DISABLE_EIS(mForceOffMask);
    }
    if( !mPipeUsage.supportEISNode() || !mPipeUsage.supportEIS_Q() )
    {
        DISABLE_EIS_QUEUE(mForceOffMask);
    }
    if( !mPipeUsage.supportTPI(TPIOEntry::YUV) )
    {
        DISABLE_TPI_YUV(mForceOffMask);
    }
    if( !mPipeUsage.supportTPI(TPIOEntry::ASYNC) )
    {
        DISABLE_TPI_ASYNC(mForceOffMask);
    }

    MY_LOGD("forceOnMask=0x%04x, forceOffMask=0x%04x", mForceOnMask, ~mForceOffMask);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareGeneralPipe()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    MBOOL needNormalStream = mPipeUsage.supportP2AP2() || mPipeUsage.supportSMP2();

    MY_LOGI("create & init DIPStream ++");
    if( needNormalStream )
    {
        mDIPStream = Feature::P2Util::DIPStream::createInstance(mSensorIndex);
        if( mDIPStream != NULL )
        {
            MBOOL secFlag = mPipeUsage.isSecureP2();
            ret = mDIPStream->init(NORMAL_STREAM_NAME, NSIoPipe::EStreamPipeID_Normal, secFlag);
        }
        else
        {
            ret = MFALSE;
        }
    }
    MY_LOGI("create & init DIPStream --");

    if( mPipeUsage.supportP2AP2() )
    {
        mP2A.setDIPStream(mDIPStream);
    }
    else if( mPipeUsage.supportSMP2() )
    {
        mP2SM.setNormalStream(mDIPStream);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::prepareNodeSetting()
{
    TRACE_FUNC_ENTER();
    NODE_LIST::iterator it, end;
    for( it = mNodes.begin(), end = mNodes.end(); it != end; ++it )
    {
        (*it)->setSensorIndex(mSensorIndex);
        (*it)->setPipeUsage(mPipeUsage);
        (*it)->setNodeSignal(mNodeSignal);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareNodeConnection()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    StreamingFeatureNode *dispHelper = &mHelper;
    if( mPipeUsage.supportTPI(TPIOEntry::DISP) )
    {
        dispHelper = &mDisp;
        this->connectData(ID_DISP_TO_HELPER, mDisp, mHelper);
    }

    if( mPipeUsage.supportDepthP2() )
    {
        this->connectData(ID_ROOT_TO_DEPTH, mRootNode, mDepth);
        if( mPipeUsage.supportBokeh() )
        {
            this->connectData(ID_DEPTH_TO_BOKEH, mDepth, mBokeh);
            this->connectData(ID_BOKEH_TO_HELPER, mBokeh, *dispHelper);
        }
    }
    else if( mPipeUsage.supportSMP2() )
    {
        this->connectData(ID_ROOT_TO_P2SM, mRootNode, mP2SM);
        this->connectData(ID_P2SM_TO_HELPER, mP2SM, mHelper);
    }
    else
    {
        this->connectData(ID_ROOT_TO_P2A, mRootNode, mP2A);
        this->connectData(ID_P2A_TO_HELPER, mP2A, *dispHelper);
        this->connectData(ID_P2A_TO_PMDP, mP2A, mP2AMDP);
        this->connectData(ID_PMDP_TO_HELPER, mP2AMDP, *dispHelper);
    }

    if( mPipeUsage.supportDSDN20() )
    {
        this->connectData(ID_P2A_TO_P2NR, mP2A, mP2NR);
        this->connectData(ID_P2NR_TO_VNR, mP2NR, mVNR, CONNECTION_SEQUENTIAL);
        if( mPipeUsage.supportTPI(TPIOEntry::YUV) && mTPIs.size() )
        {
            this->connectData(ID_VNR_TO_NEXT_FULLIMG, mVNR, mTPIs[0]);
        }
        else if( mPipeUsage.supportWarpNode() && !mPipeUsage.supportVMDPNode() )
        {
            this->connectData(ID_VNR_TO_NEXT_FULLIMG, mVNR, mWarp);
        }
        else
        {
            this->connectData(ID_VNR_TO_NEXT_FULLIMG, mVNR, mVMDP);
        }
    }

    if( mPipeUsage.supportWarpNode() )
    {
        this->connectData(ID_P2A_TO_WARP_FULLIMG, mP2A, mWarp);
        this->connectData(ID_WARP_TO_HELPER, mWarp, *dispHelper);
    }

    // EIS nodes
    if (mPipeUsage.supportEISNode())
    {
        this->connectData(ID_ROOT_TO_EIS, mRootNode, mEIS);
        this->connectData(ID_EIS_TO_WARP, mEIS, mWarp, CONNECTION_SEQUENTIAL);
        if( mPipeUsage.supportRSCNode() )
        {
            this->connectData(ID_RSC_TO_EIS, mRSC, mEIS, CONNECTION_SEQUENTIAL);
        }
    }

    if( mPipeUsage.supportRSCNode() )
    {
        this->connectData(ID_ROOT_TO_RSC, mRootNode, mRSC);
        this->connectData(ID_RSC_TO_HELPER, mRSC, mHelper, CONNECTION_SEQUENTIAL);
        if( mPipeUsage.support3DNRRSC() && mPipeUsage.supportP2AP2() )
        {
            this->connectData(ID_RSC_TO_P2A, mRSC, mP2A, CONNECTION_SEQUENTIAL);
        }
    }

    if( mPipeUsage.supportTPI(TPIOEntry::YUV) && mTPIs.size() )
    {
        StreamingFeatureNode *tpi = mTPIs[0];
        if( mPipeUsage.supportDepthP2() )
        {
            if( mPipeUsage.supportBokeh() )
            {
                this->connectData(ID_BOKEH_TO_VENDOR_FULLIMG, mBokeh, *tpi);
            }
            else
            {
                this->connectData(ID_DEPTH_TO_VENDOR, mDepth, *tpi);
            }
        }
        else
        {
            this->connectData(ID_P2A_TO_VENDOR_FULLIMG, mP2A, *tpi);
        }

        unsigned n = mTPIs.size();
        for( unsigned i = 1; i < n; ++i )
        {
            this->connectData(ID_VENDOR_TO_NEXT, *mTPIs[i-1], *mTPIs[i]);
        }
        this->connectData(ID_VENDOR_TO_NEXT, *mTPIs[n-1], mVMDP);

    }

    if( mPipeUsage.supportVMDPNode() )
    {
        if( mPipeUsage.supportWarpNode() )
        {
            this->connectData(ID_VMDP_TO_NEXT_FULLIMG, mVMDP, mWarp);
        }
        this->connectData(ID_VMDP_TO_HELPER, mVMDP, *dispHelper);
    }

    if( mPipeUsage.supportTPI(TPIOEntry::ASYNC) )
    {
        this->connectData(ID_HELPER_TO_ASYNC, mHelper, mAsync);
        this->connectData(ID_ASYNC_TO_HELPER, mAsync, mHelper);
    }

    this->setRootNode(&mRootNode);
    mRootNode.registerInputDataID(ID_ROOT_ENQUE);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareIOControl()
{
    TRACE_FUNC_ENTER();

    StreamingFeatureNode *rootN = &mP2A;
    if( mPipeUsage.supportDepthP2() )
    {
        rootN = &mDepth;
    }
    else if( mPipeUsage.supportSMP2() )
    {
        rootN = &mP2SM;
    }

    mRecordPath.push_back(rootN);
    mDisplayPath.push_back(rootN);
    mPhysicalPath.push_back(rootN);
    mPreviewCallbackPath.push_back(rootN);

    if( mPipeUsage.supportBokeh() )
    {
        mRecordPath.push_back(&mBokeh);
        mDisplayPath.push_back(&mBokeh);
        mPreviewCallbackPath.push_back(&mBokeh);
    }
    if( mPipeUsage.supportDSDN20() )
    {
        mRecordPath.push_back(&mVNR);
        mDisplayPath.push_back(&mVNR);
        mPreviewCallbackPath.push_back(&mVNR);
    }
    if( mPipeUsage.supportTPI(TPIOEntry::YUV) && mTPIs.size() )
    {
        mRecordPath.push_back(mTPIs[0]);
        mDisplayPath.push_back(mTPIs[0]);
        mPreviewCallbackPath.push_back(mTPIs[0]);
    }
    if( mPipeUsage.supportVMDPNode() )
    {
        mRecordPath.push_back(&mVMDP);
        mDisplayPath.push_back(&mVMDP);
        mPreviewCallbackPath.push_back(&mVMDP);
    }
    if( mPipeUsage.supportWarpNode() )
    {
        mRecordPath.push_back(&mWarp);
        if (mPipeUsage.supportPreviewEIS())
        {
            mDisplayPath.push_back(&mWarp);
            mPreviewCallbackPath.push_back(&mWarp);
        }
    }
    if( mPipeUsage.supportTPI(TPIOEntry::ASYNC) )
    {
        mAsyncPath.push_back(&mAsync);
    }

    mIOControl.setRoot(rootN);
    mIOControl.addStream(STREAMTYPE_PREVIEW, mDisplayPath);
    mIOControl.addStream(STREAMTYPE_RECORD, mRecordPath);
    mIOControl.addStream(STREAMTYPE_PREVIEW_CALLBACK, mPreviewCallbackPath);
    mIOControl.addStream(STREAMTYPE_PHYSICAL, mPhysicalPath);
    mIOControl.addStream(STREAMTYPE_ASYNC, mAsyncPath);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareBuffer()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    MSize fullSize(MAX_FULL_WIDTH, MAX_FULL_HEIGHT);
    MSize streamingSize = mPipeUsage.getStreamingSize();

    if( streamingSize.w > 0 && streamingSize.h > 0 )
    {
        fullSize.w = align(streamingSize.w, BUF_ALLOC_ALIGNMENT_BIT);
        fullSize.h = align(streamingSize.h, BUF_ALLOC_ALIGNMENT_BIT);
    }

    MY_LOGD("sensor(%d) StreamingSize=(%dx%d) align64=(%dx%d)", mSensorIndex, streamingSize.w, streamingSize.h, fullSize.w, fullSize.h);

    if( mPipeUsage.supportP2AFeature() || mPipeUsage.supportDepthP2() )
    {
        if( mPipeUsage.supportDepthP2() )
        {
            mDepthYuvOutPool = createFullImgPool("fpipe.depthOutImg", fullSize);
            if( mPipeUsage.supportBokeh() )
            {
                mBokehOutPool = createFullImgPool("fpipe.bokehOutImg", fullSize);
            }
        }
        else
        {
            mFullImgPool = createFullImgPool("fpipe.fullImg", fullSize);
        }
    }

    if( mPipeUsage.supportWarpNode() )
    {
        MUINT32 eis_factor = mPipeUsage.getEISFactor();

        MUINT32 modifyW = fullSize.w;
        MUINT32 modifyH = fullSize.h;

        if( mPipeUsage.supportWarpCrop() )
        {
            modifyW = fullSize.w*100.0f/eis_factor;
            modifyH = fullSize.h*100.0f/eis_factor;
        }

        modifyW = align(modifyW, BUF_ALLOC_ALIGNMENT_BIT);
        modifyH = align(modifyH, BUF_ALLOC_ALIGNMENT_BIT);

        mWarpOutputPool = createWarpOutputPool("fpipe.warpOut", MSize(modifyW, modifyH));
        mEisFullImgPool = createFullImgPool("fpipe.eisFull", fullSize);
    }
    if( mPipeUsage.supportDepthP2() )
    {
        mDepth.setOutputBufferPool(mDepthYuvOutPool, mPipeUsage.getNumDepthImgBuffer());
        if(mPipeUsage.supportBokeh())
        {
            mBokeh.setOutputBufferPool(mBokehOutPool, mPipeUsage.getNumBokehOutBuffer());
        }
    }
    else
    {
        mP2A.setFullImgPool(mFullImgPool, mPipeUsage.getNumP2ABuffer());
    }

    if( mPipeUsage.supportWarpNode() )
    {
        mWarp.setInputBufferPool(mEisFullImgPool);
        mWarp.setOutputBufferPool(mWarpOutputPool);
    }

    if( mPipeUsage.supportTPI(TPIOEntry::YUV) )
    {
        TPIUsage tpiUsage = mPipeUsage.getTPIUsage();
        EImageFormat fmt = tpiUsage.getCustomFormat(TPIOEntry::YUV, mPipeUsage.getFullImgFormat());
        MSize size = tpiUsage.getCustomSize(TPIOEntry::YUV, mPipeUsage.getStreamingSize());
        size = align(size, BUF_ALLOC_ALIGNMENT_BIT);
        mTPIBufferPool = GraphicBufferPool::create("fpipe.tpi", size, fmt, GraphicBufferPool::USAGE_HW_TEXTURE);
        mTPISharedBufferPool = SharedBufferPool::create("fpipe.tpi.shared", mTPIBufferPool);
        for( TPINode *tpi : mTPIs )
        {
            if( tpi )
            {
                tpi->setSharedBufferPool(mTPISharedBufferPool);
            }
        }
    }

    if( mPipeUsage.supportVNRNode() )
    {
        EImageFormat format = mPipeUsage.getVNRImgFormat();
        MSize size = mPipeUsage.getStreamingSize();

        if( mPipeUsage.supportTPI(TPIOEntry::YUV) && mTPIBufferPool != NULL )
        {
            format = mTPIBufferPool->getImageFormat();
            size = mTPIBufferPool->getImageSize();
        }
        else if( mPipeUsage.supportWarpNode() && mEisFullImgPool != NULL )
        {
            format = mEisFullImgPool->getImageFormat();
            size = mEisFullImgPool->getImageSize();
        }
        size = align(size, BUF_ALLOC_ALIGNMENT_BIT);

        mVNRInputPool = GraphicBufferPool::create("fpipe.vnr.in", size, format, ImageBufferPool::USAGE_HW);
        mVNROutputPool = GraphicBufferPool::create("fpipe.vnr.out", size, format, ImageBufferPool::USAGE_HW);

        mVNR.setInputBufferPool(mVNRInputPool);
        mVNR.setOutputBufferPool(mVNROutputPool);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

android::sp<IBufferPool> StreamingFeaturePipe::createFullImgPool(const char* name, MSize size)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> fullImgPool;
    EImageFormat format = mPipeUsage.getFullImgFormat();

    if (mPipeUsage.isSecureP2())
    {
        fullImgPool =  SecureBufferPool::create(name, size, format, ImageBufferPool::USAGE_HW);
    }
    else if( mPipeUsage.supportGraphicBuffer() )
    {
        NativeBufferWrapper::ColorSpace color;
        color = mPipeUsage.supportEISNode() ?
                NativeBufferWrapper::YUV_BT601_FULL :
                NativeBufferWrapper::NOT_SET;

        fullImgPool = GraphicBufferPool::create(name, size, format, GraphicBufferPool::USAGE_HW_TEXTURE, color);
    }
    else
    {
        fullImgPool = ImageBufferPool::create(name, size, format, ImageBufferPool::USAGE_HW);
    }
    MY_LOGD("sensor(%d) size=(%dx%d) format(%d) %s", mSensorIndex , size.w, size.h, format,
            mPipeUsage.isSecureP2() ? "SecureBuffer" :
                (mPipeUsage.supportGraphicBuffer() ? "GraphicBuffer" : "ImageBuffer") );

    TRACE_FUNC_EXIT();

    return fullImgPool;
}

android::sp<IBufferPool> StreamingFeaturePipe::createImgPool(const char* name, MSize size, EImageFormat fmt)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> pool;
    if( mPipeUsage.supportGraphicBuffer() )
    {
        pool = GraphicBufferPool::create(name, size, fmt, GraphicBufferPool::USAGE_HW_TEXTURE);
    }
    else
    {
        pool = ImageBufferPool::create(name, size, fmt, ImageBufferPool::USAGE_HW );
    }

    TRACE_FUNC_EXIT();
    return pool;
}

android::sp<IBufferPool> StreamingFeaturePipe::createWarpOutputPool(const char* name, MSize size)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> warpOutputPool;

    EImageFormat format = mPipeUsage.getFullImgFormat();

    if( mPipeUsage.supportGraphicBuffer() )
    {
        EImageFormat gbFmt = (!mPipeUsage.supportWPE() && mForceGpuRGBA ) ? eImgFmt_RGBA8888 : format;
        warpOutputPool = GraphicBufferPool::create(name, size, gbFmt, GraphicBufferPool::USAGE_HW_RENDER);
    }
    else
    {
        warpOutputPool = ImageBufferPool::create(name, size, format, ImageBufferPool::USAGE_HW);
    }

    MY_LOGD("sensor(%d) %s size=(%dx%d) format(%d) %s", mSensorIndex, mPipeUsage.supportWPE() ? "WPE" : "GPU" ,
            size.w, size.h, format, mPipeUsage.supportGraphicBuffer() ? "GraphicBuffer" : "ImageBuffer");

    TRACE_FUNC_EXIT();

    return warpOutputPool;
}

MVOID StreamingFeaturePipe::releaseNodeSetting()
{
    TRACE_FUNC_ENTER();
    this->disconnect();
    mDisplayPath.clear();
    mRecordPath.clear();
    mPhysicalPath.clear();
    mPreviewCallbackPath.clear();
    mAsyncPath.clear();
    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::releaseGeneralPipe()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC_ENTER();
    mP2A.setDIPStream(NULL);
    mP2SM.setNormalStream(NULL);
    if( mDIPStream )
    {
        mDIPStream->uninit(NORMAL_STREAM_NAME);
        mDIPStream->destroyInstance();
        mDIPStream = NULL;
    }
    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::releaseBuffer()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    mP2A.setFullImgPool(NULL);
    mWarp.setInputBufferPool(NULL);
    mWarp.setOutputBufferPool(NULL);

    IBufferPool::destroy(mFullImgPool);
    IBufferPool::destroy(mDepthYuvOutPool);
    IBufferPool::destroy(mBokehOutPool);
    IBufferPool::destroy(mEisFullImgPool);
    IBufferPool::destroy(mWarpOutputPool);
    SharedBufferPool::destroy(mTPISharedBufferPool);
    IBufferPool::destroy(mTPIBufferPool);
    IBufferPool::destroy(mVNRInputPool);
    IBufferPool::destroy(mVNROutputPool);

    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::applyMaskOverride(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    request->mFeatureMask |= mForceOnMask;
    request->mFeatureMask &= mForceOffMask;
    request->setDumpProp(mDebugDump, mDebugDumpCount, mDebugDumpByRecordNo);
    request->setForceIMG3O(mForceIMG3O);
    request->setForceWarpPass(mForceWarpPass);
    request->setForceGpuOut(mForceGpuOut);
    request->setForceGpuRGBA(mForceGpuRGBA);
    request->setForcePrintIO(mForcePrintIO);
    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::applyVarMapOverride(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    (void)(request);
    TRACE_FUNC_EXIT();
}

MBOOL StreamingFeaturePipe::prepareCamContext()
{
    TRACE_FUNC_ENTER();
    for(auto&& id : mAllSensorIDs)
    {
        addMultiSensorID(id);
    }

    TRACE_FUNC_EXIT();

    return MTRUE;
}

MVOID StreamingFeaturePipe::prepareFeatureRequest(const FeaturePipeParam &param)
{
    ++mCounter;
    eAppMode appMode = param.getVar<eAppMode>(SFP_VAR::APP_MODE, APP_PHOTO_PREVIEW);
    if( appMode == APP_VIDEO_RECORD ||
        appMode == APP_VIDEO_STOP )
    {
        ++mRecordCounter;
    }
    else if( mRecordCounter )
    {
        MY_LOGI("Set Record Counter %d=>0. AppMode=%d", mRecordCounter, appMode);
        mRecordCounter = 0;
    }
    this->prepareEISQControl(param);
    TRACE_FUNC("Request=%d, Record=%d, AppMode=%d", mCounter, mRecordCounter, appMode);
}

MVOID StreamingFeaturePipe::prepareEISQControl(const FeaturePipeParam &param)
{
    EISQActionInfo info;
    info.mAppMode = param.getVar<eAppMode>(SFP_VAR::APP_MODE, APP_PHOTO_PREVIEW);
    info.mRecordCount = mRecordCounter;
    info.mIsAppEIS = HAS_EIS(param.mFeatureMask);
    info.mIsReady = existOutBuffer(param.mSFPIOManager.getGeneralIOs(), IO_TYPE_RECORD);
    mEISQControl.update(info);

    TRACE_FUNC("AppMode=%d, Record=%d, AppEIS=%d, IsRecordBuffer=%d",
               info.mAppMode, info.mRecordCount, info.mIsAppEIS, info.mIsReady);
}

MVOID StreamingFeaturePipe::prepareIORequest(const RequestPtr &request)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    {
        LogString dumpStr(2048);
        const SrcCropInfo masterInfo = request->getSrcCropInfo(request->mMasterID);
        if(masterInfo.mIsSrcCrop)
        {
            dumpStr.append("sID(%d), srcCrop(" MRect_STR "), imgo(%dx%d), rrz(%dx%d) ", request->mMasterID, MRect_ARG(masterInfo.mSrcCrop), masterInfo.mIMGOSize.w, masterInfo.mIMGOSize.h, masterInfo.mRRZOSize.w, masterInfo.mRRZOSize.h);
        }
        else
        {
            dumpStr.append("sID(%d), imgo(%dx%d), rrz(%dx%d) ", request->mMasterID, masterInfo.mIMGOSize.w, masterInfo.mIMGOSize.h, masterInfo.mRRZOSize.w, masterInfo.mRRZOSize.h);
        }
        if(request->mSlaveID != INVALID_SENSOR)
        {
            const SrcCropInfo slaveInfo = request->getSrcCropInfo(request->mSlaveID);
            if(slaveInfo.mIsSrcCrop)
            {
                dumpStr.append("sID(%d), srcCrop(" MRect_STR "), imgo(%dx%d), rrz(%dx%d) ", request->mSlaveID, MRect_ARG(slaveInfo.mSrcCrop), slaveInfo.mIMGOSize.w, slaveInfo.mIMGOSize.h, slaveInfo.mRRZOSize.w, slaveInfo.mRRZOSize.h);
            }
            else
            {
                dumpStr.append("sID(%d), imgo(%dx%d), rrz(%dx%d) ", request->mSlaveID, slaveInfo.mIMGOSize.w, slaveInfo.mIMGOSize.h, slaveInfo.mRRZOSize.w, slaveInfo.mRRZOSize.h);
            }
        }
        dumpStr.append("%s master/slave(%d/%d) ReqNo(%d), feature=0x%04x(%s), cycle(%d), ZoomROI(" MCropF_STR ") SFPIOMgr:",
                request->mLog.getLogStr(), request->getMasterID(), request->mSlaveID, request->mRequestNo,
                request->mFeatureMask, request->getFeatureMaskName(), request->getNodeCycleTimeMs(), MCropF_ARG(request->getZoomROI()));
        request->mSFPIOManager.appendDumpInfo(dumpStr);
        MY_SAFE_LOGD(dumpStr.c_str(), dumpStr.length());
    }

    std::set<StreamType> generalStreams;
    if( request->hasDisplayOutput() )
    {
        generalStreams.insert(STREAMTYPE_PREVIEW);
    }
    if( request->hasRecordOutput() )
    {
        generalStreams.insert(STREAMTYPE_RECORD);
    }
    if( request->hasExtraOutput() )
    {
        generalStreams.insert(STREAMTYPE_PREVIEW_CALLBACK);
    }
    if( request->needTPIAsync() )
    {
        generalStreams.insert(STREAMTYPE_ASYNC);
    }

    request->mTimer.startPrepareIO();
    {// Master
        prepareIORequest(request, generalStreams, request->mMasterID);
    }
    if(request->hasSlave(request->mSlaveID))
    {
        prepareIORequest(request, generalStreams, request->mSlaveID);
    }
    request->mTimer.stopPrepareIO();

    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::prepareIORequest(const RequestPtr &request, std::set<StreamType> &generalStreams, MUINT32 sensorID)
{
    std::set<StreamType> streams = generalStreams;
    if( request->hasPhysicalOutput(sensorID) )
    {
        streams.insert(STREAMTYPE_PHYSICAL);
    }
    MSize rrzoSize = request->getMasterInputSize();
    StreamingReqInfo reqInfo(request->mRequestNo, request->mFeatureMask, request->mMasterID, sensorID, rrzoSize);
    IORequest<StreamingFeatureNode, StreamingReqInfo> &ioReq = request->mIORequestMap[sensorID];
    mIOControl.prepareMap(streams, reqInfo, ioReq);

    if( request->needPrintIO() )
    {
        MY_LOGD("IOUtil ReqInfo : %s", reqInfo.dump());
        mIOControl.dump(ioReq);
    }
}

MVOID StreamingFeaturePipe::releaseCamContext()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mContextMutex);
    for( int i = 0; i < P2CamContext::SENSOR_INDEX_MAX; i++ )
    {
        if( mContextCreated[i] )
        {
            P2CamContext::destroyInstance(i);
            mContextCreated[i] = MFALSE;
        }
    }

    TRACE_FUNC_EXIT();
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam
