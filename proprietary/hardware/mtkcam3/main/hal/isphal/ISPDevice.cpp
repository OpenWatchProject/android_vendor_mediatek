/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCam/ISPDevice"
#include "ISPDevice.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <log/log.h>
#include <utils/Errors.h>
#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
// for feature query
#include "FeatureTable.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_HAL_SERVER);
using ::android::hardware::graphics::common::V1_0::Dataspace;
using ::android::hardware::graphics::common::V1_0::PixelFormat;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace isphal {
namespace V1_0 {
namespace implementation {

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

using namespace std;
using namespace NSCam;
using namespace android;
using namespace android::hardware;
using namespace NSCam::ISPHal;


using namespace vendor::mediatek::hardware::camera::isphal::V1_0;
//using ::vendor::mediatek::hardware::camera::isphal::V1_0::ISPFeatureTagInfo;

void ISPDevice::serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    if (cookie != (uint64_t)this) {
        MY_LOGE("Unexpected ISPDevice serviceDied cookie 0x%" PRIx64 ", expected %p", cookie, this);
    }
    CAM_LOGE("%s: ISPHal ISPDevice serviceDied!!",__FUNCTION__);

    closeISPDevice();
}

Return<void> ISPDevice::getISPSupportOutputFormat(getISPSupportOutputFormat_cb _hidl_cb)
{
    FUNC_START;

    static hidl_vec<ISPFormat> out;
    if (out.size() == 0)
    {
        vector<ISPFormat> ispFormat;
        ISPFormat _ispFormat {
            PixelFormat::BLOB, static_cast<V1_0::DataspaceFlags>(Dataspace::V0_JFIF) };
        ispFormat.push_back(std::move(_ispFormat));
        out = ispFormat;
    }
    _hidl_cb(Status::OK, out);
    FUNC_END;

    return Void();
}

Return<void> ISPDevice::getISPSupportInputFormat(getISPSupportInputFormat_cb _hidl_cb)
{
    FUNC_START;

    static hidl_vec<ISPFormat> out;
    if (out.size() == 0)
    {
        vector<ISPFormat> ispFormat;
        // TODO: check if need to fill a specific dataspace
        ISPFormat _ispFormat {
            PixelFormat::YV12, static_cast<V1_0::DataspaceFlags>(Dataspace::UNKNOWN) };
        ispFormat.push_back(std::move(_ispFormat));
        out = ispFormat;
    }
    _hidl_cb(Status::OK, out);
    FUNC_END;

    return Void();
}

Return<void> ISPDevice::getISPSupportFeatures(getISPSupportFeatures_cb _hidl_cb)
{
    FUNC_START;

    vector<ISPFeatureTagInfo> Infos;
    ISPFeatureTagInfo Mfnr;
    Mfnr.Name = std::string("com.mediatek.control.capture.hintForIspTuning");
    Mfnr.tag = MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING;
    Mfnr.type = MetaDataType::TYPE_INT32;

    Infos.push_back(Mfnr);
    hidl_vec<ISPFeatureTagInfo> out(Infos);
    _hidl_cb(Status::OK, out);
    FUNC_END;

    return Void();
}

Return<void> ISPDevice::queryFeatureSetting(const V1_0::StatisticsInfo& info, queryFeatureSetting_cb _hidl_cb)
{
    FUNC_START;
    if (mMetaConverter == nullptr || mPipelineModel == nullptr)
    {
        MY_LOGE("device is not configured, cannot queryFeatureSetting");
        hidl_vec<FeatureSetting> out;
        _hidl_cb(Status::INTERNAL_ERROR, 0, out);
        return Void();
    }
    IMetadata data;
    hidl_vec<FeatureSetting> out;
    std::vector<std::shared_ptr<IMetadata>> setting;
    uint32_t count;
    mMetaConverter->convertToIMetadata(info.meta.data(), data);
    mPipelineModel->queryFeature(&data, setting);
    count = setting.size();
    vector<FeatureSetting> Infos;
    for (int i = 0; i < count; i++)
    {
        std::shared_ptr<IMetadata> outMeta = setting[i];
        for (int j = 0; j < outMeta->count(); j++)
        {
            auto entry = outMeta->entryAt(j);
            auto tagInfo = gFeatureMap.find(entry.tag());
            if (tagInfo != gFeatureMap.end())
            {
                FeatureSetting temp;
                temp.Name = tagInfo->second.name;
                temp.tag = tagInfo->first;
                temp.type = tagInfo->second.type;
                temp.data.setToExternal((uint8_t *)entry.data(), entry.count()*tagInfo->second.datacount);
                temp.count = entry.count();
                temp.frameNo = i;
                Infos.push_back(std::move(temp));
            }
        }

    }
    out = Infos;


    _hidl_cb(Status::OK, count, out);
    FUNC_END;

    return Void();
}

Return<int32_t> ISPDevice::getISPTuningDataSize()
{
    FUNC_START;

    int32_t size = kISP_TUNING_DATA_SIZE;
    FUNC_END;

    return size;
}

Return<Status> ISPDevice::configureISPDevice(const V1_0::ISPStreamConfiguration& config, const ::android::sp<V1_0::ICallback>& Cb)
{
    CAM_TRACE_CALL();
    FUNC_START;
    std::lock_guard<std::timed_mutex> _l(mLock);
    //unlink to death notification for existed callback
    if (mHidlCallback != nullptr){
        mHidlCallback->unlinkToDeath(this);
    }
    //link to death notification for callback
    if ( Cb != nullptr ) {
        ::android::hardware::Return<bool> linked = Cb->linkToDeath(this, (uint64_t)this);
        if (!linked.isOk())
        {
            MY_LOGE("Transaction error in linking to mHidlCallback death: %s", linked.description().c_str());
        }
        else if (!linked)
        {
            MY_LOGE("Unable to link to mHidlCallback death notifications");
        }
    }
    if (mPipelineCallback != nullptr)
    {
        mPipelineCallback->waitForReqDrained(0);
    }
    if (mPipelineModel != nullptr)
    {
        MY_LOGD("close isp pipeline +");
        mPipelineModel->beginFlush();
        mPipelineModel->endFlush();
        mPipelineModel->close();
        mPipelineModel = nullptr;
        MY_LOGD("close isp pipeline -");
    }
    if (mCamId == -1)
    {
        // currently, isp pipeline need use camera ID and sensor IDs to configuration,
        // use camera 0 to set default value
        mSensorIds.clear();
        mSensorIds.push_back(0);
        mCamId = 0;
    }
    mHidlCallback = Cb;
    mMetaConverter = new ISPMetadataConverter();
    mPipelineCallback = new ISPCallback(mHidlCallback, mMetaConverter);
    mPipelineModel = IISPPipelineModel::createInstance(mId, mCamId, mSensorIds); // camera id current is not used
    mStreamCtrl    = new ISPStreamController(mId, mMetaConverter);
    mConfigParams = nullptr;
    // stream controller configure
    MY_LOGD("Configure stream controller");
    if (mStreamCtrl->configure(config, mConfigParams) || mConfigParams == nullptr)
    {
        MY_LOGE("[configureISPDevice]ISP stream controller failed, params=%p", mConfigParams.get());
        return Status::INTERNAL_ERROR;
    }

    // pipeline configure
    MY_LOGD("Open/Configure ISP Pipeline");
    mPipelineModel->open("ISPDevice", mPipelineCallback);
    mPipelineModel->configure(mConfigParams);
    MY_LOGD("Configure ISP Pipeline done");

    FUNC_END;

    return Status::OK;
}

Return<Status> ISPDevice::closeISPDevice()
{
    CAM_TRACE_CALL();
    FUNC_START;
    std::lock_guard<std::timed_mutex> _l(mLock);

    if (mPipelineModel != nullptr)
    {
        // flush pipeline
        mPipelineModel->beginFlush();
        mPipelineModel->endFlush();
        // close pipeine
        mPipelineModel->close();
    }

    //unlink to death notification for existed device callback
    if ( mHidlCallback != nullptr ) {
        mHidlCallback->unlinkToDeath(this);
        mHidlCallback = nullptr;
    }

    // destroy all sp
    mStreamCtrl = nullptr;
    mPipelineModel = nullptr;
    mPipelineCallback = nullptr;
    mMetaConverter = nullptr;
    mConfigParams = nullptr;

    // remove isp device
    IISPManager::getInstance()->removeISPDevice(mId);
    FUNC_END;

    return Status::OK;
}

Return<Status> ISPDevice::processImgRequest(const hidl_vec<V1_0::ISPRequest>& requests)
{
    CAM_TRACE_CALL();
    FUNC_START;
    std::lock_guard<std::timed_mutex> _l(mLock);
    std::vector<std::shared_ptr<UserRequestParams>> ppl_requests;
    std::shared_ptr<UserConfigurationParams> reconfig_params = nullptr;
    // stream controller handle hidl request
    if (mStreamCtrl->parseRequests(requests, ppl_requests, reconfig_params))
    {
        MY_LOGE("[processImgRequest]ISP stream controller failed");
        return Status::INTERNAL_ERROR;
    }

    // check if need reconfigure
    if ( checkReconfigure(ppl_requests, reconfig_params, mConfigParams) )
    {
        MY_LOGD("reconfig");
        // wait request drained
        if (mPipelineCallback != nullptr)
        {
            mPipelineCallback->waitForReqDrained(0);
        }
        // close pipeline
        MY_LOGD("close isp pipeline");
        if (mPipelineModel != nullptr)
        {
            mPipelineModel->beginFlush();
            mPipelineModel->endFlush();
            mPipelineModel->close();
            mPipelineModel = nullptr;
        }
        // create new pipeline and config
        MY_LOGD("create new isp pipeline and then configure");
        mPipelineModel = IISPPipelineModel::createInstance(mId, mCamId, mSensorIds); // camera id current is not used
        mPipelineModel->open("ISPDevice", mPipelineCallback);
        mPipelineModel->configure(mConfigParams);

    }

    // pipeline handle user request
    uint32_t reqNum = (uint32_t)(ppl_requests.size());
    for(size_t i = 0; i < requests.size(); i++)
    {
        mPipelineCallback->registerReq(requests[i].reqNumber);
    }
    MY_LOGD("submit requests");
    mPipelineModel->submitRequest(ppl_requests, reqNum);

    FUNC_END;

    return Status::OK;
}

/******************************************************************************
 *
 ******************************************************************************/
ISPDevice::
~ISPDevice()
{
    MY_LOGI("destroy ISP device");
}


/******************************************************************************
 *
 ******************************************************************************/
ISPDevice::
ISPDevice(int32_t id)
    : mId(id)
{
    MY_LOGI("create ISP device");
}

auto ISPDevice::
checkReconfigure(
    const std::vector<std::shared_ptr<UserRequestParams>> &requests,
    const std::shared_ptr<UserConfigurationParams> &newParam,
    std::shared_ptr<UserConfigurationParams> &currentParam
) -> MBOOL
{
    if (requests.size() == 0)
    {
        MY_LOGD("Has no user request!!, no need reconfigure");
        return false;
    }
    MBOOL needReconfigure = false;
    if (newParam != nullptr)
    {
        MY_LOGD("Has new user ConfigureParam!!, need reconfigure");
        for (auto const& stream : newParam->vImageStreams)
        {
            currentParam->vImageStreams[stream.first] = stream.second;
        }
        needReconfigure = true;
    }
    IMetadata *phalMeta = nullptr;
    for( const auto& buffer : requests[0]->vIMetaBuffers )
    {
        switch (buffer.first)
        {
            case eSTREAMID_META_PIPE_DYNAMIC_01:
                {
                    MINT32 devId = 0;
                    phalMeta = buffer.second->tryReadLock(LOG_TAG);
                    MY_LOGD("get hal metadata (%p)", phalMeta);
                    if (phalMeta == nullptr)
                    {
                        MY_LOGW("get hal metadata failed, didn't reconfigure");
                        return needReconfigure;
                    }
                    IMetadata::IEntry const& entrySensorId = phalMeta->entryFor(MTK_HAL_REQUEST_SENSOR_ID);

                    // get sensor id
                    if  ( entrySensorId.isEmpty() )
                    {
                        MY_LOGW("No tag: MTK_HAL_REQUEST_SENSOR_ID");
                        goto EXIT;
                    }

                    if (entrySensorId.count() > 1)
                    {
                        MY_LOGW("Warning! Not Support multi-sensor id, set sensorId[0] to isp pipeline");
                    }

                    mSensorIds.clear();
                    for (MUINT i = 0; i < entrySensorId.count(); i++)
                    {
                        mSensorIds.push_back(entrySensorId.itemAt(i, Type2Type<MINT32>()));
                    }
                    MY_LOGD("current sensor Id = %d, new camera Id in request = %d", mCamId, mSensorIds[0]);
                    if (mCamId != mSensorIds[0])
                    {
                        MY_LOGD("need reconfigure");
                        needReconfigure = true;
                        mCamId = mSensorIds[0];
                    }
EXIT:
                    buffer.second->unlock(LOG_TAG, phalMeta);
                    return needReconfigure;
                }
                break;
        }
    }
    return needReconfigure;
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace isphal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
