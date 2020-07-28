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

#include "CameraDevice1Base.h"
#include "MyUtils.h"
//
#include <list>
#include <cutils/atomic.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
//
//TODO: change to #include <interfaces/camera/device/1.0/types.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
//
#include <libladder.h>
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/JobMonitor.h>
//
using namespace std;
using namespace android;
using namespace NSCam;
using namespace android::MtkCamUtils;
using namespace NSCamHW;
//
extern android::Mutex gCamDev1BaseMemeryMapLock;
/******************************************************************************
 *
 ******************************************************************************/
list<wp<CameraDevice1Base>> glwpCameraDevice1Base;
Mutex                       gCameraDevice1Baselock;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[CameraDevice1Base::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
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
CameraDevice1Base::
~CameraDevice1Base()
{
    MY_LOGD(" ");
    Mutex::Autolock _l(mOpsLock);
    mpDisplayClient.clear();
    mpCamClient.clear();
    mpCamAdapter.clear();
    vmpCamClient.clear();
    mHalPreviewWindow.cleanUpCirculatingBuffers();
    {
        Mutex::Autolock _l(gCamDev1BaseMemeryMapLock);
        mMemoryMap.clear();
    }

    if(mpCpuCtrl)
    {
        mpCpuCtrl->disable();
        mpCpuCtrl->uninit();
        mpCpuCtrl->destroyInstance();
        mpCpuCtrl = NULL;
    }
    if(mpPreferIdleCpuCtrl)
    {
        mpPreferIdleCpuCtrl->disable();
        mpPreferIdleCpuCtrl->uninit();
        mpPreferIdleCpuCtrl->destroyInstance();
        mpPreferIdleCpuCtrl = NULL;
    }
    //
    //[customize] 4-cell sensor update scenario sensor mode
    HwInfoHelper helper(getInstanceId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
    }
    //
    if (helper.get4CellSensorSupported())
    {
        mpPlugProcess->uninit();
        mpPlugProcess = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDevice1Base::
CameraDevice1Base(
    ICameraDeviceManager* deviceManager,
    std::string           instanceName,
    int32_t               instanceId,
    android::String8 const&     rDevName
)
    : mOpsLock()
    , mDeviceManager(deviceManager)
    , mInstanceName(instanceName)
    , mInstanceId(instanceId)
    , mDevName(rDevName)
    //
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(NULL)
    , mpParamsMgrV3(NULL)
    , mpCamClient()
    , mpDisplayClient()
    , mpCamAdapter(NULL)
    , vmpCamClient()
    //
    , mBatchSize(0)
    , mBatchMsgType(0)
    //
    , mIsRaw16CBEnabled(false)
    , mIsPreviewEnabled(false)
    , mLastEnableMsg(0)
    , mOrientation(0)
    , mMetadataMode(false)
    , mbWindowReady(false)
    //
    , mpCpuCtrl(NULL)
    , mpPreferIdleCpuCtrl(NULL)
    , mpPlugProcess(NULL)
    //
    , mStartPreviewTThreadHandle(0)
    , mTodoCmdMap()
    , mTodoCmdMapLock()
    //
    , mNativeWindowConsumerUsage(0)
{
    MY_LOGD("%p: mDeviceManager:%p mInstanceName:%s mInstanceId:%d",
        this, mDeviceManager, mInstanceName.c_str(), mInstanceId);

    mMemAllocator = IAllocator::getService("ashmem");
    if (mMemAllocator == nullptr) {
        ALOGE("%s: cannot get ashmemAllocator", __FUNCTION__);
    }

    mpCpuCtrl = CpuCtrl::createInstance();
    if(mpCpuCtrl != NULL)
    {
        mpCpuCtrl->init();
        mpCpuCtrl->cpuPerformanceMode(1);
    }
    mpPreferIdleCpuCtrl = CpuCtrl::createInstance();
    if(mpPreferIdleCpuCtrl != NULL)
    {
        mpPreferIdleCpuCtrl->init();
    }
    //
    //[customize] 4-cell sensor update scenario sensor mode
    HwInfoHelper helper(getInstanceId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
    }
    //
    if (helper.get4CellSensorSupported())
    {
        MSize captureSize;
        helper.getSensorSize(SENSOR_SCENARIO_ID_NORMAL_CAPTURE, captureSize);
        mpPlugProcess = IPlugProcessing::createInstance((MUINT32)IPlugProcessing::PLUG_ID_FCELL, (NSCam::IPlugProcessing::DEV_ID)getInstanceId());
        PlugInitParam initParam;
        initParam.openId = mInstanceId;
        initParam.img_w = captureSize.w;
        initParam.img_h = captureSize.h;
        mpPlugProcess->sendCommand(NSCam::NSCamPlug::SET_PARAM, NSCam::IPlugProcessing::PARAM_INIT, (MINTPTR)&initParam);
    }
    //

    ::memset(&mDeviceCallbackDebugInfo, 0, sizeof(mDeviceCallbackDebugInfo));
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice1Base::
getDeviceInterfaceBase(
    __attribute__((unused)) sp<IBase>& rpDevice
) const -> ::android::status_t
{
    //no-op
    return android::OK;////////
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice1Base::
getDeviceInfo() const -> Info const&
{
    Info* temp = new Info();
    return *temp;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
onCam1Device_NotifyCb(
    int32_t const   msgType,
    __attribute__((unused)) intptr_t const  ext1)
{
    CAM_TRACE_NAME("CameraDevice1Base::onCam1Device_NotifyCb");
    MY_LOGD("0x%08X",msgType);
    switch(msgType)
    {
        case MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE:
        {
            sp<android::ICamAdapter> spCamAdapter = mpCamAdapter;
            if( spCamAdapter != 0 &&
                mpCamClient != 0)
            {
                if( spCamAdapter->previewEnabled() == true &&
                    mpCamClient->previewEnabled() == false)
                {
                    MY_LOGD("clientStartPreview");
                    CAM_TRACE_NAME("clientStartPreview");
                    if (mpCamClient->startPreview() )
                    {
                        if(mIsPreviewEnabled)
                        {
                            MY_LOGD("enable CAMERA_MSG_PREVIEW_METADATA");
                            enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
                        }
                    }
                    else
                    {
                        MY_LOGE("mpCamClient->startPreview fail");
                    }
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CameraDevice1Base::
initDisplayClient(preview_stream_ops* window)
{
    CAM_TRACE_CALL();
#if '1'!=MTKCAM_HAVE_DISPLAY_CLIENT
    #warning "Not Build Display Client"
    MY_LOGD("Not Build Display Client");
    return  OK;
#else
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+ window(%p)", window);
    //
    //
    //  [1] Check to see whether the passed window is NULL or not.
    if  ( ! window )
    {
        MY_LOGW("NULL window is passed into...");
        mbWindowReady = false;
        //
        if  ( mpDisplayClient != 0 )
        {
            MY_LOGW("destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
        status = OK;
        goto lbExit;
    }
    mbWindowReady = true;
    //
    //
    //  [2] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize failed");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    //
    //  [3] Initialize Display Client.
    if  ( mpDisplayClient != 0 )
    {
        if  ( previewEnabled() )
        {
            MY_LOGW("Do nothing since Display Client(%p) is already created after startPreview()", mpDisplayClient.get());
//          This method must be called before startPreview(). The one exception is that
//          if the preview surface texture is not set (or set to null) before startPreview() is called,
//          then this method may be called once with a non-null parameter to set the preview surface.
            status = OK;
            goto lbExit;
        }
        else
        {
            MY_LOGW("New window is set after stopPreview or takePicture. Destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
    }
    //  [3.1] create a Display Client.
    mpDisplayClient = IDisplayClient::createInstance(getInstanceId());
    if  ( mpDisplayClient == 0 )
    {
        MY_LOGE("Cannot create mpDisplayClient");
        status = NO_MEMORY;
        goto lbExit;
    }
    //  Display Rotation
    if(mpParamsMgr->getDisplayRotationSupported())
    {
        MY_LOGD("orientation = %d", mOrientation);
        mpDisplayClient->SetOrientationForDisplay(mOrientation);
    }
    //  [3.2] initialize the newly-created Display Client.
    if  ( ! mpDisplayClient->init() )
    {
        MY_LOGE("mpDisplayClient init() failed");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.3] set preview_stream_ops & related window info.
    if  ( ! mpDisplayClient->setWindow(window, previewSize.width, previewSize.height, queryDisplayBufCount(), mNativeWindowConsumerUsage) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //  [3.4] set Image Buffer Provider Client if it exist.
    if  ( mpCamAdapter != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    //
    status = OK;
    //
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup...");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    MY_LOGD("- status(%d)", status);
    return  status;
#endif//MTKCAM_HAVE_DISPLAY_CLIENT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CameraDevice1Base::
enableDisplayClient()
{
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+");
    //
    //  [1] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //  [2] Enable
    if  ( ! mpDisplayClient->enableDisplay(previewSize.width, previewSize.height, queryDisplayBufCount(), mpCamAdapter) )
    {
        MY_LOGE("mpDisplayClient(%p)->enableDisplay failed", mpDisplayClient.get());
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    status = OK;
lbExit:
    MY_LOGD("- status(%d)",status);
    return  status;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
disableDisplayClient()
{
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->disableDisplay();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void*
CameraDevice1Base::
startPreviewThread(void* arg)
{
    CAM_LOGD("[CameraDevice1Base::startPreviewThread]+");
    CameraDevice1Base* pCam1DeviceBase = (CameraDevice1Base*)arg;
    pCam1DeviceBase->startPreviewImp();
    CAM_LOGD("[CameraDevice1Base::startPreviewThread]- done");
    pthread_exit(NULL);
    CAM_LOGD("[CameraDevice1Base::startPreviewThread]-");
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
startPreviewImp()
{
    CAM_TRACE_CALL();
    MY_LOGD("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    //
    status_t status = OK;
    //
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("init in thread failed");
        return;
    }
    //
    CamManager* pCamMgr = CamManager::getInstance();
    pCamMgr->getStartPreviewLock();
    status = mpCamAdapter->startPreview();
    pCamMgr->releaseStartPreviewLock();
    if  ( OK != status )
    {
        MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
        sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(getInstanceId(),mpCamMsgCbInfo);
        spFrameworkCBThread->init();
        IFrameworkCBThread::callback_data cbData;
        cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
        cbData.type         = CAMERA_MSG_ERROR;
        cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
        cbData.ext2         = 0;
        spFrameworkCBThread->postCB(cbData);
        spFrameworkCBThread->uninit();
        spFrameworkCBThread = NULL;
        status = OK;
        MY_LOGE("startPreview fail, return OK, callback CAMERA_ERROR_SERVER_DIED");
    }
    //
    MY_LOGD("- status(%d)", status);
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CameraDevice1Base::
waitStartPreviewDone()
{
    Mutex::Autolock _lock(mJoinPreviewThreadLock);

    if(mStartPreviewTThreadHandle != 0)
    {
        CAM_TRACE_CALL();
        MY_LOGD("E");
        int s = pthread_join(mStartPreviewTThreadHandle, NULL);
        mStartPreviewTThreadHandle = 0;
        MY_LOGD("X");
        if( s != 0 )
        {
            MY_LOGE("startPreview pthread join error: %d", s);
            return false;
        }
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
setDeviceCallback(const sp<ICameraDeviceCallback>& callback)
{
    mDeviceCallback = callback;
    mpCamMsgCbInfo->mCam1DeviceCb   = static_cast<ICam1DeviceCallback*>(this);

    mpCamMsgCbInfo->mCbCookie       = (void *)(uintptr_t)mInstanceId;
    mpCamMsgCbInfo->mNotifyCb       = sNotifyCb;
    mpCamMsgCbInfo->mDataCb         = sDataCb;
    mpCamMsgCbInfo->mDataCbTimestamp= sDataCbTimestamp;
    mpCamMsgCbInfo->mRequestMemory  = sGetMemory;

    mpCamMsgCbInfo->mMetadataCb  = sMtkDataCb;
    mMtkDeviceCallback = IMtkCameraDeviceCallback::castFrom(callback);

    if  ( mpCamClient != 0 )
    {
        MY_LOGD("camclient start setcallbacks");
        mpCamClient->setCallbacks(mpCamMsgCbInfo);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->setCallbacks(mpCamMsgCbInfo);
    }

    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CameraDevice1Base::
queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height)
{
    mpParamsMgr->getPreviewSize(&ri4Width, &ri4Height);
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CameraDevice1Base::
initCameraAdapter()
{
    bool ret = false;
    //
    //  (1) Check to see if CamAdapter has existed or not.
    if  ( mpCamAdapter != 0 )
    {
        if  ( ICamAdapter::isValidInstance(mpCamAdapter) )
        {   // do nothing & just return true if the same app.
            MY_LOGD("valid camera adapter: %s", mpCamAdapter->getName());
            ret = true;
            goto lbExit;
        }
        else
        {   // cleanup the original one if different app.
            MY_LOGW("invalid camera adapter: %s", mpCamAdapter->getName());
            mpCamAdapter->uninit();
            mpCamAdapter.clear();
        }
    }
    //
    //  (2) Create & init a new CamAdapter.
    mpCamAdapter = ICamAdapter::createInstance(mDevName,
                                            mInstanceId,
                                            mpParamsMgrV3);
    if  ( mpCamAdapter != 0 && mpCamAdapter->init() )
    {
        //  (.1) init.
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
        mpCamAdapter->enableMsgType(mpCamMsgCbInfo->mMsgEnabled);

        //  (.2) Invoke its setParameters
        if  ( OK != mpCamAdapter->setParameters() )
        {
            //  If fail, it should destroy instance before return.
            MY_LOGE("mpCamAdapter->setParameters() fail");
            goto lbExit;
        }

        //  (.3) Send to-do commands.
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            for (size_t i = 0; i < mTodoCmdMap.size(); i++)
            {
                CommandInfo const& rCmdInfo = mTodoCmdMap.valueAt(i);
                MY_LOGD("send queued cmd(%#x),args(%d,%d)", rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
                mpCamAdapter->sendCommand(rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
            }
            mTodoCmdMap.clear();
        }

        //  (.4) [DisplayClient] set Image Buffer Provider Client if needed.
        if  ( mpDisplayClient != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpDisplayClient->setImgBufProviderClient() fail");
            goto lbExit;
        }

        //  (.5) [CamClient] set Image Buffer Provider Client if needed.
        if  ( mpCamClient != 0 && ! mpCamClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpCamClient->setImgBufProviderClient() fail");
            goto lbExit;
        }
        //
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->setImgBufProviderClient(mpCamAdapter) ;
        }

    }
    else
    {
        MY_LOGE("mpCamAdapter(%p)->init() fail", mpCamAdapter.get());
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
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
CameraDevice1Base::
getCameraInfo(__attribute__((unused)) getCameraInfo_cb _hidl_cb)
{
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
setTorchMode(TorchMode mode)
{
    return mapToHidlCameraStatus(mDeviceManager->setTorchMode(mInstanceName, static_cast<uint32_t>(mode)));
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    if (cookie != (uint64_t)this) {
        MY_LOGW("Unexpected ICameraDeviceCallback serviceDied cookie 0x%" PRIx64 ", expected %p", cookie, this);
    }
    MY_LOGW("ICameraDeviceCallback has died - %s; removing it - cookie:0x%" PRIx64 " this:%p", toString(mDeviceCallbackDebugInfo).c_str(), cookie, this);

    //Early set the device as close state to avoid callbacks after serviceDied
    sSetDevConnect(mInstanceName, false);
    {
        mDeviceCallback = nullptr;
        ::memset(&mDeviceCallbackDebugInfo, 0, sizeof(mDeviceCallbackDebugInfo));
    }

    //force to close camera
    MY_LOGW("CameraService has died, force to close camera device %d", mInstanceId);

    disableMsgType(CAMERA_MSG_ALL_MSGS);
    disableMsgType(MTK_CAMERA_MSG_ALL_MSGS);
    stopPreview();
    cancelPicture();
    close();
    setPreviewWindow(nullptr);
}

/******************************************************************************
 *
 ******************************************************************************/
void
CameraDevice1Base::
open(const sp<ICameraDeviceCallback>& callback, camera_notify_callback notify_cb, camera_data_callback data_cb, camera_data_timestamp_callback data_cb_timestamp, camera_request_memory get_memory)
{
    MY_LOGD("[open] start");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    ::android::status_t status = OK;
    std::string const& instanceName = mInstanceName;

    status = mDeviceManager->startOpenDevice(instanceName);
    if ( OK != status ){
        //return mapToHidlCameraStatus(status);
    }

    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);
        status = onOpenLocked();
    }

    if (OK != status){  //open failed
        mDeviceManager->finishOpenDevice(instanceName, true/*cancel*/);
        //return mapToHidlCameraStatus(status);
    }
    else{   //open successfully
        status = mDeviceManager->finishOpenDevice(instanceName, false/*cancel*/);
        if( OK != status){
            //return mapToHidlCameraStatus(status);
        }
    }

    //Set Device Callback
    initHalPreviewWindow();

    //update device list
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        MBOOL bIsFind = MFALSE;
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote().get() != 0 &&
                (*it).promote()->getInstanceId() == mInstanceId)
            {
                bIsFind = MTRUE;
                MY_LOGE("Exist cameraId %d - %p, this - %p", mInstanceId, (*it).promote().get(), this);
                break;
            }
        }
        if(!bIsFind)
        {
            MY_LOGI("Add new cameraId %d - %p", mInstanceId, this);
            glwpCameraDevice1Base.push_back(this);
        }
    }

    //------------------------ Set Callbacks --------------------------
    mDeviceCallback = callback;
    mpCamMsgCbInfo->mCam1DeviceCb   = static_cast<ICam1DeviceCallback*>(this);

    mpCamMsgCbInfo->mCbCookie       = this;
    mpCamMsgCbInfo->mNotifyCb       = notify_cb;
    mpCamMsgCbInfo->mDataCb         = data_cb;
    mpCamMsgCbInfo->mDataCbTimestamp= data_cb_timestamp;
    mpCamMsgCbInfo->mRequestMemory  = get_memory;

    if  ( mpCamClient != 0 )
    {
        MY_LOGD("camclient start setcallbacks");
        mpCamClient->setCallbacks(mpCamMsgCbInfo);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->setCallbacks(mpCamMsgCbInfo);
    }

    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
    }
    //----------------------------------------------------------------

    sSetDevConnect(instanceName, true);

    MY_LOGD("[open] end");
    //return Status::OK;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
open(const sp<ICameraDeviceCallback>& callback)
{
    ::android::Mutex::Autolock _lOpsLock(mOpsLock);

    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    //unlink to death notification for existed device callback
    if ( mDeviceCallback != nullptr ) {
        mDeviceCallback->unlinkToDeath(this);
        mDeviceCallback = nullptr;
        ::memset(&mDeviceCallbackDebugInfo, 0, sizeof(mDeviceCallbackDebugInfo));
    }

    //link to death notification for device callback
    if ( callback != nullptr ) {
        hardware::Return<bool> linked = callback->linkToDeath(this, (uint64_t)this);
        if (!linked.isOk()) {
            MY_LOGE("Transaction error in linking to mDeviceCallback death: %s", linked.description().c_str());
        } else if (!linked) {
            MY_LOGW("Unable to link to mDeviceCallback death notifications");
        }
        callback->getDebugInfo([this](const auto& info){
            mDeviceCallbackDebugInfo = info;
        });
        MY_LOGD("Link death to ICameraDeviceCallback %s", toString(mDeviceCallbackDebugInfo).c_str());
    }

    ::android::status_t status = OK;
    std::string const& instanceName = mInstanceName;

    status = mDeviceManager->startOpenDevice(instanceName);
    if ( OK != status ){
        MY_LOGW("DeviceManager startOpenDevice failed");
        mDeviceManager->updatePowerOnDone();
        return mapToHidlCameraStatus(status);
    }
    {
        // check if needs add log line count in stereo mode.
        if  ( 0 == ::strcmp(mDevName,"MtkStereo" ) )
            setLogLevelToEngLoad(1, 1, 20000);
        //
    }

    status = onOpenLocked();

    if (OK != status){  //open failed
        MY_LOGW("CameraDevice1Base onOpenLocked failed");
        mDeviceManager->finishOpenDevice(instanceName, true/*cancel*/);
        return mapToHidlCameraStatus(status);
    }
    else{   //open successfully
        status = mDeviceManager->finishOpenDevice(instanceName, false/*cancel*/);
        if( OK != status){
            MY_LOGW("DeviceManager finishOpenDevice failed");
            return mapToHidlCameraStatus(status);
        }
    }

    //Set Device Callback
    initHalPreviewWindow();

    //update device list
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        MBOOL bIsFind = MFALSE;
        for(list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin(); it != glwpCameraDevice1Base.end(); it++)
        {
            if( (*it).promote().get() != 0 &&
                (*it).promote()->getInstanceId() == mInstanceId)
            {
                bIsFind = MTRUE;
                MY_LOGE("Exist cameraId %d - %p, this - %p", mInstanceId, (*it).promote().get(), this);
                break;
            }
        }
        if(!bIsFind)
        {
            MY_LOGI("Add new cameraId %d - %p", mInstanceId, this);
            glwpCameraDevice1Base.push_back(this);
        }
    }

    setDeviceCallback(callback);

    sSetDevConnect(instanceName, true);

    MY_LOGI("-");
    return Status::OK;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
setPreviewWindow(const sp<ICameraDevicePreviewCallback>& window)
{
    CAM_TRACE_CALL();
    MY_LOGD("+ window(%p)", &window);
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    ::android::Mutex::Autolock _lOpsLock(mOpsLock);
    mHalPreviewWindow.mPreviewCallback = window;///memory copy?

    status_t status = initDisplayClient((window == nullptr) ? nullptr : &mHalPreviewWindow);
    if  ( OK == status && previewEnabled() && mpDisplayClient != 0 )
    {
        status = enableDisplayClient();
        if(mbWindowReady)
        {
            waitStartPreviewDone();
        }
    }

    MY_LOGD("-");
    return mapToHidlCameraStatus(status);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
enableMsgType(FrameCallbackFlags msgType)
{
    MY_LOGD("+ enableMsgType: %d", msgType);

    android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->enableMsgType(msgType);
    }

    if  ( mpCamClient != 0 )
    {
        mpCamClient->enableMsgType(msgType);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->enableMsgType(msgType);
    }
    mLastEnableMsg = msgType;

    MY_LOGD("-");
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
disableMsgType(FrameCallbackFlags msgType)
{
    MY_LOGD("+ disableMsgType: %d", msgType);

    android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);

    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->disableMsgType(msgType);
    }

    if  ( mpCamClient != 0 )
    {
        mpCamClient->disableMsgType(msgType);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->disableMsgType(msgType);
    }

    MY_LOGD("-");
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice1Base::
msgTypeEnabled(FrameCallbackFlags msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
startPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    status_t status = OK;
    bool usePreviewThread = false;
    CamManager* pCamMgr = CamManager::getInstance();

    if( mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) != MtkCameraParameters::CAMERA_MODE_NORMAL &&
        mbWindowReady == false)
    {
        usePreviewThread = true;
        disableWaitSensorThread(true);
    }

    {
        CAM_TRACE_NAME("deviceStartPreview");
        if  ( mpCamAdapter != 0 && mpCamAdapter->isTakingPicture() )
        {
            MY_LOGE("Capture is not done");
            status = INVALID_OPERATION;
            return  mapToHidlCameraStatus(status);
        }

        if  ( previewEnabled() )
        {
            MY_LOGD("Preview already running");
            status = ALREADY_EXISTS;
            return  mapToHidlCameraStatus(status);
        }

        //  Check Permission.
        if ( ! pCamMgr->getPermission() )
        {
            MY_LOGE("Cannot start preview ... Permission denied");
            sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(getInstanceId(),mpCamMsgCbInfo);
            spFrameworkCBThread->init();
            IFrameworkCBThread::callback_data cbData;
            cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
            cbData.type         = CAMERA_MSG_ERROR;
            cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
            cbData.ext2         = 0;
            spFrameworkCBThread->postCB(cbData);
            spFrameworkCBThread->uninit();
            spFrameworkCBThread = NULL;
            status = OK;
            goto lbExit;
        }
        //
        if ( mpPreferIdleCpuCtrl )
        {
            mpPreferIdleCpuCtrl->enablePreferIdle();
        }

        if  ( ! onStartPreview() )
        {
            MY_LOGE("onStartPreviewLocked() fail");
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }

    {
        CAM_TRACE_NAME("clientStartPreview");
        if  ( mpDisplayClient == 0 )
        {
            MY_LOGD("DisplayClient is not ready.");
        }
        else if ( OK != (status = enableDisplayClient()) )
        {
            goto lbExit;
        }

        if  ( mpCamClient != 0 )
        {
            if  ( ! mpCamClient->startPreview() )
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startPreview();
        }
    }

    //  startPreview in Camera Adapter.
    {
        if(usePreviewThread)
        {
            if( pthread_create(&mStartPreviewTThreadHandle, NULL, startPreviewThread, this) != 0 )
            {
                ALOGE("startPreview pthread create failed");
            }
        }
        else
        {
            CAM_TRACE_NAME("adapterStartPreview");
            pCamMgr->getStartPreviewLock();
            status = mpCamAdapter->startPreview();
            pCamMgr->releaseStartPreviewLock();
            if  ( OK != status )
            {
                MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
                sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(getInstanceId(),mpCamMsgCbInfo);
                spFrameworkCBThread->init();
                IFrameworkCBThread::callback_data cbData;
                cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
                cbData.type         = CAMERA_MSG_ERROR;
                cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
                cbData.ext2         = 0;
                spFrameworkCBThread->postCB(cbData);
                spFrameworkCBThread->uninit();
                spFrameworkCBThread = NULL;
                status = OK;
                MY_LOGE("startPreview fail, return OK, callback CAMERA_ERROR_SERVER_DIED");
                goto lbExit;
            }
        }
    }

    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    mIsPreviewEnabled = true;
    status = OK;
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup after error");
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopPreview();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        disableDisplayClient();

        if ( mpPreferIdleCpuCtrl )
        {
            mpPreferIdleCpuCtrl->disable();
        }
    }
    MY_LOGI("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
stopPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");

    ::android::Mutex::Autolock _lOpsLock(mOpsLock);

    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    #if 0
    // enable check thread
    mbCheckThread = MTRUE;

    auto checkT = [this] () {
        MERROR ret = OK;
        MINT32 count = 0;
        MINT32 max = ::property_get_int32("debug.camera.stoppreview.checkt", 1000);
        MY_LOGD("checkThread (%d) +", count);
        while(this->runCheckThread()){
            usleep(10*1000); // 10 ms
            count++;
            if(count > max){
                sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getInstanceId());
                // callstack
                std::string callstack;
                UnwindCurProcessBT(&callstack);
                __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG"_TIMEOUT", callstack.c_str());
                //run exit scenario
                if (pScenarioCtrl != NULL){
                    pScenarioCtrl->exitScenario();
                    pScenarioCtrl = NULL;
                }
                // run NE flow
                //*((volatile int*)(0x0)) = 0xDEADFEED;
                CAM_LOGE("suicide: kill cameraserver - raise(SIGINT)");
                ::raise(SIGINT);
                CAM_LOGE("suicide: kill cameraserver - raise(SIGTERM)");
                ::raise(SIGTERM);
                CAM_LOGE("suicide: kill cameraserver - raise(SIGKILL)");
                ::raise(SIGKILL);
            }
        }
        MY_LOGD("checkThread (%d) -", count);
        return ret;
    };

    // fire a check thread to check whether flush is stuck
    mFuture = std::async(std::launch::async, checkT);
    #endif
    //
    if(mpCpuCtrl != NULL)
    {
        //mpCpuCtrl->init();
        mpCpuCtrl->cpuPerformanceMode(1);
    }
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    waitStartPreviewDone();
    if  ( ! previewEnabled() )
    {
        MY_LOGD("Preview already stopped, perhaps!");
        MY_LOGD("We still force to clean up again.");
    }
    {
        CAM_TRACE_NAME("adapterStopPreview");
        if  ( mpCamAdapter != 0 )
        {
            if(mpCamAdapter->isTakingPicture())
            {
                cancelPicture();
            }
            //
            if(recordingEnabled())
            {
                stopRecording();
            }
            mpCamAdapter->stopPreview();
        }
    }
    {
        CAM_TRACE_NAME("clientStopPreview");
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopPreview();
        }
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        disableDisplayClient();
    }

    {
        CAM_TRACE_NAME("deviceStopPreview");
        onStopPreview();
    }
#if 1
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->waitUntilDrained();
    }
#endif
    //
    //
    if ( mpPreferIdleCpuCtrl )
    {
        mpPreferIdleCpuCtrl->disable();
    }

lbExit:
    //  Always set it to false.
    mIsPreviewEnabled = false;
    #if 0
    // disable check thread
    mbCheckThread = MFALSE;
    mFuture.get();
    #endif
    //
    MY_LOGI("-");
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice1Base::
previewEnabled()
{
    MY_LOGD("+ mIsPreviewEnabled:%d, mpCamAdapter:%p", mIsPreviewEnabled, mpCamAdapter.get());

    if  ( ! mIsPreviewEnabled )
    {
        return  false;
    }
    else if(mStartPreviewTThreadHandle != 0)
    {
        return true;
    }
    else{
        return  ( mpCamAdapter == 0 )
            ?   false
            :   mpCamAdapter->previewEnabled()
            ;
    }

    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
storeMetaDataInBuffers(bool enable)
{
    MY_LOGD("+");

    status_t status = OK;
    if  ( mpCamClient != 0 )
    {
        //  Get recording format & size.
        //  Set.
        if  ( OK != mpCamClient->storeMetaDataInBuffers(enable) )
        {
            status = INVALID_OPERATION;
        }
    }
    if (enable) {
        mMetadataMode = true;
    }

    MY_LOGD("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
 * message is sent with the corresponding frame. Every record frame must be released
 * by a cameral hal client via releaseRecordingFrame() before the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
 * to manage the life-cycle of the video recording frames, and the client must
 * not modify/access any video recording frames.
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
startRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    status_t status = OK;
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }

    if  ( ! onStartRecording() )
    {
        MY_LOGE("onStartRecording() fail");
        status = INVALID_OPERATION;
        goto lbExit;
    }
    // zsd -> default
    if ( !strcmp(mpCamAdapter->getName(), MtkCameraParameters::APP_MODE_NAME_MTK_ZSD) )
    {
        MY_LOGW("invalid camera adapter: %s", mpCamAdapter->getName());
        stopPreview();
        mpParamsMgr->setForceHalAppMode(String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT));
        startPreview();
    }
    //  startRecording in Camera Adapter.
    {
        CAM_TRACE_NAME("adapterStartRecording");
        status = mpCamAdapter->startRecording();
        if  ( OK != status )
        {
            MY_LOGE("startRecording() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }

    {
        CAM_TRACE_NAME("clientStartRecording");
        if  ( mpCamClient != 0 )
        {
            //  Get recording format & size.
            //  Set.
            if  ( ! mpCamClient->startRecording() )
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startRecording();
        }
    }
    //
    {
        MINT32 enableFd = property_get_int32("debug.camera.VR.FD", -1);
        if( enableFd > 0 )
        {
            sendCommand((CommandType)CAMERA_CMD_START_FACE_DETECTION, 0, 0);
        }
        else if( enableFd == 0 )
        {
            sendCommand((CommandType)CAMERA_CMD_STOP_FACE_DETECTION, 0, 0);
        }
    }
lbExit:
    MY_LOGI("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 * Stop a previously started recording.
 ******************************************************************************/
Return<void>
CameraDevice1Base::
stopRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    MINT32 enableFd = property_get_int32("debug.camera.VR.FD", -1);
    if( enableFd > 0 )
    {
        sendCommand((CommandType)CAMERA_CMD_STOP_FACE_DETECTION, 0, 0);
    }

    {
        CAM_TRACE_NAME("adapterStopRecording");
        if  ( mpCamAdapter != 0 )
        {
            mpCamAdapter->stopRecording();
        }
    }

    {
        CAM_TRACE_NAME("clientStopRecording");
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopRecording();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopRecording();
        }
    }
    // check if need default -> zsd
    if  ( mpCamAdapter != 0 && msgTypeEnabled(CAMERA_MSG_PREVIEW_METADATA))
    {
        String8 halappmode;
        if  ( mpParamsMgr->evaluateHalAppMode(halappmode) &&
                halappmode != mpCamAdapter->getName() )
        {
            stopPreview();
            startPreview();
        }
    }
    MY_LOGI("-");
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<bool>
CameraDevice1Base::
recordingEnabled()
{
    MY_LOGD("+");

    bool ret = ( mpCamAdapter == 0 )
               ?   false
               :   mpCamAdapter->recordingEnabled()
               ;

    MY_LOGD("- status(%d)", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
releaseRecordingFrame(__attribute__((unused)) uint32_t memId, __attribute__((unused)) uint32_t bufferIndex)
{
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
releaseRecordingFrameHandle(__attribute__((unused)) uint32_t memId, __attribute__((unused)) uint32_t bufferIndex, __attribute__((unused)) const hidl_handle& frame)
{
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
releaseRecordingFrameHandleBatch(const hidl_vec<VideoFrameMessage>&)
{
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
autoFocus()
{
    MY_LOGD("+");

    waitStartPreviewDone();

    if  ( ! previewEnabled() )
    {
        MY_LOGW("- autoFocus: preview is not enabled");
        return Status::OK;
    }
    //
    //disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    Status status = ( mpCamAdapter == 0 )
                    ? Status::INTERNAL_ERROR
                    : mapToHidlCameraStatus(mpCamAdapter->autoFocus())
                    ;

    MY_LOGD("- status(%d)", status);
    return  status;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
takePicture()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");

    // FIXME: should get from 3A flow
    int Calibration = ::property_get_int32("vendor.flash_calibration", 0) ||
                      ::property_get_int32("vendor.flash_quick_calibration_2", 0) ||
                      ::property_get_int32("vendor.flash_ratio", 0) ||
                      ::property_get_int32("vendor.flash_ratio_quick2", 0);
    //eng mode skip jobmonitor (timeout mechanism)
    String8 s8ClientAppModeVal(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("vendor.mtk.client.appmode", value, "NULL");
    s8ClientAppModeVal = value;
    if ( s8ClientAppModeVal == MtkCameraParameters::APP_MODE_NAME_MTK_ENG )
    {
        Calibration = 1;
        MY_LOGD("eng mode skip jobmonitor");
    }
    //
    sp<JobMonitor::Helper> monitor = NULL;
    if( Calibration != 1 )
    {
        monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    }
    CAM_TRACE_CALL();
    status_t status = OK;
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }

    if(!(mLastEnableMsg & CAMERA_MSG_SHUTTER))
    {
        MY_LOGD("Disable shutter sound");
        disableMsgType(CAMERA_MSG_SHUTTER);
    }

    {
        CAM_TRACE_NAME("clientTakePicture");
        //
        if  ( mpCamClient != 0 )
        {
            mpCamClient->takePicture();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->takePicture();
        }
    }

    {
        CAM_TRACE_NAME("adapterTakePicture");
        //  takePicture in Camera Adapter.
        if( mIsRaw16CBEnabled )
        {
            MY_LOGD("CAMERA_CMD_ENABLE_RAW16_CALLBACK:1");
            mpCamAdapter->sendCommand(CAMERA_CMD_ENABLE_RAW16_CALLBACK, 1, 0);
        }
        //
        status = mpCamAdapter->takePicture();
        if  ( OK != status )
        {
            MY_LOGE("CamAdapter->takePicture() returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }
lbExit:
    MY_LOGI("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
cancelPicture()
{
    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    Status status = ( mpCamAdapter == 0 )
                      ?   Status::INTERNAL_ERROR
                      :   mapToHidlCameraStatus(mpCamAdapter->cancelPicture())
                      ;
    MY_LOGI("- status(%d)", status);
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
cancelAutoFocus()
{
    waitStartPreviewDone();

    MY_LOGD("+");
    status_t status = ( mpCamAdapter == 0 )
        ?   OK
        :   mpCamAdapter->cancelAutoFocus()
        ;
    //
    //enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    MY_LOGD("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
setParameters(const hidl_string& params)
{
    MY_LOGD("+");
    CAM_TRACE_CALL();
    //
    waitStartPreviewDone();

    status_t status = OK;
    //
    //  (1) Update params to mpParamsMgr.
    if(mpParamsMgr != 0)
    {
        status = mpParamsMgr->setParameters(String8(params.c_str()));
        //
        if(mpParamsMgr->isSetParametersChangeNothing())
        {
            status_t status = OK;
            MY_LOGD("No params changed in this setParameters, don't update the params");
            MY_LOGD("-");
            return mapToHidlCameraStatus(status);
        }
    }
    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    if ( OK == status ){
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
        status = onSetParameters(params.c_str());
    }
    MY_LOGD("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
getParameters(getParameters_cb _hidl_cb)
{
    MY_LOGD("+");
    if (mpParamsMgr != nullptr){
        waitStartPreviewDone();
        hidl_string outStr;
        String8 params_str8 = mpParamsMgr->flatten();
        uint32_t const params_len = sizeof(char) * (params_str8.length()+1);
        char*const params_string = (char*)::malloc(params_len);
        if  ( params_string )
        {
            strcpy(params_string, params_str8.string());
        }
        //
        MY_LOGV_IF(0, "- params(%p), len(%d)", params_string, params_len);
        outStr = params_string;

        //release the temp string (put_parameters)
        if  ( params_string )
        {
            free(params_string);
        }
        MY_LOGD("- length %d", params_str8.length());
        _hidl_cb(outStr);
    }
    else{
        MY_LOGD("-");
    }
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice1Base::
sendCommand(CommandType cmd, int32_t arg1, int32_t arg2)
{
    MY_LOGD("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);
    waitStartPreviewDone();

    status_t status = DEAD_OBJECT;
    int32_t mcmd = (int32_t) cmd;
    //for display rotation
    switch  (mcmd)
    {
    case CAMERA_CMD_SET_DISPLAY_ORIENTATION:
        mOrientation = arg1;
        MY_LOGD("CAMERA_CMD_SET_DISPLAY_ORIENTATION mOrientation = %d",mOrientation);
        break;
    default:
        break;
    }

    //for set consumer usage
    switch  (mcmd)
    {
    case CAMERA_CMD_SET_NATIVE_WINDOW_CONSUMER_USAGE:
        mNativeWindowConsumerUsage = arg1;
        MY_LOGD("CAMERA_CMD_SET_NATIVE_WINDOW_CONSUMER_USAGE mNativeWindowConsumerUsage = %d",mNativeWindowConsumerUsage);
        break;
    default:
        break;
    }

    //check raw16 CB enable
    switch  (mcmd)
    {
    case CAMERA_CMD_ENABLE_RAW16_CALLBACK:
         if(arg1 == 1)
            mIsRaw16CBEnabled = true;
         else if(arg1 == 0)
            mIsRaw16CBEnabled = false;
         MY_LOGD("CAMERA_CMD_ENABLE_RAW16_CALLBACK mIsRaw16CBEnabled = %d",mIsRaw16CBEnabled);
         break;

#if '1'==MTKCAM_HAVE_OT_CLIENT
        case CAMERA_CMD_START_OT:
    #if '1'==MTKCAM_HAVE_3A_HAL
            OT_Customize_PARA OTCustomData1;
            get_ot_CustomizeData(&OTCustomData1);
            MY_LOGD("Custom Data: AE_AWB lock=%d", OTCustomData1.AEAWB_LOCK);
            if (OTCustomData1.AEAWB_LOCK==1) {
                setParameters("auto-exposure-lock=true;auto-whitebalance-lock=true");
            }
    #endif
            break;

        case CAMERA_CMD_STOP_OT:
    #if '1'==MTKCAM_HAVE_3A_HAL
            setParameters("auto-exposure-lock=false;auto-whitebalance-lock=false");
    #endif
            break;
#endif // MTKCAM_HAVE_OT_CLIENT

    default:
         break;
    }
    //
    switch  (mcmd)
    {
    case CAMERA_CMD_PLAY_RECORDING_SOUND:
        MY_LOGD("-");
        return  Status::OK;
    default:
        break;
    }
    //
    //  (1) try to see if Camera Adapter can handle this command.
    sp<ICamAdapter> pCamAdapter = mpCamAdapter;
    if  ( pCamAdapter != 0 && INVALID_OPERATION != (status = pCamAdapter->sendCommand(mcmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        MY_LOGD("- status(%d)", status);
        return  mapToHidlCameraStatus(status);
    }
    //
    //  (2) try to see if Camera Client can handle this command.
    sp<ICamClient> pCamClient = mpCamClient;
    if  ( pCamClient != 0 && INVALID_OPERATION != (status = pCamClient->sendCommand(mcmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        MY_LOGD("- status(%d)", status);
        return  mapToHidlCameraStatus(status);
    }
    //
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->sendCommand(mcmd, arg1, arg2);
    }
    //
    switch  (mcmd)
    {
    case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            ssize_t index = mTodoCmdMap.indexOfKey(mcmd);
            if  (index < 0)
            {
                MY_LOGD("queue cmd(%#x),args(%d,%d)", mcmd, arg1, arg2);
                mTodoCmdMap.add(mcmd, CommandInfo(mcmd, arg1, arg2));
            }
            else
            {
                MY_LOGW("queue the same cmd(%#x),args(%d,%d) again", mcmd, arg1, arg2);
                mTodoCmdMap.editValueAt(index) = CommandInfo(mcmd, arg1, arg2);
            }
        }
        status = OK;
        break;
    default:
        MY_LOGW("not handle cmd(%#x),args(%d,%d)", mcmd, arg1, arg2);
        break;
    }

    MY_LOGD("- status(%d)", status);
    return  mapToHidlCameraStatus(status);

}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice1Base::
close()
{
    ::android::Mutex::Autolock _lOpsLock(mOpsLock);

    MY_LOGI("+");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    ::android::status_t status = OK;
    std::string const& instanceName = mInstanceName;

    status = mDeviceManager->startCloseDevice(instanceName);
    if ( OK != status ){
        MY_LOGW("startCloseDevice [%d %s]", -status, ::strerror(-status));
    }

    onCloseLocked();

    status= mDeviceManager->finishCloseDevice(instanceName);

    if ( OK != status ){
        MY_LOGW("finishCloseDevice [%d %s]", -status, ::strerror(-status));
    }

    sSetDevConnect(instanceName, false);

    //update device list
    {
        Mutex::Autolock lock(gCameraDevice1Baselock);
        MBOOL bIsFind = MFALSE;
        list<wp<CameraDevice1Base>>::iterator it = glwpCameraDevice1Base.begin();
        while(it != glwpCameraDevice1Base.end())
        {
            if((*it).promote().get() == 0 ||
               (*it).promote().get() == this ||
               (*it).promote()->getInstanceId() == mInstanceId )
            {
                MY_LOGI("Del cameraId %d - %p, this - %p", mInstanceId, (*it).promote().get(), this);
                it = glwpCameraDevice1Base.erase(it);
                bIsFind = MTRUE;
            }
            else
            {
                it++;
            }
        }
        if(!bIsFind)
        {
            MY_LOGW("Cannot find cameraId %d, this - %p", mInstanceId, this);
        }
    }

    Utils::Property::set(String8(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE), String8::empty());

    MY_LOGI("-");
    return Void();
}

/******************************************************************************
 * Template mothods
 ******************************************************************************/
status_t
CameraDevice1Base::
onOpenLocked()
{
    MY_LOGD("+");
    //[1] Create and initialize ParametersManager
    mpParamsMgr = IParamsManager::createInstance(
                                      mDevName,
                                      mInstanceId
                                  );
    if  ( ! mpParamsMgr->init() )
    {
        MY_LOGE("mpParamsMgr(%p)->init() fail", mpParamsMgr.get());
        return NO_INIT;
    }

    //[2] Create & Initialize ICamClient.
    mpCamClient = ICamClient::createInstance(mpParamsMgr);
    if  ( mpCamClient == 0 || ! mpCamClient->init() )
    {
        MY_LOGE("mpCamClient(%p)->init() fail", mpCamClient.get());
        return NO_INIT;
    }

    //  (3) override params to static metadata
    mpParamsMgrV3 = IParamsManagerV3::createInstance(mDevName, mInstanceId, mpParamsMgr);
    mpParamsMgrV3->setStaticData();

    MY_LOGD("-");
    return OK;
}

/******************************************************************************
 * Template mothods
 ******************************************************************************/
void
CameraDevice1Base::
onCloseLocked()
{
    MY_LOGD("+");
    //
    //
    //  recover params to static metadata
    if ( mpParamsMgrV3 != 0 )
    {
        mpParamsMgrV3->restoreStaticData();
    }
    //
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->uninit();
        mpCamClient.clear();
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    if(!vmpCamClient.isEmpty())
    {
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->uninit();
        }
        vmpCamClient.clear();
    }
    //
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    //
    //
    if ( mpParamsMgr != 0 )
    {
        mpParamsMgr->uninit();
    }
    //
    //  Uninitialize Device Callback.
    mpCamMsgCbInfo->mCam1DeviceCb = NULL;
    //
    if(mpCpuCtrl != NULL)
    {
        mpCpuCtrl->disable();
    }
    //
    MY_LOGD("-");
    return;
}

