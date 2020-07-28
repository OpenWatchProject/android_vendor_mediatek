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
 * @file DepthMapFlowOption_BMVSDOF.h
 * @brief DepthMap Flow option template for Bayer+Mono VSDOF
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_DEPTHMAPNODE_BM_VSDOF_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_DEPTHMAPNODE_BM_VSDOF_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
// Module header file
#include <bufferPoolMgr/NodeBufferPoolMgr_VSDOF.h>
#include <bufferPoolMgr/bufferSize/NodeBufferSizeMgr.h>
// Local header file
#include "DepthMapFlowOption.h"
#include "./qparams/BayerMonoQTemplateProvider.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::sp;
using NSIoPipe::QParams;
/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class DepthMapFlowOption_BMVSDOF
 * @brief flow optione class for B+M VSDOF
 */
class DepthMapFlowOption_BMVSDOF
: public DepthMapFlowOption
, public BayerMonoQTemplateProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef BayerMonoQTemplateProvider TemplateProvider;
    DepthMapFlowOption_BMVSDOF(
        sp<DepthMapPipeSetting> pSetting,
        sp<DepthMapPipeOption> pOption,
        sp<DepthInfoStorage> pStorage);
    virtual ~DepthMapFlowOption_BMVSDOF();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2AFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL buildQParam(
                    DepthMapRequestPtr pReq,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParam);

    virtual MBOOL onP2ProcessDone(
                    P2AFMNode* pNode,
                    sp<DepthMapEffectRequest> pReq);

    virtual INPUT_RAW_TYPE getInputRawType(
                    sp<DepthMapEffectRequest> pReq,
                    StereoP2Path path);

    virtual MBOOL buildQParam_Bayer(
                    sp<DepthMapEffectRequest> pReq,
                    const Stereo3ATuningRes& tuningResult,
                    QParams& rOutParam);

    virtual MBOOL onP2ProcessDone_Bayer(
                    P2ABayerNode* pNode,
                    sp<DepthMapEffectRequest> pReq);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL init();

    virtual MBOOL queryReqAttrs(
                        sp<DepthMapEffectRequest> pRequest,
                        EffectRequestAttrs& rReqAttrs);

    virtual MBOOL queryPipeNodeBitSet(PipeNodeBitSet& nodeBitSet);

    virtual MBOOL buildPipeGraph(
                        DepthMapPipe* pPipe,
                        const DepthMapPipeNodeMap& nodeMap);

    virtual MBOOL checkConnected(
                        DepthMapDataID dataID);

    virtual MBOOL config3ATuningMeta(
                        sp<DepthMapEffectRequest> pRequest,
                        StereoP2Path path,
                        MetaSet_T& rMetaSet);

    virtual DepthMapBufferID reMapBufferID(
                        const EffectRequestAttrs& reqAttr,
                        DepthMapBufferID bufferID
                        );

    virtual const FlowOptionConfig& getOptionConfig() {return mConfig;}
    virtual const FlowOptionID getFlowOptionID() {return DEPTHMAP_FLOWOPTION_BMVSDOF;}
    virtual MBOOL needBypassP2AFM(sp<DepthMapEffectRequest> pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_Denoise/VSDOF Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_BMVSDOF Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_BMVSDOF Public Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_BMVSDOF Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    //
    BaseBufferSizeMgr *mpSizeMgr = nullptr;
    // config
    FlowOptionConfig mConfig;
    // valid DepthMapDataID vector
    KeyedVector<DepthMapDataID, MBOOL> mvAllowDataIDMap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_BMVSDOF private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<DepthMapPipeOption> mpPipeOption;

    BayerBayerQTemplateProvider* mpQParamPvdr_BB;
};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif