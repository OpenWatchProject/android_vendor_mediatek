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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil/SecureTimer"
#include <utils/Log.h>

#include <drm/drm_framework_common.h>
#include <drm/SecureTimer.h>
#include <SecureTimerHelper.h>
#include <DrmTypeDef.h>
#include <DrmUtil.h>

#include <utils/String8.h>
#include <utils/threads.h>
#include <cutils/properties.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

using namespace android;

// read from this file to get the interval (in seconds) since last system boot-up.
#define SYS_UP_TIME_FILE "/proc/uptime"

// clock drift interval is set to 20 days.
// avoid possible drift of HW clock
// do SNTP time synchronization each 20 days.
#define CLK_DFT_INTERVAL (86400 * 20)

#define MAX_REAL_TIME 0x7fffffff
#define MIN_REAL_TIME 0x00000000

// constants
const static String8 ST_LOCATION("/data/drm/");
const static String8 ST_DIR("/data/drm/st/");
const static String8 ST_FILE("/data/drm/st/st.dat");

// singleton pattern implementation, initialize {m_pTimer}
SecureTimer* SecureTimer::m_pTimer = new SecureTimer();

// lock for load() and save() operation
Mutex SecureTimer::mLock;

SecureTimer& SecureTimer::instance(void)
{
    // WARNING: the race condition is not considered here!
    return *m_pTimer;
}

SecureTimer::SecureTimer(void) :
    m_bIsValid(false),
    m_nOffset(0),
    m_nBaseTicks(0),
    m_nBaseTime(0),
    m_nLastSync(0),
    m_nLastSave(0)
{
    updateTimeBase();
}

SecureTimer::~SecureTimer(void)
{
}

bool SecureTimer::isValid()
{
    return m_bIsValid;
}

// call this when device reboots or wake up
// called at DrmProvider
int SecureTimer::updateTimeBase()
{
    if (DrmUtil::sDDebug) ALOGD("updateTimeBase()");
    m_nBaseTime = deviceTime();
    m_nBaseTicks = deviceTicks();
    if (DrmUtil::sDDebug) ALOGD("updateTimeBase(): base=[%ld], tick=[%ld]", m_nBaseTime, m_nBaseTicks);

    // under valid state, check if it has passed 20 day interval or not.
    if (m_bIsValid && ((m_nBaseTime + m_nOffset) > (m_nLastSync + CLK_DFT_INTERVAL) ||
        ((m_nBaseTime + m_nOffset) < m_nLastSync)))
    {
        ALOGI("updateTimeBase() : 20 days interval has expired. reset secure timer.");
        reset();
    }

    if (m_bIsValid) {
        save();
    }

    return RESULT_OK;
}

// call this when the date/time is modified by user
// called at DrmProvider
int SecureTimer::updateOffset()
{
    if (DrmUtil::sDDebug) ALOGD("updateOffset()");
    if (m_bIsValid)
    {
        if (DrmUtil::sDDebug) ALOGD("updateOffset() : secure timer valid.");
        time_t dev_time = deviceTime();
        time_t dev_ticks = deviceTicks();
        if (DrmUtil::sVDebug) ALOGV("updateOffset() : time base [%ld], ticks base [%ld], old offset [%ld], device time [%ld], device ticks [%ld]",
                m_nBaseTime, m_nBaseTicks, m_nOffset, dev_time, dev_ticks);
        time_t delta = (dev_ticks - m_nBaseTicks) - (dev_time - m_nBaseTime);
        //m_nOffset = m_nBaseTime + m_nOffset + (dev_ticks - m_nBaseTicks) - dev_time;
        m_nOffset = delta + m_nOffset;
        if (DrmUtil::sDDebug) ALOGD("updateOffset() : new offset [%ld]", m_nOffset);

        m_nBaseTime = dev_time;
        m_nBaseTicks = dev_ticks;
        save();
    }

    return RESULT_OK;
}

// called at DrmProvider
int SecureTimer::updateDRMTime(time_t offset)
{
    if (DrmUtil::sDDebug) ALOGD("updateDRMTime()");
    // check offset
    time_t realTime = deviceTime() + offset;
    if (realTime < MIN_REAL_TIME || realTime > MAX_REAL_TIME)
    {
        ALOGE("updateDRMTime() : invalid offset: %ld, st state: %d.",offset,m_bIsValid);
        if (m_bIsValid)
        {
            reset();
            save();
        }
        return SecureTimerHelper::NTP_SYNC_INVALID_OFFSET;
    }

    // first we should update the base:
    updateTimeBase();

    if (DrmUtil::sDDebug) ALOGD("updateDRMTime() : update secure timer offset with value [%ld]", offset);
    m_nOffset = offset;
    m_bIsValid = true;

    getDRMTime(m_nLastSync); // stores the last time when it was synchronized
    save();

    return SecureTimerHelper::NTP_SYNC_SUCCESS;
}

// call this if a time-based rights need to be checked
// the real DRM time, or real world time, is returned by {t}
// called at drmserver side.
int SecureTimer::getDRMTime(time_t& t)
{
    if (m_bIsValid)
    {
        if (DrmUtil::sDDebug) ALOGD("getDRMTime() : secure timer valid.");
        // validate current secure timer because the device time may be changed
        //   but not notified with TIME_SET broadcast.
        time_t dev_time = deviceTime();
        time_t dev_ticks = deviceTicks();
        if (DrmUtil::sDDebug) ALOGD("getDRMTime() : validate with time base [%ld], ticks base [%ld], device time [%ld], device ticks [%ld]",
                m_nBaseTime, m_nBaseTicks, dev_time, dev_ticks);
        time_t delta = dev_ticks - m_nBaseTicks - (dev_time - m_nBaseTime);
        if (5 < delta || delta < -5)
        {
            // we allow 5 seconds for error; the error exceeds and we need to update offset
            if (DrmUtil::sDDebug) ALOGD("getDRMTime() : time changed but not notified, delta [%ld]", delta);
            updateOffset();
        }

        // get real time using offset
        t = deviceTime() + m_nOffset;
        if (DrmUtil::sDDebug) ALOGD("getDRMTime() : real time [%ld], offset [%ld]", t, m_nOffset);
        if (t < MIN_REAL_TIME || t > MAX_REAL_TIME)
        {
            ALOGE("getDRMTime() : invalid offset: %ld, st state: %d.", m_nOffset, m_bIsValid);
            if (m_bIsValid)
            {
                reset();
                save();
            }
            return SecureTimerHelper::NTP_SYNC_INVALID_OFFSET;
        }
        return CLOCK_VALID;
    }

    // if invalid, ZERO time (1970-1-1 zulu) will be returned.
    if (DrmUtil::sDDebug) ALOGD("getDRMTime() : secure timer invalid.");
    t = 0;
    return CLOCK_INVALID;
}

// called at drmserver side.
time_t SecureTimer::getOffset()
{
    if (DrmUtil::sDDebug) ALOGD("getOffset(): offset=[%ld]", m_nOffset);
    return m_nOffset;
}

int SecureTimer::load()
{
    Mutex::Autolock lock(mLock);

    if (DrmUtil::sDDebug) ALOGD("load() : re-load secure timer.");
    FILE* fp = fopen(ST_FILE.string(), "rb");
    if (NULL == fp)
    {
        ALOGE("load() : failed to open ST file, reason: [%s].",
                strerror(errno));
        reset();
        return RESULT_ERR;
    }

    // fseek(fp, 0, SEEK_SET);

    // read data
    BYTE data[24];
    bzero(data, sizeof(data));
    if (sizeof(data) != fread(data, sizeof(BYTE), sizeof(data), fp))
    {
        ALOGE("load() : failed to read data from ST file, reason: [%s].",
                strerror(errno));
        fclose(fp);
        reset();
        return RESULT_ERR;
    }
    fclose(fp);

    // decryption
    BYTE temp[24];
    bzero(temp, sizeof(temp));
    DrmUtil::rc4_decrypt_buffer(temp, data, sizeof(temp));

    // result
    DWORD result[6];
    bzero(result, sizeof(result));
    memcpy((BYTE*)result, temp, sizeof(result));

    // data / state
    m_bIsValid = (bool)(result[0]);
    if (DrmUtil::sDDebug) ALOGD("load() : re-load secure timer state: [%d]", (int)m_bIsValid);
    m_nOffset = (time_t)(result[1]);
    if (DrmUtil::sDDebug) ALOGD("load() : re-load secure timer offset: [%ld]", m_nOffset);
    m_nLastSave = (time_t)(result[2]);
    m_nLastSync = (time_t)(result[3]);
    m_nBaseTime = (time_t)(result[4]);
    m_nBaseTicks = (time_t)(result[5]);
    if (DrmUtil::sDDebug) ALOGD("load() : re-load secure timer basetime: [%ld],baseticks: [%ld],lastSync: [%ld]", m_nBaseTime, m_nBaseTicks, m_nLastSync);
    return RESULT_OK;
}

int SecureTimer::save()
{
    Mutex::Autolock lock(mLock);

    if (DrmUtil::sDDebug) ALOGD("save() : save secure timer.");
    // will save always even it's invalid. Remove the original one first
    remove(ST_FILE.string());

    // the last time it was saved - abandoned using this value
    //getDRMTime(m_nLastSave);

    // prepare data
    DWORD result[6] = {
        (DWORD)m_bIsValid,
        (DWORD)m_nOffset,
        (DWORD)m_nLastSave,
        (DWORD)m_nLastSync,
        (DWORD)m_nBaseTime,
        (DWORD)m_nBaseTicks
    };
    BYTE temp[24];
    bzero(temp, sizeof(temp));
    memcpy(temp, (BYTE*)result, sizeof(temp));

    // encryption
    BYTE data[24];
    bzero(data, sizeof(data));
    DrmUtil::rc4_encrypt_buffer(data, temp, sizeof(data));

    // check the storage position & file
    DrmUtil::checkDir(ST_DIR);
    FILE* fp = fopen(ST_FILE.string(), "wb");
    if (NULL == fp)
    {
        ALOGE("save() : failed to open ST file, reason: [%s].",
                strerror(errno));
        return RESULT_ERR;
    }

    // mod 755 so that the drm manager process can access it.
    chmod(ST_DIR.string(), 0755);
    chmod(ST_LOCATION.string(), 0755);

    // fseek(fp, 0, SEEK_SET);

    // write to file
    if (sizeof(data) != fwrite(data, sizeof(BYTE), sizeof(data), fp))
    {
        ALOGE("save() : failed to write data to ST file, reason: [%s].",
                strerror(errno));
        fclose(fp);
        remove(ST_FILE.string());
        return RESULT_ERR;
    }
    fclose(fp);

    //make sure other and group users have no permission to write
    chmod(ST_FILE.string(), 0644);

    return RESULT_OK;
}

// returns the current time of device (in seconds) since 1970-1-1 Zulu
time_t SecureTimer::deviceTime()
{
    return time(NULL); // in <time.h>
}

// returns time interval (in seconds) since the last boot of system
time_t SecureTimer::deviceTicks()
{
    // get the time interval since last time the system boot-up, by reading file: /proc/uptime
    // the format of /proc/uptime is like: xxxx.xx xxxx.xx
    // and we just read the first float value (only the integer part) for system up time (in seconds)
    // M: @{
    // ALPS01226756, Modify the method of get deviceTicks.
    FILE* fp = fopen(SYS_UP_TIME_FILE, "r");
    if (fp == NULL)
    {
        ALOGE("open proc/uptime failed: [%s].", strerror(errno));
        return 0;
    }
    float uptime = 0.0;
    float idletime = 0.0;
    if (-1 == fscanf(fp, "%f%f", &uptime, &idletime)) {
        ALOGE("Read uptime failed from proc/uptime");
        uptime = 0.0;
    }
    if (DrmUtil::sDDebug) ALOGD("uptime = %lf,idletime = %lf,return value = %d", uptime, idletime, (time_t) uptime);
    fclose(fp);
    fp = NULL;
    return (time_t) uptime;
    // M: @}
}

// reset the secure clock to an initial "invalid" state
void SecureTimer::reset()
{
    m_bIsValid = false;
    m_nOffset = 0;
    m_nBaseTicks = 0;
    m_nBaseTime = 0;
    m_nLastSync = 0;
}
