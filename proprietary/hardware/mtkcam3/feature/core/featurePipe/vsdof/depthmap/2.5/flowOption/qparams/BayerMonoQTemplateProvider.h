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
 * @file BayerMonoQTemplateProvider.h
 * @brief QParams template creator for stereo feature on B+M platform
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BM_QPARAMS_TEMPLATE_PROVIDER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BM_QPARAMS_TEMPLATE_PROVIDER_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file

// Local header file
#include "BayerBayerQTemplateProvider.h"
#include "../../DepthMapPipe_Common.h"
#include "../../DepthMapEffectRequest.h"
#include "../../bufferPoolMgr/BaseBufferHandler.h"
#include "../../bufferPoolMgr/bufferSize/BaseBufferSizeMgr.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using NSIoPipe::QParams;
/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class BayerMonoQTemplateProvider
 * @brief provide the qparams tempalte for B+M VSDOF flow, inherit B+B one
 */

class BayerMonoQTemplateProvider : public BayerBayerQTemplateProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BayerMonoQTemplateProvider() = delete;
    BayerMonoQTemplateProvider(
                        sp<DepthMapPipeSetting> pSetting,
                        sp<DepthMapPipeOption> pPipeOption,
                        DepthMapFlowOption* pFlowOption);
    virtual ~BayerMonoQTemplateProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerMonoQTemplateProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init(BaseBufferSizeMgr* pSizeMgr);
    /**
     * @brief build the corresponding QParams in runtime of Bayer run
     * @param [in] rEffReqPtr DepthMapRequestPtr
     * @param [in] tuningResult 3A tuning Result
     * @param [out] rOutParm QParams to enque into P2
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL buildQParams_NORMAL(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_CAPTURE(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_NORMAL(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_CAPTURE(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_FD_ONLY(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerBayerQTemplateProvider Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
   // frames: 1 are differnt from VSDOF
    virtual MBOOL prepareQParam_frame1(
                        DepthMapPipeOpState state,
                        MINT32 iModuleTrans,
                        QParams& rQParam);

    virtual MBOOL buildQParam_frame1(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    virtual MBOOL buildQParam_frame1_capture(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    virtual MBOOL prepareQParam_frame3(
                        DepthMapPipeOpState state,
                        QParams& rQParam);

    virtual MBOOL buildQParam_frame3(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    virtual MBOOL buildQParam_frame3_capture(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    virtual MBOOL prepareQParam_frame5(
                    DepthMapPipeOpState state,
                    QParams& rQParam);

    virtual MBOOL buildQParam_frame5_capture(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    virtual NSCam::NSIoPipe::PortID mapToPortID(DepthMapBufferID buffeID);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerMonoQTemplateProvider Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
     /**
     * @brief prepare QParams tuning templates for Bayer run
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareTemplateParams_Bayer();

    // NO FEFM
    virtual MBOOL prepareQParam_NOFEFM_frame1(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL buildQParam_frame1_NOFEFM(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    /**
     * @brief prepare the QParams template for operation state for Bayer run
     * @param [in] state DepthMap Pipe OP state
     * @param [in] iModuleTrans module rotation
     * @param [out] rQTmplate QParams template
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL prepareQParamsTemplate_BAYER_NORMAL(MINT32 iModuleTrans);
    virtual MBOOL prepareQParamsTemplate_BAYER_CAPTURE(MINT32 iModuleTrans);
    virtual MBOOL prepareQParamsTemplate_BAYER_FD_ONLY(MINT32 iModuleTrans);

    /**
     * @brief prepare/build the QParams template for Bayer run when PV/VR
     */

    virtual MBOOL prepareQParam_Bayer_NORMAL_frame0(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL prepareQParam_Bayer_NORMAL_frame1(QParams& rQParam);

    virtual MBOOL buildQParam_Bayer_NORMAL_frame0(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_Bayer_NORMAL_frame1(
                    DepthMapRequestPtr& rEffReqPtr,
                    QParamTemplateFiller& rQFiller);

/**
     * @brief prepare/build the QParams template for Bayer run when CAP
     */

    virtual MBOOL prepareQParam_Bayer_CAPTURE_frame0(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL prepareQParam_Bayer_CAPTURE_frame1(
                                MINT32 iModuleTrans,
                                QParams& rQParam);

    virtual MBOOL buildQParam_Bayer_CAPTURE_frame0(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_Bayer_CAPTURE_frame1(
                    DepthMapRequestPtr& rEffReqPtr,
                    QParamTemplateFiller& rQFiller);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BayerMonoQTemplateProvider protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // QParams template for bayer run - pv/vr
    QParams mQParam_BAYER_NORMAL;
    // QParams template for bayer run - capture
    QParams mQParam_BAYER_CAPTURE;
    // QParams template for bayer run - FD only
    QParams mQParam_BAYER_FD_ONLY;

};


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
