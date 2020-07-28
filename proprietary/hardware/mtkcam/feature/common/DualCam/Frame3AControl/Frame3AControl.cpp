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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "MtkCam/Frame3ACtrl"

#define MTKCAM_3ASYNC_ENABLE (1)
#define MTKCAM_HWSYNC_ENABLE (1)
#include <mtkcam/feature/DualCam/IFrame3AControl.h>

#include "Frame3AControl.h"
#include <mtkcam/utils/std/Log.h>

#if MTKCAM_HWSYNC_ENABLE
#include <mtkcam/drv/IHwSyncDrv.h>
#endif // MTKCAM_HWSYNC_ENABLE

//#include <mtkcam/aaa/ISync3A.h>
#include <cutils/properties.h>  // for property_get_int32()
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#if MTKCAM_HAVE_MTKSTEREO_SUPPORT
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#define DUAL_CAM 1
#else
#define DUAL_CAM 0
#endif

using namespace NS3Av3;
using namespace NSCam;
using namespace android;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(id, fmt, arg...)        CAM_LOGV("(%d)[id:%d][%s] " fmt, ::gettid(), id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)        CAM_LOGD("(%d)[id:%d][%s] " fmt, ::gettid(), id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)        CAM_LOGI("(%d)[id:%d][%s] " fmt, ::gettid(), id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)        CAM_LOGW("(%d)[id:%d][%s] " fmt, ::gettid(), id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)        CAM_LOGE("(%d)[id:%d][%s] " fmt, ::gettid(), id, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define UPDATE_METADATA(metadata, key, value, type) \
    { \
        IMetadata::IEntry tag(key); \
        tag.push_back(value, Type2Type<type>()); \
        metadata->update(key, tag); \
    }

sp<IFrame3AControl>
IFrame3AControl::
createInstance(MINT32 const openId_1, MINT32 const openId_2)
{
    return Frame3AControl::createInstance(openId_1, openId_2);
}

sp<IFrame3AControl>
Frame3AControl::
createInstance(MINT32 const openId_1, MINT32 const openId_2)
{
    return new Frame3AControl(openId_1, openId_2);
}

Frame3AControl::
Frame3AControl(MINT32 const openId_1, MINT32 const openId_2)
    : mbDvfsLevel(MTRUE)
    , mIs3ASyncEnabled(MFALSE)
    //, mpSync3AMgr(nullptr)
    , mForce3ASyncDisabled(-1)
    , mForceFrameSyncDisabled(-1)
    , mbCCUSupport(MFALSE)
{
    mLogCond = property_get_int32("vendor.debug.dualcam.f3a.log", 0);
    //
    MY_LOGD_IF(mLogCond, -1, "+");
    // Set openId_1 as master, openId_2 as slave for initial value
    // set mFrame3AInfo[0]
    mFrame3AInfo[0].openId = openId_1;
    mFrame3AInfo[0].hal3A = nullptr;
    mFrame3AInfo[0].halSensor = MAKE_HalSensorList()->createSensor(LOG_TAG, openId_1);
    mFrame3AInfo[0].shutter = 33000;
    // set mFrame3AInfo[1]
    mFrame3AInfo[1].openId = openId_2;
    mFrame3AInfo[1].hal3A = nullptr;
    mFrame3AInfo[1].halSensor = MAKE_HalSensorList()->createSensor(LOG_TAG, openId_2);
    mFrame3AInfo[1].shutter = 33000;

    // WARN: check initial value
    mFrame3AInfo[0].minFps = mFrame3AInfo[1].minFps = 0;
    mFrame3AInfo[0].maxFps = mFrame3AInfo[1].maxFps = 0;

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();


    mFrame3AInfo[0].devId = pHalSensorList->querySensorDevIdx(openId_1);
    mFrame3AInfo[1].devId = pHalSensorList->querySensorDevIdx(openId_2);

    // set hwsync time
    mFrameSyncWaitCount = getHwSyncWaitTime();
    {
#if DUAL_CAM
        MINT32 main1, main2;
        StereoSettingProvider::getStereoSensorIndex(main1, main2);
        if (main2 == openId_1 || main2 == openId_2)
        {
            mFrameSyncCtrlID = main2;
        }
        else if (main1 == openId_1 || main1 == openId_2)
        {
            MY_LOGE(-1, "Set frameSync control id to %d failed, set to %d",
                main2, main1);
            mFrameSyncCtrlID = main1;
        }
        else
        {
            MY_LOGE(-1, "Can't find frameSync control id(%d), set to %d", main2, openId_2);
            mFrameSyncCtrlID = openId_2;
        }
#else
        MY_LOGI(-1, "Set frameSync control id(%d)", openId_2);
        mFrameSyncCtrlID = openId_2;
#endif
    }
    //
    {
        Mutex::Autolock _l(mFrameSyncLock);
        mFrameSyncDrvInit = false;
        mFrameSyncOnUninit = false;
        mFrameSyncStartCount = 0;
        mFrame3AInfo[0].frameSyncState = mFrame3AInfo[1].frameSyncState = F3A_HWSYNC_DISABLED;
    }
    //MY_LOGD_IF(mLogCond, -1, "-");
    MY_LOGD(-1, "FrameSyncCtrlID: %d", mFrameSyncCtrlID);
}

Frame3AControl::
~Frame3AControl(void)
{
    uninit();
}

MBOOL
Frame3AControl::
init(void)
{
    MY_LOGD(-1, "+ (openId_1:%d, openId_2:%d)", mFrame3AInfo[0].openId, mFrame3AInfo[1].openId);
    MBOOL ret = MFALSE;

    // 1. init 3A sync
    if (!init3ASync(mFrame3AInfo[0].openId, mFrame3AInfo[1].openId))
    {
        MY_LOGE(-1, "Init 3A sync failed");
        goto lbEXIT;
    }

    // 2. init frame sync
    if (!initFrameSync())
    {
        MY_LOGE(-1, "Frame sync Main1 failed");
        goto lbEXIT;
    }

    // set force enable/disable according to properties
    checkProperty();

    ret = MTRUE;

lbEXIT:
    MY_LOGD(-1, "-");
    return ret;
}

/******************************************************************************
 * update 3a sync or frame sync according to properties
 * This function is called in init() and get()
 ******************************************************************************/
MBOOL
Frame3AControl::
checkProperty(void)
{
    MY_LOGD_IF(mLogCond, -1, "+");

    // get properties and enable/disable 3a/frame sync accordingly
    mForce3ASyncDisabled    = property_get_int32("vendor.debug.dualcam.3async.disable", 0);
    // NOTE: disable hwsync in default
    mForceFrameSyncDisabled = property_get_int32("vendor.debug.dualcam.framesync.disable", 0);

    if (mForce3ASyncDisabled > 0)
    {
        MY_LOGI(-1, "Force 3A sync disabled");
    }

    if (mForceFrameSyncDisabled > 0)
    {
        MY_LOGI(-1, "Force frame sync disabled");
    }
    MY_LOGD_IF(mLogCond, -1, "-");
    return MTRUE;
}

MBOOL
Frame3AControl::
uninit(void)
{
    MY_LOGD(-1, "+");
    // uninit 3a sync
    uninit3ASync();
    uninitFrameSync();
    // Note: there is no frameSync uninit
    mFrame3AInfo[0].openId = mFrame3AInfo[1].openId = -1;
    mFrame3AInfo[0].shutter = mFrame3AInfo[1].shutter = 0;
    mIs3ASyncEnabled = MFALSE;
    {
        Mutex::Autolock _l(mFrameSyncLock);
        mFrameSyncStartCount = 0;
        mFrame3AInfo[0].frameSyncState = mFrame3AInfo[1].frameSyncState = F3A_HWSYNC_DISABLED;
    }

    mFrame3AInfo[0].halSensor->destroyInstance(LOG_TAG);
    mFrame3AInfo[1].halSensor->destroyInstance(LOG_TAG);
    MY_LOGD(-1, "-");
    return MTRUE;
}

/******************************************************************************
 * Default frame time: 33ms (fps=30)
 * init one sensor param to pHwSync
 ******************************************************************************/
MBOOL
Frame3AControl::
initFrameSync(void)
{
    //
    MY_LOGD_IF(mLogCond, -1, "+");
    MUINT32 aeInitShutter = 33000;
    // get main1 init ae
    if (mFrame3AInfo[0].hal3A != nullptr)
    {
        mFrame3AInfo[0].hal3A->send3ACtrl(
            NS3Av3::E3ACtrl_GetInitExposureTime,
            reinterpret_cast<MINTPTR>(&aeInitShutter), reinterpret_cast<MINTPTR>(NULL));
    }
    mFrame3AInfo[0].shutter = mFrame3AInfo[1].shutter = aeInitShutter;
    {
        Mutex::Autolock _l(mFrameSyncLock);
        mFrameSyncStartCount = 0;
        mFrame3AInfo[0].frameSyncState = mFrame3AInfo[1].frameSyncState = F3A_HWSYNC_DISABLED;
    }
    //
    MY_LOGD_IF(mLogCond, -1, "-");
    return MTRUE;
}

/******************************************************************************
 * Default frame time: 33ms (fps=30)
 * init one sensor param to pHwSync
 ******************************************************************************/
MBOOL
Frame3AControl::
uninitFrameSync(void)
{
    MY_LOGD_IF(mLogCond, -1, "+");
    Mutex::Autolock _l(mFrameSyncLock);
    mFrameSyncOnUninit = true;
#if MTKCAM_HWSYNC_ENABLE
    MY_LOGD(-1, "frame sync state: main1(%d), main2(%d)", mFrame3AInfo[0].frameSyncState, mFrame3AInfo[1].frameSyncState);
    HWSyncDrv* pHwSync = MAKE_HWSyncDrv();
    if (pHwSync == NULL)
    {
        MY_LOGE(-1, "Get Hwync failed");
        return MFALSE;
    }
    switch (mFrame3AInfo[0].frameSyncState) {
        case F3A_HWSYNC_ENABLING:
        case F3A_HWSYNC_UNLOCKING_AE:
        case F3A_HWSYNC_READY:
        case F3A_HWSYNC_PAUSED:
                MY_LOGD(-1, "destroy HWSyncDrv for openId(%d) dev(%d)",  mFrame3AInfo[0].openId, mFrame3AInfo[0].devId);
                if (pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, HW_SYNC_BLOCKING_EXE,
                     mFrame3AInfo[0].devId, 0) != 0)
                {
                    MY_LOGE(-1, "sync frame openId(%d) stops failed", mFrame3AInfo[0].openId);
                }
                mFrame3AInfo[0].frameSyncState = F3A_HWSYNC_DISABLED;
                break;
        default:
            break;
    }
    switch (mFrame3AInfo[1].frameSyncState) {
        case F3A_HWSYNC_ENABLING:
        case F3A_HWSYNC_UNLOCKING_AE:
        case F3A_HWSYNC_READY:
        case F3A_HWSYNC_PAUSED:
                MY_LOGD(-1, "destroy HWSyncDrv for openId(%d)  dev(%d)",  mFrame3AInfo[1].openId, mFrame3AInfo[1].devId);
                if (pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, HW_SYNC_BLOCKING_EXE,
                                 mFrame3AInfo[1].devId, 0) != 0)
                {
                    MY_LOGE(-1, "sync frame openId(%d) stops failed", mFrame3AInfo[1].openId);
                }
                mFrame3AInfo[1].frameSyncState = F3A_HWSYNC_DISABLED;
                break;
        default:
            break;
    }
    pHwSync->destroyInstance();
    mFrameSyncDrvInit = false;
#endif // MTKCAM_HWSYNC_ENABLE
    //
    MY_LOGD_IF(mLogCond, -1, "-");
    return MTRUE;

}

/******************************************************************************
 * init3ASync
 * 1. init 3a manager
 * 2. create IHal3A for both cameras
 ******************************************************************************/
MBOOL
Frame3AControl::
init3ASync(const int openId_1, const int openId_2)
{
    //
    MBOOL ret = MFALSE;
    MY_LOGD(-1, "+ (%d:%d)", openId_1, openId_2);

    mFrame3AInfo[0].hal3A = MAKE_Hal3A(openId_1, LOG_TAG);
    if (! mFrame3AInfo[0].hal3A)
    {
        MY_LOGE(openId_1, "IHal3A::createInstance() fail: main1");
        goto lbEXIT;
    }
    mFrame3AInfo[1].hal3A = MAKE_Hal3A(openId_2, LOG_TAG);
    if (! mFrame3AInfo[1].hal3A)
    {
        MY_LOGE(openId_2, "IHal3A::createInstance() fail: main2");
        goto lbEXIT;
    }
    // check ccu support or not
    mFrame3AInfo[0].hal3A->send3ACtrl(E3ACtrl_GetCCUFrameSyncInfo, reinterpret_cast<MINTPTR>(&mbCCUSupport), 0);
    MY_LOGD(-1, "create IHal3A instance id:%d(%p) id:%d(%p) ccu(%d)",
            openId_1, mFrame3AInfo[0].hal3A, openId_2, mFrame3AInfo[1].hal3A, mbCCUSupport);
    ret = MTRUE;
lbEXIT:
    MY_LOGD_IF(mLogCond, -1, "-");
    return ret;
}

MBOOL
Frame3AControl::
uninit3ASync()
{
    MY_LOGD_IF(mLogCond, -1, "+");

    if (mFrame3AInfo[0].hal3A != nullptr)
    {
        mFrame3AInfo[0].hal3A->destroyInstance(LOG_TAG);
        mFrame3AInfo[0].hal3A = nullptr;
    }
    if (mFrame3AInfo[1].hal3A != nullptr)
    {
        mFrame3AInfo[1].hal3A->destroyInstance(LOG_TAG);
        mFrame3AInfo[1].hal3A = nullptr;
    }
    MY_LOGD_IF(mLogCond, -1, "-");
    return MTRUE;
}

MVOID
Frame3AControl::
disableReaptingTag(IMetadata* halMetadata)
{
    MUINT8 repeating = 0;
    IMetadata::getEntry<MUINT8>(halMetadata, MTK_HAL_REQUEST_REPEAT, repeating);
    IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
    entry.push_back(0, Type2Type< MUINT8 >());
    halMetadata->update(entry.tag(), entry);
    return;
}

MBOOL
Frame3AControl::
enable3ASync(MINT32 const openId, IMetadata* halMetadata)
{
    Mutex::Autolock _l(m3ASyncLock);

    if (mForce3ASyncDisabled > 0)
    {
        MY_LOGD_IF(mLogCond, openId, "3A sync has been force-disabled");
        return MTRUE;
    }

    MY_LOGD_IF(mLogCond, openId, "+");
    MBOOL ret = MFALSE;

    if (!enable2ASync(openId, halMetadata))
    {
        MY_LOGE(openId, "2A sync enable failed");
        goto lbEXIT;
    }

    if (!enableAFSync(openId, halMetadata))
    {
        MY_LOGE(openId, "AF sync enable failed");
        goto lbEXIT;
    }
    MY_LOGI(openId, "3A sync is enabled");
    ret = MTRUE;

lbEXIT:
    MY_LOGD_IF(mLogCond, openId, "-");
    return ret;
}

MBOOL
Frame3AControl::
disable3ASync(MINT32 const openId, IMetadata* halMetadata)
{
    Mutex::Autolock _l(m3ASyncLock);

    MY_LOGD_IF(mLogCond, openId, "+");
    MBOOL ret = MFALSE;

    if (!disable2ASync(openId, halMetadata))
    {
        MY_LOGE(openId, "2A sync disable failed");
        goto lbEXIT;
    }

    if (!disableAFSync(openId, halMetadata))
    {
        MY_LOGE(openId, "AF sync enable failed");
        goto lbEXIT;
    }

    MY_LOGI(openId, "3A sync is disabled");
    ret = MTRUE;

lbEXIT:
    MY_LOGD_IF(mLogCond, openId, "-");
    return ret;
}


MBOOL
Frame3AControl::
enable2ASync(MINT32 const openId, IMetadata* halMetadata)
{
#if MTKCAM_3ASYNC_ENABLE && DUAL_CAM
    MY_LOGD_IF(mLogCond, openId, "+");
    MINT32 stereoFeatureMode = StereoSettingProvider::getStereoFeatureMode();
    MINT32 syncMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE;
    do
    {
        IMetadata::IEntry tag(MTK_STEREO_SYNC2A_MODE);
        MINT32 featureMode = StereoSettingProvider::getStereoFeatureMode();
        if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM == featureMode)
            syncMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM;
        else if(v1::Stereo::E_STEREO_FEATURE_DENOISE == featureMode)
            syncMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE;
        else if((v1::Stereo::E_STEREO_FEATURE_CAPTURE | v1::Stereo::E_STEREO_FEATURE_VSDOF) == featureMode
            || (v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) == featureMode)
            syncMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF;
        else
            MY_LOGE(openId, "miss setting, please check stereo_mode(%d)", stereoFeatureMode);
        tag.push_back(syncMode, Type2Type<MINT32>());
        halMetadata->update(MTK_STEREO_SYNC2A_MODE, tag);
    }
    while (0);
    MY_LOGD_IF(mLogCond, openId, "stereo mode [%d] sync mode [%d] -", stereoFeatureMode, syncMode);
#endif // MTKCAM_3ASYNC_ENABLE
    return MTRUE;
}

MBOOL
Frame3AControl::
disable2ASync(MINT32 const openId, IMetadata* halMetadata)
{
#if MTKCAM_3ASYNC_ENABLE
    MY_LOGD_IF(mLogCond, openId, "+");
    do
    {
        IMetadata::IEntry tag(MTK_STEREO_SYNC2A_MODE);
        tag.push_back(NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE, Type2Type<MINT32>());
        halMetadata->update(MTK_STEREO_SYNC2A_MODE, tag);
    }
    while (0);
    MY_LOGD_IF(mLogCond, openId, "-");
#endif // MTKCAM_3ASYNC_ENABLE
    return MTRUE;
}

MBOOL
Frame3AControl::
enableAFSync(MINT32 const openId, IMetadata* halMetadata)
{
#if MTKCAM_3ASYNC_ENABLE
    MY_LOGD_IF(mLogCond, openId, "+");
    do
    {
        IMetadata::IEntry tag(MTK_STEREO_SYNCAF_MODE);
        tag.push_back(NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON, Type2Type<MINT32>());
        halMetadata->update(MTK_STEREO_SYNCAF_MODE, tag);
    }
    while (0);
    MY_LOGD_IF(mLogCond, openId, "-");
#endif // MTKCAM_3ASYNC_ENABLE
    return MTRUE;
}

MBOOL
Frame3AControl::
disableAFSync(MINT32 const openId, IMetadata* halMetadata)
{
#if MTKCAM_3ASYNC_ENABLE
    MY_LOGD_IF(mLogCond, openId, "+");
    do
    {
        IMetadata::IEntry tag(MTK_STEREO_SYNCAF_MODE);
        tag.push_back(NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF, Type2Type<MINT32>());
        halMetadata->update(MTK_STEREO_SYNCAF_MODE, tag);
    }
    while (0);
    MY_LOGD_IF(mLogCond, openId, "-");
#endif // MTKCAM_3ASYNC_ENABLE
    return MTRUE;
}

MINT32
Frame3AControl::
getHwSyncWaitTime(void)
{
    MUINT32 count = 1;
#if MTKCAM_HWSYNC_ENABLE
    MINT32 i = 0;
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    for (i = 0; i < 2; i++) {
        pHalSensorList->querySensorStaticInfo(mFrame3AInfo[i].devId,&sensorStaticInfo);
        if (sensorStaticInfo.FrameTimeDelayFrame > count) {
            count = sensorStaticInfo.FrameTimeDelayFrame;
        }
        MY_LOGD_IF(mLogCond, -1, "dev(%d) frameTimeDelay(%d) count(%d)", mFrame3AInfo[0].devId, sensorStaticInfo.FrameTimeDelayFrame, count);
    }
#endif //MTKCAM_HWSYNC_ENABLE
    return count;
}

MBOOL
Frame3AControl::
IsAELockState(Frame3AInfo_t* const f3aInfo, MBOOL state)
{
    MBOOL ret = MTRUE;
    NS3Av3::DualZoomInfo_T aaaInfo;
    //
    MY_LOGD_IF(mLogCond, f3aInfo->openId, "+");
    //
    f3aInfo->hal3A->send3ACtrl(E3ACtrl_GetDualZoomInfo, reinterpret_cast<MINTPTR>(&aaaInfo), 0);
    //
    if (aaaInfo.bIsAEAPLock != state) {
        ret = MFALSE;
    }
    //
    MY_LOGD_IF(mLogCond, f3aInfo->openId, "- expected(%d) actual(%d) return(%d)", state, aaaInfo.bIsAEAPLock, ret);
    return ret;
}

status_t
Frame3AControl::
setHwsyncDrv(MBOOL enable)
{
    MINT32 ret = OK;
    MY_LOGD_IF(mLogCond, -1, "+ IsEnable(%d) Dev1(%d) Dev2(%d)", enable, mFrame3AInfo[0].devId, mFrame3AInfo[1].devId);
#if MTKCAM_HWSYNC_ENABLE
    HWSyncDrv* pHwSync = NULL;
    enum {
        E_HWSYNC_NO_ERROR = OK,
        E_ERR_HWSYNC_NO_DRV,
        E_ERR_HWSYNC_ENABLE_MAIN1_FAILED,
        E_ERR_HWSYNC_ENABLE_MAIN2_FAILED,
        E_ERR_HWSYNC_DISABLE_MAIN1_FAILED,
        E_ERR_HWSYNC_DISABLE_MAIN2_FAILED,
    };
    do
    {
        // enable frame sync for both main1 and main2
        ret = E_HWSYNC_NO_ERROR;
        pHwSync = MAKE_HWSyncDrv();
        if (pHwSync == NULL)
        {
            MY_LOGE(-1, "Get pHwSync failed");
            break;
        }
        if (enable) {
            if (!mFrameSyncDrvInit) {
                //initHwSyncDrv
                // chek dvfs level
                // 1. add mode 2. get shutter time from appMeatadata or set to 0?
                MUINT32 dvfsLevel = (mbDvfsLevel) ? 3 : 0;

                if (pHwSync->sendCommand(HW_SYNC_CMD_ENUM::HW_SYNC_CMD_SET_PARA, mFrame3AInfo[0].devId,
                                                   dvfsLevel, mFrame3AInfo[0].shutter) != 0)
                {
                    MY_LOGE(mFrame3AInfo[0].openId, "Send command HW_SYNC_CMD_SET_PARA to pHwSync sensorDev:%d failed",
                            mFrame3AInfo[0].devId);
                }

                if (pHwSync->sendCommand(HW_SYNC_CMD_ENUM::HW_SYNC_CMD_SET_PARA, mFrame3AInfo[1].devId,
                                           dvfsLevel, mFrame3AInfo[1].shutter) != 0)
                {
                    MY_LOGE(mFrame3AInfo[1].openId, "Send command HW_SYNC_CMD_SET_PARA to pHwSync sensorDev:%d failed",
                            mFrame3AInfo[1].devId);
                }
                mFrameSyncDrvInit = true;
            }
            if (pHwSync->sendCommand(HW_SYNC_CMD_ENABLE_SYNC, 0, mFrame3AInfo[0].devId,
                                     HW_SYNC_NON_BLOCKING_EXE) != 0)
            {
                MY_LOGE(-1, "sync frame openId(%d) starts failed", mFrame3AInfo[0].openId);
                ret = E_ERR_HWSYNC_ENABLE_MAIN1_FAILED;
                break;
            }
            if (pHwSync->sendCommand(HW_SYNC_CMD_ENABLE_SYNC, 0, mFrame3AInfo[1].devId,
                                     HW_SYNC_NON_BLOCKING_EXE) != 0)
            {
                MY_LOGE(-1, "sync frame openId(%d) starts failed", mFrame3AInfo[1].openId);
                ret = E_ERR_HWSYNC_ENABLE_MAIN2_FAILED;
                break;
            }
        } else {
            // disable hwsync driver
            if (pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, HW_SYNC_BLOCKING_EXE,
                             mFrame3AInfo[0].devId, 0) != 0)
            {
                MY_LOGE(-1, "sync frame openId(%d) stops failed", mFrame3AInfo[0].openId);
            }
            if (pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, HW_SYNC_BLOCKING_EXE,
                             mFrame3AInfo[1].devId, 0) != 0)
            {
                MY_LOGE(-1, "sync frame openId(%d) stops failed", mFrame3AInfo[1].openId);
            }
        }
    } while (0);

    if (ret == E_ERR_HWSYNC_ENABLE_MAIN2_FAILED) {
        MY_LOGE(-1, "Can't enable main2 hwsync, disable main1 hwsync");
        if (pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, HW_SYNC_BLOCKING_EXE,
                         mFrame3AInfo[0].devId, 0) != 0)
        {
            MY_LOGE(-1, "sync frame openId(%d) stops failed", mFrame3AInfo[0].openId);
        }
    }

    if (pHwSync != NULL)
    {
        pHwSync->destroyInstance();
    }

#endif // MTKCAM_HWSYNC_ENABLE
    MY_LOGD_IF(mLogCond, -1, "-");

    return ret;
}

MBOOL
Frame3AControl::
enableFrameSync(MINT32 const openId, IMetadata* appMetadata, IMetadata* halMetadata)
{
    MY_LOGD_IF(mLogCond, openId, "+ frameSyncState main1(%d) main2(%d)", mFrame3AInfo[0].frameSyncState, mFrame3AInfo[1].frameSyncState);
    Mutex::Autolock _l(mFrameSyncLock);
    if (appMetadata == NULL || halMetadata == NULL)
    {
        MY_LOGE(openId, "Invalid address appMetadata(%p)", appMetadata);
        return MFALSE;
    }
    if(mbCCUSupport)
    {
        MINT32 main1, main2;
        StereoSettingProvider::getStereoSensorIndex(main1, main2);
        if (openId == main1)
        {
            UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, MINT32);
        }
        return MTRUE;
    }

    MINT32 dropReq = MTK_DUALZOOM_DROP_DIRECTLY;
    if (IMetadata::getEntry<MINT32>(halMetadata, MTK_DUALZOOM_DROP_REQ, dropReq)
        && dropReq == MTK_DUALZOOM_DROP_DIRECTLY)
    {
        MY_LOGD(openId, "this frame will be drop, return directly");
        return MTRUE;
    }

    if (mForceFrameSyncDisabled > 0 || mFrameSyncOnUninit)
    {
        return MTRUE;
    }

    struct Frame3AInfo* info = getFrame3AInfoFromOpenId(openId);
    if (info == NULL) {
        MY_LOGE(openId, "Can't get frame 3A info with openId %d", openId);
        return MFALSE;
    }

    if (info->frameSyncState == F3A_HWSYNC_READY) {
        // hwsync already enabled for this camera
        // Note: always disable repeating tag, may have performance issue?
        UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_ANTIBANDING_MODE, MTK_CONTROL_AE_ANTIBANDING_MODE_OFF, MUINT8);
        disableReaptingTag(halMetadata);
        if (openId == mFrameSyncCtrlID)
        {
            UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, MINT32);
        }
        return MTRUE;
    }

    MINT32 otherId = (info->openId == mFrame3AInfo[0].openId)? 1 : 0;
    MBOOL ret = MTRUE;

#if MTKCAM_HWSYNC_ENABLE
    // need openId to control number of frames that AE locks
    // use main openId to count frame number
    MBOOL repeat;
    do {
        repeat = MFALSE;
        switch (info->frameSyncState)
        {
            case F3A_HWSYNC_PAUSED:
                    if (openId == mFrameSyncCtrlID)
                    {
                        UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, MINT32);
                    }
                    info->frameSyncState = F3A_HWSYNC_READY;
                    break;

            case F3A_HWSYNC_DISABLED:
                {
                    {
                        /*Note: need to enable 2Async and set master/slave for 3a before hwsync*/
                        /*Frame3ASetting setting =
                        {
                            .openId   = info->openId,
                            .masterId = info->openId,
                            .slaveId  = (info->openId==mFrame3AInfo[0].openId)? mFrame3AInfo[1].openId : mFrame3AInfo[0].openId,
                        };
                        set(&setting, F3A_TYPE_MASTER_SLAVE_SET, NULL, halMetadata);*/

                        enable2ASync(info->openId, halMetadata);
                    }
                    UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_ON, MUINT8);
                    info->frameSyncState = F3A_HWSYNC_LOCKING_AE;
                    disableReaptingTag(halMetadata);
                }
                break;
            case F3A_HWSYNC_LOCKING_AE:
                // check if lock AE done
                UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_ON, MUINT8);
                disableReaptingTag(halMetadata);
                if (IsAELockState(info, MTRUE)) {
                    info->frameSyncState = F3A_HWSYNC_AE_LOCKED;
                    repeat = MTRUE;
                    if (openId == mFrame3AInfo[0].openId) {
                        mFrameSyncStartCount = 0;
                    }
                }
                break;
            case F3A_HWSYNC_AE_LOCKED:
                // check if other cam lock AE done
                if (mFrame3AInfo[otherId].frameSyncState != F3A_HWSYNC_AE_LOCKED) {
                    // wait for the other cam to lock AE
                    break;
                }
                // enable hwsync driver
                if (setHwsyncDrv(MTRUE) == OK) {
                    mFrame3AInfo[0].frameSyncState = mFrame3AInfo[1].frameSyncState = F3A_HWSYNC_ENABLING;
                    MY_LOGI(openId, "Frame sync is going to enabled");
                } else {
                    UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
                    info->frameSyncState = F3A_HWSYNC_CANCELING;
                    mFrame3AInfo[otherId].frameSyncState = F3A_HWSYNC_CANCELING;
                    disableReaptingTag(halMetadata);
                    ret = MFALSE;
                }
                break;
            case F3A_HWSYNC_CANCELING:
                // check AE is unlocked
                if (!IsAELockState(info, MFALSE)) {
                    UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
                    disableReaptingTag(halMetadata);
                } else {
                    info->frameSyncState = F3A_HWSYNC_DISABLED;
                }
                break;
            case F3A_HWSYNC_UNLOCKING_AE:
                if (!IsAELockState(info, MFALSE)) {
                    if (openId == mFrameSyncCtrlID)
                    {
                        UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, MINT32);
                    }
                    UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
                    disableReaptingTag(halMetadata);
                } else {
                    info->frameSyncState = F3A_HWSYNC_READY;
                }
                break;
            case F3A_HWSYNC_ENABLING:
                // only count main1 openId
                UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_ON, MUINT8);
                disableReaptingTag(halMetadata);
                if (openId != mFrame3AInfo[0].openId)
                {
                    break;
                }
                if (mFrameSyncStartCount != mFrameSyncWaitCount)
                {
                    mFrameSyncStartCount++;
                }
                else
                {
                    if (openId == mFrameSyncCtrlID)
                    {
                        UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, MINT32);
                    }
                    UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
                    info->frameSyncState = F3A_HWSYNC_UNLOCKING_AE;
                    MY_LOGI(openId, "Frame sync driver for openId(%d) is enabled", openId);
                    mFrame3AInfo[otherId].frameSyncState = F3A_HWSYNC_UNLOCKING_AE;
                    disableReaptingTag(halMetadata);
                    mFrameSyncStartCount = 0;
                }
                break;
            default:
                MY_LOGE(openId, "Frame sync state(%d) is invalid", info->frameSyncState);
                info->frameSyncState = F3A_HWSYNC_CANCELING;
                break;
        }
    } while(repeat);

    if (mFrame3AInfo[0].frameSyncState == F3A_HWSYNC_READY &&
        mFrame3AInfo[1].frameSyncState == F3A_HWSYNC_READY) {
        MY_LOGI(openId, "Frame sync is ready (%d)", mFrameSyncCtrlID);
    }
#endif // MTKCAM_HWSYNC_ENABLE

    MY_LOGD_IF(mLogCond, openId, "- frameSyncState main1(%d) main2(%d)", mFrame3AInfo[0].frameSyncState, mFrame3AInfo[1].frameSyncState);
    return ret;
}

MBOOL
Frame3AControl::
disableFrameSync(MINT32 const openId, IMetadata* appMetadata, IMetadata* halMetadata, MBOOL const pauseOnly)
{
    MY_LOGD_IF(mLogCond, openId, "+ frameSyncState main1(%d) main2(%d)", mFrame3AInfo[0].frameSyncState, mFrame3AInfo[1].frameSyncState);
    Mutex::Autolock _l(mFrameSyncLock);
    if (appMetadata == NULL || halMetadata == NULL)
    {
        MY_LOGE(openId, "Invalid address appMetadata(%p)", appMetadata);
        return MFALSE;
    }

    MINT32 dropReq = MTK_DUALZOOM_DROP_DIRECTLY;
    if (IMetadata::getEntry<MINT32>(halMetadata, MTK_DUALZOOM_DROP_REQ, dropReq)
        && dropReq == MTK_DUALZOOM_DROP_DIRECTLY)
    {
        MY_LOGD(openId, "this frame will be drop, return directly");
        return MTRUE;
    }
    Frame3AInfo_t* info = getFrame3AInfoFromOpenId(openId);
    if (info == NULL) {
        MY_LOGE(openId, "Can't get frame 3a info with openId(%d)", openId);
        return MFALSE;
    }
    MINT otherId = (info->openId == mFrame3AInfo[0].openId)? 1 : 0;

    if (info->frameSyncState == F3A_HWSYNC_DISABLED || (info->frameSyncState == F3A_HWSYNC_PAUSED && pauseOnly))
    {
        MY_LOGD_IF(mLogCond, openId, "Frame sync is already disabled");
        if (openId == mFrameSyncCtrlID)
        {
            UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF, MINT32);
        }
        return MTRUE;
    }
    // for update flicker setting
    disableReaptingTag(halMetadata);

    // disable frame sync
    MBOOL ret = MTRUE;
    MY_LOGD_IF(mLogCond, openId, "+");
#if MTKCAM_HWSYNC_ENABLE

    switch(info->frameSyncState) {
        case F3A_HWSYNC_LOCKING_AE:
        case F3A_HWSYNC_AE_LOCKED:
            // unlock AE
            UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
            disableReaptingTag(halMetadata);
            info->frameSyncState = mFrame3AInfo[otherId].frameSyncState = F3A_HWSYNC_CANCELING;
            break;

        case F3A_HWSYNC_ENABLING:
        case F3A_HWSYNC_UNLOCKING_AE:
        case F3A_HWSYNC_PAUSED:
            // disable hwsync drv and unlock AE
            if(setHwsyncDrv(MFALSE) != OK) {
                MY_LOGE(openId, "Disable hwsync driver failed. Ignore errors and move on.");
            }
            UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
            if (openId == mFrameSyncCtrlID)
            {
                UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF, MINT32);
            }
            disableReaptingTag(halMetadata);
            info->frameSyncState = mFrame3AInfo[otherId].frameSyncState = F3A_HWSYNC_CANCELING;
            break;

        case F3A_HWSYNC_READY:
            if (pauseOnly) {
                info->frameSyncState = F3A_HWSYNC_PAUSED;
            } else {
                // disable hwsync drv and unlock AE
                if(setHwsyncDrv(MFALSE) != OK) {
                    MY_LOGE(openId, "Disable hwsync driver failed. Ignore errors and move on.");
                }
                info->frameSyncState = mFrame3AInfo[otherId].frameSyncState = F3A_HWSYNC_CANCELING;
            }
            if (openId == mFrameSyncCtrlID)
            {
                UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF, MINT32);
            }
            break;

        case F3A_HWSYNC_CANCELING:
            // check unlock AE done
            if (!IsAELockState(info, MFALSE)) {
                if (openId == mFrameSyncCtrlID)
                {
                    UPDATE_METADATA(halMetadata, MTK_STEREO_HW_FRM_SYNC_MODE, NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF, MINT32);
                }
                UPDATE_METADATA(appMetadata, MTK_CONTROL_AE_LOCK, MTK_CONTROL_AE_LOCK_OFF, MUINT8);
                disableReaptingTag(halMetadata);
            } else {
                info->frameSyncState = F3A_HWSYNC_DISABLED;
            }
            break;

        default:
            MY_LOGE(openId, "Frame sync state(%d) is invalid", info->frameSyncState);
            break;
    }

    if (mFrame3AInfo[0].frameSyncState == mFrame3AInfo[1].frameSyncState)
    {
        if(mFrame3AInfo[0].frameSyncState == F3A_HWSYNC_DISABLED) {
            MY_LOGI(openId, "Frame sync is disabled");
        } else if (mFrame3AInfo[0].frameSyncState == F3A_HWSYNC_PAUSED) {
            MY_LOGI(openId, "Frame sync is paused");
        }
    }
    mFrameSyncStartCount = 0;
#endif // MTKCAM_HWSYNC_ENABLE

    MY_LOGD_IF(mLogCond, openId, "- frameSyncState main1(%d) main2(%d)", mFrame3AInfo[0].frameSyncState, mFrame3AInfo[1].frameSyncState);
    return ret;
}

int
Frame3AControl::
getIndexFromOpenId(const int openId)
{
    if (openId == mFrame3AInfo[0].openId)
    {
        return 0;
    }
    else if (openId == mFrame3AInfo[1].openId)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

Frame3AInfo_t*
Frame3AControl::
getFrame3AInfoFromOpenId(const int openId)
{
    if (openId == mFrame3AInfo[0].openId)
    {
        return &(mFrame3AInfo[0]);
    }
    else if (openId == mFrame3AInfo[1].openId)
    {
        return &(mFrame3AInfo[1]);
    }
    else
    {
        return NULL;
    }
}

MBOOL
Frame3AControl::
get(Frame3ASetting_t* setting, MUINT32 const type)
{
    int openId = setting->openId;
    int id = getIndexFromOpenId(openId);
    int request = (type == 0)? F3A_TYPE_3A_INFO : type;
    if (id < 0)
    {
        MY_LOGE(openId, "No valid Frame3AInfo available");
        return MFALSE;
    }

    if ((request & F3A_TYPE_3A_INFO) || (request & F3A_TYPE_FOV_ONLINE_INFO))
    {
        DualZoomInfo_T dualZoomInfo;
        mFrame3AInfo[id].hal3A->send3ACtrl(E3ACtrl_GetDualZoomInfo, reinterpret_cast<MINTPTR>(&dualZoomInfo), 0);

        MY_LOGD(openId, "Get 3A info: AELv_x10(%d), i4AFDAC(%d), bSyncAFDone(%d)",
            dualZoomInfo.i4AELv_x10, dualZoomInfo.i4AFDAC, dualZoomInfo.bSyncAFDone);

        setting->AELv_x10       = dualZoomInfo.i4AELv_x10;
        setting->isoValue       = dualZoomInfo.i4AEIso;
        setting->AFDAC          = dualZoomInfo.i4AFDAC;
        setting->isAFDone       = dualZoomInfo.bAFDone;
        setting->isSyncAFDone   = dualZoomInfo.bSyncAFDone;
        setting->isSync2ADone   = dualZoomInfo.bSync2ADone;

    }

    if (request & F3A_TYPE_FOV_ONLINE_INFO)
    {
        NS3Av3::DAF_TBL_STRUCT* pDafTable = NULL;
        mFrame3AInfo[id].hal3A->send3ACtrl(E3ACtrl_GetAFDAFTable, reinterpret_cast<MINTPTR>(&pDafTable), 0);
        setting->maxDac= pDafTable->af_dac_max;
        setting->minDac = pDafTable->af_dac_min;
        //
        ExpSettingParam_T expSetting;
        mFrame3AInfo[id].hal3A->send3ACtrl( E3ACtrl_GetExposureInfo, reinterpret_cast<MINTPTR>(&expSetting), 0);
        setting->expTime = expSetting.u4ExpTimeInUS0EV;

        if (mFrame3AInfo[id].halSensor != NULL)
        {
            mFrame3AInfo[id].halSensor->sendCommand(mFrame3AInfo[id].devId, SENSOR_CMD_GET_TEMPERATURE_VALUE, (MINTPTR)& setting->temperature, 0, 0);
        }
        MY_LOGD(openId, "Get 3A info: exposure time(%d) temperature(%d) maxDac(%d), minDac(%d)",
             setting->expTime, setting->temperature, setting->maxDac, setting->minDac);
    }

    if (mFrame3AInfo[0].frameSyncState == F3A_HWSYNC_READY &&
        mFrame3AInfo[1].frameSyncState == F3A_HWSYNC_READY)
    {
        setting->hwSyncMode = 1;
    }
    else
    {
        setting->hwSyncMode = 0;
    }
    return MTRUE;
}

/******************************************************************************
 * Set
 * @param  setting
 * @param  type
 * @retval true if no error, false otherwise
 * @details
 * This funtion set different settings according to type
 * such as 2a sync, frame sync, fps
 * However, the dependency between two cameras should be handle carefully
 * out of this function
 ******************************************************************************/
MBOOL
Frame3AControl::
set(Frame3ASetting_t* setting, MUINT32 const type, IMetadata* appMetadata, IMetadata* halMetadata)
{
    if (setting == NULL || type == 0)
    {
        MY_LOGD(-1, "No setting changed\n");
        return MTRUE;
    }
    MY_LOGD_IF(mLogCond, setting->openId, "+ (setting type: 0x%X)", type);

    if (type & F3A_TYPE_3A_SYNC)
    {
        if (setting->is3ASyncEnabled)
        {
            enable3ASync(setting->openId, halMetadata);
        }
        else
        {
            disable3ASync(setting->openId, halMetadata);
        }
    }

    if (type & F3A_TYPE_FRAME_SYNC)
    {
        MY_LOGD(setting->openId, "set frame sync(%d)", setting->hwSyncMode);
        switch(setting->hwSyncMode)
        {
            case 0: // pause
                disableFrameSync(setting->openId, appMetadata, halMetadata, MTRUE);
                break;
            case 1: // enable
                enableFrameSync(setting->openId, appMetadata, halMetadata);
                break;
            case 2: // disable
                disableFrameSync(setting->openId, appMetadata, halMetadata, MFALSE);
                break;
            case 3: // distroy
                uninitFrameSync();
                break;
            default:
                break;
        }
    }

    if (type & F3A_TYPE_FRAMERATE_CHANGE)
    {
        do
        {
            int id = getIndexFromOpenId(setting->openId);
            if (id < 0)
            {
                MY_LOGE(setting->openId, "No valid Frame3AInfo available");
                break;
            }
            if (setting->minFps == mFrame3AInfo[id].minFps && setting->maxFps == mFrame3AInfo[id].maxFps)
            {
                // fps doesn't change
                break;
            }
            {
                MY_LOGD(setting->openId, "set FPS: min(%d), max(%d)", setting->minFps, setting->maxFps);
                mFrame3AInfo[id].hal3A->send3ACtrl(E3ACtrl_SetMinMaxFps, setting->minFps * 1000,
                                                   setting->maxFps * 1000);
            }
            mFrame3AInfo[id].minFps = setting->minFps;
            mFrame3AInfo[id].maxFps = setting->maxFps;
        }
        while (0);
    }

    if (type & F3A_TYPE_MASTER_SLAVE_SET)
    {
        do
        {
            IMetadata::IEntry tag(MTK_STEREO_SYNC2A_MASTER_SLAVE);
            tag.push_back(setting->masterId, Type2Type<MINT32>());
            tag.push_back(setting->slaveId, Type2Type<MINT32>());
            halMetadata->update(MTK_STEREO_SYNC2A_MASTER_SLAVE, tag);

            MY_LOGD(setting->openId, "set 3A master(%d), slave(%d)", setting->masterId, setting->slaveId);
        }
        while (0);
    }

    if (type & F3A_TYPE_DROP_MODE)
    {
        do
        {
            MINT32 dropReq = setting->dropMode;
            IMetadata::getEntry<MINT32>(halMetadata, MTK_DUALZOOM_DROP_REQ, dropReq);
            if (setting->dropMode == MTK_DUALZOOM_DROP_NONE || dropReq == MTK_DUALZOOM_DROP_NONE) {
                dropReq = MTK_DUALZOOM_DROP_NONE;
            } else {
                dropReq = (dropReq >= setting->dropMode) ? dropReq : setting->dropMode;
            }
            IMetadata::setEntry<MINT32>(halMetadata, MTK_DUALZOOM_DROP_REQ, dropReq);
            MY_LOGD_IF(mLogCond, setting->openId, "set drop mode: (in:%d/out:%d)", setting->dropMode, dropReq);
        } while (0);
    }

    MY_LOGD_IF(mLogCond, setting->openId, "-");
    return MTRUE;
}
