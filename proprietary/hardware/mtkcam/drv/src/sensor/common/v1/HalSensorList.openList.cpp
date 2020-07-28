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
#include <cutils/atomic.h>
#define LOG_TAG "MtkCam/HalSensorList"
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif

MUINT32 mCreateSensorCount = 0;
//
//
/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define DEBUG_SENSOR_OPEN_CLOSE 0

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::
OpenInfo::
OpenInfo(
    MINT        iRefCount,
    HalSensor*  pHalSensor
)
    : miRefCount(iRefCount)
    , mpHalSensor(pHalSensor)
{
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensorList::
closeSensor(HalSensor*const pHalSensor, char const* szCallerName)
{
    Mutex::Autolock _l(mOpenSensorMutex);
    //
    OpenList_t::iterator it = mOpenSensorList.begin();
    for (; it != mOpenSensorList.end(); ++it)
    {

        if  ( pHalSensor == it->mpHalSensor )
        {
#if DEBUG_SENSOR_OPEN_CLOSE
            MY_LOGD("closeSensor mpHalSensor : %p, pHalSensor = %p, refcnt= %d",it->mpHalSensor,pHalSensor,it->miRefCount);
#endif
            //  Last one reference ?
            if  ( 1 == ::android_atomic_dec(&it->miRefCount) )
            {
                MY_LOGD("<%s> last user", (szCallerName?szCallerName:"Unknown"));
                //
                //  remove from open list.
                mOpenSensorList.erase(it);
                //  destroy and free this instance.
                pHalSensor->onDestroy();
                delete pHalSensor;
            }
            return;
        }
    }

    MY_LOGE("<%s> HalSensor:%p not exist", (szCallerName?szCallerName:"Unknown"), pHalSensor);
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor*
HalSensorList::
openSensor(
    SortedVector<MUINT>const& vSensorIndex,
    char const* szCallerName
)
{
    Mutex::Autolock _l(mOpenSensorMutex);
    //
    OpenList_t::iterator it = mOpenSensorList.begin();
    for (; it != mOpenSensorList.end(); ++it)
    {
        if  ( it->mpHalSensor->isMatch(vSensorIndex) )
        {
            //  The open list holds a created instance.
            //  just increment reference count and return the instance.
            ::android_atomic_inc(&it->miRefCount);
#if  DEBUG_SENSOR_OPEN_CLOSE
            MY_LOGD("openSensor mpHalSensor : %p,idx %d, %d, %d, refcnt %d",
                it->mpHalSensor, vSensorIndex[0], vSensorIndex[1], vSensorIndex[2], it->miRefCount);
#endif
            return  it->mpHalSensor;
        }
    }
#if DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("new created vSensorIdx[0] = %d, vSensorIdx[1] = %d, vSensorIdx[2] = %d",vSensorIndex[0],vSensorIndex[1],vSensorIndex[2]);
#endif

    //  It does not exist in the open list.
    //  We must create a new one and add it into.
    HalSensor* pHalSensor = NULL;

    pHalSensor = new HalSensor();

    if  ( NULL != pHalSensor )
    {
        //  onCreate callback
        if  ( ! pHalSensor->onCreate(vSensorIndex) )
        {
            MY_LOGE("HalSensor::onCreate");
            delete pHalSensor;
            return NULL;
        }

        //  push into open list (with ref. count = 1).
        mOpenSensorList.push_back(OpenInfo(1, pHalSensor));

        MY_LOGD("<%s> 1st user", (szCallerName?szCallerName:"Unknown"));
        return  pHalSensor;
    }

    MY_LOGF("<%s> Never Be Here...No memory ?", (szCallerName?szCallerName:"Unknown"));
    return  NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
IHalSensor*
HalSensorList::
createSensor(
    char const* szCallerName,
    MUINT const index
)
{
    Mutex::Autolock _l(mEnumSensorMutex);
    SortedVector<MUINT> vSensorIndex;
    vSensorIndex.add(index);
    mCreateSensorCount++;
#if DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("createSensor count = %d, caller =%s",mCreateSensorCount,szCallerName);
#endif
    return  openSensor(vSensorIndex, szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
IHalSensor*
HalSensorList::
createSensor(
    char const* szCallerName,
    MUINT const uCountOfIndex,
    MUINT const*pArrayOfIndex
)
{
    Mutex::Autolock _l(mEnumSensorMutex);
    MY_LOGF_IF(0==uCountOfIndex||0==pArrayOfIndex, "<%s> Bad uCountOfIndex:%d pArrayOfIndex:%p", szCallerName, uCountOfIndex, pArrayOfIndex);
    //
    SortedVector<MUINT> vSensorIndex;
    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        vSensorIndex.add(pArrayOfIndex[i]);
    }
    mCreateSensorCount++;
#if DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("createSensor count = %d, caller =%s",mCreateSensorCount,szCallerName);
#endif
    return  openSensor(vSensorIndex, szCallerName);
}

