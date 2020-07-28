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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "P2NRNode.h"

#define PIPE_CLASS_TAG "P2NRNode"
#define PIPE_TRACE TRACE_P2NR_NODE
#include <featurePipe/core/include/PipeLog.h>

#define P2NR_NSTM_NAME "S_P2NR"

#include "TuningHelper.h"
#include "P2CamContext.h"

#include "hal/inc/camera_custom_3dnr.h"
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_P2NR);

using NSCam::MRect;
using NSCam::NR3D::NR3DHALResult;
using NSCam::NR3D::NR3DHALParam;


using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSCam::NSIoPipe::ModuleInfo;
using NSCam::Feature::P2Util::P2SensorData;

using NSCam::NSIoPipe::NSPostProc::Hal3dnrBase;

using namespace NSCam::NSIoPipe;
using namespace NSCam::NR3D;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum DumpMaskIndex
{
    MASK_IMG3O,
};


const std::vector<DumpFilter> sFilterTable =
{
    DumpFilter( MASK_IMG3O,      "img3o" )
};

P2NRNode::P2NRNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mP2NRDatas);

    m3dnrLogLevel = getPropertyValue("vendor.debug.camera.3dnr.log.level", 0);
    TRACE_FUNC_EXIT();
}

P2NRNode::~P2NRNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL P2NRNode::onData(DataID id, const DSDNData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_P2NR )
    {
        mP2NRDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

IOPolicyType P2NRNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    IOPolicyType policy = IOPOLICY_BYPASS;
    if( reqInfo.isMaster() && HAS_DSDN20(reqInfo.mFeatureMask) )
    {
        policy = IOPOLICY_INPLACE;
    }
    return policy;
}

MBOOL P2NRNode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    StreamingFeatureNode::onInit();
    enableDumpMask(0xffff, sFilterTable);
    mDSDNCfg = mPipeUsage.getDSDNCfg();
    MSize size = mDSDNCfg.getMaxSize(DSDNCfg::DS2);
    EImageFormat fmt = mDSDNCfg.getFormat(DSDNCfg::DS2);
    mDS2ImgPool = ImageBufferPool::create("fpipe.p2nr.ds2", size, fmt, ImageBufferPool::USAGE_HW);
    if( mDS2ImgPool == NULL )
    {
        MY_LOGE("DS2 Img Pool create FAIL!!");
        ret = MFALSE;
    }

    mTuningPool = TuningBufferPool::create("fpipe.p2nr.tuningBuf", NSIoPipe::NSPostProc::INormalStream::getRegTableSize(), TuningBufferPool::BUF_PROTECT_RUN);

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2NR:init_normalStm");
    MY_LOGI("create & init NormalStream ++");
    mDIPStream = Feature::P2Util::DIPStream::createInstance(mSensorIndex);
    if( mDIPStream != NULL )
    {
        MBOOL secFlag = mPipeUsage.isSecureP2();
        ret = mDIPStream->init(P2NR_NSTM_NAME, NSIoPipe::EStreamPipeID_Normal, secFlag);
    }
    else
    {
        MY_LOGE("normal stream create instance fail!");
        ret = MFALSE;
    }
    MY_LOGI("create & init NormalStream --");
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2NRNode::onUninit()
{
    TRACE_FUNC_ENTER();
    IBufferPool::destroy(mDS2ImgPool);
    TuningBufferPool::destroy(mTuningPool);
    if( mDIPStream )
    {
        mDIPStream->uninit(P2NR_NSTM_NAME);
        mDIPStream->destroyInstance();
        mDIPStream = NULL;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2NRNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    allocate("fpipe.p2nr.dsImg", mDS2ImgPool, 4);
    allocate("fpipe.p2nr.tuning", mTuningPool, 3);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2NRNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2NRNode::onThreadLoop()
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    TRACE_FUNC("Waitloop");
    MBOOL need = MFALSE;
    DSDNData data;
    RequestPtr request;
    BasicImg out;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mP2NRDatas.deque(data) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( data.mRequest == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    request = data.mRequest;
    request->mTimer.startP2NR();
    need = request->needDSDN20();
    TRACE_FUNC("Frame %d in P2NR needDSDN20 %d", request->mRequestNo, need);
    if( need )
    {
        processP2NR(request, data.mData);
    }
    else
    {
        handleResultData(request, data.mData);
    }
    request->mTimer.stopP2NR();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2NRNode::prepare3DNR(Feature::P2Util::DIPParams &/*params*/, const RequestPtr &request, MUINT32 sensorID, DSDNImg &dsdnImg)
{
    TRACE_FUNC_ENTER();

    MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr: checkParam: requestNo=%d, sensorId=%d, FeatureMask(3DNR=%d, DSDN=%d",
        request->mRequestNo, sensorID,
        HAS_3DNR(request->mFeatureMask), HAS_DSDN20(request->mFeatureMask)
    );

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    sp<P2CamContext> p2CamContext = getP2CamContext(sensorID);
    Hal3dnrBase *p3dnr = p2CamContext->get3dnr();
    if (NULL == p3dnr)
    {
        MY_LOGW("no p3dnr!");
        TRACE_FUNC_EXIT();
        return MFALSE;
    }

    NR3DHALParam nr3dHalParam;

    // vipi related
    IImageBuffer *pVipiImgBuf2 = NULL;
    ImgBuffer prevFullImg2 = p2CamContext->getPrevFullImg2();
    if (prevFullImg2 != NULL)
    {
        pVipiImgBuf2 = prevFullImg2->getImageBufferPtr();
        nr3dHalParam.vipiInfo = NR3DHALParam::VipiInfo(MTRUE,
            pVipiImgBuf2->getImgFormat(), pVipiImgBuf2->getBufStridesInBytes(0),
            pVipiImgBuf2->getImgSize()
            );
    }
    else
    {
        nr3dHalParam.vipiInfo = NR3DHALParam::VipiInfo();
    }

    // 3a related
    nr3dHalParam.pTuningData = NULL;
    nr3dHalParam.p3A = p2CamContext->get3A();

    nr3dHalParam.featMask = NR3D_FEAT_MASK_BASIC_DSDN20;
    // put uniqueKey, requestNo, frameNo
    TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
    nr3dHalParam.uniqueKey = hint.UniqueKey;
    nr3dHalParam.requestNo = hint.RequestNo;
    nr3dHalParam.frameNo = hint.FrameNo;

    // p2aSrcCrop
    MRectF p2aSrcCropRectF;
    nr3dHalParam.p2aSrcCrop = request->getVar<MRectF>(SFP_VAR::P2A_SRC_CROP_REGION, p2aSrcCropRectF).toMRect();

    // generic
    nr3dHalParam.needChkIso = MFALSE;
    nr3dHalParam.iso = request->getVar<MUINT32>(SFP_VAR::NR3D_ISO, 200);
    nr3dHalParam.isoThreshold = request->getVar<MUINT32>(SFP_VAR::NR3D_DSDN_ISO_THRESHOLD, 0);

    // imgi related
    nr3dHalParam.isCRZUsed = 0;
    nr3dHalParam.isIMGO = 0;
    nr3dHalParam.isBinning = 0; // useless

    // gmv related info: VAR_3DNR_MV_INFO could be updated in the previous run
    NR3D::NR3DMVInfo currMvInfo;
    nr3dHalParam.gmvInfo = request->getVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, currMvInfo);

    //RSC related
    nr3dHalParam.rscInfo.isValid = 0;

    // output related: should be DS2Img, but use DS1Img due to the same size
    nr3dHalParam.dstRect = MRect(
        (dsdnImg.mDS1Img.mBuffer)->getImgSize().w,
        (dsdnImg.mDS1Img.mBuffer)->getImgSize().h);

    MY_LOGD_IF(m3dnrLogLevel >= 1, "do3dnrFlow: sensorID(%d), reqNo(%d) dstRect( (%d,%d) %dx%d)",
        sensorID, request->mRequestNo,
        nr3dHalParam.dstRect.p.x, nr3dHalParam.dstRect.p.y,
        nr3dHalParam.dstRect.s.w, nr3dHalParam.dstRect.s.h);

    // gyro: no need
    nr3dHalParam.gyroData.isValid = 0;

    mNr3dHalResult.resetGMVInfo();
    if (p3dnr->update3DNRMvInfo(nr3dHalParam, mNr3dHalResult.gmvInfo, mNr3dHalResult.isGMVInfoUpdated) == MTRUE)
    {
        if (mNr3dHalResult.isGMVInfoUpdated != MTRUE)
        {
            MY_LOGW("3dnr: dsdn20 GMV is NOT updated");
        }
    }
    else
    {
        MY_LOGW("3dnr->update3DNRMVInfo return false, which may affect 3dnr quality");
    }

    mNr3dHalResult.resetNr3dHwParam();
    MBOOL ret = p3dnr->do3dnrFlow(nr3dHalParam, mNr3dHalResult);

    TRACE_FUNC_EXIT();
    return ret;
}


MBOOL P2NRNode::processP2NR(const RequestPtr &request, DSDNImg &dsdnImg)
{
    TRACE_FUNC_ENTER();
    if( checkInput(request, dsdnImg) )
    {
        Feature::P2Util::DIPParams param;
        P2NREnqueData data;
        data.mRequest = request;
        data.mDSDNImg = dsdnImg;

        VarMap<SFP_VAR> &varMap = request->getSensorVarMap(request->getMasterID());
        MBOOL enable3dnr =  varMap.get<MBOOL>(SFP_VAR::NR3D_DSDN_CAN_ENABLE_ON_FRAME, MFALSE);
        if( request->need3DNR() )
        {
            MY_LOGD_IF(m3dnrLogLevel, "3dnr: on @ p2nr: %d", enable3dnr);
            if (enable3dnr)
            {
                if( !prepare3DNR(param, request, request->getMasterID(), dsdnImg) )
                {
                    // set mPrevFullImg to NULL to disable VIPI port
                    getP2CamContext(request->getMasterID())->setPrevFullImg2(NULL);
                }
            }
            else
            {
                // set mPrevFullImg to NULL to disable VIPI port
                getP2CamContext(request->getMasterID())->setPrevFullImg2(NULL);
            }
        }


        request->mTimer.startP2NRTuning();
        prepareTuning(param, request, data);
        request->mTimer.stopP2NRTuning();

        // prepare VIPI
        Feature::P2Util::DIPFrameParams &frame = param.mvDIPFrameParams.at(0);
        if( request->need3DNR() && getP2CamContext(request->getMasterID())->getPrevFullImg2() != NULL)
        {
            if (enable3dnr)
            {
                prepareVIPI(frame, request, data);
            }
        }

        // output
        prepareOutput(param, request, data);
        if( request->needPrintIO() )
        {
            NSCam::Feature::P2Util::printDIPParams(request->mLog, param);
        }

        // set 3dnr img3o for the next round
        getP2CamContext(request->getMasterID())->setPrevFullImg2(
            (enable3dnr && request->need3DNR() ) ? data.mDSDNImg.mDS2Img: NULL);

        enqueFeatureStream(param, data);
    }
    else
    {
        request->mTimer.stopP2NR();
        handleResultData(request, dsdnImg);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2NRNode::checkInput(const RequestPtr &request, const DSDNImg &dsdnImg)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( !dsdnImg.mFullImg.isValid() ||
        !dsdnImg.mDS1Img.isValid() )
    {
        MY_S_LOGW(request->mLog, "Frame %d: invalid input: full=%d ds1=%d", request->mRequestNo,
                dsdnImg.mFullImg.isValid(), dsdnImg.mDS1Img.isValid());
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2NRNode::prepareTuning(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2NREnqueData &data)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    MBOOL ret = MFALSE;
    MUINT32 sensorID = request->getMasterID();
    const SFPSensorInput &sensorIn = request->getSensorInput(sensorID);

    data.mTuningBuf = mTuningPool->request();
    memset(data.mTuningBuf->mpVA, 0, mTuningPool->getBufSize());
    TuningHelper::Input tuningIn(request->mP2Pack, data.mTuningBuf);
    tuningIn.mpISP = getP2CamContext(sensorID)->getISP();
    tuningIn.mP2ObjPtr.hasPQ = MFALSE;
    tuningIn.mP2ObjPtr.run = NSIoPipe::P2_RUN_S_P2NR;
    tuningIn.mNeedRegDump = request->needRegDump();

    Feature::P2Util::P2IOPack &io = tuningIn.mIOPack;
    io.mIMGI.mBuffer = data.mDSDNImg.mDS1Img.mBuffer->getImageBufferPtr();
    io.mFlag |= Feature::P2Util::P2Flag::FLAG_RESIZED;

    TuningHelper::MetaParam metaParam;
    metaParam.mHalIn = sensorIn.mHalIn;
    metaParam.mAppIn = sensorIn.mAppIn;
    metaParam.mScene = TuningHelper::Tuning_P2NR;
    metaParam.mSensorID = sensorID;

    NR3D::NR3DTuningInfo nr3dTuning;

    if(metaParam.mScene == TuningHelper::Tuning_P2NR)
    {
        if (mNr3dHalResult.nr3dHwParam.ctrl_onEn != 0)
        {
            nr3dTuning.canEnable3dnrOnFrame = request->getVar<MBOOL>(SFP_VAR::NR3D_DSDN_CAN_ENABLE_ON_FRAME, MFALSE);
            nr3dTuning.isoThreshold = request->getVar<MUINT32>(SFP_VAR::NR3D_DSDN_ISO_THRESHOLD, 100);
            nr3dTuning.mvInfo = mNr3dHalResult.gmvInfo;
            nr3dTuning.inputSize = data.mDSDNImg.mDS1Img.mBuffer->getImgSize();
            // If CRZ is used, we must correct following fields and review ISP code

            // get p2a src crop
            MRectF p2aSrcCropRectF;
            nr3dTuning.inputCrop = MRect(nr3dTuning.inputSize.w, nr3dTuning.inputSize.h);

            // gyro
            nr3dTuning.gyroData = request->getVar<NR3D::GyroData>(SFP_VAR::NR3D_GYRO, NR3D::GyroData());

            //prepare 3DNR35 before setp2isp
            nr3dTuning.nr3dHwParam = mNr3dHalResult.nr3dHwParam;
            metaParam.mpNr3dTuningInfo = &nr3dTuning;
         }
         else
         {
             MY_LOGD_IF(m3dnrLogLevel >= 1, "mkdbg: 3dnr2 prepareTuning: no need");
         }
    }
    // 3DNR tuning --  end

    Feature::P2Util::DIPFrameParams   frameParam;
    if( TuningHelper::processIsp_P2NR_Yuv2Yuv(tuningIn, frameParam, metaParam))
    {
        params.mvDIPFrameParams.push_back(frameParam);
        ret = MTRUE;
    }
    else
    {
        MY_LOGE("Prepare Raw Tuning Failed! sensor(%d),frameNo(%d),mvFrameSize(%zu)",
                 sensorID, request->mRequestNo, params.mvDIPFrameParams.size());
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2NRNode::prepareVIPI(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2NREnqueData &data)
{
    TRACE_FUNC_ENTER();
    data.mPrevFullImg = getP2CamContext(request->getMasterID())->getPrevFullImg2();
    Input input;
    input.mPortID = PortID(EPortType_Memory, EPortIndex_VIPI, PORTID_IN);
    input.mBuffer = data.mPrevFullImg->getImageBufferPtr();
    frame.mvIn.push_back(input);

    TRACE_FUNC_EXIT();
}


MBOOL P2NRNode::prepareOutput(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2NREnqueData &data)
{
    Feature::P2Util::DIPFrameParams &frame = params.mvDIPFrameParams.at(0);

    ImgBuffer ds1Buf = data.mDSDNImg.mDS1Img.mBuffer;
    FrameInInfo inInfo;
    inInfo.inSize = ds1Buf->getImgSize();
    inInfo.timestamp = ds1Buf->getTimestamp();

    prepareDs2Img(frame, request, data.mDSDNImg.mDS2Img, inInfo);

    prepareFDImg(frame, request, data.mDSDNImg.mDS1Img);
    return MTRUE;
}

MVOID P2NRNode::prepareDs2Img(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, ImgBuffer &outImg, const FrameInInfo &inInfo)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d FullImgPool=(%d/%d)", request->mRequestNo, mDS2ImgPool->peakAvailableSize(), mDS2ImgPool->peakPoolSize());

    outImg = mDS2ImgPool->requestIIBuffer();

    sp<IImageBuffer> pIMGBuffer = outImg->getImageBuffer();
    pIMGBuffer->setTimestamp(inInfo.timestamp);
    if(!pIMGBuffer->setExtParam(inInfo.inSize))
    {
        MY_S_LOGE(request->mLog, "Ds2 Img setExtParm Fail!, target size(%dx%d)", inInfo.inSize.w, inInfo.inSize.h);
    }
    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_IMG3O, PORTID_OUT);
    output.mBuffer = pIMGBuffer.get();
    frame.mvOut.push_back(output);

    TRACE_FUNC_EXIT();
}

MVOID P2NRNode::prepareFDImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, const BasicImg &ds1Img)
{
    TRACE_FUNC_ENTER();
    P2IO out;
    if ( request->popFDOutput(this, out) && out.isValid() )
    {
        Output output = toOutput(out, EPortIndex_IMG2O);
        frame.mvOut.push_back(output);

        if( !out.isCropValid())
        {
            MY_S_LOGW(request->mLog, "use default fd crop (0,0,input size), maybe something wrong");
            out.mCropDstSize = out.mBuffer->getImgSize();
            out.mCropRect = MRect(MPoint(0, 0), out.mCropDstSize);
        }
        else
        {
            MRectF cropRect = ds1Img.mTransform.applyTo(out.mCropRect);

            TRACE_FUNC("No(%d)," MTransF_STR ",source=" MCropF_STR "->result=" MCropF_STR,
                    request->mRequestNo, MTransF_ARG(ds1Img.mTransform.mOffset, ds1Img.mTransform.mScale),
                    MCropF_ARG(out.mCropRect), MCropF_ARG(cropRect));
            MSizeF sourceSize = ds1Img.mBuffer->getImageBuffer()->getImgSize();
            refineBoundaryF("P2NRNode", sourceSize, cropRect, request->needPrintIO());
            out.mCropRect = cropRect;
        }
        Feature::P2Util::push_crop(frame, IMG2O_CROP_GROUP, out.mCropRect, out.mCropDstSize);
    }
    TRACE_FUNC_EXIT();
}


MVOID P2NRNode::enqueFeatureStream(Feature::P2Util::DIPParams &params, P2NREnqueData &data)
{
    TRACE_FUNC_ENTER();
    MY_S_LOGD(data.mRequest->mLog, "P2NR sensor(%d) Frame %d enque start", mSensorIndex, data.mRequest->mRequestNo);
    data.mRequest->mTimer.startP2NREnque();
    this->incExtThreadDependency();
    this->enqueDIPStreamBase(mDIPStream, params, data);
    TRACE_FUNC_EXIT();
}

MVOID P2NRNode::onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const P2NREnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing P2NRNode class members
    TRACE_FUNC_ENTER();

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopP2NREnque();
        MY_S_LOGD(data.mRequest->mLog, "P2NR sensor(%d) Frame %d enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedP2NREnque(), params.mDequeSuccess);

        if( !params.mDequeSuccess )
        {
            MY_LOGW("Frame %d enque result failed", request->mRequestNo);
        }

        request->updateResult(params.mDequeSuccess);
        handleResultData(request, data.mDSDNImg);
        request->mTimer.stopP2NR();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID P2NRNode::handleResultData(const RequestPtr &request, const DSDNImg &dsdnImg)
{
    TRACE_FUNC_ENTER();
    handleDump(request, dsdnImg);
    handleData(ID_P2NR_TO_VNR, DSDNData(dsdnImg, request));
    TRACE_FUNC_EXIT();
}

MVOID P2NRNode::handleDump(const RequestPtr &request, const DSDNImg &dsdnImg)
{
    TRACE_FUNC_ENTER();
    if( request->needNddDump() )
    {
        if( dsdnImg.mDS2Img != NULL && allowDump(MASK_IMG3O) )
        {
            TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
            dsdnImg.mDS2Img->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, dsdnImg.mDS2Img->getImageBufferPtr(), TuningUtils::YUV_PORT_IMG3O, "p2nr");
        }
    }
    if( request->needDump() )
    {
        if( dsdnImg.mDS2Img != NULL && allowDump(MASK_IMG3O) )
        {
            dsdnImg.mDS2Img->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(request, dsdnImg.mDS2Img->getImageBufferPtr(), "p2nr-full");
        }
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
