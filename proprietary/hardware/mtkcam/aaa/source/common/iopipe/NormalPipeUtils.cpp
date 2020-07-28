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

#define LOG_TAG "mtkcam-3a"
//
#include <map>
#include <mutex>
//
#include <mtkcam/utils/std/Log.h>
//
#include <private/IopipeUtils.h>

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


/******************************************************************************
 *
 ******************************************************************************/
INormalPipeUtils*
INormalPipeUtils::
get()
{
    static INormalPipeUtils singleton;
    return &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
INormalPipeUtils::
INormalPipeUtils()
    : mpNormalPipeModule(INormalPipeModule::get())
{
    MY_LOGE_IF(!mpNormalPipeModule, "INormalPipeModule::get() fail");
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
INormalPipeUtils::
determineSelectedVersion(MUINT32 sensorIndex)
{
    MUINT32 selected_version = (MUINT32)-1;

    std::lock_guard<std::mutex> _l(mSelectedVersionMutex);

    auto it = mSelectedVersionMap.find(sensorIndex);
    if  ( it != mSelectedVersionMap.end() ) {
        selected_version = it->second;
    }
    else {
        if  ( ! mpNormalPipeModule ) {
            MY_LOGE("mpNormalPipeModule==0");
            return selected_version;
        }

        size_t count = 0;
        MUINT32 const* version = NULL;
        int err = mpNormalPipeModule->get_sub_module_api_version(&version, &count, sensorIndex);
        if  ( err < 0 || ! count || ! version ) {
            MY_LOGE(
                "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
                sensorIndex, err, count, version
            );
            return selected_version;
        }

        selected_version = *(version + count - 1); //Select max. version
        mSelectedVersionMap[sensorIndex] = selected_version;
        MY_LOGD("[%d] count:%zu Selected NormalPipe Version:%0#x", sensorIndex, count, selected_version);
    }

    return selected_version;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID*
INormalPipeUtils::
createNormalPipe(
    MUINT32 sensorIndex,
    char const* szCallerName,
    MUINT32 apiVersion
)
{
    if  ( ! mpNormalPipeModule ) {
        MY_LOGE("mpNormalPipeModule==0");
        return NULL;
    }

    MVOID* pPipe = NULL;
    int err = mpNormalPipeModule->createSubModule(
        sensorIndex, szCallerName, apiVersion, &pPipe
    );

    MY_LOGE_IF((err<0||!pPipe), "[%s:%d]err:%#x pPipe:%p", szCallerName, sensorIndex, err, pPipe);
    return pPipe;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID*
INormalPipeUtils::
createDefaultNormalPipe(
    MUINT32 sensorIndex,
    char const* szCallerName
)
{
    if  ( ! mpNormalPipeModule ) {
        MY_LOGE("mpNormalPipeModule==0");
        return NULL;
    }

    MUINT32 const apiVersion = determineSelectedVersion(sensorIndex);

    MVOID* pPipe = NULL;
    int err = mpNormalPipeModule->createSubModule(
        sensorIndex, szCallerName, apiVersion, &pPipe
    );

    MY_LOGE_IF((err<0||!pPipe), "[%s:%d]err:%#x pPipe:%p", szCallerName, sensorIndex, err, pPipe);
    return pPipe;
}

