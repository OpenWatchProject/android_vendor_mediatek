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
 * @file YuvQTemplateProvider.h
 * @brief QParams template creator for simple yuv generation ex. FD/Preview frame
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BASIC_QPARAMS_TEMPLATE_PROVIDER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BASIC_QPARAMS_TEMPLATE_PROVIDER_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
// Module header file

// Local header file
#include "../../DepthMapPipe_Common.h"
#include "../../DepthMapEffectRequest.h"
#include "../../DepthMapPipeUtils.h"
#include "../../bufferPoolMgr/BaseBufferHandler.h"
#include "../../bufferPoolMgr/bufferSize/BaseBufferSizeMgr.h"
#include "../../flowOption/DepthMapFlowOption.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using NSIoPipe::QParams;
typedef BaseBufferSizeMgr::P2ABufferSize P2ABufferSize;
/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class YuvQTemplateProvider
 * @brief provide the qparams tempalte for simple yuv generation
 */

class YuvQTemplateProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    YuvQTemplateProvider() = delete;
    YuvQTemplateProvider(
                    sp<DepthMapPipeSetting> pSetting,
                    sp<DepthMapPipeOption> pPipeOption,
                    DepthMapFlowOption* pFlowOption);
    virtual ~YuvQTemplateProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvQTemplateProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init(BaseBufferSizeMgr* pSizeMgr);
    /**
     * @brief build the corresponding QParams in runtime
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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvQTemplateProvider Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    /**
     * @brief prepare the SRZ/FEFM/QParams tuning templates
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareTemplateParams();

    /**
    * @brief QParams template preparation function for each frame
    */
    virtual MBOOL prepareQParam_frame0(
                        MINT32 iModuleTrans,
                        QParams& rQParam);

    virtual MBOOL prepareQParam_frame1(
                        QParams& rQParam);

    /**
    * @brief build QParams template for frame 0~1
    */
    virtual MBOOL buildQParam_frame0(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame1(
                    DepthMapRequestPtr& rEffReqPtr,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    /**
     * @brief query the EIS crop region with remapped dimension
     * @param [in] rEffReqPtr effect request
     * @param [in] szEISDomain EIS domain
     * @param [in] szEISDomain target domain to be remapped
     * @param [out] rOutRegion target domain to be remapped
     */
    MBOOL queryRemappedEISRegion(
                        sp<DepthMapEffectRequest> pRequest,
                        MSize szEISDomain,
                        MSize szRemappedDomain,
                        eis_region& rOutRegion);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvQTemplateProvider protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    BaseBufferSizeMgr* mpSizeMgr = nullptr;
    DepthMapFlowOption* mpFlowOption = nullptr;
    sp<DepthMapPipeOption> mpPipeOption = nullptr;
    // sensor index
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
    // QParams template for normal OpState
    QParams mQParam_NORMAL;
};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
#endif