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

/**
 * @file DepthQTemplateProvider.cpp
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
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/feature/lcenr/lcenr.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "DepthQTemplateProvider.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DepthQTemplateProvider"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace StereoHAL;
using namespace NSCam::NSIoPipe::NSPostProc;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthQTemplateProvider::
DepthQTemplateProvider(
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
DepthQTemplateProvider::
init(BaseBufferSizeMgr* pSizeMgr)
{
    mpSizeMgr = pSizeMgr;
    MBOOL bRet = prepareTemplateParams();
    bRet &= prepareTemplateParams_Bayer();
    return bRet;
}


DepthQTemplateProvider::
~DepthQTemplateProvider()
{
    MY_LOGD("[Destructor] +");

    mSrzSizeTemplateMap.clear();
    mFETuningBufferMap.clear();
    mFMTuningBufferMap_LtoR.clear();
    mFMTuningBufferMap_RtoL.clear();
    mFMTuningBufferMap_cap.clear();
    MY_LOGD("[Destructor] -");
}

NSCam::NSIoPipe::PortID
DepthQTemplateProvider::
mapToPortID(DepthMapBufferID buffeID)
{
    switch(buffeID)
    {
        case BID_P2A_IN_RSRAW1:
        case BID_P2A_IN_RSRAW2:
        case BID_P2A_IN_FSRAW1:
        case BID_P2A_IN_FSRAW2:
        case BID_P2A_IN_YUV1:
        case BID_P2A_IN_YUV2:
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
        case BID_N3D_OUT_MV_Y:
            return PORT_WDMAO;

        case BID_P2A_FE1B_INPUT:
        case BID_P2A_FE2B_INPUT:
        case BID_P2A_OUT_MY_S:
        case BID_P2A_OUT_POSTVIEW:
            return PORT_WROTO;

        case BID_P2A_OUT_FMBO_LR:
        case BID_P2A_OUT_FMBO_RL:
        case BID_P2A_OUT_FMCO_LR:
        case BID_P2A_OUT_FMCO_RL:
            return PORT_FMO;

        #if defined(GTEST) && !defined(GTEST_PROFILE)
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
DepthQTemplateProvider::
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

    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    // FE SRZ template - key is stage num, only stage 1/2 needs FE
    _SRZ_SIZE_INFO_ srzInfo_frame2;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame2, rP2ASize.mFEBO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(1,  srzInfo_frame2);
    _SRZ_SIZE_INFO_ srzInfo_frame4;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame4, rP2ASize.mFECO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(2,  srzInfo_frame4);
    // FM tuning template
    for(int iStage=1;iStage<=2;++iStage)
    {
        NSCam::NSIoPipe::FMInfo fmInfo_LR;
        // setup
        setupFMTuningInfo(fmInfo_LR, iStage, MTRUE);
        mFMTuningBufferMap_LtoR.add(iStage, fmInfo_LR);

        NSCam::NSIoPipe::FMInfo fmInfo_RL;
        // setup
        setupFMTuningInfo(fmInfo_RL, iStage, MFALSE);
        mFMTuningBufferMap_RtoL.add(iStage, fmInfo_RL);
    }
    // FM tuning template for capture, frame 6/8 - forward +  frame 7/9 - backward
    for(int frameID=6;frameID<10;++frameID)
    {
        NSCam::NSIoPipe::FMInfo fmInfo;
        // setup
        setupFMTuningInfo_cap(fmInfo, frameID);
        mFMTuningBufferMap_cap.add(frameID, fmInfo);
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

    MBOOL bRet = MTRUE;
    bRet &= prepareQParamsTemplate(MFALSE, eSTATE_NORMAL, iModuleTrans, mQParam_NORMAL);
    bRet &= prepareQParamsTemplate(MTRUE,  eSTATE_NORMAL, iModuleTrans, mQParam_NORMAL_NOFEFM);
    bRet &= prepareQParamsTemplate(MFALSE, eSTATE_CAPTURE, iModuleTrans, mQParam_CAPTURE);
    if(!bRet)
    {
        MY_LOGE("prepareQParamsTemplate failed!!!");
    }

    MY_LOGD("-");
    return bRet;
}

MBOOL
DepthQTemplateProvider::
prepareTemplateParams_Bayer()
{
    VSDOF_LOGD2("+");
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
    MBOOL bRet = prepareQParamsTemplate_BAYER_NORMAL(iModuleTrans, mQParam_BAYER_NORMAL);
    bRet &= prepareQParamsTemplate_BAYER_NORMAL_NOFEFM(iModuleTrans, mQParam_BAYER_NORMAL_NOFEFM);
    bRet &= prepareQParamsTemplate_BAYER_STANDALONE(iModuleTrans, mQParam_BAYER_STANDALONE);
    if(!bRet)
    {
        MY_LOGE("prepareQParamsTemplate failed!!!");
        return MFALSE;
    }
    VSDOF_LOGD2("-");
    return MTRUE;
}

MVOID
DepthQTemplateProvider::
setupFMTuningInfo(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iStageNum, MBOOL isLtoR)
{
    MY_LOGD("+");

    MSize szFEBufSize = (iStageNum == 1) ? mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEB_INPUT_SIZE_MAIN1 :
                                        mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEC_INPUT_SIZE_MAIN1;

    ENUM_FM_DIRECTION eDir = (isLtoR) ? E_FM_L_TO_R : E_FM_R_TO_L;
    // query tuning parameter
    StereoTuningProvider::getFMTuningInfo(eDir, fmInfo);
    MUINT32 iBlockSize =  StereoSettingProvider::fefmBlockSize(iStageNum);
    // set width/height
    fmInfo.mFMWIDTH = szFEBufSize.w/iBlockSize;
    fmInfo.mFMHEIGHT = szFEBufSize.h/iBlockSize;

    MY_LOGD("-");
}

MBOOL
DepthQTemplateProvider::
prepareQParamsTemplate_NORMAL(
    MBOOL bIsSkipFEFM,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MY_LOGD("+ bIsSkipFEFM=%d iModuleTrans=%d", bIsSkipFEFM, iModuleTrans);
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MSize szTBD(0, 0);
    MBOOL bSuccess = MTRUE;

    if(bIsSkipFEFM)
    {
        //P2A preview when skip FEFM-> only output main2 rect_in
        bSuccess &=
            QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
                addInput(PORT_IMGI).
                addCrop(eCROP_WROT, MPoint(0,0), szTBD, rP2ASize.mRECT_IN_SIZE_MAIN2, true).
                // Rect_in2 : need module transform
                addOutput(PORT_WROTO, iModuleTrans).
                generate(rQParam);
    }
    else
    {
        bSuccess = prepareQParamStage0_Main2(iFrameNum++, iModuleTrans, rQParam);
        bSuccess &= prepareQParamStage1_Main2(iFrameNum++, rQParam);
        bSuccess &= prepareQParamStage2_Main2(iFrameNum++, rQParam);
    }

    MY_LOGD("- success=%d", bSuccess);
    return bSuccess;
}

MVOID
DepthQTemplateProvider::
_prepareLCETemplate(QParamTemplateGenerator& generator)
{
    generator.addInput(PORT_LCEI).
            addInput(PORT_YNR_LCEI);
}

MBOOL
DepthQTemplateProvider::
_fillLCETemplate(
    MUINT iFrameNum,
    sp<DepthMapEffectRequest> pRequest,
    const AAATuningResult& tuningRes,
    QParamTemplateFiller& rQFiller,
    _SRZ_SIZE_INFO_& rSrzInfo
)
{
    if(tuningRes.tuningResult.pLcsBuf == nullptr)
    {
        rQFiller.delInputPort(iFrameNum, PORT_LCEI).
                delInputPort(iFrameNum, PORT_YNR_LCEI);
    }
    else
    {
        if(!setupSRZ4LCENR(pRequest, rSrzInfo))
            return MFALSE;
        auto pBufferHandler = pRequest->getBufferHandler();
        IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(eDPETHMAP_PIPE_NODEID_P2A, PBID_IN_LCSO1);
        rQFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCSOBuf).
                insertInputBuf(iFrameNum, PORT_YNR_LCEI, pLCSOBuf).
                addModuleInfo(iFrameNum, EDipModule_SRZ4, &rSrzInfo);
    }
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
setupSRZ4LCENR(
    sp<DepthMapEffectRequest> pRequest,
    _SRZ_SIZE_INFO_& rSizeInfo
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_IN_LCSO1);
    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(eDPETHMAP_PIPE_NODEID_P2A, BID_META_IN_HAL_MAIN1);
    LCENR_IN_PARAMS in;
    LCENR_OUT_PARAMS out;

    if(!tryGetMetadata<MRect>(pInHalMeta, MTK_P1NODE_BIN_CROP_REGION, in.rrz_crop_in))
    {
        MY_LOGE("Failed to query MTK_P1NODE_BIN_CROP_REGION metadata!");
        return MFALSE;
    }
    if(!tryGetMetadata<MSize>(pInHalMeta, MTK_P1NODE_BIN_SIZE, in.rrz_in))
    {
        MY_LOGE("Failed to query MTK_P1NODE_BIN_SIZE metadata!");
        return MFALSE;
    }
    if(!tryGetMetadata<MSize>(pInHalMeta, MTK_P1NODE_RESIZER_SIZE, in.rrz_out))
    {
        MY_LOGE("Failed to query MTK_HAL_REQUEST_SENSOR_SIZE metadata!");
        return MFALSE;
    }

    // preview use rrzo
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        IImageBuffer* pRawBuf = pBufferHandler->requestBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_IN_FSRAW1);
        in.resized = MFALSE;
        in.p2_in = pRawBuf->getImgSize();
    }
    else
    {
        IImageBuffer* pRawBuf = pBufferHandler->requestBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_IN_RSRAW1);
        in.resized = MTRUE;
        in.p2_in = pRawBuf->getImgSize();
    }
    in.lce_full = pLCSOBuf->getImgSize();
    calculateLCENRConfig(in, out);
    // write the srz info
    rSizeInfo = out.srz4Param;
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage0_Main2(
    int iFrameNum,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            // enable MDPCRop2
            addCrop(eCROP_WROT, MPoint(0,0), MSize(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2, true).
            addOutput(mapToPortID(BID_P2A_FE2B_INPUT), iModuleTrans).
            generate(rQParam);

    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage0_Main1(
    int iFrameNum,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_PREVIEW);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI);
    // prepare LCE
    this->_prepareLCETemplate(generator);
    // crop information
    MPoint ptOrigin(0,0);
    MSize szTBD(0,0);
    {
        generator.addInput(PORT_VIPI).
            // IMG2O : FD
            addCrop(eCROP_CRZ, ptOrigin, szTBD, rP2ASize.mFD_IMG_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_FDIMG)).
            // WDMA : MV_F
            addCrop(eCROP_WDMA, ptOrigin, szTBD, rP2ASize.mMAIN_IMAGE_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_MV_F), 0, EPortCapbility_Disp).
            // WROT: FE1B input - depth input (apply MDPCrop)
            addCrop(eCROP_WROT, ptOrigin, szTBD, rP2ASize.mFEB_INPUT_SIZE_MAIN1, true).
            addOutput(mapToPortID(BID_P2A_FE1B_INPUT), iModuleTrans).  // do module rotation
            addOutput(PORT_IMG3O); //for 3dnr
    }
    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage1_Main2(
    int iFrameNum,
    QParams& rQParam
)
{
    int iStage = 1;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iStage);

    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2, rP2ASize.mFEC_INPUT_SIZE_MAIN2).  // IMG2O: FE2CO input
            addOutput(mapToPortID(BID_P2A_FE2C_INPUT)).
            #if defined(GTEST) && !defined(GTEST_PROFILE)
            addOutput(PORT_IMG3O).
            #endif
            addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2, MSize(0,0)).  // WDMA : Rect_in2
            addOutput(mapToPortID(BID_P2A_OUT_RECT_IN2)).
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(iStage));

    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage1_Main1(
    int iFrameNum,
    QParams& rQParam
)
{
    int iStage = 1;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    QParamTemplateGenerator templateGen =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        //FE1CO
        addCrop(eCROP_CRZ, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1, rP2ASize.mFEC_INPUT_SIZE_MAIN1).
        addOutput(mapToPortID(BID_P2A_FE1C_INPUT)).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).                           // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(iStage));

    MBOOL bSuccess = templateGen.generate(rQParam);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage2_Main2(
    int iFrameNum,
    QParams& rQParam
)
{
    int iStage = 2;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iStage);

    MBOOL bSuccess =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).                           // FEO
        addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(iStage)).
        generate(rQParam);

    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParamStage2_Main1(
    int iFrameNum,
    QParams& rQParam
)
{
    int iStage = 2;
    QParamTemplateGenerator templateGen =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 5
        addInput(PORT_IMGI).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).   // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(iStage));

    return templateGen.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParam_FM(
    int iFrameNum,
    int iStage,
    MBOOL bIsLtoR,
    QParams& rQParam
)
{
    const NSCam::NSIoPipe::FMInfo* pFMInfo = (bIsLtoR) ?
                                        &mFMTuningBufferMap_LtoR.valueFor(iStage) :
                                        &mFMTuningBufferMap_RtoL.valueFor(iStage);
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_FM).
            addInput(PORT_RFEOI).
            addInput(PORT_LFEOI).
            addOutput(PORT_FMO).
            addExtraParam(EPIPE_FM_INFO_CMD, (MVOID*)pFMInfo).
            generate(rQParam);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
buildQParams_NORMAL(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    CAM_ULOGM_TAGLIFE("P2ANode::buildQParams_NORMAL");
    VSDOF_LOGD2("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_NORMAL;
    int iFrameNum = 0;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());
    //
    MBOOL bRet = buildQParamStage1_Main2(iFrameNum++, pRequest, tuningResult, qParamFiller);
    bRet &= buildQParamStage2_Main2(iFrameNum++, pRequest, tuningResult, qParamFiller);
    bRet &= buildQParamStage3_Main2(iFrameNum++, pRequest, tuningResult, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;

}

MBOOL
DepthQTemplateProvider::
buildQParams_BAYER_NORMAL(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    CAM_ULOGM_TAGLIFE("P2ANode::buildQParams_BAYER_NORMAL");
    VSDOF_LOGD2("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_BAYER_NORMAL;
    int iFrameNum = 0;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());
    //
    MBOOL bRet = buildQParamStage1_Main1(iFrameNum++, pRequest, tuningResult, qParamFiller);
    bRet &= buildQParamStage2_Main1(iFrameNum++, pRequest, tuningResult, qParamFiller);
    bRet &= buildQParamStage3_Main1(iFrameNum++, pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_4FMFrames(iFrameNum, pRequest, tuningResult, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;
}

MBOOL
DepthQTemplateProvider::
buildQParams_NORMAL_NOFEFM(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    //P2A preview when skip FEFM-> only output main2 rect_in
    CAM_ULOGM_TAGLIFE("P2ANode::buildQParams_NORMAL");
    VSDOF_LOGD2("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_NORMAL_NOFEFM;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_RSRAW2 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW2);
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(eSTEREO_SCENARIO_PREVIEW).mRECT_IN_CONTENT_SIZE_MAIN2);
    // PQParam
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WROTO))
        return MFALSE;
    //
    AAATuningResult tuningRes = tuningResult.tuningRes_main2;
    // filler buffers and the raw size crop info
    qParamFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer).  // insert tuning data
        insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW2). // input: Main2 RSRAW
        // output: Rect_in2 (WROT-MDPCrop2)
        setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW2->getImgSize(), pRectIn2Buf->getImgSize(), true).
        insertOutputBuf(iFrameNum, PORT_WROTO, pRectIn2Buf).
        addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    VSDOF_LOGD2("-");
    return MTRUE;

}

MBOOL
DepthQTemplateProvider::
buildQParamStage1_Main2(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage1_Main2");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_RSRAW2 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW2);
    AAATuningResult tuningRes = tuningResult.tuningRes_main2;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WROTO))
        return MFALSE;
    // output: FE2B input(WROT)
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2B_INPUT);
    // filler buffers and the raw size crop info
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer).  // insert tuning data
        insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW2). // input: Main2 RSRAW
        // output: FE2B input(WROT, MDPCrop2)
        setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW2->getImgSize(),
                    mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEB_INPUT_SIZE_MAIN2, true).
        insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2B_INPUT), pImgBuf).
        addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParamStage1_Main1(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage1_Main1");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // output: MV_F
    IImageBuffer* frameBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);
    // ouput: FE1B_input
    IImageBuffer* pFE1B_Input_Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    // ouput: IMG3O buffer
    IImageBuffer* pImgBuf_IMG3O = pBufferHandler->requestBuffer(nodeID, BID_P2A_INTERNAL_IMG3O);
    // input: VIPI
    if(mp3DNR_IMG3OBuf.get() == nullptr)
        rQFiller.delInputPort(iFrameNum, PORT_VIPI);
    else
        rQFiller.insertInputBuf(iFrameNum, PORT_VIPI, mp3DNR_IMG3OBuf->mImageBuffer.get());
    // input: IMGI
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW
    // LCE
    if(!this->_fillLCETemplate(iFrameNum, pRequest, tuningRes, rQFiller, mSRZ4Config_LCENR))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO) ||
        !configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO, true))
        return MFALSE;
    // get crop region
    MPoint ptEISStartPt;
    MSize szEISCropSize;
    if(!getEISCropRegion(nodeID, pRequest, ptEISStartPt, szEISCropSize))
        return MFALSE;
    // FD output
    this->_buildQParam_FD_n_FillInternal(nodeID, iFrameNum, pRequest,
                                        ptEISStartPt, szEISCropSize, rQFiller);
    //
    rQFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pFE1B_Input_Buf). //FE1B_input, apply MDPCrop2
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pFE1B_Input_Buf->getImgSize(), true).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParamStage2_Main2(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage2_Main2");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: fe2bBuf_in
    IImageBuffer* fe2bBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in =  pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(eSTEREO_SCENARIO_PREVIEW).mRECT_IN_CONTENT_SIZE_MAIN2);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);
	if(CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(), pRectIn2Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
            insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main2.tuningBuffer). // tuning data
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParamStage2_Main1(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage2_Main1");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: FE1BBuf_in
    IImageBuffer* pFE1BImgBuf = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
            insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main1.tuningBuffer).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParamStage3_Main2(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage3_Main2");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: fe2coBuf_in
    IImageBuffer* pFE2CBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2C_INPUT, pFE2CBuf_in);
    // output: FE2CO
    IImageBuffer* pFE2COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2CO);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE2CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2CO), pFE2COBuf). // FE2COBuf
            insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main2.tuningBuffer).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParamStage3_Main1(
    int iFrameNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParamStage3_Main1");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // input: fe1coBuf_in
    IImageBuffer* pFE1CBuf_in = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1C_INPUT, pFE1CBuf_in);
    // output: FE1CO
    IImageBuffer* pFE1COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1CO);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1CO), pFE1COBuf). // FE1CO
            insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main1.tuningBuffer).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);


    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_4FMFrames(
    int iFNum,
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    VSDOF_LOGD2("buildQParam_4FMFrames");
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // prepare FEO buffers, frame 2~5 has FE output
    IImageBuffer* feoBuf[6] = {NULL};
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE2BO, feoBuf[2]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE1BO, feoBuf[3]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE2CO, feoBuf[4]);
    pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_FE1CO, feoBuf[5]);

    //--> frame iFNum: FM - L to R
    int iFrameNum = iFNum;
    // output: FMBO_LR
    IImageBuffer* pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMBO_LR);
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_LSide[0]]).
            insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_RSide[0]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_LR), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    //--> frame iFNum+1: FM - R to L
    iFrameNum++;
    // output: FMBO_RL
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMBO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_RSide[0]]).
            insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_LSide[0]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_RL), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    //--> frame iFNum+2: FM - L to R
    iFrameNum++;
    // output: FMCO_LR
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_LR);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_LSide[1]]).
            insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_RSide[1]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_LR), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    ///--> frame iFNum+3: FM - R to L
    iFrameNum++;
    // output: FMCO_LR
    pFMBOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_RSide[1]]).
            insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_LSide[1]]).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_RL), pFMBOBuf).
            insertTuningBuf(iFrameNum, pTuningBuf);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
prepareQParamsTemplate_BAYER_NORMAL(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    VSDOF_LOGD2("+");
    MUINT32 iFrameNum = 0;
    MBOOL bSuccess = prepareQParamStage0_Main1(iFrameNum++, iModuleTrans, rQParam);
    bSuccess &= prepareQParamStage1_Main1(iFrameNum++, rQParam);
    bSuccess &= prepareQParamStage2_Main1(iFrameNum++, rQParam);
    bSuccess &= prepareQParam_FM(iFrameNum++, 1, MTRUE, rQParam);
    bSuccess &= prepareQParam_FM(iFrameNum++, 1, MFALSE, rQParam);
    bSuccess &= prepareQParam_FM(iFrameNum++, 2, MTRUE, rQParam);
    bSuccess &= prepareQParam_FM(iFrameNum++, 2, MFALSE, rQParam);
    VSDOF_LOGD2("- : res=%d", bSuccess);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParamsTemplate_BAYER_NORMAL_NOFEFM(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    VSDOF_LOGD2("+");
    MBOOL bSuccess = prepareQParam_Bayer_NORMAL_NOFEFM_frame0(iModuleTrans, rQParam);
    VSDOF_LOGD2("- : res=%d", bSuccess);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParam_Bayer_NORMAL_NOFEFM_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    // crop information
    MPoint ptOrigin(0,0);
    MSize szTBD(0,0);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addInput(PORT_VIPI).
        // IMG2O : FD
        addCrop(eCROP_CRZ, ptOrigin, szTBD, rP2ASize.mFD_IMG_SIZE).
        addOutput(PORT_IMG2O).
        // WDMA : MV_F
        addCrop(eCROP_WDMA, ptOrigin, szTBD, rP2ASize.mMAIN_IMAGE_SIZE).
        addOutput(PORT_WDMAO, 0, EPortCapbility_Disp).
        // WROT : MY_S
        addCrop(eCROP_WROT, ptOrigin, szTBD, rP2ASize.mMYS_SIZE).
        addOutput(PORT_WROTO, iModuleTrans).
        addOutput(PORT_IMG3O); //for 3dnr
    this->_prepareLCETemplate(generator);
    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParamsTemplate_BAYER_STANDALONE(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    // crop information
    MPoint ptOrigin(0,0);
    MSize szTBD(0,0);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addInput(PORT_VIPI).
        // IMG2O : FD
        addCrop(eCROP_CRZ, ptOrigin, szTBD, rP2ASize.mFD_IMG_SIZE).
        addOutput(PORT_IMG2O).
        // WDMA : MV_F
        addCrop(eCROP_WDMA, ptOrigin, szTBD, rP2ASize.mMAIN_IMAGE_SIZE).
        addOutput(PORT_WDMAO, 0, EPortCapbility_Disp).
        addOutput(PORT_IMG3O);
    // LCE
    this->_prepareLCETemplate(generator);
    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
buildQParams_BAYER_NORMAL_NOFEFM(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    // generate Main1
    VSDOF_LOGD2("+");
    // copy template
    rOutParm = mQParam_BAYER_NORMAL_NOFEFM;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());
    //
    MBOOL bRet = buildQParam_Bayer_NORMAL_NOFEFM_frame0(pRequest, tuningResult, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    VSDOF_LOGD2("- res=%d", bRet);
    return bRet;
}

MBOOL
DepthQTemplateProvider::
buildQParam_Bayer_NORMAL_NOFEFM_frame0(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result : main1
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // output: MV_F
    IImageBuffer* frameBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);
    // output: MY_S
    IImageBuffer* pImgBuf_MYS = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
    // ouput: IMG3O buffer
    IImageBuffer* pImgBuf_IMG3O = pBufferHandler->requestBuffer(nodeID, BID_P2A_INTERNAL_IMG3O);
    // fill inputs - VIPI
    if(mp3DNR_IMG3OBuf.get() == nullptr)
        rQFiller.delInputPort(iFrameNum, PORT_VIPI);
    else
        rQFiller.insertInputBuf(iFrameNum, PORT_VIPI, mp3DNR_IMG3OBuf->mImageBuffer.get());
    // fill input - IMGI
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW
    // LCE
    if(!this->_fillLCETemplate(iFrameNum, pRequest, tuningRes, rQFiller, mSRZ4Config_LCENR_Bayer))
        return MFALSE;
    // get crop region
    MPoint ptEISStartPt;
    MSize szEISCropSize;
    if(!getEISCropRegion(nodeID, pRequest, ptEISStartPt, szEISCropSize))
        return MFALSE;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO) ||
        !configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO, true))
        return MFALSE;
    // FD output
    this->_buildQParam_FD(iFrameNum, pRequest, ptEISStartPt, szEISCropSize, rQFiller);
    //
    rQFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            //MY_S
            insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_MYS).
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf_MYS->getImgSize()).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O);
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParams_BAYER_STANDALONE(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD2("+");
    // copy template
    rOutParm = mQParam_BAYER_STANDALONE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());
    //
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result : main1
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    // output: MV_F
    IImageBuffer* frameBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);
    // ouput: IMG3O buffer
    IImageBuffer* pImgBuf_IMG3O = pBufferHandler->requestBuffer(nodeID, BID_P2A_INTERNAL_IMG3O);

    // fill input - IMGI
    qParamFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW
    // fill inputs - VIPI
    if(mp3DNR_IMG3OBuf.get() == nullptr)
        qParamFiller.delInputPort(iFrameNum, PORT_VIPI);
    else
        qParamFiller.insertInputBuf(iFrameNum, PORT_VIPI, mp3DNR_IMG3OBuf->mImageBuffer.get());
    // LCE
    if(!this->_fillLCETemplate(iFrameNum, pRequest, tuningRes, qParamFiller, mSRZ4Config_LCENR_Bayer))
        return MFALSE;
    // get crop region
    MPoint ptEISStartPt;
    MSize szEISCropSize;
    if(!getEISCropRegion(eDPETHMAP_PIPE_NODEID_P2ABAYER, pRequest, ptEISStartPt, szEISCropSize))
        return MFALSE;
    // FD output
    this->_buildQParam_FD(iFrameNum, pRequest, ptEISStartPt, szEISCropSize, qParamFiller);
    //
    qParamFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O);

    MBOOL bRet = qParamFiller.validate();
    VSDOF_LOGD2("- res=%d", bRet);
    return bRet;
}


MBOOL
DepthQTemplateProvider::
queryRemappedEISRegion(
    sp<DepthMapEffectRequest> pRequest,
    MSize szEISDomain,
    MSize szRemappedDomain,
    eis_region& rOutRegion
)
{
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    IMetadata* pMeta_InHal = pRequest->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    if(!queryEisRegion(pMeta_InHal, rOutRegion))
    {
        MY_LOGE("Failed to query EIS region!");
        return MFALSE;
    }


    VSDOF_LOGD2("Original query-EIS region: startPt=(%d.%d, %d.%d) Size=(%d, %d)",
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

    VSDOF_LOGD2("Remapped query-EIS region: startPt=(%d.%d, %d.%d) Size=(%d, %d)",
            rOutRegion.x_int, rOutRegion.x_float, rOutRegion.y_int, rOutRegion.y_float,
            rOutRegion.s.w, rOutRegion.s.h);

    return MTRUE;
}

MINT32
DepthQTemplateProvider::
remapTransformForPostview(ENUM_ROTATION moduleRotation, ENUM_ROTATION jpegOrientation)
{
    //Try to rotate back to original and then rotate to jpegOrientation
    MINT32 postViewRot = (360 - moduleRotation + jpegOrientation)%360;
    MINT32 finalPostViewRot = remapTransform((ENUM_ROTATION)postViewRot);
    VSDOF_LOGD2("Postview rotate: moduleRotation:%d,jpegOrientation:%d, postViewRot:%d iModuleTrans:%d",moduleRotation, jpegOrientation, postViewRot, finalPostViewRot);
    return finalPostViewRot;
}

MVOID
DepthQTemplateProvider::
onHandleP2Done(DepthMapPipeNodeID nodeID, sp<DepthMapEffectRequest> pRequest)
{
    const EffectRequestAttrs& attr = pRequest->getRequestAttr();
    // NR3D exist in P2ANode when need n3d
    if(nodeID == eDPETHMAP_PIPE_NODEID_P2ABAYER && attr.opState == eSTATE_NORMAL)
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        if(!pBufferHandler->getEnquedSmartBuffer(nodeID, BID_P2A_INTERNAL_IMG3O, mp3DNR_IMG3OBuf))
        {
            MY_LOGE("Failed to get the BID_P2A_INTERNAL_IMG3O buffer!");
        }
    }
}

MBOOL
DepthQTemplateProvider::
getEISCropRegion(
    DepthMapPipeNodeID nodeID,
    sp<DepthMapEffectRequest> pRequest,
    MPoint& rptCropStart,
    MSize& rszCropSize
)
{
    // check EIS crop region
    if (pRequest->getRequestAttr().isEISOn)
    {
        eis_region region;
        IMetadata* pMeta_InHal = pRequest->getBufferHandler()->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
        // set MV_F crop for EIS
        if(queryEisRegion(pMeta_InHal, region))
        {
            rptCropStart = MPoint(region.x_int, region.y_int);
            rszCropSize = region.s;
            return MTRUE;
        }
        else
        {
            MY_LOGE("Query EIS Region Failed! reqID=%d.", pRequest->getRequestNo());
            return MFALSE;
        }
    }
    else
    {
        IImageBuffer* frameBuf_RSRAW1 = pRequest->getBufferHandler()->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
        rptCropStart = MPoint(0, 0);
        rszCropSize = frameBuf_RSRAW1->getImgSize();
        return MTRUE;
    }
}

IImageBuffer*
DepthQTemplateProvider::
get3DNRVIPIBuffer()
{
    if(mp3DNR_IMG3OBuf == nullptr)
        return nullptr;
    else
        return mp3DNR_IMG3OBuf->mImageBuffer.get();
}

MVOID
DepthQTemplateProvider::
_buildQParam_FD(
    int iFrameNum,
    DepthMapRequestPtr pRequest,
    MPoint ptCropStart,
    MSize szCropSize,
    QParamTemplateFiller& rQFiller
)
{
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // no FD-> request internal FD
    if(!bExistFD)
    {
        rQFiller.delOutputPort(iFrameNum, PORT_IMG2O, eCROP_CRZ);
    }
    else
    {
        rQFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).   // FD output
            setCrop(iFrameNum, eCROP_CRZ, ptCropStart, szCropSize, pFdBuf->getImgSize());
    }
}


MVOID
DepthQTemplateProvider::
_buildQParam_FD_n_FillInternal(
    DepthMapPipeNodeID nodeID,
    int iFrameNum,
    DepthMapRequestPtr pRequest,
    MPoint ptCropStart,
    MSize szCropSize,
    QParamTemplateFiller& rQFiller
)
{
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // no FD-> request internal FD
    if(!bExistFD)
    {
        pFdBuf = pRequest->getBufferHandler()->requestBuffer(nodeID, BID_P2A_INTERNAL_FD);
        VSDOF_LOGD("no FD buffer, reqID=%d", pRequest->getRequestNo());
    }

    // insert buffer
    rQFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).   // FD output
            setCrop(iFrameNum, eCROP_CRZ, ptCropStart, szCropSize, pFdBuf->getImgSize());
}

MBOOL
DepthQTemplateProvider::prepareQParamsTemplate(MBOOL bIsSkipFEFM,
                        DepthMapPipeOpState state, MINT32 iModuleTrans, QParams& rQParam)
{
    if (mpSizeMgr == nullptr)
        return MFALSE;

    if (state == eSTATE_NORMAL)
        return prepareQParamsTemplate_NORMAL(bIsSkipFEFM, iModuleTrans, rQParam);
    else
        return prepareQParamsTemplate_CAPTURE(iModuleTrans, rQParam);
}

MBOOL
DepthQTemplateProvider::prepareQParamsTemplate_CAPTURE(MINT32 iModuleTrans, QParams& rQParam)
{
    MY_LOGD("+");
    MBOOL bSuccess = MTRUE;
    bSuccess &= prepareQParam_frame0_cap(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame1_cap(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame2_cap(rQParam);
    bSuccess &= prepareQParam_frame3_cap(rQParam);
    bSuccess &= prepareQParam_frame4_cap(rQParam);
    bSuccess &= prepareQParam_frame5_cap(rQParam);
    bSuccess &= prepareQParam_frame6_9_cap(rQParam);

    MY_LOGD("- success=%d", bSuccess);
    return  bSuccess;
}

MBOOL
DepthQTemplateProvider::prepareQParam_frame0_cap(MINT32 iModuleTrans, QParams& rQParam)
{
    const MUINT iFrameNum = 0;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_WROT, MPoint(0,0), MSize(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2).
            addOutput(mapToPortID(BID_P2A_FE2B_INPUT), iModuleTrans).
            generate(rQParam);

    return bSuccess;
}

MBOOL
DepthQTemplateProvider::prepareQParam_frame1_cap(MINT32 iModuleTrans, QParams& rQParam)
{
    const MUINT iFrameNum = 1;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);
    //
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            // WROT: FE1B input
            addCrop(eCROP_WROT, MPoint(0,0), MSize(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(mapToPortID(BID_P2A_FE1B_INPUT), iModuleTrans).  // do module rotation
            generate(rQParam);

    return bSuccess;
}

MBOOL DepthQTemplateProvider::prepareQParam_frame2_cap(QParams& rQParam)
{
    MUINT iFrameNum = 2;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    //Since preview change to stage base, iFrameNum divide by 2 as stage 1
    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum/2);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2,
                                            rP2ASize.mFEC_INPUT_SIZE_MAIN2).//IMG2O:FE2CO input
            addOutput(mapToPortID(BID_P2A_FE2C_INPUT)).
            addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN2, MSize(0,0)).//WDMA:Rect_in2
            addOutput(mapToPortID(BID_P2A_OUT_RECT_IN2)).
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1)).
            generate(rQParam);

    return bSuccess;
}

MBOOL DepthQTemplateProvider::prepareQParam_frame3_cap(QParams& rQParam)
{
    const MUINT iFrameNum = 3;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addCrop(eCROP_CRZ, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1,
                                            rP2ASize.mFEC_INPUT_SIZE_MAIN1).  // IMG2O: FE1CO input
            addOutput(mapToPortID(BID_P2A_FE1C_INPUT)).
            addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).  // WDMA
            addOutput(mapToPortID(BID_N3D_OUT_MV_Y)).
            addOutput(PORT_FEO).                           // FEO
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1)).
            generate(rQParam);

    return bSuccess;
}

MBOOL DepthQTemplateProvider::prepareQParam_frame4_cap(QParams& rQParam)
{
    MUINT iFrameNum = 4;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    //Since preview change to stage base, iFrameNum divide by 2 as stage 2
    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum/2);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addOutput(PORT_FEO).                           // FEO
            addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).
            generate(rQParam);

    return bSuccess;
}

MBOOL DepthQTemplateProvider::prepareQParam_frame5_cap(QParams& rQParam)
{
    const MUINT iFrameNum = 5;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            addOutput(PORT_FEO).   // FEO
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).
            generate(rQParam);

    return bSuccess;
}

MBOOL DepthQTemplateProvider::prepareQParam_frame6_9_cap(QParams& rQParam)
{
    MBOOL bSuccess = MTRUE;
    //--> frame 6 ~ 9
    for (int frameID = 6; frameID < 10;++frameID)
    {
        bSuccess &=
            QParamTemplateGenerator(frameID, miSensorIdx_Main1, ENormalStreamTag_FM).
                addInput(PORT_LFEOI).
                addInput(PORT_RFEOI).
                addOutput(PORT_FMO).
                addExtraParam(EPIPE_FM_INFO_CMD, (MVOID*)&mFMTuningBufferMap_cap.
                                                         valueFor(frameID)).
                generate(rQParam);

        if (!bSuccess)
            MY_LOGE("%s at framID%d with error", __func__, frameID);
    }
    return bSuccess;
}

MBOOL DepthQTemplateProvider::buildQParams_CAPTURE( DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult, QParams& rOutParm)
{
    CAM_ULOGM_TAGLIFE("P2ANode::buildQParams_CAPTURE");
    VSDOF_LOGD2("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_CAPTURE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MBOOL bRet = MTRUE;
    bRet &= buildQParam_frame0_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame1_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame2_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame3_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame4_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame5_cap(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame6to9_cap(pRequest, tuningResult, qParamFiller);

    if (!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame0_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_YVY2          = pBufferHandler->requestBuffer(
                                                           nodeID, BID_P2A_IN_YUV2);
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if (!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam,
                          EPortIndex_WROTO))
        return MFALSE;
    // output: FE2B input(WROT)
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2B_INPUT);
    // filler buffers and the raw size crop info
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_YVY2). // input: Main2 RSRAW
             // output: FE2B input(WROT, MDPCrop2)
             setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_YVY2->getImgSize(),
                     mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE).mFEB_INPUT_SIZE_MAIN2).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2B_INPUT), pImgBuf).
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main2.tuningBuffer).
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame1_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // FOV crop : already applied in P1 RRZO
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_YVY1          = pBufferHandler->requestBuffer(
                                                           nodeID, BID_P2A_IN_YUV1);
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if (!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam,
                          EPortIndex_WROTO))
        return MFALSE;
    // ouput: FE1B_input
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    // fill input buffer/tuning buffer
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_YVY1). // input: Main1 YVY1
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pImgBuf). //FE1B_input
             setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_YVY1->getImgSize(),
                                                         pImgBuf->getImgSize()).
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main1.tuningBuffer).
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame2_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    //--> frame 2
    MUINT iFrameNum = 2;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: fe2bBuf_in
    IImageBuffer* fe2bBuf_in = nullptr;
    MBOOL bRet = pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);

    if (CC_UNLIKELY(!bRet))
        return MFALSE;
    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if (!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2,
                          *pPqParam, EPortIndex_WDMAO))
        return MFALSE;
    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
             setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(),
                                                         pRectIn2Buf->getImgSize()).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main2.tuningBuffer). // tuning data
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame3_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 3;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: FE1BBuf_in
    IImageBuffer* pFE1BImgBuf = nullptr;
    MBOOL bRet = pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: MV_Y### question
    IImageBuffer* pN3dMvYBuf = pBufferHandler->requestBuffer(nodeID, BID_N3D_OUT_MV_Y);
    ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_CAPTURE;
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    if (CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1,
                                    *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
             setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), pFE1BImgBuf->getImgSize(),
                                                         pN3dMvYBuf->getImgSize()).
             insertOutputBuf(iFrameNum, mapToPortID(BID_N3D_OUT_MV_Y), pN3dMvYBuf). // Rect_in1
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main1.tuningBuffer).
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame4_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 4;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input: fe2coBuf_in
    IImageBuffer* pFE2CBuf_in = nullptr;
    MBOOL bRet = pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2C_INPUT, pFE2CBuf_in);
    // output: FE2CO
    IImageBuffer* pFE2COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2CO);
    if (CC_UNLIKELY(!bRet))
        return MFALSE;

    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if (!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main2,
                                     *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE2CBuf_in). // FE2CBuf_in
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2CO), pFE2COBuf). // FE2COBuf
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main2.tuningBuffer).
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame5_cap(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 5;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // input: fe1coBuf_in
    IImageBuffer* pFE1CBuf_in = nullptr;
    MBOOL bRet = pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1C_INPUT, pFE1CBuf_in);
    // output: FE1CO
    IImageBuffer* pFE1COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1CO);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(nodeID, BID_PQ_PARAM);
    if(!configurePQParam(nodeID, iFrameNum, pRequest, miSensorIdx_Main1,
                                    *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1CBuf_in). // FE2CBuf_in
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1CO), pFE1COBuf). // FE1CO
             insertTuningBuf(iFrameNum, tuningResult.tuningRes_FE_main1.tuningBuffer).
             addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL DepthQTemplateProvider::buildQParam_frame6to9_cap(DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult, QParamTemplateFiller& rQFiller)
{
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

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
    MVOID* pTuningBuf      = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_LSide[0]]).
             insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_RSide[0]]).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_LR), pFMBOBuf).
             insertTuningBuf(iFrameNum, pTuningBuf);


    //--> frame 7: FM - R to L
    iFrameNum = 7;
    // output: FMBO_RL
    pFMBOBuf   = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMBO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_RSide[0]]).
             insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_LSide[0]]).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMBO_RL), pFMBOBuf).
             insertTuningBuf(iFrameNum, pTuningBuf);

    //--> frame 8: FM - L to R
    iFrameNum = 8;
    // output: FMCO_LR
    pFMBOBuf   = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_LR);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_LSide[1]]).
             insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_RSide[1]]).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_LR), pFMBOBuf).
             insertTuningBuf(iFrameNum, pTuningBuf);

    ///--> frame 9: FM - R to L
    iFrameNum = 9;
    // output: FMCO_LR
    pFMBOBuf   = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FMCO_RL);
    pTuningBuf = pBufferHandler->requestWorkingTuningBuf(nodeID, BID_P2A_TUNING);

    rQFiller.insertInputBuf(iFrameNum, PORT_RFEOI, feoBuf[frameIdx_RSide[1]]).
             insertInputBuf(iFrameNum, PORT_LFEOI, feoBuf[frameIdx_LSide[1]]).
             insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FMCO_RL), pFMBOBuf).
             insertTuningBuf(iFrameNum, pTuningBuf);

    return MTRUE;
}

MVOID
DepthQTemplateProvider::setupFMTuningInfo_cap(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameID)
{
    MY_LOGD("+");

    MSize szFEBufSize = (iFrameID<=7) ?
                        mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEB_INPUT_SIZE_MAIN1:
                        mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD).mFEC_INPUT_SIZE_MAIN1;
    MUINT32 iStage = (iFrameID<=7) ? 1 : 2;

    ENUM_FM_DIRECTION eDir = (iFrameID % 2 == 0) ? E_FM_L_TO_R : E_FM_R_TO_L;
    // query tuning parameter
    StereoTuningProvider::getFMTuningInfo(eDir, fmInfo);
    MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(iStage);
    // set width/height
    fmInfo.mFMWIDTH  = szFEBufSize.w/iBlockSize;
    fmInfo.mFMHEIGHT = szFEBufSize.h/iBlockSize;

    MY_LOGD("-");
}
}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
