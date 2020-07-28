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

#define LOG_TAG "MtkCam/Shot"
// get_boot_mode
#include <sys/stat.h>
#include <fcntl.h>
#include <cutils/properties.h>

//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/common.h>
//
#include <mtkcam/drv/IHalSensor.h>

//
#include <mtkcam/middleware/v1/camshot/ICamShot.h>
#include <mtkcam/middleware/v1/camshot/ISingleShot.h>
#include <mtkcam/middleware/v1/camshot/ISmartShot.h>
//
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "EngParam.h"
#include "EngShot.h"
//
//
#define FEATURE_MODIFY (1)
//
#if FEATURE_MODIFY
#include <mtkcam/feature/utils/FeatureProfileHelper.h>
#endif // FEATURE_MODIFY
//
//
#include <mtkcam/aaa/IHal3A.h>
//
#include <custom/aaa/AEPlinetable.h>
//
#include <MTKDngOp.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;
//
using namespace NS3Av3;
//
using namespace android;
using namespace NSShot;

#include <isp_tuning/isp_tuning.h>  // EIspProfile_*
using namespace NSIspTuning;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
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
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_EngShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    CAM_LOGD("createInstance_EngShot");
    sp<IShot>       pShot = NULL;
    sp<EngShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new EngShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new EngShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
EngShot::
onCreate()
{

    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
EngShot::
onDestroy()
{

}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
EngShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
~EngShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    case eCmd_setEngParam:
        ret = setEngParam(reinterpret_cast<void const*>(arg1), arg2);
        break;
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_reset()
{

    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
 extern "C"
{
int getBootMode(void)
{
  int fd;
  size_t s;
  char boot_mode[4] = {'0'};

  fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDWR);
  if (fd < 0)
  {
    CAM_LOGW("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
    return 0;
  }

  s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
  close(fd);

  if(s <= 0)
  {
        CAM_LOGW("could not read boot mode sys file\n");
       return 0;
  }

  boot_mode[s] = '\0';
  return atoi(boot_mode);
}
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_capture()
{
    MBOOL ret = MTRUE;
    MBOOL isMfll = MFALSE;

    NSCamShot::ICamShot *pSingleShot; // NSCamShot::ISingleShot *pSingleShot;
    CAM_LOGD("mpEngParam.mi4CamShotType : %d", mpEngParam.mi4CamShotType);
    //
    pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "EngShot");
    CAM_LOGD("ISingleShot::createInstance");
    //
    pSingleShot->init();
    //
    pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
    //
    MINT32 isEnablePostViewFromPrvCB = 0;
    if (0 == mShotParam.miPostviewDisplayFormat)
    {
        mShotParam.miPostviewDisplayFormat = mShotParam.miPostviewClientFormat;
        isEnablePostViewFromPrvCB = 1;
        CAM_LOGD("mShotParam.miPostviewDisplayFormat to %d from %d", mShotParam.miPostviewClientFormat, mShotParam.miPostviewDisplayFormat);
    }

    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

    MINT32 i4msg = 0;
    if (mpEngParam.mi4EngRawSaveEn)
    {
        i4msg |= NSCamShot::ECamShot_DATA_MSG_RAW;
    }
#if 0

    if (1 == isEnablePostViewFromPrvCB)
    {
        i4msg |= NSCamShot::ECamShot_DATA_MSG_POSTVIEW;
    }
#endif

    pSingleShot->enableDataMsg(
            NSCamShot::ECamShot_DATA_MSG_JPEG | i4msg
#if 0
            | ((ePostViewFmt != eImgFmt_UNKNOWN) ?
             NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
#endif
            );
    //
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
        if(pIHalSensorList)
        {
            MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());

            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            mpEngParam.u4RawPixelID = sensorStaticInfo.sensorFormatOrder; /* SENSOR_OUTPUT_FORMAT_RAW_B=0, SENSOR_OUTPUT_FORMAT_RAW_Gb, SENSOR_OUTPUT_FORMAT_RAW_Gr, SENSOR_OUTPUT_FORMAT_RAW_R */

#define scenario_case(scenario, KEY, mpEngParam)       \
            case scenario:                            \
                (mpEngParam).u4SensorWidth = sensorStaticInfo.KEY##Width;  \
                (mpEngParam).u4SensorHeight = sensorStaticInfo.KEY##Height; \
                break;

            switch (mpEngParam.mi4EngSensorMode)
            {
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_PREVIEW, preview, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE, capture, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO, video, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1, video1, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2, video2, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM1, SensorCustom1, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM2, SensorCustom2, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM3, SensorCustom3, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM4, SensorCustom4, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM5, SensorCustom5, mpEngParam );
            default:
                MY_LOGE("not support sensor scenario(0x%x)", mpEngParam.mi4EngSensorMode);
                ret = MFALSE;
                    break;
            }
        }
#undef  scenario_case

    MINT32 iOperMode;
    IHal3A* pHal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
    pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetOperMode,  reinterpret_cast<MINTPTR>(&iOperMode), 0);
    pHal3A->destroyInstance(LOG_TAG);
    pHal3A = NULL;

    // Align Jpeg Size to RAW Size
    //Only when raw save and in normal mode
    if((mpEngParam.mi4EngRawSaveEn == 1 && iOperMode != 2 /*EOperMode_Meta*/) &&
       (mpEngParam.mi4EngSensorMode == EngParam::ENG_SENSOR_MODE_NORMAL_PREVIEW))
    {
        mShotParam.mi4PictureWidth = ( mpEngParam.u4SensorWidth ) & (~0xF);
        mShotParam.mi4PictureHeight = ( mpEngParam.u4SensorHeight ) & (~0xF);
    }
    else if ((mpEngParam.mi4EngRawSaveEn == 1 && iOperMode != 2 /*EOperMode_Meta*/) &&
         (mpEngParam.mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE)&&
            (mpEngParam.mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO)&&
            (mpEngParam.mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1)&&
            (mpEngParam.mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2))
    {
        // 1. keep to the same ratio
        // 2. size is closed to sensor raw.

        // srcW/srcH < dstW/dstH
        if (mpEngParam.u4SensorWidth * mShotParam.mi4PostviewHeight <= mShotParam.mi4PostviewWidth * mpEngParam.u4SensorHeight) {
            mShotParam.mi4PictureWidth = ( mpEngParam.u4SensorWidth ) & (~0xF);
            mShotParam.mi4PictureHeight = ( mpEngParam.u4SensorWidth * mShotParam.mi4PostviewHeight / mShotParam.mi4PostviewWidth ) & (~0xF);
        }
        //srcW/srcH > dstW/dstH
        else
        {
            mShotParam.mi4PictureWidth = ( mpEngParam.u4SensorHeight* mShotParam.mi4PostviewWidth / mShotParam.mi4PostviewHeight ) & (~0xF);
            mShotParam.mi4PictureHeight = ( mpEngParam.u4SensorHeight ) & (~0xF);
        }

    }
     // manual exp time / sensor gain control
    if (mShotParam.muSensorSpeed != 0 && mShotParam.muSensorGain != 0)
    {
        ret = setManualAEControl(&mShotParam.mAppSetting, mShotParam.muSensorSpeed*1000, mShotParam.muSensorGain);
    }
    //EV breacket
    if (1 == mpEngParam.mi4EngRawEVBEn)
    {
        strEvSetting evSetting;
        IHal3A* pHal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
        pHal3A->send3ACtrl(E3ACtrl_GetEvSetting, (MINT8)((mpEngParam.u4EVvalue)/10), (MINTPTR)&evSetting); //D0Nenable, 0Ndisable
        MY_LOGD("send3ACtrl ev-value: %d ", ((mpEngParam.u4EVvalue)/10));
        ret = setManualAEControl(&mShotParam.mAppSetting, evSetting.u4Eposuretime, evSetting.u4AfeGain, evSetting.u4IspGain);
        pHal3A->destroyInstance(LOG_TAG);
    }
    //
    //update ISP PROFILE SETTING
    EIspProfile_T profile;
    {
         switch(mpEngParam.u4ISPProfile)
         {
             case 0:
                profile = EIspProfile_Preview;
                break;
             case 1:
                profile = EIspProfile_Capture;
                break;
             case 2:
                profile = EIspProfile_Video;
                break;
             default:
                profile = EIspProfile_Capture;
                break;
         }
         updateEntry<MUINT8>(&mShotParam.mHalSetting , MTK_3A_ISP_PROFILE , profile);
         MY_LOGD("mShotParam.mHalSetting(MTK_3A_ISP_PROFILE) = %d", mpEngParam.u4ISPProfile);
    }
    //
    //vhdr update ispprofile halmeta
    if (mpEngParam.u4VHDState != SENSOR_VHDR_MODE_NONE)
    {
        // Prepare query Feature EngShot ISP Profile
        ProfileParam profileParam(
            MSize(mpEngParam.u4SensorWidth, mpEngParam.u4SensorHeight),
            mpEngParam.u4VHDState,
            (1 == mpEngParam.mi4EngRawSaveEn)?mpEngParam.mi4EngSensorMode:SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
            ProfileParam::FLAG_NONE,
            ((mpEngParam.bIsAutoHDR) ? (ProfileParam::FMASK_AUTO_HDR_ON) : (ProfileParam::FMASK_NONE)),
            profile
        );

        MUINT8 featureProf = 0;
        if (FeatureProfileHelper::getEngShotProf(featureProf, profileParam))
        {
            updateEntry<MUINT8>(&mShotParam.mHalSetting , MTK_3A_ISP_PROFILE , featureProf);
        }
    }
    //
    // sensor param
    MUINT32 u4Scenario;
    if (1 == mpEngParam.mi4EngRawSaveEn)
    {
        u4Scenario = mpEngParam.mi4EngSensorMode;
    }
    else
    {
        u4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }

    NSCamShot::SensorParam rSensorParam(
            getOpenId(),                            //sensor idx
            u4Scenario,                             //Scenaio
            getShotRawBitDepth(),                   //bit depth
            MFALSE,                                 //bypass delay
            MFALSE,                                  //bypass scenario
            (mpEngParam.mi4EngIspMode == EngParam::ENG_RAW_TYPE_PURE_RAW ? NSCamShot::ECamShot_RAWTYPE_PURE : NSCamShot::ECamShot_RAWTYPE_PREPROC),// u4RawType
            mpEngParam.u4VHDState
            );

    //
    //DualPD update MTK_HAL_REQUEST_PASS1_DISABLE halmeta
    {
        HwInfoHelper helper(getOpenId());
        if( ! helper.updateInfos() ) {
            MY_LOGE("cannot properly update infos");
            return BAD_VALUE;
        }
        //
        MINT32 dualPDDisable = 0; //default = 0
        dualPDDisable = ((mpEngParam.mi4EngIspMode == EngParam::ENG_RAW_TYPE_PURE_RAW) && (helper.getDualPDAFSupported(u4Scenario))) ? 1 : 0;
        updateEntry<MINT32>(&mShotParam.mHalSetting , MTK_HAL_REQUEST_PASS1_DISABLE , dualPDDisable);
        MY_LOGD("set MTK_HAL_REQUEST_PASS1_DISABLE (%d)", dualPDDisable);
    }
    //
    //query NR-type if enable
    MINT32 nrtype = MTK_NR_MODE_AUTO;
    if (1 == mpEngParam.mi4ManualMultiNREn)
    {
        switch(mpEngParam.mi4ManualMultiNRType)// Overwrite auto Multi-NR values if mi4ManualMultiNREn is 1
        {
            case 0:
                nrtype = MTK_NR_MODE_OFF;
                break;
            case 1:
                nrtype = MTK_NR_MODE_MNR;
                break;
            case 2:
                nrtype = MTK_NR_MODE_SWNR;
                break;
            default:
                nrtype = MTK_NR_MODE_AUTO;
                break;
        }
        MY_LOGD("mi4ManualMultiNREn, type = %d", mpEngParam.mi4ManualMultiNRType);
    }
    //
    NRParam nrParam;
    ret = pSingleShot->getRawSize(rSensorParam, nrParam.mFullRawSize);
    // update post nr settings
    nrParam.mi4NRmode   = nrtype;
    nrParam.mpHalMeta   = &mShotParam.mHalSetting;
    nrParam.mPluginUser = mPluginUser;
    updatePostNRSetting(nrParam);
    //
    // shot param
    MINT32 lcsOpen = mShotParam.mbEnableLtm;
    MBOOL openLCS = (lcsOpen > 0) || (mpEngParam.u4VHDState == SENSOR_VHDR_MODE_ZVHDR) || (mpEngParam.u4VHDState == SENSOR_VHDR_MODE_IVHDR);
    NSCamShot::ShotParam rShotParam(
            eImgFmt_YUY2,                    //yuv format
            mShotParam.mi4PictureWidth,      //picutre width
            mShotParam.mi4PictureHeight,     //picture height
            mShotParam.mu4Transform,         //picture transform
            ePostViewFmt,                    //postview format
            mShotParam.mi4PostviewWidth,     //postview width
            mShotParam.mi4PostviewHeight,    //postview height
            0,                               //postview transform
            mShotParam.mu4ZoomRatio,         //zoom
            mShotParam.mAppSetting,
            mShotParam.mHalSetting,
            openLCS,                         // P1 open LCS to enqueue to LCE
            (mShotParam.mbEnableRrzoCapture)
            );
    MY_LOGD("rShotParam: picWxH (%d,%d), postWxH (%d, %d), fmt(%d), trans(%d)",
        mShotParam.mi4PictureWidth,mShotParam.mi4PictureHeight,mShotParam.mi4PostviewWidth,mShotParam.mi4PostviewHeight, ePostViewFmt, mShotParam.mu4Transform);

    // jpeg param
    NSCamShot::JpegParam rJpegParam(
            NSCamShot::ThumbnailParam(
                mJpegParam.mi4JpegThumbWidth,
                mJpegParam.mi4JpegThumbHeight,
                mJpegParam.mu4JpegThumbQuality,
                MTRUE),
            mJpegParam.mu4JpegQuality,         //Quality
            MFALSE                             //isSOI
            );

    //
    mpEngParam.u4Bitdepth = rSensorParam.u4Bitdepth;
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = pSingleShot->setShotParam(rShotParam);
    //
    ret = pSingleShot->setJpegParam(rJpegParam);
    //
    ret = pSingleShot->startOne(rSensorParam);
    //
    ret = pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();


    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EngShot::
onCmd_cancel()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EngShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    EngShot *pEngShot = reinterpret_cast <EngShot *>(user);
    if (NULL != pEngShot)
    {
        if ( NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pEngShot->mpShotCallback->onCB_Shutter(true,
                                                      0,
                                                     pEngShot->getShotMode()
                                                     );
        }
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EngShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    CAM_LOGD("fgCamShotDataCb");

    EngShot *pEngShot = reinterpret_cast<EngShot *>(user);
    if (NULL != pEngShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            CAM_LOGD("NSCamShot::ECamShot_DATA_MSG_POSTVIEW");
#if 0
            MUINT32 u4Size = 0;
            for (size_t i = 0; i < msg.pBuffer->getPlaneCount(); i++)
            {
                u4Size += msg.pBuffer->getBufSizeInBytes(i);
            }

            MUINT8* puBuf = (MUINT8 *)malloc(u4Size);
            if (puBuf != NULL)
            {
                MUINT8* puTmp = puBuf;
                msg.pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);//
                memcpy(puTmp, (MUINT8 *)msg.pBuffer->getBufVA(0), msg.pBuffer->getBufSizeInBytes(0));
                for (size_t i = 1; i < msg.pBuffer->getPlaneCount(); i++)
                {
                    puTmp = puTmp + msg.pBuffer->getBufSizeInBytes(i - 1);
                    memcpy(puTmp, (MUINT8 *)msg.pBuffer->getBufVA(i), msg.pBuffer->getBufSizeInBytes(i));
                }
                msg.pBuffer->unlockBuf(LOG_TAG);//
                pEngShot->handlePostViewData(puBuf, u4Size);
                free(puBuf);
            }
#endif
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pEngShot->handleJpegData(msg.pBuffer);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_RAW == msg.msgType)
        {
            msg.pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            //
            pEngShot->mpEngParam.u4rawStride=msg.pBuffer->getBufStridesInBytes(0);
            pEngShot->dumpEngRaw(msg.pBuffer->getBufSizeInBytes(0), (MUINT8*)msg.pBuffer->getBufVA(0), msg.pBuffer->getImgFormat());
            //
            msg.pBuffer->unlockBuf(LOG_TAG);

        }
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
bool
EngShot::
setEngParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to EngParam");
        return  false;
    }
    //
    if  ( size != sizeof(EngParam) )
    {
        MY_LOGE("size[%zu] != sizeof(EngParam)[%zu]; please fully build source codes", size, sizeof(EngParam));
        return  false;
    }
    //
    mpEngParam = *(const_cast<EngParam *>(reinterpret_cast<EngParam const*>(pParam)));

    return  true;
}

/******************************************************************************
*
*******************************************************************************/
bool
EngShot::
dumpEngRaw(
    uint32_t const  u4RawImgSize,
    uint8_t const*  puRawImgBuf,
    MINT const iBufferInFormat 
)
{
    MY_LOGD("size/buf=%d/%p", u4RawImgSize, puRawImgBuf);

    String8 s8RawFilePath(mpEngParam.ms8RawFilePath);
    String8 ms8RawFileExt(s8RawFilePath.getPathExtension()); // => .raw
    s8RawFilePath = s8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0
    if  ( (u4RawImgSize > 0) && (puRawImgBuf != 0) && (s8RawFilePath != ""))
    {

        char mpszSuffix[256] = {0};
        //
        //fixed data bitdepth
        int fixedBitDstDepth = 10; //for debug tool [CY]
        //
        if (mpEngParam.mi4CamShotType == EngParam::ENG_CAMSHOT_TYPE_MFLL)
        {
            mpEngParam.mi4MFLLnum++;
            sprintf(mpszSuffix, "__%dx%d_%d_%d-%02d", mpEngParam.u4SensorWidth, mpEngParam.u4SensorHeight, fixedBitDstDepth, mpEngParam.u4RawPixelID, mpEngParam.mi4MFLLnum);
        }
        else
        {
            sprintf(mpszSuffix, "__%dx%d_%d_%d", mpEngParam.u4SensorWidth, mpEngParam.u4SensorHeight, fixedBitDstDepth, mpEngParam.u4RawPixelID); /* info from EngShot::onCmd_capture */
        }

        s8RawFilePath.append(mpszSuffix);
        s8RawFilePath.append(ms8RawFileExt);

        MY_LOGD("Raw saved path: %s", s8RawFilePath.string());
    if(iBufferInFormat == eImgFmt_BAYER10_UNPAK)
    	{
	    bool ret = NSCam::Utils::saveBufToFile(s8RawFilePath.string(), const_cast<uint8_t*>(puRawImgBuf), u4RawImgSize);
        CAM_LOGD("raw saved: %d: %s", ret, s8RawFilePath.string());
    	}
	else
		{
		unsigned char *pucRawBuf = NULL;
        int rawSize = 0;


        // Start to simulate pack... //

                MTKDngOp *MyDngop = NULL;
                DngOpResultInfo MyDngopResultInfo;
                DngOpImageInfo MyDngopImgInfo;
                // Initialize //
                MY_LOGD("Start to simulate pack...\n");
                //
                MUINT32 bitDstNum = MAKE_DngInfo(LOG_TAG, getOpenId())->getRawBitDepth();//query raw bit num from 3A dnginfo //rawBitDepth;
                //
                MyDngop = MTKDngOp::createInstance(DRV_DNGOP_UNPACK_OBJ_SW);
                MyDngopImgInfo.Width = mpEngParam.u4SensorWidth; //scrBuffer->getImgSize().w;
                MyDngopImgInfo.Height = mpEngParam.u4SensorHeight; //scrBuffer->getImgSize().h;
                MyDngopImgInfo.Stride_src = mpEngParam.u4rawStride; //scrBuffer->getBufStridesInBytes(0);
                MyDngopImgInfo.Stride_dst = mpEngParam.u4SensorWidth*2; //scrBuffer->getImgSize().w*2;
                MyDngopImgInfo.BIT_NUM =  mpEngParam.u4Bitdepth; //10;//rSensorParam.u4Bitdepth , buffer bit num
                MyDngopImgInfo.BIT_NUM_DST = ((mpEngParam.u4Bitdepth == 12) && (bitDstNum == 12))?12:10;
                MyDngopImgInfo.Bit_Depth = 10;//for pack
                //
                // query buffer size
                MUINT32 buf_size = (mpEngParam.u4SensorWidth*2*mpEngParam.u4SensorHeight); //(scrBuffer->getImgSize().w*2*scrBuffer->getImgSize().h);
                MyDngopImgInfo.Buff_size = buf_size;

               // set buffer address
               unsigned char *p_buffer = new unsigned char[buf_size];
               MyDngopResultInfo.ResultAddr = p_buffer;

               // assign raw data
               MyDngopImgInfo.srcAddr = reinterpret_cast<void*>(const_cast<uint8_t*>(puRawImgBuf));

               MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);

               MY_LOGD("after Unpack raw16: w (%d),h (%d),src-s (%d),dst-s (%d),buf-size(%d)",
                        mpEngParam.u4SensorWidth, mpEngParam.u4SensorHeight,
                        mpEngParam.u4rawStride, mpEngParam.u4SensorWidth*2,
                        (mpEngParam.u4SensorWidth*2*mpEngParam.u4SensorHeight));
               //
               MyDngop->destroyInstance(MyDngop);
               //

        // End simulate pack... //

        if (p_buffer)
        {
            bool ret = NSCam::Utils::saveBufToFile(s8RawFilePath.string(), p_buffer, buf_size); // bool ret = saveBufToFile(ms8RawFilePath.string(), const_cast<uint8_t*>(puRawImgBuf), u4RawImgSize);

            delete []p_buffer;
            p_buffer = NULL;
            CAM_LOGD("Raw saved: %d: %s", ret, s8RawFilePath.string());
        }
    }

    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handlePostViewData(MUINT8* const /*puBuf*/, MUINT32 const /*u4Size*/)
{
#if 0
    MY_LOGD("+ (puBuf, size) = (%p, %d)", puBuf, u4Size);
    mpShotCallback->onCB_PostviewClient(0,
                                         u4Size,
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        );

    MY_LOGD("-");
#endif
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handleJpegData(IImageBuffer* pJpeg)
{
    //
    class scopedVar
    {
    public:
                    scopedVar(IImageBuffer* pBuf)
                    : mBuffer(pBuf) {
                        if( mBuffer )
                            mBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    }
                    ~scopedVar() {
                        if( mBuffer )
                            mBuffer->unlockBuf(LOG_TAG);
                    }
    private:
        IImageBuffer* mBuffer;
    } _local(pJpeg);
    //
    uint8_t const* puJpegBuf = (uint8_t const*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();

    MY_LOGD("+ (puJpgBuf, jpgSize) = (%p, %d)",
            puJpegBuf, u4JpegSize);

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    mpShotCallback->onCB_CompressedImage_packed(0,
                                         u4JpegSize,
                                         puJpegBuf,
                                         0,                       //callback index
                                         true,                     //final image
                                         MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                         getShotMode()
                                         );
    MY_LOGD("-");

    return MTRUE;


}


