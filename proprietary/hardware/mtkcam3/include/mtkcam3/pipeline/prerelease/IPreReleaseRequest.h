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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PRERELEASEREQUEST_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PRERELEASEREQUEST_H_
//
#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <mtkcam/def/common.h>
#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace prerelease {

class IPreReleaseRequest
    : public android::RefBase
{
public:
    using IPipelineFrame = NSCam::v3::IPipelineFrame;

public:
    virtual void                         start() = 0;
    virtual void                         uninit() = 0;
    virtual void                         waitUntilDrained() = 0;
    virtual void                         registerStreamBuffer(
                                             android::sp<IPipelineFrame>const& pFrame
                                         ) = 0;
};

class IPreReleaseRequestMgr
    : public android::RefBase
{
public:
    using IPipelineFrame = NSCam::v3::IPipelineFrame;
    using IPipelineContext = NSCam::v3::pipeline::NSPipelineContext::IPipelineContext;

public:
    enum Status {
        RELEASE = 0,
        PRERELEASE
    };

    enum RequestStatus {
        UNKNOWN = 0,
        SUCCESS,
        ERROR
    };

public:
    static IPreReleaseRequestMgr*           getInstance();
    /* called by pipeline model's beginFlush() */
    virtual android::sp<IPreReleaseRequest>
                                            createPreRelease(android::sp<IPipelineContext> pContext) = 0;
    /* called by pipeline model's endFlush() */
    virtual android::sp<IPreReleaseRequest>
                                            getPreRelease() = 0;
    /* called by pipeline frame while capture done for non-BG frame */
    virtual bool                            notifyCaptureCompleted(android::sp<IPipelineFrame>const& pFrame, uint32_t bufferStatus) = 0;
    /* called by JpegNode, JpegNode will call this API before markPreRelease */
    virtual bool                            registerStreamBuffer(android::sp<IPipelineFrame>const& pFrame) = 0;
    /* called by PipelineModelSessionFactory's decidePipelineModelSession() */
    virtual void                            configBGService(IMetadata& meta) = 0;
    /* called by waitUntilDrained(), set current PreReleaseRequest instance for un-initialing un-used modules on BGService */
    virtual void                            setPreReleaseRequest(android::sp<IPreReleaseRequest> pPreRelease) = 0;
    /* called by waitUntilDrained(), add to mReqItems if having non-finished request before it or callback to AP directly */
    virtual void                            setRequestFinished(uint32_t frameNo, uint32_t preReleaseUid, uint32_t requestStatus);
    /* called by registerStreamBuffer(), add this request to mReqItems and remove it from mvInFlightRequests, move CaptureCompleted items before it from mvInFlightRequests to mReqItems to ensure the callback order */
    virtual void                            addPreReleaseRequest(uint32_t frameNo, uint32_t preReleaseUid, int32_t imgReaderID, int64_t timestamp);
    /* called by NextCaptureListener */
    virtual uint32_t                        getPreleaseRequestCnt();
    /* called by pipeline model's destructor */
    virtual void                            uninit() = 0;
    /* called by pipeline model's processEvaluatedFrame() if request with JpegNode */
    virtual void                            addCaptureRequest(uint32_t requestNo, bool bCallback) = 0;
    virtual void                            dump() = 0;
    /* called by start(), check inflight request before starting waitUntilDrained() because notifyCaptureCompleted() may be called earlier than addCaptureRequest() */
    virtual void                            checkInFlightRequest() = 0;
    /* called by PreReleaseRequest's constructor */
    virtual void                            incPreRelease() = 0;
    /* called by PreReleaseRequest's destructor */
    virtual void                            decPreRelease() = 0;

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace prerelease
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PRERELEASEREQUEST_H_

