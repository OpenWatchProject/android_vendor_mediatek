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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_
//
#include <mtkcam3/pipeline/model/IPipelineModel.h>

#include <impl/IHalDeviceAdapter.h>
#include <impl/IPipelineModelSession.h>
#include <impl/types.h>

#include <chrono>
#include <future>
#include <memory>
#include <mutex>

#include <utils/Printer.h>
//

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
class PipelineModelImpl
    : public IPipelineModel
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Instantiation data (initialized at the creation stage).
    std::shared_ptr<PipelineStaticInfo>     mPipelineStaticInfo;
    std::shared_ptr<android::Printer>       mErrorPrinter;
    std::shared_ptr<android::Printer>       mWarningPrinter;
    std::shared_ptr<android::Printer>       mDebugPrinter;
    //
    int32_t const                           mOpenId = -1;
    int32_t                                 mLogLevel = 0;
    //
    android::sp<IHalDeviceAdapter> const    mHalDeviceAdapter;

protected:  ////    Open data (initialized at the open stage).
    std::string                             mUserName;
    android::wp<IPipelineModelCallback>     mCallback;
    std::vector<std::future<bool>>          mvOpenFutures;

protected:  ////    Configuration data (initialized at the configuration stage).
    android::sp<IPipelineModelSession>      mSession;

protected:  ////    Used to protect Open data and Configuration data.
    std::timed_mutex                        mLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.

                    /**
                     * A structure for creation parameters.
                     */
                    struct  CreationParams
                    {
                        /**
                         * @param logical device id
                         */
                        int32_t                             openId;

                        /**
                         * @param printers with several levels for debugging.
                         */
                        std::shared_ptr<android::Printer>   errorPrinter;
                        std::shared_ptr<android::Printer>   warningPrinter;
                        std::shared_ptr<android::Printer>   debugPrinter;
                    };

    static auto     createInstance(
                        CreationParams const& creationParams
                    )-> android::sp<PipelineModelImpl>;

                    PipelineModelImpl(CreationParams const& creationParams);

protected:
    virtual auto    init() -> bool;
    virtual auto    initPipelineStaticInfo() -> bool;

protected:  ////    Data member access.
    auto            getOpenId() const { return mOpenId; }

protected:  ////    Operations.
    virtual auto    waitUntilOpenDoneLocked() -> bool;

public:     ////    Operations.

    /**
     * Dump debugging state.
     */
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    open(
                        std::string const& userName,
                        android::wp<IPipelineModelCallback> const& callback
                    ) -> int override;
    virtual auto    waitUntilOpenDone() -> bool override;
    virtual auto    close() -> void override;
    virtual auto    configure(
                        std::shared_ptr<UserConfigurationParams>const& params
                    ) -> int override;
    virtual auto    submitRequest(
                        std::vector<std::shared_ptr<UserRequestParams>>const& requests,
                        uint32_t& numRequestProcessed
                    ) -> int override;
    virtual auto    beginFlush() -> int override;
    virtual auto    endFlush() -> void override;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_

