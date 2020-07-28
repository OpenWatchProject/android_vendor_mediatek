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
#include "../util/vsdof_util.h"

#include "PreProcessNode.h"

#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "PreProcessNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <PipeLog.h>

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>

#include "exif/ExifWriter.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace VSDOF::util;
using namespace NSIoPipe;
using NSCam::NSIoPipe::NSSImager::IImageTransform;

#define THREAD_NAME       ("P2DoneThread@PreProcessNode")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

#define DO_FSRAW_TO_YUV_1 "DO_FSRAW_TO_YUV_1"
#define DO_FSRAW_TO_YUV_2 "DO_FSRAW_TO_YUV_2"
#define DO_RSRAW_TO_YUV_1 "DO_RSRAW_TO_YUV_1"
#define DO_RSRAW_TO_YUV_2 "DO_RSRAW_TO_YUV_2"

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

/*******************************************************************************
 *
 ********************************************************************************/
PreProcessNode::
PreProcessNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : DualCamMFPipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
    , mpDebugDrawID(nullptr)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
    this->addWaitQueue(&mRequestsBssSrc);

    if(::property_get_int32(STRING_DRAW_TIMESTAMP, 0)){
        mpDebugDrawID = DebugDrawID::createInstance();
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
PreProcessNode::
~PreProcessNode()
{
    MY_LOGD("dctor(0x%x)", this);

    if( mpDebugDrawID != nullptr ){
        mpDebugDrawID->destroyInstance();
        mpDebugDrawID = nullptr;
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case BSS_RESULT_ORIGIN:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        case BSS_RESULT_ORDERED:
            mRequestsBssSrc.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_CALL();
    if(!DualCamMFPipeNode::onInit()){
        MY_LOGE("DualCamMFPipeNode::onInit() failed!");
        return MFALSE;
    }

    MY_LOGD("PreProcessNode::onInit=>create_3A_instance senosrIdx:(%d/%d)", mSensorIdx_Main1, mSensorIdx_Main2);
    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "DCMF_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "DCMF_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %p Main2: %p", mp3AHal_Main1, mp3AHal_Main2);
#if DCMF_TEMP == 1
    // do nothing
#else
    MY_LOGD("EIspProfile_N3D_Capture(%d) EIspProfile_N3D_Capture_toW(%d) EIspProfile_N3D_Capture_Depth(%d) EIspProfile_N3D_Capture_Depth_toW(%d)",
        NSIspTuning::EIspProfile_N3D_Capture, NSIspTuning::EIspProfile_N3D_Capture_toW,
        NSIspTuning::EIspProfile_N3D_Capture_Depth, NSIspTuning::EIspProfile_N3D_Capture_Depth_toW
    );
#endif

    mSizePrvider = StereoSizeProvider::getInstance();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onUninit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    cleanUp();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    // exit threads
    mpP2DoneTasker->requestExit();
    mpP2DoneTasker->join();

    if(mp3AHal_Main1){
        mp3AHal_Main1->destroyInstance("DCMF_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }

    if(mp3AHal_Main2){
        mp3AHal_Main2->destroyInstance("DCMF_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }
    mBufPool.uninit();
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
enqueISP(
    PipeRequestPtr& request,
    PipeRequestPtr& requestBssSrc,
    DualCamMFProcessId processID,
    IHal3A* pHal3A,
    MINT32 sensorId,
    inputData& inputs,
    outputData& outputs,
    MBOOL updateHalMeta,
    MBOOL updateFOVCropToMeta
){
    MINT32 reqNo = request->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);

    // 1.input data
    SmartTuningBuffer pBuf_tuning = mBufPool.getTuningBufPool()->request();

    CHECK_OBJECT(requestBssSrc->getImageBuffer(inputs.raw));
    sp<IImageBuffer> pBuf_raw = requestBssSrc->getImageBuffer(inputs.raw);

    CHECK_OBJECT(requestBssSrc->getMetadata(inputs.meta_app));
    IMetadata* pMeta_app = requestBssSrc->getMetadata(inputs.meta_app);

    CHECK_OBJECT(requestBssSrc->getMetadata(inputs.meta_hal));
    IMetadata* pMeta_hal = requestBssSrc->getMetadata(inputs.meta_hal);

    sp<IImageBuffer> pBuf_lcso = requestBssSrc->getImageBuffer(inputs.lcso);

    // 2.output data
    sp<IImageBuffer> pBuf_img3o = request->getImageBuffer(outputs.img3o);
    sp<IImageBuffer> pBuf_wroto = request->getImageBuffer(outputs.wroto);
    sp<IImageBuffer> pBuf_wdmao = request->getImageBuffer(outputs.wdmao);

    CHECK_OBJECT(request->getMetadata(outputs.meta_hal_out));
    IMetadata* pMeta_hal_out = request->getMetadata(outputs.meta_hal_out);

    MY_LOGD("in: raw(%s)(%p) lcso(%s)(%p) meta(%p/%p), out: img3o(%s)(%p) wroto(%s)(%p) wdmao(%s)(%p)",
        getBIDName(inputs.raw), pBuf_raw.get(),
        getBIDName(inputs.lcso), pBuf_lcso.get(),
        pMeta_app, pMeta_hal,
        getBIDName(outputs.img3o), pBuf_img3o.get(),
        getBIDName(outputs.wroto), pBuf_wroto.get(),
        getBIDName(outputs.wdmao), pBuf_wdmao.get()
    );

    // 3.1 ISP tuning
    TuningParam rTuningParam;

    MBOOL isResizedRaw = MFALSE;
    if(inputs.raw == DualCamMFBufferID::BID_INPUT_RSRAW_1 || inputs.raw == DualCamMFBufferID::BID_INPUT_RSRAW_2){
        isResizedRaw = MTRUE;
    }

    MBOOL isToWPath = MFALSE;
    MINT32 enableMfb = MFALSE;
    MUINT8 ispProfile = NSIspTuning::EIspProfile_N3D_Capture;
    {
#if DCMF_TEMP == 1
        ispProfile = NSIspTuning::EIspProfile_N3D_Capture;
#else
        enableMfb = request->getParam(DualCamMFParamID::PID_ENABLE_MFB);
        // main1 should use toW profile when main2 is mono for brightness alignment
        if( sensorId == mSensorIdx_Main1 && getSensorRawFmt(mSensorIdx_Main2) == SENSOR_RAW_MONO && isResizedRaw){
            // MY_LOGD("main1 rrzo uses N3D_Capture_Depth profile");
            // ispProfile = NSIspTuning::EIspProfile_N3D_Capture_Depth_toW;
            ispProfile = NSIspTuning::EIspProfile_N3D_Capture_Depth;
        }else if( sensorId == mSensorIdx_Main2 && getSensorRawFmt(mSensorIdx_Main2) == SENSOR_RAW_MONO && isResizedRaw){
            // MY_LOGD("main2 rrzo uses N3D_Capture_Depth profile");
            ispProfile = NSIspTuning::EIspProfile_N3D_Capture_Depth;
        }else{
            // imgo to yuv (main uses mfnr isp tuning)
            if(enableMfb && sensorId == mSensorIdx_Main1){
                ispProfile = NSIspTuning::EIspProfile_MFNR_Before_Blend;
            }else{
                ispProfile = NSIspTuning::EIspProfile_N3D_Capture;
            }
        }
#endif
        MY_LOGD("sensorId(%d), main2 sensorFmt(%d), isResizedRaw(%d) mfb(%d)=> profile(%u)",
            sensorId, getSensorRawFmt(mSensorIdx_Main2), isResizedRaw, enableMfb, ispProfile);

        trySetMetadata<MUINT8>(pMeta_hal, MTK_3A_ISP_PROFILE, ispProfile);
        ISPTuningConfig ispConfig = {pMeta_app, pMeta_hal, pHal3A, isResizedRaw, reqNo, pBuf_lcso.get()};

        if( updateHalMeta ){
            rTuningParam = applyISPTuning(getProcessName(processID), pBuf_tuning, ispConfig, pMeta_hal_out);
        }else{
            rTuningParam = applyISPTuning(getProcessName(processID), pBuf_tuning, ispConfig);
        }
    }

    // 3.2 MDP tuning
    PQParam* pPQParam = new PQParam();
    fillInPQParam(pPQParam, (MINT32)processID);

    // 3.3 Srz tuning for Imgo
    ModuleInfo* pModuleInfo = nullptr;
    MBOOL needModuleInfo = MFALSE;
    if(isResizedRaw){
        // LCE not applied to rrzo
    }else{
        pModuleInfo = new ModuleInfo();
        needModuleInfo = fillInSrzParam(pModuleInfo, (MINT32)processID, pBuf_lcso, pBuf_raw);
    }

    // 4.create enque param
    MBOOL bSuccess = MTRUE;
    MINT32 iFrameNum = 0;
    NSIoPipe::QParams qParam;

    if(pBuf_img3o != nullptr){
        MY_LOGD("[IMG3O]outputs settings: src(%dx%d) (out:%dx%d)",
            pBuf_raw->getImgSize().w, pBuf_raw->getImgSize().h,
            pBuf_img3o->getImgSize().w, pBuf_img3o->getImgSize().h
        );
    }

    MINT32 transWROTO = outputs.wrotoTrans;
    MRect srcCropWROTO = MRect(pBuf_raw->getImgSize().w, pBuf_raw->getImgSize().h);
    if(pBuf_wroto != nullptr){
        if(outputs.wrotoCrop.s.w != 0 || outputs.wrotoCrop.s.h != 0){
            srcCropWROTO = outputs.wrotoCrop;
        }else{
            updateSrcCropByOutputRatio(srcCropWROTO, pBuf_raw, pBuf_wroto, transWROTO);
        }

        MY_LOGD("[WROTO]outputs settings: src(%dx%d) (trans:%d)(crop:%d,%d,%dx%d)(out:%dx%d)",
            pBuf_raw->getImgSize().w, pBuf_raw->getImgSize().h,
            transWROTO,
            srcCropWROTO.p.x, srcCropWROTO.p.y, srcCropWROTO.s.w, srcCropWROTO.s.h,
            pBuf_wroto->getImgSize().w, pBuf_wroto->getImgSize().h
        );
    }

    MRect srcCropWDMAO = MRect(pBuf_raw->getImgSize().w, pBuf_raw->getImgSize().h);
    if(pBuf_wdmao != nullptr){
        if(outputs.wdmaoCrop.s.w != 0 || outputs.wdmaoCrop.s.h != 0){
            srcCropWDMAO = outputs.wdmaoCrop;
        }else{
            updateSrcCropByOutputRatio(srcCropWDMAO, pBuf_raw, pBuf_wdmao, 0);
        }

        MY_LOGD("[WDMAO]outputs settings: src(%dx%d) (trans:%d)(crop:%d,%d,%dx%d)(out:%dx%d)",
            pBuf_raw->getImgSize().w, pBuf_raw->getImgSize().h,
            0,
            srcCropWDMAO.p.x, srcCropWDMAO.p.y, srcCropWDMAO.s.w, srcCropWDMAO.s.h,
            pBuf_wdmao->getImgSize().w, pBuf_wdmao->getImgSize().h
        );

        if(updateFOVCropToMeta){
            updateFOVCroppingToMeta(pMeta_hal_out, srcCropWDMAO, pBuf_raw->getImgSize());
        }
    }

    String8 str = String8::format("\nenqueISP params:");

    // 4.1 QParam template
#if 0
    QParamTemplateGenerator qPTempGen = QParamTemplateGenerator(iFrameNum, sensorId, ENormalStreamTag_Normal);
#else
    QParamTemplateGenerator qPTempGen = QParamTemplateGenerator(iFrameNum, sensorId, ENormalStreamTag_Vss);
    str = str + String8::format("vss mode,");
#endif
    qPTempGen.addInput(PORT_IMGI);
#ifndef GTEST
    if(!isResizedRaw && rTuningParam.pLsc2Buf != nullptr){
        qPTempGen.addInput(PORT_IMGCI);
        str = str + String8::format("PORT_IMGCI,");
    }
    if(!isResizedRaw && pBuf_lcso != nullptr){
        qPTempGen.addInput(PORT_LCEI);
        str = str + String8::format("PORT_LCEI,");
        if(needModuleInfo){
            qPTempGen.addModuleInfo(EDipModule_SRZ4,  pModuleInfo->moduleStruct);
            qPTempGen.addInput(PORT_DEPI);
            str = str + String8::format("PORT_DEPI,");
        }
    }
    if(rTuningParam.pBpc2Buf != nullptr){
        qPTempGen.addInput(PORT_IMGBI);
        str = str + String8::format("PORT_IMGBI,");
    }
#endif

    if(pBuf_img3o != nullptr){
        qPTempGen.addOutput(PORT_IMG3O, 0);
    }
    if(pBuf_wroto != nullptr){
        qPTempGen.addOutput(PORT_WROTO, transWROTO);
        qPTempGen.addCrop(eCROP_WROT, srcCropWROTO.p, srcCropWROTO.s, pBuf_wroto->getImgSize());
    }
    if(pBuf_wdmao != nullptr){
        qPTempGen.addOutput(PORT_WDMAO, 0);
        qPTempGen.addCrop(eCROP_WDMA, srcCropWDMAO.p, srcCropWDMAO.s, pBuf_wdmao->getImgSize());
    }

    qPTempGen.addExtraParam(EPIPE_MDP_PQPARAM_CMD, (MVOID*)pPQParam);

    if(!qPTempGen.generate(qParam)){
        MY_LOGE("failed to create qParam!");
        return MFALSE;
    }

    // 4.2 QParam filler
    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGI,  pBuf_raw.get());
#ifndef GTEST
    qParamFiller.insertTuningBuf(iFrameNum,             pBuf_tuning->mpVA);
    if(!isResizedRaw && rTuningParam.pLsc2Buf != nullptr){
        qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGCI,  static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf));
        str = str + String8::format("PORT_IMGCI buf(%p),", rTuningParam.pLsc2Buf);
    }
    if(!isResizedRaw && pBuf_lcso != nullptr){
        qParamFiller.insertInputBuf(iFrameNum,  PORT_LCEI,  pBuf_lcso.get());
        str = str + String8::format("PORT_LCEI buf(%p),", pBuf_lcso.get());
        if(needModuleInfo){
            qParamFiller.insertInputBuf(iFrameNum,  PORT_DEPI,  pBuf_lcso.get());
            str = str + String8::format("PORT_DEPI buf(%p),", pBuf_lcso.get());
        }
    }
    if(rTuningParam.pBpc2Buf != nullptr){
        qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGBI,  static_cast<IImageBuffer*>(rTuningParam.pBpc2Buf));
        str = str + String8::format("PORT_IMGBI buf(%p)", rTuningParam.pBpc2Buf);
    }
#else
    qParamFiller.insertTuningBuf(iFrameNum,             nullptr);
#endif

    if(pBuf_img3o != nullptr){
        qParamFiller.insertOutputBuf(iFrameNum, PORT_IMG3O, pBuf_img3o.get());
    }
    if(pBuf_wroto != nullptr){
        qParamFiller.insertOutputBuf(iFrameNum, PORT_WROTO, pBuf_wroto.get());
    }
    if(pBuf_wdmao != nullptr){
        qParamFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, pBuf_wdmao.get());
    }

    qParamFiller.setInfo(iFrameNum, reqNo, reqNo, (MUINT32)processID);

    if(!qParamFiller.validate()){
        MY_LOGE("failed to create qParamFiller!");
        return MFALSE;
    }

    MY_LOGD("%s",  str.string());

    // 5.enque
    // save working buffers to enqueBufferPool
    EnquedBufPool *pEnqueData= new EnquedBufPool(this, request, nullptr, processID);
    pEnqueData->mPipeRequest_BssSrc = requestBssSrc;
    pEnqueData->addTuningData(pBuf_tuning);
    pEnqueData->addPQData(pPQParam);
    pEnqueData->addModuleInfo(pModuleInfo);
    pEnqueData->start();

    sp<IStopWatchCollection> pStopWatchCollection  = request->getStopWatchCollection();
    pStopWatchCollection->BeginStopWatch(getProcessName(processID), (void*)pEnqueData);

    // callbacks
    qParam.mpfnCallback = onP2SuccessCallback;
    qParam.mpfnEnQFailCallback = onP2FailedCallback;
    qParam.mpCookie = (MVOID*) pEnqueData;

    // p2 enque
    sp<P2Operator> p2Op = mwpP2Operator.promote();
    p2Op->enque(qParam, PIPE_LOG_TAG);
    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    TaskQueue<PreProcessNode>::TaskTable taskTable({{FSRAW_TO_YUV_1, &PreProcessNode::onRawToYUV_FSRAW_1_Done},
                                                    {FSRAW_TO_YUV_2, &PreProcessNode::onRawToYUV_FSRAW_2_Done},
                                                    {RSRAW_TO_YUV_1, &PreProcessNode::onRawToYUV_RSRAW_1_Done},
                                                    {RSRAW_TO_YUV_2, &PreProcessNode::onRawToYUV_RSRAW_2_Done}});
    mpP2DoneTasker = make_unique<TaskQueue<PreProcessNode>>(this, THREAD_NAME, taskTable);
    mpP2DoneTasker->run();

    initBufferPool();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadStop()
{
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    PipeRequestPtr pipeRequest = nullptr;
    PipeRequestPtr pipeRequestBssSrc = nullptr;
    // block until queue ready, or flush() breaks the blocking state too.
    if( !waitAllQueue() ){
        return MFALSE;
    }
    if( !mRequests.deque(pipeRequest) ){
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }
    if( !mRequestsBssSrc.deque(pipeRequestBssSrc) ){
        MY_LOGD("mRequestsBssSrc.deque() failed");
        return MFALSE;
    }

    // process 1 request at a time
    waitAndAddExclusiveJob(pipeRequest->getRequestNo());

    this->incExtThreadDependency();
    if(!doRawToYUV_FSRAW_1(pipeRequest, pipeRequestBssSrc)){
        MY_LOGE("failed doing doRawToYUV_FSRAW_1, please check above errors!");
        this->decExtThreadDependency();
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onP2SuccessCallback(QParams& rParams)
{
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PreProcessNode* pNode = (PreProcessNode*) (pEnqueData->mpNode);

    MY_LOGD("reqID: %d, Data: %p, ProcessId: %d(%s)",
        pEnqueData->getReqNo(), pEnqueData, pEnqueData->mProcessId, getProcessName(pEnqueData->mProcessId));
    pNode->mpP2DoneTasker->addTaskData(pEnqueData);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onP2FailedCallback(QParams& rParams)
{
    MY_LOGE("PreProcessNode operations failed!!Check above errors!");
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    PreProcessNode* pNode = (PreProcessNode*) (pEnqueData->mpNode);
    pNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mPImgInfo);
    pNode->decExtThreadDependency();
    delete pEnqueData;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doRawToYUV_FSRAW_1(
    PipeRequestPtr request, PipeRequestPtr requestBssSrc)
{
    MINT32 reqNo = request->getRequestNo();
    CAM_TRACE_FMT_BEGIN("doRawToYUV_FSRAW_1 req(%d)", reqNo);
    MET_START(DO_FSRAW_TO_YUV_1);
    MY_LOGD("+, reqID=%d", reqNo);

    // input
    PreProcessNode::inputData inputData;
    inputData.raw      = DualCamMFBufferID::BID_INPUT_FSRAW_1;
    inputData.lcso     = DualCamMFBufferID::BID_LCS_1;
    inputData.meta_app = DualCamMFBufferID::BID_META_IN_APP;
    inputData.meta_hal = DualCamMFBufferID::BID_META_IN_HAL;

    // output
    PreProcessNode::outputData outputData;

    outputData.wroto   = DualCamMFBufferID::BID_FS_YUV_1;
    outputData.wdmao   = DualCamMFBufferID::BID_SM_YUV;

    outputData.meta_hal_out = DualCamMFBufferID::BID_META_OUT_HAL;

    // fov crop
    MRect fovCrop = MRect(0,0);
    if( getFOVCropping(
        request,
        fovCrop,
        inputData.raw,
        inputData.meta_hal)
    ){
        outputData.wdmaoCrop = fovCrop;
        outputData.wrotoCrop = fovCrop;
    }

    handleByPassMeta(request);

    MBOOL bEnqueISP = enqueISP(
        request, requestBssSrc,
        DualCamMFProcessId::FSRAW_TO_YUV_1,
        mp3AHal_Main1,
        mSensorIdx_Main1,
        inputData, outputData,
        MTRUE, // update isp tuning to hal meta
        MTRUE  // update fov cropping to hal meta
    );

    if(!bEnqueISP){
        MY_LOGE("enqueISP failed!");
        return MFALSE;
    }

    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onRawToYUV_FSRAW_1_Done(
    EnquedBufPool* pEnqueData)
{
    MET_END(DO_FSRAW_TO_YUV_1);

    PipeRequestPtr request = pEnqueData->mPipeRequest;
    PipeRequestPtr requestBssSrc = pEnqueData->mPipeRequest_BssSrc;

    request->getStopWatchCollection()->EndStopWatch((void*)pEnqueData);

    drawTimestamp(
        request->getMetadata(DualCamMFBufferID::BID_META_IN_HAL),
        request->getImageBuffer(DualCamMFBufferID::BID_FS_YUV_1).get()
    );

    MINT32 reqNo = request->getRequestNo();
    CAM_TRACE_FMT_BEGIN("onRawToYUV_FSRAW_1_Done req(%d)", reqNo);
    MET_START(DO_FSRAW_TO_YUV_2);
    MY_LOGD("+, reqID=%d", reqNo);

    // postview
    if( request->getImageBuffer(DualCamMFBufferID::BID_FS_YUV_1) != nullptr &&
        request->getImageBuffer(DualCamMFBufferID::BID_RS_YUV_1) != nullptr){
        // use rs yuv to produce postview for better performance
    }else{
        producePostview(pEnqueData, DualCamMFBufferID::BID_FS_YUV_1);
    }

    updateBSSExif(request);

    // by-pass request of output not provided
    if(request->getImageBuffer(DualCamMFBufferID::BID_FS_YUV_2) == nullptr){
        MY_LOGD("FS_YUV_2 not provided");
        onRawToYUV_FSRAW_2_Done(pEnqueData);
        MY_LOGD("-, reqID=%d", reqNo);
        CAM_TRACE_FMT_END();
        return;
    }

    // input
    PreProcessNode::inputData inputData;
    inputData.raw      = DualCamMFBufferID::BID_INPUT_FSRAW_2;
    inputData.lcso     = DualCamMFBufferID::BID_LCS_2;
    inputData.meta_app = DualCamMFBufferID::BID_META_IN_APP;
    inputData.meta_hal = DualCamMFBufferID::BID_META_IN_HAL_2;

    // output
    PreProcessNode::outputData outputData;
    outputData.wdmao   = DualCamMFBufferID::BID_FS_YUV_2;
    outputData.meta_hal_out = DualCamMFBufferID::BID_META_OUT_HAL_2;

    // fov crop
    MRect fovCrop = MRect(0,0);
    if( getFOVCropping(
        request,
        fovCrop,
        inputData.raw,
        inputData.meta_hal)
    ){
        outputData.wdmaoCrop = fovCrop;
    }

    MBOOL bEnqueISP = enqueISP(
        request, requestBssSrc,
        DualCamMFProcessId::FSRAW_TO_YUV_2,
        mp3AHal_Main2,
        mSensorIdx_Main2,
        inputData, outputData,
        MTRUE, // update isp tuning to hal meta
        MTRUE  // update fov cropping to hal meta
    );

    if(!bEnqueISP){
        MY_LOGE("enqueISP failed!");
    }

    delete pEnqueData;
    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onRawToYUV_FSRAW_2_Done(
    EnquedBufPool* pEnqueData)
{
    MET_END(DO_FSRAW_TO_YUV_2);

    PipeRequestPtr request = pEnqueData->mPipeRequest;
    PipeRequestPtr requestBssSrc = pEnqueData->mPipeRequest_BssSrc;

    request->getStopWatchCollection()->EndStopWatch((void*)pEnqueData);

    drawTimestamp(
        request->getMetadata(DualCamMFBufferID::BID_META_IN_HAL_2),
        request->getImageBuffer(DualCamMFBufferID::BID_FS_YUV_2).get()
    );

    MINT32 reqNo = request->getRequestNo();
    CAM_TRACE_FMT_BEGIN("onRawToYUV_FSRAW_2_Done req(%d)", reqNo);
    MET_START(DO_RSRAW_TO_YUV_1);
    MY_LOGD("+, reqID=%d", reqNo);

    // by-pass request of output not provided
    if(request->getImageBuffer(DualCamMFBufferID::BID_RS_YUV_1) == nullptr){
        MY_LOGD("RS_YUV_1 not provided");
        onRawToYUV_RSRAW_1_Done(pEnqueData);
        MY_LOGD("-, reqID=%d", reqNo);
        CAM_TRACE_FMT_END();
        return;
    }

    // input
    PreProcessNode::inputData inputData;
    inputData.raw      = DualCamMFBufferID::BID_INPUT_RSRAW_1;
    inputData.meta_app = DualCamMFBufferID::BID_META_IN_APP;
    inputData.meta_hal = DualCamMFBufferID::BID_META_IN_HAL;

    // output
    PreProcessNode::outputData outputData;
    outputData.wdmao        = DualCamMFBufferID::BID_RS_YUV_1;
    outputData.meta_hal_out = DualCamMFBufferID::BID_META_OUT_HAL;

    MBOOL bEnqueISP = enqueISP(
        request, requestBssSrc,
        DualCamMFProcessId::RSRAW_TO_YUV_1,
        mp3AHal_Main1,
        mSensorIdx_Main1,
        inputData, outputData
    );

    if(!bEnqueISP){
        MY_LOGE("enqueISP failed!");
    }

    delete pEnqueData;
    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onRawToYUV_RSRAW_1_Done(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MET_END(DO_RSRAW_TO_YUV_1);

    PipeRequestPtr request = pEnqueData->mPipeRequest;
    PipeRequestPtr requestBssSrc = pEnqueData->mPipeRequest_BssSrc;

    request->getStopWatchCollection()->EndStopWatch((void*)pEnqueData);

    MINT32 reqNo = pEnqueData->mPipeRequest->getRequestNo();
    CAM_TRACE_FMT_BEGIN("onRawToYUV_RSRAW_1_Done req(%d)", reqNo);
    MET_START(DO_RSRAW_TO_YUV_2);
    MY_LOGD("+, reqID=%d", reqNo);

    // postview
    if( request->getImageBuffer(DualCamMFBufferID::BID_FS_YUV_1) != nullptr &&
        request->getImageBuffer(DualCamMFBufferID::BID_RS_YUV_1) != nullptr){
        // use rs yuv to produce postview for better performance
        producePostview(pEnqueData, DualCamMFBufferID::BID_RS_YUV_1);
    }else{
        // do nothing
    }

    // by-pass request of output not provided
    if(request->getImageBuffer(DualCamMFBufferID::BID_RS_YUV_2) == nullptr){
        MY_LOGD("RS_YUV_2 not provided");
        onRawToYUV_RSRAW_2_Done(pEnqueData);
        MY_LOGD("-, reqID=%d", reqNo);
        CAM_TRACE_FMT_END();
        return;
    }

    // input
    PreProcessNode::inputData inputData;
    inputData.raw      = DualCamMFBufferID::BID_INPUT_RSRAW_2;
    inputData.meta_app = DualCamMFBufferID::BID_META_IN_APP;
    inputData.meta_hal = DualCamMFBufferID::BID_META_IN_HAL_2;

    // output
    PreProcessNode::outputData outputData;
    outputData.wdmao   = DualCamMFBufferID::BID_RS_YUV_2;
    outputData.meta_hal_out = DualCamMFBufferID::BID_META_OUT_HAL_2;

    MBOOL bEnqueISP = enqueISP(
        request, requestBssSrc,
        DualCamMFProcessId::RSRAW_TO_YUV_2,
        mp3AHal_Main2,
        mSensorIdx_Main2,
        inputData, outputData
    );

    if(!bEnqueISP){
        MY_LOGE("enqueISP failed!");
    }

    delete pEnqueData;
    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
onRawToYUV_RSRAW_2_Done(
    EnquedBufPool* pEnqueData)
{
    CAM_TRACE_CALL();
    MET_END(DO_RSRAW_TO_YUV_2);

    PipeRequestPtr request = pEnqueData->mPipeRequest;
    request->getStopWatchCollection()->EndStopWatch((void*)pEnqueData);

    MINT32 reqNo = pEnqueData->mPipeRequest->getRequestNo();
    CAM_TRACE_FMT_BEGIN("onRawToYUV_RSRAW_2_Done req(%d)", reqNo);
    MY_LOGD("+, reqID=%d", reqNo);

    handleFinish(pEnqueData->mPipeRequest);

    delete pEnqueData;

    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
producePostview(
    EnquedBufPool* pEnqueData, DualCamMFBufferID srcBufId)
{
    PipeRequestPtr request = pEnqueData->mPipeRequest;

    MINT32 isMainFrame = request->getParam(DualCamMFParamID::PID_IS_MAIN);
    if(isMainFrame != 1){
        return MTRUE;
    }

    MINT32 reqNo = pEnqueData->mPipeRequest->getRequestNo();
    CAM_TRACE_FMT_BEGIN("producePostview req(%d)", reqNo);
    MY_LOGD("+, reqID=%d", reqNo);
    sp<IStopWatchCollection> pStopWatchCollection  = request->getStopWatchCollection();
    pStopWatchCollection->BeginStopWatch("postview", (void*)pEnqueData);

    sp<IImageBuffer> pBuf_yuv = request->getImageBuffer(srcBufId);
    CHECK_OBJECT(pBuf_yuv);

    sp<IImageBuffer> pBuf_postview = request->getImageBuffer(DualCamMFBufferID::BID_POSTVIEW);
    CHECK_OBJECT(pBuf_postview);

    MINT32 trans = getJpegRotation(request->getMetadata(DualCamMFBufferID::BID_META_IN_APP));

    // use IImageTransform to handle image cropping
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
            IImageTransform::createInstance(), // constructor
            [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
            );

    if (transform.get() == nullptr) {
        MY_LOGE("IImageTransform is NULL, cannot generate output");
        return MFALSE;
    }

    MRect ratioCrop = MRect(pBuf_yuv->getImgSize().w, pBuf_yuv->getImgSize().h);
    updateSrcCropByOutputRatio(ratioCrop, pBuf_yuv, pBuf_postview, trans);

    MBOOL ret = MTRUE;
    ret = transform->execute(
            pBuf_yuv.get(),
            pBuf_postview.get(),
            nullptr,
            ratioCrop, // ratio crop
            trans,     // transform
            3000  // timeout
        );

    if(!ret){
        MY_LOGE("Failed doIImageTransform!");
        return MFALSE;
    }
    pStopWatchCollection->EndStopWatch((void*)pEnqueData);
    handleData(QUICK_POSTVIEW, request);

    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
handleFinish(
    PipeRequestPtr request)
{
    MINT32 reqNo = request->getRequestNo();
    CAM_TRACE_FMT_BEGIN("PreProcessNode::handleFinish req(%d)", reqNo);
    MY_LOGD("+, reqID=%d", reqNo);

    handleData(PREPROCESS_TO_VENDOR, request);

    handleData(PREPROCESS_YUV_DONE, request);

    finishExclusiveJob(reqNo);

    this->decExtThreadDependency();

    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
handleByPassMeta(
    PipeRequestPtr request) const
{
    // Should do nothing, metadata bypass is handled by BaseDCCtrler::generatePipeRequst()
}

/*******************************************************************************
 *
 ********************************************************************************/
MUINT
PreProcessNode::
getSensorRawFmt(
    const MINT32 sensorId) const
{
    MUINT ret = SENSOR_RAW_Bayer;

    IHalSensorList *sensorList = MAKE_HalSensorList();
    if (NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
        return SENSOR_RAW_FMT_NONE;
    }

    int32_t sensorCount = sensorList->queryNumberOfSensors();
    if(sensorId >= sensorCount) {
        MY_LOGW("Sensor index should be <= %d", sensorCount-1);
        return SENSOR_RAW_FMT_NONE;
    }

    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    int sendorDevIndex = sensorList->querySensorDevIdx(sensorId);
    sensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

    ret = sensorStaticInfo.rawFmtType;

    MUINT debug = ::property_get_int32(STRING_SENSOR_FMT, ret);

    if( ret != debug ){
        MY_LOGD("openId(%d) real fmt(%d)->force fmt(%d)", sensorId, ret, debug);
        ret = debug;
    }

    MY_LOGD("openId(%d) fmt(%d)", sensorId, ret);

    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
getFOVCropping(
    PipeRequestPtr request,
    MRect& rFovCrop,
    DualCamMFBufferID srcBufId,
    DualCamMFBufferID inMetaId) const
{
    MBOOL ret = MFALSE;

    IMetadata* pMeta_in = request->getMetadata(inMetaId);

    if(pMeta_in == nullptr){
        MY_LOGW("pMeta_in == nullptr!");
        return MFALSE;
    }

    sp<IImageBuffer> pBuf_src = request->getImageBuffer(srcBufId);

    if(pBuf_src == nullptr){
        MY_LOGW("pBuf_src == nullptr!");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMeta_in->entryFor(MTK_STEREO_FEATURE_FOV_CROP_REGION);

    if( !entry.isEmpty() ){
        // there are fov cropping setting assigned by upper layer module
        rFovCrop.p.x = entry.itemAt(0, Type2Type<MINT32>());
        rFovCrop.p.y = entry.itemAt(1, Type2Type<MINT32>());
        rFovCrop.s.w = entry.itemAt(2, Type2Type<MINT32>());
        rFovCrop.s.h = entry.itemAt(3, Type2Type<MINT32>());

        MY_LOGD("get fov crop setting (%d,%d,%dx%d)",
            rFovCrop.p.x, rFovCrop.p.y, rFovCrop.s.w, rFovCrop.s.h
        );
        return MTRUE;
    }else{
        return MFALSE;
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
updateFOVCroppingToMeta(
    IMetadata* pHalMeta, const MRect& fovCrop, const MSize& srcSize) const
{
    IMetadata::IEntry entry(MTK_STEREO_FEATURE_FOV_CROP_REGION);

    entry.push_back(fovCrop.p.x, Type2Type< MINT32 >());
    entry.push_back(fovCrop.p.y, Type2Type< MINT32 >());
    entry.push_back(fovCrop.s.w, Type2Type< MINT32 >());
    entry.push_back(fovCrop.s.h, Type2Type< MINT32 >());

    entry.push_back(srcSize.w, Type2Type< MINT32 >());
    entry.push_back(srcSize.h, Type2Type< MINT32 >());

    pHalMeta->update(entry.tag(), entry);

    MY_LOGD("crop(%d,%d,%dx%d) srcSize(%dx%d)",
        fovCrop.p.x, fovCrop.p.y, fovCrop.s.w, fovCrop.s.h,
        srcSize.w, srcSize.h
    );
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
updateBSSExif(
    PipeRequestPtr request)
{
    ExifWriter writer(PIPE_LOG_TAG);
    writer.makeExifFromCollectedData_multiframe_mapping(request);

#if 0 // debug
    IMetadata* pMeta_hal_out = request->getMetadata(DualCamMFBufferID::BID_META_OUT_HAL);
    CHECK_OBJECT(pMeta_hal_out);

    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pMeta_hal_out->entryFor(MTK_3A_EXIF_METADATA);
        if (entry.isEmpty()) {
            MY_LOGW("%s: no MTK_3A_EXIF_METADATA can be used", __FUNCTION__);
            return MFALSE;
        }

        exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }
    // get exsited bss data and add to data
    IMetadata::Memory oldDebugInfoSet;
    MBOOL haveOldDebugInfo = IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, oldDebugInfoSet);
    MY_LOGD("updateBSSExif haveOldDebugInfo(%d)", haveOldDebugInfo);
#endif

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
drawTimestamp(
    IMetadata* pHalMeta, IImageBuffer* pBuf)
{
    if(mpDebugDrawID != nullptr && mpDebugDrawID->needDraw()){
        if( pHalMeta == nullptr || pBuf == nullptr){
            MY_LOGW("meta(%p) buf(%p)", pHalMeta, pBuf);
            return;
        }

        MUINT64 timestamp = 0;

        IMetadata::IEntry entry;
        entry = pHalMeta->entryFor(MTK_P1NODE_FRAME_START_TIMESTAMP);
        if (!entry.isEmpty()){
            timestamp = entry.itemAt(0, Type2Type<MINT64>());
        }else{
            MY_LOGW("no timestamp found!");
        }

        MINT64 ts_long_ms = timestamp / 1000000;

        std::string timestampStr = std::to_string(timestamp);
        std::string timestampStr_ms = std::to_string(ts_long_ms);
        int timestamp_ms = std::stoi(timestampStr_ms);

        MY_LOGI("drawTimestamp ns(%s) ms(%d)",
            timestampStr.c_str(),
            timestamp_ms
        );

        mpDebugDrawID->draw(timestamp_ms,
                            12,  // digits
                            400, // x offset
                            200, // y offset
                            (char*)(pBuf->getBufVA(0)),
                            pBuf->getImgSize().w,
                            pBuf->getImgSize().h,
                            pBuf->getBufStridesInBytes(0),
                            pBuf->getBufSizeInBytes(0)
                            );
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PreProcessNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    Vector<NSDCMF::BufferConfig> vBufConfig;

    int allocateSize = 1;
    int prealloc_tuningSize = 4;
    int max_tuningSize = prealloc_tuningSize*getDCMFCaptureCnt();

    // alloc working buffer if needed
    // {
    //     NSBMDN::BufferConfig c = {
    //         "BID_XXX",
    //         BID_XXX,
    //         (MUINT32)some_width,
    //         (MUINT32)some_height,
    //         eImgFmt_XXX,
    //         ImageBufferPool::USAGE_HW,
    //         MTRUE,
    //         MFALSE,
    //         (MUINT32)allocateSize
    //     };
    //     vBufConfig.push_back(c);
    // }

    if(!mBufPool.init(vBufConfig, prealloc_tuningSize, max_tuningSize)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PreProcessNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
