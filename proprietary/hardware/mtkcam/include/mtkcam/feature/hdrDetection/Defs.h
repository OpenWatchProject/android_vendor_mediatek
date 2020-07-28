/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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

/**
 * @file Defs.h
 *
 * HDR Detection Defs
 *
 */

#ifndef _HDR_DETECTION_DEFS_H_
#define _HDR_DETECTION_DEFS_H_

#include <mtkcam/def/BuiltinTypes.h>

// ---------------------------------------------------------------------------

namespace NSCam {

// HDR mode
enum class HDRMode : uint8_t {
    // disable high dynamic range imaging techniques
    //
    // logically equivalent to
    // scene-mode ≠ SCENE_MODE_HDR
    OFF = 0,
    // capture a scene using high dynamic range imaging techniques
    //
    // logically equivalent to
    // scene-mode = SCENE_MODE_HDR
    ON,
    // capture a scene using high dynamic range imaging techniques
    // supports HDR scene detection
    //
    // logically equivalent to
    // scene-mode = SCENE_MODE_HDR
    // hdr-auto-mode = on
    AUTO,
    // capture/preview/record a scene using high dynamic range imaging techniques
    //
    // logically equivalent to
    // scene-mode = SCENE_MODE_HDR
    // hdr-auto-mode = off
    // video-hdr = on
    VIDEO_ON,
    // capture/preview/record a scene using high dynamic range imaging techniques
    // supports HDR scene detection
    //
    // logically equivalent to
    // scene-mode = SCENE_MODE_HDR
    // hdr-auto-mode = on
    // video-hdr = on
    VIDEO_AUTO,
    // Placeholder for range check; please extend results before this one
    NUM
};

// HDR detection result
enum class HDRDetectionResult {
    // HDR detection is turned off
    NONE = -1,
    // HDR should be turned off or the strength should be moderate
    NORMAL,
    // HDR should be turned on or the strength should be aggresive
    HDR,
    // Placeholder for range check; please extend results before this one
    NUM
};

} // namespace NSCam

#endif // _HDR_DETECTION_DEFS_H_
