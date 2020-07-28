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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IREQUESTMETADATAPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IREQUESTMETADATAPOLICY_H_
//
#include "types.h"
//
#include <memory>
#include <vector>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace requestmetadata {


/**
 * Used on the input of evaluateRequest().
 */
struct EvaluateRequestParams
{
    /**
     * @param[in]
     * Request number, sent at the request stage.
     *
     */
    uint32_t                                    requestNo = 0;

    /**
     * @param[in]
     * Request App image stream info, sent at the request stage.
     *
     */
    ParsedAppImageStreamInfo const*             pRequest_AppImageStreamInfo = nullptr;

    /**
     * @param[in]
     * Request App metadata control, sent at the request stage.
     *
     */
    ParsedMetaControl const*                    pRequest_ParsedAppMetaControl = nullptr;

    /**
     * @param[in]
     * The current sensor setting
     *
     * pSensorSize is the pointer to an array of sensor sizes.
     * The array size is the same to the size of sensor id (i.e. PipelineStaticInfo::sensorId).
     */
    std::vector<MSize> const*                   pSensorSize = nullptr;

    std::vector<int32_t> const*                 pSensorId = nullptr;

    /**
     * @param[in]
     * Force to enable EXIF metadata or not.
     *
     * Force to enable EXIF metadata if true; otherwise, it is decided by the policy.
     */
    bool                                        needExif = false;

    /**
     * @param[in/out] Additional metadata
     *
     * pAdditionalApp: app control metadata
     * pAdditionalHal: hal control metadata; the vector size is the same to the
     *                 size of sensor id (i.e. PipelineStaticInfo::sensorId).
     *
     * Callers must ensure they are valid instances (non nullptr).
     * Callee will append additional metadata to them if needed.
     */
    std::shared_ptr<IMetadata>                      pAdditionalApp = nullptr;
    std::vector<std::shared_ptr<IMetadata>>const*   pAdditionalHal = nullptr;

    bool                                            isZSLMode = false;
    /**
     * @param[in]
     * RRZO buffer size. It is decided in configure stage
     */
    std::vector<MSize>                              RrzoSize;
    /**
     * @param[in]
     * the app control metadata from app request
     */
    IMetadata const*                                pRequest_AppControl = nullptr;
    /**
     * @param[in]
     * It is reconfigure request or not.
     */
    bool                                            needReconfigure = false;

    MSize                                           fixedRRZOSize;
    MSize                                           FDYuvSize;

    // For raw output, it needs P1 stream information.
    std::vector<AdditionalRawInfo> const*           additionalRawInfo = nullptr;
};


/**
 *
 */
class IRequestMetadataPolicy
{
public:
    virtual         ~IRequestMetadataPolicy() = default;

    /**
     * The policy is in charge of reporting its requirement at the request stage.
     *
     * @param[in/out] params:
     *  Callers must ensure its content.
     *
     * @return
     *      true indicates success; otherwise failure.
     */
    virtual auto    evaluateRequest(
                        EvaluateRequestParams const& params
                    ) -> int                                                = 0;

};


/**
 * A structure for creation parameters.
 */
struct  CreationParams
{
    std::shared_ptr<PipelineStaticInfo const>       pPipelineStaticInfo;

    std::shared_ptr<PipelineUserConfiguration const>pPipelineUserConfiguration;

    std::shared_ptr<IRequestMetadataPolicy>         pRequestMetadataPolicy;

};


/**
 * Make a policy - default version
 */
std::shared_ptr<IRequestMetadataPolicy> makePolicy_RequestMetadata_Default(CreationParams const& params);

/**
 * Make a policy - debug dump
 */
std::shared_ptr<IRequestMetadataPolicy> makePolicy_RequestMetadata_DebugDump(CreationParams const& params);

// App Raw16 reprocessing version
std::shared_ptr<IRequestMetadataPolicy> makePolicy_RequestMetadata_AppRaw16Reprocess(CreationParams const& params);

// SMVRBatch
std::shared_ptr<IRequestMetadataPolicy> makePolicy_RequestMetadata_SMVRBatch(CreationParams const& params);

/**
* Make a policy - HalJpeg version
*/
std::shared_ptr<IRequestMetadataPolicy> makePolicy_RequestMetadata_HalJpeg(CreationParams const& params);

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IREQUESTMETADATAPOLICY_H_

