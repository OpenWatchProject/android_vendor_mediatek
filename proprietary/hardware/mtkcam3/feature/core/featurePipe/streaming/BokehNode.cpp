
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

#include "BokehNode.h"

#include <mtkcam3/feature/stereo/hal/StereoArea.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <isp_tuning.h>

#include <stereo_tuning_provider.h>

#define PIPE_CLASS_TAG "BokehNode"
#define PIPE_TRACE TRACE_SFP_BOKEH_NODE
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/utils/p2/P2Util.h>
#include "P2CamContext.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_BOKEH);
#define ULOGMD_IF(cond, ...)       do { if ( (cond) ) { CAM_ULOGMD(__VA_ARGS__); } } while(0)

#define BOKEH_NORMAL_STREAM_NAME "SFP_Bokeh"
#define PIPE_LOG_TAG "BokehNode"

using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSIspTuning;
using VSDOF::util::sMDP_Config;

class DpBlitStream;
//=======================================================================================
#if SUPPORT_VSDOF
//=======================================================================================
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

BokehNode::BokehNode(const char *name)
    : StreamingFeatureNode(name)
    , mDIPStream(nullptr)
{
    CAM_ULOGM_APILIFE();
    this->addWaitQueue(&mDepthDatas);
}

BokehNode::~BokehNode()
{
    CAM_ULOGM_APILIFE();
}

MBOOL BokehNode::onData(DataID id, const DepthImgData &data)
{
    CAM_ULOGM_APILIFE();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    switch (id) {
    case ID_DEPTH_TO_BOKEH:
        mDepthDatas.enque(data);
        ret = MTRUE;
        break;
    default:
        ret = MFALSE;
        break;
    }
    return ret;
}

IOPolicyType BokehNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &reqInfo) const
{
    return reqInfo.isMaster() ? IOPOLICY_INOUT : IOPOLICY_BYPASS;
}

MBOOL BokehNode::onInit()
{
    CAM_ULOGM_APILIFE();
    StreamingFeatureNode::onInit();

    mDIPStream =  Feature::P2Util::DIPStream::createInstance(mSensorIndex);
    if( mDIPStream == nullptr || !mDIPStream->init(BOKEH_NORMAL_STREAM_NAME))
    {
        CAM_ULOGME("BokehNode Init DIPStream Failed ! DIPStream(%p)", mDIPStream);
    }

    mDynamicTuningPool = TuningBufferPool::create("bokeh.p2btuningBuf",
                                Feature::P2Util::DIPStream::getRegTableSize(),
                                TuningBufferPool::BUF_PROTECT_RUN);

    mbShowDepthMap  = property_get_int32("vendor.debug.tkflow.bokeh.showdepth", 0);
    mbDumpImgeBuf   = property_get_int32("vendor.debug.tkflow.bokeh.dump_img" , 0);
    mbPipeLogEnable = property_get_int32("vendor.debug.tkflow.bokeh.log"      , 0);
    mbIMG3ODumpBuf  = property_get_int32("vendor.debug.tkflow.bokeh.img3o"    , 0);
    mbDumpDIPParam  = property_get_int32("vendor.debug.tkflow.bokeh.dipparam" , 0);

    extract_by_SensorOpenId(&mDumpHint, mSensorIndex);//Need to confirm with Jou-Feng or Ray

    CAM_ULOGMD("openid(%#x), ShowDepthMap(%d), DumpImgBuf(%d)",
                mSensorIndex, mbShowDepthMap, mbDumpImgeBuf);

    return MTRUE;
}

MBOOL BokehNode::onUninit()
{
    CAM_ULOGM_APILIFE();
    if( mDIPStream )
    {
        mDIPStream->uninit(BOKEH_NORMAL_STREAM_NAME);
        mDIPStream->destroyInstance();
        mDIPStream = nullptr;
    }
    TuningBufferPool::destroy(mDynamicTuningPool);
    //
    if (mpDpStream != nullptr)
        delete mpDpStream;
    // 3A
    if (mpIspHal_Main1) {
        mpIspHal_Main1->destroyInstance("BOKEH_3A_MAIN1");
        mpIspHal_Main1 = nullptr;
    }
    if (mp3AHal_Main1) {
        mp3AHal_Main1->destroyInstance("BOKEH_3A_MAIN1");
        mp3AHal_Main1 = nullptr;
    }

    return MTRUE;
}

MBOOL BokehNode::onThreadStart()
{
    CAM_ULOGM_APILIFE();
    if( mFullImgPoolAllocateNeed && mFullImgPool != nullptr )
    {
        Timer timer;
        timer.start();
        mFullImgPool->allocate(mFullImgPoolAllocateNeed);
        timer.stop();
        CAM_ULOGMD("Bokeh FullImg %s %d buf in %d ms", STR_ALLOCATE, mFullImgPoolAllocateNeed,
                timer.getElapsed());
    }

    //Set depi size
    miDmgi = StereoSizeProvider::getInstance()->getBufferSize(
                                                E_DMG, eSTEREO_SCENARIO_PREVIEW).size;
    CAM_ULOGMD("DMGI size(%dx%d) ", miDmgi.w, miDmgi.h);
    //
    if (mDynamicTuningPool != nullptr )
    {
        Timer timer(MTRUE);
        mDynamicTuningPool->allocate(mPipeUsage.getNumP2ATuning());
        timer.stop();
        CAM_ULOGMD("Dynamic Tuning %s %d bufs in %d ms",
                STR_ALLOCATE, mPipeUsage.getNumP2ATuning(), timer.getElapsed());
    }
    //
    if (mbIMG3ODumpBuf) {
        const MINT32 usage = ImageBufferPool::USAGE_HW;
        createBufferPool(mpIMG3OPool, 1920, 1088, eImgFmt_YV12, 2,
                        "BOKEH_P2B_IMG3O", usage, MTRUE);
        mpIMG3OPool->allocate(2);
    }
    //
    if (mbShowDepthMap) {
        const MINT32 usage = ImageBufferPool::USAGE_HW;
        createBufferPool(mpDepthMapBufPool, 720, 408, eImgFmt_Y8, 2,
                        "BOKEH_P2B_DEPTHMAP_BUF", usage, MTRUE);
        mpDepthMapBufPool->allocate(2);
        mpDpStream = new DpBlitStream();
    }
    //
    #if (SUPPORT_ISP_VER == 60)
        mpIspHal_Main1 = MAKE_HalISP(mSensorIndex, "BOKEH_3A_MAIN1");
    #else
        mp3AHal_Main1 = MAKE_Hal3A(mSensorIndex, "BOKEH_3A_MAIN1");
    #endif
    CAM_ULOGMD("3A crate Instance, Main1:%p", mp3AHal_Main1);


    return MTRUE;
}

MBOOL BokehNode::onThreadStop()
{
    CAM_ULOGM_APILIFE();
    this->waitDIPStreamBaseDone();

    return MTRUE;
}

MBOOL BokehNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    DepthImgData depthData;

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mDepthDatas.deque(depthData) )
    {
        CAM_ULOGME("DepthImgData deque out of sync");
        return MFALSE;
    }
    if( depthData.mRequest == nullptr )
    {
        CAM_ULOGME("Request out of sync");
        return MFALSE;
    }

    CAM_ULOGM_APILIFE();

    depthData.mRequest->mTimer.startBokeh();
    BokehEnqueData enqueData;
    Feature::P2Util::DIPParams enqueParams;
    // Check Bokeh Input & hold strong reference
    CAM_ULOGMD("BokehNode: reqID=%d ", depthData.mRequest->mRequestNo);
    //
    enqueData.mRequest  = depthData.mRequest;
    enqueData.mInYuvImg = depthData.mData.mCleanYuvImg;
    enqueData.mDMBG     = depthData.mData.mDMBGImg;
    //
    if (enqueData.mRequest->hasGeneralOutput() &&
        depthData.mData.mDepthSucess == MTRUE)//NonmalCase
    {
        //Gather Input Buffer
        if (depthData.mData.mCleanYuvImg.mBuffer == nullptr) {
            CAM_ULOGME("Bokeh input Master YUV is nullptr!  Can not enque P2!");
            return MFALSE;
        }
        if(depthData.mData.mDMBGImg == nullptr) {
            CAM_ULOGME("Bokeh input DMBG is nullptr!  Can not enque P2!");
            return MFALSE;
        }
        // Prepare DIPParam for P2
        enqueParams.mvDIPFrameParams.push_back(Feature::P2Util::DIPFrameParams());
        enqueParams.mpCookie = (void*)&enqueData;

        Feature::P2Util::DIPFrameParams& mainFrame = enqueParams.mvDIPFrameParams.at(0);
        prepareBokehInput(mainFrame, enqueData.mRequest, enqueData);
        prepareBokehOutputs(mainFrame, enqueData.mRequest, enqueData);

        // set QoS for performance
#if (SUPPORT_ISP_VER == 40)
        struct timeval current;
        gettimeofday(&current, nullptr);
        for (size_t i = 0, n = enqueParams.mvDIPFrameParams.size(); i < n; ++i)
            enqueParams.mvDIPFrameParams.at(i).ExpectedEndTime = current;
#endif
        //
        if (mbDumpDIPParam) debugDIPParams(enqueParams);
        //
        enqueFeatureStream(enqueParams, enqueData);
    }
    else
    { // AbnormalCase
        enqueParams.mDequeSuccess = MFALSE;
        depthData.mRequest->updateResult(enqueParams.mDequeSuccess);
        //
        CAM_ULOGME("reqID=%d:Abnormal Return Buffer Result:%d",
                enqueData.mRequest->mRequestNo, enqueParams.mDequeSuccess);
        handleResultData(enqueData.mRequest, enqueData);
    }


    return MTRUE;
}

MVOID BokehNode::setOutputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mFullImgPool = pool;
    mFullImgPoolAllocateNeed = allocate;

}

MBOOL BokehNode::prepareBokehInput(Feature::P2Util::DIPFrameParams &framePa, const RequestPtr &request,
                                                               BokehEnqueData &data)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    Feature::P2Util::push_in(framePa, NSCam::NSIoPipe::PORT_IMGI,
                                data.mInYuvImg.mBuffer->getImageBufferPtr());

    #if (SUPPORT_ISP_VER == 60)
        Feature::P2Util::push_in(framePa, NSCam::NSIoPipe::PORT_CNR_BLURMAPI,
                                 data.mDMBG->getImageBufferPtr());
    #else
        Feature::P2Util::push_in(framePa, NSCam::NSIoPipe::PORT_DMGI,
                                 data.mDMBG->getImageBufferPtr());
    #endif
    MBOOL bRet = true;

    MSize imgInSize = data.mInYuvImg.mBuffer->getImageBufferPtr()->getImgSize();

    bRet &= setSRZInfo(framePa, EDipModule_SRZ3, miDmgi, imgInSize);
    if (!bRet)
        CAM_ULOGME("setSRZInfo SRZ3 fail");

    SmartTuningBuffer tuningBuf = mDynamicTuningPool->request();
    memset(tuningBuf->mpVA, 0, mDynamicTuningPool->getBufSize());
    if (!StereoTuningProvider::getBokehTuningInfo(tuningBuf->mpVA)) {
        CAM_ULOGME("set Tuning Parameter Fail!");
        return MFALSE;
    }

    ULOGMD_IF(mbPipeLogEnable, "applyIspTuning section + [ISpVer:%d]", SUPPORT_ISP_VER);
#if (SUPPORT_ISP_VER >= 50)
    const SFPSensorInput &masterIn = request->getSensorInput(request->mMasterID);
    const SFPIOMap &generalIO      = request->mSFPIOManager.getFirstGeneralIO();
    //
    _bokehNode_ispTuningConfig_ ispConfig = {
        .pInAppMeta      = masterIn.mAppIn,
        .pInHalMeta      = masterIn.mHalIn,
        .pOutApp         = generalIO.mAppOut,
        .pOutHal         = generalIO.mHalOut,
        .bInputResizeRaw = MTRUE,
        .reqNo           = static_cast<MINT32>(request->mRequestNo),
    };
    #if (SUPPORT_ISP_VER == 60)
        ispConfig.pHalIsp = mpIspHal_Main1;
    #else
        ispConfig.p3AHAL  = mp3AHal_Main1;
    #endif
    //
    trySetMetadata<MINT32>(masterIn.mHalIn, MTK_3A_ISP_BYPASS_LCE, true);
    trySetMetadata<MUINT8>(masterIn.mHalIn, MTK_3A_ISP_PROFILE,
                           NSIspTuning::EIspProfile_Bokeh);
    //
    #if (SUPPORT_ISP_VER == 60)//0:Raw,1:Yuv
        trySetMetadata<MINT32>(masterIn.mHalIn, MTK_ISP_P2_IN_IMG_FMT, 1);
    #endif
    //
    applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig);
#endif
    ULOGMD_IF(mbPipeLogEnable,"applyIspTuning section - ");

    framePa.mStreamTag  = ENormalStreamTag_Bokeh;
    framePa.mTuningData = tuningBuf->mpVA;
    framePa.UniqueKey   = ENormalStreamTag_Bokeh;


    return MTRUE;
}

MBOOL BokehNode::prepareBokehOutputs(Feature::P2Util::DIPFrameParams &framePa, const RequestPtr &request,
                                     BokehEnqueData &bokeh_data)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_FUNC_ENTER();
    //
    MUINT32 sID = request->mMasterID;
    NSCam::NSIoPipe::Output pipeOutput = {0};
    FrameInInfo inInfo;
    getFrameInInfo(inInfo, framePa);
    MSize imgiSize = inInfo.inSize;

    MCrpRsInfo mdp_crop = {}, crz_crop = {};

    P2IO output;
    // Internal Buffer has highest prority, need to push back first.
    if( request->needNextFullImg(this, sID) )
    {
        CAM_ULOGMD("Hook to plugin");
        P2IO out;
        prepareNextFullOut(out, request, bokeh_data, inInfo);
        imgiSize = bokeh_data.mInYuvImg.mBuffer->getImageBuffer()->getImgSize();
        //
        pipeOutput.mPortID = PORT_WDMAO;
        pipeOutput.mBuffer = out.mBuffer;
        // MDP
        mdp_crop.mGroupID   = 2;
        mdp_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h);
        mdp_crop.mResizeDst = imgiSize;
        // CRZ
        crz_crop.mGroupID   = 1;
        crz_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h);
        crz_crop.mResizeDst = imgiSize;

        framePa.mvCropRsInfo.push_back(mdp_crop);
        framePa.mvCropRsInfo.push_back(crz_crop);
        framePa.mvOut.push_back(pipeOutput);
    }
    else
    {
        if (request->popDisplayOutput(this, output))
        {
            pipeOutput.mPortID = PORT_WDMAO;
            pipeOutput.mBuffer = output.mBuffer;

            // MDP
            mdp_crop.mGroupID   = 2;
            mdp_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h);
            mdp_crop.mResizeDst = imgiSize;
            // CRZ
            crz_crop.mGroupID   = 1;
            crz_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h);
            crz_crop.mResizeDst = imgiSize;

            framePa.mvCropRsInfo.push_back(mdp_crop);
            framePa.mvCropRsInfo.push_back(crz_crop);
            framePa.mvOut.push_back(pipeOutput);
            //
            if (mbDumpImgeBuf && mbIMG3ODumpBuf) {
                //////////////////////////////////////////////
                ///Debug Section
                //////////////////////////////////////////////
                ImgBuffer img3o = mpIMG3OPool->requestIIBuffer();
                pipeOutput.mPortID = PORT_IMG3O;
                pipeOutput.mBuffer = img3o->getImageBufferPtr();
                framePa.mvOut.push_back(pipeOutput);
            }
        }

        if (request->popRecordOutput(this, output))
        {
            pipeOutput.mPortID = PORT_WROTO;
            pipeOutput.mBuffer = output.mBuffer;

            // MDP
            mdp_crop.mGroupID   = 3,
            mdp_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h),
            mdp_crop.mResizeDst = imgiSize,
            // CRZ
            crz_crop.mGroupID   = 1,
            crz_crop.mCropRect  = MCropRect(imgiSize.w, imgiSize.h),
            crz_crop.mResizeDst = imgiSize,

            framePa.mvCropRsInfo.push_back(mdp_crop);
            framePa.mvCropRsInfo.push_back(crz_crop);
            framePa.mvOut.push_back(pipeOutput);
        }
    }
    //
    if (mbPipeLogEnable &&
        (request->needDisplayOutput(this) || request->needRecordOutput(this))) {
        CAM_ULOGMD("IMGI size(%dx%d)"
                "P2B MDP setting: GroupId(%d) CropRect(%dx%d)"
                "P2B CRZ setting: GroupId(%d) CropRect(%dx%d)",
                imgiSize.w, imgiSize.h,
                mdp_crop.mGroupID, mdp_crop.mCropRect.s.w, mdp_crop.mCropRect.s.h,
                crz_crop.mGroupID, crz_crop.mCropRect.s.w, crz_crop.mCropRect.s.h);
    }

    if (request->needExtraOutput(this))
    {
        CAM_ULOGME("Not Support");
    }

    if (!bokeh_data.mRemainingOutputs.empty())
        CAM_ULOGME("Not Support Multiple output : RemainSize:%zu",
                bokeh_data.mRemainingOutputs.size());


    return MTRUE;
}

MBOOL BokehNode::needFullForExtraOut(std::vector<P2IO> &outs)
{
    static const MUINT32 maxMDPOut = 2;
    if(outs.size() > maxMDPOut)
        return MTRUE;
    MUINT32 rotCnt = 0;
    for (auto&& out : outs)
    {
        if (out.mTransform != 0)
        {
            ++rotCnt;
        }
    }
    return (rotCnt > 1);
}

MVOID BokehNode::prepareNextFullOut(P2IO &output, const RequestPtr &request,
                                        BokehEnqueData &data, const FrameInInfo &/*inInfo*/)
{
    TRACE_FUNC_ENTER();
    MSize outSize, resize;

    sp<IImageBuffer> pInBuffer = data.mInYuvImg.mBuffer->getImageBuffer();
    outSize = pInBuffer->getImgSize();
    NextFullInfo info;
    data.mOutNextFullImg.mBuffer = request->requestNextFullImg(this, request->mMasterID, info);
    resize = info.mResize;
    if( resize.w && resize.h )
    {
        outSize = resize;
    }
    TRACE_FUNC(" Frame %d QFullImg %s", request->mRequestNo,
                data.mOutNextFullImg.mBuffer == nullptr ? "is null" : "" );

    data.mOutNextFullImg.setAllInfo(data.mInYuvImg);
    data.mOutNextFullImg.mBuffer->getImageBuffer()->setExtParam(outSize);

    output.mBuffer      = data.mOutNextFullImg.mBuffer->getImageBufferPtr();
    output.mTransform   = 0;
    output.mCropRect    = MRect(MPoint(0,0), pInBuffer->getImgSize());
    output.mCropDstSize = outSize;

    data.mOutNextFullImg.accumulate("bokehNFull", request->mLog, pInBuffer->getImgSize(), output.mCropRect, outSize);


}

MVOID BokehNode::prepareFullImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request,
                                BokehEnqueData &data, const FrameInInfo &/*inInfo*/)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d FullImgPool=(%d/%d)", request->mRequestNo,
                    mFullImgPool->peakAvailableSize(), mFullImgPool->peakPoolSize());

    data.mOutFullImg.mBuffer    = mFullImgPool->requestIIBuffer();
    sp<IImageBuffer> pOutBuffer = data.mOutFullImg.mBuffer->getImageBuffer();
    sp<IImageBuffer> pInBuffer  = data.mInYuvImg.mBuffer->getImageBuffer();

    data.mOutFullImg.setAllInfo(data.mInYuvImg);
    if (!pOutBuffer->setExtParam(pInBuffer->getImgSize()))
    {
        CAM_ULOGME("Full Img setExtParm Fail!, target size(%04dx%04d)",
                pInBuffer->getImgSize().w, pInBuffer->getImgSize().h);
    }

    Output output;
    output.mPortID = PortID(EPortType_Memory,  NSImageio::NSIspio::EPortIndex_IMG3O, PORTID_OUT);
    output.mBuffer = pOutBuffer.get();
    frame.mvOut.push_back(output);

}

MVOID BokehNode::enqueFeatureStream(Feature::P2Util::DIPParams &params, BokehEnqueData &data)
{
    TRACE_FUNC_ENTER();
    CAM_ULOGMD("sensor(%d) Frame %d enque start", mSensorIndex, data.mRequest->mRequestNo);
    data.mRequest->mTimer.startEnqueBokeh();
    this->incExtThreadDependency();
    this->enqueDIPStreamBase(mDIPStream, params, data);

}

MVOID BokehNode::onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const BokehEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing BokehNode class members
    TRACE_FUNC_ENTER();

    RequestPtr request = data.mRequest;
    if ( request == nullptr )
    {
        CAM_ULOGME("Missing request");
    }
    else
    {
        this->decExtThreadDependency();

        if (mbDumpImgeBuf || mbShowDepthMap)
            dumpBuff(data.mRequest, params);

        if ( !params.mDequeSuccess )
            CAM_ULOGMW("Frame %d enque result failed", request->mRequestNo);

        request->updateResult(params.mDequeSuccess);
        handleResultData(request, data);

        request->mTimer.stopEnqueBokeh();
        CAM_ULOGMD("sensor(%d) Frame %d enque done in %d ms, result = %d",
                mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueBokeh(),
                params.mDequeSuccess);

        // release malloc buffer
        for (const auto& frameParam : params.mvDIPFrameParams) {
            for (const auto& moduleInfo : frameParam.mvModuleData) {
                if ( moduleInfo.moduleTag == EDipModule_SRZ3 ||
                     moduleInfo.moduleTag == EDipModule_SRZ4)
                {
                    if (moduleInfo.moduleStruct != nullptr)
                        free(moduleInfo.moduleStruct);
                }
            }
        }
        request->mTimer.stopBokeh();
    }


}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID BokehNode::handleResultData(const RequestPtr &request, const BokehEnqueData &data)
{
    // This function is not thread safe,
    // because it is called by onDIPParamsCB,
    // avoid accessing BokehNode class members
    CAM_ULOGM_FUNCLIFE();
    BasicImg full;
    if ( data.mOutNextFullImg.mBuffer != nullptr )
    {
        full = data.mOutNextFullImg;
    }
    else
    {
        full = data.mOutFullImg;
    }

    if( mPipeUsage.supportTPI(TPIOEntry::YUV) ) //ex:Face Beauty
    {
        handleData(ID_BOKEH_TO_VENDOR_FULLIMG, BasicImgData(full, request));
    }
#if 0
    else if( mPipeUsage.supportWarpNode() )
    {
        handleData(ID_P2A_TO_WARP_FULLIMG, BasicImgData(full, request));
    }
#endif
    else //To DISPLAY Preview
    {
        handleData(ID_BOKEH_TO_HELPER, HelperData(FeaturePipeParam::MSG_FRAME_DONE, request));
    }

    if( request->needDump() )
    {
        if( data.mOutFullImg.mBuffer != nullptr )
        {
            data.mOutFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mOutFullImg.mBuffer->getImageBufferPtr(),
                     "BokehNode_full");
        }
        if( data.mOutNextFullImg.mBuffer != nullptr )
        {
            data.mOutNextFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mOutNextFullImg.mBuffer->getImageBufferPtr(),
                     "BokehNode_nextfull");
        }
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MUINT32 BokehNode:: mapToBufferID( NSCam::NSIoPipe::PortID const portId,
                                    const MUINT32 scenarioID)
{
    if (portId == PORT_IMGI)
        return BOKEH_ER_BUF_MAIN1;
    if (portId == PORT_DMGI)
        return BOKEH_ER_BUF_DMG;
    if (portId == PORT_CNR_BLURMAPI)
        return BOKEH_ER_BUF_DMG;
    if (portId == PORT_DEPI)
        return BOKEH_ER_BUF_DMBG;
    if (portId == PORT_IMG3O)
        return BOKEH_ER_BUF_3DNR_OUTPUT;

    if (portId == PORT_WROTO) {
        if(scenarioID == eSTEREO_SCENARIO_CAPTURE)
            return BOKEH_ER_BUF_VSDOF_IMG;
        else if(scenarioID == eSTEREO_SCENARIO_RECORD)
            return BOKEH_ER_BUF_RECORD;
    }
    if (portId == PORT_WDMAO) {
        if (scenarioID == eSTEREO_SCENARIO_CAPTURE)
            return BOKEH_ER_BUF_WDMAIMG;
        if (scenarioID == eSTEREO_SCENARIO_RECORD ||
            scenarioID == eSTEREO_SCENARIO_PREVIEW)
            return BOKEH_ER_BUF_DISPLAY;
    }
    return BOKEH_ER_BUF_START;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL BokehNode::setSRZInfo(Feature::P2Util::DIPFrameParams& frameParam, MINT32 modulTag,
                            MSize inputSize, MSize outputSize)
{
    CAM_TRACE_NAME("BokehNode::setSRZInfo");
    // set SRZ 3
    _SRZ_SIZE_INFO_ *srzInfo = (_SRZ_SIZE_INFO_ *)malloc(sizeof(_SRZ_SIZE_INFO_));
    if(srzInfo == nullptr)
    {
        MY_LOGE("failed to allocate srzInfo");
        delete srzInfo;
        return MFALSE;
    }
    srzInfo->in_w        = inputSize.w;
    srzInfo->in_h        = inputSize.h;
    srzInfo->crop_w      = inputSize.w;
    srzInfo->crop_h      = inputSize.h;
    srzInfo->crop_x      = 0;
    srzInfo->crop_y      = 0;
    srzInfo->out_w       = outputSize.w;
    srzInfo->out_h       = outputSize.h;
    srzInfo->crop_floatX = 0;
    srzInfo->crop_floatY = 0;
    //
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag    = modulTag;
    moduleInfo.frameGroup   = 0;
    moduleInfo.moduleStruct = srzInfo;
    //
    ULOGMD_IF(mbPipeLogEnable, "srz moduleTag (%d) in(%04dx%04d) out(%04dx%04d)",
               modulTag, srzInfo->in_w, srzInfo->in_h, srzInfo->out_w, srzInfo->out_h);

    frameParam.mvModuleData.push_back(moduleInfo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID BokehNode::dumpBuff(RequestPtr reqPtr, const Feature::P2Util::DIPParams& rParams)
{
    CAM_ULOGMD("+");
    MINT32 reqID = reqPtr->mRequestNo;
    if (rParams.mvDIPFrameParams.size()==0) {
        CAM_ULOGME("rParams.mvDIPFrameParams is zero. should not happened");
        return;
    }
    //
    for (MUINT32 i = 0; (i < rParams.mvDIPFrameParams.size()) && (mbPipeLogEnable > 0) ; ++i) {
        CAM_ULOGMD("reqID(%d) in(%zu) out(%zu)", reqID,
                   rParams.mvDIPFrameParams[i].mvIn.size(),
                   rParams.mvDIPFrameParams[i].mvOut.size());
    }
    //
    if (mbShowDepthMap)
    {
        CAM_ULOGMD("ShowDepthMap (%d) +", reqID);
        IImageBuffer *input = nullptr, *output = nullptr;

        for (const auto& frameParam : rParams.mvDIPFrameParams)
        {
            for (const auto& inParam : frameParam.mvIn)
            {
                NSCam::NSIoPipe::PortID portId = inParam.mPortID;
                MUINT32 bufDataType = mapToBufferID(portId, eSTEREO_SCENARIO_PREVIEW);
                if (bufDataType == BOKEH_ER_BUF_DMG) {
                    input = inParam.mBuffer;
                    break;
                }
            }
        }

        for (const auto& frameParam : rParams.mvDIPFrameParams)
        {
            for (const auto& outParam : frameParam.mvOut)
            {
                NSCam::NSIoPipe::PortID portId = outParam.mPortID;
                MUINT32 bufDataType = mapToBufferID(portId, eSTEREO_SCENARIO_PREVIEW);
                if (bufDataType == BOKEH_ER_BUF_DISPLAY) {
                    output = outParam.mBuffer;
                    break;
                }
            }
            // mark cache to invalid, ensure get buffer from memory.
            if (input != nullptr) {
                input->syncCache(eCACHECTRL_INVALID);
                shiftDepthMapValue(input, 2);
                input->syncCache(eCACHECTRL_FLUSH);
                outputDepthMap( input, output);
            } else {
                CAM_ULOGME("Passing null pointer input to syncCache");
            }
        }
        CAM_ULOGMD("ShowDepthMap -");
    }

    if (mbDumpImgeBuf) // dump out port
    {
        CAM_ULOGMD("Dump image(%d) +", reqID);
        const SFPSensorInput &masterSensorIn = reqPtr->getSensorInput(reqPtr->mMasterID);
        IMetadata* pMeta_InHal = masterSensorIn.mHalIn;
        //
        const size_t PATH_SIZE    = 1024;
        char writepath[PATH_SIZE] = {0};
        char filename[PATH_SIZE]  = {0};
        extract(&mDumpHint, pMeta_InHal);

        for (const auto& frameParam : rParams.mvDIPFrameParams)
        {
            for (const auto& outParam : frameParam.mvOut)
            {
                NSCam::NSIoPipe::PortID portId = outParam.mPortID;
                IImageBuffer* buf = outParam.mBuffer;
                if (buf != nullptr) {
                    extract(&mDumpHint, buf);
                    if (portId == PORT_WDMAO) {
                        snprintf(filename, PATH_SIZE, "%s_%lu_reqID_%d", "P2B_DISPLAY",
                                 buf->getBufStridesInBytes(0), reqID);
                    } else if (portId == PORT_WROTO) {
                        snprintf(filename, PATH_SIZE, "%s_%lu_reqID_%d", "P2B_VSDOF",
                                 buf->getBufStridesInBytes(0), reqID);
                    } else if (portId == PORT_IMG3O) {
                        snprintf(filename, PATH_SIZE, "%s_%lu_reqID_%d", "P2B_IMG3O",
                                 buf->getBufStridesInBytes(0), reqID);
                    }
                    genFileName_YUV(writepath, PATH_SIZE, &mDumpHint,
                            TuningUtils::YUV_PORT_WDMAO, filename);
                    //
                    buf->saveToFile(writepath);
                } else {
                    CAM_ULOGME("outParam.mBuffer is nullptr, something wrong");
                }
            }

            for (const auto& inParam : frameParam.mvIn)
            {
                NSCam::NSIoPipe::PortID portId = inParam.mPortID;
                if (portId == PORT_DMGI || portId == PORT_CNR_BLURMAPI) {
                    IImageBuffer* buf = inParam.mBuffer;
                    if (buf != nullptr) {
                        extract(&mDumpHint, buf);
                        //
                        snprintf(filename, PATH_SIZE, "%s_%lu_reqID_%d",
                                 (portId == PORT_DMGI ?  "P2B_DMBG" : "P2B_CNR_BLURMAP"),
                                 buf->getBufStridesInBytes(0), reqID);
                        //
                        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint,
                                        TuningUtils::YUV_PORT_UNDEFINED, filename);
                        //
                        buf->saveToFile(writepath);
                    } else {
                        CAM_ULOGME("inParam.mBuffer is nullptr, something wrong");
                    }
                }
            }
        }
        CAM_ULOGMD("Dump image -");
    }
    CAM_ULOGMD("-");
}

//************************************************************************
//
//************************************************************************
MVOID BokehNode:: outputDepthMap( IImageBuffer* depthMap, IImageBuffer* displayResult)
{
    if (displayResult == nullptr || displayResult->getPlaneCount() != 3)
        return;
    if (mpDpStream == nullptr)
        return;
    //
    SmartImageBuffer mdpBuffer = mpDepthMapBufPool->request();
    //
    sMDP_Config config = {
        .pDpStream  = mpDpStream,
        .pSrcBuffer = depthMap,
        .pDstBuffer = mdpBuffer->mImageBuffer.get(),
        .rotAngle   = 0,
    };
    if(!excuteMDP(config)) {
        CAM_ULOGME("excuteMDP fail.");
        return;
    }
    //
    MSize outImgSize = displayResult->getImgSize();
    MSize inImgSize  = mdpBuffer->mImageBuffer->getImgSize();
    char* outAddr0   = (char*)displayResult->getBufVA(0);
    char* outAddr1   = (char*)displayResult->getBufVA(1);
    char* outAddr2   = (char*)displayResult->getBufVA(2);
    char* inAddr     = (char*)mdpBuffer->mImageBuffer->getBufVA(0);
    MINT32 halfInWidth  = inImgSize.w >> 1;
    MINT32 halfInHeight = inImgSize.h >> 1;
    MINT32 halfOutWidth = outImgSize.w  >> 1;
    //
    for (int i = 0; i < inImgSize.h; ++i) {
        memcpy(outAddr0, inAddr, inImgSize.w);
        outAddr0 += outImgSize.w;
        inAddr += inImgSize.w;
    }
    //
    for (int i = 0 ; i < halfInHeight; ++i) {
        memset(outAddr1, 128, halfInWidth);
        memset(outAddr2, 128, halfInWidth);
        outAddr1 += halfOutWidth;
        outAddr2 += halfOutWidth;
    }
    mdpBuffer = nullptr;
    return;
}

//************************************************************************
//
//************************************************************************
MVOID BokehNode:: shiftDepthMapValue(IImageBuffer* depthMap, MUINT8 shiftValue)
{
    // offset value
    MUINT8* data = (MUINT8*)depthMap->getBufVA(0);
    MSize const size = depthMap->getImgSize();
    for (int i = 0; i < size.w*size.h ; ++i) {
        *data = *data << shiftValue;
        *data = (MUINT8)std::max(0, std::min((int)*data, 255));
        data++;
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL BokehNode:: createBufferPool(android::sp<ImageBufferPool> &pPool,
    MUINT32 width, MUINT32 height, NSCam::EImageFormat format,
    MUINT32 bufCount, const char* caller, MUINT32 bufUsage, MBOOL continuesBuffer)
{
    CAM_ULOGM_FUNCLIFE();
    MBOOL ret = MFALSE;
    pPool = ImageBufferPool::create(caller, width, height, format, bufUsage, continuesBuffer);
    if(pPool == nullptr)
    {
        ret = MFALSE;
        CAM_ULOGME("Create [%s] failed.", caller);
        goto lbExit;
    }
    for(MUINT32 i=0;i<bufCount;++i)
    {
        if(!pPool->allocate())
        {
            ret = MFALSE;
            CAM_ULOGME("Allocate [%s] working buffer failed.", caller);
            goto lbExit;
        }
    }
    ret = MTRUE;
lbExit:
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
TuningParam BokehNode:: applyISPTuning(const char* name, SmartTuningBuffer& targetTuningBuf,
    const _bokehNode_ispTuningConfig_& ispConfig)
{
    CAM_ULOGMD("+, [%s] reqID=%d bIsResized=%d", name, ispConfig.reqNo, ispConfig.bInputResizeRaw);

    NS3Av3::TuningParam tuningParam = {nullptr};
    tuningParam.pRegBuf = reinterpret_cast<void*>(targetTuningBuf->mpVA);

    MetaSet_T inMetaSet(*ispConfig.pInAppMeta, *ispConfig.pInHalMeta);
    MetaSet_T outMetaSet(*ispConfig.pOutApp  , *ispConfig.pOutHal);
    MetaSet_T resultMeta;

    MUINT8 profile = -1;
    if (tryGetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, profile))
        CAM_ULOGMD("Profile:%d", profile);
    else
        CAM_ULOGMW("Failed getting profile!");

    // USE resize raw-->set PGN 0
    if (ispConfig.bInputResizeRaw)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);

    #if (SUPPORT_ISP_VER == 60)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_ISP_P2_TUNING_UPDATE_MODE, 1);
        ispConfig.pHalIsp->setP2Isp(0, inMetaSet, &tuningParam, &outMetaSet);
    #else
        ispConfig.p3AHAL->setIsp(0, inMetaSet, &tuningParam, &outMetaSet);
    #endif

    { // write ISP resultMeta to input hal Meta
        *(ispConfig.pInHalMeta) += outMetaSet.halMeta;
    }
    CAM_ULOGMD("-, [%s] reqID=%d", name, ispConfig.reqNo);
    return tuningParam;
}
/*******************************************************************************
 *
 ********************************************************************************/
template <typename T>
inline MVOID trySetMetadata(IMetadata* pMetadata, MUINT32 const tag, T const& val)
{
    if( pMetadata == nullptr ) {
        CAM_ULOGMW("pMetadata == nullptr");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <typename T>
inline MBOOL tryGetMetadata(IMetadata* pMetadata, MUINT32 const tag, T & rVal)
{
    if( pMetadata == nullptr ) {
        CAM_ULOGMW("pMetadata == nullptr");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

template <typename T>
inline MVOID updateEntry(IMetadata* pMetadata, MUINT32 const tag, T const& val)
{
    if( pMetadata == nullptr ) {
        CAM_ULOGMW("pMetadata == nullptr");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

MVOID debugDIPParams(const Feature::P2Util::DIPParams& rInputDIPParam)
{
    CAM_ULOGMD("Frame size = %zu", rInputDIPParam.mvDIPFrameParams.size());

    for (size_t index = 0; index < rInputDIPParam.mvDIPFrameParams.size(); ++index)
    {
        auto& frameParam = rInputDIPParam.mvDIPFrameParams.at(index);
        CAM_ULOGMD("=================================================\n"
                   "Frame index = %zu\n"
                   "mStreamTag=%d mSensorIdx=%d\n"
                   "Frame Expected End Time (tv_sec) = %ld\n"
                   "Frame Expected End Time (tv_usec) = %ld\n"
                   "=== mvIn section ==="
                   , index, frameParam.mStreamTag, frameParam.mSensorIdx
                   , frameParam.ExpectedEndTime.tv_sec
                   , frameParam.ExpectedEndTime.tv_usec);

        for (size_t index2=0;index2<frameParam.mvIn.size();++index2)
        {
            Input data = frameParam.mvIn[index2];
            CAM_ULOGMD("Index = %zu\n"
                       "mvIn.PortID.index = %d\n"
                       "mvIn.PortID.type = %d\n"
                       "mvIn.PortID.inout = %d\n"
                       "mvIn.mBuffer=%p "
                        , index2 , data.mPortID.index
                        , data.mPortID.type , data.mPortID.inout
                        , data.mBuffer);
            if (data.mBuffer !=nullptr)
            {
                CAM_ULOGMD("mvIn.mBuffer->getImgSize = %dx%d\n"
                           "mvIn.mBuffer->getImgFormat = %x\n"
                           "mvIn.mBuffer->getPlaneCount = %zu\n "
                            , data.mBuffer->getImgSize().w
                            , data.mBuffer->getImgSize().h
                            , data.mBuffer->getImgFormat()
                            , data.mBuffer->getPlaneCount());
                for (size_t j=0; j<data.mBuffer->getPlaneCount(); j++)
                {
                    CAM_ULOGMD("mvIn.mBuffer->getBufVA(%zu) = %lX\n"
                               "mvIn.mBuffer->getBufStridesInBytes(%zu) = %zu"
                                , j, data.mBuffer->getBufVA(j)
                                , j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                CAM_ULOGMD("mvIn.mBuffer is nullptr!!");
            }
            CAM_ULOGMD("mvIn.mTransform = %d", data.mTransform);
        }

        CAM_ULOGMD("=== mvOut section ===");
        for (size_t index2=0;index2<frameParam.mvOut.size(); index2++)
        {
            Output data = frameParam.mvOut[index2];
            CAM_ULOGMD("Index = %zu\n"
                       "mvOut.PortID.index = %d\n"
                       "mvOut.PortID.type = %d\n"
                       "mvOut.PortID.inout = %d\n"
                       "mvOut.mBuffer=%p "
                        , index2
                        , data.mPortID.index , data.mPortID.type , data.mPortID.inout
                        , data.mBuffer);
            if (data.mBuffer != nullptr)
            {
                CAM_ULOGMD("mvOut.mBuffer->getImgSize = %dx%d\n"
                           " mvOut.mBuffer->getImgFormat = %x\n"
                           " mvOut.mBuffer->getPlaneCount = %zu"
                            , data.mBuffer->getImgSize().w
                            , data.mBuffer->getImgSize().h
                            , data.mBuffer->getImgFormat()
                            , data.mBuffer->getPlaneCount());
                for (size_t j=0;j<data.mBuffer->getPlaneCount();j++)
                {
                    CAM_ULOGMD("mvOut.mBuffer->getBufVA(%zu) = %lX\n"
                               "mvOut.mBuffer->getBufStridesInBytes(%zu) = %zu "
                                , j, data.mBuffer->getBufVA(j)
                                , j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                CAM_ULOGMD("mvOut.mBuffer is nullptr!!");
            }
            CAM_ULOGMD("mvOut.mTransform = %d", data.mTransform);
        }

        CAM_ULOGMD("=== mvCropRsInfo section ===");
        for (size_t i=0;i<frameParam.mvCropRsInfo.size(); i++)
        {
            MCrpRsInfo data = frameParam.mvCropRsInfo[i];
            CAM_ULOGMD("Index = %zu\n"
                       "CropRsInfo.mGroupID=%d\n"
                       "CropRsInfo.mResizeDst=%dx%d\n"
                       "CropRsInfo.mCropRect.p_fractional=(%d,%d) \n"
                       "CropRsInfo.mCropRect.p_integral=(%d,%d) \n"
                       "CropRsInfo.mCropRect.s=%dx%d "
                        , i
                        , data.mGroupID
                        , data.mResizeDst.w, data.mResizeDst.h
                        , data.mCropRect.p_fractional.x, data.mCropRect.p_fractional.y
                        , data.mCropRect.p_integral.x, data.mCropRect.p_integral.y
                        , data.mCropRect.s.w, data.mCropRect.s.h);
        }

        CAM_ULOGMD("=== mvModuleData section ===");
        for (size_t i=0;i<frameParam.mvModuleData.size(); i++)
        {
            ModuleInfo data = frameParam.mvModuleData[i];
            CAM_ULOGMD("Index = %zu\nModuleData.moduleTag=%d", i, data.moduleTag);

            _SRZ_SIZE_INFO_ *SrzInfo = (_SRZ_SIZE_INFO_ *) data.moduleStruct;
            CAM_ULOGMD("SrzInfo->in_w=%d\n"
                       "SrzInfo->in_h=%d\n"
                       "SrzInfo->crop_w=%lu\n"
                       "SrzInfo->crop_h=%lu\n"
                       "SrzInfo->crop_x=%d\n"
                       "SrzInfo->crop_y=%d\n"
                       "SrzInfo->crop_floatX=%d\n"
                       "SrzInfo->crop_floatY=%d\n"
                       "SrzInfo->out_w=%d\n"
                       "SrzInfo->out_h=%d "
                        , SrzInfo->in_w
                        , SrzInfo->in_h
                        , SrzInfo->crop_w
                        , SrzInfo->crop_h
                        , SrzInfo->crop_x
                        , SrzInfo->crop_y
                        , SrzInfo->crop_floatX
                        , SrzInfo->crop_floatY
                        , SrzInfo->out_w
                        , SrzInfo->out_h);
        }
        CAM_ULOGMD("TuningData=%p\n=== mvExtraData section ===", frameParam.mTuningData);
        for (size_t i=0;i<frameParam.mvExtraParam.size(); i++)
        {
            auto extraParam = frameParam.mvExtraParam[i];
            if (extraParam.CmdIdx == EPIPE_FE_INFO_CMD)
            {
                FEInfo *feInfo = (FEInfo*) extraParam.moduleStruct;
                CAM_ULOGMD("mFEDSCR_SBIT=%d  mFETH_C=%d  mFETH_G=%d\n"
                           "mFEFLT_EN=%d  mFEPARAM=%d  mFEMODE=%d\n"
                           "mFEYIDX=%d  mFEXIDX=%d  mFESTART_X=%d\n"
                           "mFESTART_Y=%d  mFEIN_HT=%d  mFEIN_WD=%d"
                            , feInfo->mFEDSCR_SBIT, feInfo->mFETH_C , feInfo->mFETH_G
                            , feInfo->mFEFLT_EN   , feInfo->mFEPARAM, feInfo->mFEMODE
                            , feInfo->mFEYIDX     , feInfo->mFEXIDX , feInfo->mFESTART_X
                            , feInfo->mFESTART_Y  , feInfo->mFEIN_HT, feInfo->mFEIN_WD);

            }
            else if (extraParam.CmdIdx == EPIPE_FM_INFO_CMD)
            {
                FMInfo *fmInfo = (FMInfo*) extraParam.moduleStruct;
                CAM_ULOGMD("mFMHEIGHT=%d  mFMWIDTH=%d  mFMSR_TYPE=%d\n"
                           "mFMOFFSET_X=%d  mFMOFFSET_Y=%d  mFMRES_TH=%d\n"
                           "mFMSAD_TH=%d  mFMMIN_RATIO=%d"
                           , fmInfo->mFMHEIGHT, fmInfo->mFMWIDTH, fmInfo->mFMSR_TYPE
                           , fmInfo->mFMOFFSET_X, fmInfo->mFMOFFSET_Y, fmInfo->mFMRES_TH
                           , fmInfo->mFMSAD_TH, fmInfo->mFMMIN_RATIO);
            }
            else if(extraParam.CmdIdx == EPIPE_MDP_PQPARAM_CMD)
            {
                PQParam* param = reinterpret_cast<PQParam*>(extraParam.moduleStruct);
                if (param->WDMAPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WDMAPQParam;
                    DpIspParam& ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    CAM_ULOGMD("WDMAPQParam %p enable = %d, scenario=%d\n"
                               "WDMAPQParam iso = %d, frameNo=%d requestNo=%d\n"
                               "WDMAPQParam lensId = %d, isRefocus=%d defaultUpTable=%d\n"
                               "WDMAPQParam defaultDownTable = %d, IBSEGain=%d"
                               , dpPqParam, dpPqParam->enable, dpPqParam->scenario
                               , ispParam.iso , ispParam.frameNo, ispParam.requestNo
                               , ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable
                               , vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }
                if (param->WROTPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WROTPQParam;
                    DpIspParam&ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    CAM_ULOGMD("WROTPQParam %p enable = %d, scenario=%d\n"
                               "WROTPQParam iso = %d, frameNo=%d requestNo=%d\n"
                               "WROTPQParam lensId = %d, isRefocus=%d defaultUpTable=%d\n"
                               "WROTPQParam defaultDownTable = %d, IBSEGain=%d"
                               , dpPqParam, dpPqParam->enable, dpPqParam->scenario
                               , ispParam.iso , ispParam.frameNo, ispParam.requestNo
                               , ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable
                               , vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }
            }
        }
    }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
//=======================================================================================
#else //SUPPORT_VSDOF
//=======================================================================================

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
//=======================================================================================
#endif //SUPPORT_VSDOF
//=======================================================================================
