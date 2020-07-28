/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_BASE_VSDOF_CTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_BASE_VSDOF_CTRLER_H_

#include "../../utils/BaseController.h"

using namespace android;

namespace NSCam
{
namespace plugin
{

typedef std::shared_ptr<RequestFrame> ReqFrameSP;

class BaseVsdofCtrler : public BaseController
{
        // this class is not allow to instantiate.
    protected:
        BaseVsdofCtrler(MINT32 mode);
        BaseVsdofCtrler(const BaseVsdofCtrler&) = delete;
        BaseVsdofCtrler(BaseVsdofCtrler&) = delete;
    public:
        ~BaseVsdofCtrler();


        //
        // override BaseController
        //
    public:
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
        void            enqueFrame(
            MINT32 openId,
            MUINT32 requestNo,
            const IVendor::BufferParam& bufParam,
            const IVendor::MetaParam& metaParam,
            wp<IVendor::IDataCallback> cb) override;

#if 0
        // Enque a RequestFrame directly by openId. This method wil signal a thread which is
        // waiting for a frame. It's usually to enque a nullptr as a dummy frame to
        // raise the thread is blocked by dequeFrame.
        //  @param frame            A RequestFrame, can be nullptr.
        //  @note                   This method is thread-safe.
        void            enqueFrame(MINT32 openId, ReqFrameSP frame);
#endif
        // Clear all frames in queue.
        //  @note                   This method is thread-safe.
        void                    clearFrames() override;
        void                    doCancel() override;

        //
        // Basic operation
        //
    public:
        // do init flow
        virtual bool            init();
        virtual bool            doDualCamProcess(
            ReqFrameSP main1Frame,
            ReqFrameSP main2Frame);


    private:
        bool enqueToSpecificQueue(
            MINT32 openId,
            ReqFrameSP frame);
        void pushPairToIncomingQueue();

        //
        // get & set
        //
    public:
        // get
        inline MINT32           getMain1OpenId() const noexcept {
            return mMain1OpenId;
        }
        inline MINT32           getMain2OpenId() const noexcept {
            return mMain2OpenId;
        }
        inline MUINT32          getCaptureNum()  const noexcept {
            return mCaptureNum;
        }
        inline MUINT32          getDelayFrameNum() const noexcept {
            return mDelayFrameNum;
        }
        // set
        inline void             setCaptureNum(const MUINT32& num) noexcept {
            mCaptureNum = num;
        }
        inline void             setDelayFrameNum(const MUINT32& num) noexcept {
            mDelayFrameNum = num;
        }


        //
        // Attributes
        //
    public:
        static MINT32           mTotalRefCount;
        MINT32                  mRefCount; // = 0;

    private:
        MINT32                  mMode; // = 0;
        //
        MINT32                  mMain1OpenId;
        MINT32                  mMain2OpenId;
        //
        MINT32                  mCaptureNum; // = 0;
        MINT32                  mDelayFrameNum; // = 0;
        //
        std::deque< ReqFrameSP > mMain1Frames;
        std::deque< ReqFrameSP > mMain2Frames;
        //
};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_BASE_VSDOF_CTRLER_H_