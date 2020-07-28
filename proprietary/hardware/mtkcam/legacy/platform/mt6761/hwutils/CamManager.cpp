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
#include <mtkcam/Log.h>
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
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
#define DISABLE_EIS                 (0 || (MTKCAM_HAVE_EIS_SUPPORT == 0))

#define DEV_IOC_MAGIC               'd'
#define READ_DEV_DATA               _IOR(DEV_IOC_MAGIC, 1, unsigned int)
#define DEVINFO_DATA_SEGMENT_CODE   21
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
incDevice()
{
    Mutex::Autolock _l(mLockMtx);
    //
    mDeviceCnt++;
    if ( mDeviceCnt == 2 ) {
        MY_LOGD("enable termal policy");
        setThermalPolicy("thermal_policy_01", 1);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
decDevice()
{
    Mutex::Autolock _l(mLockMtx);
    //
    mDeviceCnt--;
    if ( mDeviceCnt == 1 ) {
        MY_LOGD("disable termal policy");
        setThermalPolicy("thermal_policy_01", 0);
    }
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
    return mDeviceCnt;
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
    return (mDeviceCnt > 1) ? frameRate : 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
isMultiDevice() const
{
    Mutex::Autolock _l(mLockFps);
    //
    return (mDeviceCnt > 1) ? true : false;
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
    MY_LOGD("mDeviceCnt(%d), mbRecord(%d), mbAvailable(%d), mbStereo(%d), 0:fps(%d); 1:fps(%d)",
            mDeviceCnt, mbRecord, mbAvailable, mbStereo, getFrameRate(0), getFrameRate(1));
    return !mbRecord && mbAvailable && !mbStereo;
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
getEISScenarioType()
{
    Mutex::Autolock _l(mLockMtx);
    //
    // Get IC segment code from EFUSE
    if (mbReadSegCode == false)
    {
        mSegmentCode = readEFUSEDevData(DEVINFO_DATA_SEGMENT_CODE) & 0xFF;
        MY_LOGD("IC Segment code = 0x%x", mSegmentCode);
        mbReadSegCode = true;
    }

    if (/*mSegmentCode == 0x42 ||*/ mSegmentCode == 0x43 || mSegmentCode == 0x46 ||
        mSegmentCode == 0x4B || mSegmentCode == 0xC2 || mSegmentCode == 0xC6)
    {
        mEISSceType = EIS_SCE_TYPE_EIS;
    }
#if DISABLE_EIS
        mEISSceType = EIS_SCE_TYPE_NULL;
#endif
    MY_LOGD("EIS Scenario Type = %d", mEISSceType);
    return mEISSceType;
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
readEFUSEDevData(uint32_t index)
{
    /**
     * Use ioctl to read EFUSE
     * Note that only (user=root, group=system) have permission to use ioctl device node
     */

    int fd = 0;
    int ret = 0;

    unsigned int devinfo_data = 0; //serve as index as well as readback data

    fd = ::open("/dev/devmap", O_RDONLY);
    if (UNLIKELY(fd<0))
    {
        MY_LOGE("Open node fail");
        goto EFUSE_FAIL;
    }

    MY_LOGD("Devinfo Index: %d", index);
    devinfo_data = index;

    if (UNLIKELY(ret = ioctl(fd, READ_DEV_DATA, &devinfo_data) != 0))
    {
        MY_LOGE("Get Devinfo Data Fail: %d", ret);
        goto EFUSE_FAIL;
    }
    else
    {
         MY_LOGD("Get Devinfo Data: 0x%x", devinfo_data);
    }

EFUSE_FAIL:
    if (fd>=0)
    {
        if (::close(fd) < 0)
        {
            MY_LOGE("Close file descriptor failed.");
        }
    }
    return devinfo_data;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
setThermalPolicy(char* policy, bool usage)
{
    void *handle = dlopen("/system/vendor/lib/libmtcloader.so", RTLD_NOW);
    if (NULL == handle)
    {
        MY_LOGW("%s, can't load thermal library: %s", __FUNCTION__, dlerror());
        return false;
    }
    else
    {
        typedef int (*load_change_policy)(char *, int);

        void *func = dlsym(handle, "change_policy");
        load_change_policy change_policy = reinterpret_cast<load_change_policy>(func);

        change_policy(policy, usage);

        dlclose(handle);
    }
    return true;
}
