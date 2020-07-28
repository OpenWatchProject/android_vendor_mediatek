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
#ifndef _MTK_HARDWARE_MTKCAM_FEATURE_DUAL_IT_EFFECTHAL_H_
#define _MTK_HARDWARE_MTKCAM_FEATURE_DUAL_IT_EFFECTHAL_H_
#include <string>
//
class DpBlitStream;
//
namespace NSCam{
namespace NSIoPipe{
namespace NSSImager{
    class IImageTransform;
};
};
};
//
using namespace android;
namespace NSCam{
class EffectRequest;
typedef android::sp<EffectRequest> EffectRequestPtr;
class DualITEffectHal : public EffectHalBase
{
    public:
        enum
        {
            DIT_BUF_JPS_LEFT,
            DIT_BUF_JPS_RIGHT,
            DIT_BUF_JPS_OUTPUT
        };
    public:
        DualITEffectHal();
        virtual ~DualITEffectHal();
        MBOOL onFlush();
    protected:
        virtual status_t   initImpl();
        virtual status_t   uninitImpl();
        //non-blocking
        virtual status_t   prepareImpl();
        virtual status_t   releaseImpl();
        virtual status_t   getNameVersionImpl(
                                EffectHalVersion &nameVersion) const;
        virtual status_t   startImpl(
                                uint64_t *uid=NULL);
        virtual status_t   abortImpl(
                                EffectResult &result,
                                EffectParameter const *parameter=NULL);
        //non-blocking
        virtual status_t   updateEffectRequestImpl(
                                const EffectRequestPtr request);
        // Currently, these funcs will not used.
        virtual bool allParameterConfigured();
        virtual status_t   getCaptureRequirementImpl(
                                EffectParameter *inputParam,
                                Vector<EffectCaptureRequirement> &requirements) const;
        virtual status_t   setParameterImpl(
                                android::String8 &key,
                                android::String8 &object);
        virtual status_t   setParametersImpl(
                                android::sp<EffectParameter> parameter);
    private:
        MBOOL doImageTransform(
                                sp<IImageBuffer> srcBuf,
                                sp<IImageBuffer> dstBuf,
                                MBOOL const isRightBuf);
        MBOOL processImageByMDP(
                                sp<IImageBuffer> srcBuf,
                                sp<IImageBuffer> dstBuf,
                                MINT32 rot);
    private:
        NSCam::NSIoPipe::NSSImager::IImageTransform*
                                        mpImgTransform = nullptr;
        mutable Mutex                   mLock;
        DpBlitStream*                   mpDpStream = nullptr;
        MINT32                          miDump = MFALSE;
        std::string                     msFilename = "";
};
};
#endif //_MTK_HARDWARE_MTKCAM_FEATURE_DUAL_IT_EFFECTHAL_H_