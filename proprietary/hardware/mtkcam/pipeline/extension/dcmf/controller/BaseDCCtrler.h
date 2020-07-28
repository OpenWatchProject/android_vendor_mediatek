/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_BASEDCCTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_BASEDCCTRLER_H_

#include "../../utils/BaseController.h"
#include <mtkcam/feature/stereo/pipe/IDualCamMultiFramePipe.h>
#include <mtkcam/aaa/IHal3A.h>

#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

using namespace android;

typedef NSCam::NSCamFeature::NSFeaturePipe::NSDCMF::IDualCamMFPipe   Pipe;
typedef NSCam::NSCamFeature::NSFeaturePipe::NSDCMF::IDualCamMFPipeCB PipeCB;
typedef NSCam::NSCamFeature::NSFeaturePipe::NSDCMF::PipeRequest      PipeReq;

namespace NSCam
{
namespace plugin
{
class BaseDCCtrler : public BaseController
{
public:
    // Due to some reason, DCMF controller is attached a container for saving
    // the original App Metadata setting because DCMF may change the
    // setting of App metadata.
    enum class AppSetting
    {
        CONTROL_AE_MODE = 0,
        CONTROL_AWB_LOCK,
        CONTROL_VIDEO_STABILIZATION_MODE,
        SENSOR_SENSITIVITY,
        SENSOR_EXPOSURE_TIME,

        SIZE,
    };
// this class is not allow to instantiate.
protected:
    BaseDCCtrler(
                    NSCamFeature::NSFeaturePipe::NSDCMF::DualCamMFType mode);
    BaseDCCtrler(
                    const BaseDCCtrler&) = delete;
    BaseDCCtrler(
                    BaseDCCtrler&) = delete;
public:
    ~BaseDCCtrler();
//
// override BaseController
//
public:
    using BaseController::enqueFrame;
    using BaseController::getEnquedTimes;
    using BaseController::dequeFrame;
    // The job will be executed while invoking BaseController::execute.
    //  @return                 Result of the job.
    //  @note                   This method always returns 0.
    //                          This method is thread-safe.
    intptr_t                job(
                                    intptr_t arg1 = 0,
                                    intptr_t arg2 = 0) override;

    // Enque a frame by open id, arguments are passed by IVendor.
    // And signal one thread which is waiting for frame.
    //  @param openId           open id
    //  @param requestNo        The request number from IVendor.
    //  @param bufParam         The argument from IVendor.
    //  @param metaParam        The argument from IVendor.
    //  @param cb               The argument from IVendor.
    //  @note                   This method is thread-safe.
    int                     enqueFrame(
                                    const MINT32 openId,
                                    MUINT32 requestNo,
                                    const IVendor::BufferParam& bufParam,
                                    const IVendor::MetaParam& metaParam,
                                    wp<IVendor::IDataCallback> cb) override;

    // Enque a RequestFrame directly by openId. This method wil signal a thread which is
    // waiting for a frame. It's usually to enque a nullptr as a dummy frame to
    // raise the thread is blocked by dequeFrame.
    //  @param frame            A RequestFrame, can be nullptr.
    //  @note                   This method is thread-safe.
    virtual void            enqueFrame(
                                    MINT32 openId,
                                    std::shared_ptr<RequestFrame> frame);
    // Clear all frames in queue.
    //  @note                   This method is thread-safe.
    void                    clearFrames() override;
    void                    doCancel() override;
//
// Basic operation
//
public:
    // do init flow
    virtual bool            init(wp<Pipe> pipe);
    virtual bool            updateDCMFRequest(
                                    sp<PipeReq>& request,
                                    std::shared_ptr<RequestFrame>& main1Frame,
                                    std::shared_ptr<RequestFrame>& main2Frame,
                                    MBOOL isMainFrame = MFALSE,
                                    MBOOL isLastFrame = MFALSE);
    virtual bool            DCMFProcessingDone(
                                    PipeCB::EventId eventType,
                                    sp<PipeReq>& request);
    virtual bool            postviewDone(
                                    PipeCB::EventId eventType,
                                    sp<PipeReq>& request);

    virtual bool            yuvDone(
                                    PipeCB::EventId eventType,
                                    sp<PipeReq>& request);

    virtual MUINT32         queryCaptureFrameCount();

private:
    bool enqueToSpecificQueue(
                                MINT32 openId,
                                std::shared_ptr<RequestFrame> frame);
    void pushPairToIncomingQueue();

    // To restore the stored app metadata setting (in m_originalAppSetting) to
    // the metadata.
    //  @param pAppMeta         The output app metadata to be restored.
    void restoreAppSetting(IMetadata* pAppMeta);
//
// get & set
//
public:
    // get
    inline MINT32           getMain1OpenId() const noexcept     {return mMain1OpenId;}
    inline MINT32           getMain2OpenId() const noexcept     {return mMain2OpenId;}
    inline MUINT32          getCaptureNum()  const noexcept     {return mCaptureNum;}
    inline MUINT32          getDelayFrameNum() const noexcept   {return mDelayFrameNum;}
    inline MUINT32          getPairEnqueTimes() const noexcept  {return mPairEnqueTimes;}
    inline MUINT32          getHWSyncStableNum() const noexcept  {return mHWSyncStableNum;}
    // set
    inline void             setCaptureNum(const MUINT32& num) noexcept {mCaptureNum = num;}
    inline void             setDelayFrameNum(const MUINT32& num) noexcept {mDelayFrameNum = num;}
    inline void             setHWSyncStableNum(const MUINT32& num) noexcept {mHWSyncStableNum = num;}
    inline void             setAppSetting(AppSetting s, const int64_t& v) noexcept { m_originalAppSetting[static_cast<size_t>(s)] = v; }
private:
    sp<PipeReq>
                            generatePipeRequst(
                                    sp<PipeCB> pipeCB,
                                    std::shared_ptr<RequestFrame>& main1Frame,
                                    std::shared_ptr<RequestFrame>& main2Frame);
    MBOOL                   enquePipeRequest(
                                    sp<PipeReq>& request,
                                    std::shared_ptr<RequestFrame>& main1Frame,
                                    std::shared_ptr<RequestFrame>& main2Frame);
protected:
    class ProcessRequestFrameSet
    {
    public:
        ProcessRequestFrameSet(
                    MINT32 reqNo,
                    std::shared_ptr<RequestFrame>& main1Frame,
                    std::shared_ptr<RequestFrame>& main2Frame
        )
        {
            requestNo = reqNo;
            this->main1Frame = main1Frame;
            this->main2Frame = main2Frame;
        }

        // copy and move object is forbidden
        ProcessRequestFrameSet(const ProcessRequestFrameSet&) = delete;
        ProcessRequestFrameSet(ProcessRequestFrameSet&&) = delete;
        std::shared_ptr<RequestFrame> main1Frame = nullptr;
        std::shared_ptr<RequestFrame> main2Frame = nullptr;
        MINT32 requestNo = -1;
    };
//
// Attributes
//
protected:
    sp<PipeCB> mpCB = nullptr;
    // saves main request frame
    std::weak_ptr<RequestFrame>                 m_mainRequestFrame;

    typedef NSCamFeature::NSFeaturePipe::NSDCMF::DualCamMFType _DualCamMFType;
    _DualCamMFType      mMode = _DualCamMFType::TYPE_DCMF_unknown;

    MINT32      mUniqueKey       = -1;
    MINT32      mSubmittedCount  = 0;
private:
    //
    MINT32      mMain1OpenId = -1;
    MINT32      mMain2OpenId = -1;
    //
    MINT32      mCaptureNum = 0;
    MINT32      mDelayFrameNum = 0;
    //
    std::deque< std::shared_ptr<RequestFrame> > mMain1Frames;
    std::deque< std::shared_ptr<RequestFrame> > mMain2Frames;
    //
    wp<Pipe> mwpPipe = nullptr;
    //
    std::mutex mFrameSetLock;
    std::vector< std::shared_ptr<ProcessRequestFrameSet> > mFrameSets;
    //
    MUINT32     mPairEnqueTimes = 0;
    MUINT32     mHWSyncStableNum = 0;

    // A container to stored the original app setting before BMDNVendor
    // overwrite it
    std::vector<int64_t> m_originalAppSetting;

    NSCam::TuningUtils::FileReadRule    mRule;
};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_BASEDCCTRLER_H_