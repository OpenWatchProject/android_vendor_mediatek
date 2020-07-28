/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICONFIGSTREAMINFOPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICONFIGSTREAMINFOPOLICY_H_
//
#include "types.h"

#include <functional>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/**
 * The function type definition.
 * Decide Pass1-specific stream info configuration at the configuration stage.
 *
 * @param[out] pvOut: Pass1-specific stream info
 *
 * @param[in] pvP1HwSetting: P1 hardware settings.
 *
 * @param[in] pvP1DmaNeed: P1 DMA need
 * The value shows which dma are needed.
 * For example,
 *      ((*pvP1DmaNeed)[0] & P1_IMGO) != 0 indicates that IMGO is needed for sensorId[0].
 *      ((*pvP1DmaNeed)[1] & P1_RRZO) != 0 indicates that RRZO is needed for sensorId[1].
 *
 * @param[in] PipelineNodesNeed: which pipeline nodes are needed.
 *
 * @param[in] pStreamingFeatureSetting: the streaming feature settings.
 *
 * @param[in] pCaptureFeatureSetting: capture feature setting
 *
 * @param[in] PipelineStaticInfo: Pipeline static information.
 *
 * @param[in] PipelineUserConfiguration: Pipeline user configuration.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
struct Configuration_StreamInfo_P1_Params
{
    std::vector<ParsedStreamInfo_P1>*   pvOut = nullptr;
    std::vector<P1HwSetting> const*     pvP1HwSetting = nullptr;
    std::vector<uint32_t> const*        pvP1DmaNeed = nullptr;
    PipelineNodesNeed const*            pPipelineNodesNeed = nullptr;
    StreamingFeatureSetting const*      pStreamingFeatureSetting = nullptr;
    CaptureFeatureSetting const*        pCaptureFeatureSetting = nullptr;
    PipelineStaticInfo const*           pPipelineStaticInfo = nullptr;
    PipelineUserConfiguration const*    pPipelineUserConfiguration = nullptr;
};
using FunctionType_Configuration_StreamInfo_P1
    = std::function<int(Configuration_StreamInfo_P1_Params const& /*params*/)>;


/**
 * The function type definition.
 * Decide non-Pass1-specific stream info configuration at the configuration stage.
 *
 * @param[out] pOut: non-Pass1-specific stream info
 *
 * @param[in] PipelineNodesNeed: which pipeline nodes are needed.
 *
 * @param[in] pStreamingFeatureSetting: the streaming feature settings.
 *
 * @param[in] pCaptureFeatureSetting: capture feature setting
 *
 * @param[in] PipelineStaticInfo: Pipeline static information.
 *
 * @param[in] PipelineUserConfiguration: Pipeline user configuration.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
struct Configuration_StreamInfo_NonP1_Params
{
    ParsedStreamInfo_NonP1*             pOut = nullptr;
    PipelineNodesNeed const*            pPipelineNodesNeed = nullptr;
    StreamingFeatureSetting const*      pStreamingFeatureSetting = nullptr;
    CaptureFeatureSetting const*        pCaptureFeatureSetting = nullptr;
    PipelineStaticInfo const*           pPipelineStaticInfo = nullptr;
    PipelineUserConfiguration const*    pPipelineUserConfiguration = nullptr;
};
using FunctionType_Configuration_StreamInfo_NonP1
    = std::function<int(Configuration_StreamInfo_NonP1_Params const& /*params*/)>;


//==============================================================================


/**
 * Policy instance makers
 *
 */

// default version
FunctionType_Configuration_StreamInfo_P1 makePolicy_Configuration_StreamInfo_P1_Default();

// SMVR version
FunctionType_Configuration_StreamInfo_P1 makePolicy_Configuration_StreamInfo_P1_SMVR();

// SMVRBatch version
FunctionType_Configuration_StreamInfo_P1 makePolicy_Configuration_StreamInfo_P1_SMVRBatch();

// default version
FunctionType_Configuration_StreamInfo_NonP1 makePolicy_Configuration_StreamInfo_NonP1_Default();

// Hal Jpeg version - decorator design pattern
FunctionType_Configuration_StreamInfo_NonP1 makePolicy_Configuration_StreamInfo_NonP1_HalJpeg(
        FunctionType_Configuration_StreamInfo_NonP1 f);
/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICONFIGSTREAMINFOPOLICY_H_

