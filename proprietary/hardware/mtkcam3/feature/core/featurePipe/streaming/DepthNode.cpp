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

#include <bitset>
#include "DepthNode.h"
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam3/feature/eis/eis_hal.h>
#include <mtkcam3/feature/DualCam/FOVHal.h>

#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>

#define PIPE_CLASS_TAG "DepthNode"
#define PIPE_TRACE TRACE_SFP_DEPTH_NODE
#include <featurePipe/core/include/PipeLog.h>
//=======================================================================================
#if SUPPORT_VSDOF
//=======================================================================================
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_DEPTH);

#define ULOGMD_IF(cond, ...)       do { if ( (cond) ) { CAM_ULOGMD(__VA_ARGS__); } } while(0)
// using namespace NSFeaturePipe_DepthMap::IDepthMapEffectRequest;
using namespace NSCam::NSIoPipe::NSPostProc;
using NSCam::Feature::P2Util::P2SensorData;
using NSFeaturePipe_DepthMap::IDepthMapEffectRequest;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;

DepthNode::DepthNode(const char *name)
    : StreamingFeatureNode(name)
{
    CAM_ULOGM_APILIFE();
    this->addWaitQueue(&mRequests);
}

DepthNode::~DepthNode()
{
    CAM_ULOGM_APILIFE();
}

MBOOL DepthNode::onData(DataID id, const RequestPtr &data)
{
    CAM_ULOGM_APILIFE();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    switch ( id ) {
    case ID_ROOT_TO_DEPTH:
        mRequests.enque(data);
        ret = MTRUE;
        break;
    default:
        ret = MFALSE;
        break;
    }
    return ret;
}

IOPolicyType DepthNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo& /*reqInfo*/) const
{
    IOPolicyType policy = IOPOLICY_INOUT;
    return policy;
}

MBOOL DepthNode::onInit()
{
    CAM_ULOGM_APILIFE();
    StreamingFeatureNode::onInit();
    //Check ISP_VER
    #if (SUPPORT_ISP_VER == 0)
        CAM_ULOGME("ISP_VER is unknown!! Abnormal!!");
        return MFALSE;
    #endif
    //depthmap pipe section
    std::vector<MUINT32> ids = mPipeUsage.getAllSensorIDs();
    sp<DepthMapPipeSetting> pPipeSetting = new DepthMapPipeSetting();
    pPipeSetting->miSensorIdx_Main1 = ids.at(MAINCAM);
    pPipeSetting->miSensorIdx_Main2 = ids.at(SUBCAM);
    //
    mSensorID.assign(ids.begin(), ids.end());
    // get main1 size by index.
    // main1 size usually put in first.
    pPipeSetting->mszRRZO_Main1 = mPipeUsage.getRrzoSizeByIndex(ids.at(MAINCAM));
    CAM_ULOGMD("SensorID:Main1(%d), Main2(%d)|"
               " RRZO_Main1 Create Size(%04dx%04d)|"
               " Assign:(Main1,Main2)(%d,%d)",
               ids.at(MAINCAM), ids.at(SUBCAM),
               pPipeSetting->mszRRZO_Main1.w, pPipeSetting->mszRRZO_Main1.h,
               mSensorID.at(MAINCAM), mSensorID.at(SUBCAM));
    //pipe option
    sp<DepthMapPipeOption> pPipeOption = new DepthMapPipeOption();
    pPipeOption->mFlowType   = eDEPTH_FLOW_TYPE_QUEUED_DEPTH;
    pPipeOption->mSensorType = (SeneorModuleType)mPipeUsage.getSensorModule();
    if(mPipeUsage.supportDPE() && mPipeUsage.supportBokeh())
    {
        pPipeOption->mFeatureMode = eDEPTHNODE_MODE_VSDOF;
    }
    else if(mPipeUsage.supportDPE())
    {
        pPipeOption->mFeatureMode = eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH;
        if(StereoSettingProvider::getDepthmapRefineLevel() == E_DEPTHMAP_REFINE_SW_OPTIMIZED)
            pPipeOption->mFeatureMode = eDEPTHNODE_MODE_MTK_DEPTH;
    }
    else
    {
        CAM_ULOGME("not support");
        return MFALSE;
    }

    pPipeOption->setEnableDepthGenControl(MFALSE, 0);
    //
    #if (SUPPORT_ISP_VER >= 60)
        mpDepthMapPipe = DepthPipeHolder::createPipe(pPipeSetting, pPipeOption);
    #else
        mpDepthMapPipe = IDepthMapPipe::createInstance(pPipeSetting, pPipeOption);
        MBOOL bRet = mpDepthMapPipe->init();
        if (!bRet) {
            CAM_ULOGME("onInit Failure, ret=%d", bRet);
            return MFALSE;
        }
    #endif

    // Create Depth or DMBG buffer pool
    if(mPipeUsage.supportBokeh())
    {
        mDMBGImgPoolAllocateNeed = 3;
        MSize dmbgSize = StereoSizeProvider::getInstance()->getBufferSize(
                                                        E_DMBG, eSTEREO_SCENARIO_PREVIEW);
        mDMBGImgPool = ImageBufferPool::create("fpipe.DMBGImg", dmbgSize.w, dmbgSize.h,
                                        eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW_AND_SW);
    }
    else
    {
        mDepthMapImgPoolAllocateNeed = 3;
        MSize depthSize = StereoSizeProvider::getInstance()->getBufferSize(
                                                    E_DEPTH_MAP, eSTEREO_SCENARIO_PREVIEW);
        mDepthMapImgPool = ImageBufferPool::create("fpipe.DepthMapImg", depthSize.w, depthSize.h,
                                           eImgFmt_Y8, ImageBufferPool::USAGE_HW_AND_SW );
    }
    //
    miLogEnable = property_get_int32("vendor.debug.vsdof.tkflow.depthnode", 0);

    CAM_ULOGMD("RunningMode:%s, IspVer:%d",
            pPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF ?
            "ALL TK Flow" : "TK Depth+TP Bokeh", SUPPORT_ISP_VER);
    return MTRUE;
}

MBOOL DepthNode::onUninit()
{
    CAM_ULOGM_APILIFE();
    //depthmap pipe section
    if (mpDepthMapPipe != nullptr) {
        if (0 != mvDepthNodePack.size()) {
            CAM_ULOGME("DepthNodePack should be release before uninit, size:%zu",
                    mvDepthNodePack.size());
            mvDepthNodePack.clear();
        }
        //
        #if (SUPPORT_ISP_VER != 60)
        {
            mpDepthMapPipe->uninit();
            delete mpDepthMapPipe;
            mpDepthMapPipe = nullptr;
        }
        #endif
    }
    //
    IBufferPool::destroy(mDMBGImgPool);
    IBufferPool::destroy(mDepthMapImgPool);
    mpDepthMapPipe = nullptr;
    return MTRUE;
}

MVOID DepthNode::onFlush()
{
    CAM_ULOGM_APILIFE();
    mpDepthMapPipe->sync();

}

MBOOL DepthNode::onThreadStart()
{
    CAM_ULOGM_APILIFE();
    if( mYuvImgPoolAllocateNeed && mYuvImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mYuvImgPool->allocate(mYuvImgPoolAllocateNeed);
        timer.stop();
        CAM_ULOGMD("mDepthYUVImg %s %d buf in %d ms", STR_ALLOCATE, mYuvImgPoolAllocateNeed,
                                                   timer.getElapsed());
    }

    if( mDMBGImgPoolAllocateNeed && mDMBGImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mDMBGImgPool->allocate(mDMBGImgPoolAllocateNeed);
        timer.stop();
        CAM_ULOGMD("mDMBGImg %s %d buf in %d ms", STR_ALLOCATE, mDMBGImgPoolAllocateNeed,
                                               timer.getElapsed());
    }

    if( mDepthMapImgPoolAllocateNeed && mDepthMapImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mDepthMapImgPool->allocate(mDepthMapImgPoolAllocateNeed);
        timer.stop();
        CAM_ULOGMD("mDepthMapImg %s %d buf in %d ms", STR_ALLOCATE, mDepthMapImgPoolAllocateNeed,
                                                   timer.getElapsed());
    }
    return MTRUE;
}

MBOOL DepthNode::onThreadStop()
{
    CAM_ULOGM_APILIFE();
    return MTRUE;
}

MBOOL DepthNode::onThreadLoop()
{
    CAM_ULOGMD("Waitloop");
    RequestPtr request;

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        CAM_ULOGME("Request deque out of sync");
        return MFALSE;
    }
    else if( request == NULL )
    {
        CAM_ULOGME("Request out of sync");
        return MFALSE;
    }

    CAM_ULOGM_APILIFE();
    //
    MUINT32 decision = 0;
    request->mTimer.startDepth();
    DepthEnqueData enqueData;
    MINT64 timestamp = 0;
    enqueData.mRequest = request;
    // Output : CleanYUV, FD, DMBG or DepthMap, App/Hal OutMeta
    SFPIOManager &ioMgr       = request->mSFPIOManager;
    const SFPIOMap &generalIO = ioMgr.getFirstGeneralIO();
    P2IO out;
    // create Request
    sp<IDepthMapEffectRequest> pDepMapReq = IDepthMapEffectRequest::createInstance(
                                                request->mRequestNo, onPipeReady, this);
    { // InputSection :: RRZO, LCSO, HalIn, AppIn Meta
        const SFPSensorInput &masterSensorIn = request->getSensorInput(request->mMasterID);
        const SFPSensorInput &slaveSensorIn  = request->getSensorInput(request->mSlaveID);
        if(get(masterSensorIn.mRRZO) != NULL)
        {
            timestamp = get(masterSensorIn.mRRZO)->getTimestamp();
        }
        //
        std::vector<MUINT32> ids = mPipeUsage.getAllSensorIDs();
        //Gather Main1 necessary input data
        decision = setInputData(ids.at(MAINCAM), pDepMapReq, masterSensorIn);
        if (decision != 0) {
            CAM_ULOGME("reqID=%d:In Main1  with ERROR index(%#x),Abnormal",
                      enqueData.mRequest->mRequestNo, decision);
            goto ABNORMAL;
        }
        //
        decision = setInputData(ids.at(SUBCAM), pDepMapReq, slaveSensorIn);
        if (decision != 0) {//Gather Main2 necessary data
            CAM_ULOGMW("reqID=%d:In Main2 with missing data, Abnormal",
                    enqueData.mRequest->mRequestNo);//Misin main2 data still can enque(stanalone)
            if (SUPPORT_ISP_VER < 50) {
                CAM_ULOGMW("VSDOF Version doesn't support Stanalone Mode, !!Not Enque!!");
                goto ABNORMAL;
            } else {
                decision = 0;//Still enque if missing Main2. Stanalone.
            }
        }
    }
    // OutputSection :: FD
    if (request->popFDOutput(this, out)) {
        pDepMapReq->pushRequestImageBuffer(
                    {PBID_OUT_FDIMG , eBUFFER_IOTYPE_OUTPUT}, out.mBuffer);
    }
    // Assume clean yuv no crop & scale
    enqueData.mOut.mCleanYuvImg.mSensorClipInfo = request->getSensorClipInfo(request->mMasterID);

    if (request->needFullImg(this, request->mMasterID)) {
        enqueData.mOut.mCleanYuvImg.mBuffer = mYuvImgPool->requestIIBuffer(); //CleanYUV
    }
    else if (request->needNextFullImg(this, request->mMasterID))//customize input yuv for next node
    {

        MSize rrzo_size = get(request->getSensorInput(request->mMasterID).mRRZO)->getImgSize();
        NextFullInfo info;
        enqueData.mOut.mCleanYuvImg.mBuffer =
                        request->requestNextFullImg(this, request->mMasterID, info);//CleanYUV
        MSize &resize = info.mResize;
        if (resize.w && resize.h) {// if assigned
            enqueData.mOut.mCleanYuvImg.mBuffer->getImageBuffer()->setExtParam(resize);
            MRectF crop((float)rrzo_size.w, (float)rrzo_size.h);
            enqueData.mOut.mCleanYuvImg.accumulate(
                "depthNFull", request->mLog, rrzo_size, crop, resize);
        } else {//not assign. set output size as RRZO size
            if (rrzo_size.w && rrzo_size.h)
                enqueData.mOut.mCleanYuvImg.mBuffer->getImageBuffer()->setExtParam(rrzo_size);
        }
        CAM_ULOGMD("Prepare Size as %s:WxH(%04dx%04d)",
                   (resize.w && resize.h) ? "customize": "RRZO",
                   (resize.w && resize.h) ? resize.w: rrzo_size.w,
                   (resize.w && resize.h) ? resize.h: rrzo_size.h);
    }
    else if (request->needDisplayOutput(this) && request->popDisplayOutput(this, out))
    {
        enqueData.mOut.mCleanYuvImg.mBuffer = new IIBuffer_IImageBuffer(out.mBuffer);
    }
    else
    {
        CAM_ULOGME("**Exception Case** For CleanYuv");
    }

    if(enqueData.mOut.mCleanYuvImg.mBuffer != NULL)
    {
        enqueData.mOut.mCleanYuvImg.mBuffer->getImageBufferPtr()->setTimestamp(timestamp);
    }
    //
    if (request->hasGeneralOutput())//Gather Output
    {
        decision = 0;
        decision = setOutputData(pDepMapReq, enqueData.mOut, generalIO);
        if (decision != 0) {
            CAM_ULOGME("reqID=%d:Output with ERROR index(%#x),Abnormal, !!Not Enque!!",
                    enqueData.mRequest->mRequestNo, decision);
            goto ABNORMAL;
        }
    } else {
        CAM_ULOGME("NOT GeneralOut,Error State(%#x)",(decision |= (1<<14)));//0x4000
        goto ABNORMAL;
    }
    //
    if(decision != 0)
        CAM_ULOGMD("reqID=%d,decision=%#x", enqueData.mRequest->mRequestNo, decision);
    //
ABNORMAL:
    if (request->hasGeneralOutput() && decision == 0) {
        if (enqueData.mRequest->mRequestNo < 0) {
            CAM_ULOGME("[DepthNode]reqID=%d released, something wrong !!",
                                        enqueData.mRequest->mRequestNo);
            return MFALSE;
        } else {//hold enqueData members,
                //because its member hold strong pointer of CleanYUV / DMBG / Depth buffers.
            this->incExtThreadDependency(); // record one request enque to depth pipe.

            android::Mutex::Autolock lock(mLock);
            CAM_ULOGMD("thread_loop reqID=%d timestamp(%ld)", enqueData.mRequest->mRequestNo, (long)timestamp);
            //For DepthNodePack, it consolidates DepthEnqueData & IDepthMapEffectRequest
            DepthNodePackage pack = {
                .depEnquePack  = make_shared<DepthEnqueData>(enqueData),
                .depEffectPack = pDepMapReq,
            };
            mvDepthNodePack.add(enqueData.mRequest->mRequestNo, pack);
        }
        enqueData.mRequest->mTimer.startEnqueDepth();
        mpDepthMapPipe->enque(pDepMapReq);
    } else { //AbnormalCase
        CAM_ULOGME("!!!Bypass DepthMapPipe!!! Something Wrong!");
        enqueData.mOut.mDepthSucess = MFALSE;

        handleResultData(enqueData.mRequest, enqueData);// send data to next nodes
        //
        enqueData.mRequest->mTimer.stopEnqueDepth();
        //Not need to decExtThreadDependency due to this case is exception
        enqueData.mRequest->mTimer.stopDepth();
    }

    return MTRUE;
}

MUINT32 DepthNode::setOutputData(sp<IDepthMapEffectRequest> pDepMapReq,
                                 DepthImg& out, const SFPIOMap &generalIO)
{
    if(miLogEnable > 0) CAM_ULOGM_FUNCLIFE();
    std::bitset<13> decision;

    if (out.mCleanYuvImg.mBuffer.get() != nullptr) {
        sp<IImageBuffer> spCleanYuvImg = out.mCleanYuvImg.mBuffer->getImageBuffer();
        if (!pDepMapReq->pushRequestImageBuffer(
                    {PBID_OUT_MV_F, eBUFFER_IOTYPE_OUTPUT}, spCleanYuvImg))
            decision.set(8); //0x100, 256
    } else
        decision.set(9);     //0x200, 512

    if (mPipeUsage.supportBokeh())
    {
        out.mDMBGImg = mDMBGImgPool->requestIIBuffer();//DMBG

        sp<IImageBuffer> spDMBGImg = out.mDMBGImg->getImageBuffer();
        if (!pDepMapReq->pushRequestImageBuffer(
                    {PBID_OUT_DMBG , eBUFFER_IOTYPE_OUTPUT}, spDMBGImg)) {
            decision.set(10);//0x400, 1024
        }
    }
    else
    {
        out.mDepthMapImg = mDepthMapImgPool->requestIIBuffer();//DepthMap

        sp<IImageBuffer> spDepthImg = out.mDepthMapImg->getImageBuffer();
        if (!pDepMapReq->pushRequestImageBuffer(
                    {PBID_OUT_DEPTHMAP, eBUFFER_IOTYPE_OUTPUT}, spDepthImg)) {
            decision.set(11);//0x800, 2048
        }
    }
    //MetaData Output
    std::shared_ptr<IMetadata> pCopyHalOutMeta = nullptr;
    IMetadata* pPushedHalOutMeta = generalIO.mHalOut;
    if(pPushedHalOutMeta == nullptr)
    {
        pCopyHalOutMeta = std::make_shared<IMetadata>();    // create a new hal out meta
        out.mpCopyHalOut = pCopyHalOutMeta;
        pPushedHalOutMeta = pCopyHalOutMeta.get();
        MY_LOGW("Hal Out Meta is null, create a new one");
    }
    if (!pDepMapReq->pushRequestMetadata(
                     {PBID_OUT_HAL_META, eBUFFER_IOTYPE_OUTPUT}, pPushedHalOutMeta))
    {
        decision.set(12);//0x1000, 4096
    }
    //
    std::shared_ptr<IMetadata> pCopyAppOutMeta = nullptr;
    IMetadata* pPushedAppOutMeta = generalIO.mAppOut;
    if(pPushedAppOutMeta == nullptr)
    {
        pCopyAppOutMeta = std::make_shared<IMetadata>();    // create a new app out meta
        out.mpCopyAppOut = pCopyAppOutMeta;
        pPushedAppOutMeta = pCopyAppOutMeta.get();
        MY_LOGW("App Out Meta is null, create a new one");
    }
    if (!pDepMapReq->pushRequestMetadata(
                     {PBID_OUT_APP_META, eBUFFER_IOTYPE_OUTPUT}, pPushedAppOutMeta))
    {
        decision.set(13);//0x2000, 8096
    }

    if (true == decision.any())
        CAM_ULOGME("assign outout Data Error, index(%#lx)", decision.to_ulong());

    return decision.to_ulong();
}

MUINT16 DepthNode::fillIntoDepthMapPipe(sp<IDepthMapEffectRequest> pDepMapReq,
                            vector<inputImgData>& vImgs, vector<inputMetaData>& vMetas)
{
    MUINT16 i, decision = 0;
    if (miLogEnable > 0) CAM_ULOGM_FUNCLIFE();

    #define insertImg2DepthMapPipe(pDepMapReq, i) \
    if (!pDepMapReq->pushRequestImageBuffer( \
         {vImgs[i].param.bufferID, vImgs[i].param.ioType}, vImgs[i].buf)) { \
        decision |= (1<<(2*i)); \
    } else { \
        ULOGMD_IF(miLogEnable > 0, "InSertBuf_OK_bID:%u,ioType:%u", \
                vImgs[i].param.bufferID, vImgs[i].param.ioType);     \
    }
    #define insertMeta2DepthMapPipe(pDepMapReq, i) \
    if (!pDepMapReq->pushRequestMetadata( \
         {vMetas[i].param.bufferID, vMetas[i].param.ioType}, vMetas[i].meta)) { \
        decision |= (1<<(2*i+1)); \
    } else { \
        ULOGMD_IF(miLogEnable > 0, "InSertMeta_OK_bID:%u,ioType:%u", \
                vMetas[i].param.bufferID, vMetas[i].param.ioType);    \
    }
    //img buffer
    for (i = 0; i < vImgs.size(); i++)
        insertImg2DepthMapPipe(pDepMapReq, i);
    //Metadata
    for (i = 0; i < vMetas.size(); i++)
        insertMeta2DepthMapPipe(pDepMapReq, i);
    //
    #undef insertImg2DepthMapPipe
    #undef insertMeta2DepthMapPipe

    return decision;
}

MUINT32 DepthNode::setInputData(MUINT32 sensorID,
        sp<IDepthMapEffectRequest> pDepMapReq, const SFPSensorInput &data)
{
    if (miLogEnable > 0) {
        CAM_ULOGMD("%s ispVer:%d",
               sensorID == mSensorID.at(MAINCAM) ? "main1" : "main2", SUPPORT_ISP_VER);
    }

    MUINT32 decision = 0;

    vector<inputImgData>  vImgs_main1 {
        {{PBID_IN_RSRAW1, eBUFFER_IOTYPE_INPUT}, get(data.mRRZO)},
        {{PBID_IN_LCSO1 , eBUFFER_IOTYPE_INPUT}, get(data.mLCSO)},
    #if (SUPPORT_ISP_VER == 60)
        {{PBID_IN_P1YUV1, eBUFFER_IOTYPE_INPUT}, get(data.mRrzYuv2)},
    #endif
    };
    vector<inputMetaData> vMetas_main1 {
        {{PBID_IN_APP_META      , eBUFFER_IOTYPE_INPUT}, data.mAppIn},
        {{PBID_IN_HAL_META_MAIN1, eBUFFER_IOTYPE_INPUT}, data.mHalIn},
        {{PBID_IN_P1_RETURN_META, eBUFFER_IOTYPE_INPUT}, data.mAppDynamicIn},
    };
    vector<inputImgData>  vImgs_main2 {
        {{PBID_IN_RSRAW2, eBUFFER_IOTYPE_INPUT}, get(data.mRRZO)},
        {{PBID_IN_LCSO2 , eBUFFER_IOTYPE_INPUT}, get(data.mLCSO)},
    };
    vector<inputMetaData> vMetas_main2 {
        {{PBID_IN_HAL_META_MAIN2, eBUFFER_IOTYPE_INPUT}, data.mHalIn},
    };

    if (sensorID == mSensorID.at(MAINCAM)) {    //main1
        decision = prepareMain1Data(data);
    } else {                //main2
        decision = prepareMain2Data(data);
    }
    if (decision != 0) {
        CAM_ULOGME("ispver:%d, CAM[%d]miss input index(%#x)", SUPPORT_ISP_VER, sensorID, decision);
        return decision;
    }

    if(miLogEnable > 0)
        CAM_ULOGMD("%s:img_Size:%zu, meta_Size:%zu",
           (sensorID == mSensorID.at(MAINCAM) ? "Main1" : "Main2"),
           (sensorID == mSensorID.at(MAINCAM) ? vImgs_main1.size()  : vImgs_main2.size()),
           (sensorID == mSensorID.at(MAINCAM) ? vMetas_main1.size() : vMetas_main2.size()));

    decision = fillIntoDepthMapPipe(pDepMapReq,
                        (sensorID == mSensorID.at(MAINCAM)) ? vImgs_main1 : vImgs_main2,
                        (sensorID == mSensorID.at(MAINCAM)) ? vMetas_main1: vMetas_main2);
    if (decision > 0) {
        CAM_ULOGME("Main%d fillInDepthMap Data Error, index(%#08x)",
               (sensorID == mSensorID.at(MAINCAM) ? 1: 2), decision);
    }

    return decision;
}

MUINT32 DepthNode::prepareMain1Data(const SFPSensorInput &data)
{
    std::bitset<6> decision;

    #define assertInput(input, idx) \
        if (input == nullptr) {decision.set(idx);}

    assertInput(get(data.mRRZO)   , 0);//0x01
    assertInput(get(data.mLCSO)   , 1);//0x02
    assertInput(data.mAppIn       , 2);//0x04
    assertInput(data.mHalIn       , 3);//0x08
    assertInput(data.mAppDynamicIn, 4);//0x10

    #if (SUPPORT_ISP_VER == 60)
        assertInput(get(data.mRrzYuv2), 5);//0x20
        /*
         * RrzYuv1 stand for P1 CRZO1
         * RrzYuv2 stand for P1 CRZO2
         * P1 has 2 sensor, with CRZO1 & CRZO2 respectly
         */
    #endif
    #undef assertInput
    //
    if (decision.test(1)) {//LCSO missing is acceptable
        decision.reset(1);
        CAM_ULOGMW("%s,decision:%lx, input LCSO might miss, but still enque", __func__,
                decision.to_ulong());
    }
    return decision.to_ulong();
}

MUINT32 DepthNode::prepareMain2Data(const SFPSensorInput &data)
{
    std::bitset<3> decision;

    #define assertInput(input, idx) \
        if (input == nullptr) {decision.set(idx);}

    assertInput(get(data.mRRZO), 0);
    assertInput(get(data.mLCSO), 1);
    assertInput(data.mHalIn    , 2);

    #undef assertInput

    return decision.to_ulong();
}

MVOID DepthNode::setOutputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    CAM_ULOGM_APILIFE();
    mYuvImgPool = pool;
    mYuvImgPoolAllocateNeed = allocate;
}

void DepthNode::updateMetadata(const RequestPtr &request,
                               const SFPSensorInput& input, const SFPIOMap& output)
{
    if (input.mHalIn == nullptr)
        return ;

    *(output.mHalOut)  += *(input.mHalIn);

    if (request->hasFDOutput()) {
        MRect fdCrop{0};
        VarMap<SFP_VAR> &varMap = request->getSensorVarMap(mSensorID.at(MAINCAM));
        if (varMap.tryGet<MRect>(SFP_VAR::FD_CROP_ACTIVE_REGION, fdCrop)) {
            ULOGMD_IF(miLogEnable > 0, "fd info: x:%d,y:%d, w:%d, h:%d",
                    fdCrop.p.x, fdCrop.p.y, fdCrop.s.w,fdCrop.s.h);
            IMetadata::setEntry<MRect>(output.mHalOut, MTK_P2NODE_FD_CROP_REGION, fdCrop);
        } else {
            CAM_ULOGMW("get VAR_FD_CROP_ACTIVE_REGION with problem!");
        }
    }
    if(miLogEnable > 0)
        CAM_ULOGMD("_inHal:%d, _outHal:%d", input.mHalIn->count(), output.mHalOut->count());
}

MVOID DepthNode::handleResultData(const RequestPtr &request, const DepthEnqueData &data)
{
    CAM_ULOGM_FUNCLIFE();

    if(mPipeUsage.supportBokeh())
    {
        updateMetadata(request, request->getSensorInput(request->mMasterID),
                        request->mSFPIOManager.getFirstGeneralIO());

        handleData(ID_DEPTH_TO_BOKEH, DepthImgData(data.mOut, request));
    }
    else
    {
        updateMetadata(request, request->getSensorInput(request->mMasterID),
                        request->mSFPIOManager.getFirstGeneralIO());

        handleData(ID_DEPTH_TO_VENDOR, DepthImgData(data.mOut, request));
    }
    //
    if (request->needDump() && data.mOut.mDepthSucess) {
        if (data.mOut.mCleanYuvImg.mBuffer != nullptr) {
            data.mOut.mCleanYuvImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mOut.mCleanYuvImg.mBuffer->getImageBufferPtr(),
                    "DepthNode_yuv");
        }
        if (mPipeUsage.supportDPE()) {
            if (data.mOut.mDepthMapImg != nullptr) {
                data.mOut.mDepthMapImg->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
                dumpData(data.mRequest, data.mOut.mDepthMapImg->getImageBufferPtr(),
                        "DepthNode_depthmap");
            }
        } else if(mPipeUsage.supportBokeh()) {
            if (data.mOut.mDMBGImg != nullptr) {
                data.mOut.mDMBGImg->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
                dumpData(data.mRequest, data.mOut.mDMBGImg->getImageBufferPtr(),
                        "DepthNode_blurmap");
            }
        }
    }
    //
    if (request->needNddDump() && data.mOut.mDepthSucess) {
        char filename[1024] = {0};
        TuningUtils::FILE_DUMP_NAMING_HINT hint =
                            request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
        snprintf(filename, 1024, "%s_%ld_reqID_%d", "BID_P2A_OUT_MV_F",
                 data.mOut.mCleanYuvImg.mBuffer->getImageBufferPtr()->getBufStridesInBytes(0),
                 request->mRequestNo);
        if (data.mOut.mCleanYuvImg.mBuffer != nullptr) {
            data.mOut.mCleanYuvImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpNddData(&hint, data.mOut.mCleanYuvImg.mBuffer->getImageBufferPtr(),
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        }
    }
    //
    if (request->needDisplayOutput(this))
    {
        request->updateResult(true);
        CAM_ULOGMD("DepthNode w/o cascade anything, buffer to preview directly");
    }
    //
}

MVOID DepthNode::onPipeReady(MVOID* tag, NSDualFeature::ResultState state,
                                            sp<IDualFeatureRequest>& request)
{
    CAM_ULOGM_FUNCLIFE();
    DepthEnqueData   enqueData;
    DepthNodePackage depthNodePack;
    DepthNode*       pDepthNode = (DepthNode*)tag;
    sp<IDepthMapEffectRequest> pDepReq = (IDepthMapEffectRequest*) request.get();

    TRACE_FUNC("DepthPipeItem reqID=%d state=%d:%s",
               pDepReq->getRequestNo(), state, ResultState2Name(state));
    // if complete/not_ready -> enue to next item
    if (state == eRESULT_COMPLETE || state == eRESULT_DEPTH_NOT_READY || state == eRESULT_FLUSH)
    {
        ssize_t idx = -1;
        {
            {
                android::Mutex::Autolock lock(pDepthNode->mLock);
                idx = pDepthNode->mvDepthNodePack.indexOfKey(pDepReq->getRequestNo());
                if (idx < 0) {
                    CAM_ULOGME("[DepthNode]idx=%zu,reqID=%u is missing, might released!!",
                            idx, pDepReq->getRequestNo());
                    return;
                }
                depthNodePack = pDepthNode->mvDepthNodePack.valueAt(idx);
                enqueData     = *(depthNodePack.depEnquePack);
            }
            //
            enqueData.mOut.mDepthSucess = MTRUE;
            pDepthNode->handleResultData(enqueData.mRequest, enqueData);//send data to next nodes

            enqueData.mRequest->mTimer.stopEnqueDepth();
            pDepthNode->decExtThreadDependency(); // tell one request call back
        }

        CAM_ULOGMD("Remove reqID=%d, mvDepEffReq size=%zu",
                pDepReq->getRequestNo(), pDepthNode->mvDepthNodePack.size());

        {
            android::Mutex::Autolock lock(pDepthNode->mLock);
            pDepthNode->mvDepthNodePack.removeItem(pDepReq->getRequestNo());
        }
        //
        enqueData.mRequest->mTimer.stopDepth();
    }
}
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

//=======================================================================================
#else //SUPPORT_VSDO
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

