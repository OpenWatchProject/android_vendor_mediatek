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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_DCMFCTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_DCMFCTRLER_H_

#include "BaseDCCtrler.h"

using namespace android;

namespace NSCam
{
namespace plugin
{

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class DCMFCtrler : public BaseDCCtrler
{
// this class is not allow to instantiate.
public:
    DCMFCtrler(NSCamFeature::NSFeaturePipe::NSDCMF::DualCamMFType mode);
    ~DCMFCtrler();
public:
    using BaseDCCtrler::getCaptureNum;
    using BaseDCCtrler::getDelayFrameNum;
    using BaseDCCtrler::setCaptureNum;
    using BaseDCCtrler::setDelayFrameNum;
//
// override base operation
//
public:
    bool            updateDCMFRequest(
                            sp<PipeReq>& request,
                            std::shared_ptr<RequestFrame>& main1Frame,
                            std::shared_ptr<RequestFrame>& main2Frame,
                            MBOOL isMainFrame = MFALSE,
                            MBOOL isLastFrame = MFALSE) override;
    bool            DCMFProcessingDone(
                            PipeCB::EventId eventType,
                            sp<PipeReq>& request) override;
    bool            yuvDone(
                            PipeCB::EventId eventType,
                            sp<PipeReq>& request) override;

    MUINT32         queryCaptureFrameCount() override;

    android::status_t
                    onEvent(
                        PipeCB::EventId eventType,
                        sp<PipeReq>& request,
                        void* param1 = NULL,
                        void* param2 = NULL);
private:
    void           updateMfllStrategy();

    void           updateHdrStrategy();

    void           updateCurrent3A();

    // mfnr
    int            m_realIso = 0;
    int            m_shutterTime = 0;
    int            m_finalRealIso = 0;
    int            m_finalShutterTime = 0;
    bool           m_enableMfnr = 0;

    // hdr
    bool           m_enableHdr = 0;
//
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class DCMFListenerImp : public NSCamFeature::NSFeaturePipe::NSDCMF::IDualCamMFPipeCB
{
public:
    DCMFListenerImp(DCMFCtrler* ctrler) : pCtrler(ctrler)
    {
    }
    virtual ~DCMFListenerImp()
    {
    }
    status_t onEvent(
                    PipeCB::EventId eventType,
                    sp<PipeReq>& request,
                    void* param1 = NULL,
                    void* param2 = NULL
    )
    {
        return pCtrler->onEvent(eventType, request, param1, param2);
    }
private:
    DCMFCtrler* pCtrler = nullptr;
};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_DCMFCTRLER_H_