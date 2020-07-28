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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IFRAMEBUFFERMANAGER_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IFRAMEBUFFERMANAGER_H_
//
#include <memory>
#include <optional>
#include <string>
//
#include <utils/Printer.h>
//
#include "types.h"
#include "IPipelineBufferSetFrameControl.h"
#include <mtkcam3/pipeline/stream/IStreamBufferProvider.h>


namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class IHistoryBufferProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definition.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual         ~IHistoryBufferProvider() = default;

public:     ////    Debugging
    virtual auto    dumpState(
                        android::Printer& printer
                    ) -> void                                                   = 0;

    virtual auto    dumpBufferState(
                        android::Printer& printer
                    ) -> void                                                   = 0;

public:     ////    (Frame) Operation
    virtual auto    enableFrameResultRecyclable(
                        uint32_t frameNo,
                        bool forceToRecycle = true
                    ) -> void                                                   = 0;


public:  ////       Select
    struct HistoryBufferSet
    {
        uint32_t                        frameNo = 0;
        uint32_t                        requestNo = 0;
        int64_t                         sensorTimestamp = 0;
        std::vector<android::sp<IImageStreamBuffer>>
                                        vpHalImageStreamBuffers;
        std::vector<android::sp<IMetaStreamBuffer>>
                                        vpHalMetaStreamBuffers;
        std::vector<android::sp<IMetaStreamBuffer>>
                                        vpAppMetaStreamBuffers;
    };

    virtual auto    beginSelect() -> std::optional<std::list<HistoryBufferSet>> = 0;

    virtual auto    endSelect() -> void                                         = 0;

                    struct ReturnUnselectedSet
                    {
                        HistoryBufferSet&&                  hbs;
                        bool                                keep = true;
                    };
    virtual auto    returnUnselectedSet(
                        ReturnUnselectedSet&& arg
                    ) -> void                                                   = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
class IFrameBufferManager
    : public IHistoryBufferProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definition.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    using IImageStreamBufferProviderT = std::shared_ptr<IImageStreamBufferProvider>;
    using Attribute = StreamAttribute;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual auto    getPipelineFrameBufferProvider() const
                        -> std::shared_ptr<IPipelineFrameStreamBufferProvider>  = 0;
    virtual auto    getPipelineFrameResultProcessor() const
                        -> std::shared_ptr<IPipelineFrameHalResultProcessor>    = 0;

public:     ////    Configuration
    virtual auto    beginConfigure() -> void                                    = 0;
    virtual auto    endConfigure() -> int                                       = 0;

                    struct AddImageStream
                    {
                        android::sp<IImageStreamInfo>       pStreamInfo = nullptr;
                        Attribute                           attribute;
                        IImageStreamBufferProviderT         pProvider = nullptr;
                    };
    virtual auto    addImageStream(AddImageStream const& arg) -> int            = 0;

                    struct AddMetaStream
                    {
                        android::sp<IMetaStreamInfo>        pStreamInfo = nullptr;
                        Attribute                           attribute;
                    };
    virtual auto    addMetaStream(AddMetaStream const& arg) -> int              = 0;

public:     ////    (Frame) Operation
                    struct TrackFrameResult
                    {
                        uint32_t                            frameNo = -1;
                        uint32_t                            requestNo = -1;
                        std::shared_ptr<TrackFrameResultParams const>
                                                            trackParams;
                    };
    virtual auto    trackFrameResult(TrackFrameResult const& arg) -> int        = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext


/******************************************************************************
 *
 ******************************************************************************/
extern "C" void createFrameBufferManager(
    std::shared_ptr<NSCam::v3::pipeline::NSPipelineContext::IFrameBufferManager>* out,
    char const* name
);


#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IFRAMEBUFFERMANAGER_H_

