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
 * @file DepthMapPipeTimer.h
 * @brief Timer for DepthMapPipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTHMAP_PIPE_TIMER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTHMAP_PIPE_TIMER_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <featurePipe/core/include/Timer.h>
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using NSCam::NSCamFeature::NSFeaturePipe::Timer;
class EffectRequestAttrs;
/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class DepthMapPipeTimer
 * @brief timer for DepthMapPipe
 */

class DepthMapPipeTimer : public Timer
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeTimer Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief add all necessary timer
     */
    ADD_TIMER(Overall);
    ADD_TIMER(P2A);
    ADD_TIMER(P2ADrv);
    ADD_TIMER(P2ASetIsp);
    ADD_TIMER(P2ABayer);
    ADD_TIMER(P2ABayerSetIsp);
    ADD_TIMER(P2ABayerDrv);
    ADD_TIMER(P2ABayerMYSResize);
    ADD_TIMER(P2ABayerN3DWaiting);
    ADD_TIMER(N3D);
    ADD_TIMER(N3DMain1Padding);
    ADD_TIMER(N3DMaskWarping);
    ADD_TIMER(N3DLearning);
    ADD_TIMER(DPE);
    ADD_TIMER(DPEDrv);
    ADD_TIMER(GF);
    ADD_TIMER(GFALGO);
    ADD_TIMER(WPE);
    ADD_TIMER(WPEDrv);
    ADD_TIMER(DLDepth);
    ADD_TIMER(DLDepthAlgo);
    /**
     * @brief show summaries
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MVOID showPerFrameSummary(MUINT32 requestNo, const EffectRequestAttrs& atr);
    MVOID showTotalSummary(MUINT32 requestNo, const EffectRequestAttrs& atr);
};



} // namespace NSFeaturePipe_DepthMap
} // namespace NSCamFeature
} // namespace NSCam
#endif
