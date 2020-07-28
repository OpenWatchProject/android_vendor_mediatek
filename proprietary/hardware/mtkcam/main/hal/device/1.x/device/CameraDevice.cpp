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

#include "CameraDevice.h"
#include "MyUtils.h"
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
#include <cutils/properties.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
using namespace NSCam;
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
extern "C"
NSCam::ICameraDeviceManager::IVirtualDevice*
createVirtualCameraDevice(CreateVirtualCameraDeviceParams* params)
{
    CameraDevice* pDevice = new CameraDevice(
                                     params->pDeviceManager,
                                     params->pMetadataProvider,
                                     params->deviceType,
                                     params->instanceId);
    return pDevice;
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDevice::
~CameraDevice()
{
    MY_LOGD("ENTRY: CameraDevice deconstructor");
}

/******************************************************************************
 *
 ******************************************************************************/
CameraDevice::
CameraDevice(
    ICameraDeviceManager* deviceManager,
    IMetadataProvider* metadataProvider,
    char const* deviceType,
    int32_t instanceId
)
    : mDeviceManager(deviceManager)
    , mStaticDeviceInfo(nullptr)
    , mMetadataProvider(metadataProvider)
    , mDeviceType(deviceType)
    , mInstanceId(instanceId)
{
    MY_LOGD("ENTRY: CameraDevice contruction");
    mStaticDeviceInfo = std::make_shared<Info>();
    if (mStaticDeviceInfo != nullptr) {
        mStaticDeviceInfo->mInstanceName    = String8::format("device@%u.%u/%s/%d", kMajorDeviceVersion, kMinorDeviceVersion, deviceType, instanceId).string();
        mStaticDeviceInfo->mInstanceId      = (MAKE_HalLogicalDeviceList()->getSensorId(instanceId))[0];//instanceId;
        mStaticDeviceInfo->mMajorVersion    = kMajorDeviceVersion;
        mStaticDeviceInfo->mMinorVersion    = kMinorDeviceVersion;
        mStaticDeviceInfo->mHasFlashUnit    = metadataProvider->getDeviceHasFlashLight();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice::
getDeviceInterfaceBase(
    sp<IBase>& rpDevice
) const -> ::android::status_t
{
    rpDevice = const_cast<IBase*>(
        static_cast<const IBase*>(this));
    return android::OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice::
getDeviceInfo() const -> Info const&
{
    return *mStaticDeviceInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
getResourceCost(getResourceCost_cb _hidl_cb)
{
    CameraResourceCost resCost;
    resCost.resourceCost = 0;
    resCost.conflictingDevices.resize(0);
    _hidl_cb(Status::OK, resCost);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
getCameraInfo(getCameraInfo_cb _hidl_cb)
{
    CameraInfo cameraInfo;
    cameraInfo.facing = (CameraFacing) ((mMetadataProvider->getDeviceFacing() == MTK_LENS_FACING_FRONT)
                          ? CameraFacing::FRONT
                          : CameraFacing::BACK)
                          ;
    // Device 1.0 does not support external camera facing.
    // The closest approximation would be front camera.
    if (cameraInfo.facing == CameraFacing::EXTERNAL) {
        cameraInfo.facing = CameraFacing::FRONT;
    }
    cameraInfo.orientation = mMetadataProvider->getDeviceWantedOrientation();

    _hidl_cb(Status::OK, cameraInfo);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
setTorchMode(TorchMode mode)
{
    if( !mDeviceManager )
    {
        MY_LOGE("mDeviceManager is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mapToHidlCameraStatus(mDeviceManager->setTorchMode(mStaticDeviceInfo->mInstanceName, static_cast<uint32_t>(mode)));
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
dumpState(const hidl_handle& fd)
{
    if( ! mpDevice.get() ){
        MY_LOGD("DumpState is called before open camera!");
        return Status::OK;
    }
    else{
        MY_LOGD("DumpState is called");
#if 0
        return mpDevice->dumpState(fd);
#endif
        return Status::OK;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
open(const sp<ICameraDeviceCallback>& callback)
{
    MY_LOGD("ENTRY: open: %s, id : %d", mStaticDeviceInfo->mInstanceName.c_str(), mInstanceId);

    //[1] Create CameraDevice1Base
    mpDevice = CameraDevice1Base::createCameraDevice(mDeviceManager,
                                  mStaticDeviceInfo->mInstanceName,
                                  mInstanceId);

    if( !mpDevice.get() ){
        MY_LOGE("Create CameraDevice1Base failed!");
        return Status::INTERNAL_ERROR;
    }
    return mpDevice->open(callback);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
setPreviewWindow(const sp<ICameraDevicePreviewCallback>& window)
{
    MY_LOGD("ENTRY: setPreviewWindow");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->setPreviewWindow(window);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
enableMsgType(FrameCallbackFlags msgType)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    return mpDevice->enableMsgType(msgType);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
disableMsgType(FrameCallbackFlags msgType)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    return mpDevice->disableMsgType(msgType);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice::
msgTypeEnabled(FrameCallbackFlags msgType)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return false;
    }
    return mpDevice->msgTypeEnabled(msgType);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
startPreview()
{
    MY_LOGD("ENTRY: startPreview");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->startPreview();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
stopPreview()
{
    MY_LOGD("ENTRY: stopPreview");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    return mpDevice->stopPreview();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice::
previewEnabled()
{
    MY_LOGD("ENTRY: previewEnabled");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return false;
    }
    return mpDevice->previewEnabled();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
storeMetaDataInBuffers(bool enable)
{
    MY_LOGD("ENTRY: storeMetaDataInBuffers");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->storeMetaDataInBuffers(enable);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
startRecording()
{
    MY_LOGD("ENTRY: start recording");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->startRecording();
}

/******************************************************************************
 * Stop a previously started recording.
 ******************************************************************************/
Return<void>
CameraDevice::
stopRecording()
{
    MY_LOGD("ENTRY: stopRecording");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    return mpDevice->stopRecording();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice::
recordingEnabled()
{
    MY_LOGD("ENTRY: recordingEnabled");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return false;
    }
    return mpDevice->recordingEnabled();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
releaseRecordingFrame(uint32_t memId, uint32_t bufferIndex)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    mpDevice->releaseRecordingFrameLocked(memId, bufferIndex, nullptr);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
releaseRecordingFrameHandle(uint32_t memId, uint32_t bufferIndex, const hidl_handle& frame)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    mpDevice->releaseRecordingFrameLocked(
            memId, bufferIndex, frame.getNativeHandle());
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
releaseRecordingFrameHandleBatch(const hidl_vec<VideoFrameMessage>& msgs)
{
    if( !mpDevice.get() && msgs.size() > 0 ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    for (auto& msg : msgs) {
        mpDevice->releaseRecordingFrameLocked(
                msg.data, msg.bufferIndex, msg.frameData.getNativeHandle());
    }
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
autoFocus()
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->autoFocus();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
takePicture()
{
    MY_LOGD("ENTRY: takePicture");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->takePicture();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
cancelPicture()
{
    MY_LOGD("ENTRY: cancelPicture");

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->cancelPicture();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
cancelAutoFocus()
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->cancelAutoFocus();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
setParameters(const hidl_string& params)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->setParameters(params);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
getParameters(getParameters_cb _hidl_cb)
{
    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Void();
    }
    return mpDevice->getParameters(_hidl_cb);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice::
sendCommand(CommandType cmd, int32_t arg1, int32_t arg2)
{
    MY_LOGD("ENTRY: sendCommand: %u", cmd);

    if( !mpDevice.get() ){
        MY_LOGE("mpDevice is NULL");
        return Status::OPERATION_NOT_SUPPORTED;
    }
    return mpDevice->sendCommand(cmd, arg1, arg2);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
close()
{
    MY_LOGD("ENTRY: close");

    if( !mpDevice.get() ){
        MY_LOGI("mpDevice is NULL");
        return Void();
    }
    mpDevice->close();
    mpDevice = nullptr;
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
setProperty(const hidl_string& key, const hidl_string& value)
{
    MY_LOGD("setProperty : key %s, value %s", key.c_str(), value.c_str());
    String8 value8 = String8(value.c_str());
    String8 key8 = String8(key.c_str());
    Utils::Property::set(key8, value8);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice::
getProperty(const hidl_string& key, getProperty_cb _hidl_cb)
{
    MY_LOGD("getProperty + : key %s", key.c_str());
    String8 value8;
    String8 key8 = String8(key.c_str());
    status_t ret =   Utils::Property::tryGet(key8, value8)
                     ?   OK
                     :   NAME_NOT_FOUND
                     ;
    if(ret != OK){
        MY_LOGE("Name not found");
    }

    MY_LOGD("getProperty - : key %s, value %s", key8.c_str(), value8.c_str());
    _hidl_cb(hidl_string(value8));
    return Void();
}
