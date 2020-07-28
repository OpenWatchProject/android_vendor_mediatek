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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_COLLECTRAW_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_COLLECTRAW_H_

// AOSP
#include <utils/RefBase.h>

// MTKCAM
#include <mtkcam/pipeline/extension/IVendorManager.h>

// STL
#include <memory>

#if MTKCAM_HAVE_HDR_SUPPORT
#define MAKE_COLLECTVendor(name, id, mode) NSCam::plugin::CollectVendor::createInstance(name, id, mode)
#else
#define MAKE_COLLECTVendor(name, id, mode) nullptr
#endif

/* For AE stable frame delay */
#define COLLECT_FRAME_DELAY    3
static_assert(COLLECT_FRAME_DELAY >= 0, "THDR_FRAME_DELAY must be or enqual to 0");

using namespace android;


namespace NSCam {
namespace plugin {

class CollectVendor : public virtual IVendor
{

public:
    struct ConfigParams
    {
        MBOOL isZSDMode;
        ConfigParams()
            : isZSDMode(MTRUE)
        {}
    };


//
// Static Methods
//
public:
    static
    android::sp<CollectVendor>     createInstance(
                                    char const*  pcszName,
                                    MINT32 const i4OpenId,
                                    MINT64 const vendorMode
                                );

//
// FrameDescriptor, for frame information sync usage. These methods are thread-safe
//
public:
    struct FrameDescriptor
    {
        MINT32  requestNo;
        MINT32  iso;
        MINT32  exp;
        MBOOL   isLast;
        MBOOL   isDropped;
        FrameDescriptor() : requestNo(-1), iso(0), exp(0), isLast(MFALSE), isDropped(MFALSE) {}
    };


public:
    static
    std::shared_ptr<FrameDescriptor>
                                takeFrameDescriptor(
                                    MINT32 requestNo
                                );

    static
    void                        addFrameDescriptor(
                                    MINT32 requestNo,
                                    const std::shared_ptr<FrameDescriptor>& d
                                );

    static
    void                        clearFrameDescriptors();

};// class CollectVendor
} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_EXTENSION_COLLECTRAW_H_
