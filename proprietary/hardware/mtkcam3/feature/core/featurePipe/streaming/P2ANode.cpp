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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2ANode.h"
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam3/feature/eis/eis_hal.h>
#include <mtkcam3/feature/fsc/fsc_util.h>
#include <mtkcam/aaa/IIspMgr.h>


#define PIPE_CLASS_TAG "P2ANode"
#define PIPE_TRACE TRACE_P2A_NODE
#include <featurePipe/core/include/PipeLog.h>
#include "P2CamContext.h"
#include "TuningHelper.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_P2A);

using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_DEPI;//left
using NSImageio::NSIspio::EPortIndex_DMGI;//right
using NSImageio::NSIspio::EPortIndex_FEO;
using NSImageio::NSIspio::EPortIndex_MFBO;
using NSCam::NSIoPipe::PORT_DCESO;
using NSImageio::NSIspio::EPortIndex_TIMGO;
using NSCam::NSIoPipe::ModuleInfo;
using NSCam::Feature::P2Util::P2SensorData;

using namespace NSCam::NSIoPipe;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum DumpMaskIndex
{
    MASK_MASTER_IMG3O,
    MASK_MASTER_NEXT_FULL,
    MASK_MASTER_IMG2O,
    MASK_SLAVE_IMG3O,
    MASK_SLAVE_NEXT_FULL,
    MASK_SLAVE_IMG2O,
    MASK_TIMGO,
};


const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_MASTER_IMG3O,      "master_img3o" ),
    DumpFilter( MASK_MASTER_NEXT_FULL,  "master_nextfull"),
    DumpFilter( MASK_MASTER_IMG2O,      "master_img2o"),
    DumpFilter( MASK_SLAVE_IMG3O,       "slave_img3o"),
    DumpFilter( MASK_SLAVE_NEXT_FULL,   "slave_nextfull"),
    DumpFilter( MASK_SLAVE_IMG2O,       "slave_img2o"),
    DumpFilter( MASK_TIMGO,             "timgo")
};

static MUINT32 calImgOffset(sp<IImageBuffer> pIMGBuffer, const MRect &tmpRect)
{
    MUINT32 u4PixelToBytes = 0;

    MINT imgFormat = pIMGBuffer->getImgFormat();

    if (imgFormat == eImgFmt_YV12 || imgFormat == eImgFmt_NV21)
    {
        u4PixelToBytes = 1;
    }
    else if (imgFormat == eImgFmt_YUY2)
    {
        u4PixelToBytes = 2;
    }
    else
    {
        MY_LOGW("unsupported image format %d", imgFormat);
    }

    return tmpRect.p.y * pIMGBuffer->getBufStridesInBytes(0) + tmpRect.p.x * u4PixelToBytes; //in byte
}

static MRectF applyViewRatio(const MRectF &src, const MSize &size)
{
    MRectF view = src;
    if( src.s.w * size.h > size.w * src.s.h )
    {
        view.s.w = (src.s.h * size.w / size.h);
        view.p.x += (src.s.w - view.s.w)/2;
    }
    else
    {
        view.s.h = (src.s.w * size.h / size.w);
        view.p.y += (src.s.h - view.s.h)/2;
    }
    return view;
}

P2ANode::P2ANode(const char *name)
    : CamNodeULogHandler(Utils::ULog::MOD_STREAMING_P2A)
    , StreamingFeatureNode(name)
    , mp3A(NULL)
    , mDIPStream(NULL)
    , mFullImgPoolAllocateNeed(0)
    , mEisMode(0)
    , mLastDualParamValid(MFALSE)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);

    m3dnrLogLevel = getPropertyValue("vendor.debug.camera.3dnr.log.level", 0);
    mForceExpectMS = getPropertyValue("vendor.debug.fpipe.p2a.expect", 0);

    TRACE_FUNC_EXIT();
}

P2ANode::~P2ANode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setDIPStream(Feature::P2Util::DIPStream *stream)
{
    TRACE_FUNC_ENTER();
    mDIPStream = stream;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setFullImgPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mFullImgPool = pool;
    mFullImgPoolAllocateNeed = allocate;
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::onInit()
{
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2A:onInit");
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    enableDumpMask(0xffff, sFilterTable);

    createDCESOs();
    createTuningBufs();

    mTimgoHal.init(mPipeUsage.getStreamingSize());
    if( mTimgoHal.isSupport() )
    {
        mTimgoPool = ImageBufferPool::create("fpipe.timgo", mTimgoHal.getBufferSize(), mTimgoHal.getBufferFmt(), ImageBufferPool::USAGE_HW);
    }

    if( mPipeUsage.support3DNRRSC() )
    {
        MY_LOGD("P2A add RSC waitQueue for 3DNR");
        this->addWaitQueue(&mRSCDatas);
    }

    if( mPipeUsage.supportDSDN20() )
    {
        mDSDNCfg = mPipeUsage.getDSDNCfg();
        MSize size = mDSDNCfg.getMaxSize(DSDNCfg::DS1);
        EImageFormat fmt = mDSDNCfg.getFormat(DSDNCfg::DS1);
        mDS1ImgPool = ImageBufferPool::create("fpipe.p2a.ds1", size, fmt, ImageBufferPool::USAGE_HW);
    }

    if(mPipeUsage.isSecureP2())
    {
        mSecBufCtrl.init(mPipeUsage.getSecureType());
    }

    TRACE_FUNC_EXIT();
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return MTRUE;
}

MBOOL P2ANode::onUninit()
{
    TRACE_FUNC_ENTER();
    destroyDCESOs();
    mTimgoHal.uninit();
    destroyTuningBufs();
    ImageBufferPool::destroy(mTimgoPool);

    IBufferPool::destroy(mDS1ImgPool);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onThreadStart()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC_ENTER();

    if( mTimgoHal.isSupport() && mTimgoPool != NULL )
    {
        Timer timer(MTRUE);
        mTimgoPool->allocate(1*3);
        timer.stop();
        MY_LOGD("mTimgo %s buf in %d ms", STR_ALLOCATE, timer.getElapsed());
    }

    if( mFullImgPoolAllocateNeed && mFullImgPool != NULL )
    {
        Timer timer(MTRUE);
        mFullImgPool->allocate(mFullImgPoolAllocateNeed);
        timer.stop();
        MY_LOGD("mFullImg %s %d buf in %d ms", STR_ALLOCATE, mFullImgPoolAllocateNeed, timer.getElapsed());
    }

    if( mDynamicTuningPool != NULL )
    {
        Timer timer(MTRUE);
        mDynamicTuningPool->allocate(mPipeUsage.getNumP2ATuning());
        timer.stop();
        MY_LOGD("Dynamic Tuning %s %d bufs in %d ms", STR_ALLOCATE, mPipeUsage.getNumP2ATuning(), timer.getElapsed());
    }

    if( mSyncTuningPool != NULL )
    {
        Timer timer(MTRUE);
        MUINT32 numSyncTuning = mPipeUsage.getNumP2ASyncTuning();
        mSyncTuningPool->allocate(numSyncTuning);
        timer.stop();
        MY_LOGD("Sync Tuning %s %d bufs in %d ms, size(%d)", STR_ALLOCATE, numSyncTuning, timer.getElapsed(), mSyncTuningPool->getBufSize());
    }

    if( mDCESOImgPool != NULL)
    {
        Timer timer(MTRUE);
        mDCESOImgPool->allocate(mPipeUsage.getNumDCESOBuffer());
        timer.stop();
        MY_LOGD("DCESO Buffer %s %d bufs in %d ms, (%u) in 1 queue",
                STR_ALLOCATE, mPipeUsage.getNumDCESOBuffer(), timer.getElapsed(), DCESO_DELAY_COUNT);

        timer.start();
        initDCEQueue(mDCEQueueMap);
        initDCEQueue(mPhyDCEQueueMap);
        initDCEQueue(mLargeDCEQueueMap);
        timer.stop();
        MY_LOGD("DCESO Queue prepare done in %d ms, queueMapSize Gen(%zu) Phy(%zu) Large(%zu)",
                    timer.getElapsed(), mDCEQueueMap.size(), mPhyDCEQueueMap.size(), mLargeDCEQueueMap.size());
    }

    if( mDS1ImgPool != NULL )
    {
        allocate("fpipe.p2a.ds1", mDS1ImgPool, 6);
    }

    init3A();
    initP2();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitDIPStreamBaseDone();
    uninitP2();
    uninit3A();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    switch( id )
    {
    case ID_ROOT_TO_P2A:
        mRequests.enque(data);
        ret = MTRUE;
        break;
    default:
        ret = MFALSE;
        break;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::onData(DataID id, const RSCData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s  rsc_data arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    if( mPipeUsage.support3DNRRSC() )
    {
        if( id == ID_RSC_TO_P2A )
        {
            mRSCDatas.enque(data);
            ret = MTRUE;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType P2ANode::getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const
{
    (void)stream;
    TRACE_FUNC("frame:%d stream=%d(%s)", reqInfo.mFrameNo, stream, toName(stream));
    IOPolicyType policy = IOPOLICY_INOUT;
    if( HAS_3DNR(reqInfo.mFeatureMask) && reqInfo.isMaster())
    {
        policy = IOPOLICY_LOOPBACK;
    }

    return policy;
}

MBOOL P2ANode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    RSCData rscData;

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    else if( request == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    if( mPipeUsage.support3DNRRSC() )
    {
        if( !mRSCDatas.deque(rscData) )
        {
            MY_LOGE("RSCData deque out of sync");
            return MFALSE;
        }
        if( request != rscData.mRequest )
        {
            MY_LOGE("P2A_RSCData out of sync request(%d) rsc(%d)", request->mRequestNo, rscData.mRequest->mRequestNo);
            return MFALSE;
        }
    }

    TRACE_FUNC_ENTER();

    request->mTimer.startP2A();
    processP2A(request, rscData);
    request->mTimer.stopP2A();// When NormalStream callback, stopP2A will be called again to record this frame duration

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL isSameTuning(SFPIOMap& io1, SFPIOMap& io2, MUINT32 sensorID)
{
    return (io1.getTuning(sensorID).mFlag == io2.getTuning(sensorID).mFlag);
}

MBOOL P2ANode::processP2A(const RequestPtr &request, const RSCData &rscData)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);

    TRACE_FUNC_ENTER();
    P2AEnqueData data;
    P2ATuningIndex tuningIndex;
    data.mRequest = request;

    if( mPipeUsage.supportBypassP2A() )
    {
        MY_LOGW("Bypass P2ANode not confirm supported now!!");
        prepareFullImgFromInput(request, data);
        handleResultData(request, data);
        return MTRUE;
    }

    SFPIOManager &ioMgr = request->mSFPIOManager;
    if(ioMgr.countAll() == 0)
    {
        MY_LOGW("No output frame exist in P2ANode, directly let SFP return.");
        handleData(ID_P2A_TO_HELPER, HelperData(FeaturePipeParam::MSG_FRAME_DONE, request));
        return MFALSE;
    }

    Feature::P2Util::DIPParams param;
    // --- Start prepare General DIPParam
    request->mTimer.startP2ATuning();

    VarMap<SFP_VAR> &varMap = request->getSensorVarMap(request->getMasterID());
    MBOOL enable3dnr = varMap.get<MBOOL>(SFP_VAR::NR3D_P2A_CAN_ENABLE_ON_FRAME, MFALSE);
    if( request->need3DNR())
    {
        //prepare 3DNR35 before setp2isp
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr: on @ p2a: %d", enable3dnr);
        if( enable3dnr == MTRUE)
        {
            if ( prepare3DNR(param, request, request->getMasterID(), rscData) != MTRUE)
            {
                // set mPrevFullImg to NULL to disable VIPI port
                getP2CamContext(request->getMasterID())->setPrevFullImg(NULL);
            }
        }
        else
        {
            if (request->needDSDN20())
            {
                if ( !prepare3DNRMvInfo(param, request, request->getMasterID(), rscData) )
                {
                    MY_LOGW("!!warn: 3dnr: prepareDSDNMvInfo fail");
                }
            }
            // set mPrevFullImg to NULL to disable VIPI port
            getP2CamContext(request->getMasterID())->setPrevFullImg(NULL);
        }
    }

    prepareRawTuning(param, request, data, tuningIndex);
    request->mTimer.stopP2ATuning();

    if( request->need3DNR() && enable3dnr)
    {
        if (tuningIndex.isGenMasterValid() )
        {
            MINT32 masterID = request->getMasterID();
            const SrcCropInfo srcCropInfo = request->getSrcCropInfo(masterID);

            getP2CamContext(masterID)->get3dnr()->configNR3D_legacy(
                param.mvDIPFrameParams[tuningIndex.mGenMaster].mTuningData, mp3A,
                srcCropInfo.mSrcCrop,
                mNr3dHalResult.nr3dHwParam);
        }
    }

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "PrepareOutput");
    if(ioMgr.countNonLarge() != 0)
    {
        MBOOL nextFullByIMG3OMaster = needNextFullByIMG3O(request, request->mMasterID);
        MBOOL nextFullByIMG3OSlave  = needNextFullByIMG3O(request, request->mSlaveID);

        prepareNonMDPIO(param, request, data, tuningIndex, nextFullByIMG3OMaster);
        prepareMasterMDPOuts(param, request, data, tuningIndex, nextFullByIMG3OMaster);
        prepareSlaveOuts(param, request, data, tuningIndex, nextFullByIMG3OSlave);
    }

    if(ioMgr.countLarge() != 0)
    {
        if(tuningIndex.isLargeMasterValid())
        {
            prepareLargeMDPOuts(param, request, (MUINT32)tuningIndex.mLargeMaster, request->mMasterID);
        }
        if(tuningIndex.isLargeSlaveValid())
        {
            prepareLargeMDPOuts(param, request, (MUINT32)tuningIndex.mLargeSlave, request->mSlaveID);
        }
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "PrepareSecure");
    for( auto &frameParam : param.mvDIPFrameParams )
    {
        if( !TuningHelper::processSecure(frameParam, mSecBufCtrl) )
        {
            MY_LOGE("Process Secure Flow Failed! SecureEnum(%d)", mSecBufCtrl.getSecureEnum());
            return MFALSE;
        }
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    MUINT32 expectMs = (mForceExpectMS > 0) ? mForceExpectMS : request->getNodeCycleTimeMs();
    NSCam::Feature::P2Util::updateExpectEndTime(param, expectMs);

    // ---- Prepare DIPParam Done ----
    if( request->needPrintIO() )
    {
        NSCam::Feature::P2Util::printDIPParams(request->mLog, param);
    }

    enqueFeatureStream(param, data, tuningIndex);


    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const P2AEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing P2ANode class members
    TRACE_FUNC_ENTER();
    for(auto&& frame : params.mvDIPFrameParams)
    {
        for( size_t i = 0; i < frame.mvExtraParam.size(); i++ )
        {
            MUINT cmdIdx = frame.mvExtraParam[i].CmdIdx;

            if( cmdIdx == EPIPE_IMG3O_CRSPINFO_CMD )
            {
                CrspInfo* extraParam = static_cast<CrspInfo*>(frame.mvExtraParam[i].moduleStruct);
                if( extraParam )
                {
                    delete extraParam;
                }
            }
        }
    }

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopEnqueP2A();
        MY_S_LOGD(data.mRequest->mLog, "sensor(%d) Frame %d enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueP2A(), params.mDequeSuccess);

        if( !params.mDequeSuccess )
        {
            MY_LOGW("Frame %d enque result failed", request->mRequestNo);
        }

        request->updateResult(params.mDequeSuccess);
        handleRunData(request, data);
        handleResultData(request, data);
        request->mTimer.stopP2A();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::handleRunData(const RequestPtr &request, const P2AEnqueData &data)
{
    for(const sp<P2ARunData> &runData : data.mRunDatas)
    {
        returnDCESO(request, runData);
    }
}

MVOID P2ANode::handleResultData(const RequestPtr &request, const P2AEnqueData &data)
{
    // This function is not thread safe,
    // because it is called by onDIPParamsCB,
    // avoid accessing P2ANode class members
    TRACE_FUNC_ENTER();
    BasicImg full, slave;
    full = data.mNextFullImg.isValid() ? data.mNextFullImg : data.mFullImg;
    slave = data.mSlaveNextFullImg.isValid() ? data.mSlaveNextFullImg : data.mSlaveFullImg;

    if( mPipeUsage.supportDSDN20() )
    {
        DSDNImg dsdn(full);
        dsdn.mSlaveImg = slave;
        dsdn.mDS1Img = data.mDS1Img;
        handleData(ID_P2A_TO_P2NR, DSDNData(dsdn, request));
    }
    else if( mPipeUsage.supportTPI(TPIOEntry::YUV) )
    {
        TRACE_FUNC("to vendor");
        handleData(ID_P2A_TO_VENDOR_FULLIMG,
                   DualBasicImgData(DualBasicImg(full, slave), request));
    }
    else if( mPipeUsage.supportWarpNode() )
    {
        handleData(ID_P2A_TO_WARP_FULLIMG, BasicImgData(full, request));
    }
    else
    {
        handleData(ID_P2A_TO_HELPER, HelperData(HelpReq(FeaturePipeParam::MSG_FRAME_DONE), request));
    }

    if( request->needP2AEarlyDisplay() && request->needDisplayOutput(this) )
    {
        handleData(ID_P2A_TO_HELPER, HelperData(HelpReq(FeaturePipeParam::MSG_DISPLAY_DONE), request));
    }

    P2AMDPReq mdpReq;
    if(!data.mRemainingOutputs.empty())
    {
        mdpReq.mMDPIn = data.mFullImg;
        mdpReq.mMDPOuts = std::move(data.mRemainingOutputs);
        mdpReq.mTuningBufs = std::move(data.mTuningBufs);
    }
    handleData(ID_P2A_TO_PMDP, P2AMDPReqData(mdpReq, request));

    if( request->needNddDump() )
    {
        if( data.mFullImg.mBuffer != NULL && allowDump(MASK_MASTER_IMG3O))
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
            data.mFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mFullImg.mBuffer->getImageBufferPtr(), TuningUtils::YUV_PORT_IMG3O);
        }
        if( data.mTimgo != NULL && allowDump(MASK_TIMGO))
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
            data.mTimgo->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mTimgo->getImageBufferPtr(), TuningUtils::YUV_PORT_TIMGO, mTimgoHal.getTypeStr());
        }
        if( data.mDS1Img.mBuffer != NULL && allowDump(MASK_MASTER_IMG2O))
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
            data.mDS1Img.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mDS1Img.mBuffer->getImageBufferPtr(), TuningUtils::YUV_PORT_IMG2O);
        }
        if( data.mNextFullImg.mBuffer != NULL && allowDump(MASK_MASTER_NEXT_FULL))
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
            data.mNextFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mNextFullImg.mBuffer->getImageBufferPtr(), TuningUtils::YUV_PORT_WDMAO, "full");
        }
    }

    if( request->needDump() )
    {
        if( data.mFullImg.mBuffer != NULL && allowDump(MASK_MASTER_IMG3O))
        {
            data.mFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mFullImg.mBuffer->getImageBufferPtr(), "full");
        }
        if( data.mNextFullImg.mBuffer != NULL && allowDump(MASK_MASTER_NEXT_FULL))
        {
            data.mNextFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mNextFullImg.mBuffer->getImageBufferPtr(), "nextfull");
        }
        if( data.mSlaveFullImg.mBuffer != NULL && allowDump(MASK_SLAVE_IMG3O))
        {
            data.mSlaveFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mSlaveFullImg.mBuffer->getImageBufferPtr(), "slaveFull");
        }
        if( data.mSlaveNextFullImg.mBuffer != NULL && allowDump(MASK_SLAVE_NEXT_FULL))
        {
            data.mSlaveNextFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mSlaveNextFullImg.mBuffer->getImageBufferPtr(), "slaveNextfull");
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::initP2()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mDIPStream != NULL )
    {
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2ANode::uninitP2()
{
    TRACE_FUNC_ENTER();

    mDIPStream = NULL;

    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepareFullImgFromInput(const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    IImageBuffer *input = NULL;
    if( (input = request->getMasterInputBuffer()) == NULL )
    {
        MY_LOGE("Cannot get input image buffer");
        ret = MFALSE;
    }
    else if( (data.mFullImg.mBuffer= new IIBuffer_IImageBuffer(input)) == NULL )
    {
        MY_LOGE("OOM: failed to allocate IIBuffer");
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::prepareNonMDPIO(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex,
    MBOOL nextFullByIMG3O)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if(!tuningIndex.isMasterMainValid())
    {
        MY_LOGE("Both master General Normal/Pure & Physical tuning not exist! Can not prepare non mdp out img.");
        return MFALSE;
    }
    MUINT32 masterIndex = tuningIndex.getMasterMainIndex();
    MBOOL isGenNormalRun = (masterIndex == (MUINT32)tuningIndex.mGenMaster);
    Feature::P2Util::DIPFrameParams &frame = params.mvDIPFrameParams.at(masterIndex);

    FrameInInfo inInfo;
    getFrameInInfo(inInfo, frame);
    const MUINT32 masterID = request->mMasterID;

    if( request->needDSDN20() )
    {
        prepareDs1Img(frame, request, inInfo, data);
    }
    else
    {
        prepareFDImg(frame, request, data);
    }

    if (nextFullByIMG3O)
    {
        NextFullInfo nf;
        data.mNextFullImg.mBuffer = request->requestNextFullImg(this, request->mMasterID, nf);
    }
    BasicImg* nextFullImg = nextFullByIMG3O ? &data.mNextFullImg : NULL;
    MBOOL needIMG3O = request->needFullImg(this, masterID) || nextFullByIMG3O;

    if( request->isForceIMG3O() || (needIMG3O && isGenNormalRun))
    {
        prepareFullImg(frame, request, data.mFullImg, inInfo, masterID, nextFullImg);
        // Full Img no need crop
    }

    VarMap<SFP_VAR> &varMap = request->getSensorVarMap(request->getMasterID());
    MBOOL enable3dnr = varMap.get<MBOOL>(SFP_VAR::NR3D_P2A_CAN_ENABLE_ON_FRAME, MFALSE);

    if( enable3dnr &&
        request->need3DNR() && isGenNormalRun &&
        getP2CamContext(request->getMasterID())->getPrevFullImg() != NULL)
    {
        prepareVIPI(frame, request, data);
        //handleVipiNr3dOffset(params, request, data);
    }

    if( mTimgoHal.isSupport() )
    {
        prepareTimgo(frame, request, data, masterID);
    }

    getP2CamContext(request->getMasterID())->setPrevFullImg(
        (enable3dnr && request->need3DNR()) ? data.mFullImg.mBuffer: NULL);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::prepareMasterMDPOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex,
    MBOOL nextFullByIMG3O)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if(!tuningIndex.isMasterMainValid())
    {
        MY_LOGE("Both master General & Physical tuning not exist! Can not prepare output img.");
        return MFALSE;
    }

    MUINT32 sID = request->mMasterID;

    // Handle Main Frame, it maybe normal or pure or physical frame
    MUINT32 masterIndex = tuningIndex.getMasterMainIndex();
    Feature::P2Util::DIPFrameParams &frame = params.mvDIPFrameParams.at(masterIndex);
    FrameInInfo inInfo;
    getFrameInInfo(inInfo, frame);
    MBOOL needExtraPhyRun = (tuningIndex.isPhyMasterValid()) && (tuningIndex.mPhyMaster != MINT32(masterIndex));
    MBOOL isGenNormalRun = (masterIndex == (MUINT32)tuningIndex.mGenMaster);
    P2IO output, dispOut;
    std::vector<P2IO> outList;
    outList.reserve(5);
    // Internal Buffer has highest prority, need to push back first.
    if( request->needNextFullImg(this, sID) && isGenNormalRun && !nextFullByIMG3O)
    {
        P2IO out;
        prepareNextFullOut(out, request, data.mNextFullImg, inInfo, data, sID);
        outList.push_back(out);
    }

    if (request->popDisplayOutput(this, output))
    {
        outList.push_back(output);
        dispOut = output;
    }

    if (request->popRecordOutput(this, output))
    {
        outList.push_back(output);
    }

    std::vector<P2IO> extraList;
    if (request->popExtraOutputs(this, extraList) )
    {
        moveAppend(extraList, outList);
    }

    if (!needExtraPhyRun && request->popPhysicalOutput(this, sID, output))
    {
        outList.push_back(output);
    }

    if(data.mFullImg.mBuffer == NULL && needFullForExtraOut(outList))
    {
        prepareFullImg(frame, request, data.mFullImg, inInfo, sID, NULL);
        if(!mPipeUsage.supportIMG3O())
        {
            MY_LOGD("Need Full img but different crop may not supportted! All output using p2amdp.");
            data.mRemainingOutputs = outList;
            outList.clear();
        }
    }

    if( dispOut.isValid() && request->needTPIAsync() && outList.size() < 2 )
    {
        ImgBuffer asyncImg = request->popAsyncImg(this);
        if( asyncImg != NULL )
        {
            P2IO async;
            data.mAsyncImg = asyncImg;
            async.mBuffer = asyncImg->getImageBufferPtr();
            async.mCropDstSize = asyncImg->getImgSize();
            async.mCropRect = dispOut.mCropRect;
            outList.push_back(async);
        }
    }

    Feature::P2Util::MDPObjPtr mdpObjPtr;
    mdpObjPtr.ispTuningBuf = frame.mTuningData;
    Feature::P2Util::prepareOneMDPFrameParam(frame, outList, data.mRemainingOutputs, mdpObjPtr);
    if(data.mRemainingOutputs.size() > 0)
    {
        prepareExtraMDPCrop(data.mFullImg, data.mRemainingOutputs);
    }

    //--- Additional Run for physical output ---
    if (needExtraPhyRun && request->popPhysicalOutput(this, sID, output))
    {
        std::vector<P2IO> phyOutList;
        phyOutList.push_back(output);
        Feature::P2Util::prepareMDPFrameParam(params, (MUINT32)tuningIndex.mPhyMaster, phyOutList);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::prepareSlaveOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex, MBOOL nextFullByIMG3O)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if(!tuningIndex.isSlaveMainValid())
    {
        TRACE_FUNC("Both slave General Normal/Pure & Physical tuning not exist! Can not prepare output img.");
        return MFALSE;
    }
    const MUINT32 sID = request->mSlaveID;

    // Handle Main Frame, it maybe normal or pure or physical frame
    MUINT32 slaveIndex = tuningIndex.getSlaveMainIndex();
    MBOOL needExtraPhyRun = (tuningIndex.isPhySlaveValid()) && (tuningIndex.mPhySlave != MINT32(slaveIndex));
    MBOOL isGenNormalRun = (slaveIndex == (MUINT32)tuningIndex.mGenSlave);
    Feature::P2Util::DIPFrameParams &frame = params.mvDIPFrameParams.at((MUINT32)slaveIndex);
    FrameInInfo inInfo;
    getFrameInInfo(inInfo, frame);

    std::vector<P2IO> outList;
    outList.reserve(2);

    if ( nextFullByIMG3O )
    {
        NextFullInfo info;
        data.mSlaveNextFullImg.mBuffer = request->requestNextFullImg(this, sID, info);
    }
    BasicImg* nextFullImg = nextFullByIMG3O ? &data.mSlaveNextFullImg : NULL;
    MBOOL needIMG3O = nextFullByIMG3O || request->needFullImg(this, sID);

    if(  needIMG3O && isGenNormalRun )
    {
        prepareFullImg(frame, request, data.mSlaveFullImg, inInfo, sID, nextFullImg);
        // Full Img no need crop
    }

    if( request->needNextFullImg(this, sID) && isGenNormalRun && !nextFullByIMG3O)
    {
        P2IO out;
        prepareNextFullOut(out, request, data.mSlaveNextFullImg, inInfo, data, sID);
        outList.push_back(out);
    }
    P2IO output;
    if (!needExtraPhyRun && request->popPhysicalOutput(this, sID, output))
    {
        outList.push_back(output);
    }

    Feature::P2Util::prepareMDPFrameParam(params, slaveIndex, outList);// no consider more MDP run for slave

    //--- Additional Run for physical output ---
    if (needExtraPhyRun && request->popPhysicalOutput(this, sID, output))
    {
        std::vector<P2IO> phyOutList;
        phyOutList.push_back(output);
        Feature::P2Util::prepareMDPFrameParam(params, (MUINT32)tuningIndex.mPhySlave, outList);
    }
    TRACE_FUNC_EXIT();
    return ret;
}


MBOOL P2ANode::prepareLargeMDPOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, MUINT32 frameIndex, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    std::vector<P2IO> outList;
    if( ! request->popLargeOutputs(this, sensorID, outList) )
    {
        MY_LOGE("Get Large Out List failed! sID(%d), QFrameInd(%d)", sensorID, frameIndex);
    }
    Feature::P2Util::prepareMDPFrameParam(params, frameIndex, outList);// no consider more MDP run for slave
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2ANode::prepareFullImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, BasicImg &outImg, const FrameInInfo &inInfo, MUINT32 sensorID,
    BasicImg *nextFullImg)
{
    TRACE_FUNC_ENTER();
    // TODO Pool maybe not the same in different sensors
    TRACE_FUNC("Frame %d FullImgPool=(%d/%d)", request->mRequestNo, mFullImgPool->peakAvailableSize(), mFullImgPool->peakPoolSize());
    if( nextFullImg != NULL)
    {
        outImg.mBuffer = nextFullImg->mBuffer;
    }
    else
    {
        outImg.mBuffer = mFullImgPool->requestIIBuffer();
    }
    sp<IImageBuffer> pIMGBuffer = outImg.mBuffer->getImageBuffer();
    outImg.mSensorClipInfo = request->getSensorClipInfo(sensorID);

    const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
    const MRect& srcCrop = srcCropInfo.mSrcCrop;
    outImg.accumulate("p2aFull", request->mLog, inInfo.inSize, toMRectF(srcCrop), srcCrop.s);
    pIMGBuffer->setTimestamp(inInfo.timestamp);
    if(!pIMGBuffer->setExtParam(srcCrop.s))
    {
        MY_LOGE("Full Img setExtParm Fail!, target size(%dx%d)", srcCrop.s.w, srcCrop.s.h);
    }

    if( nextFullImg != NULL )
    {
        nextFullImg->setAllInfo(outImg);
    }

    if(mPipeUsage.supportIMG3O())
    {
        Output output;
        output.mPortID = PortID(EPortType_Memory, EPortIndex_IMG3O, PORTID_OUT);
        output.mBuffer = pIMGBuffer.get();
        if (srcCropInfo.mIsSrcCrop)
        {
            output.mOffsetInBytes = calImgOffset(pIMGBuffer, srcCrop); //in byte
            // new driver interface to overwrite mOffsetInBytes
            CrspInfo* crspParam = new CrspInfo();
            crspParam->m_CrspInfo.p_integral.x = srcCrop.p.x;
            crspParam->m_CrspInfo.p_integral.y = srcCrop.p.y;
            crspParam->m_CrspInfo.s.w = srcCrop.s.w;
            crspParam->m_CrspInfo.s.h = srcCrop.s.h;
            ExtraParam extraParam;
            extraParam.CmdIdx = EPIPE_IMG3O_CRSPINFO_CMD;
            extraParam.moduleStruct = static_cast<void*>(crspParam);
            frame.mvExtraParam.push_back(extraParam);
        }
        frame.mvOut.push_back(output);
    }
    else
    {
        P2IO out;
        out.mBuffer = pIMGBuffer.get();
        out.mTransform = 0;
        out.mCropRect = srcCrop;
        out.mCropDstSize = srcCrop.s;
        Feature::P2Util::pushToMDP(frame, NSCam::NSIoPipe::PORT_WDMAO, out);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareVIPI(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    data.mPrevFullImg = getP2CamContext(request->getMasterID())->getPrevFullImg();
    Input input;
    input.mPortID = PortID(EPortType_Memory, EPortIndex_VIPI, PORTID_IN);
    input.mBuffer = data.mPrevFullImg->getImageBufferPtr();
    frame.mvIn.push_back(input);

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareTimgo(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data, MUINT32 /*sensorID*/)
{
    TRACE_FUNC_ENTER();
    (void)request;

    data.mTimgo = mTimgoPool->requestIIBuffer();
    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_TIMGO, PORTID_OUT);
    output.mBuffer = data.mTimgo->getImageBufferPtr();
    MSize imgiSize = MSize(0, 0);
    IImageBuffer *imgiBuf = Feature::P2Util::findInputBuffer(frame, PORT_IMGI);
    imgiSize = imgiBuf->getImgSize();

    if(!output.mBuffer->setExtParam(imgiSize))
    {
        MY_LOGE("Timgo setExtParm Fail!, target size(%dx%d)", imgiSize.w, imgiSize.h);
    }

    frame.mvOut.push_back(output);

    ExtraParam extraParam;
    extraParam.CmdIdx = EPIPE_TIMGO_DUMP_SEL_CMD;
    data.mTimgoType = (MUINT32)mTimgoHal.getType();
    extraParam.moduleStruct = (void*)&data.mTimgoType;
    frame.mvExtraParam.push_back(extraParam);

    TRACE_FUNC_EXIT();
}


MVOID P2ANode::prepareNextFullOut(P2IO &output, const RequestPtr &request, BasicImg &outImg, const FrameInInfo &inInfo, P2AEnqueData &data, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();

    const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
    const MRect& srcCropRect = srcCropInfo.mSrcCrop;
    outImg.mSensorClipInfo = request->getSensorClipInfo(sensorID);
    NextFullInfo nFullInfo;

    TRACE_FUNC(" Streaming %dx%d srcCropRect (%d,%d)(%dx%d), isSrcCrop %d",
        mPipeUsage.getStreamingSize().w, mPipeUsage.getStreamingSize().h,
        srcCropRect.p.x, srcCropRect.p.y, srcCropRect.s.w, srcCropRect.s.h, srcCropInfo.mIsSrcCrop);

    outImg.mBuffer = request->requestNextFullImg(this, sensorID, nFullInfo);
    const MSize &resize = nFullInfo.mResize;
    TRACE_FUNC("sID(%d) Frame %d QFullImg %s", sensorID, request->mRequestNo, outImg.mBuffer == NULL ? "is null" : "" );
    MSize outSize = (resize.w && resize.h) ? resize : srcCropRect.s;
    MRectF domainSrcCrop = request->needEarlyFSCVendorFullImg() ?
        request->getVar<MRectF>(SFP_VAR::FSC_RRZO_CROP_REGION, MRectF(0, 0)) : MRectF(srcCropRect.p, srcCropRect.s);

    if( mPipeUsage.isForceP2Zoom() )
    {
        MY_LOGD_IF(request->needPrintIO(), "need pre-zoom for next full, origSrc " MCropF_STR ",newCrop " MCropF_STR ", resize(%dx%d)",
                MCropF_ARG(domainSrcCrop), MCropF_ARG(request->getZoomROI()), resize.w, resize.h);
        domainSrcCrop = request->getZoomROI();
    }
    else if(resize.w && resize.h && nFullInfo.mNeedCrop)
    {
        MY_LOGD_IF(request->needPrintIO(), "need nextFull crop for next full, origSrc " MCropF_STR ", resize(%dx%d)",
                MCropF_ARG(domainSrcCrop), resize.w, resize.h);
        domainSrcCrop = applyViewRatio(domainSrcCrop, resize);
        MY_LOGD_IF(request->needPrintIO(), "need nextFull crop for next full, FinalSrcCrop " MCropF_STR,
                MCropF_ARG(domainSrcCrop));
    }


    outImg.mBuffer->getImageBuffer()->setExtParam(outSize);
    outImg.accumulate("p2aNFull", request->mLog, inInfo.inSize, domainSrcCrop, outSize);
    outImg.mBuffer->getImageBuffer()->setTimestamp(inInfo.timestamp);
    MY_LOGD_IF(request->needEarlyFSCVendorFullImg(), "mDomainOffset(%f,%f) mDomainTransformScale(%f,%f) from(%f,%f) to(%d,%d) earlyFSC(%d) flag(0x%x)",
        outImg.mTransform.mOffset.x, outImg.mTransform.mOffset.y, outImg.mTransform.mScale.w, outImg.mTransform.mScale.h,
        domainSrcCrop.s.w, domainSrcCrop.s.h, outSize.w, outSize.h, request->needEarlyFSCVendorFullImg(),
        FSCUtil::getConstrainFlag(mPipeUsage.getFSCMode()));

    output.mBuffer = outImg.mBuffer->getImageBufferPtr();
    output.mTransform = 0;
    output.mCropRect = domainSrcCrop;
    output.mCropDstSize = outSize;
    output.mDMAConstrain = FSCUtil::getConstrainFlag(mPipeUsage.getFSCMode());
    if(request->needRegDump())
    {
        android::sp<P2ADummyData> dummyData = new P2ADummyData();
        data.mDummyDatas.push_back(dummyData);

        Feature::P2Util::MDPObjPtr mdpObjPtr;
        mdpObjPtr.needDump = true;
        mdpObjPtr.isDummyPQ = true;
        mdpObjPtr.pqDebug = Feature::P2Util::PQ_DEBUG_S_P2A;
        output.mDpPqParam = Feature::P2Util::makeDpPqParam(&(dummyData->mP2Obj.pqWDMA), request->mP2Pack, 0, mdpObjPtr);
        output.mPqParam = &(dummyData->mP2Obj.pqParam);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareFDImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    P2IO out;
    if ( request->popFDOutput(this, out) && out.isValid() )
    {
        Output output = toOutput(out, EPortIndex_IMG2O);
        frame.mvOut.push_back(output);

        if( !out.isCropValid())
        {
            MY_LOGD("default fd crop");
            out.mCropDstSize = out.mBuffer->getImgSize();
            out.mCropRect = MRect(MPoint(0, 0), out.mCropDstSize);
        }
        Feature::P2Util::push_crop(frame, IMG2O_CROP_GROUP, out.mCropRect, out.mCropDstSize);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareDs1Img(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, const FrameInInfo &inInfo, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    MUINT32 sensorID = request->mMasterID;
    const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
    const MRect& srcCropRect = srcCropInfo.mSrcCrop;
    OutputInfo fdInfo;
    MSize fdSize = request->getOutputInfo(IO_TYPE_FD, fdInfo) ? fdInfo.mOutSize : MSize(0,0);
    NextFullInfo nfInfo = request->needNextFullImg(this, sensorID) ? request->getNextFullInfo(this, sensorID) : NextFullInfo();

    MRectF domainSrcCrop = (request->needEarlyFSCVendorFullImg() && request->needNextFullImg(this, sensorID) )
        ? request->getVar<MRectF>(SFP_VAR::FSC_RRZO_CROP_REGION, MRectF(0, 0)) : MRectF(srcCropRect.p, srcCropRect.s);
    if(nfInfo.mResize.w && nfInfo.mResize.h && nfInfo.mNeedCrop)
    {
        if(fdSize.w && fdSize.h)
        {
            MY_LOGW("Next Full need crop and also FD needed !! It should not supported yet ! FD maybe not correct.");
        }
        MY_LOGD_IF(request->needPrintIO(), "need DS1 crop for next full, origSrc " MCropF_STR ", resize(%dx%d)",
                MCropF_ARG(domainSrcCrop), nfInfo.mResize.w, nfInfo.mResize.h);
        domainSrcCrop = applyViewRatio(domainSrcCrop, nfInfo.mResize);
        MY_LOGD_IF(request->needPrintIO(), "need DS1 crop for next full, FinalSrcCrop " MCropF_STR,
                MCropF_ARG(domainSrcCrop));
    }
    // for later DSDN DS1
    request->setVar<MRectF>(SFP_VAR::P2A_SRC_CROP_REGION, domainSrcCrop);

    MSize ds1FullSize = (nfInfo.mResize.w && nfInfo.mResize.h) ? nfInfo.mResize : srcCropRect.s;
    MSize outSize = mDSDNCfg.getSize(DSDNCfg::DS1, ds1FullSize, fdSize,
                                                    request->mP2Pack.getSensorData().mNvramDsdn.mRatioMultiple,
                                                    request->mP2Pack.getSensorData().mNvramDsdn.mRatioDivider);

    data.mDS1Img.mSensorClipInfo = request->getSensorClipInfo(sensorID);

    data.mDS1Img.mBuffer = mDS1ImgPool->requestIIBuffer();
    data.mDS1Img.mBuffer->getImageBuffer()->setExtParam(outSize);
    data.mDS1Img.accumulate("p2aDS1", request->mLog, inInfo.inSize, domainSrcCrop, outSize);
    data.mDS1Img.mBuffer->getImageBuffer()->setTimestamp(inInfo.timestamp);

    P2IO output;
    output.mBuffer = data.mDS1Img.mBuffer->getImageBufferPtr();
    output.mTransform = 0;
    output.mCropRect = domainSrcCrop;
    output.mCropDstSize = outSize;
    output.mDMAConstrain = FSCUtil::getConstrainFlag(mPipeUsage.getFSCMode());

    frame.mvOut.push_back(toOutput(output, EPortIndex_IMG2O));
    Feature::P2Util::push_crop(frame, IMG2O_CROP_GROUP, output.mCropRect, output.mCropDstSize);

    MY_LOGD_IF(request->needEarlyFSCVendorFullImg(), "mDomainOffset(%f,%f) mDomainTransformScale(%f,%f) from(%f,%f) to(%d,%d) earlyFSC(%d) flag(0x%x)",
        data.mDS1Img.mTransform.mOffset.x, data.mDS1Img.mTransform.mOffset.y, data.mDS1Img.mTransform.mScale.w, data.mDS1Img.mTransform.mScale.h,
        domainSrcCrop.s.w, domainSrcCrop.s.h, outSize.w, outSize.h, request->needEarlyFSCVendorFullImg(),
        FSCUtil::getConstrainFlag(mPipeUsage.getFSCMode()));

    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepareExtraMDPCrop(const BasicImg &fullImg, std::vector<P2IO> &leftOutList)
{
    TRACE_FUNC_ENTER();
    if(fullImg.mBuffer == NULL)
    {
        MY_LOGE("Need Extra MDP but Full Image is NULL !!");
    }
    for(auto&& sfpOut : leftOutList)
    {
        sfpOut.mCropRect = fullImg.mTransform.applyTo(sfpOut.mCropRect);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::needFullForExtraOut(std::vector<P2IO> &outList)
{
    static const MUINT32 maxMDPOut = 2;
    if(outList.size() > maxMDPOut)
        return MTRUE;
    MUINT32 rotCnt = 0;
    for(auto&& out : outList)
    {
        if(out.mTransform != 0)
        {
            rotCnt += 1;
        }
    }
    return (rotCnt > 1);
}

MBOOL P2ANode::needNextFullByIMG3O(const RequestPtr &request, MUINT32 sensorID)
{
    MBOOL ret = MFALSE;
    if(request->needNextFullImg(this, sensorID))
    {
        NextFullInfo info = request->getNextFullInfo(this, sensorID);
        ret = info.mCanShare && !request->needEarlyFSCVendorFullImg();
    }
    return ret;
}

MVOID P2ANode::enqueFeatureStream(Feature::P2Util::DIPParams &params, P2AEnqueData &data, const P2ATuningIndex &tuningIndex)
{
    TRACE_FUNC_ENTER();
    android::String8 str;
    for(const sp<P2ARunData> &runData : data.mRunDatas)
    {
        str.appendFormat("(%s)-sID(%d)-magic(old/new)(%d/%d)", runData->mName.c_str(), runData->mSensorID, runData->mPreDCEMagic, runData->mDCESORec.mMagic3A);
    }
    MY_S_LOGD(data.mRequest->mLog, "sensor(%d) Frame %d enque start, TuningIndex, (GN/Ph/L),master(%d/%d/%d), slave(%d/%d/%d), DCESO:%s", mSensorIndex, data.mRequest->mRequestNo, tuningIndex.mGenMaster, tuningIndex.mPhyMaster, tuningIndex.mLargeMaster, tuningIndex.mGenSlave, tuningIndex.mPhySlave, tuningIndex.mLargeSlave, str.string());
    data.mRequest->mTimer.startEnqueP2A();
    this->incExtThreadDependency();

    this->enqueDIPStreamBase(mDIPStream, params, data);
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::init3A()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    if( mp3A == NULL && SUPPORT_3A_HAL )
    {
        mp3A = MAKE_Hal3A(mSensorIndex, PIPE_CLASS_TAG);
    }

    MUINT eisMode = mPipeUsage.getEISMode();
    if( EIS_MODE_IS_EIS_22_ENABLED(eisMode) ||
        EIS_MODE_IS_EIS_30_ENABLED(eisMode) )
    {
        //Disable OIS
        MY_LOGD("mEisMode: 0x%x => Disable OIS \n", eisMode);
        if( mp3A )
        {
            mp3A->send3ACtrl(E3ACtrl_SetEnableOIS, 0, 0);
        }
        else
        {
            MY_LOGE("mp3A is NULL\n");
        }
        mEisMode = mPipeUsage.getEISMode();
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::uninit3A()
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();

    if( mp3A )
    {
        //Enable OIS
        if( EIS_MODE_IS_EIS_22_ENABLED(mEisMode) ||
            EIS_MODE_IS_EIS_30_ENABLED(mEisMode) )
        {
            MY_LOGD("mEisMode: 0x%x => Enable OIS \n", mEisMode);
            mp3A->send3ACtrl(E3ACtrl_SetEnableOIS, 1, 0);
            mEisMode = EIS_MODE_OFF;
        }

        // turn OFF 'pull up ISO value to gain FPS'
        AE_Pline_Limitation_T params;
        params. bEnable = MFALSE; // disable
        params. bEquivalent= MTRUE;
        params. u4IncreaseISO_x100= 100;
        params. u4IncreaseShutter_x100= 100;
        mp3A->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

        // destroy the instance
        mp3A->destroyInstance(PIPE_CLASS_TAG);
        mp3A = NULL;

    }

    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::createDCESOs()
{
    TRACE_FUNC_ENTER();
    IHalISP *isp = getP2CamContext(mSensorIndex)->getISP();
    if(isp == NULL)
    {
        MY_LOGE("Can not get IHalISP from P2CamContext !");
    }
    else
    {
        NS3Av3::Buffer_Info bufInfo;
        if(isp->queryISPBufferInfo(bufInfo) && bufInfo.DCESO_Param.bSupport)
        {
            mDCESOImgPool = ImageBufferPool::create("fpipe.p2a_dceso", bufInfo.DCESO_Param.size, (EImageFormat)bufInfo.DCESO_Param.format, ImageBufferPool::USAGE_HW_AND_SW);
            for(MUINT32 sID : mPipeUsage.getAllSensorIDs())
            {
                android::String8 str;
                str.appendFormat("fpipe.dceQueue#%d", sID);
                mDCEQueueMap[sID] = new ThreadSafeQueue<DCESORecord>(str.string());
                mPhyDCEQueueMap[sID] = new ThreadSafeQueue<DCESORecord>(str.string());
                mLargeDCEQueueMap[sID] = new ThreadSafeQueue<DCESORecord>(str.string());
            }
        }

    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::createTuningBufs()
{
    TRACE_FUNC_ENTER();
    mDynamicTuningPool = TuningBufferPool::create("fpipe.p2aTuningBuf", NSIoPipe::NSPostProc::INormalStream::getRegTableSize(), TuningBufferPool::BUF_PROTECT_RUN);

    IHalISP *isp = getP2CamContext(mSensorIndex)->getISP();
    if( isp == NULL )
    {
        MY_LOGE("Can not get IHalISP from P2CamContext !");
    }
    else
    {
        NS3Av3::Buffer_Info bufInfo;
        MUINT32 syncSize = 0;
        if( isp->queryISPBufferInfo(bufInfo) )
        {
            syncSize = bufInfo.u4DualSyncInfoSize;
        }
        else
        {
            NS3Av3::IIspMgr* ispMgr = MAKE_IspMgr();
            if( ispMgr != NULL)
            {
                syncSize = ispMgr->queryDualSyncInfoSize();
            }
        }

        if(syncSize != 0)
        {
            mSyncTuningPool = TuningBufferPool::create("fpipe.p2aSyncTuningBuf", std::max<MUINT32>(syncSize, (MUINT32)256), TuningBufferPool::BUF_PROTECT_RUN);
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

SmartTuningBuffer P2ANode::requestSyncTuningBuf()
{
    TRACE_FUNC_ENTER();
    SmartTuningBuffer buf;
    if( mSyncTuningPool != NULL)
    {
        buf = mSyncTuningPool->request();
        memset(buf->mpVA, 0, mSyncTuningPool->getBufSize());
    }
    TRACE_FUNC_EXIT();
    return buf;
}

MBOOL P2ANode::initDCEQueue(DCE_QUEUE_MAP &queueMap)
{
    TRACE_FUNC_ENTER();
    for(auto&& it : queueMap)
    {
        for(MUINT32 i = 0 ; i < DCESO_DELAY_COUNT ; i++)
        {
            DCESORecord dceRec;
            dceRec.mImg = mDCESOImgPool->requestIIBuffer();
            it.second->push(dceRec);
        }
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::destroyDCESOs()
{
    TRACE_FUNC_ENTER();
    mDCEQueueMap.clear();
    mPhyDCEQueueMap.clear();
    mLargeDCEQueueMap.clear();
    ImageBufferPool::destroy(mDCESOImgPool);
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::destroyTuningBufs()
{
    TRACE_FUNC_ENTER();
    TuningBufferPool::destroy(mDynamicTuningPool);
    TuningBufferPool::destroy(mSyncTuningPool);
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepare3A(Feature::P2Util::DIPParams &params, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    (void)params;
    (void)request;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MINT32 P2ANode::prepareOneRawTuning(const char*             pathName,
                                          MUINT32           sensorID,
                                    const SFPIOMap          &ioMap,
                                          Feature::P2Util::DIPParams           &params,
                                    const RequestPtr        &request,
                                          P2AEnqueData      &data,
                                          P2ATuningExtra    &extra)
{
    TRACE_FUNC_ENTER();
    MINT32 retIndex = -1;
    sp<P2ARunData> runData = new P2ARunData(pathName);
    runData->mSensorID = sensorID;
    runData->mIspObj.run = NSIoPipe::P2_RUN_S_P2A;
    Feature::P2Util::P2Pack newPack = Feature::P2Util::P2Pack(request->mP2Pack, request->mP2Pack.mLog, sensorID);
    runData->mTuningBuf = mDynamicTuningPool->request();
    data.mTuningBufs.push_back(runData->mTuningBuf);
    memset(runData->mTuningBuf->mpVA, 0, mDynamicTuningPool->getBufSize());

    TuningHelper::Input tuningIn(newPack, runData->mTuningBuf);
    tuningIn.mpISP = getP2CamContext(sensorID)->getISP();
    tuningIn.mTag = NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
    tuningIn.mP2ObjPtr = runData->mIspObj.toPtrTable();
    tuningIn.mSyncTuningBuf = extra.mSyncTuning;
    tuningIn.mNeedRegDump = request->needRegDump();

    // --- prepare P2IO ---
    const SFPSensorTuning &tuning = ioMap.getTuning(sensorID);
    const SFPSensorInput &sensorIn = request->getSensorInput(sensorID);
    Feature::P2Util::P2IOPack &io = tuningIn.mIOPack;
    if(tuning.isRRZOin())
    {
        io.mIMGI.mBuffer = get(sensorIn.mRRZO);
        io.mFlag |= Feature::P2Util::P2Flag::FLAG_RESIZED;
    }
    else
    {
        io.mIMGI.mBuffer = get(sensorIn.mIMGO);
    }

    if(tuning.isLCSOin())
        io.mLCSO.mBuffer = get(sensorIn.mLCSO);

    if(tuning.isLCSHOin())
        io.mLCSHO.mBuffer = get(sensorIn.mLCSHO);

    if (io.mIMGI.mBuffer == NULL)
    {
        MY_LOGE("Invalid input buffer");
        TRACE_FUNC_EXIT();
        return retIndex;
    }

    popDCESO(extra.mDCESOQueue, runData);
    io.mDCESO.mBuffer = runData->mDCESORec.getBufferPtr();
    tuningIn.mDCESOMagicNum = runData->mDCESORec.mMagic3A;

    TuningHelper::MetaParam metaParam;
    metaParam.mHalIn = sensorIn.mHalIn;
    metaParam.mAppIn = tuning.isAppPhyMetaIn() ? sensorIn.mAppInOverride : sensorIn.mAppIn;
    metaParam.mExtraAppOut = extra.mExtraAppOut;
    metaParam.mScene = extra.mScene;
    metaParam.mSensorID = sensorID;
    metaParam.mMasterID = request->mMasterID;
    MBOOL needMetaOut = ((ioMap.isGenPath() && sensorID != request->getMasterID()) || (extra.mScene != TuningHelper::Tuning_Normal))
                    ? MFALSE : MTRUE;
    VarMap<SFP_VAR> &varMap = request->getSensorVarMap(sensorID);
    if(needMetaOut)
    {
        metaParam.mHalOut = ioMap.mHalOut;
        metaParam.mAppOut = ioMap.mAppOut;
        if(varMap.tryGet<MRect>(SFP_VAR::FD_CROP_ACTIVE_REGION, metaParam.mFdCrop))
        {
            metaParam.mIsFDCropValid = MTRUE;
        }
    }

    // 3DNR
    NR3D::NR3DTuningInfo nr3dTuning;

    MBOOL canEnable3dnrOnFrame = varMap.get<MBOOL>(SFP_VAR::NR3D_P2A_CAN_ENABLE_ON_FRAME, MFALSE);
    if( !tuning.isDisable3DNR() && metaParam.mScene == TuningHelper::Tuning_Normal
        && canEnable3dnrOnFrame
      )
    {
        if (mNr3dHalResult.nr3dHwParam.ctrl_onEn != 0)
        {
            nr3dTuning.canEnable3dnrOnFrame = canEnable3dnrOnFrame;
            nr3dTuning.isoThreshold = varMap.get<MUINT32>(SFP_VAR::NR3D_P2A_ISO_THRESHOLD, 100);
            nr3dTuning.mvInfo = mNr3dHalResult.gmvInfo;
            nr3dTuning.inputSize = io.mIMGI.mBuffer->getImgSize();
            // If CRZ is used, we must correct following fields and review ISP code
            const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
            nr3dTuning.inputCrop = srcCropInfo.mSrcCrop;
            // gyro
            nr3dTuning.gyroData = varMap.get<NR3D::GyroData>(SFP_VAR::NR3D_GYRO, NR3D::GyroData());
            //prepare 3DNR35 before setp2isp
            nr3dTuning.nr3dHwParam = mNr3dHalResult.nr3dHwParam;
            metaParam.mpNr3dTuningInfo = &nr3dTuning;
        }
    }

    Feature::P2Util::DIPFrameParams   frameParam;
    if( TuningHelper::processIsp_P2A_Raw2Yuv(tuningIn,
                                                frameParam,
                                                metaParam))
    {
        params.mvDIPFrameParams.push_back(frameParam);
        retIndex = params.mvDIPFrameParams.size() - 1;
    }
    else
    {
        MY_LOGE("Prepare Raw Tuning Failed! Path(%s), sensor(%d),frameNo(%d),mvFrameSize(%zu)",
                ioMap.pathName(), sensorID, request->mRequestNo, params.mvDIPFrameParams.size());
    }
    updateDCESO(request, runData, sensorID, metaParam.mDCESOEnqued);
    data.mRunDatas.push_back(runData);
    TRACE_FUNC_EXIT();
    return retIndex;
}

MBOOL P2ANode::prepareRawTuning(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, P2ATuningIndex &tuningIndex)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    MBOOL dualSlaveValid = request->isSlaveParamValid();
    SFPIOManager &ioMgr = request->mSFPIOManager;
    const SFPIOMap &generalIO = ioMgr.getFirstGeneralIO();
    const SFPIOMap &masterPhyIO = ioMgr.getPhysicalIO(request->mMasterID);
    const SFPIOMap &slavePhyIO = ioMgr.getPhysicalIO(request->mSlaveID);
    const SFPIOMap &masterLargeIO = ioMgr.getLargeIO(request->mMasterID);
    const SFPIOMap &slaveLargeIO = ioMgr.getLargeIO(request->mSlaveID);
    const MBOOL isMergePath_Master = SFPIOMap::isSameTuning(masterPhyIO, generalIO, request->mMasterID);
    const MBOOL isMergePath_Slave = SFPIOMap::isSameTuning(slavePhyIO, generalIO, request->mSlaveID);

    SmartTuningBuffer syncTuning = requestSyncTuningBuf();

    if(generalIO.isValid())
    {
        // -- Master ---
        if(needNormalYuv(request->mMasterID, request))
        {
            P2ATuningExtra extra(mDCEQueueMap[request->mMasterID], isMergePath_Master ? masterPhyIO.mAppOut : NULL);
            extra.mSyncTuning = syncTuning;
            tuningIndex.mGenMaster = prepareOneRawTuning("GM", request->mMasterID, generalIO, params, request, data, extra);
        }

        // -- Slave ---
        if(dualSlaveValid && needNormalYuv(request->mSlaveID, request))
        {
            P2ATuningExtra extra(mDCEQueueMap[request->mSlaveID], isMergePath_Slave ? slavePhyIO.mAppOut : NULL);
            extra.mSyncTuning = syncTuning;
            tuningIndex.mGenSlave = prepareOneRawTuning("GS", request->mSlaveID, generalIO, params, request, data, extra);
        }

        MY_LOGE_IF(!(tuningIndex.isGenMasterValid()),
                    "GeneralIO valid but General tuning master inValid !!");
    }

    if(masterPhyIO.isValid())
    {
        MBOOL masterFrameValid = tuningIndex.isGenMasterValid();
        if( ! isMergePath_Master || !masterFrameValid)
        {
            P2ATuningExtra extra(mPhyDCEQueueMap[request->mMasterID]);
            extra.mSyncTuning = syncTuning;
            tuningIndex.mPhyMaster = prepareOneRawTuning("PhM", request->mMasterID, masterPhyIO, params, request, data, extra);
        }
        else
        {
            tuningIndex.mPhyMaster = tuningIndex.mGenMaster;
        }
    }

    if(slavePhyIO.isValid() && dualSlaveValid)
    {
        MBOOL slaveFrameValid = tuningIndex.isGenSlaveValid();
        if( ! isMergePath_Slave || !slaveFrameValid)
        {
            P2ATuningExtra extra(mPhyDCEQueueMap[request->mSlaveID]);
            extra.mSyncTuning = syncTuning;
            tuningIndex.mPhySlave = prepareOneRawTuning("PhS", request->mSlaveID, slavePhyIO, params, request, data, extra);
        }
        else
        {
            tuningIndex.mPhySlave = tuningIndex.mGenSlave;
        }
    }

    if(masterLargeIO.isValid())
    {
        P2ATuningExtra extra(mLargeDCEQueueMap[request->mMasterID]);
        extra.mSyncTuning = syncTuning;
        tuningIndex.mLargeMaster = prepareOneRawTuning("LM", request->mMasterID, masterLargeIO, params, request, data, extra);
    }

    if(slaveLargeIO.isValid())
    {
        P2ATuningExtra extra(mLargeDCEQueueMap[request->mSlaveID]);
        extra.mSyncTuning = syncTuning;
        tuningIndex.mLargeSlave = prepareOneRawTuning("LS", request->mSlaveID, slaveLargeIO, params, request, data, extra);
    }

    MY_LOGI_IF((mLastDualParamValid != dualSlaveValid), "Dual Slave valid (%d)->(%d). slaveID(%d)",
            mLastDualParamValid, dualSlaveValid, request->mSlaveID);

    mLastDualParamValid = dualSlaveValid;

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::needNormalYuv(MUINT32 sensorID, const RequestPtr &request)
{
    if(sensorID == request->mMasterID)
    {
        return MTRUE;
    }
    else
    {
        return  request->needFullImg(this, sensorID)
                || request->needNextFullImg(this, sensorID);
    }
}

MVOID P2ANode::popDCESO(DCE_QUEUE_PTR &queue, android::sp<P2ARunData> &runData)
{
    TRACE_FUNC_ENTER();
    if(queue != NULL)
    {
        runData->mDCESORec = queue->pop();
        runData->mDCESOQueue = queue;
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::updateDCESO(const RequestPtr &request, const android::sp<P2ARunData> &runData, MUINT32 sensorID, MBOOL isDcesEnque)
{
    if(isDcesEnque)
    {
        runData->mPreDCEMagic = runData->mDCESORec.mMagic3A;
        runData->mDCESORec.mMagic3A = request->mP2Pack.getSensorData(sensorID).mMagic3A;
    }
}

MBOOL P2ANode::returnDCESO(const RequestPtr &/*request*/, const sp<P2ARunData> &runData)
{
    TRACE_FUNC_ENTER();
    if(runData->mDCESOQueue != NULL)
    {
        runData->mDCESOQueue->push(runData->mDCESORec);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
