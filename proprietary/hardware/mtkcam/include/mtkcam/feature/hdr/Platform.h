/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>

#include <mtkcam/utils/module/module.h>

#include <unistd.h>

// ---------------------------------------------------------------------------

namespace NSCam {

// ---------------------------------------------------------------------------

class IPerf
{
public:
    virtual ~IPerf() = default;

    virtual int enableAffinity(pid_t pid) = 0;
    virtual int disableAffinity(pid_t pid) = 0;
};

class IBoost
{
public:
    virtual ~IBoost() = default;

    struct BoostCapabilities {
        enum { INVALID_INDEX = -1 };
        BoostCapabilities(int _index = INVALID_INDEX)
            : index(_index) {};

        int index;
    };

    virtual android::status_t enableBoostCapabilities(
            const BoostCapabilities& capabilities) = 0;
    virtual android::status_t disableBoostCapabilities() = 0;
};

// ---------------------------------------------------------------------------

class HDRPerfFactory
{
public:
    virtual ~HDRPerfFactory() = default;

    template<typename... Args>
    static IPerf* getIPerf(const Args&... /*args*/)
    {
        // Typedefs of function pointer that generate IPerf instance(s)
        typedef IPerf* (*IPerf_factory_t)();
        return MAKE_MTKCAM_MODULE(
                MTKCAM_MODULE_ID_HDR_PERF, IPerf_factory_t/*, args...*/);
    }
};

// ---------------------------------------------------------------------------

class HDRBoostFactory
{
public:
    virtual ~HDRBoostFactory() = default;

    template<typename... Args>
    static IBoost* getIBoost(const Args&... /*args*/)
    {
        // Typedefs of function pointer that generate IBoost instance(s)
        typedef IBoost* (*IBoost_factory_t)();
        return MAKE_MTKCAM_MODULE(
                MTKCAM_MODULE_ID_HDR_BOOST, IBoost_factory_t/*, args...*/);
    }
};

// ---------------------------------------------------------------------------

class IPlatform
{
public:
    virtual ~IPlatform() = default;

    // getPerf() is used to enforce the computing power by
    // applying basic boost scenario
    static IPerf* getPerf();

    // getBoost() is used to enforce the computing power by
    // applying advanced boost scenario
    static IBoost* getBoost();
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

#endif // _PLATFORM_H_
