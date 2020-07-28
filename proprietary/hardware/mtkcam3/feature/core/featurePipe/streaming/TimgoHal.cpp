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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "TimgoHal.h"
#include <featurePipe/core/include/DebugUtil.h>
#define PIPE_MODULE_TAG "TimgoHal"
#define PIPE_CLASS_TAG "TimgoHal"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TimgoHal::TimgoHal()
{
}

TimgoHal::~TimgoHal()
{
}

MVOID TimgoHal::init(const MSize &streamingSize)
{
    mDipType = static_cast<NSIoPipe::EDIPTimgoDumpEnum>(getPropertyValue("vendor.camera.dip.timgo.type", NSIoPipe::EDIPTimgoDump_NONE));
    mIsSupport = toSupport(mDipType);

    if(mIsSupport)
    {
        mTypeStr = toTypeStr(mDipType);
        mBufferSize = toBufferSize(mDipType, streamingSize);
        mBufferFmt = toBufferFmt(mDipType);
        MY_LOGD("Timgo enumType(%s:%d), mBufferSize(%dx%d), mBufferFmt(0x%x)", mTypeStr, mDipType, mBufferSize.w, mBufferSize.h, mBufferFmt);
    }
    else
    {
        MY_LOGD("Not Support This Dip Timgo Dump Type(%u). Ignore It.", mDipType);
    }
}

MVOID TimgoHal::uninit()
{
}

MBOOL TimgoHal::isSupport() const
{
    return mIsSupport;
}

NSIoPipe::EDIPTimgoDumpEnum TimgoHal::getType() const
{
    return mDipType;
}

const char* TimgoHal::getTypeStr() const
{
    return mTypeStr;
}

MSize TimgoHal::getBufferSize() const
{
    return mBufferSize;
}

EImageFormat TimgoHal::getBufferFmt() const
{
    return mBufferFmt;
}

MBOOL TimgoHal::toSupport(NSIoPipe::EDIPTimgoDumpEnum dipType)
{
    switch(dipType)
    {
        case NSIoPipe::EDIPTimgoDump_AFTER_LTM: return MTRUE;
        case NSIoPipe::EDIPTimgoDump_AFTER_GGM: return MTRUE;
        default:                                return MFALSE;
    }
}

const char* TimgoHal::toTypeStr(NSIoPipe::EDIPTimgoDumpEnum dipType)
{
    switch(dipType)
    {
        case NSIoPipe::EDIPTimgoDump_NONE:      return "NONE";
        case NSIoPipe::EDIPTimgoDump_AFTER_DGN: return "AFTER_DGN";
        case NSIoPipe::EDIPTimgoDump_AFTER_LSC: return "AFTER_LSC";
        case NSIoPipe::EDIPTimgoDump_AFTER_HLR: return "AFTER_HLR";
        case NSIoPipe::EDIPTimgoDump_AFTER_LTM: return "AFTER_LTM";
        case NSIoPipe::EDIPTimgoDump_AFTER_GGM: return "AFTER_GGM";
        default:                                return "NONE";
    }
}

MSize TimgoHal::toBufferSize(NSIoPipe::EDIPTimgoDumpEnum dipType, const MSize &streamingSize)
{
    switch(dipType)
    {
        case NSIoPipe::EDIPTimgoDump_AFTER_LTM: return streamingSize;
        case NSIoPipe::EDIPTimgoDump_AFTER_GGM: return streamingSize;
        default:                                return MSize(0, 0);
    }
}

EImageFormat TimgoHal::toBufferFmt(NSIoPipe::EDIPTimgoDumpEnum dipType)
{
    switch(dipType)
    {
        case NSIoPipe::EDIPTimgoDump_AFTER_LTM: return eImgFmt_BAYER10;
        case NSIoPipe::EDIPTimgoDump_AFTER_GGM: return eImgFmt_RGB48;
        default:                                return eImgFmt_UNKNOWN;
    }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
