/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _IHDRPROC3_H_
#define _IHDRPROC3_H_

#include <vector>

#include <utils/RefBase.h>
#include <utils/Vector.h>

#include <mtkcam3/feature/hdr/HDRDefs3.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#include <mtkcam/main/core/module.h>

using namespace android;
using namespace NSCam::HDRCommon;

namespace NSCam {
namespace HDR3 {

// ---------------------------------------------------------------------------

typedef MBOOL (*HDRProcCompleteCallback_t)(
        MVOID* user, const sp<IImageBuffer>& hdrResult, MBOOL ret);

class IHDRProc3;

// ---------------------------------------------------------------------------

class HDRProc3Factory
{
public:
    virtual ~HDRProc3Factory() = default;

    template<typename... Args>
    static IHDRProc3& getIHDRProc3(const Args&... /*args*/)
    {
        // Typedefs of function pointer that generate IHDRProc3 instance(s)
        typedef IHDRProc3* (*IHDRProc3_factory_t)();
        return *MAKE_MTKCAM_MODULE(
                MTKCAM_MODULE_ID_HDR_PROC2, IHDRProc3_factory_t/*, args...*/);
    }
};

// ---------------------------------------------------------------------------

class IHDRProc3
{
public:
    virtual ~IHDRProc3() = default;

    static IHDRProc3* loadModule();

    // init the instance
    virtual MBOOL init(MINT32 openID, HDRHandle& handle) = 0;

    // uninit the instance
    virtual MBOOL uninit(const HDRHandle& handle) = 0;

    virtual MBOOL setParam(
            const HDRHandle& handle,
            MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2) = 0;

    virtual MBOOL getParam(
            const HDRHandle& handle,
            MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2) = 0;

    virtual MBOOL setShotParam(
            const HDRHandle& handle,
            MSize& pictureSize, MSize& postviewSize, MRect& cropRegion) = 0;

    virtual MBOOL prepare(const HDRHandle& handle) = 0;

    // addInputFrame() set full YUV and grayscale images as HDR's input buffers.
    //
    // NOTE: the frame index ordering should be
    // {0, 2, 4, ...} for YUV and {1, 3, 5, ...} for grayscale frames
    virtual MBOOL addInputFrame(
            const HDRHandle& handle,
            MINT32 frameIndex, const sp<IImageBuffer>& inBuffer) = 0;

    // start() is an asynchronous call and do the following tasks:
    // 1. creates a detached worker thread that processes the post-processing task
    // 2. waits for all HDR input buffers are received
    // 3. notifies the caller once the post-processing task is done
    virtual MBOOL start(const HDRHandle& handle) = 0;

    // release() releases all internal working buffers
    virtual MBOOL release(const HDRHandle& handle) = 0;

    // getHDRCapInfo() is used to
    // 1. get exposure settings
    // 2. ISP settings
    //    a. LTM configuration per HDR input frame;
    //       used to determine whether to apply LTM of each HDR input frame
    //
    // NOTE: the number of HDR input frames equals vCapParam's size
    virtual MBOOL getHDRCapInfo(
            const HDRHandle& handle,
            std::vector<HDRCaptureParam>& vCapParam) const = 0;

    // setCompleteCallback() registers a deferred callback, which notifies
    // the caller that the post-processing task is done.
    virtual MVOID setCompleteCallback(
            const HDRHandle& handle,
            HDRProcCompleteCallback_t completeCB,
            MVOID* user) = 0;
};

} // namespace HDR3
} // namespace NSCam

#endif // _IHDRPROC3_H_
