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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_APPRAW16REPROCESSOR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_APPRAW16REPROCESSOR_H_

#include <impl/IAppRaw16Reprocessor.h>
//
#include <list>
#include <mutex>
#include <set>
//
#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


class AppRaw16Reprocessor : public IAppRaw16Reprocessor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Read-only data
    std::string const       mName;
    CtorParams              mCtorParams;

protected:  ////    private request info (changable)
    std::mutex              mFlushLock;
    std::atomic_bool        mFlushing{false};

    static constexpr uint32_t BAD_REQUEST_NO = (uint32_t)-1;
    uint32_t                mReprocessRequestNo = BAD_REQUEST_NO;
    mutable std::mutex      mReprocessRequestNoLock;

    using HistoryFrameContainerT = std::list<android::sp<IPipelineFrame>>;
    HistoryFrameContainerT  mHistoryFrameContainer;
    std::mutex              mHistoryFrameContainerLock;

    #if 0
    // History JPEG control metadata
    IMetadata::IEntry       mJPEG_GPS_COORDINATES;
    IMetadata::IEntry       mJPEG_GPS_PROCESSING_METHOD;
    IMetadata::IEntry       mJPEG_GPS_TIMESTAMP;
    IMetadata::IEntry       mJPEG_ORIENTATION;
    IMetadata::IEntry       mJPEG_QUALITY;
    IMetadata::IEntry       mJPEG_THUMBNAIL_QUALITY;
    IMetadata::IEntry       mJPEG_THUMBNAIL_SIZE;
    #endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    AppRaw16Reprocessor(
                        std::string const& name,
                        CtorParams const& rCtorParams
                    );

protected:  ////    Operations (Reprocess Request Number).
    auto            setReprocessRequestNo(uint32_t requestNo) -> void;
    auto            resetReprocessRequestNo(uint32_t requestNo) -> void;

protected:  ////    Operations (History Frame Container).
    auto            acquireAllHistoryFrames() -> HistoryFrameContainerT;

protected:  ////    Operations (Keep & Reprocess)

    auto            keepFrameForReprocess(KeepFrameIfNeededParams const& params) -> void;

    auto            prepareReprocessHalBuffers(HandleReprocessFrameParams const& params) -> void;

    auto            selectHistoryFrameForReprocess(
                        android::sp<IPipelineBufferSetFrameControl>& pTargetFrameControl,
                        HistoryFrameContainerT& historyFrames,
                        int64_t requestTimestamp,
                        uint32_t requestNo
                    ) -> void;

    /**
     *
     * @param[out] pFrameBuilder
     *  The frame builder used to add Hal meta/image stream buffers into.
     *
     * @param[in/out] pHistoryFrameControl
     *  After this call, the buffers inside this history frame is transfered.
     *
     * @param[in] pReqResult
     *  The evaluated result for the request.
     *
     * @param[in] pAppRequest
     *  The App request, sent at the request stage.
     *
     * @param[in] pConfigStreamInfo_P1
     *  The Configuration P1 stream info, built up at the configuration stage.
     *
     */
    struct TransferBuffersFromFrameParams
    {
        std::shared_ptr<NSPipelineContext::IFrameBuilder>   pFrameBuilder = nullptr;
        android::sp<IPipelineBufferSetFrameControl>         pHistoryFrameControl = nullptr;
        policy::pipelinesetting::RequestResultParams const* pReqResult = nullptr;
        std::shared_ptr<ParsedAppRequest>                   pAppRequest;
        std::vector<ParsedStreamInfo_P1>const*              pConfigStreamInfo_P1 = nullptr;
    };
    auto            transferBuffersFromFrame(TransferBuffersFromFrameParams const& params) -> void;

    auto            checkJpegControlMetadata(IMetadata* pMetadata) const -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAppRaw16Reprocessor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    dumpState(android::Printer& printer) -> void;

    virtual auto    reset() -> void override;

    virtual auto    beginFlush() -> void override;
    virtual auto    endFlush() -> void override;

    virtual auto    keepFrameIfNeeded(
                        KeepFrameIfNeededParams const& params
                    ) -> bool override;

    virtual auto    isReprocessFrame(
                        IsReprocessFrameParams const& params
                    ) const -> bool override;

    virtual auto    handleReprocessFrame(
                        HandleReprocessFrameParams const& params
                    ) -> void override;

    virtual auto    debugRequestResult(
                        DebugRequestResultParams const& params
                    ) const -> void override;

    virtual auto    notifyRequestDone(uint32_t requestNo) -> void override;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_CAPTURE_APPRAW16REPROCESSOR_H_

