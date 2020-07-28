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

#define LOG_TAG "MtkCam/Utils/CamMgr"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/hw/CamManager.h>
using namespace android;
using namespace NSCam::Utils;
//
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
static CamManager singleton;
//
/******************************************************************************
 *
 ******************************************************************************/
CamManager*
CamManager::
getInstance()
{
    return &singleton;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
incDevice(int32_t openId)
{
    Mutex::Autolock _l(mLockMtx);
    //
    for(uint32_t i=0; i<mvOpenId.size(); i++)
    {
        if(mvOpenId[i] == openId)
        {
            MY_LOGW("openId %d is already exist",openId);
            return;
        }
    }
    //
    MY_LOGD("openId %d",openId);
    mvOpenId.push_back(openId);
    if ( mvOpenId.size() == 2 ) {
        MY_LOGD("enable termal policy");
        setThermalPolicy("thermal_policy_01", 1);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
decDevice(int32_t openId)
{
    Mutex::Autolock _l(mLockMtx);
    //
    for(uint32_t i=0; i<mvOpenId.size(); i++)
    {
        if(mvOpenId[i] == openId)
        {
            mvOpenId.removeItemsAt(i);
            MY_LOGD("openId %d",openId);
            if ( mvOpenId.size() == 1 ) {
                MY_LOGD("disable termal policy");
                setThermalPolicy("thermal_policy_01", 0);
            }
            return;
        }
    }
    MY_LOGW("openId %d is not Found",openId);
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamManager::
getFirstOpenId() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    int32_t openId = -1;
    if(mvOpenId.size() > 0)
    {
        openId = mvOpenId[0];
    }
     MY_LOGD("openId %d",openId);
    return openId;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
incUsingDevice(int32_t openId)
{
    {
        Mutex::Autolock _l(mLockMtx);
        //
        for(uint32_t i=0; i<mvUsingId.size(); i++)
        {
            MY_LOGD("openId %d-%d", i, mvUsingId[i]);
            if(mvUsingId[i] == openId)
            {
                MY_LOGW("openId %d is already in use",openId);
                return;
            }
        }
        //
        MY_LOGD("openId %d",openId);
        mvUsingId.push_back(openId);
    }
    {
        Mutex::Autolock _l(mLockConfig);
        MY_LOGD("%d is in config", mDeviceInConfig);
        if( mDeviceInConfig >= 0 )
        {
            mConfigCond.wait(mLockConfig);
            mDeviceInConfig = openId;
        }
        else
        {
            mDeviceInConfig = openId;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
configUsingDeviceDone(int32_t openId)
{
    Mutex::Autolock _l(mLockConfig);
    MY_LOGD("get mDeviceInConfig(%d), openId(%d)", mDeviceInConfig, openId);
    if( mDeviceInConfig != openId )
    {
        MY_LOGW("openId(%d), inConfigId(%d), doesn't unlock mConfigCond",
                openId, mDeviceInConfig);
        return;
    }
    mDeviceInConfig = -1;
    mConfigCond.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
decUsingDevice(int32_t openId)
{
    Mutex::Autolock _l(mLockMtx);
    //
    for(uint32_t i=0; i<mvUsingId.size(); i++)
    {
        if(mvUsingId[i] == openId)
        {
            mvUsingId.removeItemsAt(i);
            MY_LOGD("openId %d",openId);
            return;
        }
    }
    MY_LOGW("openId %d is not Found",openId);
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamManager::
getFirstUsingId() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    int32_t openId = -1;
    if(mvUsingId.size() > 0)
    {
        openId = mvUsingId[0];
    }
    MY_LOGD("openId %d",openId);
    return openId;
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
getDeviceCount() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    return mvOpenId.size();
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setFrameRate(uint32_t const id, uint32_t const frameRate)
{
    Mutex::Autolock _l(mLockFps);
    //
    if (id == 0) {
        mFrameRate0 = frameRate;
    }
    else if (id == 1) {
        mFrameRate1 = frameRate;
    }
    else {
        MY_LOGE("id(%d), frameRate(%d)", id, frameRate);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
getFrameRate(uint32_t const id) const
{
    Mutex::Autolock _l(mLockFps);
    //
    uint32_t frameRate = (id == 0) ? mFrameRate0 : mFrameRate1;
    return (mvOpenId.size() > 1) ? frameRate : 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
isMultiDevice() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    return (mvOpenId.size() > 1) ? true : false;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setRecordingHint(bool const isRecord)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbRecord = isRecord;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setAvailableHint(bool const isAvailable)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbAvailable = isAvailable;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setStereoHint(bool const isStereo)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbStereo = isStereo;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
getPermission() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    MY_LOGD("OpenId.size(%zu), mbRecord(%d), mbAvailable(%d), mbStereo(%d), 0:fps(%d); 1:fps(%d)",
            mvOpenId.size(), mbRecord, mbAvailable, mbStereo, getFrameRate(0), getFrameRate(1));
    return !mbRecord && mbAvailable && !mbStereo;
}
/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
incSensorCount(
    const char* caller
)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mSensorPowerCount++;
    MY_LOGD("[%s] current sensor count [%d]", caller, mSensorPowerCount);
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
decSensorCount(
    const char* caller
)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mSensorPowerCount--;
    MY_LOGD("[%s] current sensor count [%d]", caller, mSensorPowerCount);
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamManager::
getSensorCount(
) const
{
    Mutex::Autolock _l(mLockMtx);
    //
    return mSensorPowerCount;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
setThermalPolicy(char* policy, bool usage)
{
    MY_LOGD("policy(%s) usage(%d) +", policy, usage);

    void *handle = dlopen("/system/vendor/lib/libmtcloader.so", RTLD_NOW);
    if (NULL == handle)
    {
        MY_LOGW("%s, can't load thermal library: %s", __FUNCTION__, dlerror());
        return false;
    }
    else
    {
        typedef int (*load_change_policy)(char *, int);

        MY_LOGD("dl sym");
        void *func = dlsym(handle, "change_policy");

        if(NULL != func)
        {
            load_change_policy change_policy = reinterpret_cast<load_change_policy>(func);

            MY_LOGD("change policy");
            change_policy(policy, usage);
        }
        else
        {
            MY_LOGW("dlsym fail!");
        }

        MY_LOGD("dl close");
        dlclose(handle);
    }

    MY_LOGD("policy(%s) usage(%d) -", policy, usage);
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
getStartPreviewLock()
{
    MY_LOGI("+");
    mLockMtx.lock();
    while(mLockStartPreview.tryLock()!=NO_ERROR)
    {
        mLockMtx.unlock();
        usleep(10*1000);
        mLockMtx.lock();
    }
    mLockMtx.unlock();
    MY_LOGI("-");
}


void
CamManager::
releaseStartPreviewLock()
{
    MY_LOGI("+");
    Mutex::Autolock _l(mLockMtx);
    mLockStartPreview.unlock();
    MY_LOGI("-");
}
