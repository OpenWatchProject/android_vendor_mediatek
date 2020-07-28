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

#ifndef _HDRPROC2_H_
#define _HDRPROC2_H_

#include <utils/Singleton.h>
#include "HDRDefsCommon.h"

using namespace android;
using namespace NSCam::HDRCommon;
//using namespace NSCam::HDR2;

typedef MBOOL (*HDRProcCompleteCallback_t)(
        MVOID* user, const sp<IImageBuffer>& hdrResult, MBOOL ret);

// ---------------------------------------------------------------------------

class HDRProc2 : public Singleton<HDRProc2>

{
public:
    HDRProc2();
    ~HDRProc2();

    MBOOL init(MINT32 openID, HDRHandle& handle);

    MBOOL uninit(const HDRHandle& handle);

    MBOOL setParam(
            const HDRHandle& handle,
            MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2);

    MBOOL getParam(
            const HDRHandle& handle,
            MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2);

    MBOOL setShotParam(
            const HDRHandle& handle,
            MSize& pictureSize, MSize& postviewSize, MRect& cropRegion);

    MBOOL prepare(const HDRHandle& handle);

    MBOOL addInputFrame(
            const HDRHandle& handle,
            MINT32 frameIndex, const sp<IImageBuffer>& inBuffer);
    MBOOL addOutputFrame(
            const HDRHandle& handle,
            HDROutputType type, sp<IImageBuffer>& outBuffer);

    MBOOL start(const HDRHandle& handle);
    MBOOL release(const HDRHandle& handle);

#if 0
    MBOOL getHDRCapInfo(
            const HDRHandle& handle,
            MINT32& i4FrameNum,
            Vector<MUINT32>& vu4Eposuretime,
            Vector<MUINT32>& vu4SensorGain,
            Vector<MUINT32>& vu4FlareOffset);
    MBOOL getHDRCapInfo(
            const HDRHandle& handle,
            std::vector<HDRCaptureParam>& vCapParam) const;
#endif

    MVOID setCompleteCallback(
            const HDRHandle& handle,
            HDRProcCompleteCallback_t completeCB,
            MVOID* user);
    MINT32 queryHDRAlgoType(const HDRHandle& handle);

private:
    MBOOL uninitLocked(const HDRHandle* handle = nullptr);
};

#endif // _HDRPROC2_H_
