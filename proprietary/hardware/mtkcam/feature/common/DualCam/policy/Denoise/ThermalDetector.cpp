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

#define LOG_TAG "ThermalDetector"

#include <cutils/properties.h>
#include <fcntl.h>

#include <mtkcam/utils/std/Log.h>
//
#include "ThermalDetector.h"

using namespace NSCam;
using namespace android;
/******************************************************************************
*
*******************************************************************************/

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

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
 
#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

#define THREAD_NAME       ("Cam@ThermalDetector")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

#define LIB_FULL_NAME "libthha.so"

#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000

#define QUERY_THERMAL_TICK_TIME (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThermalDetector::
ThermalDetector(
)
{
    // TODO: change default value to 0
    mLogLevel = ::property_get_int32("vendor.debug.camera.denoise", 0);
    if(mLogLevel == 0)
    {
        mLogLevel = ::property_get_int32("vendor.debug.camera.denoise.thermal", 0);
    }

    MY_LOGD("loglevel:%d", mLogLevel);
}
/******************************************************************************
*
*******************************************************************************/
ThermalDetector::
~ThermalDetector(
)
{
    FUNC_START;
    FUNC_END;
}
/******************************************************************************
*
*******************************************************************************/
status_t
ThermalDetector::
execute()
{
    if(run(LOG_TAG) != OK){
        MY_LOGE("thread run failed!");
        return UNKNOWN_ERROR;
    }
    mbBackgroundPulling = true;
    MY_LOGD("background pulling thermal status");
    return OK;
}
/******************************************************************************
*
*******************************************************************************/
bool
ThermalDetector::
canUsingDualCam()
{
    Mutex::Autolock _l(mLock);
    bool value = false;
    if(mbBackgroundPulling)
    {
        value = !mPrvFlag;
    }
    else
    {
        value = (!queryThermalState()) ? 1 : 0;
    }
    return value;
}
/******************************************************************************
*
*******************************************************************************/
void
ThermalDetector::
onLastStrongRef(
    const void* /*id*/
)
{
    FUNC_START;
    FUNC_END;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
status_t
ThermalDetector::
readyToRun(
)
{
    status_t ret = UNKNOWN_ERROR;
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)THREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );
    //
    ret = OK;
    return ret;
}
/******************************************************************************
*
*******************************************************************************/
void
ThermalDetector::
requestExit(
)
{
    // destroy thread
    Thread::requestExit();
    mbRequestExit = true;
    // stop wait
    mWaitCond.signal();
}
/******************************************************************************
*
*******************************************************************************/
bool
ThermalDetector::
threadLoop(
)
{
    if(mbRequestExit)
    {
        return false;
    }
    else
    {
        {
            // ask thermal algo if it can open main2 or not.
            Mutex::Autolock _l(mLock);
            int flag = queryThermalState();
            if(flag == -1)
            {
                MY_LOGD("query thermal state fail. need to set SELinux?");
            }
            if(flag != mPrvFlag)
            {
                mPrvFlag = flag;
            }
        }
        wait(QUERY_THERMAL_TICK_TIME);
    }
    return true;
}
/******************************************************************************
*
*******************************************************************************/
void
ThermalDetector::
wait(
    int sec
)
{
    Mutex::Autolock _l(mWaitLock);
    int64_t nano_sec = sec*1000*1000*1000;
    MY_LOGD("+");
    mWaitCond.waitRelative(mWaitLock, nano_sec);
    MY_LOGD("-");
}
/******************************************************************************
*
*******************************************************************************/
int
ThermalDetector::
queryThermalState()
{
    int value = -1;
    int dual_cam_off = -1;
    if (0 == access("/proc/driver/cl_cam_dual_off", R_OK))
    {
        char buf[80];
        int fd = open("/proc/driver/cl_cam_dual_off", O_RDONLY);
        if (fd < 0)
        {
            MY_LOGI("Can't open cl_cam_dual_off: %s", strerror(errno));
        }
        else
        {
            buf[79] = (char) 0x0;
            int len = read(fd, buf, sizeof(buf) - 1);
            if (len < 0) {
                    MY_LOGI("Can't read cl_cam_dual_off: %s", strerror(errno));
            }
            close(fd);
            if (1 == sscanf(buf, "%d", &dual_cam_off))
            {
                MY_LOGD1("dual cam off: %d \n", dual_cam_off);
                value = dual_cam_off;
            }
            else
                MY_LOGD1("%s\n", buf);
        }
    }
    return value;
}