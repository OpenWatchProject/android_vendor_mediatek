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

/**
 * @file BayerMonoQTemplateProvider.cpp
 * @brief QParams template creator for stereo feature on different platforms
 */

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "BayerMonoQTemplateProvider.h"
//
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "BayerMonoQTemplateProvider"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BayerMonoQTemplateProvider::
BayerMonoQTemplateProvider(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption,
    DepthMapFlowOption* pFlowOption
)
: BayerBayerQTemplateProvider(pSetting, pPipeOption, pFlowOption)
{
}

BayerMonoQTemplateProvider::
~BayerMonoQTemplateProvider()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerMonoQTemplateProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
BayerMonoQTemplateProvider::
init(BaseBufferSizeMgr* pSizeMgr)
{
    VSDOF_LOGD("+");
    mpSizeMgr = pSizeMgr;
    MBOOL bRet = BayerBayerQTemplateProvider::init(pSizeMgr);
    bRet &= prepareTemplateParams_Bayer();
    VSDOF_LOGD("-");
    return bRet;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParams_BAYER_NORMAL(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD("+");
    // copy template
    rOutParm = mQParam_BAYER_NORMAL;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm);

    MBOOL bRet = buildQParam_Bayer_NORMAL_frame0(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_Bayer_NORMAL_frame1(rEffReqPtr, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();

    VSDOF_LOGD("- res=%d", bRet);
    return bRet;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParams_NORMAL(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    CAM_TRACE_NAME("P2AFMNode::buildQParams_NORMAL");
    VSDOF_LOGD("+, reqID=%d", rEffReqPtr->getRequestNo());
    // copy template
    rOutParm = mQParam_NORMAL;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm);

    MBOOL bRet = buildQParam_frame0(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame1(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame2(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame3(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame4(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame5(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame6to9(rEffReqPtr, tuningResult, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParams_CAPTURE(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    CAM_TRACE_NAME("P2AFMNode::buildQParams_CAPTURE");
    VSDOF_LOGD("+, reqID=%d", rEffReqPtr->getRequestNo());
    // copy template
    rOutParm = mQParam_CAPTURE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm);

    MBOOL bRet = buildQParam_frame0(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame1_capture(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame2_capture(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame3_capture(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame4(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame5_capture(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_frame6to9(rEffReqPtr, tuningResult, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_Bayer_NORMAL_frame0(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // check exist Request FD buffer or not
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = rEffReqPtr->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // input IMGI
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW
    // check LCSO exist
    if(tuningRes.tuningResult.pLcsBuf == nullptr)
        rQFiller.delInputPort(iFrameNum, PORT_LCEI);
    else
    {
        IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(nodeID, PBID_IN_LCSO1);
        rQFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCSOBuf);
    }
    //
    if(bExistFD)
    {
        // insert FD output
        rQFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).
                setCrop(iFrameNum, eCROP_CRZ, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pFdBuf->getImgSize());
    }
    else
    {   // no FD-> remove port in template.
        rQFiller.delOutputPort(iFrameNum, PORT_IMG2O, eCROP_CRZ);
    }

    // CROP info
    MPoint ptCrop(0,0);
    MSize szCrop(frameBuf_RSRAW1->getImgSize().w, frameBuf_RSRAW1->getImgSize().h);
    IMetadata* pMeta_InHal = pBufferHandler->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    // get EIS crop if EIS is on
    if (rEffReqPtr->getRequestAttr().isEISOn)
    {
        eis_region region;
        if(queryEisRegion(pMeta_InHal, region))
        {
            ptCrop = MPoint(region.x_int, region.y_int);
            szCrop = region.s;
        }
    }
    // ouput: FE1B_input
    IImageBuffer* pImgBuf_MYS_INPUT = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    // output: MV_F
    IImageBuffer* pImgBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);

    rQFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, pImgBuf_MV_F).
                setCrop(iFrameNum, eCROP_WDMA, ptCrop, szCrop, pImgBuf_MV_F->getImgSize()).
                insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_MYS_INPUT).
                setCrop(iFrameNum, eCROP_WROT, ptCrop, szCrop, pImgBuf_MYS_INPUT->getImgSize());
    return MTRUE;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_Bayer_NORMAL_frame1(
    DepthMapRequestPtr& rEffReqPtr,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input : FE1B_input for MYS input
    IImageBuffer* pImgBuf_MYSInput = nullptr;
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pImgBuf_MYSInput);
    // output: MY_S
    IImageBuffer* pImgBuf_MY_S = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_MYSInput).
                insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_MY_S).
                setCrop(iFrameNum, eCROP_WROT, MPoint(0,0),
                        pImgBuf_MYSInput->getImgSize(), pImgBuf_MY_S->getImgSize()).
                insertTuningBuf(iFrameNum, pTuningBuf); // tuning data
    return MTRUE;
}


MBOOL
BayerMonoQTemplateProvider::
buildQParams_BAYER_CAPTURE(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD("+");
    // copy template
    rOutParm = mQParam_BAYER_CAPTURE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm);

    MBOOL bRet = buildQParam_Bayer_CAPTURE_frame0(rEffReqPtr, tuningResult, qParamFiller);
    bRet &= buildQParam_Bayer_CAPTURE_frame1(rEffReqPtr, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    VSDOF_LOGD("- res=%d", bRet);
    return bRet;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_Bayer_CAPTURE_frame0(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    //
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // input : FSRAW
    IImageBuffer* pImgBuf_FSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_FSRAW1);
    // output: FD
    IImageBuffer* pImgBuf_FD = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FDIMG);
    // output: MV_F_CAP
    IImageBuffer* pImgBuf_MV_F_CAP = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F_CAP);
    // ouput: FE1B_input
    IImageBuffer* pImgBuf_FE1BO_INPUT = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    #ifndef GTEST

    // LSC
    IImageBuffer* pLSC2Src = nullptr;
    if(tuningRes.tuningResult.pLsc2Buf != NULL)
    {
        // input: LSC2 buffer (for tuning)
        pLSC2Src = static_cast<IImageBuffer*>(tuningRes.tuningResult.pLsc2Buf);
        rQFiller.insertInputBuf(iFrameNum, PORT_DEPI, pLSC2Src);
    }
    else
    {
        MINT32 iRawType;
        IMetadata* pMeta_InHal = pBufferHandler->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
        MBOOL bRet = tryGetMetadata<MINT32>(pMeta_InHal, MTK_P1NODE_RAW_TYPE, iRawType);
        if(!bRet || iRawType == NSIoPipe::NSCamIOPipe::EPipe_PURE_RAW)
        {
            MY_LOGE("LSC2 Buf not exist! bRet=%d iRawType=%x", bRet, iRawType);
            return MFALSE;
        }
        else
        {
            rQFiller.delInputPort(iFrameNum, PORT_DEPI);
        }
    }
    // BPC
    IImageBuffer* pBpcBuf = NULL;
    if(tuningRes.tuningResult.pBpc2Buf != NULL)
    {
        pBpcBuf = static_cast<IImageBuffer*>(tuningRes.tuningResult.pBpc2Buf);;
        rQFiller.insertInputBuf(iFrameNum, PORT_DMGI, pBpcBuf);
    }
    else
        rQFiller.delInputPort(iFrameNum, PORT_DMGI);
    #endif
    // check LCSO exist
    if(tuningRes.tuningResult.pLcsBuf == nullptr)
        rQFiller.delInputPort(iFrameNum, PORT_LCEI);
    else
    {
        IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(nodeID, PBID_IN_LCSO1);
        rQFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCSOBuf);
    }
    // FOV crop: crop settings already configured at preparation stage
    // fill buffer
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
                insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_FSRAW1). // input: Main1 FSRAW
                insertOutputBuf(iFrameNum, PORT_IMG2O, pImgBuf_FD).   // output: FD
                setCropResize(iFrameNum, eCROP_CRZ, pImgBuf_FD->getImgSize()).
                insertOutputBuf(iFrameNum, PORT_WDMAO, pImgBuf_MV_F_CAP). // output: MV_F_CAP
                setCropResize(iFrameNum, eCROP_WDMA, pImgBuf_MV_F_CAP->getImgSize()).
                insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_FE1BO_INPUT). // output: FE1BO
                setCropResize(iFrameNum, eCROP_WROT, pImgBuf_FE1BO_INPUT->getImgSize());
    return MTRUE;

}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_Bayer_CAPTURE_frame1(
    DepthMapRequestPtr& rEffReqPtr,
    QParamTemplateFiller& rQFiller
)
{
    int iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input : FE1B_input for MYS input
    IImageBuffer* pImgBuf_FE1BO_INPUT= nullptr;
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pImgBuf_FE1BO_INPUT);
    // output: PostView
    IImageBuffer* pImgBuf_PostView = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_POSTVIEW);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // postview rotation
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(nodeID, BID_META_IN_APP);
    MINT32 jpegOrientation;
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
        return MFALSE;
    }
    MINT32 iModuleTrans = remapTransform((ENUM_ROTATION)jpegOrientation);

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_FE1BO_INPUT).
                insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_PostView).
                setOutputTransform(iFrameNum, mapToPortID(BID_P2A_OUT_POSTVIEW), iModuleTrans).
                setCrop(iFrameNum, eCROP_WROT, MPoint(0,0),
                        pImgBuf_FE1BO_INPUT->getImgSize(), pImgBuf_PostView->getImgSize()).
                insertTuningBuf(iFrameNum, pTuningBuf); // tuning data
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerBayerQTemplateProvider Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NSCam::NSIoPipe::PortID
BayerMonoQTemplateProvider::
mapToPortID(DepthMapBufferID bufferID)
{
    switch(bufferID)
    {
        case BID_P2A_OUT_MY_S:
            return PORT_WROTO;
        default:
            return BayerBayerQTemplateProvider::mapToPortID(bufferID);
    }
}


MBOOL
BayerMonoQTemplateProvider::
prepareQParam_frame1(
    DepthMapPipeOpState state,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    // B+M VSDOF: frame 1 is different from VSDOF.
    // B+M input is RRZO, output is N3D inputs
    MUINT iFrameNum = 1;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI);

    // crop information
    MPoint ptFE1B_CROP(0,0);
    MSize szTBD(0,0);

    // no FD and MV_F buffer for B+M mode
    MBOOL bSuccess =
        // WROT: FE1BO input
        generator.addCrop(eCROP_WROT, ptFE1B_CROP, szTBD, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(mapToPortID(BID_P2A_FE1B_INPUT), iModuleTrans).    // do module rotation
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_frame1_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    return this->buildQParam_frame1(rEffReqPtr, tuningResult, rQFiller);
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_frame3(
    DepthMapPipeOpState,
    QParams& rQParam
)
{
    // no output postview when B+M
    // no output MYS when B+M on P2AFM, MY_S generate by P2ABayer
    MUINT iFrameNum = 3;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    QParamTemplateGenerator templateGen =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addCrop(eCROP_CRZ, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, rP2AFMSize.mFEC_INPUT_SIZE_MAIN1).  // IMG2O: FE1CO input
        addOutput(mapToPortID(BID_P2A_FE1C_INPUT)).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).  // WDMA : Rect_in1
        addOutput(mapToPortID(BID_P2A_OUT_RECT_IN1)).
        #ifdef GTEST
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO). // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1));

    MBOOL bSuccess = templateGen.generate(rQParam);
    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_frame3(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // no output MY_S, which is output from P2ABayer
    MUINT iFrameNum = 3;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: FE1BBuf_in
    IImageBuffer* pFE1BImgBuf = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in =pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: Rect_in1
    IImageBuffer* pRectIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN1);
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;


    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), pFE1BImgBuf->getImgSize(), pRectIn1Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN1), pRectIn1Buf). // Rect_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_frame5(
	DepthMapPipeOpState,
	QParams& rQParam
)
{
    // no output postview when B+M
    MUINT iFrameNum = 5;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MBOOL bSuccess =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 5
        addInput(PORT_IMGI).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2AFMSize.mFEC_INPUT_SIZE_MAIN1, rP2AFMSize.mCCIN_SIZE_MAIN1).  // WDMA: CC_in1
        addOutput(mapToPortID(BID_P2A_OUT_CC_IN1)).
        #ifdef GTEST
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).   // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).
        generate(rQParam);

    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
buildQParam_frame1(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // FOV crop : already applied in P1 RRZO
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;

    // fill input buffer/tuning buffer
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW

    // ouput: FE1B_input
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pImgBuf). //FE1B_input
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf->getImgSize()).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    return MTRUE;
}
MBOOL
BayerMonoQTemplateProvider::
buildQParam_frame3_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes&,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 3;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: FE1BBuf_in
    IImageBuffer* pFE1BImgBuf = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in =pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: Rect_in1
    IImageBuffer* pRectIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN1);
    // Rect_in1 : set active area
    pRectIn1Buf->setExtParam(mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE).mRECT_IN_CONTENT_SIZE_MAIN1);
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), pFE1BImgBuf->getImgSize(), pRectIn1Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN1), pRectIn1Buf). // Rect_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}


MBOOL
BayerMonoQTemplateProvider::
buildQParam_frame5_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    return this->buildQParam_frame5(rEffReqPtr, tuningResult, rQFiller);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerMonoQTemplateProvider Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
BayerMonoQTemplateProvider::
prepareTemplateParams_Bayer()
{
    VSDOF_LOGD("+");
    // prepare QParams template
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

    MBOOL bRet = prepareQParamsTemplate_BAYER_NORMAL(iModuleTrans);
    bRet &= prepareQParamsTemplate_BAYER_CAPTURE(iModuleTrans);
    if(!bRet)
    {
        MY_LOGE("prepareQParamsTemplate failed!!!");
        return MFALSE;
    }
    VSDOF_LOGD("-");
    return MTRUE;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParamsTemplate_BAYER_NORMAL(MINT32 iModuleTrans)
{
    VSDOF_LOGD("+");

    MBOOL bSuccess = prepareQParam_Bayer_NORMAL_frame0(iModuleTrans, mQParam_BAYER_NORMAL);
    bSuccess &= prepareQParam_Bayer_NORMAL_frame1(mQParam_BAYER_NORMAL);

    VSDOF_LOGD("- : res=%d", bSuccess);
    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_Bayer_NORMAL_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MUINT iFrameNum = 0;

    // FOV crop already applied in RRZO
    MPoint ptOrigin(0, 0);
    MSize szNotSet(0, 0);
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addInput(PORT_LCEI).
            // IMG2O : FD
            addCrop(eCROP_CRZ, ptOrigin, szNotSet, rP2AFMSize.mFD_IMG_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_FDIMG)).
            // WDMA : MV_F
            addCrop(eCROP_WDMA, ptOrigin, szNotSet, rP2AFMSize.mMAIN_IMAGE_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_MV_F), 0, EPortCapbility_Disp).
            // WROT: same flow with P2AFM for MYS input
            addCrop(eCROP_WROT, ptOrigin, szNotSet, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(PORT_WROTO, iModuleTrans).                // do module rotation
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_Bayer_NORMAL_frame1(QParams& rQParam)
{
    //second bayer run - input: Bayer out, output: MY_S
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MUINT iFrameNum = 1;

    // FOV crop already applied in RRZO
    MPoint ptOrigin(0, 0);
    MSize szNotSet(0, 0);
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            // WROT: MY_S
            addCrop(eCROP_WROT, ptOrigin, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, rP2AFMSize.mMYS_SIZE).
            addOutput(PORT_WROTO).
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParamsTemplate_BAYER_CAPTURE(MINT32 iModuleTrans)
{
    VSDOF_LOGD("+");

    MBOOL bSuccess = prepareQParam_Bayer_CAPTURE_frame0(iModuleTrans, mQParam_BAYER_CAPTURE);
    bSuccess &= prepareQParam_Bayer_CAPTURE_frame1(iModuleTrans, mQParam_BAYER_CAPTURE);

    VSDOF_LOGD("- : res=%d", bSuccess);
    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_Bayer_CAPTURE_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);

    // need FOV crop
    MPoint ptFD_CROP = rP2AFMSize.mFD_IMG_CROP.startPt;
    MSize szFD_CROP = rP2AFMSize.mFD_IMG_CROP.contentSize();
    MPoint ptMVF_CROP = rP2AFMSize.mMAIN_IMAGE_CROP.startPt;
    MSize szMVF_CROP = rP2AFMSize.mMAIN_IMAGE_CROP.contentSize();
    MPoint ptBAYER_CROP = rP2AFMSize.mFEB_INPUT_CROP_MAIN1.startPt;
    MSize szBAYER_CROP = rP2AFMSize.mFEB_INPUT_CROP_MAIN1.contentSize();

    //
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            #ifndef GTEST
            // Need extra input port for LSC buffer
            addInput(PORT_DEPI).
            addInput(PORT_DMGI).
            addInput(PORT_LCEI).
            #endif
            // IMG2O : FD
            addCrop(eCROP_CRZ, ptFD_CROP, szFD_CROP, rP2AFMSize.mFD_IMG_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_FDIMG)).
            // WDMA : MV_F
            addCrop(eCROP_WDMA, ptMVF_CROP, szMVF_CROP, rP2AFMSize.mMAIN_IMAGE_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_MV_F), 0 ,EPortCapbility_Cap).
            // WROT: input for next run
            addCrop(eCROP_WROT, ptBAYER_CROP, szBAYER_CROP, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(PORT_WROTO).                // do module rotation
            generate(mQParam_BAYER_CAPTURE);

    return bSuccess;
}

MBOOL
BayerMonoQTemplateProvider::
prepareQParam_Bayer_CAPTURE_frame1(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    //second bayer run - input: Bayer out, output: PostView (no MY_S needed)
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);
    MUINT iFrameNum = 1;

    // FOV crop already applied in RRZO
    MPoint ptOrigin(0, 0);
    MSize szNotSet(0, 0);
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            // WROT: PostView
            addCrop(eCROP_WROT, ptOrigin, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, szNotSet).
            addOutput(PORT_WROTO).
            generate(rQParam);

    return bSuccess;
}


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

