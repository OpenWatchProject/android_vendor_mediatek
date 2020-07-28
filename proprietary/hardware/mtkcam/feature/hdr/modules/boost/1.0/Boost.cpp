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

#define DEBUG_LOG_TAG "BOOST"

#include <Boost.h>
#include <mtkcam/feature/hdr/Platform.h>

#include <mtkcam/feature/hdr/utils/Debug.h>

#ifdef USE_PERFSERVICE
#include <perfservicenative/PerfServiceNative.h>
#include <cutils/properties.h>
#endif

using namespace NSCam;
using namespace android;

// ---------------------------------------------------------------------------

class BoostImpl final : public IBoost
{
public:
    BoostImpl();
    ~BoostImpl();

    status_t enableBoostCapabilities(
            const BoostCapabilities& capabilities) override;
    status_t disableBoostCapabilities() override;

private:
    BoostCapabilities mCapabilities;
    int mSceneHandle;
};

#ifdef USE_PERFSERVICE

BoostImpl::BoostImpl()
    : mSceneHandle(-1)
{
}

BoostImpl::~BoostImpl()
{
    HDR_TRACE_CALL();

    if (mSceneHandle != -1)
    {
        PerfServiceNative_userDisable(mSceneHandle);
        PerfServiceNative_userUnreg(mSceneHandle);
    }
}

status_t BoostImpl::enableBoostCapabilities(const BoostCapabilities& capabilities)
{
    HDR_TRACE_CALL();

    const auto& index(capabilities.index);
    if (CC_UNLIKELY(index == BoostCapabilities::INVALID_INDEX))
    {
        HDR_LOGW("invalid boost index, do nothing");
        return BAD_VALUE;
    }

    mSceneHandle = PerfServiceNative_userRegScn();
    if (CC_UNLIKELY(mSceneHandle == -1))
    {
        HDR_LOGE("register PerfService scenario failed");
        return NO_INIT;
    }

    for (int i = 0; i < HDR_MAX_CLUSTER_NUM; i++)
    {
        PerfServiceNative_userRegScnConfig(
            mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, i,
            HDRPerfGearOption[index][2 * i], 0, 0);
        PerfServiceNative_userRegScnConfig(
            mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, i,
            HDRPerfGearOption[index][2 * i + 1], 0, 0);
    }

    //PerfServiceNative_userEnable(mSceneHandle);

    return OK;
}

status_t BoostImpl::disableBoostCapabilities()
{
    if (mSceneHandle == -1)
        return OK;

    PerfServiceNative_userDisable(mSceneHandle);
    PerfServiceNative_userUnreg(mSceneHandle);

    return OK;
}

#else

BoostImpl::BoostImpl()
    : mSceneHandle(-1)
{
}

BoostImpl::~BoostImpl()
{
}

status_t BoostImpl::enableBoostCapabilities(const BoostCapabilities& /*capabilities*/)
{
    return INVALID_OPERATION;
}

status_t BoostImpl::disableBoostCapabilities()
{
    return INVALID_OPERATION;
};

#endif // #ifdef USE_PERFSERVICE

// ---------------------------------------------------------------------------

IBoost* IPlatform::getBoost()
{
    return new BoostImpl;
}
