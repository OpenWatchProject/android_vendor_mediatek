/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#define  LOG_TAG "MfllCore/Exif"

#include "MfllExifInfo.h"
#include "MfllUtilities.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>

// CUSTOM (common)
#include <custom/debug_exif/dbg_exif_param.h>
// CUSTON (platform)
#include <camera_custom_nvram.h>

#include <functional> // std::function
#include <unordered_map> // std::unordered_map


#if (MFLL_MF_TAG_VERSION > 0)
// check if main MF tag version are the same or not
static_assert(
        MFLL_MF_TAG_VERSION == __namespace_mf(MFLL_MF_TAG_VERSION)::MF_DEBUG_TAG_VERSION,
        "MFLL_MF_TAG_VERSION (form makefile of MFNR) doesn't equal to " \
        "MF_DEBUG_TAG_VERSION (from custom debug_exif)."
        );

// check if sub MF tag version are the same or smaller than the version we're using
static_assert(
        MFLL_MF_TAG_SUBVERSION <= __namespace_mf(MFLL_MF_TAG_VERSION)::MF_DEBUG_TAG_SUBVERSION,
        "MFLL_MF_TAG_SUBVERSION must be smaller or equal to MF_DEBUG_TAG_SUBVERSION."
        );
#endif


using namespace mfll;

IMfllExifInfo* IMfllExifInfo::createInstance(void)
{
    return reinterpret_cast<IMfllExifInfo*>(new MfllExifInfo);
}

void IMfllExifInfo::destroyInstance(void)
{
    decStrong((void*)this);
}

// extension function for sendCommand
// the implementation should be implemented in platform-dependent part
inline const std::unordered_map<
    std::string,  // key
    std::function<enum MfllErr(IMfllExifInfo*, const std::deque<void*>&)>
    >*
getExtFunctionMap();

//-----------------------------------------------------------------------------
// platform dependent part
//-----------------------------------------------------------------------------
#include "MfllExifInfo_platform.h"

MfllExifInfo::MfllExifInfo()
{
#if (MFLL_MF_TAG_VERSION > 0)
    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
    m_dataMap[MF_TAG_VERSION] = makeDebugExifTagVersion(
            MFLL_MF_TAG_VERSION,
            MFLL_MF_TAG_SUBVERSION
            );
#endif
}

MfllExifInfo::~MfllExifInfo()
{
}

enum MfllErr MfllExifInfo::init()
{
    std::unique_lock<std::mutex> _l(m_mutex);
    return MfllErr_Ok;
}

enum MfllErr MfllExifInfo::updateInfo(unsigned int key, uint32_t value)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    m_dataMap[key] = value;
    return MfllErr_Ok;
}

uint32_t MfllExifInfo::getInfo(unsigned int key)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    return m_dataMap[key];
}

const std::map<unsigned int, uint32_t>& MfllExifInfo::getInfoMap()
{
    return m_dataMap;
}

enum MfllErr MfllExifInfo::sendCommand(
        const std::string& cmd,
        const std::deque<void*>& dataset)
{
    auto pExtFuncMap = getExtFunctionMap();
    if (pExtFuncMap == NULL)
        return MfllErr_NotImplemented;

    auto func = pExtFuncMap->at(cmd.c_str());
    if (func) {
        return func(static_cast<IMfllExifInfo*>(this), dataset);
    }
    return MfllErr_NotImplemented;
}

unsigned int MfllExifInfo::getVersion()
{
#if (MFLL_MF_TAG_VERSION > 0)
    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
    return (unsigned int)m_dataMap[MF_TAG_VERSION];
#else
    return 0;
#endif
}
