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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/DCNode.Eng"
//
#include <cutils/properties.h>      // [debug] should be remove
//
#include <CamUtils.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <vector>
using namespace std;
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <DefaultCtrlNodeImpl.h>
using namespace NSCamNode;
//
#include <camera_custom_eis.h>

#define MAX_VIDEO_DUMPCNT  200
static uint32_t g_u4RawDumpCnt = 0;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeEngImpl::
DefaultCtrlNodeEngImpl(const char* userName)
    : DefaultCtrlNodeImpl(userName)
{
    FUNC_NAME;
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeEngImpl::
~DefaultCtrlNodeEngImpl()
{
    FUNC_NAME;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onInit()
{
    FUNC_START;
    //
    mpHal3a = IHal3A::createInstance(
                        IHal3A::E_Camera_1,
                        getSensorIdx(),
                        LOG_TAG);
    if(mpHal3a == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        return MFALSE;
    }
    //
    mpHal3a->addCallbacks(this);
    //
    updateDelay(EQueryType_Init);
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if(mpIspSyncCtrl)
    {
        MINT32 i;
        vector<MUINT32> zoomRatios;
        for(i=0; i<=mspParamsMgr->getInt(CameraParameters::KEY_MAX_ZOOM); i++)
        {
            zoomRatios.push_back(mspParamsMgr->getZoomRatioByIndex(i));
        }
        mpIspSyncCtrl->setZoomRatioList(zoomRatios);
        //
        if(CamManager::getInstance()->isMultiDevice())
        {
            mpIspSyncCtrl->setHdrState(SENSOR_VHDR_MODE_NONE);
        }
        else
        {
            mpIspSyncCtrl->setHdrState(mspParamsMgr->getVHdr());
        }
        mpIspSyncCtrl->addPass1ReplaceBuffer(mReplaceBufNumFullsize, mReplaceBufNumResize);
        mpIspSyncCtrl->setPreviewSize(mPreviewSize.w, mPreviewSize.h);
        if (mspParamsMgr != NULL)
        {
            mpIspSyncCtrl->setCurZoomRatio(mspParamsMgr->getZoomRatioByIndex(mspParamsMgr->getInt(CameraParameters::KEY_ZOOM)));
        }
        //
        MUINT32 sensorW, sensorH;
        mpIspSyncCtrl->getSensorInfo(
                            mSensorScenario,
                            sensorW,
                            sensorH,
                            mSensorType);
        mSensorSize.w = sensorW;
        mSensorSize.h = sensorH;
        //
        MSize videoSize, rrzoMinSize, initRrzoSize;
        //
        if( mSensorSize.w*mSensorSize.h >= IMG_2160P_SIZE &&
            mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE)
        {
            videoSize.w = IMG_2160P_W;
            videoSize.h = IMG_2160P_H;
        }
        else
        {
            videoSize.w = IMG_1080P_W;
            videoSize.h = IMG_1080P_H;
        }
        //
        rrzoMinSize.w = EIS_MIN_WIDTH;
        rrzoMinSize.h = (MUINT32)((EIS_MIN_WIDTH*mPreviewSize.h)/mPreviewSize.w);
        //
        MSize maxSize, sizeList[3] = {mPreviewSize, videoSize, rrzoMinSize};
        //4K2K video record does not support full size output.
        MY_LOGD("RH(%d)",mRecordingHint);
        if( mRecordingHint == MTRUE &&
            mMode == MODE_VIDEO_PREVIEW)
        {
            maxSize.w = 0;
            maxSize.h = 0;
            for(i=0; i<3; i++)
            {
                MY_LOGD("sizeList[%d]=%dx%d",
                        i,
                        sizeList[i].w,
                        sizeList[i].h);
                if(maxSize.w < sizeList[i].w)
                {
                   maxSize.w = sizeList[i].w;
                }
                if(maxSize.h < sizeList[i].h)
                {
                   maxSize.h = sizeList[i].h;
                }
            }
            //
            MY_LOGD("maxSize(%dx%d)",
                    maxSize.w,
                    maxSize.h);
            initRrzoSize.w = (maxSize.w*EIS_FACTOR)/100;
            initRrzoSize.h = (maxSize.h*EIS_FACTOR)/100;
            MY_LOGD("initRrzoSize(%dx%d)",
                    initRrzoSize.w,
                    initRrzoSize.h);
            //
            MUINT32 aaaUpdatePeriod;
            if(mPreviewMaxFps < AAA_UPDATE_BASE)
            {
                aaaUpdatePeriod = 1;
            }
            else
            {
                aaaUpdatePeriod = mPreviewMaxFps/AAA_UPDATE_BASE;
            }
            if(mspParamsMgr->getVideoStabilization())
            {
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))
                {
                    mpIspSyncCtrl->setVideoSize(3840, 2160);
                }
                else
                {
                    mpIspSyncCtrl->setVideoSize(
                                        (MUINT32)((mPreviewSize.w*EIS_FACTOR)/100),
                                        (MUINT32)((mPreviewSize.h*EIS_FACTOR)/100));
                }
            }
            else
            {
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))
                {
                    mpIspSyncCtrl->setVideoSize(3840, 2160);
                }
                else
                {
                    mpIspSyncCtrl->setVideoSize(0, 0);
                }
            }
            //
            mpIspSyncCtrl->set3AUpdatePeriod(aaaUpdatePeriod);
            mpIspSyncCtrl->setRrzoMinSize(
                                rrzoMinSize.w,
                                rrzoMinSize.h);
        }
        else
        {
            initRrzoSize.w = mPreviewSize.w;
            initRrzoSize.h = mPreviewSize.h;
            //
            mpIspSyncCtrl->setVideoSize(0, 0);
            mpIspSyncCtrl->set3AUpdatePeriod(1);
            mpIspSyncCtrl->setRrzoMinSize(0, 0);
        }
        //
        if( initRrzoSize.w > mSensorSize.w)
        {
            initRrzoSize.w = mSensorSize.w;
        }
        if( initRrzoSize.h > mSensorSize.h)
        {
            initRrzoSize.h = mSensorSize.h;
        }
        MY_LOGD("initRrzoSize(%dx%d)",
                initRrzoSize.w,
                initRrzoSize.h);
        //
        mpIspSyncCtrl->setPass1InitRrzoSize(
                            initRrzoSize.w,
                            initRrzoSize.h);
        mpIspSyncCtrl->calRrzoMaxZoomRatio();
    }
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpIspSyncCtrl->getHdrState() != SENSOR_VHDR_MODE_NONE)
    {
        mpVHdrHal = VHdrHal::CreateInstance(LOG_TAG,getSensorIdx());
        if(mpVHdrHal)
        {
            mpVHdrHal->Init(mspParamsMgr->getVHdr());
        }
        else
        {
            MY_LOGE("mpVHdrHal is NULL");
        }
    }
    #endif
    //
#if 1
    MBOOL supportAdvEis = isAdvEisSupport_Hal1(); // from camera_custom_eis
    if(mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE || !supportAdvEis)
    {
        mEisScenario = EIS_SCE_EIS;
    }
    else
    {
        mEisScenario = EIS_SCE_EIS_PLUS;
    }

    /* EIS scenario control for Jade Minus */
    switch (CamManager::getInstance()->getEISScenarioType())
    {
     case CamManager::EIS_SCE_TYPE_NULL:
         mEisScenario = EIS_SCE_NONE;
         break;
     case CamManager::EIS_SCE_TYPE_EIS:
         mEisScenario = EIS_SCE_EIS;
     case CamManager::EIS_SCE_TYPE_DONT_CARE:
     default: /* fallthrough */
        break;
    }

#else
    mEisScenario = EIS_SCE_EIS;

    /* EIS scenario control for Jade Minus */
    switch (CamManager::getInstance()->getEISScenarioType())
    {
     case CamManager::EIS_SCE_TYPE_NULL:
         mEisScenario = EIS_SCE_NONE;
         break;
     case CamManager::EIS_SCE_TYPE_EIS:
         mEisScenario = EIS_SCE_EIS;
     case CamManager::EIS_SCE_TYPE_DONT_CARE:
     default: /* fallthrough */
        break;
    }

#endif
    //
    mRollbackBufCnt = 0;
    //
    { // video raw dump
        int camera_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        int tg_out_mode = 0; // NORNAL
        MY_LOGD("camera mode = %d", camera_mode);
        if (camera_mode == 1)
        {
            tg_out_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_PREVIEW_DUMP_RESOLUTION);
            MY_LOGD("tg_out_mode = %d", tg_out_mode);
        }

        if (camera_mode != 0)
        {

            MY_LOGD("create RawDumpThread instance");

            status_t status = NO_ERROR;

            NSCam::IHalSensorList *pSensorHalList = NULL;
            NSCam::SensorStaticInfo sensorStaticInfo;

            pSensorHalList = NSCam::IHalSensorList::get();
            MUINT32 sensorDev = pSensorHalList->querySensorDevIdx(getSensorIdx());
            pSensorHalList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            MUINT32 rawSensorBit = 0;
            switch (sensorStaticInfo.rawSensorBit)
            {
                case RAW_SENSOR_8BIT:
                    rawSensorBit = 8;
                    break;
                case RAW_SENSOR_10BIT:
                    rawSensorBit = 10;
                    break;
                case RAW_SENSOR_12BIT:
                    rawSensorBit = 12;
                    break;
                case RAW_SENSOR_14BIT:
                    rawSensorBit = 14;
                    break;
            }
            //
            mpRawDumpCmdQueThread = IRawDumpCmdQueThread::createInstance(sensorStaticInfo.sensorFormatOrder, rawSensorBit, mspParamsMgr);
            if  ( mpRawDumpCmdQueThread  == 0 || OK != (status = mpRawDumpCmdQueThread->run("DefaultCtrlNodeEngImpl::onInit")) )
            {
                MY_LOGE(
                    "Fail to run mpRawDumpCmdQueThread  - mpRawDumpCmdQueThread .get(%p), status[%s(%d)]",
                    mpRawDumpCmdQueThread.get(), ::strerror(-status), -status
                );
                g_u4RawDumpCnt = 0;
                // goto lbExit;
            }

            MY_LOGD("mpRawDumpCmdQueThread::setCallbacks is called");
            mpRawDumpCmdQueThread->setCallbacks(mspCamMsgCbInfo);
        }
    }
    char rRawCntValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.eng.video.dumpcnt", rRawCntValue, "0");
    mu4MaxRawDumpCnt = (atoi(rRawCntValue) == 0) ? MAX_VIDEO_DUMPCNT : atoi(rRawCntValue);
    MY_LOGD("mu4MaxRawDumpCnt:%d", mu4MaxRawDumpCnt);

    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onUninit()
{
    FUNC_START;
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    #if EIS_TEST
    if(mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(LOG_TAG);
        mpEisHal = NULL;
    }
    #endif //EIS_TEST
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpVHdrHal)
    {
        mpVHdrHal->Uninit();
        mpVHdrHal->DestroyInstance(LOG_TAG);
        mpVHdrHal = NULL;
    }
    #endif
    //
    if(mpHal3a)
    {
        mpHal3a->removeCallbacks(this);
        mpHal3a->destroyInstance(LOG_TAG);
        mpHal3a = NULL;
    }
    //
    if(mpCallbackZoom)
    {
        mpCallbackZoom->destroyUser();
        mpCallbackZoom->destroyInstance();
        mpCallbackZoom = NULL;
    }
    // start of Video Raw Dump
    int camera_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    if (camera_mode != 0)
    {
        sp<IRawDumpCmdQueThread> pRawDumpCmdQueThread = mpRawDumpCmdQueThread;
        if ( pRawDumpCmdQueThread  != 0 ) {
            MY_LOGD(
                "RawDumpCmdQ Thread: (tid, getStrongCount)=(%d, %d)",
                pRawDumpCmdQueThread->getTid(), pRawDumpCmdQueThread->getStrongCount()
            );
            pRawDumpCmdQueThread->requestExit();
        }
    }
    // end of Video Raw Dump
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void EngUtil(MINT32, IHal3A*, sp<IParamsManager>);

MBOOL
DefaultCtrlNodeEngImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    //FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    if(msg != PASS1_CONFIG_FRAME)
    {
        MY_LOGD("(0x%08X,0x%08X,0x%08X)",
                msg,
                ext1,
                ext2);
    }
    //
    switch(msg)
    {
        case PASS1_START_ISP:
        {
            if(mpHal3a)
            {
                ECmd_T cmd;
                E3APreviewMode_T pvMode;
                //
                mCurMagicNum = ext1;
                if( mMode == MODE_NORMAL_PREVIEW ||
                    mMode == MODE_ZSD_PREVIEW)
                {
                    pvMode = EPv_Normal;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Preview;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Preview;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Preview;
                    }
                }
                else
                if( mMode == MODE_VIDEO_PREVIEW ||
                    mMode == MODE_VIDEO_RECORD)
                {
                    pvMode = EPv_Video;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Video;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Video;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Video;
                    }
                }
                else
                {
                    MY_LOGE("un-support mMode(%d)",mMode);
                }
                mpHal3a->setSensorMode(mSensorScenario);
                //mpHal3a->set3APreviewMode(pvMode);
                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    VHDR_HAL_CONFIG_DATA vhdrConfigData;
                    vhdrConfigData.cameraVer = VHDR_CAMERA_VER_1;
                    mpVHdrHal->ConfigVHdr(vhdrConfigData);
                }
                #endif
                //
                mpHal3a->sendCommand(cmd);
                ParamIspProfile_T _3A_profile(
                        mIspProfile,
                        mCurMagicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3a->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            //
            if (CamManager::getInstance()->getEISScenarioType() != CamManager::EIS_SCE_TYPE_NULL)
            {
                #if EIS_TEST
                MBOOL bCreateEis = MFALSE;
                if( mPreviewMaxFps <= EIS_MAX_FPS &&
                    (   mMode == MODE_VIDEO_PREVIEW ||
                        mMode == MODE_VIDEO_RECORD ||
                        mMode == MODE_NORMAL_PREVIEW ||
                        mMode == MODE_ZSD_PREVIEW))
                {
                    bCreateEis = MTRUE;
                }
                //
                if(bCreateEis)
                {
                    mpEisHal = EisHal::CreateInstance(LOG_TAG,getSensorIdx());
                    if(mpEisHal)
                    {
                        mpEisHal->Init();
                        if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                        {
                            EIS_PASS_ENUM eisPass = EIS_PASS_1;
                            EIS_HAL_CONFIG_DATA  eisConfig;
                            MINT32 eisErr;
                            eisConfig.sensorType    = mSensorType;
                            eisConfig.configSce     = mEisScenario;
                            eisConfig.memAlignment  = 2;
                            eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                            if(eisErr != EIS_RETURN_NO_ERROR)
                            {
                                MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                            }
                        }
                    }
                    else
                    {
                        MY_LOGE("mpEisHal is NULL");
                    }
                }
                #endif //EIS_TEST
            }
            break;
        }
        case PASS1_STOP_ISP:
        {
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_SET_STATE,EIS_SW_STATE_UNINIT);
            }
            //
            if(mpHal3a)
            {
                mpHal3a->sendCommand(ECmd_CameraPreviewEnd);
            }
            else
            {
                MY_LOGW("mpHal3a is NULL");
            }
            //
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_CHECK_STATE,EIS_SW_STATE_UNINIT_CHECK);
            }
            break;
        }
        case PASS1_EOF:
        {
            {
                Mutex::Autolock _l(mLock);
                //
                if( mpHal3a &&
                    getFlag(muStateFlag, FLAG_DO_3A_UPDATE) && // to avoid send update after precaptrue-end
                    ext1 != MAGIC_NUM_INVALID )
                {
                    MUINT32 zoomRatio = 0, cropX = 0, cropY = 0, cropW = 0, cropH = 0;
                    if(mpIspSyncCtrl->getCurPass2ZoomInfo(zoomRatio,cropX,cropY,cropW,cropH))
                    {
                        MY_LOGD("setZoom:ZR(%d),Crop(%d,%d,%dx%d)",zoomRatio,cropX,cropY,cropW,cropH);
                        mpHal3a->setZoom(
                                    zoomRatio,
                                    cropX,
                                    cropY,
                                    cropW,
                                    cropH);
                    }
                    //do 3A update
                    mCurMagicNum = ext1;
                    ParamIspProfile_T _3A_profile(
                            mIspProfile,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3a->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
                }
                else
                {
                    MY_LOGD("skip update");
                }
            }
            //
            { // for engineer mode only // less than 1 ms in normal mode
                String8 const s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mspParamsMgr->getHalAppMode());
                if (s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG)
                {
                    EngUtil(getSensorIdx(), mpHal3a, mspParamsMgr);
                }
            }
            //
            if(updateReadyBuffer(ext2))
            {
                if (CamManager::getInstance()->getEISScenarioType() != CamManager::EIS_SCE_TYPE_NULL)
                {
                    mbUpdateEis = MTRUE;
                }
                mbUpdateVHdr = MTRUE;
            }
            else
            {
                mbUpdateVHdr = MFALSE;

                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    //set timestamp = 0 for VHDR driver.
                    mpVHdrHal->DoVHdr(0);
                }
                #endif

                mbUpdateEis = MFALSE;
                if(mpEisHal)
                {
                    //set timestamp = 0 for EIS driver.
                    mpEisHal->DoEis(EIS_PASS_1,NULL,0);
                }
            }
            break;
        }
        case PASS1_CONFIG_FRAME:
        {
            #if EIS_TEST
            if(mpEisHal)
            {
                if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                {
                    EIS_PASS_ENUM eisPass = EIS_PASS_1;
                    EIS_HAL_CONFIG_DATA  eisConfig;
                    MINT32 eisErr;
                    eisConfig.sensorType    = mSensorType;
                    eisConfig.configSce     = mEisScenario;
                    eisConfig.memAlignment  = 2;
                    eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                    if(eisErr != EIS_RETURN_NO_ERROR)
                    {
                        MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                    }
                }
            }
            #endif //EIS_TEST
            break;
        }
        default:
        {
            ret = MTRUE;
        }
    }
    //
    //FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNC_START;
    MBOOL         doPost  = MFALSE;
    MUINT32       dstdata = 0;
    MY_LOGV("(%d,0x%08X,0x%08X)", data, buf, ext);

    {
        //drop frame or not
        Mutex::Autolock _l(mLock);
        if( getFlag(muStateFlag, FLAG_BUF_PREVIEW_OK) )
            doPost = MTRUE;
    }

    if( !doPost )
    {
        // drop frame
        handleReturnBuffer(data, buf);
        return MTRUE;
    }

    // update fps
    static MINT64 i8TimeStamp_Old = 0xFFFFFFF;
    static MINT64 i8TimeStamp_New = 0;
    IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
    i8TimeStamp_New = pImageBuffer->getTimestamp();
    if (i8TimeStamp_New > i8TimeStamp_Old)
    {
        mspParamsMgr->updatePreviewFrameInterval((i8TimeStamp_New - i8TimeStamp_Old)/1000);
    }
    i8TimeStamp_Old = i8TimeStamp_New;

    //post buffer to Pass2
    switch(data)
    {
        case CONTROL_FULLRAW:
        {
            dstdata = CONTROL_CAP_SRC;
            break;
        }
        case CONTROL_RESIZEDRAW:
        {
            dstdata = CONTROL_PRV_SRC;
            { // start of Video Raw Dump

                static bool mEnableRawDump = false;
                //(4) enable RawDump
                {
                    String8 const s = mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_DUMP_FLAG);
#undef TRUE
#undef FALSE
                    bool RawDumpFlag = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? 1 : 0;

                    if ( mEnableRawDump != RawDumpFlag )
                    {
                        MY_LOGD("RawDump flag changed to %d ", RawDumpFlag);
                        mEnableRawDump = RawDumpFlag;
                         g_u4RawDumpCnt = 0;
                    }
                }


                /*
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE)==1)
                {
                    RAWDUMPCNT = 60;
                }
                else if((mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))==2)
                {
                    RAWDUMPCNT = 120;
                }
                */
                if (mpRawDumpCmdQueThread != 0 && (g_u4RawDumpCnt < mu4MaxRawDumpCnt) && mEnableRawDump == true )
                {
                    MY_LOGD("u4RawDumpCnt, Max = (%d, %d)", g_u4RawDumpCnt, mu4MaxRawDumpCnt); // debug

                    IImageBuffer* pImgbuffer = (IImageBuffer*)buf;
                    mpRawDumpCmdQueThread->postCommand(pImgbuffer);
                    g_u4RawDumpCnt++;
                }
                else if (g_u4RawDumpCnt == mu4MaxRawDumpCnt)
                {
                    MY_LOGD("Stop");
                    mspParamsMgr->set(MtkCameraParameters::KEY_RAW_DUMP_FLAG, CameraParameters::FALSE);
                    mpRawDumpCmdQueThread->postCommand(NULL);
                    mEnableRawDump=false;
                    g_u4RawDumpCnt=0;
                }
            } // end of Video Raw Dump

            break;
        }
        default:
        {
            MY_LOGE("not support yet: %d", data);
            break;
        }
    }
    //
    #if EIS_TEST
    if(mbUpdateEis)
    {
        if(mpEisHal)
        {
            IspSyncControl::EIS_CROP_INFO eisCrop;
            eisCrop.enable = MFALSE;
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
            EIS_HAL_CONFIG_DATA eisHalCfgData;
            EIS_STATISTIC_STRUCT* pEisStat = new EIS_STATISTIC_STRUCT;

            eisHalCfgData.p1ImgW = (MUINT32)pImageBuffer->getImgSize().w;
            eisHalCfgData.p1ImgH = (MUINT32)pImageBuffer->getImgSize().h;

            mpEisHal->DoEis(
                        EIS_PASS_1,
                        &eisHalCfgData,
                        pImageBuffer->getTimestamp());
            //
            if( mpEisHal->GetEisSupportInfo(getSensorIdx()) &&
                (mMode == MODE_VIDEO_PREVIEW || mMode == MODE_VIDEO_RECORD))
            {
                if( mspParamsMgr->getVideoStabilization() &&
                    mEisScenario == EIS_SCE_EIS)
                {
                    MUINT32 tarWidth = 0, tarHeight = 0;
                    mpEisHal->GetEisResult(
                                eisCrop.xInt,
                                eisCrop.xFlt,
                                eisCrop.yInt,
                                eisCrop.yFlt,
                                tarWidth,
                                tarHeight);
                    //
                    eisCrop.enable = MTRUE;
                    eisCrop.scaleFactor = 100.0/EIS_FACTOR;
                }
            }
            // get EI statistic informaiton
            mpEisHal->GetEisStatistic(pEisStat);

            mpIspSyncCtrl->setEisResult(
                            pImageBuffer,
                            eisCrop,
                            *pEisStat);
        }
        //
        mbUpdateEis = MFALSE;
    }
    #endif
    //
    if(mbUpdateVHdr)
    {
        #ifdef MTK_CAM_VHDR_SUPPORT
        if(mpVHdrHal)
        {
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
            mpVHdrHal->DoVHdr(pImageBuffer->getTimestamp());
        }
        #endif
        //
        mbUpdateVHdr = MFALSE;
    }
    //
    handlePostBuffer(dstdata, buf);
    //
    if( data == CONTROL_RESIZEDRAW )
    {
    //  wait here to let pass1node possible to receive CONTROL_STOP_PASS1 notify
    //  before deque next buffer
        MBOOL wait = MFALSE;
        {
            Mutex::Autolock _l(mLock);
            wait = getFlag(muStateFlag, FLAG_PRECAPTURE);
        }

        if( wait )
        {
#define TIMEOUT_WAIT_READYTOCAP     (10000) //us
            CAM_TRACE_NAME("waitReadyToCap");
            MY_LOGD("wait for %d", TIMEOUT_WAIT_READYTOCAP);
            usleep(TIMEOUT_WAIT_READYTOCAP);
#undef TIMEOUT_WAIT_READYTOCAP
        }
    }
    //FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
}

