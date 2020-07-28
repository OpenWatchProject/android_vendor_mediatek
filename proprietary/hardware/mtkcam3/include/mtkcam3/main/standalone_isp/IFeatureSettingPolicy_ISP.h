/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_ISP_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_FEATURESETTINGPOLICY_ISP_H_
//
//// MTKCAM
#include <mtkcam/def/common.h>
//
#include <map>
#include <memory>
#include <vector>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace featuresetting_isp {

/**
 * Used on the output of evaluateRequest().
 *
 * Request-stage policy
 *  $ update frame setting for isp reprocess
 */

/**
 * Used on the input of evaluateRequest().
 */
struct RequestParams
{
    /**************************************************************************
     * Request parameters
     *
     * The parameters related to this reprocess request is shown as below.
     *
     **************************************************************************/

    // Request number, sent at the request stage.
    uint32_t                           requestNo = 0;

    // Request's input frame count(N) from App, and the input frame index(0 ~ N-1)
    int8_t                             frameCount = -1;
    int8_t                             frameIndex = -1;

    // Input YUV(ex:NV21) format stream for reprocessing.
    android::sp<IImageStreamInfo>      pAppImage_Input_Yuv = nullptr;

    // Input RAW16(ex:10bit, 12bit, ...) format stream for reprocessing.
    android::sp<IImageStreamInfo>      pAppImage_Input_RAW16 = nullptr;

    // Input private RAW format stream for reprocessing
    android::sp<IImageStreamInfo>      pAppImage_Input_Priv = nullptr;

    // Output YUV format stream for reprocessing.
    android::sp<IImageStreamInfo>      pAppImage_Output_Yuv = nullptr;

    // Output RAW16 format stream for reprocessing.
    android::sp<IImageStreamInfo>      pAppImage_Output_RAW16 = nullptr;

    // Output Jpeg format stream for reprocessing.
    android::sp<IImageStreamInfo>      pAppImage_Output_Jpeg = nullptr;

    // Request App control metadata, and support multiframe's App & Hal result metadata.
    // (TODO: openId & sensorId are in per-frame halmeta for algo query nvram tuning)
    IMetadata*                         pRequest_AppControl = nullptr;
    IMetadata*                         pRequest_AppResult  = nullptr;
    IMetadata*                         pRequest_HalResult  = nullptr; /*Hal Control & Hal Result are the same metadata*/
};

struct ConfigurationInputParams
{
    int32_t                            pipelineUniqueKey = 0;

};

/**
 * isp capture feature settings
 */
struct IspCaptureFeatureSetting
{
    // hint support feature for dedicated scenario for P2 node init
    uint64_t                                    supportedScenarioFeatures = 0; /*eFeatureIndexMtk and eFeatureIndexCustomer*/

    uint32_t                                    pluginUniqueKey           = 0;
};

/**
 * Used on the output of evaluateConfiguration().
 *
 * At the configuration stage, capture feature and streaming feature policy will
 * output their both requirements.
 */
struct ConfigurationOutputParams
{
    // TODO: use the struct for expandability.
    IspCaptureFeatureSetting   ispCaptureParams;  // isp config setting for P2C
};

/**
 * Used on the input of queryRecommendedSetting().
 */
struct RecommendedParams
{
    /**************************************************************************
     * Recommended parameters
     *
     * The parameters related to query recommended feature's setting for ISP HIDL user is shown as below.
     *
     **************************************************************************/
    // use latest app result metadata as input to query feature's suggested perframe setting.
    // input:
    IMetadata const*                         pRequest_AppMetadata = nullptr;
    // output:
    std::vector<std::shared_ptr<IMetadata>>  framesSetting;
};


/**
 *
 */
class IFeatureSettingPolicy_ISP
{
public:
    virtual         ~IFeatureSettingPolicy_ISP() = default;

    /**
    * The policy is in charge of reporting its requirement at the configuration stage.
    *
    * @param[in] in:
    *  Callers must promise its content. The callee is not allowed to modify it.
    *
    * @param[out] out:
    *  On this call, the callee must allocate and set up its content.
    *
    * @return
    *      true indicates success; otherwise failure.
    */
    virtual auto    evaluateConfiguration(
                        ConfigurationOutputParams* out,
                        ConfigurationInputParams const* in
                    ) -> int                                                = 0;

    /**
     * The policy is in charge of reporting its requirement at the request stage.
     *
     * @param[in/out] requestParms:
     *  Callers must ensure it's a non-nullptr.
     *  Callers must promise its content. The callee is allowed to modify it.
     *
     * @return
     *      true indicates success; otherwise failure.
     */
    virtual auto    evaluateRequest(
                        RequestParams* requestParms
                    ) -> int                                                = 0;
    /**
     * The policy is in charge of reporting its recommended feature setting to invoke dedicate feature for HIDL user.
     *
     * @param[in/out] requestParms:
     *  Callers must ensure it's a non-nullptr.
     *  Callers must promise its content. The callee is allowed to modify it.
     *
     * @return
     *      true indicates success; otherwise failure.
     */
    virtual auto    queryRecommendedSetting(
                        RecommendedParams* recommendedParms
                    ) -> int                                                = 0;

};

/**
 * A structure for creation parameters.
 */
struct  CreationParams
{
    /**
     * Session parameters.
     *
     * The session parameters contain the initial values of any request keys.
     * The Hal implementation can advertise any settings that can potentially introduce
     * unexpected delays when their value changes during active process requests.
     * Typical examples are parameters that trigger time-consuming HW re-configurations
     * or internal isp pipeline updates.
     * The field is optional, clients can choose to ignore it and avoid including
     * any initial settings. If parameters are present, then hal must examine their
     * values and configure the internal isp pipeline accordingly.
     */
    IMetadata const*                   sessionParams = nullptr;

    // TODO: use the struct for expandability.
    // TODO: remove sensor info if no need(those info must per-frame attached)
    /**
     *  Logical device open id
     */
    int32_t                                     openId = -1;

    /**
     *  Physical sensor id (0, 1, 2)
     */
    std::vector<int32_t>                        sensorId;

};

auto createFeatureSettingPolicyInstance_ISP(
    CreationParams const& params
) -> std::shared_ptr<IFeatureSettingPolicy_ISP>;



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IFEATURESETTINGPOLICY_ISP_H_

