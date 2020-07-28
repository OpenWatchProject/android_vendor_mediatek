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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "DSDNCfg.h"
#include "StreamingFeature_Common.h"

#define PIPE_CLASS_TAG "DSDNCfg"
#define PIPE_TRACE TRACE_DSDNCFG
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define DSDN_FMT_FULL eImgFmt_NV21
#define DSDN_FMT_DS1  eImgFmt_YUY2
#define DSDN_FMT_DS2  eImgFmt_NV21

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

MVOID DSDNCfg::config(const MSize &fullSize, const MSize &fdSize, MUINT32 multi, MUINT32 divide)
{
    mDebugMultiple = property_get_int32("vendor.debug.fpipe.dsdn.multiple", 0);
    mDebugDivider = property_get_int32("vendor.debug.fpipe.dsdn.divider", 0);
    if(mDebugMultiple != 0 && mDebugDivider != 0)
    {
        MY_LOGI("Use debug ratio(m/d)=(%d/%d) as init ratio", mDebugMultiple, mDebugDivider);
        mInitMultiple = mDebugMultiple;
        mInitDivider = mDebugDivider;
    }
    else
    {
        mInitMultiple = multi;
        mInitDivider = divide;
    }
    //for some hw limitation
    mFullSize = align(fullSize, BUF_ALLOC_ALIGNMENT_BIT);
    mDS1Size = mDS2Size = align( calculateDSSize(mFullSize, fdSize, mInitMultiple, mInitDivider), BUF_ALLOC_ALIGNMENT_BIT);
}

MSize DSDNCfg::getMaxSize(eDSDNImg img) const
{
    switch(img)
    {
    case DSDNCfg::FULL:  return mFullSize;
    case DSDNCfg::DS1:   return mDS1Size;
    case DSDNCfg::DS2:   return mDS2Size;
    default:              return MSize(0,0);
    }
}

MSize DSDNCfg::getSize(eDSDNImg img, const MSize &fullSize, const MSize &fdSize, MUINT32 multiple, MUINT32 divide) const
{
    MUINT32 multi = mInitMultiple;
    MUINT32 div = mInitDivider;
    if(mDebugMultiple != 0 && mDebugDivider != 0)
    {
        MY_LOGI("Use debug ratio(m/d)=(%d/%d)", mDebugMultiple, mDebugDivider);
        multi = mDebugMultiple;
        div = mDebugDivider;
    }
    else if( multiple == 0 || divide == 0)
    {
        MY_LOGW("nvram multi(%d) or divide(%d) == 0 !! useinitRatio(m/d)=(%d/%d)",
                    multiple, divide, mInitMultiple, mInitDivider);
    }
    else if( ((float)multiple / (float)divide) > ((float)mInitMultiple / (float)mInitDivider) )
    {
        MY_LOGW("ratio > Inital Ratio !! use initial ratio. newRatio(m/d)=(%d/%d), initRatio(m/d)=(%d/%d)",
                    multiple, divide, mInitMultiple, mInitDivider);
    }
    else
    {
        multi = multiple;
        div = divide;
    }

    MSize outSize;
    switch(img)
    {
        case DSDNCfg::FULL: outSize = fullSize;                                         break;
        case DSDNCfg::DS1:  outSize = calculateDSSize(fullSize, fdSize, multi, div);    break;
        case DSDNCfg::DS2:  outSize = calculateDSSize(fullSize, fdSize, multi, div);    break;
        default:            outSize = fullSize;
    }

    if( !isValid(outSize) )
    {
        switch(img)
        {
            case DSDNCfg::FULL: outSize = mFullSize;   break;
            case DSDNCfg::DS1:  outSize = mDS1Size;    break;
            case DSDNCfg::DS2:  outSize = mDS2Size;    break;
            default:            outSize = mFullSize;
        }
        MY_LOGW("DSDN out size = 0 !! img(%d) inFullSize" MSize_STR ",fdSize" MSize_STR ",m/d = (%d/%d), finalSize" MSize_STR,
                img, MSize_ARG(fullSize), MSize_ARG(fdSize), multi, div, MSize_ARG(outSize));
    }

    return outSize;
}

EImageFormat DSDNCfg::getFormat(eDSDNImg img) const
{
    switch(img)
    {
    case DSDNCfg::FULL: return DSDN_FMT_FULL;
    case DSDNCfg::DS1:  return DSDN_FMT_DS1;
    case DSDNCfg::DS2:  return DSDN_FMT_DS2;
    default:            return eImgFmt_UNKNOWN;
    }
}

MSize DSDNCfg::calculateDSSize(const MSize &fullSize, const MSize &/*fdSize*/, MUINT32 multi, MUINT32 div) const
{
    const MUINT32 align = 2;
    MSize size = fullSize;
    size.w = (size.w * multi + div - 1) / div;
    size.h = (size.h * multi + div - 1) / div;

    size.w = (size.w + align - 1) & ~(align-1);
    size.h = (size.h + align - 1) & ~(align-1);
    return size;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

