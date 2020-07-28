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
 * @file DepthMapFactory.cpp
 * @brief factory for flow option/buffer pool mgr
*/

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include "./flowOption/DepthMapFlowOption_VSDOF.h"
#include "./bufferPoolMgr/NodeBufferPoolMgr_VSDOF.h"
// Local header file
#include "DepthMapFactory.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace StereoHAL;
using namespace v1::Stereo;

MBOOL queryDepthMapFlowOption(
    const sp<DepthMapPipeSetting>& pSetting,
    const sp<DepthMapPipeOption>& pPipeOption,
    const sp<DepthInfoStorage>& pStorage,
    sp<DepthMapFlowOption>& rpFlowOption
)
{
    if(pPipeOption->mSensorType == BAYER_AND_BAYER &&
        (pPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF ||
        pPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH ))
    {
        MY_LOGD("create DepthMapFlowOption_VSDOF, mbEnableLCE:%d", pPipeOption->mbEnableLCE);
        rpFlowOption = new DepthMapFlowOption_VSDOF(pSetting, pPipeOption, pStorage);
        return rpFlowOption->init();
    }
    else if(pPipeOption->mSensorType == BAYER_AND_MONO &&
        (pPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF ||
        pPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH ))
    {
        MY_LOGD("create DepthMapFlowOption_VSDOF, mbEnableLCE:%d", pPipeOption->mbEnableLCE);
        rpFlowOption = new DepthMapFlowOption_VSDOF(pSetting, pPipeOption, pStorage);
        return rpFlowOption->init();
    }

    MY_LOGE("Un-spported sensor type=%d and feature mode=%d combination!",
                pPipeOption->mSensorType, pPipeOption->mFeatureMode);

    return MFALSE;
}

MBOOL queryBufferPoolMgr(
    const sp<DepthMapPipeSetting>& pSetting,
    const sp<DepthMapPipeOption>& pPipeOption,
    const sp<DepthMapFlowOption>& pFlowOption,
    sp<BaseBufferPoolMgr>& rPoolMgr)
{
    if(pPipeOption->mSensorType == BAYER_AND_BAYER &&
        (pPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF ||
        pPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH))
    {
        PipeNodeBitSet nodeBitSet;
        if(pFlowOption->queryPipeNodeBitSet(nodeBitSet))
        {
            MY_LOGD("create NodeBufferPoolMgr_VSDOF for B+B, mbEnableLCE:%d", pPipeOption->mbEnableLCE);
            rPoolMgr = new NodeBufferPoolMgr_VSDOF(nodeBitSet, pSetting, pPipeOption);
            return MTRUE;
        }
    }
    else if(pPipeOption->mSensorType == BAYER_AND_MONO &&
            (pPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF ||
            pPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH))
    {
        PipeNodeBitSet nodeBitSet;
        if(pFlowOption->queryPipeNodeBitSet(nodeBitSet))
        {
            MY_LOGD("create NodeBufferPoolMgr_VSDOF for B+M, mbEnableLCE:%d", pPipeOption->mbEnableLCE);
            rPoolMgr = new NodeBufferPoolMgr_VSDOF(nodeBitSet, pSetting, pPipeOption);
            return MTRUE;
        }
    }

    MY_LOGE("Un-spported sensor state=%d and feature mode=%d combination!",
                pPipeOption->mSensorType, pPipeOption->mFeatureMode);

    return MFALSE;
}


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
