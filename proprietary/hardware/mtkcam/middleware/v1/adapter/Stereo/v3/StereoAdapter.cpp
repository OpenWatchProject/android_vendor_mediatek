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

#define LOG_TAG "MtkCam/StereoAdapter"
//
#include "MyUtils.h"
//
#include <inc/CamUtils.h>
#include <cutils/properties.h>  // For property_get().
#include <cutils/atomic.h>

using namespace android;
using namespace MtkCamUtils;
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
//
#include "inc/v3/StereoAdapter.h"
using namespace NSStereoAdapter;
//
#include <mtkcam/drv/IHalSensor.h>
using namespace NSCam;
//
#include <StereoFlowControl.h>
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)

//
#define FUNC_START                  MY_LOGD2("+")
#define FUNC_END                    MY_LOGD2("-")
//
#define ENABLE_MAIN2_ISO            (1600)
#define DISENABLE_MAIN2_ISO         (800)

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
*
*******************************************************************************/
sp<ICamAdapter>
createStereoAdapter(
    String8 const&      rAppMode,
    int32_t const       i4OpenId,
    sp<IParamsManagerV3>  pParamsMgrV3
)
{
    CAM_LOGD("createStereoAdapter");
    return new CamAdapter(
        rAppMode,
        i4OpenId,
        pParamsMgrV3
    );
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
CamAdapter(
    String8 const&      rAppMode,
    int32_t const       i4OpenId,
    sp<IParamsManagerV3>  pParamsMgrV3
)
    : BaseCamAdapter(rAppMode, i4OpenId, pParamsMgrV3)
    //
    , msAppMode(rAppMode)
    //
    , msFocusArea("")
    , mEngParam()
{
    char cProperty[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("vendor.camera.adapter.loglevel", cProperty, "2");
    mLogLevel = ::atoi(cProperty);

    MY_LOGD1(
        "AppMode:%s, sizeof=%d, this=%p, mpStateManager=%p",
        msAppMode.string(),
        sizeof(CamAdapter),
        this,&mpStateManager
    );
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
~CamAdapter()
{
    MY_LOGD1("tid(%d), OpenId(%d)", ::gettid(), getOpenId());
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
init()
{
    FUNC_START;
    bool ret = false;
    status_t status = OK;
    sp<StereoFlowControl> stereoFlowControl = nullptr;
    sp<IShotCallback> cb = this;
    sp<ZipImageCallbackThread> pCallbackThread = NULL;
    MINT32 sensorId_main  = -1;
    MINT32 sensorId_main2  = -1;
    //
    mpStateManager = IStateManager::createInstance();
    if ( mpStateManager != NULL )
    {
        if(!(mpStateManager->init()))
        {
            MY_LOGE("mpStateManager->init fail");
            goto lbExit;
        }
    }
    //
    MY_LOGD("stereo mode");
    mFlowControlType = IFlowControl::ControlType_T::CONTROL_STEREO;

    mpFlowControl = IFlowControl::createInstance(
                            "StereoCamAdapter",
                            getOpenId(),
                            mFlowControlType,
                            mpParamsManagerV3,
                            mpImgBufProvidersMgr,
                            this
                        );
    if ( mpFlowControl == 0 ) goto lbExit;

    mpCaptureCmdQueThread = ICaptureCmdQueThread::createInstance(this);
    if  ( mpCaptureCmdQueThread == 0 || OK != (status = mpCaptureCmdQueThread->run("Capture@CmdQue") ) )
    {
        MY_LOGE(
            "Fail to run CaptureCmdQueThread - mpCaptureCmdQueThread.get(%p), status[%s(%d)]",
            mpCaptureCmdQueThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }

    pCallbackThread = new ZipImageCallbackThread( mpStateManager, mpCamMsgCbInfo);
    if  ( !pCallbackThread.get() || OK != (status = pCallbackThread->run("ZipImageCallbackThread") ) )
    {
        MY_LOGE(
            "Fail to run ZipImageCallbackThread - pCallbackThread.get(%p), status[%s(%d)]",
            pCallbackThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    mpZipCallbackThread = pCallbackThread;

    mpFrameworkCBThread = IFrameworkCBThread::createInstance(getOpenId(), mpCamMsgCbInfo);
    if  ( mpFrameworkCBThread == 0 || OK != (status = mpFrameworkCBThread->init() ) )
    {
        MY_LOGE(
            "Fail to run mpFrameworkCBThread - mpFrameworkCBThread.get(%p), status[%s(%d)]",
            mpFrameworkCBThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    //
    //ret = updateShotInstance();
    ret = ((StereoFlowControl*)mpFlowControl.get())->setCallbacks(this);
lbExit:
    if(!ret)
    {
        MY_LOGE("init() fail; now call uninit()");
        uninit();
    }
    FUNC_END;
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
uninit()
{
    FUNC_START;
    bool ret = true;
    //  Close Command Queue Thread of Capture.
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    mpCaptureCmdQueThread = 0;
    if  ( pCaptureCmdQueThread != 0 ) {
        pCaptureCmdQueThread->requestExit();
        pCaptureCmdQueThread->join();
        pCaptureCmdQueThread = 0;
    }
    //
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    if( pCallbackThread != NULL )
    {
        pCallbackThread->forceExit();
        pCallbackThread = NULL;
    }
    mpZipCallbackThread = NULL;
    //
    if( previewEnabled())
    {
        MY_LOGD("Force to stop preview start (%d)",mbTakePicPrvNotStop);
        stopPreview();
        MY_LOGD1("Force to stop preview done");
    }
    //  Close NotifyCB Thread
    sp<IFrameworkCBThread> pFrameworkCBThread = mpFrameworkCBThread;
    mpFrameworkCBThread = 0;
    if  ( pFrameworkCBThread != 0 ) {
        pFrameworkCBThread->stopCB();
        pFrameworkCBThread->uninit();
        pFrameworkCBThread = 0;
    }
    //
    if(mpStateManager != NULL)
    {
        mpStateManager->uninit();
        mpStateManager->destroyInstance();
        mpStateManager = NULL;
    }
    //
    mpFlowControl = NULL;
    //
    BaseCamAdapter::uninit();
    //
    FUNC_END;
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    FUNC_START;
    MY_LOGD1("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)",cmd,arg1,arg2);

    //
    switch(cmd)
    {
        case CAMERA_CMD_CANCEL_CSHOT:
        {
            return cancelPicture();
        }
        case CAMERA_CMD_SET_CSHOT_SPEED:
        {
            return setCShotSpeed(arg1);
        }
        case CAMERA_CMD_ENABLE_RAW16_CALLBACK:
        {
            CAM_LOGD("[sendCommand] CAMERA_CMD_ENABLE_RAW16_CALLBACK (%d)\n", arg1);
            mIsRaw16CBEnable = ( arg1 == 1 ) ? MTRUE :  MFALSE;
            mpFlowControl->sendCommand(StereoFlowControl::CONTROL_DNG_FLAG, mIsRaw16CBEnable, 0);
            return OK;
        }
        default:
        {
            break;
        }
    }

    return mpFlowControl->sendCommand(cmd, arg1, arg2);
}

/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
doNotifyCallback(
    int32_t _msgType,
    int32_t _ext1,
    int32_t _ext2
)
{
    IFrameworkCBThread::callback_data cbData;
    cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
    cbData.type         = _msgType;
    cbData.ext1         = _ext1;
    cbData.ext2         = _ext2;

    if(mpFrameworkCBThread != nullptr){
        mpFrameworkCBThread->postCB(cbData);
    }
}

/******************************************************************************
*
*******************************************************************************/

void
CamAdapter::
doDataCallback(
    int32_t  _msgType,
    void*    _data,
    uint32_t  _size
)
{
    switch(_msgType)
    {
        case MTK_CAMERA_MSG_EXT_DATA_AF:
        {
            int32_t* pSrc = reinterpret_cast<int32_t*>(_data);
            for(int i=0;i<_size;i++)
            {
                MY_LOGD2("data(%d)=%d",i,pSrc[i]);
            }
            camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, sizeof(int32_t)*(_size+1), 1, mpCamMsgCbInfo->mCbCookie);
            if (pmem)
            {
                int32_t* pDst = reinterpret_cast<int32_t*>(pmem->data);
                pDst[0] = MTK_CAMERA_MSG_EXT_DATA_AF;
                for (uint32_t i = 0; i < _size; i++)
                {
                    pDst[i+1] = pSrc[i];
                }
                for(int i=0;i<_size+1;i++)
                {
                    MY_LOGD2("pDst(%d)=%d",i,pDst[i]);
                }
                //
                IFrameworkCBThread::callback_data cbData;
                cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_DATA;
                cbData.type         = MTK_CAMERA_MSG_EXT_DATA;
                cbData.pMemory      = pmem;
                cbData.pMetadata    = NULL;
                if (mpFrameworkCBThread != 0) mpFrameworkCBThread->postCB(cbData);
            }
            break;
        }
        default:
        {
            MY_LOGW("Not support this data callback");
            break;
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
doExtCallback(
    int32_t _msgType,
    int32_t _ext1,
    int32_t _ext2
)
{
    if (_msgType == MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE)
    {
        sp<ICam1DeviceCallback> pCam1DeviceCb = mpCamMsgCbInfo->mCam1DeviceCb.promote();
        if  ( pCam1DeviceCb != 0 ) {
            pCam1DeviceCb->onCam1Device_NotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE, 0);
        }
    }
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
msgTypeEnabled( int32_t msgType )
{
    return msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}
