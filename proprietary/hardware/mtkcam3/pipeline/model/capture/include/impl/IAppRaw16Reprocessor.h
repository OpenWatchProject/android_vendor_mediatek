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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_INCLUDE_IMPL_IAPPRAW16REPROCESSOR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_INCLUDE_IMPL_IAPPRAW16REPROCESSOR_H_

#include <memory>
#include <string>
#include <vector>
//
#include <utils/RefBase.h>
//
#include <impl/types.h>
#include <mtkcam3/pipeline/policy/IPipelineSettingPolicy.h>
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


class IAppRaw16Reprocessor : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    using RequestResultParams = NSCam::v3::pipeline::policy::pipelinesetting::RequestResultParams;

public:     ////    Instantiation.

    struct CtorParams
    {
        std::shared_ptr<PipelineStaticInfo>         pPipelineStaticInfo;
        std::shared_ptr<PipelineUserConfiguration>  pUserConfiguration;
    };

    static  auto    makeInstance(
                        std::string const& name,
                        CtorParams const& rCtorParams
                    ) -> android::sp<IAppRaw16Reprocessor>;

    virtual         ~IAppRaw16Reprocessor() = default;

public:     ////    Operations.

    /**
     * Dump the state.
     */
    virtual auto    dumpState(android::Printer& printer) -> void            = 0;

    /**
     * This call will reset all per-request status.
     */
    virtual auto    reset() -> void                                         = 0;

    /**
     * Turn on the flush flag as flush begin and do flush.
     * Turn off the flush flag as flush end.
     */
    virtual auto    beginFlush() -> void                                    = 0;
    virtual auto    endFlush() -> void                                      = 0;

    /**
     * @param[in] isReprocessRequest
     *  true indicates it's a reprocess request.
     *
     * @param[in] pPipelineFrame
     *  The pipeline frame to keep if needed.
     *
     * @param[in] pReqResult
     *  The evaluated result for the request.
     *
     * @param[in] isMainFrame
     *  true indicates it's a main frame.
     *
     * @param[in] pAppMetaControl
     *  The Request App metadata control, sent at the request stage.
     *
     * @param[in] pAppRequest
     *  The App request, sent at the request stage.
     *
     */
    struct KeepFrameIfNeededParams
    {
        bool                                isReprocessRequest = false;
        android::sp<IPipelineFrame>         pPipelineFrame = nullptr;
        RequestResultParams const*          pReqResult = nullptr;
        bool                                isMainFrame = true;
        std::shared_ptr<IMetadata>          pAppMetaControl = nullptr;
        std::shared_ptr<ParsedAppRequest>   pAppRequest = nullptr;
    };
    virtual auto    keepFrameIfNeeded(
                        KeepFrameIfNeededParams const& params
                    ) -> bool                                               = 0;

    /**
     * Is a reprocess frame.
     * true indicates it's a reprocess frame with an App RAW16 input stream buffer.
     *
     * @param[in] isMainFrame
     *  true indicates it's a main frame.
     *
     * @param[in] pAppRequest
     *  The App request, sent at the request stage.
     */
    struct IsReprocessFrameParams
    {
        bool                                isMainFrame = true;
        std::shared_ptr<ParsedAppRequest>   pAppRequest = nullptr;
    };
    virtual auto    isReprocessFrame(
                        IsReprocessFrameParams const& params
                    ) const -> bool                                         = 0;

    /**
     * Handle a reprocess frame.
     *
     * @param[out] pFrameBuilder
     *  The frame builder used to add Hal meta/image stream buffers into.
     *
     * @param[in] pReqResult
     *  The evaluated result for the request.
     *
     * @param[in] pAppMetaControl
     *  The Request App metadata control, sent at the request stage.
     *
     * @param[in] pAppRequest
     *  The App request, sent at the request stage.
     *
     * @param[in] pConfigStreamInfo_P1
     *  The Configuration P1 stream info, built up at the configuration stage.
     *
     */
    struct HandleReprocessFrameParams
    {
        std::shared_ptr<NSPipelineContext::IFrameBuilder>
                                                        pFrameBuilder = nullptr;
        RequestResultParams const*                      pReqResult = nullptr;
        std::shared_ptr<IMetadata>                      pAppMetaControl = nullptr;
        std::shared_ptr<ParsedAppRequest>               pAppRequest = nullptr;
        std::vector<ParsedStreamInfo_P1>const*          pConfigStreamInfo_P1 = nullptr;
    };
    virtual auto    handleReprocessFrame(
                        HandleReprocessFrameParams const& params
                    ) -> void                                               = 0;

    /**
     * Debug the result for a request.
     *
     * @param[in] requestNo: the request number which is being updated.
     */
    struct DebugRequestResultParams
    {
        uint32_t    requestNo = 0;
        android::Vector<android::sp<IMetaStreamBuffer>>const*
                    pvAppOutMeta = nullptr;
    };
    virtual auto    debugRequestResult(
                        DebugRequestResultParams const& params
                    ) const -> void                                         = 0;

    /**
     * Notify that a request is done.
     *
     * @param[in] requestNo: the request number that's done.
     */
    virtual auto    notifyRequestDone(uint32_t requestNo) -> void           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_INCLUDE_IMPL_IAPPRAW16REPROCESSOR_H_

