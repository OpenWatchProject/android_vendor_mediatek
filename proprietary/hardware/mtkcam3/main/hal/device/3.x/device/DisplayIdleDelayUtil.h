/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef DISPLAY_IDLE_DELAY_UTIL_H_
#define DISPLAY_IDLE_DELAY_UTIL_H_

#include <mtkcam/def/common.h>
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/ULog.h>

using namespace vendor::mediatek::hardware::power::V2_0;
using namespace android;

namespace NSCam {
namespace v3 {

/******************************************************************************
 *
 ******************************************************************************/
class DisplayIdleDelayUtil : public android::hardware::hidl_death_recipient
{
public:
    DisplayIdleDelayUtil()
    {
        _init();
    }

    virtual ~DisplayIdleDelayUtil()
    {
        disable();
        _powerHalService = NULL;
    }

    virtual bool enable()
    {
        Mutex::Autolock lock(_Lock);
        bool ret = true;
        _init();
        if(_powerHalService)
        {
            const int DISPLAY_IDLE_DELAY_DEFAULT = 100;
            int displayIdleDelay = ::property_get_int32("vendor.cam3dev.displayidledelay", DISPLAY_IDLE_DELAY_DEFAULT);
            if(displayIdleDelay < 0) {
                displayIdleDelay = DISPLAY_IDLE_DELAY_DEFAULT;
            }

            if(-1 == _powerHalHandle) {
                _powerHalHandle = _powerHalService->scnReg();
            }

            if (-1 == _powerHalHandle) {
                CAM_ULOGE(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Power Hal Service is not ready");
            } else {
                CAM_ULOGD(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Set display idle delay to %d, handle %d", displayIdleDelay, _powerHalHandle);
                _powerHalService->scnConfig(_powerHalHandle, MtkPowerCmd::CMD_SET_DISP_IDLE_TIME, displayIdleDelay, 0, 0, 0);
                _powerHalService->scnEnable(_powerHalHandle, 0);
            }
        }
        else
        {
            CAM_ULOGW(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] _powerHalService is NULL");
            ret = false;
        }

        return ret;
    }

    virtual void disable()
    {
        Mutex::Autolock lock(_Lock);
        if(_powerHalService)
        {
            CAM_ULOGD(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Reset display idle delay, handle: %d", _powerHalHandle);
            _powerHalService->scnDisable(_powerHalHandle);
            _powerHalService->scnUnreg(_powerHalHandle);
            _powerHalService->unlinkToDeath(this);
            _powerHalHandle = -1;
        }
    }

    virtual void serviceDied(uint64_t cookie, const android::wp<android::hidl::base::V1_0::IBase>& who) override
    {
        CAM_ULOGW(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] IPower hidl service died");
        Mutex::Autolock lock(_Lock);
        _powerHalService = nullptr;
        _powerHalHandle  = -1;
    }

protected:
    void _init()
    {
        if (_powerHalService == nullptr)
        {
            CAM_ULOGD(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Loaded power HAL service +");
            _powerHalService = IPower::getService();
            CAM_ULOGD(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Loaded power HAL service -");
            if (_powerHalService != nullptr) {
                //Work around for IPower::linkToDeath
                this->incStrong(this);
                android::hardware::Return<bool> linked = _powerHalService->linkToDeath(this, 0);
                if (!linked.isOk()) {
                    CAM_ULOGE(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Transaction error in linking to PowerHAL death: %s",
                    linked.description().c_str());
                } else if (!linked) {
                    CAM_ULOGW(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Unable to link to PowerHAL death notifications");
                } else {
                   // CAM_ULOGD(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Link to death notification successful");
                }
            } else {
                CAM_ULOGW(NSCam::Utils::ULog::MOD_CAMERA_DEVICE, "[DisplayIdleDelayUtil] Couldn't load power HAL service");
            }
        }
    }

protected:
    mutable Mutex           _Lock;
    MINT32                  _powerHalHandle = -1;
    sp<IPower>              _powerHalService;
};

/******************************************************************************
 *
 ******************************************************************************/
};
};
#endif  //DISPLAY_IDLE_DELAY_UTIL_H_
