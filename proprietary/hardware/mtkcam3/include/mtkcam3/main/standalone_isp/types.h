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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_MODEL_TYPES_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_MODEL_TYPES_H_
//
#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
//
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
//
#include <utils/StrongPointer.h>
#include <utils/KeyedVector.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace isppipeline {
namespace model {

const static int32_t kISP_TUNING_DATA_SIZE = 16777216; // 16M


/**
 * Pipeline User Configuration Parameters, used by IPipelineModel::configure().
 */
struct UserConfigurationParams
{
    /**
     * @param[in] The operation mode of pipeline.
     *  The caller must promise its value.
     */
    //uint32_t                                    operationMode = 0;

    /**
     * Session wide camera parameters.
     *
     * The session parameters contain the initial values of any request keys that were
     * made available via ANDROID_REQUEST_AVAILABLE_SESSION_KEYS. The Hal implementation
     * can advertise any settings that can potentially introduce unexpected delays when
     * their value changes during active process requests. Typical examples are
     * parameters that trigger time-consuming HW re-configurations or internal camera
     * pipeline updates. The field is optional, clients can choose to ignore it and avoid
     * including any initial settings. If parameters are present, then hal must examine
     * their values and configure the internal camera pipeline accordingly.
     */
    IMetadata                                   sessionParams;

    /**
     * @param[in] App image streams to configure.
     *  The caller must promise the number of buffers and each content.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>>
                                                vImageStreams;

    /**
     * @param[in] App meta streams to configure.
     *  The caller must promise the number of buffers and each content.
     */
    std::unordered_map<StreamId_T, android::sp<IMetaStreamInfo>>
                                                vMetaStreams;

    /**
     * @param[in] App image streams min frame duration to configure.
     *  The caller must promise its initial value.
     */
    //std::unordered_map<StreamId_T, int64_t>     vMinFrameDuration;

    /**
     * @param[in] App image streams stall frame duration to configure.
     *  The caller must promise its initial value.
     */
    //std::unordered_map<StreamId_T, int64_t>     vStallFrameDuration;

    /**
     * @param[in] physical camera id list
     */
    //std::vector<int32_t>                        vPhysicCameras;
};


/**
 * Pipeline User Request Params, used by IPipelineModel::submitRequest().
 */
struct UserRequestParams
{
    /**
     * @param[in] request number.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    uint32_t                                    requestNo = 0;

    /**
     * @param[in,out] input App image stream buffers, if any.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamBuffer>>
                                                vIImageBuffers;

    /**
     * @param[in,out] output App image stream buffers.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamBuffer>>
                                                vOImageBuffers;

    /**
     * @param[in,out] input App meta stream buffers.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    std::unordered_map<StreamId_T, android::sp<IMetaStreamBuffer>>
                                                vIMetaBuffers;

    std::shared_ptr<std::vector<MUINT8>>        pFDData = nullptr;

    int64_t                                     FDDataTimestamp = 0;

};


/**
 * Used by IPipelineModelCallback::onFrameUpdated().
 */
struct UserOnFrameUpdated
{
    /**
     * @param[in] request number.
     */
    uint32_t                                    requestNo = 0;

    /**
     * @param[in] user id.
     *  In fact, it is pipeline node id from the viewpoint of pipeline implementation,
     *  but the pipeline users have no such knowledge.
     */
    intptr_t                                    userId = 0;

    /**
     * @param[in] how many output metadata are not finished.
     */
    ssize_t                                     nOutMetaLeft = 0;

    /**
     * @param[in] (partial) output metadata.
     */
    std::vector<android::sp<IMetaStreamBuffer>> vOutMeta;

    /**
     * @param[in] the timestamp of the start of frame.
     */
    int64_t                                     timestampStartOfFrame = 0;

   /**
     * @param[in] (partial) physical output metadata.
     */
    android::KeyedVector<int, android::sp<IMetaStreamBuffer>> vPhysicalOutMeta;

    bool                                         bFrameEnd = false;

    bool                                         bisEarlyCB = false;

    const IMetadata*                             pMetadata = nullptr;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_MODEL_TYPES_H_

