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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASE_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASE_H_
//
#include <impl/IPipelineModelSession.h>

#include <string>

#include <mtkcam3/pipeline/policy/IPipelineSettingPolicy.h>
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include <mtkcam3/pipeline/pipeline/ISessionLifeHolder.h>

#include <mtkcam/utils/std/ULog.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineModelSessionBase
    : public IPipelineModelSession
    , public IPipelineBufferSetFrameControl::IAppCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    using IPipelineContext = NSCam::v3::pipeline::NSPipelineContext::IPipelineContext;
    using IPipelineContextT = NSCam::v3::pipeline::NSPipelineContext::IPipelineContext;
    using HalMetaStreamBuffer = NSCam::v3::Utils::HalMetaStreamBuffer;

    struct  StaticInfo
    {
        std::shared_ptr<PipelineStaticInfo> pPipelineStaticInfo;
        std::shared_ptr<PipelineUserConfiguration>
                                            pUserConfiguration;
        std::shared_ptr<PipelineUserConfiguration2>
                                            pUserConfiguration2;
    };

    struct  DebugInfo
    {
        std::shared_ptr<android::Printer>   pErrorPrinter;
        std::shared_ptr<android::Printer>   pWarningPrinter;
        std::shared_ptr<android::Printer>   pDebugPrinter;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Read-only data
    std::string     mSessionName;
    StaticInfo      mStaticInfo;

protected:  ////    Operatable data (allowed to use their operations)
    DebugInfo       mDebugInfo;
    android::wp<IPipelineModelCallback>     mPipelineModelCallback;

    using IPipelineSettingPolicy = NSCam::v3::pipeline::policy::pipelinesetting::IPipelineSettingPolicy;
    std::shared_ptr<IPipelineSettingPolicy> mPipelineSettingPolicy;

    std::shared_ptr<ISessionLifeHolder>     mpSessionLifeHolder;

protected:  ////    Writable data

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data member access.
    auto            getSessionName() const -> std::string const& { return mSessionName; }

protected:  ////    Template Methods.

    virtual auto    getCurrentPipelineContext() const -> android::sp<IPipelineContextT> = 0;

    virtual auto    submitOneRequest(
                        std::shared_ptr<ParsedAppRequest>const& request
                    ) -> int                                                = 0;

protected:  ////    Operations.

                    /**
                     * Determine the timestamp of the start of frame (SOF).
                     *
                     * @param[in] streamId.
                     *      The SOF timestamp is contained in the meta steram buffer
                     *      whose stream id equal to the given one.
                     *      Usually, it is the stream id of P1Node output HAL meta stream buffer.
                     *
                     * @param[in] vMetaStreamBuffer: a vector of meta stream buffer,
                     *      one of which may have the stream id equal to the given streamId.
                     *
                     * @return
                     *      Nonzero indicates the SOF timestamp.
                     *      0 indicates that the SOF timestamp is not found.
                     */
    static  auto    determineTimestampSOF(
                        StreamId_T const streamId,
                        android::Vector<android::sp<IMetaStreamBuffer>>const& vMetaStreamBuffer
                    ) -> int64_t;

    virtual auto    updateFrameTimestamp(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result,
                        int64_t timestampStartOfFrame
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Derived Class).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    struct CtorParams
                    {
                        StaticInfo                              staticInfo;
                        DebugInfo                               debugInfo;
                        android::wp<IPipelineModelCallback>     pPipelineModelCallback;
                        std::shared_ptr<IPipelineSettingPolicy> pPipelineSettingPolicy;
                    };
                    PipelineModelSessionBase(
                        std::string const&& sessionName,
                        CtorParams const& rCtorParams
                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

    virtual auto    onImageBufferReleased(
                      ImageBufferReleased&& arg
                    ) -> void override;

    virtual auto    onPipelineFrameDestroy(
                      PipelineFrameDestroy&& arg
                    ) -> void override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    submitRequest(
                        std::vector<std::shared_ptr<UserRequestParams>>const& requests,
                        uint32_t& numRequestProcessed
                        ) -> int override;

    virtual auto    beginFlush() -> int override;

    virtual auto    endFlush() -> void override;

    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void override;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASE_H_

