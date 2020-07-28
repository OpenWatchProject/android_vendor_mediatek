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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "FOVHal"

#include <utils/RefBase.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/DualCam/FOVHal.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#if ENABLE_FOV_ALGO
#include <MTKFOV.h>
#include <mtkcam/aaa/INvBufUtil.h>
#endif
#include <utils/Mutex.h>

#define PIPE_MODULE_TAG "FOVHal"
#define PIPE_CLASS_TAG "FOVHal"
#define PIPE_TRACE TRACE_FOV_HAL
//#include <featurePipe/core/include/PipeLog.h>
#include <cutils/properties.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <camera_custom_dualzoom.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#endif

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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


namespace NSCam {

class FOVHalImp : public FOVHal
{

public:
    virtual ~FOVHalImp() {}
    MBOOL init(FOVHal::InitConfig config);
    MBOOL uninit();
    MBOOL doFOV(FOVHal::Params& Param);
    MBOOL getFOVMarginandCrop(FOVHal::Configs config, MSize& Margin, MRect& Crop, float& Scale);
    MBOOL getFOVCrop(FOVHal::Configs config, MRect& Crop);
    MBOOL setPipelineConfig(FOVHal::Configs config);
    MBOOL setCameraSyncInfo(int CamId, MBOOL needFlash);
    MBOOL getCameraSyncInfo(int CamId, bool& needFlash);
    MBOOL setCamId(int CamId);
    MBOOL getMasterCamId(int& CamId);
    MBOOL getFEFMSRZ1Info(const FOVHal::Configs& config,
                  DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo, _SRZ_SIZE_INFO_& rInfo,
                  DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo2, _SRZ_SIZE_INFO_& rInfo2);
    virtual MBOOL setCurrentCropInfo(MINT32 CamID, MINT32 ZoomRatio, MRect ActiveCrop, MSize Margin);
    virtual MBOOL getCurrentCropInfo(MINT32 CamID, MRect &Crop, MSize &Margin, MPoint &Offset, float &Scale);
private:
    void  setFOVDefaultOutput(FOVHal::Params& Param);
    #if ENABLE_FOV_ALGO
    MBOOL loadFOVMap(MUINT32*& nvramData, MUINT32*& eepromData);
    MBOOL storeFOVMap(void);
    void  updateCalibInfo(FOVHal::Params& Param, FOVImageInfo& info);
    #if (20 == FOV_HAL_VER)
    void  updateFOVInfo(FOVImageInfo& img_info);
    #endif
    #endif
private:
    #if ENABLE_FOV_ALGO
    MTKFOV* mFovObj;
    MUINT32 mWorkingBufSize;
    MUINT8* mWorkingBuf;
    MINT32  mDumpResult;
    MPoint  mFOVShift;
    float   mFOVScale;
    FOVResultInfo mResult;
    MBOOL   mFOVMapUpdated;
    #endif
    MINT32  mEisFactor;
    MBOOL   mIs4k2k;
    MSize   mStreamingSize;
    mutable Mutex mControlMutex;
    MBOOL   mNeedFlash;
    MINT32  mMasterId;
    MRect   mActiveCrop;
    MSize   mMargin;
};

const MUINT32 FOV_FEATURE_MATCHING_BLOCK_SIZE = 16;
// config data
FOVHal::SizeConfig FOV_ConfigData_4to3 = {MSize(400, 304), FOV_FEATURE_MATCHING_BLOCK_SIZE, MSize(4032, 3016), 0.5149};
FOVHal::SizeConfig FOV_ConfigData_16to9 = {MSize(400, 224), FOV_FEATURE_MATCHING_BLOCK_SIZE, MSize(4032, 2270), 0.5149};

static FOVHalImp singleton;

FOVHal::SizeConfig::
SizeConfig(
    const MSize& feImgSize,
    const MUINT32& iBlockSize,
    MSize szTeleUseResol,
    MFLOAT zoomFactor
)
: mFEImgSize(feImgSize)
, miBlockSize(iBlockSize)
, mTeleUseResol(szTeleUseResol)
, mAnchorZoomFactor(zoomFactor)
{
    mFEOSize.w = mFEImgSize.w/iBlockSize*40;
    mFEOSize.h = mFEImgSize.h/iBlockSize;

    mFMOSize.w = (mFEOSize.w/40) * 2;
    mFMOSize.h = mFEOSize.h;
}

FOVHal*
FOVHal::getInstance()
{
    return &singleton;
}


FOVHal::SizeConfig
FOVHal::getSizeConfig(RAW_RATIO ratio)
{
    if(ratio == RATIO_16_9)
        return FOV_ConfigData_16to9;
    else
        return FOV_ConfigData_4to3;
}

MBOOL
FOVHal::configTuning_FE(
    const SizeConfig& config,
    NSCam::NSIoPipe::FEInfo* pFETuning
)
{
    switch(config.miBlockSize) {
        case 16:
            pFETuning->mFEMODE = 1;
            pFETuning->mFEPARAM = 8;
            pFETuning->mFEFLT_EN = 1;
            pFETuning->mFETH_G = 0;
            pFETuning->mFETH_C = 2;
            pFETuning->mFEDSCR_SBIT = 3;
            pFETuning->mFEXIDX = 0;
            pFETuning->mFEYIDX = 0;
            pFETuning->mFESTART_X = 0;
            pFETuning->mFESTART_Y = 0;
            pFETuning->mFEIN_HT = 0;
            pFETuning->mFEIN_WD = 0;
            break;
        case 8:
        default:
            pFETuning->mFEMODE = 2;
            pFETuning->mFEPARAM = 4;
            pFETuning->mFEFLT_EN = 1;
            pFETuning->mFETH_G = 1;
            pFETuning->mFETH_C = 0;
            pFETuning->mFEDSCR_SBIT = 3;
            pFETuning->mFEXIDX = 0;
            pFETuning->mFEYIDX = 0;
            pFETuning->mFESTART_X = 0;
            pFETuning->mFESTART_Y = 0;
            pFETuning->mFEIN_HT = 0;
            pFETuning->mFEIN_WD = 0;
            break;
    }
    return MTRUE;
}

MBOOL
FOVHal::configTuning_FM(
    const SizeConfig& config,
    NSCam::NSIoPipe::FMInfo* pFMTuning
)
{
    pFMTuning->mFMOFFSET_X = 16;
    pFMTuning->mFMOFFSET_Y = 16;
    pFMTuning->mFMSR_TYPE = 0;
    pFMTuning->mFMMIN_RATIO = 90;
    pFMTuning->mFMSAD_TH = 1023;
    pFMTuning->mFMRES_TH = 0;
    pFMTuning->mFMWIDTH = config.mFEImgSize.w/config.miBlockSize;
    pFMTuning->mFMHEIGHT = config.mFEImgSize.h/config.miBlockSize;

    return MTRUE;
}

FOVHal::RAW_RATIO
FOVHal::queryRatio(MSize inputSize)
{
    MUINT32 szDiff = inputSize.w - inputSize.h;
    if(szDiff * 3 > inputSize.w)
        return RATIO_16_9;
    else
        return RATIO_4_3;
}

// public API
MBOOL
FOVHalImp::setPipelineConfig(Configs config)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        return 0;
    }
    if (config.CamID == DUALZOOM_FOV_APPLIED_CAM)
    {
        mEisFactor = config.EisFactor;
        mIs4k2k = config.Is4K2K;
        mStreamingSize = config.MainStreamingSize;
    }
    #endif
    return true;
}

MBOOL
FOVHalImp::setCameraSyncInfo(int CamId, MBOOL needFlash)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        return 0;
    }
    if(mMasterId == CamId)
    {
        mNeedFlash = needFlash;
    }
    #endif
    return true;
}

MBOOL
FOVHalImp::getCameraSyncInfo(int CamId, bool& needFlash)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        return 0;
    }
    needFlash = mNeedFlash;
    #endif
    return true;
}

MBOOL
FOVHalImp::setCamId(int CamId)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    mMasterId = CamId;
    #endif
    return true;
}

MBOOL
FOVHalImp::getMasterCamId(int& CamId)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        return 0;
    }
    CamId = mMasterId;
    return true;
    #else
    return 0;
    #endif
}

MBOOL
FOVHal::isSupportMultiFov()
{
    #if (20 == FOV_HAL_VER)
    return true;
    #else
    return 0;
    #endif
}

// public API
MBOOL
FOVHalImp::setCurrentCropInfo(MINT32 CamID, MINT32 ZoomRatio, MRect ActiveCrop, MSize Margin)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        mMargin = MSize(0, 0);
        return true;
    }
    if (CamID == DUALZOOM_FOV_APPLIED_CAM)
    {
        if (ZoomRatio >= DUALZOOM_START_FOV_ZOOM_RATIO)
        {
            mActiveCrop = ActiveCrop;
            mMargin = Margin;
        }
        else
        {
            mMargin = MSize(0, 0);
        }
    }
    #endif
    return true;
}

MBOOL
FOVHalImp::getCurrentCropInfo(MINT32 CamID, MRect &Crop, MSize &Margin, MPoint &Offset, float &Scale)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        Margin = MSize(0, 0);
        return true;
    }
    if (CamID == DUALZOOM_FOV_APPLIED_CAM)
    {
        Crop = mActiveCrop;
        Margin = mMargin;
        #if ENABLE_FOV_ALGO
        Offset = mFOVShift;
        Scale = mFOVScale;
        #endif
    }
    else
    {
        Margin = MSize(0, 0);
    }
    #else
    Margin = MSize(0, 0);
    #endif
    return true;
}


MBOOL
FOVHalImp::getFOVMarginandCrop(Configs config, MSize& Margin, MRect& Crop, float& Scale)
{
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    Mutex::Autolock _l(mControlMutex);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
    {
        Margin = MSize(0, 0);
        Crop.s = MSize(0, 0);
        return 0;
    }
    if (config.CamID == DUALZOOM_FOV_APPLIED_CAM && config.ZoomRatio >= DUALZOOM_START_FOV_ZOOM_RATIO)
    {
        if (mEisFactor != 0)
        {
            MINT32 MinMargin_W = 1920 * DUALZOOM_FOV_MARGIN_COMBINE_EIS / 100;
            Margin.w = (config.CropSize.w * DUALZOOM_FOV_MARGIN_COMBINE_EIS / mEisFactor) * mEisFactor / (mEisFactor - DUALZOOM_FOV_MARGIN_COMBINE_EIS);
            Margin.h = (config.CropSize.h * DUALZOOM_FOV_MARGIN_COMBINE_EIS / mEisFactor) * mEisFactor / (mEisFactor - DUALZOOM_FOV_MARGIN_COMBINE_EIS);
            Margin.w = Margin.w + (Margin.w & 0x01);
            Margin.h = Margin.h + (Margin.h & 0x01);
            if (Margin.w < MinMargin_W)
            {
                Margin.w = MinMargin_W;
                Margin.h = Margin.w * config.CropSize.h / config.CropSize.w;
            }
        }
        else
        {
            Margin.w = DUALZOOM_FOV_MARGIN_PIXEL;
            Margin.h = DUALZOOM_FOV_MARGIN_PIXEL * config.SensorSize.h / config.SensorSize.w;
        }
        #if ENABLE_FOV_ALGO
        if (mFOVShift.x != 0 && mFOVShift.y != 0 && mFOVScale != 0)
        {
            FOVImageInfo img_info;
            FOVResultInfo Result;
            img_info.current_scale = ((float)(config.ZoomRatio))/100.0f;
            img_info.fovActiveScale = DUALZOOM_START_FOV_ZOOM_RATIO / 100.0f;
            img_info.switch_scale = DUALZOOM_SWICH_CAM_ZOOM_RATIO / 100.0f;
            img_info.cropRatio_initial = 0.5149;
            img_info.Calibration_on_Tele = false;//false;//true;
            img_info.online_calibration = FOV_ONLINE_CALIBRATION_OFF;
            img_info.imgW_wide_ori = 4032;
            img_info.imgH_wide_ori = 3016;
            img_info.imgW_tele_ori = 4032;
            img_info.imgH_tele_ori = 3016;
            mFovObj->FOVFeatureCtrl(FOV_FEATURE_ADD_IMAGE, (MUINT32*)&img_info, NULL);
            mFovObj->FOVMain();
            mFovObj->FOVFeatureCtrl(FOV_FEATURE_GET_RESULT, NULL, &Result);
            mFOVShift.x = Result.OffsetX;
            mFOVShift.y = Result.OffsetY;
            mFOVScale   = Result.scale;
            Crop.p.x = mFOVShift.x;
            Crop.p.y = mFOVShift.y;
            Crop.s.w = Margin.w;
            Crop.s.h = Margin.h;
            Scale = mFOVScale;
            //MY_LOGD("fov data : %d, %d, %f",Crop.p.x, Crop.p.y, Scale);
        }
        else
        #endif
        {
            Crop.p.x = 0;
            Crop.p.y = 0;
            Crop.s.w = Margin.w;
            Crop.s.h = Margin.h;
            Scale = 1.0;
        }
    }
    else
    {
        Margin = MSize(0, 0);
        Crop.s = MSize(0, 0);
    }
    #endif
    return true;
}

// public API
MBOOL
FOVHalImp::getFOVCrop(Configs config, MRect& Crop)
{
    Mutex::Autolock _l(mControlMutex);
    // TODO implement
    return true;
}



// public API
MBOOL
FOVHalImp::init(FOVHal::InitConfig config)
{
    // to do implement
    #if ENABLE_FOV_ALGO
    Mutex::Autolock _l(mControlMutex);
    FOVImageInfo img_info;
    mFovObj = MTKFOV::createInstance(DRV_FOV_OBJ_CPU);
    mWorkingBuf = NULL;
    FOVResultInfo Result;
    //init
    MY_LOGD("FOV HAL init");
    mMasterId = -1;
    img_info.CalibrationMode = FOV_CALIBRATION_TYPE_ROTATION;
    img_info.online_calibration = FOV_ONLINE_CALIBRATION_INFINITY;
    /*Read HW fefm data*/
    img_info.FeBlockSize[0] = 16;
    img_info.FeBlockSize[1] = 16;
    img_info.imgW_FEFM = 400;
    img_info.imgH_FEFM = 304;
    img_info.fovActiveScale = DUALZOOM_START_FOV_ZOOM_RATIO / 100.0f;
    img_info.switch_scale = DUALZOOM_SWICH_CAM_ZOOM_RATIO / 100.0f;
    img_info.cropRatio_initial = 0.5149;
    img_info.Calibration_on_Tele = false;//true;

    //for EEPROM
    img_info.wide_WD_check = 4032;
    img_info.wide_HT_check = 3016;
    img_info.tele_WD_check = 4176;
    img_info.tele_HT_check = 3120;
    img_info.distNea_A_check = 20.0;
    img_info.distFar_B_check = 90.0;

    MUINT32 *pNVData = NULL;
    MUINT32 *pEEPData = NULL;
    if(!loadFOVMap(pNVData, pEEPData))
    {
        MY_LOGD("No FOV map data found, use default map!");
    }
    img_info.input_nvram_data = pNVData;
    img_info.input_eeprom_data = pEEPData;
    img_info.imgW_wide_ori = 4032;
    img_info.imgH_wide_ori = 3016;
    img_info.imgW_tele_ori = 4032;
    img_info.imgH_tele_ori = 3016;

    img_info.fov_tuning_para.learningRate = 0.05;
    img_info.fov_tuning_para.thre_var_mv_square = 9.0;
    #if (20 == FOV_HAL_VER)
    // update AF data
    updateFOVInfo(img_info);
    #else
    img_info.support_multi_dist = false;
    #endif
    mFovObj->FOVInit((MUINT32*)&img_info, NULL);

    //Allocate working buffer
    mFovObj->FOVFeatureCtrl(FOV_FEATURE_GET_WORKBUF_SIZE, NULL, &mWorkingBufSize);
    if(mWorkingBufSize != 0)
    {
        mWorkingBuf = (MUINT8*) malloc(mWorkingBufSize);
        mFovObj->FOVFeatureCtrl(FOV_FEATURE_SET_WORKBUF_ADDR, mWorkingBuf, NULL);
    }
    mFovObj->FOVFeatureCtrl(FOV_FEATURE_GET_RESULT, NULL, &Result);
    mFOVShift.x = Result.OffsetX;
    mFOVShift.y = Result.OffsetY;
    mFOVScale   = Result.scale;
    /*if (*Result.output_nvram_data)
        mFOVMapUpdated = MTRUE;*/
    mDumpResult = property_get_int32("vendor.debug.camera.DumpFOVInfo", 0);

    // create calibration thread

    #endif
    return true;
}

MBOOL
FOVHalImp::uninit()
{
    // to do implement
    #if ENABLE_FOV_ALGO
    Mutex::Autolock _l(mControlMutex);
    if(mFovObj != NULL)
    {
        mFovObj->FOVReset();
        mFovObj->destroyInstance(mFovObj);
        mFovObj = NULL;
        storeFOVMap();
    }
    if(mWorkingBuf != NULL)
    {
        free(mWorkingBuf);
        mWorkingBuf = NULL;
    }
    mFOVShift.x = 0;
    mFOVShift.y = 0;
    mFOVScale = 1.0;
    #endif
    //mNeedFlash = -1;
    mMasterId = -1;
    return true;
}

MBOOL
FOVHalImp::doFOV(FOVHal::Params& Param)
{
    Mutex::Autolock _l(mControlMutex);
    if ( Param.FOVType == MTK_FOV_TYPE_OFFLINE ) {

        #if ENABLE_FOV_ALGO
        if (Param.DoFOV || Param.DoCalibration)
        {
            FOVImageInfo img_info;
            // for offline fov
            img_info.current_scale = ((float)(Param.ZoomRatio))/100.0f;
            img_info.fovActiveScale = DUALZOOM_START_FOV_ZOOM_RATIO / 100.0f;
            img_info.switch_scale = DUALZOOM_SWICH_CAM_ZOOM_RATIO / 100.0f;
            img_info.cropRatio_initial = 0.5149;
            img_info.Calibration_on_Tele = false;//false;//true;
            img_info.online_calibration = FOV_ONLINE_CALIBRATION_OFF;
            img_info.imgW_wide_ori = 4032;//Param.FOVInfo.ActiveArrayW;
            img_info.imgH_wide_ori = 3016;//Param.FOVInfo.ActiveArrayH;
            img_info.imgW_tele_ori = 4032;//Param.FOVInfo.ActiveArrayW_Main2;
            img_info.imgH_tele_ori = 3016;//Param.FOVInfo.ActiveArrayH_Main2;
            img_info.imgW_crop = Param.FOVInfo.FOVCropW;
            img_info.imgH_crop = Param.FOVInfo.FOVCropH;
            img_info.imgW_zoom = Param.FOVInfo.RealCropW;
            img_info.imgH_zoom = Param.FOVInfo.RealCropH;
            img_info.imgW_crop_resize = Param.SrcImgSize.w;
            img_info.imgH_crop_resize = Param.SrcImgSize.h;
            img_info.imgW_zoom_resize = (Param.SrcImgSize.w * Param.FOVInfo.RealCropW / Param.FOVInfo.FOVCropW) & ~0x01;
            img_info.imgH_zoom_resize = (Param.SrcImgSize.h * Param.FOVInfo.RealCropH / Param.FOVInfo.FOVCropH) & ~0x01;
            img_info.imageAlignCenter_x = Param.FOVInfo.CropCenterX;
            img_info.imageAlignCenter_y = Param.FOVInfo.CropCenterY;
            #if (20 == FOV_HAL_VER)
            // multi distance
            img_info.wide_DAC = Param.FOVInfo.Main1AFDac;
            img_info.tele_DAC = Param.FOVInfo.Main2AFDac;
            img_info.AF_SUCCESS_wide = Param.FOVInfo.Main1AFSuccess;
            img_info.AF_SUCCESS_tele = Param.FOVInfo.Main2AFSuccess;
            img_info.AF_DONE_wide = Param.FOVInfo.Main1AFDone;
            img_info.AF_DONE_tele = Param.FOVInfo.Main2AFDone;
            #endif

            // for calibration
            if (Param.DoCalibration)
            {
                MY_LOGD("Update calib");
                updateCalibInfo(Param, img_info);
            }
            mFovObj->FOVFeatureCtrl(FOV_FEATURE_ADD_IMAGE, (MUINT32*)&img_info, NULL);
            mFovObj->FOVMain();
            mFovObj->FOVFeatureCtrl(FOV_FEATURE_GET_RESULT, NULL, &mResult);
            if (mDumpResult)
            {
                MY_LOGD("*************INPUT***************");
                MY_LOGD("img_info.current_scale : %f", img_info.current_scale);
                MY_LOGD("img_info.imgW_wide_ori : %d", img_info.imgW_wide_ori);
                MY_LOGD("img_info.imgH_wide_ori : %d", img_info.imgH_wide_ori);
                MY_LOGD("img_info.imgW_tele_ori : %d", img_info.imgW_tele_ori);
                MY_LOGD("img_info.imgH_tele_ori : %d", img_info.imgH_tele_ori);
                MY_LOGD("img_info.imgW_crop : %d", img_info.imgW_crop);
                MY_LOGD("img_info.imgH_crop : %d", img_info.imgH_crop);
                MY_LOGD("img_info.imgW_zoom : %d", img_info.imgW_zoom);
                MY_LOGD("img_info.imgH_zoom : %d", img_info.imgH_zoom);
                MY_LOGD("img_info.imgW_crop_resize : %d", img_info.imgW_crop_resize);
                MY_LOGD("img_info.imgH_crop_resize : %d", img_info.imgH_crop_resize);
                MY_LOGD("img_info.imgW_zoom_resize : %d", img_info.imgW_zoom_resize);
                MY_LOGD("img_info.imgH_zoom_resize : %d", img_info.imgH_zoom_resize);
                MY_LOGD("img_info.imageAlignCenter_x : %d", img_info.imageAlignCenter_x);
                MY_LOGD("img_info.imageAlignCenter_y : %d", img_info.imageAlignCenter_y);
                #if (20 == FOV_HAL_VER)
                MY_LOGD("img_info.wide_DAC : %d", img_info.wide_DAC);
                MY_LOGD("img_info.tele_DAC : %d", img_info.tele_DAC);
                MY_LOGD("img_info.AF_SUCCESS_wide : %d", img_info.AF_SUCCESS_wide);
                MY_LOGD("img_info.AF_SUCCESS_tele : %d", img_info.AF_SUCCESS_tele);
                MY_LOGD("img_info.AF_DONE_wide : %d", img_info.AF_DONE_wide);
                MY_LOGD("img_info.AF_DONE_tele : %d", img_info.AF_DONE_tele);
                #endif
                MY_LOGD("*************CALIB***************");
                MY_LOGD("img_info.online_calibration : %d", img_info.online_calibration);
                MY_LOGD("img_info.imgW_FEFM : %d", img_info.imgW_FEFM);
                MY_LOGD("img_info.imgH_FEFM : %d", img_info.imgH_FEFM);
                MY_LOGD("img_info.FEFM_WD_crop_tele : %d", img_info.FEFM_WD_crop_tele);
                MY_LOGD("img_info.FEFM_HT_crop_tele : %d", img_info.FEFM_HT_crop_tele);
                MY_LOGD("img_info.Fe_wide : %p", img_info.Fe_wide);
                MY_LOGD("img_info.Fe_tele : %p", img_info.Fe_tele);
                MY_LOGD("img_info.Fm_wide : %p", img_info.Fm_wide);
                MY_LOGD("img_info.Fm_tele : %p", img_info.Fm_tele);
                MY_LOGD("img_info.FeBlockSize[0] : %d", img_info.FeBlockSize[0]);
                MY_LOGD("img_info.FeBlockSize[1] : %d", img_info.FeBlockSize[1]);
                MY_LOGD("img_info.autoFocus_DAC : %f", img_info.autoFocus_DAC);
                MY_LOGD("img_info.infinitySceneProbability : %f", img_info.infinitySceneProbability);
                MY_LOGD("img_info.ISO : %d", img_info.ISO);
                MY_LOGD("img_info.temperature : %d", img_info.temperature);
                MY_LOGD("*************OUTPUT**************");
                MY_LOGD("mResult.WarpMapX[0] : %f", mResult.WarpMapX[0]/32.0f);
                MY_LOGD("mResult.WarpMapX[1] : %f", mResult.WarpMapX[1]/32.0f);
                MY_LOGD("mResult.WarpMapX[2] : %f", mResult.WarpMapX[2]/32.0f);
                MY_LOGD("mResult.WarpMapX[3] : %f", mResult.WarpMapX[3]/32.0f);
                MY_LOGD("mResult.WarpMapY[0] : %f", mResult.WarpMapY[0]/32.0f);
                MY_LOGD("mResult.WarpMapY[1] : %f", mResult.WarpMapY[1]/32.0f);
                MY_LOGD("mResult.WarpMapY[2] : %f", mResult.WarpMapY[2]/32.0f);
                MY_LOGD("mResult.WarpMapY[3] : %f", mResult.WarpMapY[3]/32.0f);
            }
            if (Param.DoFOV)
            {
                Param.WarpMapSize.w = 2;
                Param.WarpMapSize.h = 2;
                 // warp X
                Param.pWarpMapX[0]= mResult.WarpMapX[0];
                Param.pWarpMapX[1]= mResult.WarpMapX[1];
                Param.pWarpMapX[2]= mResult.WarpMapX[2];
                Param.pWarpMapX[3]= mResult.WarpMapX[3];
                // warp Y
                Param.pWarpMapY[0]= mResult.WarpMapY[0];
                Param.pWarpMapY[1]= mResult.WarpMapY[1];
                Param.pWarpMapY[2]= mResult.WarpMapY[2];
                Param.pWarpMapY[3]= mResult.WarpMapY[3];

                Param.WPEOutput.w = img_info.imgW_zoom_resize;
                Param.WPEOutput.h = img_info.imgH_zoom_resize;

                Param.SensorMargin.w = Param.FOVInfo.FOVCropW - Param.FOVInfo.RealCropW;
                Param.SensorMargin.h = Param.FOVInfo.FOVCropH - Param.FOVInfo.RealCropH;
                Param.ResizeMargin.w = img_info.imgW_crop_resize - img_info.imgW_zoom_resize;
                Param.ResizeMargin.h = img_info.imgH_crop_resize - img_info.imgH_zoom_resize;

                //Param.FOVShift.x = mFOVShift.x;
                //Param.FOVShift.y = mFOVShift.y;
                //Param.FOVScale= mFOVScale;
                Param.FOVShift.x = mFOVShift.x = mResult.OffsetX;
                Param.FOVShift.y = mFOVShift.y = mResult.OffsetY;
                Param.FOVScale = mFOVScale = mResult.scale;
            }
            else
            {
                setFOVDefaultOutput(Param);
            }
            if (Param.DoCalibration)
            {
                Param.FOVUpdated = (mResult.update_nvram_flag == 1);
                mFOVMapUpdated |= (mResult.update_nvram_flag == 1);
                MY_LOGD("update_nvram_flag : %d", mResult.update_nvram_flag);
            }
        }
        else
        {
            setFOVDefaultOutput(Param);
        }
        #else
        Param.WarpMapSize.w = 2;
        Param.WarpMapSize.h = 2;
        // warp X
        Param.pWarpMapX[0]= 0;
        Param.pWarpMapX[1]= (Param.SrcImgSize.w - 1) << 5;
        Param.pWarpMapX[2]= 0;
        Param.pWarpMapX[3]= (Param.SrcImgSize.w - 1) << 5;
        // warp Y
        Param.pWarpMapY[0]= 0;
        Param.pWarpMapY[1]= 0;
        Param.pWarpMapY[2]= (Param.SrcImgSize.h - 1) << 5;
        Param.pWarpMapY[3]= (Param.SrcImgSize.h - 1) << 5;

        Param.WPEOutput.w = (Param.SrcImgSize.w * Param.FOVInfo.RealCropW / Param.FOVInfo.FOVCropW) & ~0x01;
        Param.WPEOutput.h = (Param.SrcImgSize.h * Param.FOVInfo.RealCropH / Param.FOVInfo.FOVCropH) & ~0x01;

        Param.SensorMargin.w = Param.FOVInfo.FOVCropW - Param.FOVInfo.RealCropW;
        Param.SensorMargin.h = Param.FOVInfo.FOVCropH - Param.FOVInfo.RealCropH;
        Param.ResizeMargin.w = Param.SrcImgSize.w - Param.WPEOutput.w;
        Param.ResizeMargin.h = Param.SrcImgSize.h - Param.WPEOutput.h;

        Param.FOVShift.x = 0;
        Param.FOVShift.y = 0;
        Param.FOVScale= 1.0;

        MY_LOGD("full image size : %dx%d", Param.SrcImgSize.w, Param.SrcImgSize.h);
        MY_LOGD("WPE OUTPUT image size : %dx%d", Param.WPEOutput.w, Param.WPEOutput.h);
        #endif
    }
    return true;
}

// private API
void
FOVHalImp::setFOVDefaultOutput(FOVHal::Params& Param)
{
    Param.WarpMapSize.w = 2;
    Param.WarpMapSize.h = 2;
    // warp X
    Param.pWarpMapX[0]= 0;
    Param.pWarpMapX[1]= (Param.SrcImgSize.w - 1) << 5;
    Param.pWarpMapX[2]= 0;
    Param.pWarpMapX[3]= (Param.SrcImgSize.w - 1) << 5;
    // warp Y
    Param.pWarpMapY[0]= 0;
    Param.pWarpMapY[1]= 0;
    Param.pWarpMapY[2]= (Param.SrcImgSize.h - 1) << 5;
    Param.pWarpMapY[3]= (Param.SrcImgSize.h - 1) << 5;

    Param.WPEOutput.w = Param.SrcImgSize.w;
    Param.WPEOutput.h = Param.SrcImgSize.h;

    Param.SensorMargin.w = 0;
    Param.SensorMargin.h = 0;
    Param.ResizeMargin.w = 0;
    Param.ResizeMargin.h = 0;
    Param.FOVShift.x = 0;
    Param.FOVShift.y = 0;
    Param.FOVScale= 1.0;
}

#if ENABLE_FOV_ALGO
void
FOVHalImp::updateCalibInfo(FOVHal::Params& Param, FOVImageInfo& info)
{
    info.online_calibration = FOV_ONLINE_CALIBRATION_INFINITY;
    info.imgW_FEFM = 400;
    info.imgH_FEFM = 304;
    info.FeBlockSize[0] = 16;
    info.FeBlockSize[1] = 16;
    if (Param.MasterID == DUALZOOM_WIDE_CAM_ID)
    {
        info.Fe_wide = Param.FE_master;
        info.Fe_tele = Param.FE_slave;
        info.Fm_wide = Param.FM_master;
        info.Fm_tele = Param.FM_slave;
        if (Param.FECrop_slave.w > 4032)
        {
            info.FEFM_WD_crop_tele = 4032;
            info.FEFM_HT_crop_tele = 3016;
        }
        else
        {
            info.FEFM_WD_crop_tele = Param.FECrop_slave.w;
            info.FEFM_HT_crop_tele = Param.FECrop_slave.h;
        }
        info.FEFM_WD_crop_wide = Param.FECrop_master.w;
        info.FEFM_HT_crop_wide = Param.FECrop_master.h;
    }
    else
    {
        info.Fe_wide = Param.FE_slave;
        info.Fe_tele = Param.FE_master;
        info.Fm_wide = Param.FM_slave;
        info.Fm_tele = Param.FM_master;
        if (Param.FECrop_master.w > 4032)
        {
            info.FEFM_WD_crop_tele = 4032;
            info.FEFM_HT_crop_tele = 3016;
        }
        else
        {
            info.FEFM_WD_crop_tele = Param.FECrop_master.w;
            info.FEFM_HT_crop_tele = Param.FECrop_master.h;
        }
        info.FEFM_WD_crop_wide = Param.FECrop_slave.w;
        info.FEFM_HT_crop_wide = Param.FECrop_slave.h;
    }
}

MBOOL
FOVHalImp::loadFOVMap(MUINT32*& nvramData, MUINT32*& eepromData)
{
    #if (20 == FOV_HAL_VER)
    #define FOV_MAP_TABLE_SIZE  (360)
    #else
    #define FOV_MAP_TABLE_SIZE  (36)
    #endif

    mFOVMapUpdated = MFALSE;

    MBOOL NVDataFound = MFALSE;
    if (auto pNvBufUtil = MAKE_NvBufUtil())
    {
        pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FOV, SENSOR_DEV_MAIN, (void*&)nvramData);
        if (nvramData)
        {
            MINT32 *size = (MINT32*)nvramData;
            MY_LOGD("NVRAM data size %d", *size);
            if (*size == FOV_MAP_TABLE_SIZE)
                NVDataFound = MTRUE;
            else
                *size = 0; // indicate no map data in NVRAM
        }
    }

    if (!NVDataFound)
    {
        //// load map data from EEPROM
        int32_t devIdx[2];
        StereoSettingProvider::getStereoSensorDevIndex(devIdx[0], devIdx[1]);
        //Get calibration data
        CAM_CAL_DATA_STRUCT calibrationData;
        CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
        MUINT32 queryResult;
        queryResult = pCamCalDrvObj->GetCamCalCalData(devIdx[0], CAMERA_CAM_CAL_DATA_STEREO_DATA, (void *)&calibrationData);

        if (queryResult == 0)
        {
            eepromData = (MUINT32 *)calibrationData.Stereo_Data.Data;
        }

        mFOVMapUpdated = MTRUE;
    }

    return NVDataFound;
}

MBOOL
FOVHalImp::storeFOVMap(void)
{
    if (!mFOVMapUpdated)
        return MTRUE;

    if (auto pNvBufUtil = MAKE_NvBufUtil())
    {
        pNvBufUtil->write(CAMERA_NVRAM_DATA_FOV, SENSOR_DEV_MAIN);
        MY_LOGD("FOV map updated");
        return MTRUE;
    }
    return MFALSE;
}
#if (20 == FOV_HAL_VER)
void
FOVHalImp::updateFOVInfo(FOVImageInfo& img_info)
{
    // set AF DAC data
    {
        int32_t devIdx[2];
        StereoSettingProvider::getStereoSensorDevIndex(devIdx[0], devIdx[1]);
        //Get calibration data
        CAM_CAL_DATA_STRUCT calibrationData;
        CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
        MUINT32 queryResult;

        img_info.fov_tuning_para.FOV_WideTele_normalized_DAC_diff_thre = 0.3;
        img_info.fov_tuning_para.mappingTemporalUpdateRate = 0.05;
        img_info.fov_tuning_para.learningRate_fuzzy = 0.25;
        img_info.fov_tuning_para.interpolationWeightCenter = 0.8;
        img_info.support_multi_dist = true;

        queryResult = pCamCalDrvObj->GetCamCalCalData(devIdx[0], CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&calibrationData);
        //MacroPos = calibrationData.Single2A.S2aAf[1];
        img_info.wide_DAC_INF = calibrationData.Single2A.S2aAf[0];
        img_info.wide_DAC_MID = calibrationData.Single2A.S2aAF_t.AF_Middle_calibration;

        queryResult = pCamCalDrvObj->GetCamCalCalData(devIdx[1], CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&calibrationData);
        img_info.tele_DAC_MAC= calibrationData.Single2A.S2aAf[1];
        img_info.tele_DAC_INF = calibrationData.Single2A.S2aAf[0];

        if (mDumpResult)
        {
            MY_LOGD("img_info.wide_DAC_INF : %d", img_info.wide_DAC_INF);
            MY_LOGD("img_info.wide_DAC_MID : %d", img_info.wide_DAC_MID);
            MY_LOGD("img_info.tele_DAC_MAC : %d", img_info.tele_DAC_MAC);
            MY_LOGD("img_info.tele_DAC_INF : %d", img_info.tele_DAC_INF);
        }

        pCamCalDrvObj->destroyInstance();
    }
}
#endif
#endif

MBOOL
FOVHalImp::getFEFMSRZ1Info(
    const FOVHal::Configs& config,
    DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo,
    _SRZ_SIZE_INFO_& rInfo,
    DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo2,
    _SRZ_SIZE_INFO_& rInfo2
)
{
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    FOVHal::RAW_RATIO ratio = FOVHal::queryRatio(config.MainStreamingSize);
    FOVHal::SizeConfig sizeCfg = FOVHal::getSizeConfig(ratio);
    DUAL_ZOOM_FOV_FEFM_INFO* pWideInfo;
    DUAL_ZOOM_FOV_FEFM_INFO* pTeleInfo;
    _SRZ_SIZE_INFO_* pWideSRZ;
    _SRZ_SIZE_INFO_* pTeleSRZ;
    MSize WideStreamSize;
    MSize TeleStreamSize;

    if (pFOVInfo == nullptr || pFOVInfo2 == nullptr)
    {
        MY_LOGE("FOVInfo is null!! Failed to calculate the srz info.");
        return MFALSE;
    }

    if(config.CamID == DUALZOOM_WIDE_CAM_ID)
    {
        pWideInfo = pFOVInfo;
        pWideSRZ = &rInfo;
        WideStreamSize = config.MainStreamingSize;
        pTeleInfo = pFOVInfo2;
        pTeleSRZ = &rInfo2;
        TeleStreamSize = config.SlaveStreamingSize;
    }
    else
    {
        pTeleInfo = pFOVInfo;
        pTeleSRZ = &rInfo;
        TeleStreamSize = config.MainStreamingSize;
        pWideInfo = pFOVInfo2;
        pWideSRZ = &rInfo2;
        WideStreamSize = config.SlaveStreamingSize;
    }
    pWideSRZ->in_w = WideStreamSize.w;
    pWideSRZ->in_h = WideStreamSize.h;
    pWideSRZ->out_w = sizeCfg.mFEImgSize.w;
    pWideSRZ->out_h = sizeCfg.mFEImgSize.h;
    pTeleSRZ->in_w = TeleStreamSize.w;
    pTeleSRZ->in_h = TeleStreamSize.h;
    pTeleSRZ->out_w = sizeCfg.mFEImgSize.w;
    pTeleSRZ->out_h = sizeCfg.mFEImgSize.h;
    if (pTeleInfo->P1CropW * sizeCfg.mAnchorZoomFactor < pWideInfo->P1CropW)
    {
        // Wide > Tele, do Wide crop
        MSize WideCrop;
        MFLOAT RRZOFactor = 1.0;
        WideCrop.w = ((int)(pTeleInfo->P1CropW * sizeCfg.mAnchorZoomFactor));
        WideCrop.h = ((int)(pTeleInfo->P1CropH * sizeCfg.mAnchorZoomFactor));
        RRZOFactor = (MFLOAT)WideStreamSize.w / (MFLOAT)pWideInfo->P1CropW;
        // config wide crop
        pWideSRZ->crop_w = ((int)(WideCrop.w * RRZOFactor)) & (~0x01);
        pWideSRZ->crop_h = ((int)(WideCrop.h * RRZOFactor)) & (~0x01);
        pWideSRZ->crop_x = (WideStreamSize.w - pWideSRZ->crop_w) / 2;
        pWideSRZ->crop_y = (WideStreamSize.h - pWideSRZ->crop_h) / 2;
        pWideSRZ->crop_floatX = pWideSRZ->crop_floatY = 0;
        // config tele crop
        pTeleSRZ->crop_w = TeleStreamSize.w;
        pTeleSRZ->crop_h = TeleStreamSize.h;
        pTeleSRZ->crop_x = 0;
        pTeleSRZ->crop_y = 0;
        pTeleSRZ->crop_floatX = pTeleSRZ->crop_floatY = 0;
        MY_LOGD("RRZOFactor : %f, crop(%d, %d)", RRZOFactor, WideCrop.w, WideCrop.h);
    }
    else
    {
        // Tele > Wide, do Tele crop
        MSize TeleCrop;
        MFLOAT RRZOFactor = 1.0;
        TeleCrop.w = ((int)(pWideInfo->P1CropW / sizeCfg.mAnchorZoomFactor));
        TeleCrop.h = ((int)(pWideInfo->P1CropH / sizeCfg.mAnchorZoomFactor));
        RRZOFactor = (MFLOAT)TeleStreamSize.w / (MFLOAT)pTeleInfo->P1CropW;
        // config wide crop
        pWideSRZ->crop_w = WideStreamSize.w;
        pWideSRZ->crop_h = WideStreamSize.h;
        pWideSRZ->crop_x = 0;
        pWideSRZ->crop_y = 0;
        pWideSRZ->crop_floatX = pWideSRZ->crop_floatY = 0;
        // config tele crop
        pTeleSRZ->crop_w = ((int)(TeleCrop.w * RRZOFactor)) & (~0x01);
        pTeleSRZ->crop_h = ((int)(TeleCrop.h * RRZOFactor)) & (~0x01);
        pTeleSRZ->crop_x = (TeleStreamSize.w - pTeleSRZ->crop_w) / 2;
        pTeleSRZ->crop_y = (TeleStreamSize.h - pTeleSRZ->crop_h) / 2;
        pTeleSRZ->crop_floatX = pTeleSRZ->crop_floatY = 0;
        MY_LOGD("RRZOFactor : %f, crop(%d, %d)", RRZOFactor, TeleCrop.w, TeleCrop.h);
    }
    MY_LOGD("Wide SRZ info, in(%d,%d), out(%d,%d), crop(%d,%d,%d,%d)", pWideSRZ->in_w, pWideSRZ->in_h,
        pWideSRZ->out_w, pWideSRZ->out_h, pWideSRZ->crop_x, pWideSRZ->crop_y, (int)pWideSRZ->crop_w, (int)pWideSRZ->crop_h);
    MY_LOGD("Tele SRZ info, in(%d,%d), out(%d,%d), crop(%d,%d,%d,%d)", pTeleSRZ->in_w, pTeleSRZ->in_h,
        pTeleSRZ->out_w, pTeleSRZ->out_h, pTeleSRZ->crop_x, pTeleSRZ->crop_y, (int)pTeleSRZ->crop_w, (int)pTeleSRZ->crop_h);
    return MTRUE;
#else
    return MTRUE;
#endif
}

} // namespace NSCam
