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

#define LOG_TAG "MtkCam/Util/LogicalDevice"

#include "MyUtils.h"
#include <mtkcam/utils/std/ULog.h>

#define CAM_ULOG_MODULE_ID MOD_UTILITY
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);
#include <cutils/properties.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <vector>

#include "LogicalDeviceMetadataHelper.h"
#include "VirtualDeviceIdsMapHelper.h"

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif

#include <kd_imgsensor_define.h>

#define MAX_SENSOR_NAME_SIZE (128)
#define MAX_DEVICE_NAME_SIZE (192)

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace android;
using namespace NSCam;
using namespace NSLogicalDeviceMetadataHelper;
using namespace NSVirtualDeviceIdsMapHelper;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Hal Sensor List Interface.
 ******************************************************************************/
class HalLogicalDeviceList
      : public IHalLogicalDeviceList
{
public:
                                    HalLogicalDeviceList();

protected:  ////                    Destructor.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~HalLogicalDeviceList() {}

public:     ////                    Attributes.

    /**
     * Query the number of logical devices.
     * This call is legal only after searchDevices().
     */
    MUINT                   queryNumberOfDevices() const override;

    /**
     * Query the number of image sensors.
     * This call is legal only after searchDevices().
     */
    MUINT                   queryNumberOfSensors() const override;

    /**
     * Query static information for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    IMetadata const&        queryStaticInfo(MUINT const index) const override;

    /**
     * Query the driver name for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    char const*             queryDriverName(MUINT const index) const override;

    /**
     * Query the sensor type of NSSensorType::Type for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    MUINT                   queryType(MUINT const index) const override;

    /**
     * Query the sensor facing direction for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    MUINT                   queryFacingDirection(
                                        MUINT const index
                                    ) const override;

    /**
     * Query SensorDev Index by sensor list index.
     * This call is legal only after searchSensors().
     * Return SENSOR_DEV_MAIN, SENSOR_DEV_SUB,...
     */
    MUINT                   querySensorDevIdx(MUINT const index) const override;

    /**
     * Query static SensorStaticInfo for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    SensorStaticInfo const* querySensorStaticInfo(
                                        MUINT const indexDual
                                    ) const override;

    /**
     * Query Sensor Information.
     * This call is legal only after searchSensors().
     */
    MVOID                   querySensorStaticInfo(
                                        MUINT indexDual,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const override;

    /**
     * Search sensors and return the number of image sensors.
     */
    MUINT                   searchDevices() override;

    std::vector<MINT32>     getSensorId(MUINT deviceId) override;

    std::vector<MINT32>     getRemappingSensorId(MUINT deviceId) override;

    MINT32                  getDeviceId(MINT32 sensorId) override;

    MINT32                  getSupportedFeature(MUINT deviceId) const override;

    SensorSyncType          getSyncType(MUINT deviceId) const override;

    MINT32                  getDeviceIdByVID(MUINT vid) override;

    MINT32                  getSensorSyncMasterDevId(MUINT deviceId) const override;

    bool                    updateMultiCamSensorMetadata(MINT32 sensorId, IMetadata &metadata);

    uint32_t                getVirtualInstanceId(uint32_t instanceId) const override;

    bool                    getVidByDrvName(std::string drvName, int32_t sensorId, uint32_t &vid) const;

    bool                    isLogicalMulticam(MUINT deviceId) const;

    bool                    getHiddenByDrvName(std::string drvName) const;

    bool                    isCameraHidden(uint32_t instanceId) const;
private:
    struct TempSensorInfo
    {
        MINT32 SensorId;
        MINT32 RemappingSensorId;
        MINT32 RawType;
        MINT32 Facing;
        MINT32 CaptureModeWidth;
        std::string Name;
        bool isHidden;
    };
    using SensorInfo_t = std::vector<struct TempSensorInfo>;

    class SyncTypeInfo : public virtual RefBase
    {
    public:
        SensorSyncType syncType = SensorSyncType::NOT_SUPPORT;
        MUINT32 masterDevId = 0xFF;
        std::vector<MUINT32> syncMode;
        std::vector<MUINT32> slaveDevId; //only one master, but can have more than 1 slave.
    };

    //
    // local function
    MINT32                          createDeviceMap();
    MINT32                          addLogicalDevice(SensorInfo_t vInfo, struct LogicalSensorStruct* pLogicalSen, MINT32 DevNum);
    void                            dumpDebugInfo();
    // query sync mode
    sp<SyncTypeInfo>                querySyncMode(std::vector<MINT32>& Sensors);

    class CamDeviceInfo : public virtual RefBase
    {
    public:
        std::vector<MINT32> Sensors;
        std::vector<MINT32> RemappingSensorId;
        std::vector<bool> isHidden;
        MUINT DualFeature;
        MINT32 RawType;
        std::string Name;
        sp<SyncTypeInfo> syncTypeInfo = nullptr;
        IMetadata sensorStaticMetadata;
        bool isLogicalMulticam = true;
    };

    // Logical camera device(s) defined in camera_custom_stereo_setting.h are
    // loaded by default. However, there are exceptions for certain circumstances.
    // In the light of reasons mention aboved, we add rules to check whether to
    // add the logical camera device(s) or not.
    bool isSkipThisLogicalDevice(const LogicalSensorStruct& logicalDevice,
        const sp<CamDeviceInfo>& camDeviceInfo);

    KeyedVector< MUINT, sp<CamDeviceInfo> > mDeviceSensorMap;
    mutable Mutex mLock;
    //
    sp<LogicalDeviceMetadataHelper> mpLogicalDeviceMetadataHelper = nullptr;
    sp<VirtualDeviceIdsMapHelper> mpVirtualDeviceIdsMapHelper = nullptr;
};


};  //namespace NSCam

static Mutex gHalLogicalDevCreateLock;


/******************************************************************************
 *
 ******************************************************************************/
IHalLogicalDeviceList* IHalLogicalDeviceList::get()
{
    Mutex::Autolock _l(gHalLogicalDevCreateLock);
    static HalLogicalDeviceList* gInst = new HalLogicalDeviceList();

    //MY_LOGD("gInst : %p", gInst);

    return gInst;
}

HalLogicalDeviceList::
HalLogicalDeviceList()
{
    // create logical device metadata helper.
    mpLogicalDeviceMetadataHelper = new LogicalDeviceMetadataHelper();
    // create virtual device ids map
    mpVirtualDeviceIdsMapHelper = new VirtualDeviceIdsMapHelper();
}

MINT32
HalLogicalDeviceList::
addLogicalDevice(SensorInfo_t vInfo, struct LogicalSensorStruct* pLogicalSen, MINT32 DevNum)
{
    sp<CamDeviceInfo> Info = new CamDeviceInfo();
    // check sensor count is match to NumofCombinSensor.
    Info->isLogicalMulticam = pLogicalSen->isLogicalMulticam;
    if((pLogicalSen->SensorIDs.size())%(pLogicalSen->NumofCombinSensor) != 0)
    {
        MY_LOGE("Sensor list count(%zu) does not match to combin sensor count(%d)",
                    pLogicalSen->SensorIDs.size(),
                    pLogicalSen->NumofCombinSensor);
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
        aee_system_exception(
            LOG_TAG,
            NULL,
            DB_OPT_DEFAULT,
            "logic device create fail.\nCRDISPATCH_KEY:logic device create fail.");
#endif
        return -1;
    }

    for(int i = 0; i < pLogicalSen->NumofCombinSensor; i++)
    {
        MINT32 idx = pLogicalSen->SensorIDs[DevNum*pLogicalSen->NumofCombinSensor + i];
        if(idx < 0)
        {
            return -1;
        }
        auto &sensorInfo = vInfo[idx];
        Info->Sensors.push_back(idx);
        Info->RemappingSensorId.push_back(sensorInfo.RemappingSensorId);
        Info->isHidden.push_back(sensorInfo.isHidden);
        if (i == 0)
        {
            Info->Name = sensorInfo.Name + "_" + pLogicalSen->Name;
        }
    }
    Info->DualFeature = pLogicalSen->Feature;
    Info->RawType = SENSOR_RAW_Bayer;
    Info->syncTypeInfo = querySyncMode(Info->Sensors);

    // construct static metadata from the primary physical image sensor and
    // update parts fo them for the logical camera device
    //
    // NOTE: Ground rules of static metadata loading mechanism
    // '_': file name belonging to config_static_metadata_*.h are parsed by
    //      android::NSMetadataProvider::MetadataProvider
    // '-': file name belonging to config_static_metadata-*.h are parsed by
    //      NSCam::HalLogicalDeviceList
    // '.': file name belonging to config_static_metadata.*.h are parsed by
    //      NSCam::NSHalSensor::HalSensorList
    {
        MY_LOGD("query static metadata for multi-cam (%s)", Info->Name.c_str());
        SensorStaticInfo sensorStaticInfo;
        auto sensorDevId = MAKE_HalSensorList()->querySensorDevIdx(Info->Sensors[0]);
        MAKE_HalSensorList()->querySensorStaticInfo(sensorDevId, &sensorStaticInfo);
        auto ret = mpLogicalDeviceMetadataHelper->constructStaticMetadata(
                mDeviceSensorMap.size(), // for current id, it is get DeviceSensorMap.size() (for index)
                Info->Name,
                (sensorStaticInfo.facingDirection==0)?true:false,
                Info->sensorStaticMetadata);
        if(ret != OK)
        {
            MY_LOGD("skip add to device list.");
            return 0;
        }

        // update sensor query metadata
        updateMultiCamSensorMetadata(Info->Sensors[0], Info->sensorStaticMetadata);
    }

    if (isSkipThisLogicalDevice(*pLogicalSen, Info))
    {
        MY_LOGW("custom dev(%s) is skipped", Info->Name.c_str());
        return -1;
    }

    MY_LOGI("add new logic device: %s", Info->Name.c_str());
    mDeviceSensorMap.add(mDeviceSensorMap.size(), Info);
    return 0;
}

void
HalLogicalDeviceList::
dumpDebugInfo()
{
    MY_LOGI_IF(1, "map size : %zu", mDeviceSensorMap.size());
    for(unsigned int i = 0; i < mDeviceSensorMap.size(); i++)
    {
        MY_LOGI_IF(1, "index(%u) name : %s", i, queryDriverName(i));
        MY_LOGI_IF(1, "index(%u) facing : %d", i, queryFacingDirection(i));
    }
}

sp<HalLogicalDeviceList::SyncTypeInfo>
HalLogicalDeviceList::
querySyncMode(
    std::vector<MINT32>& Sensors)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(CC_UNLIKELY(!pHalSensorList))
    {
        MY_LOGA("create hal sensor list fail");
    }
    // for sync, main sensor must support master mode, and other
    // sensors need support slave mode.
    String8 checkString("");
    sp<SyncTypeInfo> info = new SyncTypeInfo();
    std::vector<bool> masterCheckList;
    std::vector<bool> slaveCheckList;
    std::vector<MUINT32> devIdList;
    for(auto& sensorId : Sensors)
    {
        NSCam::IHalSensor* pHalSensor = pHalSensorList->createSensor(
                                            LOG_TAG,
                                            sensorId);
        if(CC_UNLIKELY(!pHalSensor))
        {
        MY_LOGA("create hal sensor fail");
        }
        MUINT32 syncMode = 0;
        MUINT32 sensorDevId = pHalSensorList->querySensorDevIdx(sensorId);
        MINT ret = pHalSensor->sendCommand(
                                sensorDevId,
                                SENSOR_CMD_GET_SENSOR_SYNC_MODE_CAPACITY,
                                (MUINTPTR)&syncMode,
                                0 /* unused */,
                                0 /* unused */);
        if(ret != 0)
        {
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
            aee_system_exception(
                LOG_TAG,
                NULL,
                DB_OPT_DEFAULT,
                "query sync mode fail.\nCRDISPATCH_KEY:query sync mode fail.");
#endif
        }
        info->syncMode.push_back(syncMode);
        if((syncMode & SENSOR_MASTER_SYNC_MODE) && (syncMode & SENSOR_SLAVE_SYNC_MODE))
        {
            masterCheckList.push_back(true);
            slaveCheckList.push_back(true);
        }
        else if(syncMode & SENSOR_MASTER_SYNC_MODE)
        {
            masterCheckList.push_back(true);
            slaveCheckList.push_back(false);
        }
        else if(syncMode & SENSOR_SLAVE_SYNC_MODE)
        {
            masterCheckList.push_back(false);
            slaveCheckList.push_back(true);
        }
        else
        {
            masterCheckList.push_back(false);
            slaveCheckList.push_back(false);
        }
        devIdList.push_back(sensorDevId);
        checkString.appendFormat("S[%d:D%d:M%d:S%d] ",
                                    sensorId,
                                    sensorDevId,
                                    !!(syncMode & SENSOR_MASTER_SYNC_MODE),
                                    !!(syncMode & SENSOR_SLAVE_SYNC_MODE));
    }
    MINT masterIndex = -1;
    MINT slaveIndex = -1;
    for(unsigned int i = 0;i<masterCheckList.size();++i)
    {
        if(masterCheckList[i])
        {
            masterIndex = (MINT)i;
            for(unsigned int j=0;j<slaveCheckList.size();++j)
            {
                if(i != j)
                {
                    if(slaveCheckList[j])
                    {
                        slaveIndex = (MINT)j;
                        break;
                    }
                }
            }
            if(slaveIndex != -1) break;
        }
    }
    // set master id
    if(masterIndex != -1)
    {
        info->masterDevId = devIdList[masterIndex];
        checkString.appendFormat("M[D:%d] ", devIdList[masterIndex]);
    }
    for(unsigned int i=0;i<slaveCheckList.size();++i)
    {
        if(((MINT)i != masterIndex) && slaveCheckList[i] )
        {
            info->slaveDevId.push_back(devIdList[i]);
            checkString.appendFormat("S[D:%d] ", devIdList[i]);
        }
    }
    checkString.appendFormat("Master[D:%d] SlaveList[%zu]", info->masterDevId, info->slaveDevId.size());
    if(info->masterDevId != 0xFF && info->slaveDevId.size() == (Sensors.size() -1))
    {
        info->syncType = SensorSyncType::CALIBRATED;
        checkString.appendFormat("R[Calibrated]");
    }
    else
    {
        info->syncType = SensorSyncType::APPROXIMATE;
        checkString.appendFormat("R[Approximate]");
    }
    MY_LOGI("%s", checkString.string());
    return info;
}

bool
HalLogicalDeviceList::
isSkipThisLogicalDevice(const LogicalSensorStruct& logicalDevice,
        const sp<CamDeviceInfo>& camDeviceInfo __attribute__((unused)))
{
    if (logicalDevice.isFeatureSupported(NSCam::DEVICE_FEATURE_SECURE_CAMERA))
    {
        // skip if this capability is not supported through the device build configuration
        // NOTE: this situation is expected due to the feature option may be not
        // enabled for certain devices of the same platform
#ifndef MTKCAM_SECURITY_SUPPORT
        MY_LOGI("skip adding this logical device(%s): secure camera is not supported",
            logicalDevice.Name.c_str());
        return true;
#endif
    }

    return false;
}

MINT32
HalLogicalDeviceList::
createDeviceMap()
{
    SensorInfo_t vTempInfo;
    unsigned int i = 0;

    // firstly, we create a logical camera device per physical camera
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(CC_UNLIKELY(!pHalSensorList))
    {
        MY_LOGA("create pHalSensorList fail");
    }
    size_t const sensorNum = pHalSensorList->searchSensors();
    MY_LOGD("sensorNum : %zu", sensorNum);
    for(i = 0; i < sensorNum; i++)
    {
        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        int sendorDevIndex = pHalSensorList->querySensorDevIdx(i);
        pHalSensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

        TempSensorInfo TempInfo;
        TempInfo.SensorId = i;
        TempInfo.RawType = sensorStaticInfo.rawFmtType;
        TempInfo.Facing = sensorStaticInfo.facingDirection;
        TempInfo.CaptureModeWidth = sensorStaticInfo.captureWidth;
        TempInfo.Name = pHalSensorList->queryDriverName(i);
        uint32_t remappingSensorId;
        if(getVidByDrvName(TempInfo.Name, i, remappingSensorId))
        {
            TempInfo.RemappingSensorId = (MINT32)remappingSensorId;
        }
        else
        {
            TempInfo.RemappingSensorId = i;
        }
        TempInfo.isHidden = getHiddenByDrvName(TempInfo.Name);
        vTempInfo.push_back(TempInfo);
        MY_LOGD("%d : facing(%d), Name(%s), remapping(%d), isHidden(%d)"
            , i, TempInfo.Facing, TempInfo.Name.c_str()
            , TempInfo.RemappingSensorId, TempInfo.isHidden);

        sp<CamDeviceInfo> Info = new CamDeviceInfo();
        Info->Sensors.push_back(i);
        Info->RemappingSensorId.push_back(TempInfo.RemappingSensorId);
        Info->DualFeature = 0;
        Info->RawType = TempInfo.RawType;
        Info->Name = TempInfo.Name;
        // add physical sensor static metadata
        Info->sensorStaticMetadata = MAKE_HalSensorList()->queryStaticInfo(Info->Sensors[0]);
        MY_LOGD("i : %d, remapping: %d, Info Name : %s", i, Info->RemappingSensorId[0], Info->Name.c_str());

        mDeviceSensorMap.add(i, Info);
    }

    // then we also create logical camera device(s) from the custom file:
    // camera_custom_stereo_setting.h if available
    {
        std::vector<struct LogicalSensorStruct> CustomDevList =
            LogicalCamJSONUtil::getLogicalDevices();
        MY_LOGD("manual device count = %zu", CustomDevList.size());
        for (auto&& logicalDevice : CustomDevList)
        {
            for (int j = 0; j < logicalDevice.NumofDefinition; j++)
            {
                addLogicalDevice(vTempInfo, &logicalDevice, j);
            }
        }
    }

    dumpDebugInfo();
    return 0;
}

MUINT
HalLogicalDeviceList::
queryNumberOfDevices() const
{
    Mutex::Autolock _l(mLock);
    return mDeviceSensorMap.size();
}

MUINT
HalLogicalDeviceList::
queryNumberOfSensors() const
{
    Mutex::Autolock _l(mLock);
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(CC_UNLIKELY(!pHalSensorList))
    {
        MY_LOGA("create pHalSensorList fail");
    }
    return pHalSensorList->queryNumberOfSensors();
}


IMetadata const&
HalLogicalDeviceList::
queryStaticInfo(MUINT const index) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[queryStaticInfo] mDeviceSensorMap.size() == 0");
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(index);
    if(info == nullptr)
    {
        MY_LOGA("cannot query static info index(%d)", index);
    }
    return info->sensorStaticMetadata;
}

char const*
HalLogicalDeviceList::
queryDriverName(MUINT const index) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[queryDriverName] mDeviceSensorMap.size() == 0");
    }
    MY_LOGD("queryDriverName index : %d", index);
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(index);
    MY_LOGD("queryDriverName : %s", info->Name.c_str());

    return info->Name.c_str();
}

MUINT
HalLogicalDeviceList::
queryType(MUINT const index) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[queryType] mDeviceSensorMap.size() == 0");
    }
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    auto map_index = mDeviceSensorMap.indexOfKey(index);
    if(CC_UNLIKELY(!pHalSensorList) || map_index < 0)
    {
        MY_LOGA("pHalSensorList(%p) info(%d)", pHalSensorList, index);
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueAt(map_index);
    if(info == nullptr)
    {
        MY_LOGA("[%d] CamDeviceInfo is nullptr", index);
    }
    return pHalSensorList->queryType(info->Sensors[0]);
}

MUINT
HalLogicalDeviceList::
queryFacingDirection(MUINT const index) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[queryFacingDirection] mDeviceSensorMap.size() == 0");
    }
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    auto map_index = mDeviceSensorMap.indexOfKey(index);
    if(CC_UNLIKELY(!pHalSensorList) || map_index < 0)
    {
        MY_LOGA("pHalSensorList(%p) info(%d)", pHalSensorList, index);
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueAt(map_index);
    if(info == nullptr)
    {
        MY_LOGA("[%d] CamDeviceInfo is nullptr", index);
    }
    return pHalSensorList->queryFacingDirection(info->Sensors[0]);
}

MUINT
HalLogicalDeviceList::
querySensorDevIdx(MUINT const index) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[querySensorDevIdx] mDeviceSensorMap.size() == 0");
    }
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    auto map_index = mDeviceSensorMap.indexOfKey(index);
    if(CC_UNLIKELY(!pHalSensorList) || map_index < 0)
    {
        MY_LOGA("pHalSensorList(%p) info(%d)", pHalSensorList, index);
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueAt(map_index);
    if(info == nullptr)
    {
        MY_LOGA("[%d] CamDeviceInfo is nullptr", index);
    }

    return pHalSensorList->querySensorDevIdx(info->Sensors[0]);
}

SensorStaticInfo const*
HalLogicalDeviceList::
querySensorStaticInfo(MUINT const indexDual) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[querySensorStaticInfo] mDeviceSensorMap.size() == 0");
    }
    MY_LOGD("queryDriverName1 index : %d", indexDual);
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(CC_UNLIKELY(!pHalSensorList))
    {
        MY_LOGA("create halSensorList fail");
    }
    return pHalSensorList->querySensorStaticInfo(indexDual);
}

MVOID
HalLogicalDeviceList::
querySensorStaticInfo(MUINT indexDual,
                               SensorStaticInfo *pSensorStaticInfo) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[querySensorStaticInfo] mDeviceSensorMap.size() == 0");
    }
    MY_LOGD("queryDriverName2 index : %d", indexDual);
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(CC_UNLIKELY(!pHalSensorList))
    {
        MY_LOGA("create halSensorList fail");
    }
    pHalSensorList->querySensorStaticInfo(indexDual, pSensorStaticInfo);
}

MUINT
HalLogicalDeviceList::
searchDevices()
{
    //Mutex::Autolock _l(mLock);
    #ifdef MTKCAM_DISABLE_SENSOR
    return 0;
    #endif

    if( mDeviceSensorMap.size() == 0 )
    {
        MY_LOGD("Create logical device map");
        createDeviceMap();
    }

    return mDeviceSensorMap.size();
}

std::vector<MINT32>
HalLogicalDeviceList::
getSensorId(MUINT deviceId)
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getSensorId] mDeviceSensorMap.size() == 0");
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(deviceId);

    return info->Sensors;
}

std::vector<MINT32>
HalLogicalDeviceList::
getRemappingSensorId(MUINT deviceId)
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getSensorId] mDeviceSensorMap.size() == 0");
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(deviceId);

    return info->RemappingSensorId;
}

MINT32
HalLogicalDeviceList::
getDeviceId(MINT32 sensorId)
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getDeviceId] mDeviceSensorMap.size() == 0");
    }
    for (unsigned int i = 0; i < mDeviceSensorMap.size(); i++)
    {
        sp<CamDeviceInfo> info = mDeviceSensorMap.valueAt(i);
        if (info->Sensors.size() == 1 && info->Sensors[0] == sensorId)
        {
            return i;
        }
    }

    return -1;
}


MINT32
HalLogicalDeviceList::
getDeviceIdByVID(MUINT vid)
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getDeviceIdByVID] mDeviceSensorMap.size() == 0");
    }
    for (unsigned int i = 0; i < mDeviceSensorMap.size(); i++)
    {
        sp<CamDeviceInfo> info = mDeviceSensorMap.valueAt(i);
        if (info->RemappingSensorId[0] == vid)
        {
            return i;
        }
    }

    return -1;
}

MINT32
HalLogicalDeviceList::
getSupportedFeature(
    MUINT deviceId
) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getDeviceId] mDeviceSensorMap.size() == 0");
    }
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(deviceId);
    return info->DualFeature;
}

SensorSyncType
HalLogicalDeviceList::
getSyncType(
    MUINT deviceId
) const
{
    Mutex::Autolock _l(mLock);
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(deviceId);
    return info->syncTypeInfo->syncType;
}

MINT32
HalLogicalDeviceList::
getSensorSyncMasterDevId(
    MUINT deviceId
) const
{
    Mutex::Autolock _l(mLock);
    sp<CamDeviceInfo> info = mDeviceSensorMap.valueFor(deviceId);
    return info->syncTypeInfo->masterDevId;
}

bool
HalLogicalDeviceList::
updateMultiCamSensorMetadata(
    MINT32 sensorId,
    IMetadata &metadata
)
{
    SensorStaticInfo pSensorStaticInfo;
    // get sensor metadata
    auto sensor_metadata = MAKE_HalSensorList()->queryStaticInfo(sensorId);
    auto entry = sensor_metadata.entryFor(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED);
    if(entry.isEmpty())
    {
        MY_LOGE("cannot get MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED");
        return false;
    }
    auto tagToString = [](MINT64 tag)
    {
        switch((MUINT32)tag)
        {
            case MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION: return "MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION";
            case MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE: return "MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE";
            case MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE: return "MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE";
            case MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT: return "MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT";
            case MTK_SENSOR_INFO_ORIENTATION: return "MTK_SENSOR_INFO_ORIENTATION";
            case MTK_SENSOR_INFO_WANTED_ORIENTATION: return "MTK_SENSOR_INFO_WANTED_ORIENTATION";
            case MTK_SENSOR_ORIENTATION: return "MTK_SENSOR_ORIENTATION";
            case MTK_LENS_FACING: return "MTK_LENS_FACING";
            case MTK_CONTROL_AF_AVAILABLE_MODES: return "MTK_CONTROL_AF_AVAILABLE_MODES";
            case MTK_CONTROL_MAX_REGIONS: return "MTK_CONTROL_MAX_REGIONS";
            default: return "not defined";
        }
    };
    MINT64 tagVal = 0;
    for(MUINT i=0;i<entry.count();++i)
    {
        tagVal = entry.itemAt(i, Type2Type<MINT64>());
        MY_LOGD("update sensor manual tag (%s)", tagToString(tagVal));
        // get value from physical sensor metadata
        auto sensorMetadataEntry = sensor_metadata.entryFor((MUINT32)tagVal);
        if(sensorMetadataEntry.isEmpty())
        {
            MY_LOGE("%s is not exist in sensor metadata", tagToString(tagVal));
        }
        else
        {
            metadata.update(tagVal, sensorMetadataEntry);
        }
    }
    return true;
}

uint32_t
HalLogicalDeviceList::
getVirtualInstanceId(
    uint32_t instanceId
) const
{
    uint32_t result = instanceId;
    std::string deviceName = queryDriverName((MUINT)result);
    uint32_t value;
    if(getVidByDrvName(deviceName, instanceId, value))
    {
        result = value;
        MY_LOGD("remapping instanceId(%u) vid(%u)", instanceId, result);
    }
    return result;
}

bool
HalLogicalDeviceList::
getVidByDrvName(
    std::string drvName,
    int32_t sensorId,
    uint32_t &vid
) const
{
    bool ret = false;
    vid = ((uint32_t)-1);
    if(mpVirtualDeviceIdsMapHelper != nullptr)
    {
        ret = mpVirtualDeviceIdsMapHelper->queryVID(drvName, sensorId, vid);
    }
    return ret;
}

bool
HalLogicalDeviceList::
isLogicalMulticam(
    MUINT deviceId
) const
{
    Mutex::Autolock _l(mLock);
    if (mDeviceSensorMap.size() == 0)
    {
        MY_LOGE("[getSensorId] mDeviceSensorMap.size() == 0");
    }
    auto &info = mDeviceSensorMap.valueFor(deviceId);

    if(info->Sensors.size() > 1)
    {
        return info->isLogicalMulticam;
    }

    return false;
}
bool
HalLogicalDeviceList::
getHiddenByDrvName(
    std::string drvName
) const
{
    bool ret = false;
    bool isHidden = false;

    if(mpVirtualDeviceIdsMapHelper != nullptr)
    {
        ret = mpVirtualDeviceIdsMapHelper->queryHidden(drvName);
    }

    return ret;
}

bool
HalLogicalDeviceList::
isCameraHidden(
    uint32_t instanceId
) const
{
    uint32_t result = instanceId;
    std::string deviceName = queryDriverName((MUINT)result);

    return getHiddenByDrvName(deviceName);
}