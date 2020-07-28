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
 * @file NR3DCommon.h
 * @brief NR3DCommon
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_NR3DCOMMON_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_NR3DCOMMON_NODE_H

#include <mtkcam3/feature/3dnr/util_3dnr.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam3/feature/include/common/3dnr/3dnr_hal_base.h>
#include "../DepthMapPipe_Common.h"
#include "../DepthMapEffectRequest.h"
#include "../flowOption/P2AFlowOption.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define DEPTH_3DNR_USER "DepthPipie"
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class NR3DCommon
 * @brief NR3D common base class
 */

class NR3DCommon
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    NR3DCommon() {};
    virtual ~NR3DCommon() {}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NR3DCommon public functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init function
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL init(MUINT32 miSensorIdx);
    MBOOL uninit();
    /**
     * @brief setup 3dnr metadata 3dnr
     * @param [in] rEffReqPtr request
     * @param [in] tuningResult tuning data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL setup3DNRMeta(
                DepthMapPipeNodeID nodeID,
                DepthMapRequestPtr& rEffReqPtr);

    /**
     * @brief perform 3dnr hal to generate tuning data
     * @param [in] pRequest request
     * @param [in] p3AHAL 3A HAL
     * @param [in] pFlowOption flow option
     * @param [out] tuningResult tuning data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL perform3dnr(
                    DepthMapRequestPtr& pRequest,
                    IHal3A* p3AHAL,
                    sp<DepthMapFlowOption> pFlowOption,
                    Stereo3ATuningRes& tuningResult);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NR3DCommon protected member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    //
    MBOOL mDebugEnable = MFALSE;
    NSCam::NSIoPipe::NSPostProc::Hal3dnrBase* mp3dnrHal = nullptr;
    NR3DHALResult mNr3dResult;
    MUINT32 miSensorIdx = -1;
};

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif
