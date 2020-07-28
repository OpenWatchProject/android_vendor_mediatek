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
#ifndef __MFLLEXIFINFO_PLATFORM_H__
#define __MFLLEXIFINFO_PLATFORM_H__

#include <mtkcam3/feature/mfnr/IMfllGyro.h>

#include <cstdint>

#include <custom/debug_exif/dbg_exif_param.h>

#if ((MFLL_MF_TAG_VERSION > 0) && (MFLL_MF_TAG_VERSION != 1))
#   error "MT6761 only supports MF_TAG_VERSION 1"
#endif


enum MfllErr MfllExifInfo::updateInfo(IMfllNvram *pNvram)
{
    //TODO
    return MfllErr_Ok;
}

enum MfllErr MfllExifInfo::updateInfo(const MfllCoreDbgInfo_t &dbgInfo __attribute__((unused)))
{
#if (MFLL_MF_TAG_VERSION > 0)
    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);

    std::unique_lock<std::mutex> _l(m_mutex);
    m_dataMap[MF_TAG_MAX_FRAME_NUMBER] = dbgInfo.frameCapture;
    m_dataMap[MF_TAG_PROCESSING_NUMBER] = dbgInfo.frameBlend;

    /* iso & exposure */
    m_dataMap[MF_TAG_EXPOSURE] = dbgInfo.exp;
    m_dataMap[MF_TAG_ISO] = dbgInfo.iso;

    m_dataMap[MF_TAG_RAW_WIDTH] = dbgInfo.width;
    m_dataMap[MF_TAG_RAW_HEIGHT] = dbgInfo.height;
    m_dataMap[MF_TAG_BSS_ENABLE] = dbgInfo.bss_enable;
    m_dataMap[MF_TAG_MEMC_SKIP] = dbgInfo.memc_skip;
    m_dataMap[MF_TAG_MFB_MODE] = dbgInfo.shot_mode;

    m_dataMap[MF_TAG_AIS_EXPOSURE] = dbgInfo.ori_exp;
    m_dataMap[MF_TAG_AIS_ISO] = dbgInfo.ori_iso;
    m_dataMap[MF_TAG_AIS_CALCULATED_EXPOSURE] = dbgInfo.exp;
    m_dataMap[MF_TAG_AIS_CALCULATED_ISO] = dbgInfo.iso;
#endif
    return MfllErr_Ok;
}


// ----------------------------------------------------------------------------
// extensions, for sendCommand(const std::string&, const std::deque<void*>&)
//
const std::unordered_map<std::string, std::function<enum MfllErr(IMfllExifInfo*, const std::deque<void*>&)>>*
getExtFunctionMap()
{
    static std::unordered_map<std::string, std::function<enum MfllErr(IMfllExifInfo*, const std::deque<void*>&)>>
        sExtFunctions;

    // TODO: describes here

    return &sExtFunctions;
}

#endif//__MFLLEXIFINFO_PLATFORM_H__
