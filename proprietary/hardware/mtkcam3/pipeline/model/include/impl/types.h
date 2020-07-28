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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_INCLUDE_IMPL_TYPES_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_INCLUDE_IMPL_TYPES_H_

#include <mtkcam3/pipeline/model/types.h>
#include <mtkcam3/pipeline/policy/types.h>
//
#include <memory>
#include <unordered_map>
//
#include <utils/String8.h>
#include <utils/StrongPointer.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 *  Pipeline Static Information.
 *
 ******************************************************************************/
using PipelineStaticInfo
    = NSCam::v3::pipeline::policy::PipelineStaticInfo;


/******************************************************************************
 *
 *  1st Configuration related definitions.
 *
 ******************************************************************************/


/**
 *  Parsed App configuration
 */
using ParsedAppConfiguration
    = NSCam::v3::pipeline::policy::ParsedAppConfiguration;


/**
 *  App image stream info configuration
 */
using ParsedAppImageStreamInfo
    = NSCam::v3::pipeline::policy::ParsedAppImageStreamInfo;


/**
 *  Pipeline user configuration
 */
using PipelineUserConfiguration
    = NSCam::v3::pipeline::policy::PipelineUserConfiguration;


/**
 *  Pipeline user configuration
 */
using ParsedMultiCamInfo
    = NSCam::v3::pipeline::policy::ParsedMultiCamInfo;

/**
 *  SMVRBatch info configuration
 */
using ParsedSMVRBatchInfo
    = NSCam::v3::pipeline::policy::ParsedSMVRBatchInfo;


/**
 *  Pipeline user configuration
 *
 *  The following parameters or interfaces are set up at the configuration stage,
 *  and  are never changed AFTER the configuration stage.
 */
struct PipelineUserConfiguration2
{
    /**
     * The factory of App stream info builder.
     *
     * It is used to create IStreamInfoBuilder instances.
     */
    std::shared_ptr<IStreamInfoBuilderFactory>  pStreamInfoBuilderFactory = nullptr;

    /**
     * The App stream buffer provider.
     *
     * If this field is valid, those output App image stream buffers which are
     * not provided with requests are going to be acquired from this provider.
     */
    std::shared_ptr<IImageStreamBufferProvider> pImageStreamBufferProvider = nullptr;

};


/******************************************************************************
 *
 *  2nd Configuration related definitions.
 *
 ******************************************************************************/


/**
 *  (Pass1-specific) stream info configuration
 */
using ParsedStreamInfo_P1
    = NSCam::v3::pipeline::policy::ParsedStreamInfo_P1;


/**
 *  (Non Pass1-specific) stream info configuration
 */
using ParsedStreamInfo_NonP1
    = NSCam::v3::pipeline::policy::ParsedStreamInfo_NonP1;


/**
 *  Pipeline nodes need.
 *  true indicates its corresponding pipeline node is needed.
 */
using PipelineNodesNeed
    = NSCam::v3::pipeline::policy::PipelineNodesNeed;


/**
 *  Pipeline topology.
 */
using PipelineTopology
    = NSCam::v3::pipeline::policy::PipelineTopology;


/**
 *  Sensor Setting
 */
using SensorSetting
    = NSCam::v3::pipeline::policy::SensorSetting;


/**
 *  Pass1-specific HW settings
 */
using P1HwSetting
    = NSCam::v3::pipeline::policy::P1HwSetting;


/**
 * Streaming feature settings
 */
using StreamingFeatureSetting
    = NSCam::v3::pipeline::policy::StreamingFeatureSetting;

/**
 * Capture feature settings
 */
using CaptureFeatureSetting
    = NSCam::v3::pipeline::policy::CaptureFeatureSetting;


/******************************************************************************
 *
 *  Request related definitions.
 *
 ******************************************************************************/


/**
 *  Parsed App image stream buffers
 */
struct ParsedAppImageStreamBuffers
{
    /**************************************************************************
     *  App image stream buffer set
     **************************************************************************/

    /**
     * Input App image stream buffers, if any.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamBuffer>>
                                                vIImageBuffers;

    /**
     * Output App image stream buffers.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamBuffer>>
                                                vOImageBuffers;

    /**
     * Output stream for physical RAW16/DNG capture.
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, std::vector<android::sp<IImageStreamBuffer>>>
                                                vAppImage_Output_RAW16_Physical;
    /**
     * Output stream for isp tuning data stream (Raw)
     */
    android::sp<IImageStreamBuffer>             pAppImage_Output_IspTuningData_Raw;

    /**
     * Output stream for isp tuning data stream (Yuv)
     */
    android::sp<IImageStreamBuffer>             pAppImage_Output_IspTuningData_Yuv;

    /**
     * Output stream for physical isp meta stream (Raw)
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, android::sp<IImageStreamBuffer>>
                                                vAppImage_Output_IspTuningData_Raw_Physical;

    /**
     * Output stream for physical isp meta stream (Yuv)
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, android::sp<IImageStreamBuffer>>
                                                vAppImage_Output_IspTuningData_Yuv_Physical;
};


/**
 *  Parsed App request
 */
struct ParsedAppRequest
{
    using ParsedMetaControl = NSCam::v3::pipeline::policy::ParsedMetaControl;

    /**
     * Request number.
     */
    uint32_t                                    requestNo = 0;

    /**
     * App metadata control (stream buffer), sent at the request stage.
     *
     * pParsedAppMetaControl is a partial parsed result from pAppMetaControl, just for the purpose
     * of a quick reference.
     */
    android::sp<IMetaStreamBuffer>              pAppMetaControlStreamBuffer = nullptr;
    std::shared_ptr<ParsedMetaControl>          pParsedAppMetaControl = nullptr;

    /**
     * App image stream buffers, sent at the request stage.
     */
    std::shared_ptr<ParsedAppImageStreamBuffers>pParsedAppImageStreamBuffers;

    /**
     * App image stream info, sent at the request stage.
     */
    std::shared_ptr<ParsedAppImageStreamInfo>   pParsedAppImageStreamInfo;

};


/******************************************************************************
 *
 ******************************************************************************/
static inline android::String8 toString(const ParsedAppImageStreamBuffers& o __unused)
{
    android::String8 os;

    if ( auto const& buffers = o.vIImageBuffers; !buffers.empty() ) {
        for (auto const& b : buffers) {
            os += "\n    ";
            os += b.second->toString();
        }
    }

    if ( auto const& buffers = o.vOImageBuffers; !buffers.empty() ) {
        for (auto const& b : buffers) {
            os += "\n    ";
            os += b.second->toString();
        }
    }

    for (auto const& v : o.vAppImage_Output_IspTuningData_Raw_Physical) {
        os.appendFormat("\n    physic cam(%d)", v.first);
        os += "\n    ";
        os += v.second->toString();
    }

    for (auto const& v : o.vAppImage_Output_IspTuningData_Yuv_Physical) {
        os.appendFormat("\n    physic cam(%d)", v.first);
        os += "\n    ";
        os += v.second->toString();
    }

    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_INCLUDE_IMPL_TYPES_H_

