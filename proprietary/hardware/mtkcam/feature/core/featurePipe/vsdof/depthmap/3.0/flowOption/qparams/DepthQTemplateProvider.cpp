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
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
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
    mFMTuningBufferMap.clear();

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
        case BID_P2A_OUT_CC_IN1:
        case BID_P2A_OUT_CC_IN2:
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
            return PORT_PAK2O;

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
    // FE SRZ template - frame 2/4 need FEO srz crop
    _SRZ_SIZE_INFO_ srzInfo_frame2;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame2, rP2ASize.mFEBO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(2,  srzInfo_frame2);
    _SRZ_SIZE_INFO_ srzInfo_frame4;
    CONFIG_SRZINFO_TO_CROPAREA(srzInfo_frame4, rP2ASize.mFECO_AREA_MAIN2);
    mSrzSizeTemplateMap.add(4,  srzInfo_frame4);
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

    MBOOL bRet = prepareQParamsTemplate(MFALSE, eSTATE_NORMAL, iModuleTrans, mQParam_NORMAL);
    bRet = prepareQParamsTemplate(MTRUE, eSTATE_NORMAL, iModuleTrans, mQParam_NORMAL_NOFEFM);
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
    MBOOL bRet = prepareQParamsTemplate_BAYER_NORMAL_NOFEFM(iModuleTrans, mQParam_BAYER_NORMAL_NOFEFM);
    bRet &= prepareQParamsTemplate_BAYER_CAPTURE(iModuleTrans, mQParam_BAYER_CAPTURE);
    bRet &= prepareQParamsTemplate_BAYER_STANDALONE(iModuleTrans, mQParam_BAYER_STANDALONE);
    if(!bRet)
    {
        MY_LOGE("prepareQParamsTemplate failed!!!");
        return MFALSE;
    }
    VSDOF_LOGD("-");
    return MTRUE;
}

MVOID
DepthQTemplateProvider::
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
DepthQTemplateProvider::
setupPQParamSetting(DpPqParam* pParam, MUINT iFrameID)
{
    ENUM_PASS2_ROUND pass2Round;
    convertFrameIdToPass2Round(iFrameID, pass2Round);
    StereoTuningProvider::getPass2MDPPQParam(pass2Round, pParam->u.isp.vsdofParam);
}

MBOOL
DepthQTemplateProvider::
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
DepthQTemplateProvider::
prepareQParamsTemplate(
    MBOOL bIsSkipFEFM,
    DepthMapPipeOpState state,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    if(mpSizeMgr == nullptr)
        return MFALSE;

    if(state == eSTATE_NORMAL)
        return prepareQParamsTemplate_NORMAL(bIsSkipFEFM, iModuleTrans, rQParam);
    else
        return prepareQParamsTemplate_CAPTURE(iModuleTrans, rQParam);
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
                addCrop(eCROP_WROT, MPoint(0,0), szTBD, rP2ASize.mRECT_IN_CONTENT_SIZE_MAIN2, true).
                // Rect_in2 : need module transform
                addOutput(PORT_WROTO, iModuleTrans).
                generate(rQParam);
    }
    else
    {
        bSuccess = prepareQParam_frame0(iModuleTrans, rQParam);
        bSuccess &= prepareQParam_frame1(eSTATE_NORMAL, iModuleTrans, rQParam);
        bSuccess &= prepareQParam_frame2(rQParam);
        bSuccess &= prepareQParam_frame3(rQParam);
        bSuccess &= prepareQParam_frame4(rQParam);
        bSuccess &= prepareQParam_frame5(rQParam);
        bSuccess &= prepareQParam_frame6_9(rQParam);
    }

    MY_LOGD("- success=%d", bSuccess);
    return bSuccess;
}

MVOID
DepthQTemplateProvider::
_prepareLCETemplate(QParamTemplateGenerator& generator)
{
    generator.addInput(PORT_LCEI).
            addInput(PORT_DEPI);
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
                delInputPort(iFrameNum, PORT_DEPI);
    }
    else
    {
        if(!setupSRZ4LCENR(pRequest, rSrzInfo))
            return MFALSE;
        auto pBufferHandler = pRequest->getBufferHandler();
        IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(eDPETHMAP_PIPE_NODEID_P2A, PBID_IN_LCSO1);
        rQFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCSOBuf).
                insertInputBuf(iFrameNum, PORT_DEPI, pLCSOBuf).
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
prepareQParamsTemplate_CAPTURE(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MY_LOGD("+");
    MBOOL bSuccess = prepareQParam_frame0(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame1(eSTATE_CAPTURE, iModuleTrans, rQParam);
    bSuccess &= prepareQParam_frame2(rQParam);
    bSuccess &= prepareQParam_frame3(rQParam);
    bSuccess &= prepareQParam_frame4(rQParam);
    bSuccess &= prepareQParam_frame5(rQParam);
    bSuccess &= prepareQParam_frame6_9(rQParam);

    MY_LOGD("- success=%d", bSuccess);
    return  bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParam_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
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
prepareQParam_frame1(
    DepthMapPipeOpState state,
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 1;
    ENUM_STEREO_SCENARIO scenario = (state == eSTATE_CAPTURE) ?
                                        eSTEREO_SCENARIO_CAPTURE : eSTEREO_SCENARIO_RECORD;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(scenario);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI);
    // prepare LCE
    this->_prepareLCETemplate(generator);
    // crop information
    MPoint ptOrigin(0,0);
    MSize szTBD(0,0);

    if(state == eSTATE_NORMAL)
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
    else
    {
        // WROT: FE1B input
        generator.addCrop(eCROP_WROT, ptOrigin, szTBD, rP2ASize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(mapToPortID(BID_P2A_FE1B_INPUT), iModuleTrans);    // do module rotation;
    }

    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParam_frame2(QParams& rQParam)
{
    MUINT iFrameNum = 2;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum);

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
            addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1));

    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParam_frame3(
    QParams& rQParam
)
{
    MUINT iFrameNum = 3;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    QParamTemplateGenerator templateGen =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addCrop(eCROP_CRZ, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1, rP2ASize.mFEC_INPUT_SIZE_MAIN1).  // IMG2O: FE1CO input
        addOutput(mapToPortID(BID_P2A_FE1C_INPUT)).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).  // WDMA : Rect_in1
        addOutput(mapToPortID(BID_P2A_OUT_RECT_IN1)).
        addOutput(PORT_FEO).                           // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(1));

    MBOOL bSuccess = templateGen.generate(rQParam);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParam_frame4(QParams& rQParam)
{
    MUINT iFrameNum = 4;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    MVOID* srzInfo = (MVOID*)&mSrzSizeTemplateMap.valueFor(iFrameNum);
    MBOOL bSuccess =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main2, ENormalStreamTag_Normal).
        addInput(PORT_IMGI).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEC_INPUT_SIZE_MAIN2, rP2ASize.mCCIN_SIZE_MAIN2).  // WDMA: CC_in2
        addOutput(mapToPortID(BID_P2A_OUT_CC_IN2)).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).                           // FEO
        addModuleInfo(EDipModule_SRZ1, srzInfo).       // FEO SRZ1 config
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2)).
        generate(rQParam);

    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParam_frame5(QParams& rQParam)
{
    //--> frame 5
    MUINT iFrameNum = 5;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);

    QParamTemplateGenerator templateGen =
    QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).   // frame 5
        addInput(PORT_IMGI).
        addCrop(eCROP_WDMA, MPoint(0,0), rP2ASize.mFEC_INPUT_SIZE_MAIN1, rP2ASize.mCCIN_SIZE_MAIN1).  // WDMA: CC_in1
        addOutput(mapToPortID(BID_P2A_OUT_CC_IN1)).
        #if defined(GTEST) && !defined(GTEST_PROFILE)
        addOutput(PORT_IMG3O).
        #endif
        addOutput(PORT_FEO).   // FEO
        addExtraParam(EPIPE_FE_INFO_CMD, (MVOID*)&mFETuningBufferMap.valueFor(2));

    return templateGen.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
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
            addOutput(PORT_PAK2O).
            addExtraParam(EPIPE_FM_INFO_CMD, (MVOID*)&mFMTuningBufferMap.valueFor(frameID)).
            generate(rQParam);
    }
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
    CAM_TRACE_NAME("P2ANode::buildQParams_NORMAL");
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_NORMAL;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MBOOL bRet = buildQParam_frame0(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame1(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame2(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame3(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame4(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame5(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame6to9(pRequest, tuningResult, qParamFiller);
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
    CAM_TRACE_NAME("P2ANode::buildQParams_NORMAL");
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_NORMAL_NOFEFM;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_RSRAW2 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW2);
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ? eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN2);

    // PQParam
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WROTO))
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

    VSDOF_LOGD("-");
    return MTRUE;

}

MBOOL
DepthQTemplateProvider::
buildQParams_CAPTURE(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    CAM_TRACE_NAME("P2ANode::buildQParams_CAPTURE");
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    // copy template
    rOutParm = mQParam_CAPTURE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MBOOL bRet = buildQParam_frame0(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame1_capture(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame2_capture(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame3_capture(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame4(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame5(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_frame6to9(pRequest, tuningResult, qParamFiller);

    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;
}


MBOOL
DepthQTemplateProvider::
buildQParam_frame0(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* frameBuf_RSRAW2 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW2);
    AAATuningResult tuningRes = tuningResult.tuningRes_main2;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WROTO))
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
buildQParam_frame1(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // FOV crop : already applied in P1 RRZO
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
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
    // check exist Request FD buffer or not
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // no FD-> request internal FD
    if(!bExistFD)
        pFdBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_INTERNAL_FD);
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO) ||
        !configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO, true))
        return MFALSE;
    // get crop region
    MPoint ptEISStartPt;
    MSize szEISCropSize;
    if(!getEISCropRegion(eDPETHMAP_PIPE_NODEID_P2A, pRequest, ptEISStartPt, szEISCropSize))
        return MFALSE;

    rQFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).   // FD output
            setCrop(iFrameNum, eCROP_CRZ, ptEISStartPt, szEISCropSize, pFdBuf->getImgSize()).
            insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pFE1B_Input_Buf). //FE1B_input, apply MDPCrop2
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pFE1B_Input_Buf->getImgSize(), true).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame1_capture(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // FOV crop : already applied in P1 RRZO
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2A;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO))
        return MFALSE;

    // fill input buffer/tuning buffer
    rQFiller.insertTuningBuf(iFrameNum, tuningRes.tuningBuffer). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, frameBuf_RSRAW1); // input: Main1 RSRAW

    // ouput: FE1B_input
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pImgBuf). //FE1B_input
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf->getImgSize()).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame2(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in =  pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ?
                                        eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN2);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);
	if(CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(), pRectIn2Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
            insertTuningBuf(iFrameNum, pTuningBuf). // tuning data
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame2_capture(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2B_INPUT, fe2bBuf_in);
    // output: fe2cBuf_in
    IImageBuffer* fe2cBuf_in =  pBufferHandler->requestBuffer(nodeID, BID_P2A_FE2C_INPUT);
    // output: Rect_in2
    IImageBuffer* pRectIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN2);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ?
                                        eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    // Rect_in2 : set active area
    pRectIn2Buf->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN2);
    // output: FE2BO
    IImageBuffer* pFE2BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;
    // output: Rect_in2 NEEDs setCrop because the Rect_in size is change for each scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, fe2bBuf_in).  // fe2bBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE2C_INPUT), fe2cBuf_in). // fe2cBuf_in
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), fe2bBuf_in->getImgSize(), pRectIn2Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN2), pRectIn2Buf). // Rect_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2BO), pFE2BOBuf). // FE2BO
            insertTuningBuf(iFrameNum, pTuningBuf). // tuning data
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);
    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame3(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in =pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: Rect_in1
    IImageBuffer* pRectIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN1);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ? eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    pRectIn1Buf->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN1);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in IMG2O
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), pFE1BImgBuf->getImgSize(), pRectIn1Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN1), pRectIn1Buf). // Rect_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame3_capture(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);
    // output: FE1C_input
    IImageBuffer* pFE1CBuf_in =pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1C_INPUT);
    // output: Rect_in1
    IImageBuffer* pRectIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN1);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ?
                                        eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    // Rect_in1 : set active area
    pRectIn1Buf->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN1);
    // output: FE1BO
    IImageBuffer* pFE1BOBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1BO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    // PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1C_INPUT), pFE1CBuf_in). // FE1C_input
            setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), pFE1BImgBuf->getImgSize(), pRectIn1Buf->getImgSize()).
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_RECT_IN1), pRectIn1Buf). // Rect_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1BO), pFE1BOBuf). // FE1BO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE2_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE2_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame4(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE2C_INPUT, pFE2CBuf_in);
    // output: CC_in2
    IImageBuffer* pCCIn2Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_CC_IN2);
    // output: FE2CO
    IImageBuffer* pFE2COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE2CO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main2, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE2CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_CC_IN2), pCCIn2Buf). // CC_in2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE2CO), pFE2COBuf). // FE2COBuf
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);

    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN2);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN2), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame5(
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
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1C_INPUT, pFE1CBuf_in);
    // output: CC_in1
    IImageBuffer* pCCIn1Buf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_CC_IN1);
    // output: FE1CO
    IImageBuffer* pFE1COBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_FE1CO);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO))
        return MFALSE;

    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1CBuf_in). // FE2CBuf_in
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_CC_IN1), pCCIn1Buf). // CC_in1
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_OUT_FE1CO), pFE1COBuf). // FE1CO
            insertTuningBuf(iFrameNum, pTuningBuf).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);


    #if defined(GTEST) && !defined(GTEST_PROFILE)
    IImageBuffer* pFEInputBuf = pBufferHandler->requestBuffer(nodeID, BID_FE3_HWIN_MAIN1);
    rQFiller.insertOutputBuf(iFrameNum, mapToPortID(BID_FE3_HWIN_MAIN1), pFEInputBuf);
    #endif
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_frame6to9(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
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
DepthQTemplateProvider::
prepareQParamsTemplate_BAYER_NORMAL_NOFEFM(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    VSDOF_LOGD("+");
    MBOOL bSuccess = prepareQParam_Bayer_NORMAL_NOFEFM_frame0(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_Bayer_NORMAL_NOFEFM_frame1(iModuleTrans, rQParam);
    VSDOF_LOGD("- : res=%d", bSuccess);
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
        // WROT : Rect_in1 need module transform
        addCrop(eCROP_WROT, ptOrigin, szTBD, rP2ASize.mRECT_IN_CONTENT_SIZE_MAIN1).
        addOutput(PORT_WROTO, iModuleTrans).
        addOutput(PORT_IMG3O); //for 3dnr
    this->_prepareLCETemplate(generator);
    return generator.generate(rQParam);
}


MBOOL
DepthQTemplateProvider::
prepareQParam_Bayer_NORMAL_NOFEFM_frame1(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 1;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    // crop information
    MPoint ptOrigin(0,0);
    MSize szTBD(0,0);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
        // IMGI: FD buffer or FD_internal buffer
        addInput(PORT_IMGI).
        // WROT : MY_S
        addCrop(eCROP_WROT, ptOrigin, rP2ASize.mFD_IMG_SIZE, szTBD).
        addOutput(PORT_WROTO, iModuleTrans);

    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParamsTemplate_BAYER_CAPTURE(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    VSDOF_LOGD("+");
    MBOOL bSuccess = prepareQParam_Bayer_CAPTURE_frame0(iModuleTrans, rQParam);
    bSuccess &= prepareQParam_Bayer_CAPTURE_frame1(iModuleTrans, rQParam);
    VSDOF_LOGD("- : res=%d", bSuccess);
    return bSuccess;
}

MBOOL
DepthQTemplateProvider::
prepareQParam_Bayer_CAPTURE_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);

    // need FOV crop
    MPoint ptFD_CROP = rP2ASize.mFD_IMG_CROP.startPt;
    MSize szFD_CROP = rP2ASize.mFD_IMG_CROP.contentSize();
    MPoint ptMVF_CROP = rP2ASize.mMAIN_IMAGE_CROP.startPt;
    MSize szMVF_CROP = rP2ASize.mMAIN_IMAGE_CROP.contentSize();
    MPoint ptBAYER_CROP = rP2ASize.mFEB_INPUT_CROP_MAIN1.startPt;
    MSize szBAYER_CROP = rP2ASize.mFEB_INPUT_CROP_MAIN1.contentSize();

    //
    auto generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            #ifndef GTEST
            addInput(PORT_DMGI).
            #endif
            // IMG2O : FD
            addCrop(eCROP_CRZ, ptFD_CROP, szFD_CROP, rP2ASize.mFD_IMG_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_FDIMG)).
            // WDMA : MV_F_CAP
            addCrop(eCROP_WDMA, ptMVF_CROP, szMVF_CROP, rP2ASize.mMAIN_IMAGE_SIZE).
            addOutput(mapToPortID(BID_P2A_OUT_MV_F), 0 ,EPortCapbility_Cap).
            // WROT: input for next run
            addCrop(eCROP_WROT, ptBAYER_CROP, szBAYER_CROP, rP2ASize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(PORT_WROTO);                // do module rotation
    // LCE
    this->_prepareLCETemplate(generator);
    return generator.generate(rQParam);
}

MBOOL
DepthQTemplateProvider::
prepareQParam_Bayer_CAPTURE_frame1(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    //second bayer run - input: Bayer out, output: PostView (no MY_S needed)
    const P2ABufferSize& rP2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);
    MUINT iFrameNum = 1;

    // FOV crop already applied in RRZO
    MPoint ptOrigin(0, 0);
    MSize szNotSet(0, 0);
    MBOOL bSuccess =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal).
            addInput(PORT_IMGI).
            // WROT: PostView
            addCrop(eCROP_WROT, ptOrigin, rP2ASize.mFEB_INPUT_SIZE_MAIN1, szNotSet).
            addOutput(PORT_WROTO).
            // WROT: MY_S
            addCrop(eCROP_WDMA, ptOrigin, rP2ASize.mFEB_INPUT_SIZE_MAIN1, szNotSet).
            addOutput(PORT_WDMAO).
            generate(rQParam);

    return bSuccess;
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
    VSDOF_LOGD("+");
    // copy template
    rOutParm = mQParam_BAYER_NORMAL_NOFEFM;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());
    //
    MBOOL bRet = buildQParam_Bayer_NORMAL_NOFEFM_frame0(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_Bayer_NORMAL_NOFEFM_frame1(pRequest, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    VSDOF_LOGD("- res=%d", bRet);
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
    // ouput: Rect_in1
    IImageBuffer* pImgBuf_RectIn1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_RECT_IN1);
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ?
                                        eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    pImgBuf_RectIn1->setExtParam(mpSizeMgr->getP2A(scenario).mRECT_IN_CONTENT_SIZE_MAIN1);
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
    // check exist Request FD buffer or not
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // no FD-> request internal FD
    if(!bExistFD)
        pFdBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_INTERNAL_FD);
    // get crop region
    MPoint ptEISStartPt;
    MSize szEISCropSize;
    if(!getEISCropRegion(eDPETHMAP_PIPE_NODEID_P2ABAYER, pRequest, ptEISStartPt, szEISCropSize))
        return MFALSE;
    //
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO) ||
        !configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WDMAO, true))
        return MFALSE;

    rQFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).   // FD output
            setCrop(iFrameNum, eCROP_CRZ, ptEISStartPt, szEISCropSize, pFdBuf->getImgSize()).
            insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            //Rect_in1 : apply MDPCrop2
            insertOutputBuf(iFrameNum, mapToPortID(BID_P2A_FE1B_INPUT), pImgBuf_RectIn1).
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf_RectIn1->getImgSize(), true).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O);
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParam_Bayer_NORMAL_NOFEFM_frame1(
    DepthMapRequestPtr& pRequest,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input : FD
    IImageBuffer* pFdBuf = nullptr;
    // no FD-> request internal FD
    if(!pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf))
    {
        if(!pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_OUT_INTERNAL_FD, pFdBuf))
        {
            MY_LOGE("Failed to get FD buffer! reqID=%d", pRequest->getRequestNo());
            return MFALSE;
        }
    }
    // output: MY_S
    IImageBuffer* pImgBuf_MY_S = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    //PQ
    PQParam* pPqParam = (PQParam*)pBufferHandler->requestWorkingTuningBuf(BID_PQ_PARAM);
    if(!configurePQParam(iFrameNum, pRequest, miSensorIdx_Main1, *pPqParam, EPortIndex_WROTO, true))
        return MFALSE;

    // fill input - IMGI
    rQFiller.insertTuningBuf(iFrameNum, pTuningBuf). // insert tuning data
            insertInputBuf(iFrameNum, PORT_IMGI, pFdBuf). // input: FD
            setCrop(iFrameNum, eCROP_WROT, MPoint(0, 0), pFdBuf->getImgSize(), pImgBuf_MY_S->getImgSize()).
            insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_MY_S).
            addExtraParam(iFrameNum, EPIPE_MDP_PQPARAM_CMD, (void*)pPqParam);
    return MTRUE;
}

MBOOL
DepthQTemplateProvider::
buildQParams_BAYER_CAPTURE(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD("+");
    // copy template
    rOutParm = mQParam_BAYER_CAPTURE;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm, pRequest->getRequestNo());

    MBOOL bRet = buildQParam_Bayer_CAPTURE_frame0(pRequest, tuningResult, qParamFiller);
    bRet &= buildQParam_Bayer_CAPTURE_frame1(pRequest, qParamFiller);
    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    VSDOF_LOGD("- res=%d", bRet);
    return bRet;
}

MBOOL
DepthQTemplateProvider::
buildQParam_Bayer_CAPTURE_frame0(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    //
    MUINT iFrameNum = 0;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
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
    // LCE
    if(!this->_fillLCETemplate(iFrameNum, pRequest, tuningRes, rQFiller, mSRZ4Config_LCENR_Bayer))
        return MFALSE;
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
DepthQTemplateProvider::
buildQParam_Bayer_CAPTURE_frame1(
    DepthMapRequestPtr& pRequest,
    QParamTemplateFiller& rQFiller
)
{
    int iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2ABAYER;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
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

MBOOL
DepthQTemplateProvider::
buildQParams_BAYER_STANDALONE(
    DepthMapRequestPtr& pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD("+");
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
    // check exist Request FD buffer or not
    IImageBuffer* pFdBuf = nullptr;
    MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    // no FD-> request internal FD
    if(!bExistFD)
        qParamFiller.delOutputPort(iFrameNum, PORT_IMG2O, eCROP_CRZ);
    else
    {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pFdBuf).   // FD output
                 setCrop(iFrameNum, eCROP_CRZ, ptEISStartPt, szEISCropSize, pFdBuf->getImgSize());
    }
    qParamFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, frameBuf_MV_F).  // MV_F
            setCrop(iFrameNum, eCROP_WDMA, ptEISStartPt, szEISCropSize, frameBuf_MV_F->getImgSize()).
            insertOutputBuf(iFrameNum, PORT_IMG3O, pImgBuf_IMG3O);

    MBOOL bRet = qParamFiller.validate();
    VSDOF_LOGD("- res=%d", bRet);
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
DepthQTemplateProvider::
remapTransformForPostview(ENUM_ROTATION moduleRotation, ENUM_ROTATION jpegOrientation)
{
    //Try to rotate back to original and then rotate to jpegOrientation
    MINT32 postViewRot = (360 - moduleRotation + jpegOrientation)%360;
    MINT32 finalPostViewRot = remapTransform((ENUM_ROTATION)postViewRot);
    VSDOF_LOGD("Postview rotate: moduleRotation:%d,jpegOrientation:%d, postViewRot:%d iModuleTrans:%d",moduleRotation, jpegOrientation, postViewRot, finalPostViewRot);
    return finalPostViewRot;
}

MVOID
DepthQTemplateProvider::
onHandleP2Done(DepthMapPipeNodeID nodeID, sp<DepthMapEffectRequest> pRequest)
{
    const EffectRequestAttrs& attr = pRequest->getRequestAttr();
    // NR3D exist in P2ANode when need n3d
    if((nodeID == eDPETHMAP_PIPE_NODEID_P2A && attr.opState == eSTATE_NORMAL && attr.needFEFM) ||
        (nodeID == eDPETHMAP_PIPE_NODEID_P2ABAYER && attr.opState == eSTATE_NORMAL && !attr.needFEFM))
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

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
