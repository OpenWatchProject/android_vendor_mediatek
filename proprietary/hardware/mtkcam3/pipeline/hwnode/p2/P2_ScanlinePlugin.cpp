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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_ScanlinePlugin.h"
#include "P2_DebugControl.h"
#define P2_CLASS_TAG    ScanlinePlugin
#define P2_TRACE        TRACE_P2_SCANLINE_PLUGIN
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace P2
{

P2ScanlinePlugin::P2ScanlinePlugin()
{
    TRACE_FUNC_ENTER();
    mEnabled = property_get_int32("vendor.debug.mtkcam.p2.scanline", 0);
    mScanline = DebugScanLine::createInstance();
    TRACE_FUNC_EXIT();
}

P2ScanlinePlugin::~P2ScanlinePlugin()
{
    TRACE_FUNC_ENTER();
    if( mScanline )
    {
        mScanline->destroyInstance();
        mScanline = NULL;
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2ScanlinePlugin::onPlugin(const P2Img *img)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mScanline &&
        isValid(img) &&
        img->getDir() & IO_DIR_OUT )
    {
        IImageBuffer *ptr = img->getIImageBufferPtr();
        if( ptr )
        {
            if( (MUINT32)ptr->getImgSize().w == ptr->getBufStridesInBytes(0) )
            {
                mScanline->drawScanLine(ptr->getImgSize().w,
                                        ptr->getImgSize().h,
                                        (void*)(ptr->getBufVA(0)),
                                        ptr->getBufSizeInBytes(0),
                                        ptr->getBufStridesInBytes(0));
                ret = MTRUE;
            }
        }
        else
        {
            MY_LOGE("invalid ptr=NULL img=%p", img);
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ScanlinePlugin::isEnabled() const
{
    return mEnabled;
}

} // namespace
