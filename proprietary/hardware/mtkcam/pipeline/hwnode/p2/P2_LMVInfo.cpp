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
#define P2_TRACE        TRACE_P2_LMV_INFO
#include "P2_LogHeader.h"

namespace P2
{

LMVInfo::LMVInfo()
    : is_valid(MFALSE)
    , x_int(0)
    , x_float(0)
    , y_int(0)
    , y_float(0)
    , s(MSize(0, 0))
    , x_mv_int(0)
    , x_mv_float(0)
    , y_mv_int(0)
    , y_mv_float(0)
    , is_from_zzr(0)
    , gmvX(0)
    , gmvY(0)
    , gmvMax(0)
    , confX(0)
    , confY(0)
    , expTime(0)
    , ihwTS(0)
    , ilwTS(0)
    , ts(0)
    , isFrontBin(MFALSE)
{
}

LMVInfo::LMVInfo(const IMetadata *meta, const Logger &logger)
    : is_valid(MFALSE)
    , x_int(0)
    , x_float(0)
    , y_int(0)
    , y_float(0)
    , s(MSize(0, 0))
    , x_mv_int(0)
    , x_mv_float(0)
    , y_mv_int(0)
    , y_mv_float(0)
    , is_from_zzr(0)
    , gmvX(0)
    , gmvY(0)
    , gmvMax(0)
    , confX(0)
    , confY(0)
    , expTime(0)
    , ihwTS(0)
    , ilwTS(0)
    , ts(0)
    , isFrontBin(MFALSE)
{
    IMetadata::IEntry entry;
    if( meta == NULL )
    {
        MY_S_LOGW(logger, "invalid metadata = NULL");
    }
    else
    {
        entry = meta->entryFor(MTK_EIS_REGION);
        if( entry.count() <= EIS_REGION_INDEX_HEIGHT &&
            logger->getLogLevel() >= 1 )
        {
            MY_S_LOGD(logger, "meta=%p size=%d no MTK_EIS_REGION count=%d", meta, meta->count(), entry.count());
        }
    }
    auto t32 = Type2Type<MINT32>();
    auto tbool = Type2Type<MBOOL>();
    if( entry.count() > EIS_REGION_INDEX_HEIGHT )
    {
        this->is_valid      = MTRUE;
        this->x_int         = entry.itemAt(EIS_REGION_INDEX_XINT, t32);
        this->x_float       = entry.itemAt(EIS_REGION_INDEX_XFLOAT, t32);
        this->y_int         = entry.itemAt(EIS_REGION_INDEX_YINT, t32);
        this->y_float       = entry.itemAt(EIS_REGION_INDEX_YFLOAT, t32);
        this->s.w           = entry.itemAt(EIS_REGION_INDEX_WIDTH, t32);
        this->s.h           = entry.itemAt(EIS_REGION_INDEX_HEIGHT, t32);
    }
    if( entry.count() > EIS_REGION_INDEX_ISFROMRZ )
    {
        MINT32 xmv          = entry.itemAt(EIS_REGION_INDEX_MV2CENTERX, t32);
        MINT32 ymv          = entry.itemAt(EIS_REGION_INDEX_MV2CENTERY, t32);
        this->is_from_zzr   = entry.itemAt(EIS_REGION_INDEX_ISFROMRZ, t32);
        MBOOL xmv_neg = xmv >> 31;
        MBOOL ymv_neg = ymv >> 31;
        if( xmv_neg ) xmv = ~xmv + 1;
        if( ymv_neg ) ymv = ~ymv + 1;
        this->x_mv_int = (xmv & (~0xFF)) >> 8;
        this->x_mv_float = (xmv & (0xFF)) << 31;
        if( xmv_neg ) this->x_mv_int = ~this->x_mv_int + 1;
        if( xmv_neg ) this->x_mv_float = ~this->x_mv_float + 1;
        this->y_mv_int = (ymv & (~0xFF)) >> 8;
        this->y_mv_float = (ymv & (0xFF)) << 31;
        if( ymv_neg ) this->y_mv_int = ~this->y_mv_int + 1;
        if( ymv_neg ) this->y_mv_float = ~this->y_mv_float + 1;
    }
    if( entry.count() > EIS_REGION_INDEX_GMVY )
    {
        this->gmvX          = entry.itemAt(EIS_REGION_INDEX_GMVX, t32);
        this->gmvY          = entry.itemAt(EIS_REGION_INDEX_GMVY, t32);
    }
    if( entry.count() > EIS_REGION_INDEX_LWTS )
    {
        this->confX         = entry.itemAt(EIS_REGION_INDEX_CONFX, t32);
        this->confY         = entry.itemAt(EIS_REGION_INDEX_CONFY, t32);
        this->expTime       = entry.itemAt(EIS_REGION_INDEX_EXPTIME, t32);
        this->ihwTS         = entry.itemAt(EIS_REGION_INDEX_HWTS, t32);
        this->ilwTS         = entry.itemAt(EIS_REGION_INDEX_LWTS, t32);
        this->ts = ((MINT64)(ihwTS&0xFFFFFFFF)) << 32;
        this->ts += (MINT64)(ilwTS&0xFFFFFFFF);
    }
    if( entry.count() > EIS_REGION_INDEX_ISFRONTBIN )
    {
        this->isFrontBin    = entry.itemAt(EIS_REGION_INDEX_ISFRONTBIN, tbool);
    }
    if( entry.count() > EIS_REGION_INDEX_MAX_GMV )
    {
        this->gmvMax        = entry.itemAt(EIS_REGION_INDEX_MAX_GMV, t32);
    }
}

} // namespace P2
