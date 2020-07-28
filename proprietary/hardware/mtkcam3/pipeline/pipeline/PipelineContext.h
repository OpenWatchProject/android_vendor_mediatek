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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_PIPELINECONTEXT_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_PIPELINECONTEXT_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
//
#include <inttypes.h>
//
#include <string>
#include <vector>
//
#include <utils/Printer.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/String8.h>
#include <utils/Vector.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineContext
    : public IPipelineContext
{
    friend class StreamBuilder;
    friend class NodeBuilder;
    friend class PipelineBuilder;
    friend class RequestBuilderImpl;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    class PipelineContextImpl;
    android::sp<PipelineContextImpl>        mpImpl;

private:    ////    Configuration
    android::sp<PipelineContext>            mpOldContext;
    std::shared_ptr<ISessionLifeHolder>     mpSessionLifeHolder;

private:    ////    multi-cam
    android::sp<MultiCamSyncHelperT>        mpSyncHelper = nullptr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Attributes
    auto            getImpl() const -> PipelineContextImpl*;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual void    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                    ~PipelineContext();
                    PipelineContext(char const* name);

public:     ////
    static  auto    magicName() -> std::string;
    static  auto    cast(IPipelineContext* pContext) -> PipelineContext*;

public:     ////    Attributes
    auto            getName() const -> char const*;

public:  ////       Debugging
    auto            dump() -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineContext Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Debugging
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void override;

public:     ////    Configuration

    virtual auto    setDataCallback(
                        android::wp<IDataCallback> pCallback
                    ) -> int override;

    virtual auto    setInitFrameCount(uint32_t count) -> void override;

    virtual auto    beginConfigure(
                        android::sp<IPipelineContext> oldContext = nullptr
                    ) -> int override;
    virtual auto    endConfigure(
                        MBOOL asyncConfig = MFALSE,
                        MBOOL isMultiThread = MFALSE
                    ) -> int override;

    virtual auto    getMultiCamSyncHelper() -> android::sp<MultiCamSyncHelperT> override;
    virtual auto    setMultiCamSyncHelper(
                        android::sp<MultiCamSyncHelperT>& helper
                    ) -> void override;

    virtual auto    getSessionLifeHolder() const -> std::shared_ptr<ISessionLifeHolder> override;
    virtual auto    setSessionLifeHolder(
                        std::shared_ptr<ISessionLifeHolder> const& mgr) -> void override;

public:     ////    Attributes
    virtual auto    getNativeName() const -> std::string override { return magicName(); }
    virtual auto    getNativeHandle() -> void* override { return this; }

    virtual auto    queryINodeActor(
                        NodeId_T const nodeId
                    ) const -> android::sp<INodeActor> override;

public:     ////    Operations
    virtual auto    queue(
                        android::sp<IPipelineFrame>const& frame
                    ) -> int override;

    virtual auto    queue(
                        std::vector<android::sp<IPipelineFrame>>const& frames
                    ) -> int override;

    virtual auto    kick(
                        android::sp<IPipelineFrame>const &pFrame
                    ) -> int override;

    virtual auto    beginFlush() -> int override;
    virtual auto    endFlush() -> int override;

    virtual auto    waitUntilDrained() -> int override;
    virtual auto    waitUntilRootNodeDrained() -> int override;
    virtual auto    waitUntilNodeDrained(NodeId_T const nodeId) -> int override;

public:     ////    fbm
    virtual auto    getHistoryBufferProvider() const
                        -> std::shared_ptr<IHistoryBufferProvider> override;
};


/******************************************************************************
*
******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_PIPELINECONTEXT_H_

