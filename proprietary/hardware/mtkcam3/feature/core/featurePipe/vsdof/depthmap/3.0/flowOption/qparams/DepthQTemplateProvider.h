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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
 * @file DepthQTemplateProvider.h
 * @brief QParams template creator for depth generateion
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_DEPTH_QPARAMS_TEMPLATE_PROVIDER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_DEPTH_QPARAMS_TEMPLATE_PROVIDER_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <DpDataType.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
// Module header file
#include <stereo_tuning_provider.h>
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

#define PQ_FRAME_SIZE 6

using NSIoPipe::QParams;

typedef BaseBufferSizeMgr::P2ABufferSize P2ABufferSize;
/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class DepthQTemplateProvider
 * @brief QParams template creator for depth generateion
 */

class DepthQTemplateProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DepthQTemplateProvider() = delete;
    DepthQTemplateProvider(
                        sp<DepthMapPipeSetting> pSetting,
                        sp<DepthMapPipeOption> pPipeOption,
                        DepthMapFlowOption* pFlowOption);
    virtual ~DepthQTemplateProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthQTemplateProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init(BaseBufferSizeMgr* pSizeMgr);
    /**
     * @brief build the corresponding QParams in runtime of each OpState
     * @param [in] rpRequest DepthMapRequestPtr
     * @param [in] tuningResult 3A tuning Result
     * @param [out] rOutParm QParams to enque into P2
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL buildQParams_NORMAL(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_NORMAL_NOFEFM(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_CAPTURE(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_NORMAL_NOFEFM(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_CAPTURE(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);

    virtual MBOOL buildQParams_BAYER_STANDALONE(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParm);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthQTemplateProvider Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    /**
     * @brief map buffer id to its P2 output port
     * @param [in] buffeID DepthMapNode buffer id
     * @return
     * - DMA output port id
     */
    virtual NSCam::NSIoPipe::PortID mapToPortID(DepthMapBufferID buffeID);

    /**
     * @brief prepare the SRZ/FEFM/QParams tuning templates
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareTemplateParams();
    /**
     * @brief prepare the QParams template for operation state
     * @param [in] state DepthMap Pipe OP state
     * @param [in] iModuleTrans module rotation
     * @param [in] is skip FEFM not not
     * @param [out] rQTmplate QParams template
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL prepareQParamsTemplate(
                        MBOOL bIsSkipFEFM,
                        DepthMapPipeOpState state,
                        MINT32 iModuleTrans,
                        QParams& rQParam);

    virtual MBOOL prepareQParamsTemplate_NORMAL(
                        MBOOL bIsSkipFEFM,
                        MINT32 iModuleTrans,
                        QParams& rQParam);
    virtual MBOOL prepareQParamsTemplate_CAPTURE(
                        MINT32 iModuleTrans,
                        QParams& rQParam);

    /**
     * @brief prepare QParams tuning templates for Bayer run
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareTemplateParams_Bayer();

    /**
     * @brief prepare the QParams template for operation state for Bayer run
     * @param [in] state DepthMap Pipe OP state
     * @param [in] iModuleTrans module rotation
     * @param [out] rQTmplate QParams template
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL prepareQParamsTemplate_BAYER_NORMAL_NOFEFM(
                                    MINT32 iModuleTrans,
                                    QParams& rQParam
                                    );

    virtual MBOOL prepareQParamsTemplate_BAYER_CAPTURE(
                                    MINT32 iModuleTrans,
                                    QParams& rQParam
                                    );
    virtual MBOOL prepareQParamsTemplate_BAYER_STANDALONE(
                                    MINT32 iModuleTrans,
                                    QParams& rQParam
                                    );
    /**
     * @brief QParams template preparation function for each frame
     */
    virtual MBOOL prepareQParam_frame0(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL prepareQParam_frame2(QParams& rQParam);
    virtual MBOOL prepareQParam_frame3(QParams& rQParam);
    virtual MBOOL prepareQParam_frame4(QParams& rQParam);
    virtual MBOOL prepareQParam_frame5(QParams& rQParam);
    virtual MBOOL prepareQParam_frame6_9(QParams& rQParam);
    // frames: 1 are differnt between OpState
    virtual MBOOL prepareQParam_frame1(
                        DepthMapPipeOpState state,
                        MINT32 iModuleTrans,
                        QParams& rQParam);
    /**
     * @brief prepare/build the QParams template for Bayer run
     */

    virtual MBOOL prepareQParam_Bayer_NORMAL_NOFEFM_frame0(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL prepareQParam_Bayer_NORMAL_NOFEFM_frame1(
                                MINT32 iModuleTrans,
                                QParams& rQParam);

    virtual MBOOL prepareQParam_Bayer_CAPTURE_frame0(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    virtual MBOOL prepareQParam_Bayer_CAPTURE_frame1(
                                MINT32 iModuleTrans,
                                QParams& rQParam);
    /**
     * @brief build QParams template for frame 0~9
     */
    virtual MBOOL buildQParam_frame0(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame1(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame2(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame3(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame4(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame5(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame6to9(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    // buildQParam for capture scenario
    virtual MBOOL buildQParam_frame1_capture(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame2_capture(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_frame3_capture(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);

    /**
     * @brief build the QParams template for Bayer run when CAP
     */
    virtual MBOOL buildQParam_Bayer_NORMAL_NOFEFM_frame0(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_Bayer_NORMAL_NOFEFM_frame1(
                    DepthMapRequestPtr& rpRequest,
                    QParamTemplateFiller& rQFiller);

    virtual MBOOL buildQParam_Bayer_CAPTURE_frame0(
                    DepthMapRequestPtr& rpRequest,
                    const Stereo3ATuningRes& tuningResult,
                    QParamTemplateFiller& rQFiller);
    virtual MBOOL buildQParam_Bayer_CAPTURE_frame1(
                    DepthMapRequestPtr& rpRequest,
                    QParamTemplateFiller& rQFiller);

    /**
     * @brief setup the FE/FM tuning buffer
     */
    MVOID setupFMTuningInfo(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameID);

    /**
     * @brief query the EIS crop region with remapped dimension
     * @param [in] rpRequest effect request
     * @param [in] szEISDomain EIS domain
     * @param [in] szEISDomain target domain to be remapped
     * @param [out] rOutRegion target domain to be remapped
     */
    MBOOL queryRemappedEISRegion(
                        sp<DepthMapEffectRequest> pRequest,
                        MSize szEISDomain,
                        MSize szRemappedDomain,
                        eis_region& rOutRegion);

    /**
    * @brief remap ENUM_ROTATION into iTransform for Postview
    */
    MINT32 remapTransformForPostview(ENUM_ROTATION moduleRotation, ENUM_ROTATION jpegOrientation);

    /**
     * @brief setup the VSDOF PQparam default value
     */
    MVOID setupPQParamSetting(DpPqParam* pParam, MUINT iFrameID);
    MBOOL convertFrameIdToPass2Round(MUINT32 frameID, ENUM_PASS2_ROUND &round);
    /**
     * @brief callback function when P2 operation done
     */
    MVOID onHandleP2Done(
                DepthMapPipeNodeID nodeID,
                sp<DepthMapEffectRequest> pRequest);
    /**
     * @brief get the 3dnr VIPI buffer of current run
     */
    IImageBuffer* get3DNRVIPIBuffer();
    /**
     * @brief get EIS crop region
     */
    MBOOL getEISCropRegion(
                    DepthMapPipeNodeID nodeID,
                    sp<DepthMapEffectRequest> pRequest,
                    MPoint& rptCropStart,
                    MSize& rszCropSize
                    );

    /**
     * @brief calc the start-point offset in bytes
     */
    size_t calcStartPointOffset(EImageFormat format, const MPoint& startPt);
    /**
     * @brief LCE part
     */
    inline MVOID _prepareLCETemplate(QParamTemplateGenerator& generator);

    inline MBOOL _fillLCETemplate(
                        MUINT iFrameNum,
                        sp<DepthMapEffectRequest> pRequest,
                        const AAATuningResult& tuningRes,
                        QParamTemplateFiller& rQFiller,
                        _SRZ_SIZE_INFO_& rSrzInfo);

    MBOOL setupSRZ4LCENR(
                    sp<DepthMapEffectRequest> pRequest,
                    _SRZ_SIZE_INFO_& rSizeInfo);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthQTemplateProvider protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    BaseBufferSizeMgr* mpSizeMgr = nullptr;
    DepthMapFlowOption* mpFlowOption = nullptr;
    sp<DepthMapPipeOption> mpPipeOption = nullptr;
    // sensor index
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
    // QParams template for normal/capture OpState
    QParams mQParam_NORMAL;
    QParams mQParam_NORMAL_NOFEFM;
    QParams mQParam_CAPTURE;
    QParams mQParam_BAYER_NORMAL_NOFEFM;
    QParams mQParam_BAYER_CAPTURE;
    QParams mQParam_BAYER_STANDALONE;
    // SRZ CROP/RESIZE template, frameID to SRZInfo map
    KeyedVector<MUINT, _SRZ_SIZE_INFO_> mSrzSizeTemplateMap;
    // FE Tuning Buffer Map, key=stage, value= tuning buffer
    KeyedVector<MUINT, NSCam::NSIoPipe::FEInfo> mFETuningBufferMap;
    // FM Tuning Buffer Map, key=frame ID, value= tuning buffer
    KeyedVector<MUINT, NSCam::NSIoPipe::FMInfo> mFMTuningBufferMap;
    // P2 enque extraParam for PQ, key=frame ID, value= PQParam
    KeyedVector<MUINT, PQParam> mVSDOFPQParamMap;
    PQParam mBayerPQParam;
    //  frame 0~5 DpIspParam
    DpPqParam mDpPqParamMap[PQ_FRAME_SIZE];
    DpPqParam mBayerDpPqParam;
    // SRZ4 config for LCSO
    _SRZ_SIZE_INFO_ mSRZ4Config_LCENR;
    _SRZ_SIZE_INFO_ mSRZ4Config_LCENR_Bayer;

    // Record the frame index of the L/R-Side which is defined by ALGO.
    // L-Side could be main1 or main2 which can be identified by the sensors' locations.
    // Frame 0,3,5 is main1, 1,2,6 is main2.
    int frameIdx_LSide[2];
    int frameIdx_RSide[2];
    //
    SmartFatImageBuffer mp3DNR_IMG3OBuf = nullptr;
};


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
