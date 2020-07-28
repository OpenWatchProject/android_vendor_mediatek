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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_INCLUDE_IMPL_IPIPELINEMODELSESSION_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_INCLUDE_IMPL_IPIPELINEMODELSESSION_H_

#include <mtkcam3/pipeline/model/IPipelineModel.h>
#include <impl/types.h>
//
#include <memory>
//
#include <utils/Printer.h>


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
class IPipelineModelSession : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Session Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual         ~IPipelineModelSession() = default;

    /**
     * Submit a set of requests.
     *
     * @param[in] requests: a set of requests to submit.
     *
     * @param[out] numRequestProcessed: number of requests successfully submitted.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    submitRequest(
                        std::vector<std::shared_ptr<UserRequestParams>>const& requests,
                        uint32_t& numRequestProcessed
                        ) -> int                                            = 0;

    /**
     * turn on flush flag as flush begin and do flush
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    beginFlush() -> int                                     = 0;

    /**
     * turn off flush flag as flush end
     *
     */
    virtual auto    endFlush() -> void                                      = 0;

    /**
     * Dump debugging state.
     */
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineModelSessionFactory
{
public:     ////

    /**
     * A structure for creation parameters.
     */
    struct  CreationParams
    {
        std::shared_ptr<PipelineStaticInfo>         pPipelineStaticInfo;
        std::shared_ptr<UserConfigurationParams>    pUserConfigurationParams;

        std::shared_ptr<android::Printer>           pErrorPrinter;
        std::shared_ptr<android::Printer>           pWarningPrinter;
        std::shared_ptr<android::Printer>           pDebugPrinter;

        uint64_t                                    uiConfigTimestamp;

        android::sp<IPipelineModelCallback>         pPipelineModelCallback;
    };

    static  auto    createPipelineModelSession(
                        CreationParams const& params
                    ) -> android::sp<IPipelineModelSession>;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_INCLUDE_IMPL_IPIPELINEMODELSESSION_H_

