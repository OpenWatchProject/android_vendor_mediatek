/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_POLICY_REQUESTSENSORCONTROLPOLICY_MULTICAM_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_POLICY_REQUESTSENSORCONTROLPOLICY_MULTICAM_H_
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/HwTransform.h>
//
#include <mtkcam3/pipeline/policy/IRequestSensorControlPolicy.h>
#include <mtkcam3/3rdparty/core/sensor_control_type.h>
#include <unordered_map>
#include <mutex>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace requestsensorcontrol {
class SensorControllerBehaviorManager;
class Hal3AObject
{
public:
    Hal3AObject(const std::vector<int32_t> &sensorId);
    ~Hal3AObject();
public:
    bool update();
    bool isAFDone(int32_t sensorId);
    MINT32 getAEIso(int32_t sensorId);
    MINT32 getAELv_x10(int32_t sensorId);
    bool is2ASyncReady(int32_t masterSensorId);
    bool isAFSyncDone(int32_t masterSensorId);
    bool isAaoReady(std::vector<uint32_t> const& sensorList);
    bool isFramesyncReady(int32_t masterSensorId);
    bool setSync2ASetting(int32_t masterSensorId, int32_t resumeSensorId);
private:
    std::unordered_map<int32_t, std::shared_ptr<NS3Av3::IHal3A> >
                    mvHal3A;
    std::unordered_map<int32_t, NS3Av3::DualZoomInfo_T>
                    mv3AInfo;
};
/******************************************************************************
 *
 ******************************************************************************/
class DeviceInfoHelper
{
public:
    DeviceInfoHelper(
        const int32_t &deviceId);
    ~DeviceInfoHelper();
public:
    bool getPhysicalSensorFov(
                    int32_t physicalSensorId,
                    float &h,
                    float &v);
    bool getPhysicalSensorActiveArray(
                    int32_t physicalSensorId,
                    MRect &activeArray);
    bool getPhysicalSensorFullSize(
                    int32_t physicalSensorId,
                    MSize &sensorSize);
    bool getPhysicalFocalLength(
                    int32_t physicalSensorId,
                    MFLOAT &focalLength);
    bool getLogicalFocalLength(
                    std::vector<MFLOAT>& focalLength);
private:
    auto    updatePhysicalFocalLength(
                    int32_t sensorId,
                    MFLOAT &focalLength,
                    IMetadata &sensorMetadata) -> bool;
    auto    updatePhysicalSensorFov(
                    int32_t sensorId,
                    MFLOAT &focalLength,
                    std::shared_ptr<NSCam::MSizeF>& pSensorFov,
                    IMetadata &sensorMetadata) -> bool;
private:
    std::unordered_map<int32_t, std::shared_ptr<NSCam::SensorStaticInfo> >
                    mvPhysicalSensorStaticInfo;
    std::unordered_map<int32_t, std::shared_ptr<MRect> >
                    mvPhysicalSensorActiveArray;
    std::unordered_map<int32_t, std::shared_ptr<MSizeF> >
                    mvPhysicalSensorFov;
    std::unordered_map<int32_t, MFLOAT >
                    mvPhysicalFocalLength;
    std::vector<MFLOAT> mvAvailableFocalLength;
};
/******************************************************************************
 *
 ******************************************************************************/
struct ParsedSensorStateResult
{
    // online/offline list
    std::vector<uint32_t> vNeedOnlineSensorList;
    std::vector<uint32_t> vNeedOfflineSensorList;
    // online state
    std::vector<uint32_t> vStandbySensorIdList;
    std::vector<uint32_t> vResumeSensorIdList;
    std::vector<uint32_t> vStreamingSensorIdList;
    std::vector<uint32_t> vGotoStandbySensorIdList;
    // store alternative crop region
    std::unordered_map<uint32_t, MRect> vAlternactiveCropRegion;
    std::unordered_map<uint32_t, MRect> vAlternactiveCropRegion_Cap;
};
/******************************************************************************
 *
 ******************************************************************************/
struct BehaviorUserData
{
    ParsedSensorStateResult* parsedResult = nullptr;
    uint32_t requestNo = 0;
    int32_t masterId = -1;
    std::vector<int32_t> vSensorId;
    std::vector<uint32_t> vSensorMode;
    bool bNeedP2Capture = false;
    bool bNeedFusion = false;
    bool isSensorSwitchGoing = false;
    bool isAaoReadyForStreaming = true;
    bool isFrameSyncReadyForStreaming = true;
    bool isNeedNotifySync3A = false;
    bool enableSyncAf = false;
};
/******************************************************************************
 *
 ******************************************************************************/
class SensorStateManager
{
public:
    SensorStateManager() = delete;
    SensorStateManager(
                        const std::vector<int32_t> &vSensorId,
                        uint32_t maxOnlineCount,
                        uint32_t loglevel);
    ~SensorStateManager();
public:
    enum SensorStateType
    {
        E_ONLINE,
        E_OFFLINE,
        E_INVALID,
    };
private:
    struct SensorState
    {
        SensorStateType eSensorState = SensorStateType::E_ONLINE;
        sensorcontrol::SensorStatus eSensorOnlineStatus =
                sensorcontrol::SensorStatus::E_STREAMING;
        uint32_t iIdleCount = 0;
    };
public:
    /*
     * according SensorControlParamOut to update sensor status.
     */
    bool update(
            sensorcontrol::SensorControlParamOut const& out,
            ParsedSensorStateResult &result,
            bool const& bCaptureing);
    /*
     * notify sensor switch done to SensorStateManager.
     */
    void switchSensorDone();
    /*
     * query sensor switch is going or not.
     */
    bool isSensorSwitchGoing();
public:
    /*
     * Return sensor state for specific id.
     * E_ONLINE: sensor already config p1node.
     * E_OFFLINE: sensor does not connect with p1node.
     * E_INVALID: not exist id.
     */
    SensorStateManager::SensorStateType getSensorState(uint32_t sensorId) const;
    /*
     * this value only effect only sensor state is ONLINE.
     * If query this function in OFFLINE state, it will crash.
     */
    sensorcontrol::SensorStatus getSensorOnlineState(uint32_t sensorId) const;
    int32_t getMasterId() const;
private:
    void setSwitchGoingFlag(bool flag);
    void setSensorState(uint32_t &id, SensorStateType type);
    /*
     * select needed offline sensor from specific id list.
     * return value: remove count.
     */
    bool selectNeedOfflineFromList(
                ParsedSensorStateResult &parsedSensorStateResult,
                std::unordered_map<uint32_t, sensorcontrol::SensorStatus>
                                        &vResidueOnlineStateList,
                std::vector<uint32_t> const& selectList,
                unsigned int &removeCount
            );
    bool updateSensorState(
                sensorcontrol::SensorControlParamOut const& out,
                ParsedSensorStateResult &parsedSensorStateResult,
                std::unordered_map<uint32_t, sensorcontrol::SensorStatus>
                                        &vResidueOnlineStateList
            );
    bool updateOnlineSensorResult(
                ParsedSensorStateResult &parsedSensorStateResult,
                std::unordered_map<uint32_t, sensorcontrol::SensorStatus>
                                        const& vResidueOnlineStateList
            );
    /*
     * reset idle count to all sensors.
     */
    bool resetIdleCountToAllSensors();
private:
    //
    bool mbSensorSwitchGoing = false;
    std::mutex mLock;
    //
    uint32_t mLogLevel = 0;
    uint32_t mMaxOnlineSensorCount = 0;
    int32_t miCurrentMasterId = -1;
    std::unordered_map<uint32_t, SensorState> mvSensorStateList;
    // for check switch time
    std::chrono::time_point<std::chrono::system_clock> pre_time;
    // store switching camera id
    std::vector<uint32_t> mvSwitchingIdList;
    // cache data(last updated)
    ParsedSensorStateResult mLastUpdateData;
    // in first request
    bool mbInit = true;
};
/******************************************************************************
 *
 ******************************************************************************/
class RequestSensorControlPolicy_Multicam : public IRequestSensorControlPolicy
{
public:
    auto    evaluateRequest(
                        requestsensorcontrol::RequestOutputParams& out,
                        requestsensorcontrol::RequestInputParams const& in
                    ) -> int override;
    auto    sendPolicyDataCallback(
                        MUINTPTR arg1,
                        MUINTPTR arg2,
                        MUINTPTR arg3
                    ) -> bool override;

public:
    // RequestSensorControlPolicy Interfaces.
    RequestSensorControlPolicy_Multicam() = delete;
    RequestSensorControlPolicy_Multicam(CreationParams const& params);
    virtual ~RequestSensorControlPolicy_Multicam();

private:
    virtual auto    getPolicyData(
                                sensorcontrol::SensorControlParamIn &sensorControlIn,
                                requestsensorcontrol::RequestInputParams const& in) -> bool;
    virtual auto    get3AData(
                                int32_t sensorId,
                                std::shared_ptr<sensorcontrol::SensorControlInfo> &info) -> bool;
    virtual auto    getSensorData(
                                int32_t sensorId,
                                std::shared_ptr<sensorcontrol::SensorControlInfo> &info) -> bool;
    virtual auto    getDecisionResult(
                                sensorcontrol::SensorControlParamOut &out,
                                sensorcontrol::SensorControlParamIn &in) -> bool;

    virtual auto    updateCacheData(sensorcontrol::SensorControlParamOut &out) -> bool;
    virtual auto    updateP2List(
                                requestsensorcontrol::RequestInputParams const& in,
                                requestsensorcontrol::RequestOutputParams & out,
                                ParsedSensorStateResult const& result
                                ) -> bool;
    virtual auto    dumpSensorControlParam(
                                sensorcontrol::SensorControlParamOut &out,
                                sensorcontrol::SensorControlParamIn &in,
                                requestsensorcontrol::RequestInputParams const& req_in) -> bool;

private:
    CreationParams mPolicyParams;
    sensorcontrol::FeatureMode mFeatureMode = sensorcontrol::FeatureMode::E_None;
    //
    std::shared_ptr<Hal3AObject> mpHal3AObj = nullptr;
    std::shared_ptr<DeviceInfoHelper> mpDeviceInfoHelper = nullptr;
    std::unordered_map<int32_t, sensorcontrol::SensorStatus>
                    mvSensorStatus;
    //
    std::unique_ptr<
        SensorStateManager,
        std::function<void(SensorStateManager*)> > mSensorStateManager;
    //
    std::unique_ptr<
        SensorControllerBehaviorManager,
        std::function<void(SensorControllerBehaviorManager*)> > mBehaviorManager;
    //
    std::mutex mCapControlLock;
    uint32_t mCurrentActiveMasterId = -1;
    uint32_t mCurrentActiveFrameNo = -1;
    //
    int32_t mLogLevel = -1;
    //
    bool mbNeedQueryAAOState = true;
    bool mbNeedQeuryFrameSyncState = true;
    // When this flag is raised, it has to set specific metadata tag to notify 3A.
    bool mbNeedNotify3ASyncDone = false;
    // aao moniter list
    // when sensor switch is occure, it has to check both active sensor aao ready.
    std::vector<uint32_t> mvAAOMoniterList;
    // store preview streaming sensor id by request id.
    std::mutex mReqSensorStreamingMapLock;
    std::unordered_map<uint32_t, std::vector<uint32_t> > mvReqSensorStreamingMap;
    // capture request number list
    bool isCaptureing = false;
};

/******************************************************************************
 *
 ******************************************************************************/
class ISensorControllerBehavior;
class SensorControlBehavior;
class Sync3ABehavior;
class IspBehavior;
class FrameSyncBehavior;

class SensorControllerBehaviorManager
{
public:
    enum BEHAVIOR
    {
        Sync3A,
        ISP,
        FrameSync,
        SensorCrop
    };
public:
    SensorControllerBehaviorManager(
                int32_t logLevel);
    ~SensorControllerBehaviorManager();
public:
    bool update(
                requestsensorcontrol::RequestOutputParams& out,
                BehaviorUserData const& userData);
private:
    std::unordered_map<BEHAVIOR, std::shared_ptr<ISensorControllerBehavior> >
                    mvBehaviorList;
    //
    int32_t mLogLevel = -1;
};
/******************************************************************************
 *
 ******************************************************************************/
class ISensorControllerBehavior
{
public:
    ISensorControllerBehavior() = default;
    virtual ~ISensorControllerBehavior() = default;
public:
    virtual bool update(
                        RequestOutputParams& out,
                        BehaviorUserData const& userData) = 0;
};
/******************************************************************************
 *
 ******************************************************************************/
class SensorControllerBehaviorBase : public ISensorControllerBehavior
{
public:
    SensorControllerBehaviorBase(int32_t logLevel)
        : mLogLevel(logLevel)
    {
    }
    virtual ~SensorControllerBehaviorBase(){}
public:
    bool update(
        RequestOutputParams& out __attribute__((unused)),
        BehaviorUserData const& userData __attribute__((unused))) override
    {
        return false;
    }
protected:
    int32_t mLogLevel = -1;
};
/******************************************************************************
 *
 ******************************************************************************/
class Sync3ABehavior : public SensorControllerBehaviorBase
{
public:
    Sync3ABehavior(int32_t logLevel)
    : SensorControllerBehaviorBase(logLevel){}
    virtual ~Sync3ABehavior();
public:
    bool update(
                RequestOutputParams& out,
                BehaviorUserData const& userData __attribute__((unused))) override;

private:
    std::unordered_map<int32_t, bool> mvIsSensorAf;
};
/******************************************************************************
 *
 ******************************************************************************/
class IspBehavior : public SensorControllerBehaviorBase
{
public:
    IspBehavior(int32_t logLevel)
    : SensorControllerBehaviorBase(logLevel){}
    virtual ~IspBehavior();
public:
    bool update(
                RequestOutputParams& out,
                BehaviorUserData const& userData) override;
};
/******************************************************************************
 *
 ******************************************************************************/
class FrameSyncBehavior : public SensorControllerBehaviorBase
{
public:
    FrameSyncBehavior(int32_t logLevel)
    : SensorControllerBehaviorBase(logLevel){}
    virtual ~FrameSyncBehavior();
public:
    bool update(
                RequestOutputParams& out,
                BehaviorUserData const& userData) override;
};
/******************************************************************************
 *
 ******************************************************************************/
class DomainConvert;
class SensorCropBehavior : public SensorControllerBehaviorBase
{
public:
    SensorCropBehavior(int32_t logLevel)
    : SensorControllerBehaviorBase(logLevel){}
    virtual ~SensorCropBehavior();
public:
    bool update(
                RequestOutputParams& out,
                BehaviorUserData const& userData) override;
private:
    // update sensor crop to metadata
    bool updateSensorCrop(
                RequestOutputParams& out,
                BehaviorUserData const& userData);
    bool getSensorCropRect(
                int32_t sensorId,
                std::unordered_map<uint32_t, MRect> const& vCropRegion,
                MRect &cropRect);
private:
    bool needBuildTransformMatrix = true;
    //
    std::unique_ptr<
        DomainConvert,
        std::function<void(DomainConvert*)> > mDomainConvert;
};
/******************************************************************************
 * convert active domain to tg domain
 ******************************************************************************/
class DomainConvert
{
public:
    DomainConvert(
            std::vector<int32_t> &sensorIdList,
            std::vector<uint32_t> &sensorModeList);
    ~DomainConvert();
public:
    auto convertToTgDomain(
                        int32_t sensorId,
                        MRect &srcRect,
                        MRect &dstRect) -> bool;
private:
    auto buildTransformMatrix(
                        std::vector<int32_t> &sensorIdList,
                        std::vector<uint32_t> &sensorModeList) -> bool;
private:
    std::unordered_map<int32_t, NSCamHW::HwMatrix>
                    mvTransformMatrix;
};
};  //namespace requestsensorcontrol
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_POLICY_REQUESTSENSORCONTROLPOLICY_MULTICAM_H_

