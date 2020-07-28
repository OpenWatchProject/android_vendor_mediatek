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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
#include <cutils/properties.h>  // For property_get().

using namespace android;
using namespace MtkCamUtils;
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkDefaultCamAdapter.h"
using namespace NSMtkDefaultCamAdapter;
//
#include <mtkcam/drv/IHalSensor.h>
using namespace NSCam;
//


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


/******************************************************************************
*
*******************************************************************************/
sp<ICamAdapter>
createMtkDefaultCamAdapter(
    String8 const&      rName,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
)
{
    return new CamAdapter(
        rName,
        i4OpenId,
        pParamsMgr
    );
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
CamAdapter(
    String8 const&      rName,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
)
    : BaseCamAdapter(rName, i4OpenId, pParamsMgr)
    //
    , mpStateManager(NULL)
    , mspStateManager(NULL)
    //
    , mpCaptureCmdQueThread(0)
    , mpZipCallbackThread(0)
    , mpFrameworkCBThread(0)
    //
    , mpShot(0)
    #if '1'==MTKCAM_HAVE_3A_HAL
    , mpCallback3a(NULL)
    , mspCallback3a(NULL)
    #endif
    //
    , mUserName("None")
    , mpCamGraph(NULL)
    , mpAllocBufHdl(NULL)
    , mpDefaultBufHdl(NULL)
    //
    , mpPass1Node(NULL)
    , mpDefaultCtrlNode(NULL)
    , mspDefaultCtrlNode(NULL)
    , mpPass2Node(NULL)
    , mpRecBufSnapshotlNode(NULL)
    //
    , mbTwoRunP2(MFALSE)
    , mpTwoRunP1AllocBufHdl(NULL)
    , mpExtImgProcNode(NULL)
    , mpImgTransformNode(NULL)
    //
    , mpPdafNode(NULL)
    //
    , mpCapBufMgr(NULL)
    //
    , mbTakePicPrvNotStop(false)
    , mbFixFps(false)
    , mPreviewMaxFps(0)
    , mShotMode(0)
    //
    , mLastVdoWidth(0)
    , mLastVdoHeight(0)
    //
    , mspThermalMonitor(NULL)
    , mpResMgrDrv(NULL)
    , mb4K2KVideoRecord(MFALSE)
    , mbLowPowerVideoRecord(MTRUE)
    , mPipStartPreviewFail(MFALSE)
    , mEnRaw16CB(0)
    , mspRwbGpuProc(NULL)
    , reuseZsdP2outBuf(NULL)
    , mb4CellReMosaicCapFlow(MFALSE)
    , mbCaptureFinished(MFALSE)
{
    MY_LOGD(
        "sizeof=%d, this=%p, mpStateManager=%p",
        sizeof(CamAdapter),
        this,&mpStateManager
    );
    //Set default value
    mpParamsMgr->getDefaultPreviewFps(mDefaultPreviewFps);
    mpParamsMgr->getDefaultPreviewFpsRange(mDefaultPreviewFpsRange[0],mDefaultPreviewFpsRange[1]);
    MY_LOGD("FPS:%d,%d-%d",
            mDefaultPreviewFps,
            mDefaultPreviewFpsRange[0],
            mDefaultPreviewFpsRange[1]);
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
~CamAdapter()
{
    MY_LOGD("tid(%d), OpenId(%d)", ::gettid(), getOpenId());
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
init()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    mpResMgrDrv = ResMgrDrv::createInstance(getOpenId());
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_IDLE;
        mode.dev = ResMgrDrv::DEV_CAM;
        mpResMgrDrv->init();
        mpResMgrDrv->setMode(&mode);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_PIP,
                CamManager::getInstance()->isMultiDevice());
    }
    else
    {
        MY_LOGE("mpResMgrDrv is NULL");
        ret = false;
    }
    //RWB sensor
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    if(pIHalSensorList)
    {
        MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());

        NSCam::SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
        pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        //
        if(sensorStaticInfo.rawFmtType == SENSOR_RAW_RWB)
        {
            //RwbGpuProcess init
            mspRwbGpuProc = RwbGpuProc::createInstance(getOpenId());
            if(mspRwbGpuProc == NULL)
            {
                MY_LOGE("mspRwbGpuProc is NULL");
                ret = false;
            }
        }
    }
    //
    mpCapBufMgr         = CapBufMgr::createInstance();
    mpAllocBufHdl       = AllocBufHandler::createInstance();
    mpDefaultBufHdl     = DefaultBufHandler::createInstance(
                                                "",
                                                mpImgBufProvidersMgr);
    //
    mpDefaultCtrlNode   = DefaultCtrlNode::createInstance("", DefaultCtrlNode::CTRL_NODE_DEFAULT);
    mspDefaultCtrlNode = mpDefaultCtrlNode;
    //
    if(mpDefaultBufHdl)
    {
        MY_LOGD("DefaultBufHandler:init");
        mpDefaultBufHdl->init();
    }
    //
    mpStateManager = IStateManager::createInstance();
    mspStateManager = mpStateManager;
    if ( mpStateManager != NULL )
    {
        if(!(mpStateManager->init()))
        {
            MY_LOGE("mpStateManager->init fail");
            goto lbExit;
        }
    }
    //
    if(OK != init3A())
    {
        MY_LOGE("Fail to init 3A");
        goto lbExit;
    }
    //
    mspThermalMonitor = ThermalMonitor::createInstance(
                                            IHal3A::E_Camera_1,
                                            getOpenId());
    //
    mpCaptureCmdQueThread = ICaptureCmdQueThread::createInstance(this);
    if  ( mpCaptureCmdQueThread == 0 || OK != (status = mpCaptureCmdQueThread->run("CamAdapter::init") ) )
    {
        MY_LOGE(
            "Fail to run CaptureCmdQueThread - mpCaptureCmdQueThread.get(%p), status[%s(%d)]",
            mpCaptureCmdQueThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }

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
    ret = true;
lbExit:
    if(!ret)
    {
        MY_LOGE("init() fail; now call uninit()");
        uninit();
    }
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
uninit()
{
    MY_LOGD("+");
    //
    //RWB sensor
    if(mspRwbGpuProc != NULL)
    {
        mspRwbGpuProc->destroyInstance();
        mspRwbGpuProc = NULL;
    }

    //
    //  Close Command Queue Thread of Capture.
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    mpCaptureCmdQueThread = 0;
    if  ( pCaptureCmdQueThread != 0 ) {
        pCaptureCmdQueThread->requestExit();
        pCaptureCmdQueThread->join();
        pCaptureCmdQueThread = 0;
    }
    //
    {
        sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
        if  ( pCallbackThread.get() )
        {
            pCallbackThread->doCallback(ZipImageCallbackThread::callback_type_exit);
            pCallbackThread = 0;
            mpZipCallbackThread = 0;
        }

    }
    //
    if( previewEnabled() ||
        mbTakePicPrvNotStop)
    {
        MY_LOGD("Force to stop preview start (%d)",mbTakePicPrvNotStop);
        mbTakePicPrvNotStop = false;
        stopPreview();
        MY_LOGD("Force to stop preview done");
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
    if ( mpDefaultCtrlNode != NULL )
    {
        mpDefaultCtrlNode->destroyInstance();
        mpDefaultCtrlNode = NULL;
        mspDefaultCtrlNode = NULL;
    }

    //
    if ( mpAllocBufHdl != NULL )
    {
        mpAllocBufHdl->destroyInstance();
        mpAllocBufHdl = NULL;
    }
    //
    if ( mpDefaultBufHdl != NULL )
    {
        mpDefaultBufHdl->uninit();
        mpDefaultBufHdl->destroyInstance();
        mpDefaultBufHdl = NULL;
    }
    //
    if( mpCapBufMgr != NULL)
    {
        mpCapBufMgr->destroyInstance();
        mpCapBufMgr = NULL;
    }
    //
    if(mspThermalMonitor != NULL)
    {
        if(mspThermalMonitor->isRunning())
        {
            mspThermalMonitor->stop();
        }
        mspThermalMonitor->exit();
        mspThermalMonitor = NULL;
    }
    //
    uninit3A();
    //
    if(mpStateManager != NULL)
    {
        Mutex::Autolock _l(mStateLock);
        mpStateManager->uninit();
        mpStateManager->destroyInstance();
        mpStateManager = NULL;
    }
    mspStateManager = NULL;
    //
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_SENSOR_SIZE,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_SENSOR_FPS,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_PIP,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_VFB,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_EIS20,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_IVHDR,
                0);
        mpResMgrDrv->setItem(
                ResMgrDrv::ITEM_STEREO,
                0);
        mpResMgrDrv->triggerCtrl();
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_NONE;
        mpResMgrDrv->setMode(&mode);
        mpResMgrDrv->uninit();
        mpResMgrDrv->destroyInstance();
        mpResMgrDrv = NULL;
    }
    //
    freeReuseZsdP2outBuf();
    //
    BaseCamAdapter::uninit();
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    MY_LOGD("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)",cmd,arg1,arg2);
    //
    switch  (cmd)
    {
        case CAMERA_CMD_START_SMOOTH_ZOOM:
        {
            if(mpDefaultCtrlNode)
            {
                if(!mpDefaultCtrlNode->startSmoothZoom(arg1))
                {
                    return BAD_VALUE;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        case CAMERA_CMD_STOP_SMOOTH_ZOOM:
        {
            if(mpDefaultCtrlNode)
            {
                if(!mpDefaultCtrlNode->stopSmoothZoom())
                {
                    return INVALID_OPERATION;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        {
            CAM_LOGD("[sendCommand] CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG (%d)\n", arg1);
            enableAFMove(arg1);
            return OK;
        }
        case CAMERA_CMD_CANCEL_CSHOT:
        {
            cancelPicture();
            return OK;
        }
        case CAMERA_CMD_SET_CSHOT_SPEED:
        {
            return setCShotSpeed(arg1);
        }
        case CAMERA_CMD_START_GD_PREVIEW:
        {
            if(mpPass2Node)
            {
                if(!mpPass2Node->enableGDPreview(MTRUE))
                {
                    return INVALID_OPERATION;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        case CAMERA_CMD_CANCEL_GD_PREVIEW:
        {
            if(mpPass2Node)
            {
                if(!mpPass2Node->enableGDPreview(MFALSE))
                {
                    return INVALID_OPERATION;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        case CAMERA_CMD_ENABLE_RAW16_CALLBACK:
        {
            CAM_LOGD("[sendCommand] CAMERA_CMD_ENABLE_RAW16_CALLBACK (%d)\n", arg1);
            mEnRaw16CB = arg1;
            return OK;
        }
        case CAMERA_CMD_START_SD_PREVIEW:
        {
            if(mpPass2Node)
            {
                if(!mpPass2Node->enableSDPreview(MTRUE))
                {
                    return INVALID_OPERATION;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        case CAMERA_CMD_CANCEL_SD_PREVIEW:
        {
            if(mpPass2Node)
            {
                if(!mpPass2Node->enableSDPreview(MFALSE))
                {
                    return INVALID_OPERATION;
                }
                return OK;
            }
            else
            {
                return INVALID_OPERATION;
            }
        }
        default:
        {
            break;
        }
    }
    return  BaseCamAdapter::sendCommand(cmd, arg1, arg2);
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

    if (mpFrameworkCBThread != 0)
    {
        mpFrameworkCBThread->postCB(cbData);
    }
}
