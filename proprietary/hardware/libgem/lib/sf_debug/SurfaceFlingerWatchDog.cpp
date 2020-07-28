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
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <selinux/android.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <utils/ProcessCallStack.h>

#include "sf_debug/SurfaceFlingerWatchDog.h"

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#else
#include <fcntl.h>
#endif

namespace android {


#ifdef HAVE_AEE_FEATURE
// AED Exported Functions
static int aee_ioctl_wdt_kick(int value) {
    int ret = 0;
    int fd = open(AE_WDT_DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        ALOGD("[SW-WD] ERROR: open %s failed.", AE_WDT_DEVICE_PATH);
        return 1;
    } else {
        if (ioctl(fd, AEEIOCTL_WDT_KICK_POWERKEY, (int)value) != 0) {
            ALOGD("[SW-WD] ERROR(%s): aee wdt kick powerkey ioctl failed.", strerror(errno));
            close(fd);
            return 1;
        }
    }
    close(fd);
    return ret;
}

static int aee_ioctl_swt_set(nsecs_t time) {
    int ret = 0;
    int fd = open(AE_WDT_DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        ALOGD("[SF-WD] ERROR: open %s failed.", AE_WDT_DEVICE_PATH);
        return 1;
    } else {
        if (ioctl(fd, AEEIOCTL_SET_SF_STATE, (long long)(&time)) != 0) {
            ALOGD("[SW-WD] ERROR(%s): aee swt set state ioctl failed.", strerror(errno));
            close(fd);
            return 1;
        }
    }
    close(fd);
    return ret;
}
#endif



//==================================================================================================
// RTTDumper
//
#define SF_WATCHDOG_RTTCOUNT    10
#define RTT_FOLDER_PATH         "/data/anr/SF_RTT/"
#define RTT_FILE_NAME           "rtt_dump"
#define RTT_DUMP                (RTT_FOLDER_PATH RTT_FILE_NAME)
#define RTT_IDLE                1

class RTTDumper : public Thread {
public:
    RTTDumper()
        : mLastFinishTime(0)
        , mUseProcessCallstack(1)
    {
    }

    ~RTTDumper() {}

    bool dumpRTT() {
        if (!isRunning()) {
            nsecs_t idleTime = 0;
            {
                Mutex::Autolock _l(mLastFinishTimeLock);
                idleTime = systemTime() - mLastFinishTime;
            }
            if (idleTime > s2ns(RTT_IDLE))
                return run("RTT_Dumper", PRIORITY_NORMAL) == NO_ERROR;
            else
                ALOGD("[SW-WD] RTT Dumper cool down");
        }
        return false;
    }

private:
    nsecs_t mLastFinishTime;
    int mUseProcessCallstack;
    mutable Mutex mLastFinishTimeLock;

    bool createFolder(const char* path) {
        struct stat sb;
        if (stat(path, &sb) != 0) {
            if (mkdir(path, 0777) != 0) {
                ALOGE("[SW-WD-RTT] mkdir(%s) failed: %s", path, strerror(errno));
                return false;
            }
            if (selinux_android_restorecon(path, 0) == -1) {
                ALOGE("[SW-WD-RTT] restorecon failed(%s) failed", path);
                return false;
            } else {
                ALOGV("[SW-WD-RTT] restorecon(%s)", path);
            }
        }
        return true;
    }

    void dumpProcess(int pid) {
        char cmds[256];
        sprintf(cmds, "rtt -f bt -p %d >> %s.txt", pid, RTT_DUMP);
        system(cmds);
    }

    void dumpProcess() {
        char path[PATH_MAX];
        sprintf(path, "%s%s.txt", RTT_FOLDER_PATH, RTT_FILE_NAME);
        int fd = open(path, O_CREAT|O_WRONLY|O_APPEND, 0644);
        if (fd > 0) {
            ProcessCallStack pcs;
            pcs.update();
            pcs.dump(fd);
            close(fd);
        } else {
            ALOGW("failed to open rtt file[%s]", path);
        }
    }

    bool threadLoop() {
        static uint32_t rtt_ct = SF_WATCHDOG_RTTCOUNT;
        if (rtt_ct > 0) {
            rtt_ct --;
        } else {
            ALOGD("[SW-WD-RTT] swap rtt dump file");

            // swap rtt dump file
            char cmds[256];
            snprintf(cmds, sizeof(cmds), "mv %s.txt %s_1.txt", RTT_DUMP, RTT_DUMP);
            system(cmds);

            rtt_ct = SF_WATCHDOG_RTTCOUNT;
        }

        // create rtt folder
        createFolder(RTT_FOLDER_PATH);

        char buffer[PROPERTY_VALUE_MAX];
        if (property_get("vendor.debug.sf.watchdog_callstack", buffer, NULL) > 0) {
            mUseProcessCallstack = atoi(buffer);
        }

        // append SurfaceFlinger rtt information to rtt file
        if (mUseProcessCallstack) {
            dumpProcess();
        } else {
            dumpProcess(getpid());

            // append HWC rtt information to rtt file
            property_get("vendor.debug.sf.hwc_pid", buffer, "0");
            int hwcPid = atoi(buffer);
            if (hwcPid > 0) {
                dumpProcess(hwcPid);
            }
        }

        {
            Mutex::Autolock _l(mLastFinishTimeLock);
            mLastFinishTime = systemTime();
        }
        return false;
    }
};



//==================================================================================================
// WDNotify
//
class WDNotify : public SWWatchDog::Recipient {
public:
    explicit WDNotify(SFWatchDog* wdt) :
        mSFWdt(wdt) {}

    ~WDNotify() {}

    void onSetAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg) {
        mSFWdt->onSetAnchor(id, tid, anchorTime, msg);
    }

    void onDelAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg, const nsecs_t& now) {
        mSFWdt->onDelAnchor(id, tid, anchorTime, msg, now);
    }

    void onTimeout(  const SWWatchDog::anchor_id_t& id, const pid_t& tid, const nsecs_t& anchorTime,
                     const String8& msg, const nsecs_t& now) {
        mSFWdt->onTimeout(id, tid, anchorTime, msg, now);
    }

    void onTick(const nsecs_t& now) {
        mSFWdt->onTick(now);
    }

private:
    SFWatchDog* mSFWdt;
};



//==================================================================================================
// SFWatchDog
//
SFWatchDog::SFWatchDog() :
    mShowLog(false),
    mUpdateCount(0),
    mTimer(SWWatchDog::DEFAULT_TIMER),
    mThreshold(SWWatchDog::DEFAULT_THRESHOLD),
    mTimeout(false),
    mID(SWWatchDog::NO_ANCHOR),
    mFreshAnchor(true) {
    sp<WDNotify> notify = new WDNotify(this);
    SWWatchDog::setWDTNotify(notify);
    SWWatchDog::setTickNotify(notify);

    mRTTDumper = new RTTDumper;
}

SFWatchDog::~SFWatchDog() {}

bool SFWatchDog::setAnchor(const String8& msg) {
#ifndef DISABLE_SWWDT
    if (mID != SWWatchDog::NO_ANCHOR) {
        ALOGE("[SW-WD] anchor(%#" PRIxPTR ") already exist, replacing by <<%s>>", mID, msg.string());
        return false;
    }
    {
        Mutex::Autolock _l(mFreshAnchorLock);
        mFreshAnchor = true;
    }
    mID = SWWatchDog::setAnchor(msg, mThreshold);
    return mID != SWWatchDog::NO_ANCHOR;
#else
    (void)(mID);
    (void)(msg);
    return true;
#endif
}

bool SFWatchDog::delAnchor() {
#ifndef DISABLE_SWWDT
    if (mID == SWWatchDog::NO_ANCHOR) {
        ALOGE("[SW-WD] [%s] There is no anchor.", __func__);
        return false;
    }
    anchor_id_t id = mID;
    mID = SWWatchDog::NO_ANCHOR;
    return SWWatchDog::delAnchor(id);
#else
    return true;
#endif
}

void SFWatchDog::setSuspend() {
#ifdef HAVE_AEE_FEATURE
    Mutex::Autolock _l(mAEELock);
    aee_ioctl_wdt_kick(WDT_SETBY_SF);
#endif
}

void SFWatchDog::setResume() {
#ifdef HAVE_AEE_FEATURE
    Mutex::Autolock _l(mAEELock);
    aee_ioctl_wdt_kick(WDT_SETBY_SF);
#endif
}

void SFWatchDog::setThreshold(const SWWatchDog::msecs_t& threshold) {
    if (threshold != mThreshold) {
        ALOGD("SF watch dog change threshold from %" PRId64 " --> %" PRId64 ".", mThreshold, threshold);

        // SFWatchDog::mThreshold unit is ms.
        // SWWatchDog::mThreshold unit is ns.
        // Because SFWatchDog::mThreshold hide SWWatchDog::mThreshold.
        // therefore, we should set SWWatchDog::mThreshold via SWWatchDog::setThreshold().
        mThreshold = threshold;
        SWWatchDog::setThreshold(mThreshold);
    }
}

void SFWatchDog::getProperty() {
    char value[PROPERTY_VALUE_MAX];

    if (property_get("vendor.debug.sf.wdthreshold", value, NULL) > 0) {
        nsecs_t threshold = static_cast<nsecs_t>(atoi(value));
        setThreshold(threshold);
    }

    if (property_get("vendor.debug.sf.wdtimer", value, NULL) > 0) {
        nsecs_t timer = static_cast<nsecs_t>(atoi(value));
        if (timer != mTimer) {
            ALOGD("SF watch dog change timer from %" PRId64 " --> %" PRId64 ".", mTimer, timer);
            mTimer = timer;
            setTimer(mTimer);
        }
    }

    property_get("vendor.debug.sf.wdlog", value, "0");
    mShowLog = atoi(value);
}

void SFWatchDog::onSetAnchor(const SWWatchDog::anchor_id_t& /*id*/, const pid_t& tid,
                             const nsecs_t& anchorTime, const String8& msg) {
    if (mShowLog) {
        ALOGV("[SW-WD] Set Anchor <<%s>> TID=%d, AnchorTime=%" PRId64 ".",
            msg.string(), tid, anchorTime);
    }

    ++mUpdateCount;
}

void SFWatchDog::onDelAnchor(const SWWatchDog::anchor_id_t& /*id*/, const pid_t& tid,
                             const nsecs_t& anchorTime, const String8& msg, const nsecs_t& now) {
    if (mShowLog) {
        ALOGV("[SW-WD] Delete Anchor <<%s>> TID=%d, AnchorTime=%" PRId64 " SpendTime=%" PRId64 ".",
            msg.string(), tid, anchorTime, now - anchorTime);
    }
}

void SFWatchDog::onTimeout( const SWWatchDog::anchor_id_t& /*id*/, const pid_t& tid,
                            const nsecs_t& anchorTime, const String8& msg, const nsecs_t& now) {
    String8 warningMessage;
    {
        Mutex::Autolock _l(mFreshAnchorLock);
        mFreshAnchor = false;
    }
    ALOGW("[SW-WD] TID=%d SpendTime=%" PRId64 "ms Threshold=%" PRId64
          "ms AnchorTime=%" PRId64 " Now=%" PRId64 ".",
            tid, ns2ms(now - anchorTime), getThreshold(), anchorTime, now);
    ALOGV("[SW-WD] %s", msg.string());

    if (mRTTDumper->isRunning()) {
        ALOGW("[SW-WD] RTT is still dumping");
    } else {
        if (mRTTDumper->dumpRTT()) {
            ALOGW("[SW-WD] Start to dump RTT");
        } else {
            ALOGW("[SW-WD] Start RTT Dumper fail");
        }
    }

#ifdef HAVE_AEE_FEATURE
    nsecs_t spendTime = 1;
    if (getThreshold() != 0) {
        spendTime = ns2ms(now - anchorTime);
    }
    aee_ioctl_swt_set(spendTime);
#endif
    mTimeout = true;
}

void SFWatchDog::onTick(const nsecs_t& now) {
    if (mShowLog) {
        ALOGI("[SW-WD] Tick Now=%" PRId64 ".", now);
    }

    getProperty();

#ifdef HAVE_AEE_FEATURE
    if (!mTimeout) {
        aee_ioctl_swt_set(1);
    }
#endif
    mTimeout = false;

    if (mUpdateCount) {
        if (mShowLog)
            ALOGD("[SW-WD] mUpdateCount: %d", mUpdateCount);
#ifdef HAVE_AEE_FEATURE
        {
            Mutex::Autolock _l(mAEELock);
            aee_ioctl_wdt_kick(WDT_SETBY_SF);
        }
#endif
        mUpdateCount = 0;
    }
}

// ----------------------------------------------------------------------------
SFWatchDogAPI *createInstance() {
    return new SFWatchDog;
}

};  // namespace android
