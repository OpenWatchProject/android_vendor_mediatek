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

/**
 * @file YuvQTemplateProvider.cpp
 * @brief QParams template creator for simple yuv generation ex. FD/Preview frame
*/

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <imageio/ispio_pipe_ports.h>
#include <drv/isp_reg.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "YuvQTemplateProvider.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "YuvQTemplateProvider"
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

YuvQTemplateProvider::
YuvQTemplateProvider(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption,
    DepthMapFlowOption* pFlowOption
)
: miSensorIdx_Main1(pSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(pSetting->miSensorIdx_Main2)
, mpFlowOption(pFlowOption)
, mpPipeOption(pPipeOption)
{

}

YuvQTemplateProvider::
~YuvQTemplateProvider()
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvQTemplateProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
YuvQTemplateProvider::
init(BaseBufferSizeMgr* pSizeMgr)
{
    mpSizeMgr = pSizeMgr;
    return prepareTemplateParams();
}


MBOOL
YuvQTemplateProvider::
buildQParams_NORMAL(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParm
)
{
    VSDOF_LOGD("+, reqID=%d", rEffReqPtr->getRequestNo());

    // copy template
    rOutParm = mQParam_NORMAL;
    // filler
    QParamTemplateFiller qParamFiller(rOutParm);

    MBOOL bRet = buildQParam_frame0(rEffReqPtr, tuningResult, qParamFiller);
         bRet &= buildQParam_frame1(rEffReqPtr, tuningResult, qParamFiller);

    if(!bRet)
        return MFALSE;

    bRet = qParamFiller.validate();
    return bRet;

}

MBOOL
YuvQTemplateProvider::
buildQParam_frame0(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    // current node: P2AFMNode
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input : RSRAW
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);
    // tuning result
    AAATuningResult tuningRes = tuningResult.tuningRes_main1;

    // FOV crop : already applied in P1 RRZO
    MUINT iFrameNum = 0;

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

    // output: MV_F/MY_S
    IImageBuffer* pImgBuf_MV_F = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MV_F);
    // check EIS on/off
    if (rEffReqPtr->getRequestAttr().isEISOn)
    {
        eis_region region;
        IMetadata* pMeta_InHal = pBufferHandler->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
        // set MV_F crop for EIS
        if(queryEisRegion(pMeta_InHal, region))
        {
            rQFiller.setCrop(iFrameNum, eCROP_WDMA, MPoint(region.x_int, region.y_int), region.s, pImgBuf_MV_F->getImgSize());
        }
        else
        {
            MY_LOGE("Query EIS Region Failed! reqID=%d.", rEffReqPtr->getRequestNo());
            return MFALSE;
        }
    }
    else
    {   // MV_F
        rQFiller.setCrop(iFrameNum, eCROP_WDMA, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf_MV_F->getImgSize());
    }
    // buffer
    rQFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, pImgBuf_MV_F);

    // ouput: FE1B_input
    IImageBuffer* pImgBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_FE1B_INPUT);
    rQFiller.insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf). //FE1B_input
            setCrop(iFrameNum, eCROP_WROT, MPoint(0,0), frameBuf_RSRAW1->getImgSize(), pImgBuf->getImgSize());

    return MTRUE;
}

MBOOL
YuvQTemplateProvider::
buildQParam_frame1(
    DepthMapRequestPtr& rEffReqPtr,
    const Stereo3ATuningRes& tuningResult,
    QParamTemplateFiller& rQFiller
)
{
    MUINT iFrameNum = 1;
    DepthMapPipeNodeID nodeID = eDPETHMAP_PIPE_NODEID_P2AFM;
    sp<BaseBufferHandler> pBufferHandler = rEffReqPtr->getBufferHandler();
    // input: FE1BBuf_in
    IImageBuffer* pFE1BImgBuf = nullptr;
    MBOOL bRet=pBufferHandler->getEnqueBuffer(nodeID, BID_P2A_FE1B_INPUT, pFE1BImgBuf);

    // tuning buffer
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);

    if(CC_UNLIKELY(!bRet))
        return MFALSE;

    // Rect_in1 need setCrop. bcz its size is not constant between scenario
    rQFiller.insertInputBuf(iFrameNum, PORT_IMGI, pFE1BImgBuf). // FE1BBuf_in
            insertTuningBuf(iFrameNum, pTuningBuf);

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
                insertOutputBuf(iFrameNum, PORT_WROTO, pMYSBuf);
    }
    else
    {
        IImageBuffer* pMYSBuf = pBufferHandler->requestBuffer(nodeID, BID_P2A_OUT_MY_S);
        rQFiller.setCropResize(iFrameNum, eCROP_WROT, pMYSBuf->getImgSize()).
                insertOutputBuf(iFrameNum, PORT_WROTO, pMYSBuf);
    }

    return MTRUE;
}

MBOOL
YuvQTemplateProvider::
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

MBOOL
YuvQTemplateProvider::
prepareQParam_frame0(
    MINT32 iModuleTrans,
    QParams& rQParam
)
{
    MUINT iFrameNum = 0;
    ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_PREVIEW;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(scenario);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal)
                                .addInput(PORT_IMGI);

    // crop information
    MPoint ptFD_CROP(0,0), ptMVF_CROP(0,0), ptFE1B_CROP(0,0);
    MSize dummySize(0, 0), szFE1B_CROP(0, 0);

    MBOOL bSuccess =
         // IMG2O : FD
        generator.addCrop(eCROP_CRZ, ptFD_CROP, dummySize, rP2AFMSize.mFD_IMG_SIZE).
            addOutput(PORT_IMG2O).
            // WDMA : MV_F
            addCrop(eCROP_WDMA, ptMVF_CROP, dummySize, rP2AFMSize.mMAIN_IMAGE_SIZE).
            addOutput(PORT_WDMAO).
            // WROT: FE1B input
            addCrop(eCROP_WROT, ptFE1B_CROP, szFE1B_CROP, rP2AFMSize.mFEB_INPUT_SIZE_MAIN1).
            addOutput(PORT_WROTO, iModuleTrans).                // do module rotation
            generate(rQParam);

    return bSuccess;
}

MBOOL
YuvQTemplateProvider::
prepareQParam_frame1(
    QParams& rQParam
)
{
    MUINT iFrameNum = 1;
    ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_PREVIEW;
    const P2ABufferSize& rP2AFMSize = mpSizeMgr->getP2A(scenario);
    //
    QParamTemplateGenerator generator =
        QParamTemplateGenerator(iFrameNum, miSensorIdx_Main1, ENormalStreamTag_Normal)
                                .addInput(PORT_IMGI);

    // WROT : MY_S
    MBOOL bSuccess = generator.addCrop(eCROP_WROT, MPoint(0,0), rP2AFMSize.mFEB_INPUT_SIZE_MAIN1, MSize(0,0)).
                        addOutput(PORT_WROTO).
                        generate(rQParam);
    return bSuccess;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvQTemplateProvider Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
YuvQTemplateProvider::
prepareTemplateParams()
{
    MY_LOGD("+");

    // rotation
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    MINT32 iModuleTrans = remapTransform(eRot);

    MBOOL bSuccess = prepareQParam_frame0(iModuleTrans, mQParam_NORMAL);
         bSuccess &= prepareQParam_frame1(mQParam_NORMAL);

    MY_LOGD("-");
    return bSuccess;
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
