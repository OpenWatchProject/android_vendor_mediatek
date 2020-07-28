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

#include "P2_DumpPlugin.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2DumpPlugin
#define P2_TRACE        TRACE_P2_DUMP_PLUGIN
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

#define STR_DUMP_ENABLE   "vendor.debug.p2f.dump.enable"
#define STR_DUMP_MODE     "vendor.debug.p2f.dump.mode"
#define STR_DUMP_START    "vendor.debug.p2f.dump.start"
#define STR_DUMP_COUNT    "vendor.debug.p2f.dump.count"
#define STR_DUMP_IN_MASK  "vendor.debug.p2f.dump.in"
#define STR_DUMP_OUT_MASK "vendor.debug.p2f.dump.out"
#define STR_DUMP_NDD_MASK "vendor.debug.camera.preview.dump"
#define STR_DUMP_CONTINUE "vendor.debug.camera.continue.dump"


namespace P2
{

P2DumpPlugin::P2DumpPlugin()
{
    MBOOL enable = property_get_int32(STR_DUMP_ENABLE, 0);
    MUINT32 mode = property_get_int32(STR_DUMP_MODE, P2_DUMP_DEBUG);
    if( enable
        || (P2DumpType)mode == P2_DUMP_PER_FRAME )
    {
        mMode = (P2DumpType)mode;
    }
    if( mMode != P2_DUMP_NONE )
    {
        mStart = property_get_int32(STR_DUMP_START, 0);
        mCount = property_get_int32(STR_DUMP_COUNT, 0);
        mInMask = property_get_int32(STR_DUMP_IN_MASK, ~0);
        mOutMask = property_get_int32(STR_DUMP_OUT_MASK, ~0);
    }
    MY_LOGI("mode/start/count(%d/%d/%d) mask: in/out(0x%x/0x%x)",
            mMode, mStart, mCount, mInMask, mOutMask);
}

P2DumpPlugin::~P2DumpPlugin()
{
}

MBOOL P2DumpPlugin::onPlugin(const P2Img *img)
{
    MBOOL ret = MFALSE;
    if(needDump(img))
    {
        if( isNddMode() )
        {
            img->dumpNddBuffer();
        }
        else
        {
            img->dumpBuffer();
        }
        ret = MTRUE;
    }
    return ret;
}

MBOOL P2DumpPlugin::isNddMode() const
{
    return mMode == P2_DUMP_NDD;
}

MBOOL P2DumpPlugin::isDebugMode() const
{
    return mMode == P2_DUMP_DEBUG;
}

MBOOL P2DumpPlugin::isFrameMode() const
{
    return mMode == P2_DUMP_PER_FRAME;
}

#ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM

P2DumpType P2DumpPlugin::determineP2DumpType(MINT32 frameNo)
{
    P2DumpType type = P2_DUMP_NONE;
    if( isNddMode() )
    {
        if( property_get_int32(STR_DUMP_NDD_MASK, 0) > 0 || isFrameDump(frameNo))
        {
            type = P2_DUMP_NDD;
        }
    }
    else if( isDebugMode() )
    {
        if(isFrameDump(frameNo))
        {
            type = P2_DUMP_DEBUG;
        }
    }
    else if( isFrameMode() )
    {
        if( property_get_int32(STR_DUMP_ENABLE, 0) )
        {
            type = P2_DUMP_PER_FRAME;
        }
    }
    return type;
}
#endif

P2DumpType P2DumpPlugin::needDumpFrame(MINT32 frameNo)
{
#ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM
    P2DumpType type = P2_DUMP_NONE;
    MINT32 noskipframe = property_get_int32("vendor.debug.camera.noskip.frame", 0);
    if (noskipframe)
    {
        type = determineP2DumpType(frameNo);
    }
    else
    {
        if ((frameNo != 0) && (frameNo %10 == 0))
            type = determineP2DumpType(frameNo);
    }
    return type;
#else
    P2DumpType type = P2_DUMP_NONE;
    if( isNddMode() )
    {
        if( property_get_int32(STR_DUMP_NDD_MASK, 0) > 0 || isFrameDump(frameNo))
        {
            type = P2_DUMP_NDD;
        }
    }
    else if( isDebugMode() )
    {
        if(isFrameDump(frameNo))
        {
            type = P2_DUMP_DEBUG;
        }
    }
    else if( isFrameMode() )
    {
        if( property_get_int32(STR_DUMP_ENABLE, 0) )
        {
            type = P2_DUMP_PER_FRAME;
        }
    }
    return type;
#endif
}

MBOOL P2DumpPlugin::isFrameDump(MINT32 frameNo)
{
    MINT32 continueDump = property_get_int32(STR_DUMP_CONTINUE, 0);
    if(continueDump != mPreContinueDump && continueDump > 0)
    {
        mCount = continueDump;
        mStart = frameNo;
    }
    mPreContinueDump = continueDump;

    return ( (mStart < 0) || ((frameNo >= mStart) && ((MUINT32)(frameNo - mStart) < mCount)) );
}

MBOOL P2DumpPlugin::needDumpIn(DUMP_IN mask) const
{
    return (mInMask & (1<<mask));
}

MBOOL P2DumpPlugin::needDumpOut(DUMP_OUT mask) const
{
    return (mOutMask & (1<<mask));
}

MBOOL P2DumpPlugin::needDump(const P2Img * img) const
{
    MBOOL ret = MFALSE;
    if( img && img->isValid() )
    {
        ID_IMG mirrorID = img->getMirrorID();
        switch(mirrorID)
        {
            case IN_FULL:
                ret = needDumpIn(DUMP_IN_IMGO);
                break;
            case IN_RESIZED:
                ret = needDumpIn(DUMP_IN_RRZO);
                break;
            case IN_LCSO:
                ret = needDumpIn(DUMP_IN_LCSO);
                break;
            case OUT_YUV:
                {
                    if(img->isDisplay())
                    {
                        ret = needDumpOut(DUMP_OUT_DISPLAY);
                    }
                    else if( img->isRecord())
                    {
                        ret = needDumpOut(DUMP_OUT_RECORD);
                    }
                    else
                    {
                        ret = needDumpOut(DUMP_OUT_PREVIEWCB);
                    }
                }
                break;
            default:
                break;
        };
    }
    return ret;

}

} // namespace
