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
#define LOG_TAG "MtkCam/P2Node_F"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <cutils/properties.h>  // For property_get().
using namespace NSCam;
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/drv/res_mgr_drv.h>
//
#include <mtkcam/hwutils/CamManager.h>
#include <camera_custom_eis.h>
using namespace NSCam::Utils;
//
#include "./inc/pass2nodeImpl.h"
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#endif

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")
#define FUNC_NAME  MY_LOGD("")

#define NOTIFY_FP_RECORD (1)

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
FeaturePass2::
FeaturePass2(Pass2NodeType const type)
    : PrvPass2(type)
    , mpFeaturePipe(NULL)
    , mspCamMsgCbInfo(NULL)
    , mFeatureMask(0)
    , mSensorType(SENSOR_TYPE_UNKNOWN)
    , mbFeature(MFALSE)
    , mbGDPreview(MFALSE)
    , mbSDPreview(MFALSE)
    , mbRecordingHint(MFALSE)
    , mbIsRecording(MFALSE)
    , mbInit(MFALSE)
{
    mpFeaturePipe = NULL;

    mFeaturePipeTuning.SmoothLevel      = 0;
    mFeaturePipeTuning.SkinColor        = 0;
    mFeaturePipeTuning.EnlargeEyeLevel  = 0;
    mFeaturePipeTuning.SlimFaceLevel    = 0;
    mFeaturePipeTuning.ExtremeBeauty    = MFALSE;
    mFeaturePipeTuning.Rotation         = 0;

    mTouch.x = 0;
    mTouch.y = 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
moduleStart()
{
    FUNC_START;

    mbInit = MFALSE;
    // trigger first loop for init
    triggerLoop();

    FUNC_END;
lbExit:
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
doInitialization()
{
    MBOOL doInitial = MFALSE;

    if( !mbInit )
    {
        MY_LOGD("doInitialization");
        mbInit = MTRUE;
        configFeature();
        doInitial = MTRUE;
    }

lbExit:
    return doInitial;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
enableFeature(MBOOL en)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("en(%d)",en);
    mbFeature = en;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
enableGDPreview(MBOOL en)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("en(%d),FeatureMask0x%08X)",en,mFeatureMask);
    mbGDPreview = en;
    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
    {
        MY_LOGD("GD should be handled by FDClient");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
enableSDPreview(MBOOL en)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("SD en(%d),FeatureMask0x%08X)",en,mFeatureMask);
    mbSDPreview = en;
    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
    {
        MY_LOGD("SD should be handled by FDClient");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
startRecording(
    MUINT32 width,
    MUINT32 height,
    MUINT32 fps,
    MBOOL   isYuv,
    sp<IParamsManager>  spParamsMgr)
{
    FUNC_START;
    MBOOL ret = PrvPass2::startRecording(width, height, fps, isYuv, spParamsMgr);
    Mutex::Autolock _l(mLock);
    mbIsRecording = MTRUE;
#if NOTIFY_FP_RECORD
    if(mpFeaturePipe)
    {
        if( FEATURE_MASK_IS_EIS_ENABLED(mFeatureMask)){
            mpFeaturePipe->startEISRecord();
            spParamsMgr->set(MtkCameraParameters::KEY_EIS25_MODE,1);
        }else{
            spParamsMgr->set(MtkCameraParameters::KEY_EIS25_MODE,0);
        }
    }
    else
    {
        MY_LOGE("mpFeaturePipe is NULL");
    }
#endif
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
stopRecording()
{
    FUNC_START;
    MBOOL ret = PrvPass2::stopRecording();
    Mutex::Autolock _l(mLock);
    mbIsRecording = MFALSE;
    //
    //
#if NOTIFY_FP_RECORD
    if(mpFeaturePipe)
    {
        if( FEATURE_MASK_IS_EIS_ENABLED(mFeatureMask))
            mpFeaturePipe->stopEISRecord();
    }
    else
    {
        MY_LOGE("mpFeaturePipe is NULL");
    }
#endif
    //
    FUNC_END;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
setParameters(
    sp<IParamsManager>  spParamsMgr,
    MUINT32             openId)
{
    Mutex::Autolock _l(mLock);
    ResMgrDrv* pResMgrDrv = NULL;
    //
    //
    if(spParamsMgr->getDisplayRotationSupported())
    {
        mbIsNeedDisplayRotation = MTRUE;
    }
    else
    {
        mbIsNeedDisplayRotation = MFALSE;
    }
    //
    if(!mbFeature)
    {
        FEATURE_MASK_CLEAR(mFeatureMask);
        MY_LOGD("All features are disabled FeatureMask(0x%08X)",mFeatureMask);
        return;
    }
    //
    if(mSensorType == SENSOR_TYPE_UNKNOWN)
    {
        SensorStaticInfo sensorInfo;
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(openId),
                &sensorInfo);
        mSensorType = sensorInfo.sensorType;
        MY_LOGD("mSensorType(%d)",mSensorType);
    }
    //
    mFeaturePipeTuning.SmoothLevel      = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL);
    mFeaturePipeTuning.SkinColor        = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SKIN_COLOR);
    mFeaturePipeTuning.EnlargeEyeLevel  = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_ENLARGE_EYE);
    mFeaturePipeTuning.SlimFaceLevel    = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SLIM_FACE);
    mFeaturePipeTuning.Rotation         = spParamsMgr->getInt(MtkCameraParameters::KEY_ROTATION);
    //
    if(::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FB_EXTREME_BEAUTY), MtkCameraParameters::TRUE) == 0)
    {
        mFeaturePipeTuning.ExtremeBeauty = MTRUE;
    }
    else
    {
        mFeaturePipeTuning.ExtremeBeauty = MFALSE;
    }
    //
    spParamsMgr->getFaceBeautyTouchPosition(mTouch);
    //
    FEATURE_MASK_CLEAR(mFeatureMask);
    //
    MINT32 vdoWidth, vdoHeight;
    spParamsMgr->getVideoSize(
                    &vdoWidth,
                    &vdoHeight);
    if(!mbPreview)
    {
        mVdoSizeBeforePreview.w = vdoWidth;
        mVdoSizeBeforePreview.h = vdoHeight;
    }
    MY_LOGD("bPreivew(%d),VdoSize before preview:%dx%d,Cur VdoSize:%dx%d",
            mbPreview,
            mVdoSizeBeforePreview.w,
            mVdoSizeBeforePreview.h,
            vdoWidth,
            vdoHeight);
    //
    if( ::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_3DNR_MODE), "on") == 0 &&
        mSensorType == SENSOR_TYPE_RAW)
    {
        if( (   spParamsMgr->getRecordingHint()  &&
                vdoWidth*vdoHeight <= IMG_1080P_SIZE     ) ||
            !(spParamsMgr->getRecordingHint()))
        {
            #if 1   //Vent@20140228: For 3DNR enable.
            char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("vendor.camera.3dnr.enable", EnableOption, "1");
            if (EnableOption[0] == '1')
            {
                #ifdef  MTK_CAM_NR3D_SUPPORT
                FEATURE_MASK_ENABLE_3DNR(mFeatureMask);
                #endif  // MTK_CAM_NR3D_SUPPORT
            }
            #else   // Original.
                #ifdef  MTK_CAM_NR3D_SUPPORT
                FEATURE_MASK_ENABLE_3DNR(mFeatureMask);
                #endif  // MTK_CAM_NR3D_SUPPORT
            #endif  //Vent@20140228: End.
        }
    }

    MBOOL isVFB = (::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0) ? MTRUE : MFALSE;
    MBOOL isEISQ = (::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_ADV_EIS), MtkCameraParameters::TRUE) == 0) ? MTRUE : MFALSE;
    int camMode = spParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    MBOOL isTkApp = (camMode >= 0) && (camMode != MtkCameraParameters::CAMERA_MODE_NORMAL);
    MBOOL isvHDRDisabled = (spParamsMgr->getVHdr() == SENSOR_VHDR_MODE_NONE);

    if( spParamsMgr->getVideoStabilization() &&
        spParamsMgr->getRecordingHint() &&
        (mVdoSizeBeforePreview.w*mVdoSizeBeforePreview.h) <= IMG_1080P_SIZE &&
        !isVFB && isvHDRDisabled)
    {
        if (CamManager::getInstance()->getEISScenarioType() == CamManager::EIS_SCE_TYPE_DONT_CARE
            && isAdvEisSupport_Hal1()/*from camera_custom_eis*/)
        {
            FEATURE_MASK_ENABLE_EIS(mFeatureMask);
            MBOOL forceEisQ = (property_get_int32("vendor.debug.camera.eisq", 0) > 0);
            if(isTkApp || isEISQ || forceEisQ)
            {
                FEATURE_MASK_ENABLE_EIS_Q(mFeatureMask);
            }
        }
    }

    if( spParamsMgr->getVideoStabilization() &&
        (!spParamsMgr->getRecordingHint()) &&
        !isVFB && isvHDRDisabled)
    {
        if (CamManager::getInstance()->getEISScenarioType() == CamManager::EIS_SCE_TYPE_DONT_CARE
            && isAdvEisSupport_Hal1()/*from camera_custom_eis*/)
        {
            FEATURE_MASK_ENABLE_EIS_CALIBRATION(mFeatureMask);
        }
    }

    if(mbIsRecording && FEATURE_MASK_IS_EIS_ENABLED(mFeatureMask)
        && spParamsMgr->getInt(MtkCameraParameters::KEY_EIS25_MODE) == 0)
    {
        MY_LOGD("Notify stop EIS Record!");
        if(mpFeaturePipe)
            mpFeaturePipe->notifyEISRecordStopping();
    }

    //
    mbRecordingHint = spParamsMgr->getRecordingHint();
    //
    if(::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0)
    {
        FEATURE_MASK_ENABLE_VFB(mFeatureMask);
        //
        if(mbGDPreview)
        {
            FEATURE_MASK_ENABLE_GESTURE_SHOT(mFeatureMask);
        }

        if(mbSDPreview)
        {
            FEATURE_MASK_ENABLE_SMILE_SHOT(mFeatureMask);
        }
    }
    //
    if( !(CamManager::getInstance()->isMultiDevice()) &&
        spParamsMgr->getVHdr() == SENSOR_VHDR_MODE_IVHDR)
    {
        FEATURE_MASK_ENABLE_VHDR(mFeatureMask);
    }
    MY_LOGD("FeatureMask(0x%08X), bRecordingHint(%d)",mFeatureMask, mbRecordingHint);
    //
    pResMgrDrv = ResMgrDrv::createInstance(getSensorIdx());
    if(pResMgrDrv)
    {
        if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
        {
            pResMgrDrv->setItem(
                            ResMgrDrv::ITEM_VFB,
                            MTRUE);
        }
        else
        {
            pResMgrDrv->setItem(
                            ResMgrDrv::ITEM_VFB,
                            MFALSE);
        }
        //
        // Currently EIS no need to use high cpu feq.
        pResMgrDrv->setItem(
                        ResMgrDrv::ITEM_EIS20,
                        MFALSE);
        //
        pResMgrDrv->destroyInstance();
        pResMgrDrv = NULL;
    }
    else
    {
        MY_LOGW("pResMgrDrv is NULL");
    }
}


/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
setCamMsgCb(sp<CamMsgCbInfo> spCamMsgCbInfo)
{
    Mutex::Autolock _l(mLock);
    //
    FUNC_NAME;
    mspCamMsgCbInfo = spCamMsgCbInfo;
}


/*******************************************************************************
*
********************************************************************************/
IHalPostProcPipe*
FeaturePass2::
createPipe()
{
    if( mPass2Type == PASS2_FEATURE )
    {
        mpFeaturePipe = IFeaturePipe::createInstance(getName(), getSensorIdx());
        return mpFeaturePipe;
    }

    MY_LOGE("not support type(0x%x)", mPass2Type);
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
configFeature()
{
    Mutex::Autolock _l(mLock);
    //
    if(mspCamMsgCbInfo != NULL)
    {
        mpFeaturePipe->setCallbacks(mspCamMsgCbInfo);
    }
    //
    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
    {
        if( mbGDPreview &&
            !FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(mFeatureMask))
        {
            FEATURE_MASK_ENABLE_GESTURE_SHOT(mFeatureMask);
            MY_LOGD("FeatureMask(0x%08X)",mFeatureMask);
        }

        if( mbSDPreview &&
            !FEATURE_MASK_IS_SMILE_SHOT_ENABLED(mFeatureMask))
        {
            FEATURE_MASK_ENABLE_SMILE_SHOT(mFeatureMask);
            MY_LOGD("FeatureMask(0x%08X)",mFeatureMask);
        }
    }
    //
    if(!mpFeaturePipe->setTuning(mFeaturePipeTuning))
    {
        MY_LOGE("FeaturePipe->setTuning fail");
    }
    //
    if(!mpFeaturePipe->setTouch(mTouch.x, mTouch.y))
    {
        MY_LOGE("FeaturePipe->setTouch fail");
    }
    // Must be before setFeature
    if(!mpFeaturePipe->set(RECORDING_HINT, (MINTPTR) mbRecordingHint))
    {
        MY_LOGE("FeaturePipe->set RECORDING_HINT fail");
    }
    //
    if(!mpFeaturePipe->setFeature(mFeatureMask))
    {
        MY_LOGE("FeaturePipe->setFeature fail");
    }
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
