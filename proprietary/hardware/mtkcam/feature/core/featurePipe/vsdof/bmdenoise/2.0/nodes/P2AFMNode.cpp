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
#include "P2AFMNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "P2AFMNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_BM_DENOISE "doBMDeNoise"
#define DO_MDP_ROT_BACK "doMDPRotBack"
#define DO_POST_PROCESS "doPostProcess"
#define DO_THUMBNAIL "doThumbnail"

// buffer alloc size
#define BUFFER_ALLOC_SIZE 1
#define TUNING_ALLOC_SIZE 3
#define SHADING_GAIN_SIZE 200

// debug settings
#define USE_DEFAULT_ISP 0
#define USE_DEFAULT_SHADING_GAIN 0
#define USE_DEFAULT_AMATRIX 0

#define DEFAULT_DYNAMIC_SHADING 32

// 2'comp, using ASL and ASR to do sign extension
#define TO_INT(a, bit) ((MINT32)((MINT32) (a)) << (32-(bit)) >> (32-(bit)))
#define OB_TO_INT(a) ((-(TO_INT((a), 13)))/4) /*10bit*/

#include <PipeLog.h>

#include <DpBlitStream.h>
#include "../util/vsdof_util.h"

#include <camera_custom_nvram.h>

#include <stereo_crop_util.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;

/*******************************************************************************
 *
 ********************************************************************************/
P2AFMNode::
P2AFMNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests);

    // Record the frame index of the L/R-Side which is defined by ALGO.
    // L-Side could be main1 or main2 which can be identified by the sensors' locations.
    // Frame 0,3,5 is main1, 1,2,6 is main2.
    if(STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())
    {
        frameIdx_LSide[0] = 3;      // frame index of Left Side(Main1)
        frameIdx_LSide[1] = 5;
        frameIdx_RSide[0] = 2;      // frame index of Right Side(Main2)
        frameIdx_RSide[1] = 4;
    }
    else
    {
        // Main1: R, Main2: L
        frameIdx_LSide[0] = 2;
        frameIdx_LSide[1] = 4;
        frameIdx_RSide[0] = 3;
        frameIdx_RSide[1] = 5;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
P2AFMNode::
~P2AFMNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onData(
    DataID id,
    ImgInfoMapPtr &imgInfo)
{
    MY_LOGD("onData(imgInfo) +");

    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_TO_P2AFM:
        case BSS_TO_P2AFM:
            mImgInfoRequests.enque(imgInfo);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    MY_LOGD("onData(imgInfo) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onUninit()
{
    CAM_TRACE_NAME("P2AFMNode::onUninit");
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
cleanUp()
{
    FUNC_START;
    if(mpINormalStream != nullptr)
    {
        mpINormalStream->uninit(PIPE_LOG_TAG);
        mpINormalStream->destroyInstance();
        mpINormalStream = nullptr;
    }
    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("BMDENOISE_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }

    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("BMDENOISE_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }

    mBufPool.uninit();
    FUNC_END;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
doP2AFM(EffectRequestPtr request)
{
    CAM_TRACE_NAME("P2AFMNode::doNonDeNoiseCapture");
    MY_LOGD("+, reqID=%d", request->getRequestNo());


    // todo
    MY_LOGD("-, reqID=%d", request->getRequestNo());
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onThreadStart()
{
    MY_LOGD("+");
    // Create NormalStream
    MY_LOGD("onInit=>create normalStream");
    CAM_TRACE_BEGIN("onInit=>create normalStream");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miOpenId);

    if (mpINormalStream == NULL)
    {
        MY_LOGE("mpINormalStream create instance for BMDeNoiseNode Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init(PIPE_LOG_TAG);
    CAM_TRACE_END();

    // BufferPoolSet init
    VSDOF_LOGD("BufferPoolSet init");
    CAM_TRACE_BEGIN("P2AFMNode::mBufPoolSet::init");
    initBufferPool();
    CAM_TRACE_END();

    // 3A: create instance
    MY_LOGD("onInit=>create_3A_instance senosrIdx:(%d/%d)", mSensorIdx_Main1, mSensorIdx_Main2);
    CAM_TRACE_BEGIN("onInit=>create_3A_instance");
    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);
    CAM_TRACE_END();

    // prepare templates
    prepareTemplateParams();

    // prepare burst trigger QParams
    MBOOL bRet = prepareBurstQParams();
    if(!bRet)
        MY_LOGE("prepareBurstQParams Error! Please check the error msg above!");

    MY_LOGD("-");

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onThreadLoop()
{
    FUNC_START;
    ImgInfoMapPtr imgInfo = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) )
    {
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }

    CAM_TRACE_NAME("P2AFMNode::onThreadLoop");

    // doInputDataDump(request);

    // mark on-going-request start
    this->incExtThreadDependency();

    //prepare enque parameter
    QParams enqueParams;
    EnquedBufPool *enquedData= new EnquedBufPool(this, imgInfo->getRequestPtr(), imgInfo, BMDeNoiseProcessId::P2AFM);

    VSDOF_PRFLOG("threadLoop start, reqID=%d", imgInfo->getRequestNo());

    MBOOL bRet;

    // copy the corresponding QParams template
    enqueParams = mBurstParamTmplate_Cap;
    bRet = buildQParams_CAP(imgInfo, enqueParams, enquedData);

    //debugQParams(enqueParams);
    if(!bRet)
    {
        MY_LOGE("Failed to build P2 enque parametes.");
        delete enquedData;
        this->decExtThreadDependency();
        return MFALSE;
    }

    // callback
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParams.mpCookie = (MVOID*) enquedData;

    enquedData->start();
    enquedData->mProcessId = BMDeNoiseProcessId::P2AFM;
    {
        CAM_TRACE_BEGIN("P2AFMNode::NormalStream::enque");
        AutoProfileUtil proflie(PIPE_LOG_TAG, "P2AFMNode::NormalStream::enque");
        bRet = mpINormalStream->enque(enqueParams);
        CAM_TRACE_END();
    }

    if(!bRet)
    {
        MY_LOGE("mpINormalStream enque failed! reqID=%d", imgInfo->getRequestNo());
        delete enquedData;
        this->decExtThreadDependency();
        return MFALSE;
    }

    VSDOF_PRFLOG("threadLoop end! reqID=%d", imgInfo->getRequestNo());

    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
prepareTemplateParams()
{
    VSDOF_LOGD("+");
    //SRZ: crop first, then resize.
    #define CONFIG_SRZINFO_TO_CROPAREA(SrzInfo, StereoArea) \
        SrzInfo.in_w =  StereoArea.size.w;\
        SrzInfo.in_h =  StereoArea.size.h;\
        SrzInfo.crop_w = StereoArea.size.w - StereoArea.padding.w;\
        SrzInfo.crop_h = StereoArea.size.h - StereoArea.padding.h;\
        SrzInfo.crop_x = StereoArea.startPt.x;\
        SrzInfo.crop_y = StereoArea.startPt.y;\
        SrzInfo.crop_floatX = 0;\
        SrzInfo.crop_floatY = 0;\
        SrzInfo.out_w = StereoArea.size.w - StereoArea.padding.w;\
        SrzInfo.out_h = StereoArea.size.h - StereoArea.padding.h;

    // FE SRZ template - frame 2/4 need FEO srz crop
    _SRZ_SIZE_INFO_ srzInfo_frame2;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame2, P2AFM_MAIN2_FEBO_AREA);
    mSrzSizeTemplateMap.add(2,  srzInfo_frame2);
    _SRZ_SIZE_INFO_ srzInfo_frame4;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame4, P2AFM_MAIN2_FECO_AREA);
    mSrzSizeTemplateMap.add(4,  srzInfo_frame4);

    // FM tuning template
    // frame 6/8 - forward +  frame 7/9 - backward
    for(int frameID=6;frameID<10;++frameID)
    {
        NSIoPipe::FMInfo fmInfo;
        setupEmptyTuningWithFM(fmInfo, frameID);
        mFMTuningBufferMap.add(frameID, fmInfo);
    }

    // prepare FE tuning buffer
    for (int iStage=1;iStage<=2;++iStage)
    {
        MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(iStage);
        NSIoPipe::FEInfo feInfo;
        StereoTuningProvider::getFETuningInfo(feInfo, iBlockSize);
        mFETuningBufferMap.add(iStage, feInfo);
    }

    VSDOF_LOGD("-");
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
prepareBurstQParams_CAP(MINT32 iModuleTrans)
{
    MY_LOGD("+");
    using namespace NSCam::NSIoPipe::NSPostProc;

    // difference with PV : extra LSC2 buffer, different MV_F size, different Rect_in size
    bool bSuccess;
    //--> frame 0
    // dummy size will be filled with the correct value when runtime.
    MSize dummySize(0,0);
    MPoint zeroPos(0,0);
    {
        bSuccess =
        QParamTemplateGenerator(0, mSensorIdx_Main2, ENormalStreamTag_Normal).   // frame 0
            addInput(PORT_IMGI).
            addCrop(eCROP_WROT, zeroPos, dummySize, P2AFM_FE2BO_INPUT_SIZE).  // WROT : FE2BO input
            addOutput(mapToPortID(BID_P2AFM_FE2B_INPUT), iModuleTrans).
            generate(mBurstParamTmplate_Cap);
    }

    //--> frame 1
    {

        bSuccess &=
        QParamTemplateGenerator(1, mSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 1
            addInput(PORT_IMGI).
            #ifndef GTEST
            addInput(PORT_DEPI).    // Capture need extra LSC2 buffer input
            #endif
            addCrop(eCROP_WROT, zeroPos, dummySize, P2AFM_FE1BO_INPUT_SIZE).  // WROT: FE1BO input
            addOutput(mapToPortID(BID_P2AFM_FE1B_INPUT), iModuleTrans).       // do module rotation
            generate(mBurstParamTmplate_Cap);
    }

    //--> frame 2
    {
        MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(2);

        bSuccess &=
        QParamTemplateGenerator(2, mSensorIdx_Main2, ENormalStreamTag_Normal).   // frame 2
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, zeroPos, P2AFM_FE2BO_INPUT_SIZE, P2AFM_FE2CO_INPUT_SIZE).  // IMG2O: FE2CO input
            addOutput(mapToPortID(BID_P2AFM_FE2C_INPUT)).
            addCrop(eCROP_WDMA, zeroPos, P2AFM_FE2BO_INPUT_SIZE, P2AFM_RECT_IN_CAP_SIZE).  // WDMA : Rect_in2
            addOutput(mapToPortID(BID_P2AFM_OUT_RECT_IN2_CAP)).
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1)).  // stage 1 FE
            generate(mBurstParamTmplate_Cap);
    }

    //--> frame 3
    {
        bSuccess &=
        QParamTemplateGenerator(3, mSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 3
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, zeroPos, P2AFM_FE1BO_INPUT_SIZE, P2AFM_FE1CO_INPUT_SIZE).  // IMG2O: FE1CO input
            addOutput(mapToPortID(BID_P2AFM_FE1C_INPUT)).
            addCrop(eCROP_WDMA, zeroPos, P2AFM_FE1BO_INPUT_SIZE, P2AFM_RECT_IN_CAP_SIZE).  // WDMA : Rect_in1
            addOutput(mapToPortID(BID_P2AFM_OUT_RECT_IN1_CAP)).
            addOutput(PORT_FEO).                           // FEO
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1)).  // stage 1 FE
            generate(mBurstParamTmplate_Cap);
    }

    //--> frame 4
    {
        MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(4);
        bSuccess &=
        QParamTemplateGenerator(4, mSensorIdx_Main2, ENormalStreamTag_Normal).   // frame 4
            addInput(PORT_IMGI).
            // CC_in change to WDMA
            //addCrop(eCROP_CRZ, zeroPos, mBufConfig.P2AFM_FE2CO_INPUT_SIZE, mBufConfig.P2AFM_CCIN_SIZE).  // IMG2O: CC_in2
            addCrop(eCROP_WDMA, zeroPos, P2AFM_FE2CO_INPUT_SIZE, P2AFM_CCIN_SIZE).  // IMG2O: CC_in2
            addOutput(mapToPortID(BID_P2AFM_OUT_CC_IN2)).
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).  // stage 2 FE
            generate(mBurstParamTmplate_Cap);
    }


    //--> frame 5
    {
        bSuccess &=
        QParamTemplateGenerator(5, mSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 5
            addInput(PORT_IMGI).
            //addCrop(eCROP_CRZ, zeroPos, mBufConfig.P2AFM_FE1CO_INPUT_SIZE, mBufConfig.P2AFM_CCIN_SIZE).  // IMG2O: CC_in1
            addCrop(eCROP_WDMA, zeroPos, P2AFM_FE1CO_INPUT_SIZE, P2AFM_CCIN_SIZE).  // IMG2O: CC_in1
            addOutput(mapToPortID(BID_P2AFM_OUT_CC_IN1)).
            addOutput(PORT_FEO).                           // FEO
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).  // stage 2 FE
            generate(mBurstParamTmplate_Cap);
    }

    //--> frame 6 ~ 9
    for(int frameID=6;frameID<10;++frameID)
    {
        bSuccess &=
        QParamTemplateGenerator(frameID, mSensorIdx_Main1, ENormalStreamTag_FM).
            addInput(PORT_DEPI).
            addInput(PORT_DMGI).
            addOutput(PORT_MFBO).
            addExtraParam(EPIPE_FM_INFO_CMD, (MVOID*)&mFMTuningBufferMap.valueFor(frameID)).
            generate(mBurstParamTmplate_Cap);
    }

    MY_LOGD("-");

    //MY_LOGD("debugQParams mBurstParamTmplate_Cap:");
    //debugQParams(mBurstParamTmplate_Cap);

    return bSuccess;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
buildQParams_CAP(ImgInfoMapPtr &imgInfo, QParams& rEnqueParam, EnquedBufPool* pEnquePoolData)
{
    CAM_TRACE_NAME("P2AFMNode::buildQParams_CAP");
    MINT32 reqNo = imgInfo->getRequestPtr()->getRequestNo();
    VSDOF_PRFLOG("+, reqID=%d", reqNo);
    // Get the input/output buffer inside the request
    sp<IImageBuffer> pBuf_INPUTRAW2, pBuf_INPUTRAW1;
    SmartImageBuffer poolBuf, fe2boBuf_in, fe1boBuf_in,  fe2coBuf_in, fe1coBuf_in;
    SmartImageBuffer rectIn1Buf, ccIn1Buf, ccIn2Buf, fDBuf;
    SmartGraphicBuffer rectIn2Buf;

    IMetadata* pMeta_InApp = imgInfo->getMetadata(BID_META_IN_APP);
    IMetadata* pMeta_InHal_main1 = imgInfo->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_InHal_main2 = imgInfo->getMetadata(BID_META_IN_HAL_MAIN2);
    IMetadata pMeta_InHal_Mod_main1 = (*pMeta_InHal_main1);
    IMetadata pMeta_InHal_Mod_main2 = (*pMeta_InHal_main2);
    SmartImageBuffer feoBuf[6];
    SmartTuningBuffer tuningBuf;

#if 0
    //Supports pipeRequest input with FSRAW or RSRAW
    if( imgInfo->getIImageBuffer(BID_INPUT_RSRAW_1) != nullptr && imgInfo->getIImageBuffer(BID_INPUT_RSRAW_2) != nullptr){
        MY_LOGD("P2AFM uses RS raws");
        pBuf_INPUTRAW1 = imgInfo->getIImageBuffer(BID_INPUT_RSRAW_1);
        pBuf_INPUTRAW2 = imgInfo->getIImageBuffer(BID_INPUT_RSRAW_2);
    }else if( imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1) != nullptr && imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2) != nullptr){
        MY_LOGD("P2AFM uses FS raws");
        pBuf_INPUTRAW1 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1);
        pBuf_INPUTRAW2 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2);
    }else{
        MY_LOGE("%p,%p,%p,%p, case not supported!",
            imgInfo->getIImageBuffer(BID_INPUT_RSRAW_1).get(),
            imgInfo->getIImageBuffer(BID_INPUT_RSRAW_2).get(),
            imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1).get(),
            imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2).get()
        );
        return MFALSE;
    }
#else
    if( imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1) != nullptr && imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2) != nullptr){
        MY_LOGD("P2AFM uses FS raws");
        pBuf_INPUTRAW1 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1);
        pBuf_INPUTRAW2 = imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2);
    }else{
        MY_LOGE("%p,%p,%p,%p, case not supported!",
            imgInfo->getIImageBuffer(BID_INPUT_RSRAW_1).get(),
            imgInfo->getIImageBuffer(BID_INPUT_RSRAW_2).get(),
            imgInfo->getIImageBuffer(BID_INPUT_FSRAW_1).get(),
            imgInfo->getIImageBuffer(BID_INPUT_FSRAW_2).get()
        );
        return MFALSE;
    }
#endif

    #define ADD_REQUEST_BUFFER_TO_MVOUT(bufID, reqBuf, filler, frameID, portID) \
        pEnquePoolData->addBuffData(bufID, reqBuf); \
        filler.insertOutputBuf(frameID, portID, reqBuf->mImageBuffer.get());

    #define ADD_REQUEST_GRAPHIC_BUFFER_TO_MVOUT(bufID, reqBuf, filler, frameID, portID) \
        pEnquePoolData->addGBuffData(bufID, reqBuf); \
        filler.insertOutputBuf(frameID, portID, reqBuf->mImageBuffer.get());

    #define ADD_REQUEST_BUFFER_WITH_AREA_TO_MVOUT(bufID, reqBuf, validSize, filler, frameID, portID) \
        pEnquePoolData->addBuffData(bufID, reqBuf); \
        reqBuf->mImageBuffer->setExtParam(validSize); \
        filler.insertOutputBuf(frameID, portID, reqBuf->mImageBuffer.get());

    QParamTemplateFiller qParamFiller(rEnqueParam);
    MPoint zeroPos(0,0);
    // Make sure the ordering inside the mvIn mvOut
    int mvInIndex = 0, mvOutIndex = 0;
    MUINT iFrameNum = 0;

    FrameInfoPtr framePtr_inHalMeta;
    //--> frame 0
    {
        // Apply tuning data
        VSDOF_PRFLOG("+, reqID=%d  frame 0, request tuning buffer!", reqNo);

        tuningBuf = mBufPool.getTuningBufPool()->request();

        // make sure the output is 16:9, get crop size& point
        MSize cropSizeMain2;
        MPoint startPointMain2;
        calCropForScreen(pBuf_INPUTRAW2, startPointMain2, cropSizeMain2);

        VSDOF_PRFLOG("+, reqID=%d  frame 0, add tuning buffer!", reqNo);
        pEnquePoolData->addTuningData(tuningBuf);
        ISPTuningConfig ispConfig = {pMeta_InApp, &pMeta_InHal_Mod_main2, mp3AHal_Main2, MTRUE, reqNo};
        TuningParam rTuningParam = applyISPTuning(tuningBuf, ispConfig, MFALSE);
        // insert tuning data
        #ifndef GTEST
            qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
        #else
            qParamFiller.insertTuningBuf(iFrameNum, nullptr);
        #endif
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pBuf_INPUTRAW2.get()). // input: Main2 RSRAW
            setCrop(iFrameNum, eCROP_WROT, startPointMain2, cropSizeMain2, P2AFM_FE2BO_INPUT_SIZE); // WROT crop for ZOOM

        // output: FE2BO input(WROT)
        fe2boBuf_in = mBufPool.getBufPool(BID_P2AFM_FE2B_INPUT)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_FE2B_INPUT, fe2boBuf_in, qParamFiller, iFrameNum,
                                    mapToPortID(BID_P2AFM_FE2B_INPUT));
    }

    //--> frame 1
    {
        iFrameNum = 1;
        // Apply tuning data
        tuningBuf = mBufPool.getTuningBufPool()->request();
        pEnquePoolData->addTuningData(tuningBuf);
        sp<IImageBuffer> frameBuf_MV_F;
        // Apply ISP tuning
        ISPTuningConfig ispConfig = {pMeta_InApp, &pMeta_InHal_Mod_main1, mp3AHal_Main1, MFALSE, reqNo};
        TuningParam rTuningParam = applyISPTuning(tuningBuf, ispConfig, MTRUE);
        // insert tuning data
        #ifndef GTEST
            qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
        #else
            qParamFiller.insertTuningBuf(iFrameNum, nullptr);
        #endif

        // UT does not test 3A
        #ifndef GTEST
        if(rTuningParam.pLsc2Buf != NULL)
        {
            // input: LSC2 buffer (for tuning)
            IImageBuffer* pLSC2Src = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
            qParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, pLSC2Src);
        }
        else
        {
            MY_LOGE("LSC2 buffer from 3A is NULL!!");
            return MFALSE;
        }
        #endif

        // make sure the output is 16:9, get crop size& point
        MSize cropSizeMain1;
        MPoint startPointMain1;
        calCropForScreen(pBuf_INPUTRAW1, startPointMain1, cropSizeMain1);

        // input: Main1 Resize RAW
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pBuf_INPUTRAW1.get());

        // ouput: FE1BO_input
        fe1boBuf_in = mBufPool.getBufPool(BID_P2AFM_FE1B_INPUT)->request();

        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_FE1B_INPUT, fe1boBuf_in, qParamFiller, iFrameNum,
                                    mapToPortID(BID_P2AFM_FE1B_INPUT));
        // FE1BO input crop
        qParamFiller.setCrop(iFrameNum, eCROP_WROT, startPointMain1, cropSizeMain1, P2AFM_FE1BO_INPUT_SIZE);
    }

    //--> frame 2
    {
        iFrameNum = 2;
        // input: fe2boBuf_in
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2boBuf_in->mImageBuffer.get());
        // output: FE2CO input
        fe2coBuf_in = mBufPool.getBufPool(BID_P2AFM_FE2C_INPUT)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_FE2C_INPUT, fe2coBuf_in, qParamFiller, iFrameNum,
                                    mapToPortID(BID_P2AFM_FE2C_INPUT));
        // output: Rect_in2
        rectIn2Buf = mBufPool.getGraphicBufPool(BID_P2AFM_OUT_RECT_IN2_CAP)->request();
        ADD_REQUEST_GRAPHIC_BUFFER_TO_MVOUT(BID_P2AFM_OUT_RECT_IN2_CAP, rectIn2Buf, qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_RECT_IN2_CAP));

        // output: FE2BO
        feoBuf[iFrameNum] = mBufPool.getBufPool(BID_P2AFM_OUT_FE2BO)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FE2BO, feoBuf[iFrameNum], qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FE2BO));
        // tuning data
        tuningBuf = mBufPool.getTuningBufPool()->request();
        pEnquePoolData->addTuningData(tuningBuf);
        qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
    }

    //--> frame 3
    {
        iFrameNum = 3;
        // input: fe1boBuf_in
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe1boBuf_in->mImageBuffer.get());
        // output: FE1CO input
        fe1coBuf_in = mBufPool.getBufPool(BID_P2AFM_FE1C_INPUT)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_FE1C_INPUT, fe1coBuf_in, qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_FE1C_INPUT));

        // output: Rect_in1
        rectIn1Buf = mBufPool.getBufPool(BID_P2AFM_OUT_RECT_IN1_CAP)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_RECT_IN1_CAP, rectIn1Buf, qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_RECT_IN1_CAP));

        // output: FE1BO
        feoBuf[iFrameNum] = mBufPool.getBufPool(BID_P2AFM_OUT_FE1BO)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FE1BO, feoBuf[iFrameNum], qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FE1BO));

        // tuning data
        tuningBuf = mBufPool.getTuningBufPool()->request();
        pEnquePoolData->addTuningData(tuningBuf);
        qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
    }

    //--> frame 4
    {
        iFrameNum = 4;
        // input: fe2coBuf_in
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2coBuf_in->mImageBuffer.get());
        // output: CC_in
        ccIn2Buf = mBufPool.getBufPool(BID_P2AFM_OUT_CC_IN2)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_CC_IN2, ccIn2Buf, qParamFiller, iFrameNum,
                                    mapToPortID(BID_P2AFM_OUT_CC_IN2));
        // output: FE2CO
        feoBuf[iFrameNum] = mBufPool.getBufPool(BID_P2AFM_OUT_FE2CO)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FE2CO, feoBuf[iFrameNum], qParamFiller, iFrameNum,
                                    mapToPortID(BID_P2AFM_OUT_FE2CO));

        // tuning data
        tuningBuf = mBufPool.getTuningBufPool()->request();
        pEnquePoolData->addTuningData(tuningBuf);
        qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
    }
    //--> frame 5
    {
        iFrameNum = 5;
        // input: fe1coBuf_in
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe1coBuf_in->mImageBuffer.get());
        // output: CC_in
        ccIn1Buf = mBufPool.getBufPool(BID_P2AFM_OUT_CC_IN1)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_CC_IN1, ccIn1Buf, qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_CC_IN1));
        // output: FE1CO
        feoBuf[iFrameNum] = mBufPool.getBufPool(BID_P2AFM_OUT_FE1CO)->request();
        ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FE1CO, feoBuf[iFrameNum], qParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FE1CO));

        // tuning data
        tuningBuf = mBufPool.getTuningBufPool()->request();
        pEnquePoolData->addTuningData(tuningBuf);
        qParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
    }

    // build FE/FM frame
    _buildFEFMFrame(qParamFiller, feoBuf, pEnquePoolData);

    VSDOF_PRFLOG("-, reqID=%d", reqNo);

    MBOOL bRet = qParamFiller.validate();

    return bRet;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
_buildFEFMFrame(QParamTemplateFiller& rQParamFiller, SmartImageBuffer feoBuf[], EnquedBufPool* pEnquePoolData)
{
    #define ADD_REQUEST_BUFFER_TO_MVOUT(bufID, reqBuf, filler, frameID, portID) \
        pEnquePoolData->addBuffData(bufID, reqBuf); \
        filler.insertOutputBuf(frameID, portID, reqBuf->mImageBuffer.get());

    #define ADD_REQUEST_BUFFER_WITH_AREA_TO_MVOUT(bufID, reqBuf, validSize, filler, frameID, portID) \
        pEnquePoolData->addBuffData(bufID, reqBuf); \
        reqBuf->mImageBuffer->setExtParam(validSize); \
        filler.insertOutputBuf(frameID, portID, reqBuf->mImageBuffer.get());
    SmartTuningBuffer tuningBuf;
    //--> frame 6: FM - L to R
    int iFrameNum = 6;
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_LSide[0]]->mImageBuffer.get());
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_RSide[0]]->mImageBuffer.get());
    SmartImageBuffer fmoBuf = mBufPool.getBufPool(BID_P2AFM_OUT_FMBO_LR)->request();
    ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FMBO_LR, fmoBuf, rQParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FMBO_LR));
    // tuning data
    tuningBuf = mBufPool.getTuningBufPool()->request();
    pEnquePoolData->addTuningData(tuningBuf);
    rQParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);

    //--> frame 7: FM - R to L
    iFrameNum = 7;
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_RSide[0]]->mImageBuffer.get());
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_LSide[0]]->mImageBuffer.get());
    fmoBuf = mBufPool.getBufPool(BID_P2AFM_OUT_FMBO_RL)->request();
    ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FMBO_RL, fmoBuf, rQParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FMBO_RL));
    // tuning data
    tuningBuf = mBufPool.getTuningBufPool()->request();
    pEnquePoolData->addTuningData(tuningBuf);
    rQParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);

    //--> frame 8: FM - L to R
    iFrameNum = 8;
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_LSide[1]]->mImageBuffer.get());
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_RSide[1]]->mImageBuffer.get());
    fmoBuf = mBufPool.getBufPool(BID_P2AFM_OUT_FMCO_LR)->request();
    ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FMCO_LR, fmoBuf, rQParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FMCO_LR));
    // tuning data
    tuningBuf = mBufPool.getTuningBufPool()->request();
    pEnquePoolData->addTuningData(tuningBuf);
    rQParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);

    ///--> frame 9: FM
    iFrameNum = 9;
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_RSide[1]]->mImageBuffer.get());
    rQParamFiller.insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_LSide[1]]->mImageBuffer.get());
    fmoBuf = mBufPool.getBufPool(BID_P2AFM_OUT_FMCO_RL)->request();
    ADD_REQUEST_BUFFER_TO_MVOUT(BID_P2AFM_OUT_FMCO_RL, fmoBuf, rQParamFiller, iFrameNum, mapToPortID(BID_P2AFM_OUT_FMCO_RL));
    // tuning data
    tuningBuf = mBufPool.getTuningBufPool()->request();
    pEnquePoolData->addTuningData(tuningBuf);
    rQParamFiller.insertTuningBuf(iFrameNum, tuningBuf->mpVA);
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
setupEmptyTuningWithFM(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameID)
{
    MY_LOGD("+");

    MSize szFEBufSize = (iFrameID<=7) ? P2AFM_FE1BO_INPUT_SIZE : P2AFM_FE1CO_INPUT_SIZE;
    MUINT32 iStage = (iFrameID<=7) ? 1 : 2;

    ENUM_FM_DIRECTION eDir = (iFrameID % 2 == 0) ? E_FM_L_TO_R : E_FM_R_TO_L;
    // query tuning parameter
    StereoTuningProvider::getFMTuningInfo(eDir, fmInfo);
    MUINT32 iBlockSize =  StereoSettingProvider::fefmBlockSize(iStage);
    // set width/height
    fmInfo.mFMWIDTH = szFEBufSize.w/iBlockSize;
    fmInfo.mFMHEIGHT = szFEBufSize.h/iBlockSize;

    MY_LOGD("-");
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
prepareBurstQParams()
{
    MY_LOGD("+");
    using namespace NSCam::NSIoPipe::NSPostProc;
    QParams burstParams;

    // module rotation
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    MINT32 iModuleTrans = -1;
    switch(eRot)
    {
        case eRotate_0:
            iModuleTrans = 0;
            break;
        case eRotate_90:
            iModuleTrans = eTransform_ROT_90;
            break;
        case eRotate_180:
            iModuleTrans = eTransform_ROT_180;
            break;
        case eRotate_270:
            iModuleTrans = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("Not support module rotation =%d", eRot);
            return MFALSE;
    }
    MBOOL bRet = prepareBurstQParams_CAP(iModuleTrans);
    MY_LOGD("-");
    return bRet;
}

/*******************************************************************************
 *
 ********************************************************************************/
TuningParam
P2AFMNode::
applyISPTuning(
        SmartTuningBuffer& targetTuningBuf,
        const ISPTuningConfig& ispConfig,
        MBOOL isMain1
)
{
    CAM_TRACE_NAME("P2AFMNode::applyISPTuning");
    VSDOF_PRFLOG("+, reqID=%d bIsResized=%d", ispConfig.reqNo, ispConfig.bInputResizeRaw);

    TuningParam tuningParam = {NULL, NULL};
    tuningParam.pRegBuf = reinterpret_cast<void*>(targetTuningBuf->mpVA);

    MetaSet_T inMetaSet;
    inMetaSet.appMeta = (*ispConfig.pInAppMeta);
    inMetaSet.halMeta = (*ispConfig.pInHalMeta);

    // USE resize raw-->set PGN 0
    if(ispConfig.bInputResizeRaw)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
    {
        // capture scenario: set capture isp profile
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Capture);
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    }

    // main1 path need to-W profile
    if(isMain1)
    {
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Capture_toW);
    }
    else
    {
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Capture);
    }

    SimpleTimer timer(true);
    // UT do not test setIsp
    #ifndef GTEST
    MetaSet_T resultMeta;
    ispConfig.p3AHAL->setIsp(0, inMetaSet, &tuningParam, &resultMeta);
    // write ISP resultMeta to input hal Meta
    // DONT DO THIS! hal meta write back is done by DeNoiseNode.
    // (*pMeta_InHal) += resultMeta.halMeta;
    #endif

    VSDOF_PRFLOG("-, reqID=%d setIsp_time=%f", ispConfig.reqNo, timer.countTimer());

    return tuningParam;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
onP2Callback(QParams& rParams)
{
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    P2AFMNode* pP2AFMNode = (P2AFMNode*) (pEnqueData->mpNode);
    pP2AFMNode->handleP2Done(rParams, pEnqueData);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
onP2FailedCallback(QParams& rParams)
{
    MY_LOGE("P2A operations failed!!Check the following log:");
    EnquedBufPool* pEnqueData = (EnquedBufPool*) (rParams.mpCookie);
    P2AFMNode* pP2AFMNode = (P2AFMNode*) (pEnqueData->mpNode);
    //pP2AFMNode->debugQParams(rParams);
    pP2AFMNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mPipeRequest);
    delete pEnqueData;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
handleP2Done(QParams& rParams, EnquedBufPool* pEnqueBufPool)
{
    CAM_TRACE_NAME("P2AFMNode::handleP2Done");
    PipeRequestPtr request = pEnqueBufPool->mPipeRequest;
    pEnqueBufPool->stop();
    MY_LOGD("BMDeNoise_Profile: %s time(%d ms) reqID=%d",
        getProcessName(pEnqueBufPool->mProcessId),
        pEnqueBufPool->getElapsed(),
        request->getRequestNo()
    );

    // dumping buffers
    dumpingP2Buffers(pEnqueBufPool);

    // prepare data for N3D input
    SmartImageBuffer smBuf;
    #define ADD_ENQUE_BUFF_TO_INFO_MAP(infoMap, enqueBufPool, BID)\
        smBuf = enqueBufPool->mEnquedSmartImgBufMap.valueFor(BID); \
        infoMap->addSmartBuffer(BID, smBuf);

    SmartGraphicBuffer smgBuf;
    #define ADD_ENQUE_GRAPHIC_BUFF_TO_INFO_MAP(infoMap, enqueBufPool, BID)\
        smgBuf = enqueBufPool->mEnquedSmartGBufMap.valueFor(BID); \
        infoMap->addGraphicBuffer(BID, smgBuf);
    // FEO/FMO

    sp<ImageBufInfoMap> TON3D_ImgBufInfo = new ImageBufInfoMap(request);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FE1BO);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FE2BO);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FE1CO);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FE2CO);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FMBO_LR);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FMBO_RL);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FMCO_LR);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_FMCO_RL);
    // CC_in
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_CC_IN1);
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_CC_IN2);
    // Rect_in
    ADD_ENQUE_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_RECT_IN1_CAP);
    ADD_ENQUE_GRAPHIC_BUFF_TO_INFO_MAP(TON3D_ImgBufInfo, pEnqueBufPool, BID_P2AFM_OUT_RECT_IN2_CAP);

    // pass to N3D
    handleData(P2AFM_TO_N3D_FEFM_CCin, TON3D_ImgBufInfo);
    // release enque cookie
    delete pEnqueBufPool;
    VSDOF_PRFLOG("- :reqID=%d", request->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();

    #undef ADD_ENQUE_BUFF_TO_INFO_MAP
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
doDataDump(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx)
{
    MY_LOGD("doDataDump: BID:%d +", BID);

    char filepath[1024];
    snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    const char* writeFileName = getBIDName(BID);

    char writepath[1024];
    snprintf(writepath,
        1024, "%s/%s_%dx%d.yuv", filepath, writeFileName,
        pBuf->getImgSize().w, pBuf->getImgSize().h);

    pBuf->saveToFile(writepath);

    MY_LOGD("doDataDump: BID:%d -", BID);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
doInputDataDump(EffectRequestPtr request)
{
    // do data dump
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
dumpingP2Buffers(EnquedBufPool* pEnqueBufPool)
{
// TODO:
// Move to the beginning of the request
#if 0
    EffectRequestPtr request = pEnqueBufPool->mRequest;
    SmartImageBuffer smBuf;

    #define ADD_ENQUE_BUFF_TO_INFO_MAP(infoMap, enqueBufPool, BID)\
        smBuf = enqueBufPool->mEnquedSmartImgBufMap.valueFor(BID); \
        infoMap->addImageBuffer(BID, smBuf);

    // Dump P1Raw
    if(checkToDump(TO_DUMP_RAWS))
    {
        FrameInfoPtr framePtr;
        sp<IImageBuffer> frameBuf;
        char postfix[50]={'\0'};
        if(request->vInputFrameInfo.indexOfKey(BID_INPUT_RSRAW_1)>=0)
        {
            framePtr = request->vInputFrameInfo.valueFor(BID_INPUT_RSRAW_1);
            framePtr->getFrameBuffer(frameBuf);
            // snprintf(postfix, 50, "_%d.raw", frameBuf->getBufStridesInBytes(0));
            // handleDump(TO_DUMP_RAWS, framePtr, "BID_INPUT_RSRAW_1", postfix);
            handleDump(frameBuf.get(), BID_INPUT_RSRAW_1, request->getRequestNo());
        }
        if(request->vInputFrameInfo.indexOfKey(BID_INPUT_RSRAW_2)>=0)
        {
            framePtr = request->vInputFrameInfo.valueFor(BID_INPUT_RSRAW_2);
            framePtr->getFrameBuffer(frameBuf);
            // snprintf(postfix, 50, "_%d.raw", frameBuf->getBufStridesInBytes(0));
            // handleDump(TO_DUMP_RAWS, framePtr, "BID_INPUT_RSRAW_2", postfix);
            handleDump(frameBuf.get(), BID_INPUT_RSRAW_2, request->getRequestNo());
        }
    }

    #undef ADD_ENQUE_BUFF_TO_INFO_MAP
#endif
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
doOutputDataDump(EffectRequestPtr request, EnquedBufPool* pEnqueBufPool)
{
    //
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    MY_LOGD("+ : initBufferPool");

    StereoSizeProvider* pSizePrvder = StereoSizeProvider::getInstance();

    // PV/VR - frame 0
    Pass2SizeInfo pass2SizeInfo;
    pSizePrvder->getPass2SizeInfo(PASS2A_P, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_FE2BO_INPUT_SIZE = pass2SizeInfo.areaWROT;
    P2AFM_MAIN2_FEAO_AREA = pass2SizeInfo.areaFEO;
    P2AFM_FEAO_INPUT_SIZE = pass2SizeInfo.areaFEO.size;

    // frame 1
    pSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_FD_IMG_SIZE = pass2SizeInfo.areaIMG2O;
    P2AFM_MAIN_IMAGE_SIZE = pass2SizeInfo.areaWDMA.size;
    P2AFM_FE1BO_INPUT_SIZE = pass2SizeInfo.areaWROT;

    // frame 2
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_FE2CO_INPUT_SIZE = pass2SizeInfo.areaIMG2O;
    P2AFM_RECT_IN_VR_SIZE_MAIN2 = pass2SizeInfo.areaWDMA.size;
    P2AFM_MAIN2_FEBO_AREA = pass2SizeInfo.areaFEO;

    // frame 3
    pSizePrvder->getPass2SizeInfo(PASS2A_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_FE1CO_INPUT_SIZE = pass2SizeInfo.areaIMG2O;
    P2AFM_RECT_IN_VR_SIZE_MAIN1 = pass2SizeInfo.areaWDMA.size;

    // frame 4
    pSizePrvder->getPass2SizeInfo(PASS2A_P_3, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_MAIN2_FECO_AREA = pass2SizeInfo.areaFEO;
    // cc_in
    P2AFM_CCIN_SIZE = pass2SizeInfo.areaIMG2O;

    // PV - frame 2
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_RECT_IN_PV_SIZE_MAIN2 = pass2SizeInfo.areaWDMA.size;
    // frame 3
    pSizePrvder->getPass2SizeInfo(PASS2A_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_RECT_IN_PV_SIZE_MAIN1 = pass2SizeInfo.areaWDMA.size;

    // CAP
    // frame 1
    pSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_MAIN_IMAGE_CAP_SIZE = pass2SizeInfo.areaWDMA.size;

    // frame 2
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    P2AFM_RECT_IN_CAP_SIZE = pass2SizeInfo.areaWDMA.size;
    P2AFM_RECT_IN_CAP_IMG_SIZE = pass2SizeInfo.areaWDMA.size - pass2SizeInfo.areaWDMA.padding;

    //FE FM Buffer pool
    MUINT32 fe_width=0, fe_height=0;
    MUINT32 fm_width=0, fm_height=0;

    MUINT32 iBlockSize;

    Vector<NSBMDN::BufferConfig> vBufConfig;

    if(P2AFM_CONST_FE_EXEC_TIMES== 2)
    {
        // stage 2 - FEFM
        MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(1);
        // calculate the buffers which is the input for FE HW, use Main1 version/because Main2 has SRC CROP
        MSize szFEBufSize = P2AFM_FE1BO_INPUT_SIZE;
        queryFEOBufferSize(szFEBufSize, iBlockSize, fe_width, fe_height);
        queryFMOBufferSize(fe_width, fe_height, fm_width, fm_height);

        {
            NSBMDN::BufferConfig c = {
                "mpFMOB_BufPool_LR",
                BID_P2AFM_OUT_FMBO_LR,
                (MUINT32)fm_width,
                (MUINT32)fm_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFMOB_BufPool_RL",
                BID_P2AFM_OUT_FMBO_RL,
                (MUINT32)fm_width,
                (MUINT32)fm_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFE2OB_BufPool",
                BID_P2AFM_OUT_FE2BO,
                (MUINT32)fe_width,
                (MUINT32)fe_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFE1OB_BufPool",
                BID_P2AFM_OUT_FE1BO,
                (MUINT32)fe_width,
                (MUINT32)fe_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        // stage 3 - FEFM
        iBlockSize = StereoSettingProvider::fefmBlockSize(2);
        // calculate the buffers which is the input for FE HW, use Main1 version/because Main2 has SRC CROP
        szFEBufSize = P2AFM_FE1CO_INPUT_SIZE;
        queryFEOBufferSize(szFEBufSize, iBlockSize, fe_width, fe_height);
        queryFMOBufferSize(fe_width, fe_height, fm_width, fm_height);

        {
            NSBMDN::BufferConfig c = {
                "mpFMOC_BufPool_LR",
                BID_P2AFM_OUT_FMCO_LR,
                (MUINT32)fm_width,
                (MUINT32)fm_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFMOC_BufPool_RL",
                BID_P2AFM_OUT_FMCO_RL,
                (MUINT32)fm_width,
                (MUINT32)fm_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFE2OC_BufPool",
                BID_P2AFM_OUT_FE2CO,
                (MUINT32)fe_width,
                (MUINT32)fe_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

        {
            NSBMDN::BufferConfig c = {
                "mpFE1OC_BufPool",
                BID_P2AFM_OUT_FE1CO,
                (MUINT32)fe_width,
                (MUINT32)fe_height,
                eImgFmt_STA_BYTE,
                ImageBufferPool::USAGE_HW,
                MTRUE,
                MFALSE,
                (MUINT32)(P2AFM_WORKING_BUF_SET)
            };
            vBufConfig.push_back(c);
        }

    }
    else
    {
        MY_LOGE("P2AFM_CONST_FE_EXEC_TIMES const error! val = %d", P2AFM_CONST_FE_EXEC_TIMES);
    }

    // Capture RECT_IN1
    {
        NSBMDN::BufferConfig c = {
            "RECT_IN1",
            BID_P2AFM_OUT_RECT_IN1_CAP,
            (MUINT32)P2AFM_RECT_IN_CAP_SIZE.w,
            (MUINT32)P2AFM_RECT_IN_CAP_SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    // Capture IN2(graphic buffer)
    {
        NSBMDN::BufferConfig c = {
            "RECT_IN2",
            BID_P2AFM_OUT_RECT_IN2_CAP,
            (MUINT32)P2AFM_RECT_IN_CAP_SIZE.w,
            (MUINT32)P2AFM_RECT_IN_CAP_SIZE.h,
            eImgFmt_YV12,
            GraphicBufferPool::USAGE_HW_TEXTURE,
            MTRUE,
            MTRUE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    //FEBO_Main1 input
    {
        NSBMDN::BufferConfig c = {
            "mpFEBOInBufPool_Main1",
            BID_P2AFM_FE1B_INPUT,
            (MUINT32)P2AFM_FE1BO_INPUT_SIZE.w,
            (MUINT32)P2AFM_FE1BO_INPUT_SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    //FEBO_Main2 input
    {
        NSBMDN::BufferConfig c = {
            "mpFEBOInBufPool_Main2",
            BID_P2AFM_FE2B_INPUT,
            (MUINT32)P2AFM_FE2BO_INPUT_SIZE.w,
            (MUINT32)P2AFM_FE2BO_INPUT_SIZE.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    //FECO_Main1 input
    {
        NSBMDN::BufferConfig c = {
            "mpFECOInBufPool_Main1",
            BID_P2AFM_FE1C_INPUT,
            (MUINT32)P2AFM_FE1CO_INPUT_SIZE.w,
            (MUINT32)P2AFM_FE1CO_INPUT_SIZE.h,
            eImgFmt_YUY2,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    //FECO_Main2 input
    {
        NSBMDN::BufferConfig c = {
            "mpFECOInBufPool_Main2",
            BID_P2AFM_FE2C_INPUT,
            (MUINT32)P2AFM_FE2CO_INPUT_SIZE.w,
            (MUINT32)P2AFM_FE2CO_INPUT_SIZE.h,
            eImgFmt_YUY2,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    // CC_in1
    {
        NSBMDN::BufferConfig c = {
            "mpCCIn1BufPool",
            BID_P2AFM_OUT_CC_IN1,
            (MUINT32)P2AFM_CCIN_SIZE.w,
            (MUINT32)P2AFM_CCIN_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    // CC_in2
    {
        NSBMDN::BufferConfig c = {
            "mpCCIn2BufPool",
            BID_P2AFM_OUT_CC_IN2,
            (MUINT32)P2AFM_CCIN_SIZE.w,
            (MUINT32)P2AFM_CCIN_SIZE.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)(P2AFM_WORKING_BUF_SET)
        };
        vBufConfig.push_back(c);
    }

    if(!mBufPool.init(vBufConfig, P2AFM_TUNING_BUF_SET)){
        MY_LOGE("Error! lPease check above errors!");
    }

    MY_LOGD("- : initBufferPool ");
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
queryFEOBufferSize(MSize iBufSize, MUINT iBlockSize, MUINT32 &riFEOWidth, MUINT32 &riFEOHeight)
{
    riFEOWidth = iBufSize.w/iBlockSize*40;
    riFEOHeight = iBufSize.h/iBlockSize;
    MY_LOGD("queryFEOBufferSize: iBufSize=%dx%d  ouput=%dx%d", iBufSize.w, iBufSize.h, riFEOWidth, riFEOHeight);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2AFMNode::
queryFMOBufferSize(MUINT32 iFEOWidth, MUINT32 iFEOHeight, MUINT32 &riFMOWidth, MUINT32 &riFMOHeight)
{
    riFMOWidth = (iFEOWidth/40) * 2;
    riFMOHeight = iFEOHeight;

    MY_LOGD("queryFMOBufferSize: iFEOWidth=%d iFEOHeight=%d  ouput=%dx%d", iFEOWidth, iFEOHeight, riFMOWidth, riFMOHeight);
}

/*******************************************************************************
 *
 ********************************************************************************/
NSCam::NSIoPipe::PortID
P2AFMNode::
mapToPortID(const MUINT32 nodeDataType)
{
    switch(nodeDataType)
    {
        case BID_INPUT_FSRAW_1:
        case BID_INPUT_FSRAW_2:
        case BID_INPUT_RSRAW_1:
        case BID_INPUT_RSRAW_2:
            return PORT_IMGI;

        case BID_P2AFM_OUT_FDIMG:
        case BID_P2AFM_FE1C_INPUT:
        case BID_P2AFM_FE2C_INPUT:
            return PORT_IMG2O;

        case BID_P2AFM_OUT_FE1BO:
        case BID_P2AFM_OUT_FE2BO:
        case BID_P2AFM_OUT_FE1CO:
        case BID_P2AFM_OUT_FE2CO:
            return PORT_FEO;

        case BID_P2AFM_OUT_RECT_IN1:
        case BID_P2AFM_OUT_RECT_IN2:
        case BID_P2AFM_OUT_RECT_IN2_CAP:
        case BID_P2AFM_OUT_RECT_IN1_CAP:
        case BID_P2AFM_OUT_MV_F:
        case BID_P2AFM_OUT_MV_F_CAP:
            return PORT_WDMAO;

        case BID_P2AFM_FE1B_INPUT:
        case BID_P2AFM_FE2B_INPUT:
            return PORT_WROTO;

        case BID_P2AFM_OUT_CC_IN1:
        case BID_P2AFM_OUT_CC_IN2:
            return PORT_WDMAO;

        case BID_P2AFM_OUT_FMBO_LR:
        case BID_P2AFM_OUT_FMBO_RL:
        case BID_P2AFM_OUT_FMCO_LR:
        case BID_P2AFM_OUT_FMCO_RL:
            return PORT_MFBO;

        default:
            MY_LOGE("mapToPortID: not exist nodeDataType=%d", nodeDataType);
            break;
    }

    return NSCam::NSIoPipe::PortID();
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2AFMNode::
calCropForScreen(sp<IImageBuffer> pFrameBuf, MPoint &rCropStartPt, MSize& rCropSize )
{
    MSize srcSize = pFrameBuf->getImgSize();

    MRect rect;
    rect.p.x = 0;
    rect.p.y = 0;
    rect.s.w = srcSize.w;
    rect.s.h = srcSize.h;

    CropUtil::cropRectByImageRatio(rect, StereoSettingProvider::imageRatio());

    rCropStartPt.x = rect.p.x;
    rCropStartPt.y = rect.p.y;
    rCropSize.w = rect.s.w;
    rCropSize.h = rect.s.h;

    VSDOF_LOGD("calCropForScreen rCropStartPt: (%d, %d), \
                    rCropSize: %dx%d ", rCropStartPt.x, rCropStartPt.y, rCropSize.w, rCropSize.h);

    return MTRUE;

}
