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

#define LOG_TAG "mtkcam-RequestSensorControlPolicy"

#include <mtkcam/aaa/IHal3A.h> // get 3a info
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/utils/metadata/IMetadata.h>
//
#include <mtkcam3/3rdparty/core/sensor_control.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <camera_custom_stereo.h>
//
#include <math.h>
//
#include "RequestSensorControlPolicy_Multicam.h"
#include "MyUtils.h"
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
#define RUNTIME_QUERY_FOV 1

#define MAX_IDLE_DECIDE_FRAME_COUNT 160
//
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


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
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace requestsensorcontrol {
/******************************************************************************
 *
 ******************************************************************************/
Hal3AObject::
Hal3AObject(
    const std::vector<int32_t> &sensorId
)
{
    for(auto&& id:sensorId)
    {
        auto pHal3a =
            std::shared_ptr<NS3Av3::IHal3A>(
                    MAKE_Hal3A(id, LOG_TAG),
                    [&](auto *p)->void
                    {
                        if(p)
                        {
                            p->destroyInstance(LOG_TAG);
                            p = nullptr;
                        }
                    }
        );
        mvHal3A.insert({id, pHal3a});
        mv3AInfo.insert({id, NS3Av3::DualZoomInfo_T()});
    }
}
/******************************************************************************
 *
 ******************************************************************************/
Hal3AObject::
~Hal3AObject()
{
    MY_LOGD("release object");
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
update()
{
    bool ret = false;
    NS3Av3::DualZoomInfo_T aaaInfo;
    for(auto&& hal3a:mvHal3A)
    {
        auto const& id = hal3a.first;
        auto const& pHal3A = hal3a.second;
        pHal3A->send3ACtrl(
                    NS3Av3::E3ACtrl_GetDualZoomInfo,
                    (MINTPTR)&aaaInfo,
                    0);
        auto iter = mv3AInfo.find(id);
        if(iter != mv3AInfo.end())
        {
            iter->second = aaaInfo;
            ret = true;
        }
        else
        {
            MY_LOGE("[Should not happened] get 3a info fail. (%d)", id);
            ret = false;
        }
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
isAFDone(
    int32_t sensorId
)
{
    auto iter = mv3AInfo.find(sensorId);
    if(iter != mv3AInfo.end())
    {
        return iter->second.bAFDone;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3AObject::
getAEIso(
    int32_t sensorId
)
{
    auto iter = mv3AInfo.find(sensorId);
    if(iter != mv3AInfo.end())
    {
        return iter->second.i4AEIso;
    }
    return -1;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3AObject::
getAELv_x10(
    int32_t sensorId
)
{
    auto iter = mv3AInfo.find(sensorId);
    if(iter != mv3AInfo.end())
    {
        return iter->second.i4AELv_x10;
    }
    return -1;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
is2ASyncReady(
    int32_t masterSensorId
)
{
    auto iter = mv3AInfo.find(masterSensorId);
    if(iter != mv3AInfo.end())
    {
        return iter->second.bSync2ADone;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
isAFSyncDone(
    int32_t masterSensorId
)
{
    auto iter = mv3AInfo.find(masterSensorId);
    if(iter != mv3AInfo.end())
    {
        return iter->second.bSyncAFDone;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
isAaoReady(
    std::vector<uint32_t> const& sensorList
)
{
    MBOOL ret = MTRUE;
    for(auto&& id:sensorList)
    {
        auto iter = mvHal3A.find(id);
        if(iter != mvHal3A.end())
        {
            MBOOL value;
            iter->second->send3ACtrl(
                        NS3Av3::E3ACtrl_GetAAOIsReady,
                        (MINTPTR)&value,
                        0);
            ret &= value;
        }
    }
    if(ret)
        return true;
    else
        return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Hal3AObject::
isFramesyncReady(
    int32_t masterSensorId
)
{
    auto iter = mv3AInfo.find(masterSensorId);
    if(iter != mv3AInfo.end())
    {
        if(iter->second.bIsFrameSyncDone)
            return true;
        else
            return false;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool Hal3AObject::setSync2ASetting(
    int32_t masterSensorId,
    int32_t resumeSensorId)
{
    auto iter = mvHal3A.find(masterSensorId);
    if(iter != mvHal3A.end())
    {
        MY_LOGI("master(%d) slave(%d)", masterSensorId, resumeSensorId);
        iter->second->send3ACtrl(
            NS3Av3::E3ACtrl_Sync3A_Sync2ASetting,
            masterSensorId,
            resumeSensorId);
        return true;
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
DeviceInfoHelper::
DeviceInfoHelper(
    const int32_t &deviceId
)
{
    auto pSensorList = MAKE_HalSensorList();
    auto pLogicalDevice = MAKE_HalLogicalDeviceList();
    if(pSensorList == nullptr)
    {
        MY_LOGE("create sensor hal list fail.");
        return;
    }
    if(pLogicalDevice == nullptr)
    {
        MY_LOGE("logical device list fail.");
        return;
    }
    auto sensorId = pLogicalDevice->getSensorId(deviceId);
    for(auto&& id:sensorId)
    {
        IMetadata sensorMetadata = MAKE_HalLogicalDeviceList()->queryStaticInfo(id);
        // get sensor static info.
        auto sensorDevId = pSensorList->querySensorDevIdx(id);
        auto sensorStaticInfo = std::make_shared<SensorStaticInfo>();
        if(sensorStaticInfo != nullptr)
        {
            pSensorList->querySensorStaticInfo(sensorDevId, sensorStaticInfo.get());
            MFLOAT focalLength = .0f;
            updatePhysicalFocalLength(id, focalLength, sensorMetadata);
            mvPhysicalFocalLength.insert({id, focalLength});
#if RUNTIME_QUERY_FOV
            auto sensorFov = std::make_shared<MSizeF>();
            updatePhysicalSensorFov(id, focalLength, sensorFov, sensorMetadata);
            mvPhysicalSensorFov.insert({id, sensorFov});
#endif
            mvPhysicalSensorStaticInfo.insert({id, sensorStaticInfo});
        }
        // get active array size from static metadata.
        // (get physic sensor)
        auto activeArray = std::make_shared<MRect>();
        if(!IMetadata::getEntry<MRect>(&sensorMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, *activeArray.get()))
        {
            MY_LOGE("get MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION fail.");
        }
        MY_LOGD("[physical] sensorId(%d) active(%d, %d, %d, %d)",
                                                    id,
                                                    activeArray->p.x,
                                                    activeArray->p.y,
                                                    activeArray->s.w,
                                                    activeArray->s.h);
        mvPhysicalSensorActiveArray.insert({id, activeArray});
    }
    // qeury available focal length
    auto pMetadataProvider = NSMetadataProviderManager::valueForByDeviceId(deviceId);
    if(pMetadataProvider == nullptr)
    {
        MY_LOGA("MetaProvider is nullptr. id(%d)", deviceId);
    }
    auto logicalDeviceMeta = pMetadataProvider->getMtkStaticCharacteristics();
    auto availableFocalLengthEntry = logicalDeviceMeta.takeEntryFor(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
    for(size_t i=0;i<availableFocalLengthEntry.count();i++)
    {
        mvAvailableFocalLength.push_back(availableFocalLengthEntry.itemAt(i, Type2Type<MFLOAT>()));
        MY_LOGD("add available focal length(%f)", mvAvailableFocalLength[i]);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
DeviceInfoHelper::
~DeviceInfoHelper()
{
    MY_LOGD("release device info helper");
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DeviceInfoHelper::
getPhysicalSensorFov(
    int32_t physicalSensorId,
    float &h,
    float &v
)
{
    bool ret = false;
#if RUNTIME_QUERY_FOV
    auto iter = mvPhysicalSensorFov.find(physicalSensorId);
    if(iter != mvPhysicalSensorFov.end())
    {
        h = iter->second->w;
        v = iter->second->h;
        ret = true;
    }
    else
    {
        auto pSensorStaticInfo = mvPhysicalSensorStaticInfo[physicalSensorId];
        if(pSensorStaticInfo != nullptr)
        {
            h = pSensorStaticInfo->horizontalViewAngle;
            v = pSensorStaticInfo->verticalViewAngle;
        }
        else
        {
            MY_LOGE("cannot get fov value from static metadata sensorId (%d)", physicalSensorId);
        }
        ret = false;
    }
#else
    auto pSensorStaticInfo = mvPhysicalSensorStaticInfo[physicalSensorId];
    if(pSensorStaticInfo != nullptr)
    {
        h = pSensorStaticInfo->horizontalViewAngle;
        v = pSensorStaticInfo->verticalViewAngle;
    }
    else
    {
        MY_LOGE("cannot get fov value from static metadata sensorId (%d)", physicalSensorId);
        ret = false;
    }
#endif
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DeviceInfoHelper::
getPhysicalSensorActiveArray(
    int32_t physicalSensorId,
    MRect &activeArray
)
{
    bool ret = false;
    auto pActiveArray = mvPhysicalSensorActiveArray[physicalSensorId];
    if(pActiveArray != nullptr)
    {
        activeArray = *pActiveArray.get();
        ret = true;
    }
    else
    {
        MY_LOGE("cannot get physical active array. sensor(%d)", physicalSensorId);
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DeviceInfoHelper::
getPhysicalSensorFullSize(
    int32_t physicalSensorId,
    MSize &sensorSize
)
{
    bool ret = false;
    auto pActiveArray = mvPhysicalSensorActiveArray[physicalSensorId];
    if(pActiveArray != nullptr)
    {
        sensorSize = MSize(
                            pActiveArray->s.w,
                            pActiveArray->s.h
                    );
        ret = true;
    }
    else
    {
        MY_LOGE("cannot get physical active array. sensor(%d)", physicalSensorId);
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DeviceInfoHelper::
getPhysicalFocalLength(
    int32_t physicalSensorId,
    MFLOAT &focalLength
)
{
    bool ret = false;
    auto iter = mvPhysicalFocalLength.find(physicalSensorId);
    if(iter != mvPhysicalFocalLength.end())
    {
        focalLength = iter->second;
    }
    else
    {
        MY_LOGE("cannot get physical focal length. sensor(%d)", physicalSensorId);
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DeviceInfoHelper::
getLogicalFocalLength(
    std::vector<MFLOAT>& focalLength
)
{
    focalLength = mvAvailableFocalLength;
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
DeviceInfoHelper::
updatePhysicalFocalLength(
    int32_t sensorId __attribute__((unused)),
    MFLOAT &focalLength,
    IMetadata &sensorMetadata
) -> bool
{
    if(!IMetadata::getEntry<MFLOAT>(&sensorMetadata, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, focalLength))
    {
        MY_LOGE("get MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS fail.");
        return false;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
DeviceInfoHelper::
updatePhysicalSensorFov(
    int32_t sensorId,
    MFLOAT &focalLength,
    std::shared_ptr<MSizeF>& pSensorFov,
    IMetadata &sensorMetadata
) -> bool
{
#define PI 3.14159265
    float fPhysicalSize_h = .0f;
    float fPhysicalSize_v = .0f;
    if(!IMetadata::getEntry<MFLOAT>(&sensorMetadata, MTK_SENSOR_INFO_PHYSICAL_SIZE, fPhysicalSize_h, 0))
    {
        MY_LOGE("get MTK_SENSOR_INFO_PHYSICAL_SIZE (h) fail.");
    }
    if(!IMetadata::getEntry<MFLOAT>(&sensorMetadata, MTK_SENSOR_INFO_PHYSICAL_SIZE, fPhysicalSize_v, 1))
    {
        MY_LOGE("get MTK_SENSOR_INFO_PHYSICAL_SIZE (v) fail.");
    }
    // compute fov
    double result_h = (2.0) * atan(fPhysicalSize_h/((2.0) * focalLength)) * 180 / PI;
    double result_v = (2.0) * atan(fPhysicalSize_v/((2.0) * focalLength)) * 180 / PI;
    pSensorFov->w = (MFLOAT)result_h;
    pSensorFov->h = (MFLOAT)result_v;
    MY_LOGD("sensorId(%d) focal_length(%f) physical_size(%f x %f) fov_h(%f) fov_v(%f)",
            sensorId, focalLength, fPhysicalSize_h, fPhysicalSize_v,
            pSensorFov->w, pSensorFov->h);
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
SensorStateManager::
SensorStateManager(
    const std::vector<int32_t> &vSensorId,
    uint32_t maxOnlineCount,
    uint32_t loglevel
) : mLogLevel(loglevel),
    mMaxOnlineSensorCount(maxOnlineCount)
{
    for(size_t i=0;i<vSensorId.size();i++)
    {
        auto sensorState = SensorStateType::E_ONLINE;
        auto sensorOnlineStatue = sensorcontrol::SensorStatus::E_STREAMING;
        if(i > (maxOnlineCount - 1))
        {
            sensorState = SensorStateType::E_OFFLINE;
            sensorOnlineStatue = sensorcontrol::SensorStatus::E_NONE;
        }
        SensorState s = {
            .eSensorState = sensorState,
            .eSensorOnlineStatus = sensorOnlineStatue,
            .iIdleCount = 0
        };
        mvSensorStateList.insert({vSensorId[i], std::move(s)});
    }
    MY_LOGD("loglevel(%" PRIu32 ") maxOnlineCount(%u)", loglevel, maxOnlineCount);
}
/******************************************************************************
 *
 ******************************************************************************/
SensorStateManager::
~SensorStateManager()
{
    MY_LOGD("release obj");
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
update(
    sensorcontrol::SensorControlParamOut const& out,
    ParsedSensorStateResult &result,
    bool const& bCaptureing
)
{
    if((bCaptureing || isSensorSwitchGoing()) && mbInit == false)
    {
        MY_LOGD("using previous data. c(%d) s(%d)", bCaptureing, isSensorSwitchGoing());
        // result sensor count first.
        resetIdleCountToAllSensors();
        // clone last updated result.
        mLastUpdateData.vNeedOnlineSensorList.clear();
        mLastUpdateData.vNeedOfflineSensorList.clear();
        result = mLastUpdateData;
    }
    else
    {
        std::unordered_map<uint32_t, sensorcontrol::SensorStatus>
                            vResidueOnlineStateList;
        if(!updateSensorState(out, result, vResidueOnlineStateList))
        {
            return false;
        }
        if(!updateOnlineSensorResult(result, vResidueOnlineStateList))
        {
            return false;
        }
        mLastUpdateData = result;
        mbInit = false;
    }
    // resume already add to streaming list.
    // still switching, remove from streaming list.
    for(auto iter = result.vStreamingSensorIdList.begin();
        iter != result.vStreamingSensorIdList.end();)
    {
        auto iter_switching = std::find(mvSwitchingIdList.begin(),
                                          mvSwitchingIdList.end(),
                                          *iter);
        if(iter_switching != mvSwitchingIdList.end()) {
            iter = result.vStreamingSensorIdList.erase(iter);
        }
        else {
            ++iter;
        }
    }
    // dump all list for debug
    {
        String8 s("online sensor state:");
        s.appendFormat("\nStandbySensorIdList:");
        for(auto&& item:result.vStandbySensorIdList)
        {
            s.appendFormat(" %d", item);
        }
        s.appendFormat("\nResumeSensorIdList:");
        for(auto&& item:result.vResumeSensorIdList)
        {
            s.appendFormat(" %d", item);
        }
        s.appendFormat("\nStreamingSensorIdList:");
        for(auto&& item:result.vStreamingSensorIdList)
        {
            s.appendFormat(" %d", item);
        }
        s.appendFormat("\nGotoStandbySensorIdList:");
        for(auto&& item:result.vGotoStandbySensorIdList)
        {
            s.appendFormat(" %d", item);
        }
        MY_LOGD("%s", s.string());
    }
    // copy alternactive crop region
    {
        for(auto&& item:out.vResult)
        {
            if(item.second != nullptr)
            {
                result.vAlternactiveCropRegion.insert(
                            {item.first,
                            item.second->mrAlternactiveCropRegion});
                result.vAlternactiveCropRegion_Cap.insert(
                            {item.first,
                            item.second->mrAlternactiveCropRegion_Cap});
            }
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
void
SensorStateManager::
switchSensorDone()
{
    setSwitchGoingFlag(false);
    auto cur_Time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = cur_Time - pre_time;
    MY_LOGI("switch done. using time: %lf\n", diff.count());
}
/******************************************************************************
 *
 ******************************************************************************/
SensorStateManager::SensorStateType
SensorStateManager::
getSensorState(
    uint32_t sensorId
) const
{
    SensorStateManager::SensorStateType type = E_INVALID;
    auto iter = mvSensorStateList.find(sensorId);
    if(iter != mvSensorStateList.end())
    {
        type = iter->second.eSensorState;
    }
    else
    {
        MY_LOGE("[%d] cannot get sensor state in mvSensorStateList.", sensorId);
    }
    return type;
}
/******************************************************************************
 *
 ******************************************************************************/
sensorcontrol::SensorStatus
SensorStateManager::
getSensorOnlineState(
    uint32_t sensorId
) const
{
    sensorcontrol::SensorStatus type = sensorcontrol::E_NONE;
    auto iter = mvSensorStateList.find(sensorId);
    if(iter != mvSensorStateList.end())
    {
        type = iter->second.eSensorOnlineStatus;
    }
    else
    {
        MY_LOGE("[%d] cannot get sensor online state in mvSensorStateList.", sensorId);
    }
    return type;
}
/******************************************************************************
 *
 ******************************************************************************/
int32_t
SensorStateManager::
getMasterId() const
{
    return miCurrentMasterId;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
isSensorSwitchGoing()
{
    std::lock_guard<std::mutex> lock(mLock);
    return mbSensorSwitchGoing;
}
/******************************************************************************
 *
 ******************************************************************************/
void
SensorStateManager::
setSwitchGoingFlag(
    bool flag
)
{
    std::lock_guard<std::mutex> lock(mLock);
    MY_LOGD("as-is(%d) to-be(%d)", mbSensorSwitchGoing, flag);
    mbSensorSwitchGoing = flag;
    // if switch done, clear list.
    if(!flag)
        mvSwitchingIdList.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
void
SensorStateManager::
setSensorState(
    uint32_t &id,
    SensorStateType type
)
{
    auto iter = mvSensorStateList.find(id);
    if(iter != mvSensorStateList.end())
    {
        if(type == SensorStateType::E_ONLINE)
        {
            iter->second.eSensorState = SensorStateType::E_ONLINE;
            iter->second.eSensorOnlineStatus =
                        sensorcontrol::SensorStatus::E_STREAMING;
            iter->second.iIdleCount = 0;
        }
        else if(type == SensorStateType::E_OFFLINE)
        {
            iter->second.eSensorState = SensorStateType::E_OFFLINE;
            iter->second.eSensorOnlineStatus =
                        sensorcontrol::SensorStatus::E_NONE;
            iter->second.iIdleCount = 0;
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
selectNeedOfflineFromList(
    ParsedSensorStateResult &parsedSensorStateResult,
    std::unordered_map<uint32_t, sensorcontrol::SensorStatus> &vResidueOnlineStateList,
    std::vector<uint32_t> const& selectList,
    unsigned int &removeCount
)
{
    auto remove_item_from_residue_online_list = [&vResidueOnlineStateList](uint32_t id)
    {
        auto iter = vResidueOnlineStateList.find(id);
        if(iter != vResidueOnlineStateList.end())
        {
            vResidueOnlineStateList.erase(iter);
        }
        return;
    };
    // remove size greate or equal to selectList size.
    if((removeCount > selectList.size())&&((removeCount - selectList.size()) >= 0))
    {
        for(auto&& id : selectList)
        {
            parsedSensorStateResult.vNeedOfflineSensorList.push_back(id);
            remove_item_from_residue_online_list(id);
        }
        removeCount -= selectList.size();
    }
    else
    {
        for(size_t i=0;i<removeCount;i++)
        {
            parsedSensorStateResult.vNeedOfflineSensorList.push_back(
                            selectList[i]);
            remove_item_from_residue_online_list(
                            selectList[i]);
        }
        // in this case, standby size is greater than remove size.
        // so, removeCount will be zero;
        removeCount = 0;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
updateSensorState(
    sensorcontrol::SensorControlParamOut const& out,
    ParsedSensorStateResult &parsedSensorStateResult,
    std::unordered_map<uint32_t, sensorcontrol::SensorStatus> &vResidueOnlineStateList
)
{
    uint32_t streamingCount = 0;
    std::vector<uint32_t> vCurrent_Online_StandbyState;
    std::vector<uint32_t> vDecision_StreamingState;
    std::vector<uint32_t> vOnlineList;
    for(auto&& item:out.vResult)
    {
        if(item.second != nullptr)
        {
            if(item.second->isMaster)
            {
                miCurrentMasterId = item.first;
            }
            // check streaming count whether greate than max online count.
            if(item.second->iSensorControl == sensorcontrol::SensorStatus::E_STREAMING)
            {
                vDecision_StreamingState.push_back(item.first);
                if(++streamingCount > mMaxOnlineSensorCount)
                {
                    MY_LOGE("streaming count (%u) > max online sensor count (%u)",
                                streamingCount,
                                mMaxOnlineSensorCount);
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
                    aee_system_exception(
                        LOG_TAG,
                        NULL,
                        DB_OPT_DEFAULT,
                        "streaming count check fail.\nCRDISPATCH_KEY:streaming count check fail.");
#endif
                    return false;
                }
                // check need online
                {
                    auto sensorState = getSensorState(item.first);
                    if(sensorState == E_INVALID)
                    {
                        return false;
                    }
                    else if(sensorState == E_OFFLINE)
                    {
                        // current decision need streaming, but sensor state is offline.
                        // push sensor id to need online sensor list.
                        parsedSensorStateResult.vNeedOnlineSensorList.push_back(item.first);
                        vOnlineList.push_back(item.first);
                    }
                    else // E_ONLINE
                    {
                        vOnlineList.push_back(item.first);
                        // store current sensor state is online, but online state is E_STANDBY.
                        // For, checking needs offline.
                        auto onlineState = getSensorOnlineState(item.first);
                        if(onlineState == sensorcontrol::SensorStatus::E_STANDBY)
                        {
                            vCurrent_Online_StandbyState.push_back(item.first);
                        }
                        // Store decision to vResidueOnlineStateList.
                        vResidueOnlineStateList.insert({
                                                    item.first,
                                                    item.second->iSensorControl});
                    }
                }
            }
            else if(item.second->iSensorControl == sensorcontrol::SensorStatus::E_STANDBY)
            {
                // get sensor state, and store online in online list.
                auto sensorState = getSensorState(item.first);
                if(sensorState == E_ONLINE)
                {
                    vOnlineList.push_back(item.first);
                    // store current sensor state is online, but online state is E_STANDBY.
                    // For checking needs offline.
                    auto onlineState = getSensorOnlineState(item.first);
                    if(onlineState == sensorcontrol::SensorStatus::E_STANDBY)
                    {
                        vCurrent_Online_StandbyState.push_back(item.first);
                    }
                    // Store decision to vResidueOnlineStateList.
                    vResidueOnlineStateList.insert({
                                                item.first,
                                                item.second->iSensorControl});
                }
            }
        }
        else
        {
            MY_LOGE("[%d] decision result is nullptr", item.first);
            return false;
        }
    }
    // after check decision result. It has to check whether some sensor need offline.
    if(vOnlineList.size() > mMaxOnlineSensorCount)
    {
        auto remove_item_from_online_list = [&vOnlineList](std::vector<uint32_t> &list)
        {
            for(auto&& id:list)
            {
                auto iter = std::find(vOnlineList.begin(), vOnlineList.end(), id);
                if(iter != vOnlineList.end())
                {
                    vOnlineList.erase(iter);
                }
            }
            return;
        };
        unsigned int remove_size = vOnlineList.size() - mMaxOnlineSensorCount;
        // 1. remove decision wants streaming sensor id.
        remove_item_from_online_list(vDecision_StreamingState);
        // 2. select need offline from online and standby mode.
        if(vCurrent_Online_StandbyState.size() > 0 && remove_size > 0)
        {
            // remove size greate or equal to online standby size.
            if((remove_size - vCurrent_Online_StandbyState.size()) >= 0)
            {
                // remove from online list and set to offline.
                // it means all sensor id in vCurrent_Online_StandbyState will
                // all be offline.
                remove_item_from_online_list(vCurrent_Online_StandbyState);
            }
            selectNeedOfflineFromList(
                            parsedSensorStateResult,
                            vResidueOnlineStateList,
                            vCurrent_Online_StandbyState,
                            remove_size);
        }
        // 3. if remove_size > 0, select offline from online list.
        if(remove_size > 0)
        {
            selectNeedOfflineFromList(
                            parsedSensorStateResult,
                            vResidueOnlineStateList,
                            vOnlineList,
                            remove_size);
        }
        // 4. [Error]
        if(remove_size > 0)
        {
            MY_LOGE("online(%zu) count(%d) remove_size(%" PRIu32 ")",
                            vOnlineList.size(),
                            mMaxOnlineSensorCount,
                            remove_size);
            MY_LOGE("offline logic error, please check");
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
            aee_system_exception(
                LOG_TAG,
                NULL,
                DB_OPT_DEFAULT,
                "offline logic error.\nCRDISPATCH_KEY:offline logic error.");
#endif
            return false;
        }
    }
    for(auto&& item:parsedSensorStateResult.vNeedOnlineSensorList)
    {
        mvSwitchingIdList.push_back(item);
    }
    for(auto&& item:parsedSensorStateResult.vNeedOfflineSensorList)
    {
        mvSwitchingIdList.push_back(item);
    }
    // check need switch sensor
    if(parsedSensorStateResult.vNeedOnlineSensorList.size() > 0 ||
       parsedSensorStateResult.vNeedOfflineSensorList.size() > 0)
    {
        if(!isSensorSwitchGoing())
        {
            setSwitchGoingFlag(true);
            for(auto&& item:parsedSensorStateResult.vNeedOnlineSensorList)
            {
                setSensorState(item, SensorStateType::E_ONLINE);
            }
            for(auto&& item:parsedSensorStateResult.vNeedOfflineSensorList)
            {
                setSensorState(item, SensorStateType::E_OFFLINE);
            }
            // start timer
            pre_time = std::chrono::system_clock::now();
        }
        else
        {
            MY_LOGW("want to swich, but already has task going.");
        }
    }
    // dump debug data
    {
        String8 s("need online:");
        for(auto&& item:parsedSensorStateResult.vNeedOnlineSensorList)
        {
            s.appendFormat(" %d", item);
        }
        s.appendFormat("\nneed offline:");
        for(auto&& item:parsedSensorStateResult.vNeedOfflineSensorList)
        {
            s.appendFormat(" %d", item);
        }
        s.appendFormat("\nresidue:");
        for(auto&& item:vResidueOnlineStateList)
        {
            s.appendFormat(" [%d:%d]", item.first, item.second);
        }
        s.appendFormat("\nswitching:");
        for(auto&& item:mvSwitchingIdList)
        {
            s.appendFormat(" %d", item);
        }
        MY_LOGD("%s", s.string());
    }
    //
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
updateOnlineSensorResult(
    ParsedSensorStateResult &parsedSensorStateResult,
    std::unordered_map<uint32_t, sensorcontrol::SensorStatus>
                                        const& vResidueOnlineStateList
)
{
    // using mvSensorIdList as search rule, to keep camera open order.
    for(auto&& item:vResidueOnlineStateList)
    {
        const auto &id = item.first;
        // get sensor status info
        auto iter_sensor_state_info = mvSensorStateList.find(id);
        auto check_iter_sensor_state_info =
                    (iter_sensor_state_info != mvSensorStateList.end());
        if(check_iter_sensor_state_info)
        {
            // SensorStateInfo data
            auto &pSensorStateInfo = iter_sensor_state_info->second;
            // current online state
            const auto &eSensorOnlineResult = pSensorStateInfo.eSensorOnlineStatus;
            // decision online state
            const auto &eDecisionSensorOnlineResult = item.second;
            // is not switch
            auto iter_switching = std::find(mvSwitchingIdList.begin(),
                                            mvSwitchingIdList.end(),
                                            id);
            bool isSwitching = (iter_switching != mvSwitchingIdList.end()) ? true : false;
            if(isSwitching) continue;
            // check previous sensor status
            bool isGoToStandby =
                    (eSensorOnlineResult == sensorcontrol::SensorStatus::E_STREAMING) &&
                    (eDecisionSensorOnlineResult == sensorcontrol::SensorStatus::E_STANDBY);
            bool isStreaming =
                    (eDecisionSensorOnlineResult == sensorcontrol::SensorStatus::E_STREAMING);
            bool isGoToStreaming =
                    ((eSensorOnlineResult == sensorcontrol::SensorStatus::E_STANDBY) ||
                    (eSensorOnlineResult == sensorcontrol::SensorStatus::E_NONE)) &&
                    isStreaming;
            bool isStandby =
                    ((eSensorOnlineResult == sensorcontrol::SensorStatus::E_STANDBY) &&
                    (eDecisionSensorOnlineResult == sensorcontrol::SensorStatus::E_STANDBY)) ||
                    ((eSensorOnlineResult == sensorcontrol::SensorStatus::E_NONE) &&
                    (eDecisionSensorOnlineResult == sensorcontrol::SensorStatus::E_STANDBY));
            //
            if(!isStandby)
            {
                if(isStreaming)
                {
                    // reset idle count
                    pSensorStateInfo.iIdleCount = 0; // reset
                }
                if(isGoToStreaming)
                {
                    parsedSensorStateResult.vResumeSensorIdList.push_back(id);
                    MY_LOGD("sensorId (%d) want to go to streaming", id);
                    pSensorStateInfo.eSensorOnlineStatus = sensorcontrol::SensorStatus::E_STREAMING;
                }
                if(isGoToStandby && ((++pSensorStateInfo.iIdleCount) > MAX_IDLE_DECIDE_FRAME_COUNT))
                {
                    parsedSensorStateResult.vGotoStandbySensorIdList.push_back(id);
                    // need add to streaming list, otherwise vStreamingSensorIdList doesn't
                    // contain this id.
                    parsedSensorStateResult.vStreamingSensorIdList.push_back(id);
                    pSensorStateInfo.iIdleCount = 0; // reset
                    MY_LOGD("sensorId (%d) want to go to standby", id);
                    pSensorStateInfo.eSensorOnlineStatus = sensorcontrol::SensorStatus::E_STANDBY;
                }
                else
                {
                    parsedSensorStateResult.vStreamingSensorIdList.push_back(id);
                    MY_LOGD_IF(mLogLevel >= 1, "sensorId (%d) streaming isGoToStandby(%d)", id, isGoToStandby);
                }
            }
            else
            {
                parsedSensorStateResult.vStandbySensorIdList.push_back(id);
            }
            MY_LOGD_IF(mLogLevel >= 1, "sensorId (%d) isStreaming(%d), isStandby(%d), isGoToStreaming(%d), isGoToStandby(%d)",
                id, isStreaming, isStandby, isGoToStreaming, isGoToStandby);
        }
        else
        {
            MY_LOGE("cannot find id(%d) in mvSensorStateList", id);
            return false;
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorStateManager::
resetIdleCountToAllSensors()
{
    for(auto&& item:mvSensorStateList) {
        item.second.iIdleCount = 0;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
RequestSensorControlPolicy_Multicam::
RequestSensorControlPolicy_Multicam(
    CreationParams const& params
) : mPolicyParams(params)
{
    // create 3a object
    mpHal3AObj =
            std::shared_ptr<Hal3AObject>(
                    new Hal3AObject(
                        mPolicyParams.pPipelineStaticInfo->sensorId));
    // create device info helper
    mpDeviceInfoHelper =
            std::shared_ptr<DeviceInfoHelper>(
                    new DeviceInfoHelper(
                        mPolicyParams.pPipelineStaticInfo->openId));

    for(auto&& sensorId:mPolicyParams.pPipelineStaticInfo->sensorId)
    {
        // set sensor init status.
        mvSensorStatus.insert({sensorId, sensorcontrol::SensorStatus::E_NONE});
    }

    auto multicamInfo = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->pParsedMultiCamInfo;
    auto max_streaming_size = 2;
    if(multicamInfo)
    {
        max_streaming_size = multicamInfo->mSupportPass1Number;
        if(multicamInfo->mDualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_ZOOM)
        {
            mFeatureMode = sensorcontrol::FeatureMode::E_Zoom;
        }
        else if(multicamInfo->mDualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)
        {
            mFeatureMode = sensorcontrol::FeatureMode::E_Bokeh;
        }
        else
        {
            mFeatureMode = sensorcontrol::FeatureMode::E_Multicam;
        }
        MY_LOGD("mFeatureMode(%" PRIu32 ")", mFeatureMode);
    }
    else
    {
        mFeatureMode = sensorcontrol::FeatureMode::E_None;
    }

    mLogLevel = ::property_get_int32("vendor.debug.policy.sensorcontrol", -1);

    mSensorStateManager =
            std::unique_ptr<SensorStateManager,
                            std::function<void(SensorStateManager*)> >(
                                new SensorStateManager(
                                    mPolicyParams.pPipelineStaticInfo->sensorId,
                                    max_streaming_size,
                                    mLogLevel),
                                [](SensorStateManager *p)
                                {
                                    if(p)
                                    {
                                        delete p;
                                    }
                                });

    mBehaviorManager =
            std::unique_ptr<SensorControllerBehaviorManager,
                            std::function<void(SensorControllerBehaviorManager*)> >(
                                new SensorControllerBehaviorManager(
                                    mLogLevel),
                                [](SensorControllerBehaviorManager *p)
                                {
                                    if(p)
                                    {
                                        delete p;
                                    }
                                });
}
/******************************************************************************
 *
 ******************************************************************************/
RequestSensorControlPolicy_Multicam::
~RequestSensorControlPolicy_Multicam()
{
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
evaluateRequest(
    requestsensorcontrol::RequestOutputParams& out,
    requestsensorcontrol::RequestInputParams const& in
) -> int
{
    if(in.pRequest_SensorMode == nullptr)
    {
        MY_LOGE("in.pRequest_SensorMode is null, please check flow");
        return UNKNOWN_ERROR;
    }
    if(out.pMultiCamReqOutputParams == nullptr)
    {
        MY_LOGE("out.pMultiCamReqOutputParams is null");
        return UNKNOWN_ERROR;
    }
    // check is capture req
    if(in.needP2CaptureNode)
    {
        isCaptureing = true;
    }
    else
    {
        isCaptureing = false;
    }
    //
    sensorcontrol::SensorControlParamIn senControlParamIn;
    sensorcontrol::SensorControlParamOut senControlParamOut;
    ParsedSensorStateResult parsedSensorStateResult;
    senControlParamIn.mFeatureMode = mFeatureMode;
    senControlParamIn.mRequestNo = in.requestNo;
    //
    getPolicyData(senControlParamIn, in);
    getDecisionResult(senControlParamOut, senControlParamIn);
    // cache data
    updateCacheData(senControlParamOut);
    dumpSensorControlParam(senControlParamOut, senControlParamIn, in);
    if(mSensorStateManager != nullptr)
    {
        if(mSensorStateManager->update(senControlParamOut, parsedSensorStateResult, isCaptureing))
        {
            // if it needs switch sensor, it has to set AAO and check framesync.
            /*if(parsedSensorStateResult.vNeedOfflineSensorList.size() > 0 ||
               parsedSensorStateResult.vNeedOnlineSensorList.size() > 0)
            {
                mbNeedQueryAAOState = true;
                mbNeedQeuryFrameSyncState = true;
                // current streaming.
                for(auto&& id:parsedSensorStateResult.vStreamingSensorIdList)
                {
                    mvAAOMoniterList.push_back(id);
                }
                // store current need online list.
                for(auto&& id:parsedSensorStateResult.vNeedOnlineSensorList)
                {
                    mvAAOMoniterList.push_back(id);
                }
                MY_LOGI("start aao/framesync check");
            }*/
            // update metadata and specific behavior control
            if(mBehaviorManager != nullptr)
            {
                BehaviorUserData userData;
                // set ae init setting for 2A.
                {
                    std::lock_guard<std::mutex> lk(mCapControlLock);
                    for(auto&& resume_id:parsedSensorStateResult.vResumeSensorIdList)
                    {
                        // for go to streaming path, it has to set init ae to other cam.
                        auto ret = mpHal3AObj->setSync2ASetting(mCurrentActiveMasterId, resume_id);
                        userData.enableSyncAf = true;
                        MY_LOGD("enable syncaf");
                    }
                }
                // query aao status only used for sensor switch flow.
                // used to check online list sensor are ready to output aao or not.
                /*auto getAaoState = [
                                    this,
                                    &parsedSensorStateResult]() -> bool
                {
                    if(mpHal3AObj != nullptr)
                    {
                        auto ret = mpHal3AObj->isAaoReady(mvAAOMoniterList);
                        if(ret)
                        {
                            mbNeedQueryAAOState = false;
                            mvAAOMoniterList.clear();
                            MY_LOGI("stop aao check");
                        }
                        return ret;
                    }
                    return true;
                };
                auto getFramesyncState = [
                                    this](int32_t masterId) -> bool
                {
                    if(mpHal3AObj != nullptr)
                    {
                        auto ret = mpHal3AObj->isFramesyncReady(masterId);
                        if(ret)
                        {
                            mbNeedQeuryFrameSyncState = false;
                            MY_LOGI("stop framesync check");
                        }
                        return ret;
                    }
                    return false;
                };*/
                userData.parsedResult = &parsedSensorStateResult;
                userData.requestNo = in.requestNo;
                userData.masterId = mSensorStateManager->getMasterId();
                userData.vSensorId = mPolicyParams.pPipelineStaticInfo->sensorId;
                userData.vSensorMode = *in.pRequest_SensorMode;
                //userData.bNeedP2Capture = in.needP2CaptureNode;
                //userData.bNeedFusion = in.needFusion;
                userData.isSensorSwitchGoing = mSensorStateManager->isSensorSwitchGoing();
                userData.isAaoReadyForStreaming = true; // always true.
                                                 //(mbNeedQueryAAOState)?getAaoState():true;
                userData.isFrameSyncReadyForStreaming = true; // always true, no needs to check this.
                                    /*(userData.isAaoReadyForStreaming)?
                                        ((mbNeedQeuryFrameSyncState)?
                                            getFramesyncState(mSensorStateManager->getMasterId()):true)
                                    :false;*/
                if(parsedSensorStateResult.vStreamingSensorIdList.size() > 1 &&
                    mbNeedNotify3ASyncDone)
                {
                    userData.isNeedNotifySync3A = true;
                    out.pMultiCamReqOutputParams->need3ASwitchDoneNotify = true;
                    mbNeedNotify3ASyncDone = false;
                    userData.enableSyncAf = true;
                    MY_LOGI("set 3a notify, enable sync af");
                }
                auto masterId = mSensorStateManager->getMasterId();
                out.pMultiCamReqOutputParams->masterId = masterId;
                out.pMultiCamReqOutputParams->prvMasterId = mCurrentActiveMasterId;
                if(mFeatureMode != sensorcontrol::FeatureMode::E_Zoom)
                {
                    // zoom has special sync af control.
                    // zoom no needs always enable sync af.
                    userData.enableSyncAf = true;
                }
                mBehaviorManager->update(out, userData);
            }
            // In switch flow, it needs master and slave id for wake up next 3a setting.
            // cannot set this before mBehaviorManager->update.
            // Otherwise, sync3a will work incorrect timing.
            if(parsedSensorStateResult.vNeedOnlineSensorList.size() > 0 &&
               parsedSensorStateResult.vStreamingSensorIdList.size() > 0)
            {
                out.pMultiCamReqOutputParams->switchControl_Sync2ASensorList.push_back(parsedSensorStateResult.vStreamingSensorIdList[0]);
                for(auto&& id:parsedSensorStateResult.vNeedOnlineSensorList)
                {
                    // sync 2a only support 2 sensor, when add success break this loop.
                    out.pMultiCamReqOutputParams->switchControl_Sync2ASensorList.push_back(id);
                    break;
                }
            }
            // data clone: todo
            out.pMultiCamReqOutputParams->flushSensorIdList = parsedSensorStateResult.vNeedOfflineSensorList;
            out.pMultiCamReqOutputParams->configSensorIdList = parsedSensorStateResult.vNeedOnlineSensorList;
            out.pMultiCamReqOutputParams->goToStandbySensorList = parsedSensorStateResult.vGotoStandbySensorIdList;
            out.pMultiCamReqOutputParams->streamingSensorList = parsedSensorStateResult.vStreamingSensorIdList;
            out.pMultiCamReqOutputParams->standbySensorList = parsedSensorStateResult.vStandbySensorIdList;
            out.pMultiCamReqOutputParams->resumeSensorList = parsedSensorStateResult.vResumeSensorIdList;
            // after decide streaming sensor, it has to update list which provide by P2NodeDecision
            updateP2List(in, out, parsedSensorStateResult);
            {
                // push data to streaming list.
                std::lock_guard<std::mutex> lk(mCapControlLock);
                mvReqSensorStreamingMap.emplace(in.requestNo, out.pMultiCamReqOutputParams->streamingSensorList);
            }
        }
        else
        {
            MY_LOGE("update sensor state fail");
            goto lbExit;
        }
    }
lbExit:
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
sendPolicyDataCallback(
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3
) -> bool
{
    std::lock_guard<std::mutex> lk(mCapControlLock);
    uint32_t type = *(uint32_t*)arg1;
    if(type == SetMasterId)
    {
        uint32_t frameno = *(uint32_t*)arg2;
        uint32_t masterid = *(uint32_t*)arg3;
        MY_LOGD("recive type(%" PRIu32 ") frameno(%" PRIu32 ") master(%" PRIu32 ")",
                    type, frameno, masterid);
        mCurrentActiveMasterId = masterid;
        mCurrentActiveFrameNo = frameno;
        {
            // remove old request data
            for(auto iter = mvReqSensorStreamingMap.begin();
                iter != mvReqSensorStreamingMap.end();)
            {
                if(iter->first < mCurrentActiveFrameNo) {
                    iter = mvReqSensorStreamingMap.erase(iter);
                }
                else {
                    ++iter;
                }
            }
        }
    }
    else if(type == SwitchDone)
    {
        MY_LOGI("switch done");
        if(mSensorStateManager != nullptr)
        {
            mSensorStateManager->switchSensorDone();
            mbNeedNotify3ASyncDone = true;
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
getPolicyData(
    sensorcontrol::SensorControlParamIn &sensorControlIn,
    requestsensorcontrol::RequestInputParams const& in
) -> bool
{
    auto pAppMetadata = in.pRequest_AppControl;
    if(pAppMetadata == nullptr)
    {
        MY_LOGE("create pAppMetadata fail");
        return false;
    }
    sensorControlIn.vSensorIdList = mPolicyParams.pPipelineStaticInfo->sensorId;
    // (1) set sensor related information. (3A data, sensor info, app data)
    for(auto&& sensorId:sensorControlIn.vSensorIdList)
    {
        auto pInfo = std::make_shared<sensorcontrol::SensorControlInfo>();
        if(pInfo == nullptr)
        {
            MY_LOGE("create SensorControlInfo fail");
            return false;
        }
        get3AData(sensorId, pInfo);
        getSensorData(sensorId, pInfo);
        sensorControlIn.vInfo.insert({sensorId, pInfo});
    }
    // get crop region from app metadata.
    MRect crop_region;
    if(IMetadata::getEntry<MRect>(pAppMetadata, MTK_SCALER_CROP_REGION, crop_region))
    {
        MY_LOGD_IF(mLogLevel >= 1, "crop region(%d, %d, %d, %d)",
                                                crop_region.p.x,
                                                crop_region.p.y,
                                                crop_region.s.w,
                                                crop_region.s.h);
    }
    sensorControlIn.mrCropRegion = crop_region;
    mpDeviceInfoHelper->getLogicalFocalLength(sensorControlIn.mAvailableFocalLength);
    if(IMetadata::getEntry<MFLOAT>(in.pRequest_AppControl, MTK_LENS_FOCAL_LENGTH, sensorControlIn.mFocalLength))
    {
        MY_LOGD_IF(mLogLevel >= 1, "focal_length(%f)", sensorControlIn.mFocalLength);
    }
    // get zoom ratio
    IMetadata::getEntry<MFLOAT>(pAppMetadata, MTK_MULTI_CAM_ZOOM_VALUE, sensorControlIn.mZoomRatio);
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
get3AData(
    int32_t sensorId,
    std::shared_ptr<sensorcontrol::SensorControlInfo> &info
) -> bool
{
    if(info == nullptr)
    {
        MY_LOGE("SensorControlInfo is nullptr, sensorId (%d)", sensorId);
        return false;
    }
    // update 3a info first
    if(mpHal3AObj == nullptr)
    {
        MY_LOGE("mpHal3AObj is nullptr");
        return false;
    }
    mpHal3AObj->update(); // update 3a cache info.
    info->bAFDone = mpHal3AObj->isAFDone(sensorId);
    info->iAEIso = mpHal3AObj->getAEIso(sensorId);
    info->iAELV_x10 = mpHal3AObj->getAELv_x10(sensorId);
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
getSensorData(
    int32_t sensorId,
    std::shared_ptr<sensorcontrol::SensorControlInfo> &info
) -> bool
{
    if(info == nullptr)
    {
        MY_LOGE("SensorControlInfo is nullptr, sensorId (%d)", sensorId);
        return false;
    }
    if(mpDeviceInfoHelper == nullptr)
    {
        MY_LOGE("mpDeviceInfoHelper is nullptr, sensorId (%d)", sensorId);
        return false;
    }
    mpDeviceInfoHelper->getPhysicalSensorFov(
                                    sensorId,
                                    info->fSensorFov_H,
                                    info->fSensorFov_V);
    mpDeviceInfoHelper->getPhysicalSensorActiveArray(
                                    sensorId,
                                    info->mActiveArrayRegion);
    mpDeviceInfoHelper->getPhysicalSensorFullSize(
                                    sensorId,
                                    info->msSensorFullSize);
    mpDeviceInfoHelper->getPhysicalFocalLength(
                                    sensorId,
                                    info->mFocalLength);
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
getDecisionResult(
    sensorcontrol::SensorControlParamOut &out,
    sensorcontrol::SensorControlParamIn &in
) -> bool
{
    auto ret = sensorcontrol::decistion_sensor_control(out, in);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
updateCacheData(
    sensorcontrol::SensorControlParamOut &out
) -> bool
{
    for(auto&& item:out.vResult)
    {
        auto iter = mvSensorStatus.find(item.first);
        if(iter != mvSensorStatus.end())
        {
            iter->second = item.second->iSensorControl;
        }
        else
        {
            MY_LOGE("cannot get sensor status in mvSensorStatus(%d)", item.first);
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
updateP2List(
    requestsensorcontrol::RequestInputParams const& in,
    requestsensorcontrol::RequestOutputParams & out,
    ParsedSensorStateResult const& result
) -> bool
{
    // check preview master is exist in streaming list first.
    uint32_t prvMasterId = -1;
    {
        std::lock_guard<std::mutex> lk(mCapControlLock);
        {
            prvMasterId = mCurrentActiveMasterId;
            auto prvMasterIter =
                    std::find(result.vStreamingSensorIdList.begin(),
                              result.vStreamingSensorIdList.end(),
                              mCurrentActiveMasterId);
            if(prvMasterIter == result.vStreamingSensorIdList.end())
            {
                if(result.vStreamingSensorIdList.size() > 0)
                {
                    prvMasterId = result.vStreamingSensorIdList[0];
                    MY_LOGI("capture request, preview master not exist in streaming list. assign first element.");
                }
                else
                {
                    MY_LOGI("capture request, but no streaming sensor. request fail");
                    return false;
                }
            }
        }
    }

    auto checkPhysicalLsitAndRemoveItemBySensorId = [&out](uint32_t id)
    {
        // remove data from p2 list.
        auto removeItemBySensorId = [](
                                    uint32_t &targetId,
                                    RequestOutputParams::StreamIdMap* pMap)
        {
            if(pMap == nullptr) return false;
            bool ret = false;
            auto iter = pMap->find(targetId);
            if(iter != pMap->end())
            {
                pMap->erase(iter);
                ret = true;
            }
            return ret;
        };
        bool hasMarkError = false;
        hasMarkError |= removeItemBySensorId(id, out.vMetaStreamId_from_CaptureNode_Physical);
        hasMarkError |= removeItemBySensorId(id, out.vImageStreamId_from_CaptureNode_Physical);
        hasMarkError |= removeItemBySensorId(id, out.vMetaStreamId_from_StreamNode_Physical);
        hasMarkError |= removeItemBySensorId(id, out.vImageStreamId_from_StreamNode_Physical);
        return hasMarkError;
    };

    auto updateStreamandMasterId = [&out](uint32_t const& masterId){
        out.pMultiCamReqOutputParams->prvMasterId = masterId;
        out.pMultiCamReqOutputParams->prvStreamingSensorList.push_back(masterId);
    };

    MINT32 isCShot = false;
    IMetadata::getEntry<MINT32>(in.pRequest_AppControl, MTK_CSHOT_FEATURE_CAPTURE, isCShot);
    // if current output sensor is standby, remove it from p2 list.
    for(auto&& id:mPolicyParams.pPipelineStaticInfo->sensorId)
    {
        // 1. If sensor ID not exist in vStreamingSensorIdList, it means
        //    standby or offline.
        // 2. If CShot occured, it has to remove none master id streams.
        if(std::find(
                    result.vStreamingSensorIdList.begin(),
                    result.vStreamingSensorIdList.end(),
                    id) == result.vStreamingSensorIdList.end() ||
          (isCShot && (id != mCurrentActiveMasterId)))
        {
            if(checkPhysicalLsitAndRemoveItemBySensorId(id))
                out.pMultiCamReqOutputParams->markErrorSensorList.push_back(id);
        }
    }
    {
        // for capture update.
        std::lock_guard<std::mutex> lk(mCapControlLock);
        // special flow for tk zoom: only support capture master cam.

        if(in.needP2CaptureNode && mCurrentActiveFrameNo == -1) // If first request is capturing, just assign streamingSensorList
        {
            MY_LOGD("First request is capturing");
            auto list = out.pMultiCamReqOutputParams->streamingSensorList;
            out.pMultiCamReqOutputParams->prvMasterId = out.pMultiCamReqOutputParams->masterId;
            out.pMultiCamReqOutputParams->prvStreamingSensorList.assign(list.begin(), list.end());
        }
        else if(in.needP2CaptureNode && !in.needFusion &&
           in.bLogicalCaptureOutput)
        {
            // master id is current active id, not get from sensor state manager.
            MY_LOGD("only capture master id(%" PRIu32 ")", prvMasterId);
            updateStreamandMasterId(prvMasterId);
        }
        else if(in.bLogicalRawOutput)
        {
            if(mCurrentActiveFrameNo == -1)
            {
                auto id = out.pMultiCamReqOutputParams->streamingSensorList[0];
                out.pMultiCamReqOutputParams->prvMasterId = id;
                out.pMultiCamReqOutputParams->prvStreamingSensorList.push_back(id);
                MY_LOGD("[Raw-1] only capture master id(%" PRIu32 ")", id);
            }
            else
            {
                // master id is current active id, not get from sensor state manager.
                MY_LOGD("only capture master id(%" PRIu32 ")", prvMasterId);
                updateStreamandMasterId(prvMasterId);
            }
        }
        else if(in.needP2CaptureNode)
        {
            if(isCShot)
            {
                MY_LOGD("[Raw] only capture master id(%" PRIu32 ")", prvMasterId);
                updateStreamandMasterId(prvMasterId);
            }
            else
            {
                auto iter = mvReqSensorStreamingMap.find(mCurrentActiveFrameNo);
                if(iter != mvReqSensorStreamingMap.end())
                {
                    updateStreamandMasterId(prvMasterId);
                    for(auto&& id:iter->second)
                    {
                        auto invStreamIter =
                        std::find(result.vStreamingSensorIdList.begin(),
                                  result.vStreamingSensorIdList.end(),
                                  id);
                        if(prvMasterId != id && invStreamIter != result.vStreamingSensorIdList.end())
                        {
                            out.pMultiCamReqOutputParams->prvStreamingSensorList.push_back(id);
                        }
                    }
                }
                else
                {
                    MY_LOGE("cannot find current active frame number [%" PRIu32 "]", in.requestNo);
                }
            }
        }
    }

    if(out.pMultiCamReqOutputParams->prvStreamingSensorList.size()>0)
    {
        android::String8 temp("prv streaming: ");
        for(auto&& id:out.pMultiCamReqOutputParams->prvStreamingSensorList)
        {
            temp.appendFormat("%" PRIu32 " ", id);
        }
        MY_LOGD("%s", temp.string());
    }

    if(out.pMultiCamReqOutputParams->markErrorSensorList.size()>0)
    {
        android::String8 temp("mark error: ");
        for(auto&& id:out.pMultiCamReqOutputParams->markErrorSensorList)
        {
            temp.appendFormat("%" PRIu32 " ", id);
        }
        MY_LOGD("%s", temp.string());
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
RequestSensorControlPolicy_Multicam::
dumpSensorControlParam(
    sensorcontrol::SensorControlParamOut &out,
    sensorcontrol::SensorControlParamIn &in,
    requestsensorcontrol::RequestInputParams const& req_in
) -> bool
{
    if(mLogLevel >= 2) {
        if(!mpHal3AObj)
        {
            MY_LOGE("mpHal3AObj instance not exist.");
            return false;
        }
        android::String8 dump_str("");
        dump_str.appendFormat("[Input]\nreq:(%d)\n", req_in.requestNo);
        dump_str.appendFormat("sensor list:\n");
        for(auto&& sensorId : in.vSensorIdList)
        {
            dump_str.appendFormat("%d", sensorId);
        }
        dump_str.appendFormat("\nSensor control info:\n");
        for(auto&& item : in.vInfo)
        {
            dump_str.appendFormat("id: (%d)\n", item.first);
            dump_str.appendFormat("iSensorStatus:(%d) bAFDone:(%d) iAEIso:(%d) iAELV_x10:(%d)"
                                  " eSensorStatus:(%d) fSensorFov_H:(%f) fSensorFov_V:(%f)"
                                  " msSensorFullSize:(%dx%d) mActiveArrayRegion(%d,%d, %dx%d)\n",
                                  item.second->iSensorStatus, item.second->bAFDone,
                                  item.second->iAEIso, item.second->iAELV_x10,
                                  item.second->eSensorStatus, item.second->fSensorFov_H,
                                  item.second->fSensorFov_V, item.second->msSensorFullSize.w,
                                  item.second->msSensorFullSize.h, item.second->mActiveArrayRegion.p.x,
                                  item.second->mActiveArrayRegion.p.y, item.second->mActiveArrayRegion.s.w,
                                  item.second->mActiveArrayRegion.s.h);
            dump_str.appendFormat("bSync2ADone(%d)", mpHal3AObj->is2ASyncReady(item.first));
        }
        dump_str.appendFormat("mrCropRegion: (%d,%d, %dx%d)",
                                in.mrCropRegion.p.x,
                                in.mrCropRegion.p.y,
                                in.mrCropRegion.s.w,
                                in.mrCropRegion.s.h);

        dump_str.appendFormat("[Output]\n");
        dump_str.appendFormat("zoom ratio:(%f)", out.fZoomRatio);
        MY_LOGD("%s", dump_str.string());
        for(auto&& item : out.vResult)
        {
            MY_LOGD("id: (%d), iSensorControl:(%d) fAlternavtiveFov_H:(%f) fAlternavtiveFov_V:(%f) mrAlternactiveCropRegion(%d,%d, %dx%d)"
                                  " isMaster(%d) mrAlternactiveCropRegion_Cap(%d,%d, %dx%d)\n", item.first,
                                  item.second->iSensorControl, item.second->fAlternavtiveFov_H,
                                  item.second->fAlternavtiveFov_V, item.second->mrAlternactiveCropRegion.p.x,
                                  item.second->mrAlternactiveCropRegion.p.y, item.second->mrAlternactiveCropRegion.s.w,
                                  item.second->mrAlternactiveCropRegion.s.h, item.second->isMaster,
                                  item.second->mrAlternactiveCropRegion_Cap.p.x, item.second->mrAlternactiveCropRegion_Cap.p.y,
                                  item.second->mrAlternactiveCropRegion_Cap.s.w, item.second->mrAlternactiveCropRegion_Cap.s.h);
        }
    }
    return true;
}
/******************************************************************************
 * Make a function target as a policy - multicam version
 ******************************************************************************/
std::shared_ptr<IRequestSensorControlPolicy>
                makePolicy_RequestSensorControl_Multicam_Zoom(
                    CreationParams const& params)
{
    return std::make_shared<RequestSensorControlPolicy_Multicam>(params);
}
/******************************************************************************
 *
 ******************************************************************************/
SensorControllerBehaviorManager::
SensorControllerBehaviorManager(
    int32_t logLevel
)
{
    // create behavior.
    mvBehaviorList.insert(
                    {BEHAVIOR::Sync3A,
                    std::make_shared<Sync3ABehavior>(logLevel)});
    mvBehaviorList.insert(
                    {BEHAVIOR::ISP,
                    std::make_shared<IspBehavior>(logLevel)});
    mvBehaviorList.insert(
                    {BEHAVIOR::FrameSync,
                    std::make_shared<FrameSyncBehavior>(logLevel)});
    mvBehaviorList.insert(
                    {BEHAVIOR::SensorCrop,
                    std::make_shared<SensorCropBehavior>(logLevel)});
    //
    mLogLevel = logLevel;
}
/******************************************************************************
 *
 ******************************************************************************/
SensorControllerBehaviorManager::
~SensorControllerBehaviorManager()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorControllerBehaviorManager::
update(
    RequestOutputParams& out,
    BehaviorUserData const& userData
)
{
    if(!userData.isSensorSwitchGoing)
    {
        for(auto&& item:mvBehaviorList)
        {
            auto pBehavior = item.second;
            if(pBehavior != nullptr)
            {
                pBehavior->update(out, userData);
            }
        }
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
Sync3ABehavior::
~Sync3ABehavior()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
Sync3ABehavior::
update(
    RequestOutputParams& out,
    BehaviorUserData const& userData
)
{
    // if streaming list less than 2, skip behavior.
    if(userData.parsedResult->vStreamingSensorIdList.size() < 2 ||
       userData.parsedResult->vGotoStandbySensorIdList.size() > 0)
    {
        // for single cam, no need to sync2a.
        // if someone wants to standby, no need to set sync 2A.
        out.pMultiCamReqOutputParams->needSync2A = false;
        out.pMultiCamReqOutputParams->needSyncAf = false;
        return true;
    }
    /*if(userData.bNeedP2Capture && !userData.bNeedFusion)
    {
        MY_LOGD("capture with no fusion. ignore 3a update");
        return true;
    }*/
    //if(userData.isAaoReadyForStreaming)
    {
        out.pMultiCamReqOutputParams->needSync2A = true;
    }

    // check sensors support AF or not
    out.pMultiCamReqOutputParams->needSyncAf = true;
    for(auto idx : userData.vSensorId)
    {
        bool isAF = false;
        auto got = mvIsSensorAf.find(idx);

        if(got == mvIsSensorAf.end())
        {
            isAF = StereoSettingProvider::isSensorAF(idx);
            mvIsSensorAf.emplace(idx, isAF);
        }
        else
        {
            isAF = got->second;
        }

        if(isAF == false)
        {
            // false if one is not AF
            out.pMultiCamReqOutputParams->needSyncAf = false;
            break;
        }
    }
    if(!userData.enableSyncAf)
    {
        out.pMultiCamReqOutputParams->needSyncAf = false;
    }
    if(userData.isNeedNotifySync3A)
    {
        out.pMultiCamReqOutputParams->need3ASwitchDoneNotify = true;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
IspBehavior::
~IspBehavior()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
IspBehavior::
update(
    RequestOutputParams& out __attribute__((unused)),
    BehaviorUserData const& userData __attribute__((unused))
)
{
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
FrameSyncBehavior::
~FrameSyncBehavior()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
FrameSyncBehavior::
update(
    RequestOutputParams& out,
    BehaviorUserData const& userData
)
{
    bool needSync = false;
    // if streaming list is less than 2, skip this module.
    if(userData.parsedResult->vStreamingSensorIdList.size() >= 2)
    {
        needSync = true;
    }
    out.pMultiCamReqOutputParams->needFramesync = needSync;
    out.pMultiCamReqOutputParams->needSynchelper_3AEnq = false;  // no need 3A enq sync.
    out.pMultiCamReqOutputParams->needSynchelper_Timestamp = needSync;
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
SensorCropBehavior::
~SensorCropBehavior()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorCropBehavior::
update(
    RequestOutputParams& out,
    BehaviorUserData const& userData
)
{
    if(needBuildTransformMatrix)
    {
        auto sensorId = userData.vSensorId;
        auto sensorMode = userData.vSensorMode;
        mDomainConvert =
            std::unique_ptr<DomainConvert,
                            std::function<void(DomainConvert*)> >(
                                new DomainConvert(
                                    sensorId,
                                    sensorMode),
                                [](DomainConvert *p)
                                {
                                    if(p)
                                    {
                                        delete p;
                                    }
                                });
        needBuildTransformMatrix = false;
    }
    updateSensorCrop(out, userData);
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorCropBehavior::
updateSensorCrop(
    RequestOutputParams& out,
    BehaviorUserData const& userData
)
{
    MRect sensorCrop;
    for(auto&& id:userData.parsedResult->vStreamingSensorIdList)
    {
        if(getSensorCropRect(id, userData.parsedResult->vAlternactiveCropRegion, sensorCrop))
        {
            MRect tgCropRect;
            if(mDomainConvert->convertToTgDomain(id, sensorCrop, tgCropRect)) {
                out.pMultiCamReqOutputParams->tgCropRegionList.emplace(id, tgCropRect);
            }
            out.pMultiCamReqOutputParams->sensorScalerCropRegionList.emplace(id, sensorCrop);
        }
    }
    for(auto&& item:userData.parsedResult->vAlternactiveCropRegion_Cap)
    {
        MRect tgCropRect;
        if(mDomainConvert->convertToTgDomain(item.first, item.second, tgCropRect)) {
            out.pMultiCamReqOutputParams->tgCropRegionList_Cap.emplace(item.first, tgCropRect);
        }
        out.pMultiCamReqOutputParams->sensorScalerCropRegionList_Cap.emplace(item.first, item.second);
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SensorCropBehavior::
getSensorCropRect(
    int32_t sensorId,
    std::unordered_map<uint32_t, MRect> const& vCropRegion,
    MRect &cropRect
)
{
    auto iter_sensor_param =
                vCropRegion.find(sensorId);
    auto check_iter_sensor_param =
                (iter_sensor_param != vCropRegion.end());
    if(check_iter_sensor_param)
    {
        cropRect = iter_sensor_param->second;
    }
    else
    {
        MY_LOGE("cannot get sensor crop info in vAlternactiveCropRegion. (%d)",
                                sensorId);
        return false;
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
DomainConvert::
DomainConvert(
    std::vector<int32_t> &sensorIdList,
    std::vector<uint32_t> &sensorModeList
)
{
    MY_LOGD("build covert matrix");
    buildTransformMatrix(sensorIdList, sensorModeList);
}

/******************************************************************************
 *
 ******************************************************************************/
DomainConvert::
~DomainConvert()
{
    MY_LOGD("release domain convert");
}

/******************************************************************************
 *
 ******************************************************************************/
auto
DomainConvert::
convertToTgDomain(
    int32_t sensorId,
    MRect &srcRect,
    MRect &dstRect
) -> bool
{
    auto iter = mvTransformMatrix.find(sensorId);
    if(iter != mvTransformMatrix.end())
    {
        auto matrix = iter->second;
        matrix.transform(srcRect, dstRect);
    }
    else
    {
        MY_LOGE("cannot get transform matrix");
        return false;
    }
    MY_LOGD_IF(0, "transform done: src(%d, %d, %dx%d) dst(%d, %d, %dx%d)",
            srcRect.p.x, srcRect.p.y, srcRect.s.w, srcRect.s.h,
            dstRect.p.x, dstRect.p.y, dstRect.s.w, dstRect.s.h);
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
DomainConvert::
buildTransformMatrix(
    std::vector<int32_t> &sensorIdList,
    std::vector<uint32_t> &sensorModeList
) -> bool
{
    int index = 0;
    for(auto&& sensorId:sensorIdList)
    {
        NSCamHW::HwTransHelper helper(sensorId);
        NSCamHW::HwMatrix mat;
        if(index >= (int)sensorModeList.size())
        {
            MY_LOGE("index(%d) >= sensorModeList.size(%zu)",
                    index, sensorModeList.size());
            continue;
        }
        auto sensorMode = sensorModeList[index++];
        if(!helper.getMatrixFromActive(sensorMode, mat))
        {
            MY_LOGE("Get hw matrix failed. sensor id(%d) sensor mode(%d)",
                        sensorId,
                        sensorMode);
            return false;
        }
        mvTransformMatrix.insert({sensorId, mat});
        MY_LOGD("build transform matrix. sensor(%d) mode(%d)",
                        sensorId,
                        sensorMode);
    }
    return true;
}
};  //namespace requestsensorcontrol
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

