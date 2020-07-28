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
#ifndef ANDROID_SF_WATCHDOG
#define ANDROID_SF_WATCHDOG

#include <stdint.h>
#include <sys/types.h>
#include <utils/String8.h>
#include <system/window.h>

#include "ui_ext/SWWatchDog.h"
#include "sf_debug/SurfaceFlingerWatchDogAPI.h"

namespace android {

class WDNotify;
class RTTDumper;

class SFWatchDog : public SFWatchDogAPI {
public:
    SFWatchDog();
    ~SFWatchDog();

    /**
     * Hide SWWatchDog::setAnchor/delAnchor.
     * Because SFWatchDog only support to set one anchor at a time.
     * @param  msg The meesage want to pass to onTimeout.
     * @return     Success or Fail
     */
    bool setAnchor(const String8& msg);
    bool delAnchor();

    /**
     * Set Watchdog Suspend/Resume, and kick aee wdt
     */
    void setSuspend();
    void setResume();

    /**
     * Set the WatchDog timeout threshold.
     * It will only effect the anchor which create by this SWWatchDog in the future.
     * @param threshold The anchor timeout threshold, unit: ms
     */
    void setThreshold(const SWWatchDog::msecs_t& threshold);

private:
    friend class WDNotify;
    void getProperty();

    // process WDT notify
    void onSetAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg);
    void onDelAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg, const nsecs_t& now);
    void onTimeout(  const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg, const nsecs_t& now);
    void onTick(const nsecs_t& now);

    volatile bool mShowLog;
    uint32_t mUpdateCount;
    SWWatchDog::msecs_t mTimer;
    SWWatchDog::msecs_t mThreshold;
    bool mTimeout;
    anchor_id_t mID;
    mutable Mutex mAEELock;
    bool mFreshAnchor;
    mutable Mutex mFreshAnchorLock;
    sp<RTTDumper> mRTTDumper;
};

};  // namespace android

#endif  //ANDROID_SF_WATCHDOG
