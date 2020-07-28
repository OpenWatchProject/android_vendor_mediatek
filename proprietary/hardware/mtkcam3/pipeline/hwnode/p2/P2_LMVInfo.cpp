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

#include "P2_LMVInfo.h"
#include "P2_DebugControl.h"
#define P2_CLASS_TAG    LMVInfo
#define P2_TRACE        TRACE_LMV_INFO
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace P2 {

LMVInfo extractLMVInfo(const ILog &log, const IMetadata *halMeta)
{
    TRACE_S_FUNC_ENTER(log);

    LMVInfo lmvInfo;
    IMetadata::IEntry entry;
    IMetadata::IEntry validityEntry;

    if( halMeta == NULL )
    {
        MY_S_LOGW(log, "invalid metadata = NULL");
    }
    else
    {
        entry = halMeta->entryFor(MTK_EIS_REGION);
        if( entry.count() <= LMV_REGION_INDEX_HEIGHT &&
            log.getLogLevel() >= 1 )
        {
            MY_S_LOGD(log, "meta=%p size=%d no MTK_EIS_REGION count=%d", halMeta, halMeta->count(), entry.count());
        }
        validityEntry = halMeta->entryFor(MTK_LMV_VALIDITY);
    }

    auto t32 = Type2Type<MINT32>();

    if( validityEntry.count() > 0 )
    {
        lmvInfo.is_valid = (validityEntry.itemAt(0, t32) == 1) ? MTRUE : MFALSE;
    }

    if( entry.count() > LMV_REGION_INDEX_HEIGHT )
    {
        lmvInfo.x_int       = entry.itemAt(LMV_REGION_INDEX_XINT, t32);
        lmvInfo.x_float     = entry.itemAt(LMV_REGION_INDEX_XFLOAT, t32);
        lmvInfo.y_int       = entry.itemAt(LMV_REGION_INDEX_YINT, t32);
        lmvInfo.y_float     = entry.itemAt(LMV_REGION_INDEX_YFLOAT, t32);
        lmvInfo.s.w         = entry.itemAt(LMV_REGION_INDEX_WIDTH, t32);
        lmvInfo.s.h         = entry.itemAt(LMV_REGION_INDEX_HEIGHT, t32);
    }
    if( entry.count() > LMV_REGION_INDEX_ISFROMRZ )
    {
        MINT32 xmv          = entry.itemAt(LMV_REGION_INDEX_MV2CENTERX, t32);
        MINT32 ymv          = entry.itemAt(LMV_REGION_INDEX_MV2CENTERY, t32);
        lmvInfo.is_from_zzr = entry.itemAt(LMV_REGION_INDEX_ISFROMRZ, t32);
        MBOOL xmv_neg = xmv >> 31;
        MBOOL ymv_neg = ymv >> 31;
        if( xmv_neg ) xmv = ~static_cast<MUINT32>(xmv) + 1;
        if( ymv_neg ) ymv = ~static_cast<MUINT32>(ymv) + 1;
        lmvInfo.x_mv_int    = (xmv & (~0xFF)) >> 8;
        lmvInfo.x_mv_float  = (xmv & (0xFF)) << 31;
        if( xmv_neg ) lmvInfo.x_mv_int = ~lmvInfo.x_mv_int + 1;
        if( xmv_neg ) lmvInfo.x_mv_float = ~lmvInfo.x_mv_float + 1;
        lmvInfo.y_mv_int    = (ymv & (~0xFF)) >> 8;
        lmvInfo.y_mv_float  = (ymv & (0xFF)) << 31;
        if( ymv_neg ) lmvInfo.y_mv_int = ~lmvInfo.y_mv_int + 1;
        if( ymv_neg ) lmvInfo.y_mv_float = ~lmvInfo.y_mv_float + 1;
    }
    if( entry.count() > LMV_REGION_INDEX_GMVY )
    {
        lmvInfo.gmvX        = entry.itemAt(LMV_REGION_INDEX_GMVX, t32);
        lmvInfo.gmvY        = entry.itemAt(LMV_REGION_INDEX_GMVY, t32);
    }
    if( entry.count() > LMV_REGION_INDEX_LWTS )
    {
        lmvInfo.confX       = entry.itemAt(LMV_REGION_INDEX_CONFX, t32);
        lmvInfo.confY       = entry.itemAt(LMV_REGION_INDEX_CONFY, t32);
        lmvInfo.expTime     = entry.itemAt(LMV_REGION_INDEX_EXPTIME, t32);
        lmvInfo.ihwTS       = entry.itemAt(LMV_REGION_INDEX_HWTS, t32);
        lmvInfo.ilwTS       = entry.itemAt(LMV_REGION_INDEX_LWTS, t32);
        lmvInfo.ts          = ((MINT64)(lmvInfo.ihwTS&0xFFFFFFFF)) << 32;
        lmvInfo.ts          += (MINT64)(lmvInfo.ilwTS&0xFFFFFFFF);
    }
    if( entry.count() > LMV_REGION_INDEX_ISFRONTBIN )
    {
        lmvInfo.isFrontBin  = (entry.itemAt(LMV_REGION_INDEX_ISFRONTBIN, t32) == 1) ? MTRUE : MFALSE;
    }
    if( entry.count() > LMV_REGION_INDEX_MAX_GMV )
    {
        lmvInfo.gmvMax      = entry.itemAt(LMV_REGION_INDEX_MAX_GMV, t32);
    }

    TRACE_S_FUNC(log,
                 "is_valid(%d),x_int(%d),x_float(%d),y_int(%d),y_float(%d),s(%dx%d),"
                 "x_mv_int(%d),x_mv_float(%d),y_mv_int(%d),y_mv_float(%d),is_from_zzr(%d),"
                 "gmvX(%d),gmvY(%d),gmvMax(%d),"
                 "confX(%d),confY(%d),expTime(%d),ihwTS(%d),ilwTS(%d),ts(%" PRId64 "),isFrontBin(%d)",
                 lmvInfo.is_valid, lmvInfo.x_int, lmvInfo.x_float, lmvInfo.y_int, lmvInfo.y_float, lmvInfo.s.w, lmvInfo.s.h,
                 lmvInfo.x_mv_int, lmvInfo.x_mv_float, lmvInfo.y_mv_int, lmvInfo.y_mv_float, lmvInfo.is_from_zzr,
                 lmvInfo.gmvX, lmvInfo.gmvY, lmvInfo.gmvMax,
                 lmvInfo.confX, lmvInfo.confY, lmvInfo.expTime, lmvInfo.ihwTS, lmvInfo.ilwTS, lmvInfo.ts, lmvInfo.isFrontBin);

    TRACE_S_FUNC_EXIT(log);
    return lmvInfo;
}

} // namespace P2
