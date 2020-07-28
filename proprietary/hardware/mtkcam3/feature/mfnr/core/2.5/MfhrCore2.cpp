/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define LOG_TAG "MfhrCore2"

#include <mtkcam3/feature/mfnr/MfllLog.h>

#include "MfhrCore2.h"

// MFNR Core
#include <MfllOperationSync.h>
#include <MfllUtilities.h>
//
#include <mtkcam3/feature/mfnr/IMfllMfb.h>
#include <mtkcam3/feature/mfnr/IMfllCapturer.h>

// AOSP
#include <cutils/compiler.h>

// STL
#include <chrono>

using namespace mfll;

constexpr int32_t ver_featured   = static_cast<int32_t>(IMfllCore::Type::MFHR);


MfhrCore2::MfhrCore2() : _MFLL_CORE_CLASS_()
{
    m_spMfb = IMfllMfb::createInstance();
    if (CC_UNLIKELY(m_spMfb.get() == nullptr)) {
        mfllLogE("unexpected error, create IMfllMfb failed");
        return;
    }

    m_spCapturer = IMfllCapturer::createInstance(IMfllCapturer::Type::YuvCapturer);
    if (CC_UNLIKELY(m_spCapturer.get() == nullptr)) {
        mfllLogE("unexpected error, create IMfllCapturer failed");
        return;
    }

    setMfhrByPassOption();
}


enum MfllErr MfhrCore2::do_Init(const MfllConfig_t& cfg)
{
    mfllLogD("MfhrCore2: %s", __FUNCTION__);
#define __SET__(V, METHOD, T) if (CC_LIKELY(V.get())) V->METHOD(T)
    // set mode to capturer, mfb
    __SET__(m_spMfb,        setShotMode, cfg.mfll_mode);
    __SET__(m_spCapturer,   setShotMode, cfg.mfll_mode);
    // post_nr_type
    __SET__(m_spMfb,        setPostNrType, cfg.post_nr_type);
    __SET__(m_spCapturer,   setPostNrType, cfg.post_nr_type);
    // set core
    __SET__(m_spMfb,        setMfllCore, this);
    __SET__(m_spCapturer,   setMfllCore, this);
#undef  __SET__
    if (CC_LIKELY(m_spMfb.get()))
        m_spMfb->init(cfg.sensor_id);
    else
        mfllLogE("IMfllMfb is null");

    return _MFLL_CORE_CLASS_::do_Init(cfg);
}


enum MfllErr MfhrCore2::setBypassOption(
        const MfllBypassOption_t& b)
{
    auto err = _MFLL_CORE_CLASS_::setBypassOption(b);
    if ( CC_LIKELY(err == MfllErr_Ok) ) {
        err = setMfhrByPassOption();
    }
    return err;
}


unsigned int MfhrCore2::getVersion()
{
    auto ver = _MFLL_CORE_CLASS_::getVersion();
    ver &= 0xFFFFFC00;
    ver |= ver_featured;
    return ver;
}


std::string MfhrCore2::getVersionString()
{
    auto ver = getVersion();
    return mfll::makeRevisionString(
            MFLL_MAJOR_VER(ver),
            MFLL_MINOR_VER(ver),
            MFLL_FEATURE_VER(ver)
            );
}


MfllErr MfhrCore2::setMfhrByPassOption()
{
    m_bypass.bypassBss = 1;
    for (size_t i = 0; i < MFLL_MAX_FRAMES; i++) {
        m_bypass.bypassAllocRawBuffer[i] = 1;
        m_bypass.bypassAllocQyuvBuffer[i] = 1;
        m_bypass.bypassAllocRrzoBuffer[i] = 1;
    }
    return MfllErr_Ok;
}
