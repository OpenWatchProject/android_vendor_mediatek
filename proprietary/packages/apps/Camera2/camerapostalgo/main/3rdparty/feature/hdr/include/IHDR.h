/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _IHDR_H_
#define _IHDR_H_

#include <vector>

#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/Condition.h>
#include <semaphore.h>

#include <core/common.h>
#include <utils/imgbuf/IImageBuffer.h>
#include <HDRDefs2.h>


using namespace android;
using namespace NSCam::HDRCommon;

namespace NSCam {
// ---------------------------------------------------------------------------

typedef MBOOL (*HDRProcCompleteCallback_t)(MVOID* user, const sp<IImageBuffer>& hdrResult, MBOOL ret);

// ---------------------------------------------------------------------------

class IHDR : public virtual RefBase
{
public:
    virtual ~IHDR() = default;

    // init the instance
    virtual MBOOL init() = 0;

    // uninit the instance
    virtual MBOOL uninit() = 0;

    // Update resolution information...make HDR directory
    virtual MBOOL updateInfo() = 0;

    virtual MBOOL setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2) = 0;

    virtual MBOOL getParam(MUINT32 paramId, MUINT32& rArg1, MUINT32& rArg2) = 0;

    virtual MBOOL setShotParam(void const* pParam) = 0;

    // prepare() is used to init HDR HAL and
    // allocate processing memory asynchronously
    virtual MBOOL prepare() = 0;

    // addInputFrame() set full YUV and grayscale images as HDR's input buffers.
    //
    // NOTE: the frame index ordering should be
    // {0, 2, 4, ...} for YUV and {1, 3, 5, ...} for grayscale frames
    virtual MBOOL addInputFrame(
            MINT32 frameIndex, const sp<IImageBuffer>& inBuffer) = 0;

    // process() is an asynchronous call and do the following tasks:
    // 1. creates a detached worker thread that processes the post-processing task
    // 2. waits for all HDR input buffers are received
    // 3. notifies the caller once the post-processing task is done
    virtual MBOOL process() = 0;

    // release() releases all internal working buffers
    virtual MBOOL release()= 0;

    // getHDRCapInfo() is used to
    // 1. get exposure settings
    // 2. record the number of HDR input frames to mu4OutputFrameNum
    // @deprecated, valid until HDR 2.2
    //virtual MBOOL getCaptureInfo(Vector<NS3Av3::CaptureParam_T>& vCapExpParam, MINT32& hdrFrameNum) = 0;

    // getHDRCapInfo() is used to
    // 1. get exposure settings
    // 2. ISP settings
    //    a. LTM configuration per HDR input frame;
    //       used to determine whether to apply LTM of each HDR input frame
    //
    // NOTE: the number of HDR input frames equals vCapParam's size
    // valid since HDR 2.3
    //virtual MBOOL getCaptureInfo(std::vector<HDR2::HDRCaptureParam>& vCapParam) = 0;

    // setCompleteCallback() registers a deferred callback, which notifies
    // the caller that the post-processing task is done.
    virtual MVOID setCompleteCallback(
            HDRProcCompleteCallback_t completeCB, MVOID* user) = 0;
    //query algo type
    virtual MINT32 queryHDRAlgoType(void) = 0;
};
} // namespace NSCam

#endif // _IHDR_H_
