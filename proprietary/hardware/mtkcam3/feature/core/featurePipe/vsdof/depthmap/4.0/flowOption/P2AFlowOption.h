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
 * @file P2AFlowOption.h
 * @brief Flow option template for P2ANode
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_P2ANODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_P2ANODE_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/aaa/aaa_hal_common.h>
// Module header file
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
// Local header file
#include "../DepthMapEffectRequest.h"
#include "../DepthMapPipeUtils.h"
#include "../DepthMapPipe_Common.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {
using namespace NS3Av3;

/*******************************************************************************
* Enum Define
********************************************************************************/


/*******************************************************************************
* Const Definition
********************************************************************************/

// DMA port
using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_LCEI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_FEO;
using NSCam::NSIoPipe::PORT_MFBO;
using NSCam::NSIoPipe::PORT_PAK2O;

/*******************************************************************************
* Struct Definition
********************************************************************************/
struct AAATuningResult
{
public:
    AAATuningResult(): tuningResult() {}
    AAATuningResult(MVOID* pTuningBuf, MVOID* pLcsBuf = nullptr)
    : tuningResult()
    {
        tuningResult.pRegBuf = pTuningBuf;
        tuningResult.pLcsBuf = pLcsBuf;
        tuningBuffer = pTuningBuf;
    }
public:
    TuningParam tuningResult;
    MVOID*  tuningBuffer = nullptr;
};

struct Stereo3ATuningRes
{
    AAATuningResult tuningRes_main1;
    AAATuningResult tuningRes_main2;
    AAATuningResult tuningRes_FE_main1;
    AAATuningResult tuningRes_FE_main2;
};

/*******************************************************************************
* Class Definition
********************************************************************************/
class P2ANode;
class P2ABayerNode;
/**
 * @class P2AFlowOption
 * @brief Flow option template class for P2A Node
 */

class P2AFlowOption
{
public:
    P2AFlowOption() {}
    virtual ~P2AFlowOption() {}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2AFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief build the enque QParams
     * @param [in] pReq DepthMap EffectRequest
     * @param [in] tuningResult tuning result after applied the 3A setIsp
     * @param [out] rOutParam output QParams
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL buildQParam(
                    sp<DepthMapEffectRequest>,
                    const Stereo3ATuningRes&,
                    QParams& rOutParam) = 0;

     /**
     * @brief callback when P2 hw operations finished
     * @param [in] pNode P2ANode instance pointer
     * @param [in] pReq DepthMap EffectRequest
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL onP2ProcessDone(
                    P2ANode* pNode,
                    sp<DepthMapEffectRequest> pReq) = 0;

    /**
     * @brief build the enque QParams for bayer run (not required for all scenarios)
     * @param [in] pReq DepthMap EffectRequest
     * @param [in] tuningResult tuning result after applied the 3A setIsp
     * @param [out] rOutParam output QParams
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL buildQParam_Bayer(
                    sp<DepthMapEffectRequest>,
                    const Stereo3ATuningRes&,
                    QParams&) {return MTRUE;}
    /**
     * @brief callback when P2 hw operations finished for bayer run
     * @param [in] pNode P2ANode instance pointer
     * @param [in] pReq DepthMap EffectRequest
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL onP2ProcessDone_Bayer(
                    P2ABayerNode*,
                    sp<DepthMapEffectRequest>) {return MTRUE;}

    /**
     * @brief get the input raw type of main1/main2
     * @param [in] pReq DepthMap EffectRequest
     * @param [in] path main1 or main2 path
     * @return
     * - eRESIZE_RAW indicate the input raw type is Resize
     * - eFULL_RAW indicate the input raw type is full
     */
    virtual INPUT_RAW_TYPE getInputRawType(
                    sp<DepthMapEffectRequest> pReq,
                    StereoP2Path path
                    ) = 0;
}; //P2AFlowOption

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
