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

#define NORMAL_STREAM_NAME "StreamingFeature"

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
    , mWarp("fpipe.warp")
    , mMDP("fpipe.mdp")
#if SUPPORT_VFB
    , mVFB("fpipe.vfb")
    , mFD("fpipe.fd")
    , mP2B("fpipe.p2b")
#endif // SUUPPORT_VFB
#if SUPPORT_FOV
    , mFOV("fpipe.fov")
    , mFOVWarp("fpipe.fovwarp")
#endif
    , mEIS("fpipe.eis")
    , mRSC("fpipe.rsc")
#if MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT
    , mN3D_P2Node("fpipe.n3dp2")
    , mN3D("fpipe.n3d")
#endif
    , mHelper("fpipe.helper")
    , mVendor("fpipe.vendor")
    , mVendorMDP("fpipe.vmdp")
    , mVendorFOV("fpipe.vfov")
    , mNormalStream(NULL)
{
    TRACE_FUNC_ENTER();

    MY_LOGI("create pipe(%p): SensorIndex=%d UsageMode=%d VendorNode=%d EisMode=0x%x 3DNRMode=%d fscMode=0x%x tsq=%d StreamingSize=%dx%d usageHint.VendorNodeSize=%dx%d pipeUsage.VendorCusSize=%d usageHint.secType=%d" , this, mSensorIndex,
        mPipeUsage.getMode(), mPipeUsage.getVendorMode(), mPipeUsage.getEISMode(), mPipeUsage.get3DNRMode(), usageHint.mFSCMode, usageHint.mUseTSQ, usageHint.mStreamingSize.w, usageHint.mStreamingSize.h,
        usageHint.mVendorCusSize.w, usageHint.mVendorCusSize.h, mPipeUsage.supportVendorCusSize(), usageHint.mSecType);

    mNodes.push_back(&mRootNode);
    mNodes.push_back(&mP2A);

    if( mPipeUsage.supportWarpNode() )
    {
        mNodes.push_back(&mWarp);
    }

    if( mPipeUsage.supportMDPNode() )
    {
        mNodes.push_back(&mMDP);
    }

    if( mPipeUsage.supportRSCNode() )
    {
        mNodes.push_back(&mRSC);
    }

    if( mPipeUsage.supportEISNode() )
    {
        mNodes.push_back(&mEIS);
    }

    if( mPipeUsage.supportVFB() )
    {
        #if SUPPORT_VFB
        mNodes.push_back(&mVFB);
        mNodes.push_back(&mFD);
        mNodes.push_back(&mP2B);
        #endif // SUPPORT_VFB
    }

    if( mPipeUsage.supportVendor() )
    {
        mNodes.push_back(&mVendor);
        mNodes.push_back(&mVendorMDP);
    }

    // dual zoom mode
    if( mPipeUsage.supportDual())
    {
#if SUPPORT_FOV
        mNodes.push_back(&mFOV);
        mNodes.push_back(&mFOVWarp);
#endif
    }

    if( mPipeUsage.supportN3D())
    {
        #if MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT
        mNodes.push_back(&mN3D_P2Node);
        mNodes.push_back(&mN3D);
        #endif
    }

    if( mPipeUsage.supportVendorFOV())
    {
        mNodes.push_back(&mVendorFOV);
    }
    mNodes.push_back(&mHelper);

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
    MY_LOGE_IF(!mEarlyInited, "NormalStream init failed!");

    TRACE_FUNC_EXIT();
}

StreamingFeaturePipe::~StreamingFeaturePipe()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("destroy pipe(%p): SensorIndex=%d", this, mSensorIndex);
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
    MBOOL ret;
    ret = PARENT_PIPE::init();
    mCounter = 0;
    mEISQControl.init(mPipeUsage);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::uninit(const char *name)
{
    TRACE_FUNC_ENTER();
    (void)name;
    MBOOL ret;
    ret = PARENT_PIPE::uninit();
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::enque(const FeaturePipeParam &param)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    this->prepareFeatureRequest(param);
    RequestPtr request;
    request = new StreamingFeatureRequest(mPipeUsage, param, mCounter, mRecordCounter, mEISQControl.getCurrentState());
    if(request == NULL)
    {
        MY_LOGE("OOM: Cannot allocate StreamingFeatureRequest");
    }
    else
    {
        request->setDisplayFPSCounter(&mDisplayFPSCounter);
        request->setFrameFPSCounter(&mFrameFPSCounter);
        if( mUsePerFrameSetting )
        {
            this->prepareDebugSetting();
        }
        this->applyMaskOverride(request);
        this->applyVarMapOverride(request);
        mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
        ret = CamPipe::enque(ID_ROOT_ENQUE, request);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::flush()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Trigger flush");
    mNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    if( mPipeUsage.supportEIS_Q() )
    {
        MY_LOGD("Notify EIS: flush begin");
        mEIS.triggerDryRun();
    }
    CamPipe::sync();
    mEISQControl.reset();
    mWarp.clearTSQ();
    mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
    if( mPipeUsage.supportEIS_Q() )
    {
        MY_LOGD("Notify EIS: flush end");
        mEIS.triggerDryRun();
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::setJpegParam(NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd, int arg1, int arg2)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mNormalStream != NULL )
    {
        ret = mNormalStream->setJpegParam(cmd, arg1, arg2);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::setFps(MINT32 fps)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mNormalStream != NULL )
    {
        ret = mNormalStream->setFps(fps);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 StreamingFeaturePipe::getRegTableSize()
{
    TRACE_FUNC_ENTER();
    MUINT32 ret = 0;
    if( mNormalStream != NULL )
    {
        ret = mNormalStream->getRegTableSize();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::sendCommand(NSCam::NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mNormalStream != NULL )
    {
        ret = mNormalStream->sendCommand(cmd, arg1, arg2, arg3);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::addMultiSensorId(MUINT32 sensorId)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    if( sensorId < P2CamContext::SENSOR_INDEX_MAX )
    {
        android::Mutex::Autolock lock(mContextMutex);
        if( !mContextCreated[sensorId] )
        {
            P2CamContext::createInstance(sensorId, mPipeUsage);
            mContextCreated[sensorId] = MTRUE;
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

IImageBuffer* StreamingFeaturePipe::requestBuffer()
{
    TRACE_FUNC_ENTER();
    IImageBuffer *buffer = NULL;
    buffer = mInputBufferStore.requestBuffer();
    TRACE_FUNC_EXIT();
    return buffer;
}

MBOOL StreamingFeaturePipe::returnBuffer(IImageBuffer *buffer)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = mInputBufferStore.returnBuffer(buffer);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeaturePipe::onInit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("+");
    MBOOL ret;
    ret = mEarlyInited &&
          this->prepareDebugSetting() &&
          this->prepareNodeSetting() &&
          this->prepareNodeConnection() &&
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
    CHECK_DEBUG_MASK(EIS_25);
    CHECK_DEBUG_MASK(EIS_30);
    CHECK_DEBUG_MASK(EIS_QUEUE);
    CHECK_DEBUG_MASK(VFB);
    CHECK_DEBUG_MASK(VFB_EX);
    CHECK_DEBUG_MASK(3DNR);
    CHECK_DEBUG_MASK(VHDR);
    CHECK_DEBUG_MASK(VENDOR);
    CHECK_DEBUG_MASK(VENDOR_FOV);
    #undef CHECK_DEBUG_SETTING

    #if !SUPPORT_VFB
    DISABLE_VFB(mForceOffMask);
    DISABLE_VFB_EX(mForceOffMask);
    #endif // SUPPORT_VFB

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
    if( !mPipeUsage.supportVFB() )
    {
        DISABLE_VFB(mForceOffMask);
        DISABLE_VFB_EX(mForceOffMask);
    }
    if( !mPipeUsage.supportEISNode() )
    {
        DISABLE_EIS(mForceOffMask);
    }
    if( !mPipeUsage.supportEIS_30() )
    {
        DISABLE_EIS_30(mForceOffMask);
    }
    if( !mPipeUsage.supportEIS_25() )
    {
        DISABLE_EIS_25(mForceOffMask);
    }
    if( !mPipeUsage.supportEISNode() || !mPipeUsage.supportEIS_Q() )
    {
        DISABLE_EIS_QUEUE(mForceOffMask);
    }
    if( !mPipeUsage.supportVendor() )
    {
        DISABLE_VENDOR(mForceOffMask);
    }
    if( !mPipeUsage.supportVendorFOV() )
    {
        DISABLE_VENDOR_FOV(mForceOffMask);
    }

    MY_LOGD("forceOnMask=0x%04x, forceOffMask=0x%04x", mForceOnMask, ~mForceOffMask);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareGeneralPipe()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( !mPipeUsage.supportBypassP2A() )
    {
        mNormalStream = INormalStream::createInstance(mSensorIndex);
        if( mNormalStream != NULL )
        {
            MBOOL secFlag = mPipeUsage.getSecureFlag();
            ret = mNormalStream->init(NORMAL_STREAM_NAME, secFlag);
        }
        else
        {
            ret = MFALSE;
        }
        mP2A.setNormalStream(mNormalStream);
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
    TRACE_FUNC_ENTER();

    // TODO consider root to DepthMap or P2A, currently always P2A
    this->connectData(ID_ROOT_TO_P2A, mRootNode, mP2A);

    this->connectData(ID_P2A_TO_HELPER, mP2A, mHelper);

    if( mPipeUsage.supportWarpNode() )
    {
        this->connectData(ID_P2A_TO_WARP_FULLIMG, mP2A, mWarp);
        this->connectData(ID_WARP_TO_HELPER, mWarp, mHelper, CONNECTION_SEQUENTIAL);
    }

    #if SUPPORT_VFB
    if( mPipeUsage.supportVFB() )
    {
        // VFB nodes
        this->connectData(ID_P2A_TO_FD_DSIMG, mP2A, mFD);
        this->connectData(ID_P2A_TO_VFB_DSIMG, mP2A, mVFB);
        this->connectData(ID_P2A_TO_P2B_FULLIMG, mP2A, mP2B);
        this->connectData(ID_FD_TO_VFB_FACE, mFD, mVFB);
        this->connectData(ID_VFB_TO_P2B, mVFB, mP2B);
        this->connectData(ID_VFB_TO_WARP, mVFB, mWarp);
        this->connectData(ID_MDP_TO_P2B_FULLIMG, mMDP, mP2B);
    }
    #endif // SUPPORT_VFB

    // EIS nodes
    if( mPipeUsage.supportEIS_22() )
    {
        this->connectData(ID_P2A_TO_EIS_P2DONE, mP2A, mEIS);
        this->connectData(ID_EIS_TO_WARP, mEIS, mWarp, CONNECTION_SEQUENTIAL);
    }
    else if( mPipeUsage.supportEIS_25() )
    {
        this->connectData(ID_P2A_TO_EIS_P2DONE, mP2A, mEIS);
        this->connectData(ID_EIS_TO_WARP, mEIS, mWarp, CONNECTION_SEQUENTIAL);
    }
    else if( mPipeUsage.supportEIS_30() )
    {
        this->connectData(ID_P2A_TO_EIS_P2DONE, mP2A, mEIS);
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
        if( mPipeUsage.support3DNRRSC() )
            this->connectData(ID_RSC_TO_P2A, mRSC, mP2A, CONNECTION_SEQUENTIAL);
    }

    #if SUPPORT_VFB
    // EIS + VFB
    if( mPipeUsage.supportVFB() && mPipeUsage.supportEIS_22() )
    {
        this->connectData(ID_EIS_TO_VFB_WARP, mEIS, mVFB);
    }
    #endif // SUPPORT_VFB

    if( mPipeUsage.supportVendor() )
    {
        this->connectData(ID_P2A_TO_VENDOR_FULLIMG, mP2A, mVendor);
        this->connectData(ID_VENDOR_TO_VMDP_FULLIMG, mVendor, mVendorMDP);
        if( mPipeUsage.supportWarpNode() )
        {
            this->connectData(ID_VMDP_TO_NEXT_FULLIMG, mVendorMDP, mWarp);
        }
        this->connectData(ID_VMDP_TO_HELPER, mVendorMDP, mHelper);
    }

    if(mPipeUsage.supportFOV())
    {
#if SUPPORT_FOV
        this->connectData(ID_P2A_TO_FOV_FEFM, mP2A, mFOV);
        this->connectData(ID_P2A_TO_FOV_FULLIMG, mP2A, mFOV);
        this->connectData(ID_P2A_TO_FOV_WARP, mP2A, mFOVWarp);
        //
        this->connectData(ID_FOV_TO_FOV_WARP, mFOV, mFOVWarp);
        this->connectData(ID_FOV_WARP_TO_HELPER, mFOVWarp, mHelper);
        if (mPipeUsage.supportVendor() )
        {
            this->connectData(ID_FOV_WARP_TO_VENDOR, mFOVWarp, mVendor);
        }
        if( mPipeUsage.supportEIS_30() )
        {
            this->connectData(ID_FOV_TO_EIS_WARP, mFOV, mEIS);
        }
        if( mPipeUsage.supportWarpNode() )
        {
            this->connectData(ID_FOV_TO_EIS_FULLIMG, mFOVWarp, mWarp);
        }
#endif
    }
    #if MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT
    if( mPipeUsage.supportN3D())
    {
        this->connectData(ID_P2A_TO_N3DP2, mP2A, mN3D_P2Node);
        this->connectData(ID_N3DP2_TO_N3D, mN3D_P2Node, mN3D);
        this->connectData(ID_N3D_TO_HELPER, mN3D, mHelper);
        if(mPipeUsage.supportVendor())
        {
            this->connectData(ID_N3D_TO_VMDP, mN3D, mVendorMDP);
        }
    }
    #endif
    if (mPipeUsage.supportVendorFOV() )
    {
        this->connectData(ID_P2A_TO_VFOV_FULLIMG, mP2A, mVendorFOV);
        if (mPipeUsage.supportVendor() )
        {
            this->connectData(ID_VFOV_TO_VENDOR, mVendorFOV, mVendor);
        }
        else
        {
            this->connectData(ID_VFOV_TO_VMDP_FULLIMG, mVendorFOV, mVendorMDP);
            this->connectData(ID_VMDP_TO_HELPER, mVendorMDP, mHelper);
        }
    }

    this->setRootNode(&mRootNode);
    mRootNode.registerInputDataID(ID_ROOT_ENQUE);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingFeaturePipe::prepareBuffer()
{
    TRACE_FUNC_ENTER();

    MSize fullSize(MAX_FULL_WIDTH, MAX_FULL_HEIGHT);
    MSize streamingSize = mPipeUsage.getStreamingSize();

    if( streamingSize.w > 0 && streamingSize.h > 0 )
    {
        // align 64
        fullSize.w = align(streamingSize.w, 6);
        fullSize.h = align(streamingSize.h, 6);
    }

    MY_LOGD("sensor(%d) StreamingSize=(%dx%d) align64=(%dx%d)", mSensorIndex, streamingSize.w, streamingSize.h, fullSize.w, fullSize.h);

    if( mPipeUsage.supportP2AFeature() || mPipeUsage.supportVendor() || mPipeUsage.supportYUVIn() )
    {
        mFullImgPool = createFullImgPool("fpipe.fullImg", fullSize);

        if( mPipeUsage.supportVendor() )
        {
            if( mPipeUsage.supportVendorFullImg() )
            {
                mVendorFullImgPool = createVendorImgPool("fpipe.vendorFullImg", mPipeUsage.supportVendorCusSize() ? mPipeUsage.getVendorCusSize() : fullSize);
            }
            if( !mPipeUsage.supportVendorInplace() )
            {
                mVendorImgPool = createFullImgPool("fpipe.vendorImg", fullSize);
            }
        }
    }

    if( mPipeUsage.supportEISFullImg() )
    {
        // TODO: reduce buffer size
        mEisFullImgPool = createFullImgPool("fpipe.eisFull", fullSize);
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

        // align 32
        modifyW = align(modifyW, 5);
        modifyH = align(modifyH, 5);

        mWarpOutputPool = createWarpOutputPool("fpipe.warpOut", MSize(modifyW, modifyH));

        #if SUPPORT_VFB
        if( mPipeUsage.supportVFB() )
        {
            mMDPOutputPool = ImageBufferPool::create("fpipe.mdp", fullSize.w, fullSize.h, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
            mDsImgPool = ImageBufferPool::create("fpipe.dsImg", DS_IMAGE_WIDTH, DS_IMAGE_HEIGHT, eImgFmt_YUY2, ImageBufferPool::USAGE_HW );
        }
        #endif // SUPPORT_VFB
    }
    if (mPipeUsage.supportVendorFOV())
    {
        // TODO: select NV21 or YV12
        if( mPipeUsage.supportGraphicBuffer() )
        {
            mVendorFovFullImgPool = GraphicBufferPool::create("fpipe.vfovIn", fullSize.w, fullSize.h, HAL_PIXEL_FORMAT_YCrCb_420_SP, GraphicBufferPool::USAGE_HW_TEXTURE);
        }
        else
        {
            mVendorFovFullImgPool = ImageBufferPool::create("fpipe.vfovIn", fullSize.w, fullSize.h, eImgFmt_NV21, ImageBufferPool::USAGE_HW );
        }
        // output color format should be as same as original pipeline, such as yv12
        mVendorFovOutImgPool = createFullImgPool("fpipe.vfovOut", fullSize);
    }

    {
        mP2A.setFullImgPool(mFullImgPool, mPipeUsage.getNumP2ABuffer());
    }
    if( mPipeUsage.supportVFB() )
    {
        mP2A.setDsImgPool(mDsImgPool);
    }
    if( mPipeUsage.supportWarpNode() )
    {
        if( mPipeUsage.supportEISFullImg() )
        {
            mP2A.setEisFullImgPool(mEisFullImgPool);
            mWarp.setInputBufferPool(mEisFullImgPool);
        }
        else
        {
            mWarp.setInputBufferPool(mFullImgPool);
        }

        mWarp.setOutputBufferPool(mWarpOutputPool);
    }
    if( mPipeUsage.supportMDPNode() )
    {
        mMDP.setMDPOutputPool(mMDPOutputPool);
    }
    if( mPipeUsage.supportVendor() )
    {
        if( mPipeUsage.supportVendorFullImg() )
        {
            mP2A.setVendorFullImgPool(mVendorFullImgPool);
            mVendor.setInImgPool(mVendorFullImgPool, mPipeUsage.getNumVendorOutBuffer());
        }
        if( !mPipeUsage.supportVendorInplace() )
        {
            mVendor.setOutImgPool(mVendorImgPool, mPipeUsage.getNumVendorOutBuffer());
        }
    }
#if SUPPORT_FOV
    if (mPipeUsage.supportDual() && mPipeUsage.supportVendor())
    {
        mFOVWarpOutputPool = createFullImgPool("fpipe.fovWarpOut", fullSize);
        mFOVWarp.setOutputBufferPool(mFOVWarpOutputPool);
    }
#endif
    if(mPipeUsage.supportVendorFOV())
    {
        mP2A.setVFOVFullImgPool(mVendorFovFullImgPool);
        mVendorFOV.setInputBufferPool(mVendorFovFullImgPool, mPipeUsage.getNumVendorFOVBuffer());
        mVendorFOV.setOutputBufferPool(mVendorFovOutImgPool, mPipeUsage.getNumVendorFOVBuffer()/2); // 2 in 1 out
    }

    mInputBufferStore.init(mFullImgPool);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

android::sp<IBufferPool> StreamingFeaturePipe::createFullImgPool(const char* name, MSize size)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> fullImgPool;

    if( mPipeUsage.supportGraphicBuffer() )
    {
        NativeBufferWrapper::ColorSpace color = mPipeUsage.supportEISNode() ?
                                                NativeBufferWrapper::YUV_BT601_FULL : NativeBufferWrapper::NOT_SET;
        android_pixel_format_t format = mPipeUsage.supportFull_YUY2() ? HAL_PIXEL_FORMAT_YCbCr_422_I : HAL_PIXEL_FORMAT_YV12;
        fullImgPool = GraphicBufferPool::create(name, size.w, size.h, format, GraphicBufferPool::USAGE_HW_TEXTURE, color);
    }
    else
    {
        NSCam::EImageFormat format = mPipeUsage.supportFull_YUY2() ? eImgFmt_YUY2 : eImgFmt_YV12;
        fullImgPool = ImageBufferPool::create(name, size.w, size.h, format, ImageBufferPool::USAGE_HW );
    }

    TRACE_FUNC_EXIT();

    return fullImgPool;
}

android::sp<IBufferPool> StreamingFeaturePipe::createVendorImgPool(const char* name, MSize size)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> vendorImgPool;

    if( mPipeUsage.supportGraphicBuffer() )
    {
        android_pixel_format_t format = HAL_PIXEL_FORMAT_YV12;
        if( mPipeUsage.supportVendorCusFormat() )
        {
            toPixelFormat(SUPPORT_VENDOR_FULL_FORMAT, format);
        }
        else if( mPipeUsage.supportFull_YUY2() )
        {
            format = HAL_PIXEL_FORMAT_YCbCr_422_I;
        }

        vendorImgPool = GraphicBufferPool::create(name, size.w, size.h, format, GraphicBufferPool::USAGE_HW_TEXTURE);
    }
    else
    {
        NSCam::EImageFormat format = eImgFmt_YV12;
        if( mPipeUsage.supportVendorCusFormat() )
        {
            format = SUPPORT_VENDOR_FULL_FORMAT;
        }
        else if( mPipeUsage.supportFull_YUY2() )
        {
            format = eImgFmt_YUY2;
        }

        vendorImgPool = ImageBufferPool::create(name, size.w, size.h, format, ImageBufferPool::USAGE_HW );
    }

    TRACE_FUNC_EXIT();

    return vendorImgPool;
}

android::sp<IBufferPool> StreamingFeaturePipe::createWarpOutputPool(const char* name, MSize size)
{
    TRACE_FUNC_ENTER();

    android::sp<IBufferPool> warpOutputPool;

    if( mPipeUsage.supportGraphicBuffer() )
    {
        android_pixel_format_t warpOutFmt = HAL_PIXEL_FORMAT_YV12;

        if( mPipeUsage.supportWPE() )
        {
            warpOutFmt = HAL_PIXEL_FORMAT_YCbCr_422_I;
        }
        else // GPU
        {
            warpOutFmt = mForceGpuRGBA ? HAL_PIXEL_FORMAT_RGBA_8888 : HAL_PIXEL_FORMAT_YV12;
        }

        MY_LOGD("sensor(%d) %s outsize=(%dx%d) format(%d) GraphicBuffer", mSensorIndex, mPipeUsage.supportWPE() ? "WPE" : "GPU", size.w, size.h, warpOutFmt);

        warpOutputPool = GraphicBufferPool::create(name, size.w, size.h, warpOutFmt, GraphicBufferPool::USAGE_HW_RENDER);
    }
    else
    {
        MY_LOGD("sensor(%d) WPE outsize=(%dx%d) format(%d) ImageBuffer", mSensorIndex, size.w, size.h, eImgFmt_YUY2);

        warpOutputPool = ImageBufferPool::create(name, size.w, size.h, eImgFmt_YUY2, ImageBufferPool::USAGE_HW);
    }

    TRACE_FUNC_EXIT();

    return warpOutputPool;
}

MVOID StreamingFeaturePipe::releaseNodeSetting()
{
    TRACE_FUNC_ENTER();
    this->disconnect();
    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::releaseGeneralPipe()
{
    TRACE_FUNC_ENTER();
    mP2A.setNormalStream(NULL);
    if( mNormalStream )
    {
        mNormalStream->uninit(NORMAL_STREAM_NAME);
        mNormalStream->destroyInstance();
        mNormalStream = NULL;
    }
    TRACE_FUNC_EXIT();
}

MVOID StreamingFeaturePipe::releaseBuffer()
{
    TRACE_FUNC_ENTER();

    mP2A.setDsImgPool(NULL);
    mP2A.setFullImgPool(NULL);
    mP2A.setEisFullImgPool(NULL);
    mWarp.setInputBufferPool(NULL);
    mWarp.setOutputBufferPool(NULL);
    mVendor.setOutImgPool(NULL);
#if SUPPORT_FOV
    mFOVWarp.setOutputBufferPool(NULL);
#endif
    mVendorFOV.setOutputBufferPool(NULL);
    mVendorFOV.setInputBufferPool(NULL);

    mInputBufferStore.uninit();

    IBufferPool::destroy(mFullImgPool);
    IBufferPool::destroy(mVendorImgPool);
    IBufferPool::destroy(mVendorFullImgPool);
    IBufferPool::destroy(mVendorFovFullImgPool);
    IBufferPool::destroy(mVendorFovOutImgPool);
    IBufferPool::destroy(mEisFullImgPool);
    IBufferPool::destroy(mWarpOutputPool);
    IBufferPool::destroy(mFOVWarpOutputPool);
    ImageBufferPool::destroy(mMDPOutputPool);
    ImageBufferPool::destroy(mDsImgPool);

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

    addMultiSensorId(mSensorIndex);

    TRACE_FUNC_EXIT();

    return MTRUE;
}

MVOID StreamingFeaturePipe::prepareFeatureRequest(const FeaturePipeParam &param)
{
    ++mCounter;
    eAppMode appMode = param.getVar<eAppMode>(VAR_APP_MODE, APP_PHOTO_PREVIEW);
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
    NSCam::NSIoPipe::Output record;
    info.mAppMode = param.getVar<eAppMode>(VAR_APP_MODE, APP_PHOTO_PREVIEW);
    info.mRecordCount = mRecordCounter;
    info.mIsAppEIS = HAS_EIS(param.mFeatureMask);
    info.mIsReady = getOutBuffer(param.getQParams(), IO_TYPE_RECORD, record);
    mEISQControl.update(info);

    TRACE_FUNC("AppMode=%d, Record=%d, AppEIS=%d, IsRecordBuffer=%d",
               info.mAppMode, info.mRecordCount, info.mIsAppEIS, info.mIsReady);
}

MVOID StreamingFeaturePipe::releaseCamContext()
{
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
