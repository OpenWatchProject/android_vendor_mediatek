/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015-2016. All rights reserved.
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

// MAKE_MFNR_CORE API
// {{{
#include <mtkcam3/feature/mfnr/IMfllCore.h>
#define LOG_TAG "MfllCore/Factory"
#include <mtkcam3/feature/mfnr/MfllLog.h>
namespace mfll {
    template<int major, int minor, int featured>
    IMfllCore*  MAKE_MFNR_CORE()
    {
        mfllLogE("create un-supported MFNR Core (%d.%d.%d)",
                major, minor, featured);
        return nullptr;
    }
}
#undef LOG_TAG
// }}}

// Other factory
#include "Factory1_6.hpp"
#include "Factory2_0.hpp"

#define LOG_TAG "MfllCore/Factory"

// MFNR Core Library
#include <mtkcam3/feature/mfnr/MfllLog.h>

// MFNR Core Internal
#include <MfllUtilities.h>

// AOSP
#include <cutils/compiler.h>


namespace mfll {

IMfllCore* IMfllCore::createInstance(
        int             major   /* = 0 */,
        int             minor   /* = 0 */,
        IMfllCore::Type type    /* = IMfllCore::Type::DEFAULT */
        )
{
    IMfllCore* pCore = nullptr;

    // check if using default
    if (major <= 0)     major = MFLL_CORE_VERSION_MAJOR;
    if (minor <= 0)     minor = MFLL_CORE_VERSION_MINOR;
    int              featured = static_cast<int>(type);

    switch (MFLL_MAKE_REVISION(major, minor, featured)) {
#define _REGISTER_MFLL_CORE_(V1, V2, V3) \
    case MFLL_MAKE_REVISION(V1, V2, V3): \
        pCore = MAKE_MFNR_CORE<V1, V2, V3>(); \
        break

    //
    // Register supports MFNR version to MFNR Core Library
    //
    _REGISTER_MFLL_CORE_(1, 6, 0);
    _REGISTER_MFLL_CORE_(2, 0, 0);
    _REGISTER_MFLL_CORE_(2, 0, static_cast<int>(IMfllCore::Type::MFHR));

#undef  _REGISTER_MFLL_CORE_
    default:
        goto LB_FAIL;

    }

    mfllLogD("MFNR Build version: %d.%d.x",
            MFLL_CORE_VERSION_MAJOR,
            MFLL_CORE_VERSION_MINOR);

    mfllLogD("Create MFNR Core version: %s",
            mfll::makeRevisionString(major, minor, featured).c_str());

    return pCore;

LB_FAIL:
    mfllLogE("Cannot create core library %s",
            mfll::makeRevisionString(major, minor, featured).c_str());

    return nullptr;
}
}; // namespace mfll
