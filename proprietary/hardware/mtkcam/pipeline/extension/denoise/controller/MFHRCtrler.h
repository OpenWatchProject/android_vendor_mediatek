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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_MFHRCTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_MFHRCTRLER_H_

#include "BaseDNCtrler.h"

using namespace android;

namespace NSCam
{
namespace plugin
{

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class MFHRCtrler : public BaseDNCtrler
{
// this class is not allow to instantiate.
public:
    MFHRCtrler(
                    MINT32 mode);
    ~MFHRCtrler();
public:
    using BaseDNCtrler::getCaptureNum;
    using BaseDNCtrler::getDelayFrameNum;
    using BaseDNCtrler::setCaptureNum;
    using BaseDNCtrler::setDelayFrameNum;
    typedef NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB PipeCB;
//
// override base operation
//
public:
    bool            updateDenoiseRequest(
                            sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request,
                            std::shared_ptr<RequestFrame>& main1Frame,
                            std::shared_ptr<RequestFrame>& main2Frame,
                            MBOOL isMainFrame = MFALSE) override;
    bool            denoiseProcessingDone(
                            NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB::EventId eventType,
                            sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request) override;
    bool            postviewDone(
                            NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB::EventId eventType,
                            sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request) override;
    android::status_t
                    onEvent(
                        PipeCB::EventId eventType,
                        sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request,
                        void* param1 = NULL,
                        void* param2 = NULL);
//
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class MFHRListenerImp : public NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB
{
public:
    MFHRListenerImp(MFHRCtrler* ctrler) : pCtrler(ctrler)
    {
    }
    virtual ~MFHRListenerImp()
    {
    }
    status_t onEvent(
                    NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB::EventId eventType,
                    sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request,
                    void* param1 = NULL,
                    void* param2 = NULL
    )
    {
        return pCtrler->onEvent(eventType, request, param1, param2);
    }
private:
    MFHRCtrler* pCtrler = nullptr;
};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_MFHRCTRLER_H_