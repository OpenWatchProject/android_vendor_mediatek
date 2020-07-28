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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_
//
#include "MyUtils.h"
//
#include <mtkcam/middleware/v3/pipeline/IPipelineModelMgr.h>
#include "PipelineModelFactory.h"
//
#include <future>
#include <vector>
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


class PipelineModelMgr
    : public IPipelineModelMgr
{

public:  ////                            Operations.
                                    PipelineModelMgr(
                                        android::wp<IPipelineModelMgr::IAppCallback> const& pAppCallback
                                    );
    virtual                         ~PipelineModelMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    android::wp<IPipelineModelMgr::IAppCallback>const
                                    mpAppCallback;

    ConfigurePipeline::Params       mConfigParams;

    mutable android::Mutex          mPipelineLock;
    android::Vector<
        android::sp<IPipelineModel>
        >                           mvPipelineDestroying;
    MINT                            mCurrentOperationMode;
    android::sp<IPipelineModel>     mpCurrentPipeline;

    std::vector< std::future<MERROR> >
                                    mvFutures;

public:
    virtual MERROR                  configurePipeline(
                                        ConfigureParam const& rParams
                                    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual char const*             getName() const;

    virtual MINT32                  getOpenId() const;

public:     ////                    Operations.
    /**
     * Submit a request.
     *
     * @param[in] request: App request to submit.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  submitRequest(IPipelineModelMgr::AppRequest& request);

    /**
     * turn on flush flag as flush begin and do flush
     *
     */
    virtual MERROR                  beginFlush();

    /**
     * turn off flush flag as flush end
     *
     */
    virtual MVOID                   endFlush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //MVOID                           dump();

    //MVOID                           dumpConfigure();

protected:
    android::sp<IPipelineModel>     getPipeline() const {
                                        android::Mutex::Autolock _l(mPipelineLock);
                                        return mpCurrentPipeline;
                                    }
    //
    //MINT                            evaluatePipelineModel(IPipelineModelMgr::AppRequest& request);
    MINT                            evalPipelineScene(MINT const op_mode) const;
    MBOOL                           isReconfigPipeline(
                                        ConfigurePipeline::Params const& curConfig,
                                        IPipelineModelMgr::AppRequest const& request,
                                        MINT& pipelineScene //new pipeline scene
                                    );
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_

