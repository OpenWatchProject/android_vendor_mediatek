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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/******************************************************************************
 *
 ******************************************************************************/

inline MBOOL CHECK_FMT_UF(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
        case eImgFmt_UFO_FG_BAYER8:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER14:
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}

inline MINT _MapUFtoB(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_UFO_BAYER8:
        ret = eImgFmt_BAYER8;
        break;
    case eImgFmt_UFO_BAYER10:
        ret = eImgFmt_BAYER10;
        break;
    case eImgFmt_UFO_BAYER12:
        ret = eImgFmt_BAYER12;
        break;
    case eImgFmt_UFO_BAYER14:
        ret = eImgFmt_BAYER14;
        break;
    case eImgFmt_UFO_FG_BAYER8:
        ret = eImgFmt_FG_BAYER8;
        break;
    case eImgFmt_UFO_FG_BAYER10:
        ret = eImgFmt_FG_BAYER10;
        break;
    case eImgFmt_UFO_FG_BAYER12:
        ret = eImgFmt_FG_BAYER12;
        break;
    case eImgFmt_UFO_FG_BAYER14:
        ret = eImgFmt_FG_BAYER14;
        break;
    default:
        PIPE_ERR("unsupported uf fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_PAK(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
    case eImgFmt_BAYER8:
    case eImgFmt_BAYER10:
    case eImgFmt_BAYER12:
    case eImgFmt_BAYER14:
    case eImgFmt_BAYER16_APPLY_LSC:
    case eImgFmt_BAYER8_UNPAK:
    case eImgFmt_BAYER10_UNPAK:
    case eImgFmt_BAYER12_UNPAK:
    case eImgFmt_BAYER14_UNPAK:
        break;
    default:
        ret = MFALSE;
        break;
    }
    return ret;
}

inline MINT _MapPAKtoB(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_BAYER8:
        ret = eImgFmt_BAYER8_UNPAK;
        break;
    case eImgFmt_BAYER10:
        ret = eImgFmt_BAYER10_UNPAK;
        break;
    case eImgFmt_BAYER12:
        ret = eImgFmt_BAYER12_UNPAK;
        break;
    case eImgFmt_BAYER14:
        ret = eImgFmt_BAYER14_UNPAK;
        break;
    case eImgFmt_BAYER8_UNPAK:
        ret = eImgFmt_BAYER8;
        break;
    case eImgFmt_BAYER10_UNPAK:
        ret = eImgFmt_BAYER10;
        break;
    case eImgFmt_BAYER12_UNPAK:
        ret = eImgFmt_BAYER12;
        break;
    case eImgFmt_BAYER14_UNPAK:
        ret = eImgFmt_BAYER14;
        break;
    case eImgFmt_BAYER16_APPLY_LSC:
        ret = eImgFmt_BAYER16_APPLY_LSC;
        break;
    default:
        PIPE_ERR("unsupported PAK fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_UF_UNPAK(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}

inline MINT _MapUFtoUnPak(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_UFO_BAYER8:
        ret = eImgFmt_BAYER8_UNPAK;
        break;
    case eImgFmt_UFO_BAYER10:
        ret = eImgFmt_BAYER10_UNPAK;
        break;
    case eImgFmt_UFO_BAYER12:
        ret = eImgFmt_BAYER12_UNPAK;
        break;
    case eImgFmt_UFO_BAYER14:
        ret = eImgFmt_BAYER14_UNPAK;
        break;
    default:
        PIPE_ERR("unsupported uf fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_MISMATCH(MINT CfgFmt, MINT EnqFmt)
{
    MBOOL ret = MFALSE;

    if (CfgFmt == EnqFmt) {
        ret = MTRUE;
    }
    else {
        ret = (((CHECK_FMT_UF(CfgFmt)) 	  && (_MapUFtoB(CfgFmt) 	== EnqFmt)) ||
                ((CHECK_FMT_PAK(CfgFmt))	  && (_MapPAKtoB(CfgFmt)	== EnqFmt)) ||
                ((CHECK_FMT_UF_UNPAK(CfgFmt)) && (_MapUFtoUnPak(CfgFmt) == EnqFmt)))
                ? (MTRUE) : (MFALSE);
    }

    return ret;
}

inline MVOID TIME_SPEC_CAL(struct timespec &ts, MUINT32 &u4TimeoutMs)
{
    MUINT32 delta_s = u4TimeoutMs / 1000;
    MUINT32 delta_ns = (u4TimeoutMs - delta_s * 1000) * 1000000;
    MUINT32 const NPS = 1000000000; // ns per sec

    if ((ts.tv_nsec + delta_ns) >= NPS) { // 2*10^9=0x77359400 < 32bit
        MUINT32 dds = (delta_ns + ts.tv_nsec) / NPS;

        ts.tv_sec = (ts.tv_sec + delta_s + dds);
        ts.tv_nsec = (ts.tv_nsec + delta_ns) - dds * NPS;
    }
    else {
        ts.tv_sec = ts.tv_sec + delta_s;
        ts.tv_nsec = ts.tv_nsec + delta_ns;
    }
}

inline MUINTPTR ALIGN_CAL(MUINTPTR data, MUINT32 align_num)
{
    MUINT32 ShiftBit = 0;

    switch(align_num) {
        case 16:
            ShiftBit = 4;
            break;
        default:
            PIPE_ERR("unknow align num(%d)", align_num);
            break;
    }

    data = ((data + align_num - 1) >> ShiftBit) << ShiftBit;

    return data;
}

inline MINT64 getTimeStamp_ns(MUINT64 const i4TimeStamp_sec, MUINT32 const i4TimeStamp_us)
{
    MINT64 Stamp_ns = 0;
    Stamp_ns = i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    return Stamp_ns;
}

