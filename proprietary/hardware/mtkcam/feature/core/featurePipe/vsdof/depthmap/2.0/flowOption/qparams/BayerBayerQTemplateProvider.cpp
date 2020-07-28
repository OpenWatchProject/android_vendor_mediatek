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
 * @file BayerBayerQTemplateProvider.cpp
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
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "BayerBayerQTemplateProvider.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "BayerBayerQTemplateProvider"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace StereoHAL;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BayerBayerQTemplateProvider::
BayerBayerQTemplateProvider(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption,
    DepthMapFlowOption* pFlowOption
)
: miSensorIdx_Main1(pSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(pSetting->miSensorIdx_Main2)
, mpFlowOption(pFlowOption)
, mpPipeOption(pPipeOption)
{
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

MBOOL
BayerBayerQTemplateProvider::
init(BaseBufferSizeMgr* pSizeMgr)
{
    mpSizeMgr = pSizeMgr;
    return prepareTemplateParams();
}


BayerBayerQTemplateProvider::
~BayerBayerQTemplateProvider()
{
    MY_LOGD("[Destructor] +");

    mSrzSizeTemplateMap.clear();
    mFETuningBufferMap.clear();
    mFMTuningBufferMap.clear();
    mVSDOFPQParamMap.clear();

    MY_LOGD("[Destructor] -");
}

NSCam::NSIoPipe::PortID
BayerBayerQTemplateProvider::
mapToPortID(DepthMapBufferID buffeID)
{
    switch(buffeID)
    {
        case BID_P2A_IN_RSRAW1:
        case BID_P2A_IN_RSRAW2:
        case BID_P2A_IN_FSRAW1:
        case BID_P2A_IN_FSRAW2:
            return PORT_IMGI;

        case BID_P2A_OUT_FDIMG:
        case BID_P2A_FE1C_INPUT:
        case BID_P2A_FE2C_INPUT:
            return PORT_IMG2O;

        case BID_P2A_OUT_FE1AO:
        case BID_P2A_OUT_FE2AO:
        case BID_P2A_OUT_FE1BO:
        case BID_P2A_OUT_FE2BO:
        case BID_P2A_OUT_FE1CO:
        case BID_P2A_OUT_FE2CO:
            return PORT_FEO;

        case BID_P2A_OUT_RECT_IN1:
        case BID_P2A_OUT_RECT_IN2:
        case BID_P2A_OUT_MV_F:
        case BID_P2A_OUT_MV_F_CAP:
            return PORT_WDMAO;

        case BID_P2A_FE1B_INPUT:
        case BID_P2A_FE2B_INPUT:
        case BID_P2A_OUT_MY_S:
        case BID_P2A_OUT_POSTVIEW:
            return PORT_WROTO;

        case BID_P2A_OUT_CC_IN1:
        case BID_P2A_OUT_CC_IN2:
            return PORT_WDMAO;

        case BID_P2A_OUT_FMBO_LR:
        case BID_P2A_OUT_FMBO_RL:
        case BID_P2A_OUT_FMCO_LR:
        case BID_P2A_OUT_FMCO_RL:
            return PORT_MFBO;

        #ifdef GTEST
        case BID_FE2_HWIN_MAIN1:
        case BID_FE2_HWIN_MAIN2:
        case BID_FE3_HWIN_MAIN1:
        case BID_FE3_HWIN_MAIN2:
            return PORT_IMG3O;
        #endif

        default:
            MY_LOGE("mapToPortID: not exist buffeID=%d", buffeID);
            break;
    }

    return NSCam::NSIoPipe::PortID();

}


MBOOL
BayerBayerQTemplateProvider::
prepareTemplateParams()
{
    MY_LOGD("+");
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

    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    // FE SRZ template - frame 2/4 need FEO srz crop
    _SRZ_SIZE_INFO_ srzInfo_frame2;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame2, rP2AFMSize.mFEBO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(2,  srzInfo_frame2);
    _SRZ_SIZE_INFO_ srzInfo_frame4;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame4, rP2AFMSize.mFECO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(4,  srzInfo_frame4);

    // VSDOF PQ Param
    for(int frameID=0;frameID<PQ_FRAME_SIZE;++frameID)
    {
        PQParam pqParam;
        DpPqParam dpPqParam;
        setupPQParamSetting(&mVSDOFTuningBufferArray[frameID], frameID);
        pqParam.WDMAPQParam = (void*)&mVSDOFTuningBufferArray[frameID];
        pqParam.WROTPQParam = (void*)&mVSDOFTuningBufferArray[frameID];
        mVSDOFPQParamMap.add(frameID, pqParam);
    }
    // FM tuning template
    // frame 6/8 - forward +  frame 7/9 - backward
    for(int frameID=6;frameID<10;++frameID)
    {
        NSCam::NSIoPipe::FMInfo fmInfo;
        // setup
        setupFMTuningInfo(fmInfo, frameID);
        mFMTuningBufferMap.add(frameID, fmInfo);
    }
    // prepare FE tuning buffer
    for (int iStage=1;iStage<=2;++iStage)
    {
        // setup
        MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(iStage);
        NSCam::NSIoPipe::FEInfo feInfo;
        StereoTuningProvider::getFETuningInfo(feInfo, iBlockSize);
        mFETuningBufferMap.add(iStage, feInfo);
    }

    // prepare QParams template
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    MINT32 iModuleTrans = remapTransform(eRot);

    MBOOL bRet = prepareQParamsTemplate(eSTATE_NORMAL, iModuleTrans, mQParam_NORMAL);
    bRet &= prepareQParamsTemplate(eSTATE_CAPTURE, iModuleTrans, mQParam_CAPTURE);
    if(!bRet)
    {
        MY_LOGE("prepareQParamsTemplate failed!!!");
    }

    MY_LOGD("-");
    return bRet;
}

MVOID
BayerBayerQTemplateProvider::
setupFMTuningInfo(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameID)
{
    MY_LOGD("+");

    MSize szFEBufSize = (iFrameID<=7) ? mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEB_INPUT_SIZE_MAIN1 :
                                        mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEC_INPUT_SIZE_MAIN1;
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

MVOID
BayerBayerQTemplateProvider::
setupPQParamSetting(DpPqParam* pParam, MUINT iFrameID)
{
    ENUM_PASS2_ROUND pass2Round;
    convertFrameIdToPass2Round(iFrameID, pass2Round);
    StereoTuningProvider::getPass2MDPPQParam(pass2Round, pParam->u.isp.vsdofParam);
}

MBOOL
BayerBayerQTemplateProvider::
convertFrameIdToPass2Round(MUINT32 frameID, ENUM_PASS2_ROUND &round)
{
    bool bResult = true;

    switch (frameID) {
        case 0:
            round = PASS2A_P;
            break;
        case 1:
            round = PASS2A;
            break;
        case 2:
            round = PASS2A_P_2;
            break;
        case 3:
            round = PASS2A_2;
            break;
        case 4:
            round = PASS2A_P_3;
            break;
        case 5:
            round = PASS2A_3;
            break;
        default:
            MY_LOGE("frameID is not defined for P2 round %u", frameID);
            bResult = false;
            break;
    }
    return bResult;
}


MBOOL
BayerBayerQTemplateProvider::
prepareQParamsTemplate(
    DepthMapPipeOpState state,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MY_LOGD("+");
    using namespace NSCam::NSIoPipe::NSPostProc;

    if(mpSizeMgr == nullptr)
        return MFALSE;

    MBOOL bSuccess = prepareQParam_frame0(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame1(state, iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame2(rQParam);
    bSuccess &= prepareQParam_frame3(state, rQParam);
    bSuccess &= prepareQParam_frame4(rQParam);
    bSuccess &= prepareQParam_frame5(state, rQParam);
    bSuccess &= prepareQParam_frame6_9(rQParam);

    MY_LOGD("-");
    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_WROT, MPoint(0,0), MSize(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN2).
            addOutput(mapToPortID(BID_P2A_FE2B_INPUT), iModuleTrans).
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame1(
    DepthMapPipeOpState state,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 1;
    ENUM_STEREO_SCENARIO scenario = (state == eSTATE_CAPTURE) ?
                                        eSTEREO_SCENARIO_CAPTURE : eSTEREO_SCENARIO_RECORD;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(scenario);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI);

    // crop information
    MPoint ptFD_CROP(0,0), ptMVF_CROP(0,0), ptFE1B_CROP(0,0);
    MSize szFD_CROP(0, 0);
    MSize szMVF_CROP(0, 0);
    MSize szFE1B_CROP(0, 0);

    if(state == eSTATE_CAPTURE)
    {
        #ifndef GTEST
        generator.addInput(PORT_DEPI);
        generator.addInput(PORT_DMGI);
        #endif
        // need FOV crop
        ptFD_CROP = rP2AFMSize.mFD_IMG_CROP.startPt;
        szFD_CROP = rP2AFMSize.mFD_IMG_CROP.contentSize();
        ptMVF_CROP = rP2AFMSize.mMAIN_IMAGE_CROP.startPt;
        szMVF_CROP = rP2AFMSize.mMAIN_IMAGE_CROP.contentSize();
        ptFE1B_CROP = rP2AFMSize.mFEB_INPUT_CROP_MAIN1.startPt;
        szFE1B_CROP = rP2AFMSize.mFEB_INPUT_CROP_MAIN1.contentSize();
    }

    MBOOL bSuccess =
         // IMG2O : FD
        generator.addCrop(eCROP_CRZ, ptFD_CROP, szFD_CROP, rP2AFMSize.mFD_IMG_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_FDIMG)).
            // WDMA : MV_F
            addCrop(eCROP_WDMA, ptMVF_CROP, szMVF_CROP, rP2AFMSize.mMAIN_IMAGE_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_MV_F), 0, EPortCapbility_Disp).
            // WROT: FE1B input
            addCrop(eCROP_WROT, ptFE1B_CROP, szFE1B_CROP, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(mapToPortID(BID_P2A_FE1B_INPUT), iModuleTrans).  // do module rotation
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame2(QParams& rQParam)
{
    MUINT iFrameNum = 2;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN2, rP2AFMSize.mFEC_INPUT_SIZE_MAIN2).  // IMG2O: FE2CO input
            addOutput(mapToPortID(BID_P2A_FE2C_INPUT)).
            addCrop(eCROP_WDMA, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN2, MSize(0,0)).  // WDMA : Rect_in2
            addOutput(mapToPortID(BID_P2A_OUT_RECT_IN2)).
            #ifdef GTEST
            addOutput(PORT_IMG3O).
            #endif
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1)).
            generate(rQParam);

    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame3(
    DepthMapPipeOpState state,
    QParams& rQParam
)
{
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


    if(state == eSTATE_NORMAL)
    {
        // add extra WROT output: EIS cropped MY_S for queued flow type
        templateGen.addCrop(eCROP_WROT, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).
                    addOutput(mapToPortID(BID_P2A_OUT_MY_S));
    }
    else
    {
        // post view need to rotate back
        ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
        int rotDeg = (int)eRot;
        if(rotDeg != 0)
        {
            rotDeg = 360 - rotDeg;
        }
        MINT32 iModuleTrans = remapTransform((ENUM_ROTATION)rotDeg);
        // add post view output port, MY_S is moved to frame 5.
        templateGen.addCrop(eCROP_WROT, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).
                    addOutput(mapToPortID(BID_P2A_OUT_POSTVIEW), iModuleTrans);
    }

    MBOOL bSuccess = templateGen.generate(rQParam);
    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame4(QParams& rQParam)
{
    MUINT iFrameNum = 4;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum);
    MBOOL bSuccess =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2AFMSize.mFEC_INPUT_SIZE_MAIN2, rP2AFMSize.mCCIN_SIZE_MAIN2).  // WDMA: CC_in2
        addOutput(mapToPortID(BID_P2A_OUT_CC_IN2)).
        #ifdef GTEST
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).                           // FEO
        addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).
        generate(rQParam);

    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame5(
    DepthMapPipeOpState state,
	QParams& rQParam
)
{
    //--> frame 5
    MUINT iFrameNum = 5;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    QParamTemplateGenerator templateGen =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 5
        addInput(PORT_IMGI).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2AFMSize.mFEC_INPUT_SIZE_MAIN1, rP2AFMSize.mCCIN_SIZE_MAIN1).  // WDMA: CC_in1
        addOutput(mapToPortID(BID_P2A_OUT_CC_IN1)).
        #ifdef GTEST
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).   // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2));

	if(state == eSTATE_CAPTURE)
    {
        // add extra WROT output: EIS cropped MY_S for queued flow type
        templateGen.addCrop(eCROP_WROT, MPoint(0,0), rP2AFMSize.mFEC_INPUT_SIZE_MAIN1, MSize(0,0)).
                    addOutput(mapToPortID(BID_P2A_OUT_MY_S));
    }

     MBOOL bSuccess = templateGen.generate(rQParam);
    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
prepareQParam_frame6_9(QParams& rQParam)
{
    MBOOL bSuccess = MTRUE;
    //--> frame 6 ~ 9
    for(int frameID=6;frameID<10;++frameID)
    {
        bSuccess &=
        QParamTemplateGenerator(frameID, miSensorIdx_Main1, ENormalStreamTag_FM).
            addInput(PORT_DEPI).
            addInput(PORT_DMGI).
            addOutput(PORT_MFBO).
            addExtraParam(EPIPE_FM_INFO_CMD, (MVOID*)&mFMTuningBufferMap.valueFor(frameID)).
            generate(rQParam);
    }
    return bSuccess;
}

MBOOL
BayerBayerQTemplateProvider::
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
BayerBayerQTemplateProvider::
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
BayerBayerQTemplateProvider::
buildQParam_frame0(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main2, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;

    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    IImageBuffer* frameBuf_RSRAW2 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW2);

    AAATuningResult tuningRes = tuningResult.tuningRes_main2;
    // output: FE2B input(WROT)
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2B_INPUT);
    // filler buffers and the raw size crop info
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer).  // insert tuning data
        insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW2). // input: Main2 RSRAW
        setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW2->getImgSize(),
                    mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEB_INPUT_SIZE_MAIN2).
        insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2B_INPUT), pImgBuf).// output: FE2B input(WROT)
        addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
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

    // check exist Request FD buffer or not
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = rEffReqPtr->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // fill buffer
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW

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
    // output: MV_F
    IImageBuffer* frameBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);
    // check EIS on/off
    if (rEffReqPtr->getRequestAttr().isEISOn)
    {
        eis_region region;
        IMetadata* pMeta_InHal = pBufferHandler->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
        // set MV_F crop for EIS
        if(queryEisRegion(pMeta_InHal, region))
        {
            rQFiller.setCrop(iFrameNum, eCROP_WDMA, MPoint(region.x_int, region.y_int), region.s, frameBuf_MV_F->getImgSize());
        }
        else
        {
            MY_LOGE("Query EIS Region Failed! reqID=%d.", rEffReqPtr->getRequestNo());
            return MFALSE;
        }
    }
    else
    {   // MV_F
        rQFiller.setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), frameBuf_MV_F->getImgSize());
    }

    // only depth input need enable PQ, remove the WMDA port which is MV_F
    mVSDOFPQParamMap.editValueFor(iFrameNum).WDMAPQParam = nullptr;
    // ouput: FE1B_input
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    rQFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pImgBuf). //FE1B_input
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf->getImgSize()).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame1_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // UT does not test 3A
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
        VSDOF_LOGD("pBpc2Buf exist!");
        pBpcBuf = static_cast<IImageBuffer*>(tuningRes.tuningResult.pBpc2Buf);;
        rQFiller.insertInputBuf(iFrameNum, PORT_DMGI, pBpcBuf);
    }
    else
        rQFiller.delInputPort(iFrameNum, PORT_DMGI);
    #endif

    // input : FSRAW
    IImageBuffer* pBuf_FSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_FSRAW1);
    // output : FD
    IImageBuffer* pFDBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FDIMG);
    // output : MV_F_CAP
    IImageBuffer* pBuf_MV_F_CAP = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F_CAP);
    // output: FE1B_input
    IImageBuffer* pFE1BInBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    // FOV crop: crop settings already configured at preparation stage
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pBuf_FSRAW1). //FSRAW
            setCropResize(iFrameNum, eCROP_CRZ, pFDBuf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FDIMG), pFDBuf). //FD
            setCropResize(iFrameNum, eCROP_WDMA, pBuf_MV_F_CAP->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_MV_F_CAP), pBuf_MV_F_CAP). //MV_F_CAP
            setCropResize(iFrameNum, eCROP_WROT, pFE1BInBuf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pFE1BInBuf). //FE1B_input
            insertTuningBuf(iFrameNum, tuningRes.tuningBuffer).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame2(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    //--> frame 2
    MUINT iFrameNum = 2;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main2, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: fe2bBuf_in
    IImageBuffer* fe2bBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in =  pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
	if(CC_UNLIKELY(!bRet))
        return MFALSE;

    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(), pRectIn2Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
            insertTuningBuf(iFrameNum, pTuningBuf). // tuning data
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));
    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame2_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    //--> frame 2
    MUINT iFrameNum = 2;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main2, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: fe2bBuf_in
    IImageBuffer* fe2bBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in =  pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    // Rect_in2 : set active area
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE).mRECT_IN_CONTENT_SIZE_MAIN2);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(), pRectIn2Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
            insertTuningBuf(iFrameNum, pTuningBuf). // tuning data
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));
    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame3(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
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


    // output: MY_S with EIS cropped
    if(rEffReqPtr->getRequestAttr().isEISOn)
    {
        eis_region region;
        // EIS input buffer: main1 RRZO
        IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
        MBOOL bRes = queryRemappedEISRegion(rEffReqPtr, frameBuf_RSRAW1->getImgSize(),
                                            pFE1BImgBuf->getImgSize(), region);
        if(!bRes)
            return MFALSE;
        // MY_S buffer(240x136)
        IImageBuffer* pMYSBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
        rQFiller.setCrop(iFrameNum, eCROP_WROT, MPoint(region.x_int, region.y_int), region.s, pMYSBuf->getImgSize()).
                insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_MY_S), pMYSBuf);
    }
    else
    {
        IImageBuffer* pMYSBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
        rQFiller.setCropResize(iFrameNum, eCROP_WROT, pMYSBuf->getImgSize()).
                insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_MY_S), pMYSBuf);
    }

    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame3_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
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
    // output: postview
    IImageBuffer* pPostViewBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_POSTVIEW);
    // postview rotation
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(nodeID, BID_META_IN_APP);
    MINT32 jpegOrientation;
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
        return MFALSE;
    }
    ENUM_ROTATION moduleRot = StereoSettingProvider::getModuleRotation();
    MINT32 iModuleTrans = remapTransformForPostview((ENUM_ROTATION)moduleRot, (ENUM_ROTATION)jpegOrientation);
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
            setCropResize(iFrameNum, eCROP_WROT, pPostViewBuf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_POSTVIEW), pPostViewBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame4(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 4;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main2, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: fe2coBuf_in
    IImageBuffer* pFE2CBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2C_INPUT, pFE2CBuf_in);
    // output: CC_in2
    IImageBuffer* pCCIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_CC_IN2);
    // output: FE2CO
    IImageBuffer* pFE2COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2CO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE2CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_CC_IN2), pCCIn2Buf). // CC_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2CO), pFE2COBuf). // FE2COBuf
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));

    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame5(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 5;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();

    // input: fe1coBuf_in
    IImageBuffer* pFE1CBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1C_INPUT, pFE1CBuf_in);
    // output: CC_in1
    IImageBuffer* pCCIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_CC_IN1);
    // output: FE1CO
    IImageBuffer* pFE1COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1CO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_CC_IN1), pCCIn1Buf). // CC_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1CO), pFE1COBuf). // FE1CO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));


    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame5_capture(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 5;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    if(!configureDpPqParam(rEffReqPtr, miSensorIdx_Main1, mVSDOFTuningBufferArray[iFrameNum]))
        return MFALSE;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();

    // input: fe1coBuf_in
    IImageBuffer* pFE1CBuf_in;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1C_INPUT, pFE1CBuf_in);
    // output: CC_in1
    IImageBuffer* pCCIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_CC_IN1);
    // output: FE1CO
    IImageBuffer* pFE1COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1CO);
    // output: MY_S
    IImageBuffer* pMYSBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
	// tuning
	MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_CC_IN1), pCCIn1Buf). // CC_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1CO), pFE1COBuf). // FE1CO
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_MY_S), pMYSBuf). // MY_S
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)&mVSDOFPQParamMap.editValueFor(iFrameNum));


    #ifdef GTEST
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
buildQParam_frame6to9(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();

    // prepare FEO buffers, frame 2~5 has FE output
    IImageBuffer* feoBuf[6] = {NULL};
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE2BO, feoBuf[2]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE1BO, feoBuf[3]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE2CO, feoBuf[4]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE1CO, feoBuf[5]);

    //--> frame 6: FM - L to R
    int iFrameNum = 6;
    // output: FMBO_LR
    IImageBuffer* pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMBO_LR);
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_LSide[0]]).
            insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_RSide[0]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_LR), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);


    //--> frame 7: FM - R to L
    iFrameNum = 7;
    // output: FMBO_RL
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMBO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_RSide[0]]).
            insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_LSide[0]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_RL), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    //--> frame 8: FM - L to R
    iFrameNum = 8;
    // output: FMCO_LR
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_LR);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_LSide[1]]).
            insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_RSide[1]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_LR), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    ///--> frame 9: FM - R to L
    iFrameNum = 9;
    // output: FMCO_LR
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_DEPI, feoBuf[frameIdx_RSide[1]]).
            insertInputBuf(iFrameNum, PORT_DMGI, feoBuf[frameIdx_LSide[1]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_RL), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    return MTRUE;
}

MBOOL
BayerBayerQTemplateProvider::
queryRemappedEISRegion(
    sp<DepthMapEffectRequest> pRequest,
    MSize szEISDomain,
    MSize szRemappedDomain,
    eis_region& rOutRegion
)
{
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    IMetadata* pMeta_InHal = pRequest->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    if(!queryEisRegion(pMeta_InHal, rOutRegion))
    {
        MY_LOGE("Failed to query EIS region!");
        return MFALSE;
    }


    VSDOF_LOGD("Original query-EIS region: startPt=(%d.%d, %d.%d) Size=(%d, %d)",
            rOutRegion.x_int, rOutRegion.x_float, rOutRegion.y_int, rOutRegion.y_float,
            rOutRegion.s.w, rOutRegion.s.h);

    float ratio_width = szRemappedDomain.w * 1.0 / szEISDomain.w;
    float ratio_height = szRemappedDomain.h * 1.0 / szEISDomain.h;

    rOutRegion.x_int = (MUINT32) ceil(rOutRegion.x_int * ratio_width);
    rOutRegion.x_float = (MUINT32) ceil(rOutRegion.x_int * ratio_width);
    rOutRegion.y_int = (MUINT32) ceil(rOutRegion.y_int * ratio_height);
    rOutRegion.y_float = (MUINT32) ceil(rOutRegion.y_float * ratio_height);
    rOutRegion.s.w = ceil(rOutRegion.s.w * ratio_width);
    rOutRegion.s.h = ceil(rOutRegion.s.h * ratio_height);

    VSDOF_LOGD("Remapped query-EIS region: startPt=(%d.%d, %d.%d) Size=(%d, %d)",
            rOutRegion.x_int, rOutRegion.x_float, rOutRegion.y_int, rOutRegion.y_float,
            rOutRegion.s.w, rOutRegion.s.h);

    return MTRUE;
}

MINT32
BayerBayerQTemplateProvider::
remapTransformForPostview(ENUM_ROTATION moduleRotation, ENUM_ROTATION jpegOrientation)
{
    //Try to rotate back to original and then rotate to jpegOrientation
    MINT32 postViewRot = (360 - moduleRotation + jpegOrientation)%360;
    MINT32 finalPostViewRot = remapTransform((ENUM_ROTATION)postViewRot);
    VSDOF_LOGD("Postview rotate: moduleRotation:%d,jpegOrientation:%d, postViewRot:%d iModuleTrans:%d",moduleRotation, jpegOrientation, postViewRot, finalPostViewRot);
    return finalPostViewRot;
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
