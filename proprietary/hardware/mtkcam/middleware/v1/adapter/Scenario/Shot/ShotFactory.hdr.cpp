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

#define LOG_TAG "MtkCam/Shot"

#include <mtkcam/utils/std/Log.h>

#include <mtkcam/middleware/v1/IShot.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/middleware/v1/IParamsManager.h>

#include <mtkcam/feature/hdrDetection/Defs.h>

#include <cutils/properties.h>
#include <cutils/compiler.h>

using namespace android;
using namespace android::NSShot;
using namespace NSCam;

// ---------------------------------------------------------------------------

extern sp<IShot> createInstance_NormalShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId);

#ifdef MTKCAM_HAVE_HDR
extern sp<IShot> createInstance_HdrShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId);

extern sp<IShot> createInstance_VHdrShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId,
        uint32_t const      u4VHDRMode,
        bool     const      bIsAutoHDR);

extern sp<IShot> createInstance_CollectShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId);

extern sp<IShot> createInstance_ZsdCollectShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId);

extern sp<IShot> createInstance_ZHDRShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId,
        bool const          isAutoHDR);

extern sp<IShot> createInstance_ZsdZHDRShot(
        char const * const  pszShotName,
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId,
        bool const          isAutoHDR);
#else
#define createInstance_HdrShot \
        createInstance_NormalShot

#define createInstance_VHdrShot(name, smode, id, ...) \
        createInstance_NormalShot(name, smode, id)

#define createInstance_CollectShot(name, mode, id, ...) \
            createInstance_NormalShot(name, mode, id)

#define createInstance_ZsdCollectShot(name, mode, id, ...) \
            createInstance_NormalShot(name, mode, id)

#define createInstance_ZHDRShot(name, mode, id, ...) \
        createInstance_NormalShot(name, mode, id)

#define createInstance_ZsdZHDRShot(name, mode, id, ...) \
        createInstance_NormalShot(name, mode, id)
#endif

// ---------------------------------------------------------------------------

static const char* toString(const uint32_t vHDRMode)
{
    switch (vHDRMode)
    {
        case SENSOR_VHDR_MODE_IVHDR:
            return MtkCameraParameters::VIDEO_HDR_MODE_IVHDR;
        case SENSOR_VHDR_MODE_MVHDR:
            return MtkCameraParameters::VIDEO_HDR_MODE_MVHDR;
        case SENSOR_VHDR_MODE_ZVHDR:
            return MtkCameraParameters::VIDEO_HDR_MODE_ZVHDR;
        case SENSOR_VHDR_MODE_NONE:
            return "none";
        default:
            CAM_LOGW("invalid vhdr mode(%d)", vHDRMode);
            return "N/A";
    }
}

// ---------------------------------------------------------------------------

extern sp<IShot> createHdrShotInstance(
        uint32_t const      u4ShotMode,
        int32_t const       i4OpenId,
        sp<IParamsManager>  pParamsMgr,
        bool                isZsd)
{
    // NOTE:
    // query from feature table to decide whether
    // single-frame HDR capture or multi-frame HDR capture is supported
    bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();
    CAM_LOGD("isSingleFrameCaptureHDR(%s)",
            isSingleFrameCaptureHDR ? "yes" : "no");

    if (isSingleFrameCaptureHDR)
    {
        const uint32_t VHDRMode = pParamsMgr->getVHdr();
        const bool isAutoHDR = (pParamsMgr->getHDRMode() == HDRMode::AUTO) ||
                               (pParamsMgr->getHDRMode() == HDRMode::VIDEO_AUTO);
        CAM_LOGD("VHDRMode(%s) auto(%s)",
                toString(VHDRMode), isAutoHDR ? "on" : "off");

        if (SENSOR_VHDR_MODE_ZVHDR == VHDRMode) {
            return isZsd
                ? createInstance_ZsdZHDRShot("ZHDR", u4ShotMode, i4OpenId,isAutoHDR)
                : createInstance_ZHDRShot("ZHDR", u4ShotMode, i4OpenId,isAutoHDR);
        }
        else {
            return createInstance_VHdrShot(
                    "VHDR", u4ShotMode, i4OpenId, VHDRMode, isAutoHDR);
        }
    }

    {
        return createInstance_HdrShot("HDR", u4ShotMode, i4OpenId);
    }
}
