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
#define LOG_TAG "MtkCam/ParamsManagerV3"


#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <math.h>

#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>  // For property_get().

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace android;

#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>

#include <camera_custom_eis.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <mtkcam/feature/DualCam/FOVHal.h>
#endif
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1) || (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT==1)
#define DUAL_CAM 1
#else
#define DUAL_CAM 0
#endif
using namespace NSCamHW;

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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

 /******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGD1(...)           MY_LOGD_IF(1 <= mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)           MY_LOGD_IF(2 <= mLogLevel, __VA_ARGS__)
/******************************************************************************
 *
 ******************************************************************************/
class ParamsManagerV3Imp
        : public IParamsManagerV3
{

public:     ////                    Interface.
    //
    virtual bool                       init();
    //
    virtual bool                       uninit();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes .
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual char const*                getName()    const {return mName.string();}
    virtual int32_t                    getOpenId()   const {return mi4OpenId;}
    virtual sp<IParamsManager>         getParamsMgr()   const {return mpParamsMgr;}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////     Operations .
    virtual status_t                updateRequest(IMetadata *request, MINT32 sensorMode, MINT32 templateId);
    virtual status_t                updateRequestHal(IMetadata *request, MINT32 sensorMode, MBOOL isCapture);
    virtual status_t                updateRequestJpeg(IMetadata *request)   const;
    virtual status_t                updateRequestPreview(IMetadata *request)   const;
    virtual status_t                updateRequestRecord(IMetadata *request)   const;
    virtual status_t                updateRequestVSS(IMetadata *request)   const;
    virtual status_t                updateRequestEng(IMetadata *request)      const;

    //override some tags to static data of MetaDataProvider
    virtual status_t                setStaticData();
    virtual status_t                restoreStaticData();

    virtual void                    setPreviewMaxFps(MUINT32 fps) {Mutex::Autolock _l(mMutex); mPreviewMaxFps = fps;}
    virtual MUINT32                 getPreviewMaxFps() {Mutex::Autolock _l(mMutex); return mPreviewMaxFps;}
    virtual status_t                calculateCropRegion(MINT32 sensorMode);
    virtual status_t                getCropRegion(
                                        MSize& sensorSize,
                                        MRect& activeArrayCrop,
                                        MRect& sensorSizeCrop,
                                        MRect& activeArrayPreviewCrop,
                                        MRect& sensorSizePreviewCrop
                                    ) const;
    virtual status_t                getCropRegion(
                                        MSize& sensorSize,
                                        MRect& activeArrayCrop,
                                        MRect& sensorSizeCrop,
                                        MRect& activeArrayPreviewCrop,
                                        MRect& sensorSizePreviewCrop,
                                        MSize& margin
                                    ) const;
    virtual MRect                   getAAAPreviewCropRegion() const;
    virtual status_t                getCapCropRegion(
                                        MRect& activeArrayCrop,
                                        MRect& sensorSizeCrop,
                                        MRect& sensorSizePreviewCrop
                                    ) const;
    virtual int                     getZoomRatio();
    virtual void                    setCancelAF(bool bCancelAF){Mutex::Autolock _l(mMutex); mbCancelAF = bCancelAF;}
    virtual bool                    getCancelAF() const {Mutex::Autolock _l(mMutex); return mbCancelAF;}

    virtual void                    setAfTriggered(bool bAfTriggered){Mutex::Autolock _l(mMutex); mbAfTriggered = bAfTriggered;}
    virtual bool                    getAfTriggered() const {Mutex::Autolock _l(mMutex); return mbAfTriggered;}
    virtual void                    setForceTorch(bool bForceTorch){Mutex::Autolock _l(mMutex); mbForceTorch = bForceTorch;}
    virtual void                    setControlCALTM(bool bEnableControlCALTM, bool bEnableCALTM){
                                        Mutex::Autolock _l(mMutex);
                                        mbEnableControlCALTM = bEnableControlCALTM;
                                        mbEnableCALTM = bEnableCALTM;
                                    }

public:     //// scenario related control
    virtual void                    setScenario(int32_t scenario) {
                                        Mutex::Autolock _l(mMutex);
                                        mScenario = scenario;
                                    }
    virtual void                    updateBasedOnScenario(IMetadata* meta, MBOOL& bRepeat);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

public:
                                    ParamsManagerV3Imp(
                                        String8 const& rName,
                                        MINT32 const cameraId,
                                        sp<IParamsManager> pParamsMgr
                                    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual                         ~ParamsManagerV3Imp(){};

    int                             wbModeStringToEnum(const char *wbMode);

    int                             effectModeStringToEnum(const char *effectMode);

    int                             abModeStringToEnum(const char *abMode);

    int                             sceneModeStringToEnum(const char *sceneMode);

    int                             decideFOVMargin(MINT32 sensorMode, MINT32 ZoomRatio, MSize SensorSize, MRect& SensorCrop, MSize& Margin);

    enum flashMode_t {
        FLASH_MODE_OFF     = 0,
        FLASH_MODE_AUTO,
        FLASH_MODE_ON,
        FLASH_MODE_TORCH,
        FLASH_MODE_RED_EYE,
        FLASH_MODE_INVALID = -1
    };

    flashMode_t                     flashModeStringToEnum(const char *flashMode);


    enum focusMode_t {
        FOCUS_MODE_OFF                = MTK_CONTROL_AF_MODE_OFF,
        FOCUS_MODE_AUTO               = MTK_CONTROL_AF_MODE_AUTO,
        FOCUS_MODE_MACRO              = MTK_CONTROL_AF_MODE_MACRO,
        FOCUS_MODE_CONTINUOUS_VIDEO   = MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO,
        FOCUS_MODE_CONTINUOUS_PICTURE = MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
        FOCUS_MODE_EDOF               = MTK_CONTROL_AF_MODE_EDOF,
        FOCUS_MODE_INFINITY,
        FOCUS_MODE_FIXED,
        FOCUS_MODE_INVALID            = -1
    };

    focusMode_t                     focusModeStringToEnum(const char *focusMode);

    bool                            boolFromString(const char *boolStr) const;

    struct my_params
    {
        my_params()
            : mWbMode(0)
            , mEffectMode(0)
            , mAntibandingMode(0)
            , mSceneMode(0)
            , mParamFlashMode(FLASH_MODE_OFF)
            , mParamFocusMode(FOCUS_MODE_OFF)
            , mUseParmsFlashMode(false)
            , mUseParmsFocusMode(false)
            , mFlashMode(0)
            , mFocusMode(0)
            , mVideoStabilization(false)
            , mExposureCompensation(0)
            , mAutoExposureLock(0)
            , mAutoWhiteBalanceLock(0)
            , mZoom(0)
            , mJpegQuality(0)
            , mJpegThumbQuality(0)
            , mJpegRotation(0)
            , mGgpsEnabled(0)
            , mGpsTimestamp(0)
            , mGpsProcessingMethod("GPS")
            {
                mPreviewFpsRange[0] = 0;
                mPreviewFpsRange[1] = 0;
                mJpegThumbSize[0]   = 0;
                mJpegThumbSize[1]   = 0;
                mGpsCoordinates[0]  = 0;
                mGpsCoordinates[1]  = 0;
                mGpsCoordinates[2]  = 0;
            };
        //
        MINT32                          mWbMode;
        MUINT8                          mEffectMode;
        MUINT8                          mAntibandingMode;
        MINT32                          mSceneMode;
        MINT32                          mPreviewFpsRange[2];
        flashMode_t                     mParamFlashMode;
        focusMode_t                     mParamFocusMode;
        bool                            mUseParmsFlashMode;
        bool                            mUseParmsFocusMode;

        MINT32                          mFlashMode;
        MINT32                          mFocusMode;

        bool                            mVideoStabilization;

        MINT32                          mExposureCompensation;
        bool                            mAutoExposureLock;
        bool                            mAutoWhiteBalanceLock;
        int                             mZoom;
        Vector<int>                     mRatio;

        // jpeg related
        MINT32                          mJpegThumbSize[2];
        MUINT8                          mJpegQuality, mJpegThumbQuality;
        MINT32                          mJpegRotation;

        bool                            mGgpsEnabled;
        MDOUBLE                         mGpsCoordinates[3];
        MINT64                          mGpsTimestamp;
        String8                         mGpsProcessingMethod;
    };

    struct eng_params
    {
        MINT32                           mManualFpsRange[2]; //Min/max
    };

    struct eis_params
    {
         MUINT32                         mEISEnabled;
    };

    virtual MERROR                  set(/*MtkCameraParameters const& paramString, */my_params& params);

    // Update passed-in request for common parameters
    virtual MERROR                  updateRequest(IMetadata *request, MINT32 sensorMode, my_params const& params);

    // Add/update JPEG entries in metadata
    virtual MERROR                  updateRequestJpeg(IMetadata *request, my_params const& params) const;
    //static const unsigned int       NUM_ZOOM_STEPS = 100;

//Eng part
    virtual MERROR                  setEng(/*MtkCameraParameters const& paramString, */eng_params& params) const;

    // Update passed-in request for common parameters
    virtual MERROR                  updateEngRequest(IMetadata *request, eng_params const& params) const;

    virtual MERROR                  setEIS(/*MtkCameraParameters const& paramString, */eis_params& params) const;

    // Update passed-in request for EIS preview parameters
    virtual MERROR                  updatePreviewEISRequest(IMetadata *request, eis_params const& params) const;

    // Update passed-in request for EIS record parameters
    virtual MERROR                  updateRecordEISRequest(IMetadata *request, eis_params const& params) const;

    // Update passed-in request for HDR detection parameters
    virtual MERROR                  updateHDRDetectionRequest(IMetadata *request) const;

    virtual void                    getPreviewFpsRange(MINT32& minFps, MINT32& maxFps);
    virtual void                    setPreviewFpsRange(MINT32& minFps, MINT32& maxFps);
    //
    virtual void                    getLastUpdateRequest(IMetadata& meta) {Mutex::Autolock _l(mMutex); meta = mLastUpdateRequestMetadata;}

private:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    String8 const                   mName;
    MINT32                          mLogLevel;
    MINT32                          mi4OpenId;

    // 3dnr on/off parameter
    MINT32                          m3dnr_mode;

    //
    MINT                            mPreviewMaxFps;
    MINT                            mDefaultPreviewFps;
    MINT                            mDefaultPreviewFpsRange[2];

    // static info
    MSize                           mSensorSize;
    MRect                           mActiveArrayCrop;
    MRect                           mSensorSizeCrop;
    MRect                           mActiveArrayPreviewCrop;
    MRect                           mSensorSizePreviewCrop;
    MSize                           mActiveMargin;
    MRect                           mActiveShiftCrop;
    MRect                           mSensorShiftCrop;
    float                           mCropScale;
    KeyedVector<MINT32, MRect>      mvSensorModeAaCrop;

    //check if cancelling AF focus locked after PreCapture
    bool                            mbCancelAF;
    bool                            mbAfTriggered;
    focusMode_t                     mOldFocusMode;

    // scenario
    int32_t                         mScenario;

    int                             mZoomRatio;

    //MZAF info
    MUINT8                          mMZAFEnable;

    // last update sensor mode
    MINT32                          mLastUpdateSensorMode;

    // ConvertParamImp   params;
    sp<android::IParamsManager>     mpParamsMgr;

    // Need torch or not when entering record
    MBOOL                           mNeedFlashLight;

    MBOOL                           mbForceTorch;
    MINT32                          mSupportDRE;
    MBOOL                           mbEnableControlCALTM;
    MBOOL                           mbEnableCALTM;

    mutable Mutex                   mMutex;

    //
    MUINT32                         mCropShift;

    // debug set Flash-On as Torch
    MINT32                          mFlashAsTorch;
    //
    MINT32                          mPreviewFpsRange[2];
    //
    IMetadata                       mLastUpdateRequestMetadata;

    // color space
    MINT32                          mIspColorSpace;
    //
    MRect                           mPreviewCropRegion;
};

/******************************************************************************
 *
 ******************************************************************************/
static int parse(const char *str, int *first/*, char delim*/, char **endptr = NULL)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);

    *first = w;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static void parseSizesList(const char *sizesStr, Vector<int> &values)
{
    MY_LOGD_IF(0,"%s\n", sizesStr);
    if (sizesStr == 0) {
        return;
    }

    char *sizeStartPtr = (char *)sizesStr;

    while (true) {
        int ivalue;
        int success = parse(sizeStartPtr, &ivalue, &sizeStartPtr);
        if (success == -1 || (*sizeStartPtr != ',' && *sizeStartPtr != '\0')) {
            MY_LOGE("Picture sizes string \"%s\" contains invalid character.", sizesStr);
            return;
        }
        values.push(ivalue);

        if (*sizeStartPtr == '\0') {
            return;
        }
        sizeStartPtr++;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ParamsManagerV3Imp::
ParamsManagerV3Imp(
    String8 const& rName,
    MINT32 cameraId,
    sp<android::IParamsManager> pParamsMgr
)
    : IParamsManagerV3()
    , mName(rName)
    , mi4OpenId(cameraId)
    , m3dnr_mode(MTK_NR_FEATURE_3DNR_MODE_OFF)
    , mPreviewMaxFps(0)
    , mActiveArrayCrop(MRect(MPoint(0,0),MSize(0,0)))
    , mSensorSizeCrop(MRect(MPoint(0,0),MSize(0,0)))
    , mActiveArrayPreviewCrop(MRect(MPoint(0,0),MSize(0,0)))
    , mSensorSizePreviewCrop(MRect(MPoint(0,0),MSize(0,0)))
    , mCropScale(0.0)
    , mbCancelAF(false)
    , mbAfTriggered(false)
    , mOldFocusMode(FOCUS_MODE_OFF)
    , mScenario(eScenario_define_start)
    , mMZAFEnable(0)
    , mLastUpdateSensorMode(-1)
    , mpParamsMgr(pParamsMgr)
    , mNeedFlashLight(false)
    , mbForceTorch(MFALSE)
    , mSupportDRE(property_get_int32("camera.mdp.dre.enable", 0))
    , mbEnableControlCALTM(MFALSE)
    , mbEnableCALTM(MFALSE)
    , mCropShift(property_get_int32( "debug.camera.cropshift" , 2))
    , mFlashAsTorch(property_get_int32("debug.camera.flashastorch", 0))
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.paramsv3", 0);
    }
    //
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4OpenId);
    if( ! pMetadataProvider.get() ) {
            MY_LOGE("pMetadataProvider.get() is NULL");
    }

    //Set default value
    mpParamsMgr->getDefaultPreviewFps(mDefaultPreviewFps);
    mpParamsMgr->getDefaultPreviewFpsRange(mDefaultPreviewFpsRange[0],mDefaultPreviewFpsRange[1]);
    mZoomRatio = 0;
    const int kFpsToApiScale = 1000;
    mDefaultPreviewFpsRange[0] /= kFpsToApiScale;
    mDefaultPreviewFpsRange[1] /= kFpsToApiScale;
    MY_LOGD1("FPS:%d,%d-%d",
            mDefaultPreviewFps,
            mDefaultPreviewFpsRange[0],
            mDefaultPreviewFpsRange[1]);
    if(mCropShift > 0)
    {
        MY_LOGD("Force to crop %d",mCropShift);
    }
    if(mFlashAsTorch != 0) {
        MY_LOGD("Force Flash as Torch %d",mFlashAsTorch);
    }
    //
    mPreviewFpsRange[0] = mDefaultPreviewFpsRange[0];
    mPreviewFpsRange[1] = mDefaultPreviewFpsRange[1];
    //
#if (MTKCAM_ISP_SUPPORT_COLOR_SPACE!=0)
    MINT32 debugIspColorSpace = ::property_get_int32("debug.isp.colorspace", -1);
    if ( debugIspColorSpace < 0 ) {
        mIspColorSpace = MTK_ISP_COLOR_SPACE_DISPLAY_P3;
        MY_LOGD("MTK_ISP_SUPPORT_COLOR_SPACE(%d)",mIspColorSpace);
    }
    else
    {
        mIspColorSpace = debugIspColorSpace;
        MY_LOGD("force to set mIspColorSpace to (%d)",mIspColorSpace);
    }
#else
    MINT32 debugIspColorSpace = ::property_get_int32("debug.isp.colorspace", -1);
    if ( debugIspColorSpace < 0 ) {
        mIspColorSpace = -1;
        MY_LOGD("MTK_ISP_SUPPORT_COLOR_SPACE not define, set mIspColorSpace to (-1)");
    }
    else
    {
        mIspColorSpace = debugIspColorSpace;
        MY_LOGD("MTK_ISP_SUPPORT_COLOR_SPACE not define, but force to set mIspColorSpace to (%d)",mIspColorSpace);
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IParamsManagerV3>
IParamsManagerV3::
createInstance(
    String8 const& rName,
    MINT32 cameraId,
    sp<android::IParamsManager> pParamsMgr
)
{
    return new ParamsManagerV3Imp(rName, cameraId, pParamsMgr);
}
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
        MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}
/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
inline MVOID
updateEntryArray(
    IMetadata* pMetadata,
    MUINT32 const tag,
    const T* array,
    MUINT32 size
)
{
    IMetadata::IEntry entry(tag);
    for (MUINT32 i = size; i != 0; i--)
    {
        entry.push_back(*array++, Type2Type< T >());
    }
    pMetadata->update(tag, entry);
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ParamsManagerV3Imp::
init()
{

    return true;

}

/******************************************************************************
 *
 ******************************************************************************/
bool
ParamsManagerV3Imp::
uninit()
{

    return true;

}


/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestJpeg(
    IMetadata */*request*/
) const
{
#warning [TODO]
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestPreview(
    IMetadata *request
) const
{
    eis_params eisparams;
    setEIS(eisparams);
    updatePreviewEISRequest(request, eisparams);
    updateHDRDetectionRequest(request);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestRecord(
    IMetadata *request
) const
{
    eis_params eisparams;
    setEIS(eisparams);
    updateRecordEISRequest(request, eisparams);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestVSS(
    IMetadata *request
) const
{
    eis_params eisparams;
    setEIS(eisparams);
    updateRecordEISRequest(request, eisparams);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestEng(
    IMetadata *request
) const
{
    //update params of eng part
    eng_params engparams;
    setEng(engparams);
    updateEngRequest(request, engparams);

    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequest(
   IMetadata *request,
   MINT32     sensorMode,
   MINT32     templateId = -1
)
{
    if ( templateId != -1 ) {
        ITemplateRequest* obj = NSTemplateRequestManager::valueFor(mi4OpenId);
        if(obj == NULL) {
            obj = ITemplateRequest::getInstance(mi4OpenId);
            NSTemplateRequestManager::add(mi4OpenId, obj);
        }
        *request += obj->getMtkData(templateId);
    }
    //
    my_params params;
    set(params);
    mLastUpdateSensorMode = sensorMode;
    updateRequest(request, sensorMode, params);
    updateRequestJpeg(request, params);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
updateRequestHal(
    IMetadata *request,
    MINT32     sensorMode,
    MBOOL      isCapture
)
{
    // for 3A
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_ISO_SPEED      , PARAMSMANAGER_MAP_INST(eMapIso)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_ISO_SPEED)));
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_BRIGHTNESS_MODE, PARAMSMANAGER_MAP_INST(eMapLevel)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_BRIGHTNESS)));
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_CONTRAST_MODE  , PARAMSMANAGER_MAP_INST(eMapLevel)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_CONTRAST)));
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_HUE_MODE       , PARAMSMANAGER_MAP_INST(eMapLevel)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_HUE)));
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_EDGE_MODE      , PARAMSMANAGER_MAP_INST(eMapLevel)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_EDGE)));
    updateEntry<MINT32>(request, MTK_HAL_REQUEST_SATURATION_MODE, PARAMSMANAGER_MAP_INST(eMapLevel)->valueFor(mpParamsMgr->getStr(MtkCameraParameters::KEY_SATURATION)));
    //
    MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
    MSize sensorSize;
    calculateCropRegion(sensorMode);
    getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
    updateEntry<MRect>(request , MTK_3A_PRV_CROP_REGION , reqPreviewCropRegion);
    //
    {
        Mutex::Autolock _l(mMutex);
        mPreviewCropRegion = reqPreviewCropRegion;
    }
    //
    MBOOL bUpdateSensorModeAaCrop = MFALSE;
    MRect sensorModeAaCrop;
    if( mvSensorModeAaCrop.indexOfKey(sensorMode) >= 0 )
    {
        sensorModeAaCrop = mvSensorModeAaCrop.valueFor(sensorMode);
        bUpdateSensorModeAaCrop = MTRUE;
    }
    else
    {
        MRect sensorRect = MRect(0,0);
        HwInfoHelper hwInfoHelper(getOpenId());
        if( !hwInfoHelper.updateInfos() ||
            !hwInfoHelper.getSensorSize(sensorMode, sensorRect.s) )
        {
            MY_LOGE("updateInfos or getSensorSize fail");
        }
        else
        {
            HwTransHelper hwTransHelper(getOpenId());
            HwMatrix matToActive;
            if( hwTransHelper.getMatrixToActive(sensorMode, matToActive) )
            {
                matToActive.transform(sensorRect, sensorModeAaCrop);
                mvSensorModeAaCrop.add(sensorMode, sensorModeAaCrop);
                bUpdateSensorModeAaCrop = MTRUE;
            }
            else
            {
                MY_LOGE("get matrix fail");
            }
        }
    }
    if( bUpdateSensorModeAaCrop )
    {
        updateEntry<MRect>(request , MTK_SENSOR_MODE_INFO_ACTIVE_ARRAY_CROP_REGION , sensorModeAaCrop);
        MY_LOGD1("sensor mode:%d, sensorModeAaCrop(%d,%d,%dx%d)"
                 , sensorMode, sensorModeAaCrop.p.x, sensorModeAaCrop.p.y, sensorModeAaCrop.s.w, sensorModeAaCrop.s.h);
    }
    // for MZAF
    updateEntry<MUINT8>(request , MTK_FOCUS_MZ_ON , mMZAFEnable);
    //
    {
        Mutex::Autolock _l(mMutex);
        MY_LOGD1("mSupportDRE(%d), mbEnableControlCALTM(%d), mbEnableCALTM(%d)", mSupportDRE, mbEnableControlCALTM, mbEnableCALTM);
        if( mSupportDRE != 0 && mbEnableControlCALTM )
        {
            updateEntry<MBOOL>(request , MTK_P2NODE_CTRL_CALTM_ENABLE, mbEnableCALTM);
        }
    }
    // for ISP color space
    if(mIspColorSpace >= 0)
    {
        //the condition need to update color space
        //for example: if(isCapture || mScenario == eScenario_Camera_Preview )
        //             //it means that only capture case or camera preview case need to update color space
        //now condition: camera preview/capture/video preview/video recording all need to update color space
        {
            IMetadata::setEntry<MINT32>(request, MTK_ISP_COLOR_SPACE, static_cast<MINT32>(mIspColorSpace));
        }
    }
    return OK;
}
/******************************************************************************
 * override some tags to static data of MetaDataProvider
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
setStaticData()
{
    IMetadata setting;
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4OpenId);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("pMetadataProvider.get() is NULL");
        return DEAD_OBJECT;
    }
    // set available scene modes
    String8 sa = mpParamsMgr->getStr(MtkCameraParameters::KEY_SUPPORTED_SCENE_MODES);
    const char* a = sa.string();
    const char* b;
    IMetadata::IEntry entryAvailableScene(MTK_CONTROL_AVAILABLE_SCENE_MODES);

    int len=sa.length();
    while( len > 0 )
    {
        b = strchr(a,',');
        if (b == 0)
        {
            String8 k(a, (size_t)(len));
            entryAvailableScene.push_back(sceneModeStringToEnum(k), Type2Type<MUINT8>());
            len=0;
            break;
        }
        else
        {
            String8 k(a, (size_t)(b-a));
            len-=((b-a)+1);
            a = b+1;
            entryAvailableScene.push_back(sceneModeStringToEnum(k), Type2Type<MUINT8>());
        }
    }
    setting.update(entryAvailableScene.tag(), entryAvailableScene);

    // set default value to scene mode overrides
    IMetadata::IEntry entrySceneModeOverride(MTK_CONTROL_SCENE_MODE_OVERRIDES);
    for (MUINT i = 0 ; i < entryAvailableScene.count() ; i++)
    {
      entrySceneModeOverride.push_back(MTK_CONTROL_AE_MODE_ON  , Type2Type<MUINT8>());
      entrySceneModeOverride.push_back(MTK_CONTROL_AWB_MODE_OFF, Type2Type<MUINT8>());
      entrySceneModeOverride.push_back(MTK_CONTROL_AF_MODE_OFF , Type2Type<MUINT8>());
    }
    setting.update(entrySceneModeOverride.tag(), entrySceneModeOverride);

    // set available focal lengths
    //updateEntry<MFLOAT>(&setting, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, mpParamsMgr->getFloat(MtkCameraParameters::KEY_FOCAL_LENGTH));

    // override AE exposure value steps
    MUINT32 ab = (MUINT32)(1.0f / mpParamsMgr->getFloat(MtkCameraParameters::KEY_EXPOSURE_COMPENSATION_STEP));
    updateEntry<MRational >(&setting, MTK_CONTROL_AE_COMPENSATION_STEP,MRational(1, ab));
    // save to MetadataProvider
    pMetadataProvider->setStaticData(&setting);

  return OK;
}

/******************************************************************************
 * override some tags to static data of MetaDataProvider
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
restoreStaticData()
{
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4OpenId);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("pMetadataProvider.get() is NULL");
        return DEAD_OBJECT;
    }
    pMetadataProvider->restoreStaticData();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
set(/*MtkCameraParameters const& paramString, */my_params& params) {

    MtkCameraParameters newParams;// = paramString;

    newParams.unflatten(mpParamsMgr->flatten());

    newParams.getPreviewFpsRange(
                &params.mPreviewFpsRange[0],
                &params.mPreviewFpsRange[1]);
    const int kFpsToApiScale = 1000;
    params.mPreviewFpsRange[0] /= kFpsToApiScale;
    params.mPreviewFpsRange[1] /= kFpsToApiScale;
    //
    if( mDefaultPreviewFpsRange[0] != params.mPreviewFpsRange[0] ||
        mDefaultPreviewFpsRange[1] != params.mPreviewFpsRange[1])
    {
        MY_LOGD("Use preview-fps-range");
    }
    else
    if( (mDefaultPreviewFps != newParams.getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE)) ||
        (mpParamsMgr->getRecordingHint() && mpParamsMgr->getVideoStabilization()))
    {
        MY_LOGD("Use preview-frame-rate");
        MUINT32 previewFps = newParams.getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE);
        MUINT32 fixedFps   = ( !mpParamsMgr->getRecordingHint() && (previewFps > 30) ) ? 30 : previewFps;
        MBOOL isDynamicFps = ( ::strcmp(newParams.get(MtkCameraParameters::KEY_DYNAMIC_FRAME_RATE), CameraParameters::TRUE) == 0 ) ? MTRUE : MFALSE;
        MBOOL isSlowMotion = ( mpParamsMgr->getRecordingHint() && (previewFps > 30) ) ? MTRUE : MFALSE;
        //
        {
          Mutex::Autolock _l(mMutex);
          if(mPreviewMaxFps > 0)
          {
              if(static_cast<int>(previewFps) > mPreviewMaxFps)
              {
                  previewFps = mPreviewMaxFps;
              }
          }
        }
        //
        if(isSlowMotion)
        {
            params.mPreviewFpsRange[0] = previewFps;
            params.mPreviewFpsRange[1] = previewFps;
        }
        else
        {
            if(mpParamsMgr->getRecordingHint() && mpParamsMgr->getVideoStabilization())
            {
                MINT32 videoWidth = 0,videoHeight = 0;

                if (EISCustom::isEnabledFixedFPS())
                {
                    params.mPreviewFpsRange[0] = fixedFps;
                    params.mPreviewFpsRange[1] = fixedFps;
                }else
                {
                    params.mPreviewFpsRange[0] = 5;
                    params.mPreviewFpsRange[1] = previewFps;
                }

                mpParamsMgr->getVideoSize(&videoWidth,&videoHeight);

                if( (videoWidth*videoHeight) >= (VR_UHD_W*VR_UHD_H) )
                {
                    // Get EIS Mode
                    MUINT32 eisMode  = 0;
                    MUINT32 eis_mask = EISCustom::USAGE_MASK_4K2K;
                    if( mpParamsMgr->getVHdr() != SENSOR_VHDR_MODE_NONE )
                    {
                        eis_mask |= EISCustom::USAGE_MASK_VHDR;
                    }
                    eisMode = EISCustom::getEISMode(eis_mask);
                    if( EIS_MODE_IS_EIS_30_ENABLED(eisMode) ||
                        EIS_MODE_IS_EIS_25_ENABLED(eisMode) ||
                        EIS_MODE_IS_EIS_22_ENABLED(eisMode) )
                    {
                        if (EISCustom::isEnabledFixedFPS())
                        {
                            params.mPreviewFpsRange[0] = EISCustom::get4K2KRecordFPS();
                        }else
                        {
                            params.mPreviewFpsRange[0] = 5;
                        }
                        params.mPreviewFpsRange[1] = EISCustom::get4K2KRecordFPS();
                    }
                }
                MY_LOGD("EIS VR FPS Max/Min (%d/%d)",params.mPreviewFpsRange[0],params.mPreviewFpsRange[1]);
            }else if(isDynamicFps)
            {
                params.mPreviewFpsRange[0] = 5;
                params.mPreviewFpsRange[1] = previewFps;
            }else
            {
                params.mPreviewFpsRange[0] = fixedFps;
                params.mPreviewFpsRange[1] = fixedFps;
            }
        }
    }
    else
    {
        MY_LOGD("Use default FPS");
    }

    //
    MUINT32 forceFPS = ::property_get_int32("debug.forceFPS.enable", 0);
    if(forceFPS > 0)
    {
        MY_LOGD("forceFPS(%d)", forceFPS);
        params.mPreviewFpsRange[0] = forceFPS;
        params.mPreviewFpsRange[1] = forceFPS;
    }
    ////////////////////////////////////////////////////
    // for IMX519 customized flow:
    // preview 30fps but capture need change to 60fps
    // if app set fps bigger than 30 but no recording hint
    // to forced reset the max fps as 30
    int limitFps = 30;
    if(mpParamsMgr->getNonSlowmotionPreviewLimitMaxFps(&limitFps) &&
       !mpParamsMgr->getRecordingHint())
    {
        if(params.mPreviewFpsRange[0] > limitFps)
        {
            MY_LOGD("Min FPS(%d) bigger than %d, need to reset as %d",params.mPreviewFpsRange[0],limitFps,limitFps);
            params.mPreviewFpsRange[0] = limitFps;
        }
        if(params.mPreviewFpsRange[1] > limitFps)
        {
            MY_LOGD("Max FPS(%d) bigger than %d, need to reset as %d",params.mPreviewFpsRange[1],limitFps,limitFps);
            params.mPreviewFpsRange[1] = limitFps;
        }
    }
    ////////////////////////////////////////////////////
    MY_LOGD("FPS:Min(%d),Max(%d)",
            params.mPreviewFpsRange[0],
            params.mPreviewFpsRange[1]);
    //
    mPreviewFpsRange[0] = params.mPreviewFpsRange[0];
    mPreviewFpsRange[1] = params.mPreviewFpsRange[1];
    //

    // PREVIEW_FORMAT
    //validatedParams.previewFormat =
    //        formatStringToEnum(newParams.getPreviewFormat());

    // PICTURE_SIZE
    //newParams.getPictureSize(&validatedParams.pictureWidth,
    //        &validatedParams.pictureHeight);
    // JPEG_THUMBNAIL_WIDTH/HEIGHT
    params.mJpegThumbSize[0] =
            newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    params.mJpegThumbSize[1] =
            newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);

    // 3dnr on/off parameter check
    const char *pTmp = newParams.get(MtkCameraParameters::KEY_3DNR_MODE );
    if (pTmp != NULL)
    {
        if (strcmp(pTmp, "on") == 0)
            this->m3dnr_mode = MTK_NR_FEATURE_3DNR_MODE_ON;
        else
            this->m3dnr_mode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    }
    else
    {
        this->m3dnr_mode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    }
    MY_LOGD("mkdbg: Requested 3DNR-mode: val: %d", this->m3dnr_mode);

    // JPEG_THUMBNAIL_QUALITY
    int quality = newParams.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
    // also makes sure quality fits in uint8_t
    if (quality < 0 || quality > 100) {
        MY_LOGE("Requested JPEG thumbnail quality %d is not supported",
                 quality);
    }
    params.mJpegThumbQuality = quality;

    // JPEG_QUALITY
    quality = newParams.getInt(CameraParameters::KEY_JPEG_QUALITY);
    // also makes sure quality fits in uint8_t
    if (quality < 0 || quality > 100) {
        MY_LOGE("Requested JPEG quality %d is not supported",
                    quality);
    }
    params.mJpegQuality = quality;

    // ROTATION
    params.mJpegRotation =
            newParams.getInt(CameraParameters::KEY_ROTATION);
    if (params.mJpegRotation != 0 &&
            params.mJpegRotation != 90 &&
            params.mJpegRotation != 180 &&
            params.mJpegRotation != 270) {
        MY_LOGE("Requested picture rotation angle %d is not supported",
                params.mJpegRotation);
        return BAD_VALUE;
    }

    // GPS
    params.mGgpsEnabled = true;
    const char *gpsLatStr =
            newParams.get(CameraParameters::KEY_GPS_LATITUDE);
    if (gpsLatStr != NULL) {
        const char *gpsLongStr =
                newParams.get(CameraParameters::KEY_GPS_LONGITUDE);
        const char *gpsAltitudeStr =
                newParams.get(CameraParameters::KEY_GPS_ALTITUDE);
        const char *gpsTimeStr =
                newParams.get(CameraParameters::KEY_GPS_TIMESTAMP);
        const char *gpsProcMethodStr =
                newParams.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
        if (gpsProcMethodStr == NULL)
        {
            MY_LOGE("gpsProcMethodStr == NULL & set default to GPS");
        }
        if (gpsLongStr == NULL ||
                gpsAltitudeStr == NULL ||
                gpsTimeStr == NULL ||
                gpsProcMethodStr == NULL) {
            MY_LOGE("Incomplete set of GPS parameters provided");
            params.mGgpsEnabled = false;
        } else {
            char *endPtr;
            params.mGpsCoordinates[0] = strtod(gpsLatStr, &endPtr);
            if (endPtr == gpsLatStr) {
                MY_LOGE("Malformed GPS latitude: %s", gpsLatStr);
                params.mGgpsEnabled = false;
            }

            params.mGpsCoordinates[1] = strtod(gpsLongStr, &endPtr);
            if (endPtr == gpsLongStr) {
                ALOGE("Malformed GPS longitude: %s", gpsLongStr);
                params.mGgpsEnabled = false;
            }

            params.mGpsCoordinates[2] = strtod(gpsAltitudeStr, &endPtr);
            if (endPtr == gpsAltitudeStr) {
                ALOGE("Malformed GPS altitude: %s", gpsAltitudeStr);
                params.mGgpsEnabled = false;
            }

            params.mGpsTimestamp = strtoll(gpsTimeStr, &endPtr, 10);
            if (endPtr == gpsTimeStr) {
                ALOGE("Malformed GPS timestamp: %s", gpsTimeStr);
                params.mGgpsEnabled = false;
            }

            if ( params.mGgpsEnabled )
                params.mGpsProcessingMethod = gpsProcMethodStr;
        }
    } else {
        params.mGgpsEnabled = false;
    }


    // EFFECT
    params.mEffectMode = effectModeStringToEnum(
        newParams.get(CameraParameters::KEY_EFFECT) );
    MY_LOGD1("Effect mode = %s", newParams.get(CameraParameters::KEY_EFFECT));
    MY_LOGD1("mEffectMode = %d", params.mEffectMode);

    // ANTIBANDING
    params.mAntibandingMode = abModeStringToEnum(
        newParams.get(CameraParameters::KEY_ANTIBANDING) );
    MY_LOGD1("mAntibandingMode = %d", params.mAntibandingMode);

    // SCENE_MODE
    params.mSceneMode = sceneModeStringToEnum(
        newParams.get(CameraParameters::KEY_SCENE_MODE) );
    MY_LOGD1("Scene mode = %s", newParams.get(CameraParameters::KEY_SCENE_MODE));
    MY_LOGD1("Scene mode = %d", params.mSceneMode);

    // FLASH_MODE
    params.mUseParmsFlashMode = true;
    {
        Mutex::Autolock _l(mMutex);
        if( mbForceTorch )
        {
            params.mParamFlashMode = flashModeStringToEnum(CameraParameters::FLASH_MODE_TORCH);
            MY_LOGD("set force torch on");
        }
        else
        {
            params.mParamFlashMode = flashModeStringToEnum(newParams.get(CameraParameters::KEY_FLASH_MODE));
        }
    }
    params.mFlashMode = MTK_CONTROL_AE_MODE_ON_AUTO_FLASH;

    // WHITE_BALANCE
    params.mWbMode = wbModeStringToEnum(
            newParams.get(CameraParameters::KEY_WHITE_BALANCE) );

    // FOCUS_MODE
    params.mUseParmsFocusMode = true;
    params.mParamFocusMode = focusModeStringToEnum(newParams.get(CameraParameters::KEY_FOCUS_MODE));
    if( mOldFocusMode != params.mParamFocusMode )
    {
        setAfTriggered(MFALSE);
        mOldFocusMode = params.mParamFocusMode;
    }
    params.mFocusMode = MTK_CONTROL_AF_MODE_OFF;

    // EXPOSURE_COMPENSATION
    params.mExposureCompensation =
        newParams.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);

    // AUTO_EXPOSURE_LOCK (always supported)
    params.mAutoExposureLock = boolFromString(
        newParams.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK));

    // AUTO_WHITEBALANCE_LOCK (always supported)
    params.mAutoWhiteBalanceLock = boolFromString(
        newParams.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK));

    // FOCUS_AREAS
    //res = parseAreas(newParams.get(CameraParameters::KEY_FOCUS_AREAS),
    //        &validatedParams.focusingAreas);
    // METERING_AREAS
    // res = parseAreas(newParams.get(CameraParameters::KEY_METERING_AREAS),
    //        &validatedParams.meteringAreas);

    // ZOOM
    params.mZoom = newParams.getInt(CameraParameters::KEY_ZOOM);
    MY_LOGD1("mZoom = %d", params.mZoom);

    // ZOOM
    const char *zoomRatios = newParams.get(CameraParameters::KEY_ZOOM_RATIOS);
    parseSizesList(zoomRatios, params.mRatio);
    MY_LOGD1("mRatio = %s ", zoomRatios);
    MY_LOGD1("mRatio size = %zu ", params.mRatio.size());
    for(size_t ii = 0; ii < params.mRatio.size(); ii++) {
        MY_LOGD1("%d, ", params.mRatio[ii]);
    }
    //MY_LOGD("mRatio = %d", validatedParams.mRatio);
    // VIDEO_SIZE
    //newParams.getVideoSize(&validatedParams.videoWidth,
    //        &validatedParams.videoHeight);

    // VIDEO_STABILIZATION
    params.mVideoStabilization = boolFromString(
        newParams.get(CameraParameters::KEY_VIDEO_STABILIZATION) );

    // LIGHTFX
    //validatedParams.lightFx = lightFxStringToEnum(
    //    newParams.get(CameraParameters::KEY_LIGHTFX));
    //*this = validatedParams;

    // Multi-AF window Enable
    mMZAFEnable = 0;
    MBOOL isMzafEnable = MFALSE;
    const char *pTmpMzafEnable = newParams.get(MtkCameraParameters::KEY_MZAF_ENABLE );
    if (pTmpMzafEnable != NULL)
    {
        isMzafEnable = ( ::strcmp( pTmpMzafEnable, CameraParameters::TRUE) == 0 ) ? MTRUE : MFALSE;
    }
    else
    {
        MY_LOGW("KEY_MZAF_ENABLE no value");
    }

    MBOOL isMzafSupport = MFALSE;
    const char *pTmpMzafSupport = newParams.get(MtkCameraParameters::KEY_IS_SUPPORT_MZAF );
    if (pTmpMzafSupport != NULL)
    {
        isMzafSupport = ( ::strcmp( pTmpMzafSupport, CameraParameters::TRUE) == 0 ) ? MTRUE : MFALSE;
    }
    else
    {
        MY_LOGW("KEY_IS_SUPPORT_MZAF no value");
    }

    if (isMzafEnable && isMzafSupport)
    {
        mMZAFEnable = 1;
    }
    MINT32 forceEnableMZAF = ::property_get_int32("debug.enablemzaf.enable", -1);
    if(forceEnableMZAF!=-1)
    {
        if(forceEnableMZAF>0)
        {
            mMZAFEnable = 1;
        }
        else
        {
            mMZAFEnable = 0;
        }
    }
    MY_LOGD("MZAF Enable(%d)",mMZAFEnable);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updateRequest(IMetadata *request, MINT32 sensorMode, my_params const& params)
{
    IMetadata::IEntry entry =
            request->entryFor(MTK_CONTROL_CAPTURE_INTENT);
    if ( !entry.isEmpty() ) {
        updateEntryArray<MINT32>(request, MTK_CONTROL_AE_TARGET_FPS_RANGE, params.mPreviewFpsRange, 2);
    } else {
        MY_LOGE("setting does not contain capture intent. Something wrong.");
    }

    MUINT8 reqWbLock = params.mAutoWhiteBalanceLock ?
            MTK_CONTROL_AWB_LOCK_ON : MTK_CONTROL_AWB_LOCK_OFF;
    updateEntry<MUINT8>(request , MTK_CONTROL_AWB_LOCK , reqWbLock);

    updateEntry<MUINT8>(request , MTK_CONTROL_EFFECT_MODE , params.mEffectMode);

    updateEntry<MUINT8>(request , MTK_CONTROL_AE_ANTIBANDING_MODE , params.mAntibandingMode);

    bool sceneModeActive =
            params.mSceneMode != (MUINT8)MTK_CONTROL_SCENE_MODE_DISABLED;
    MUINT8 reqControlMode = MTK_CONTROL_MODE_AUTO;
    if (sceneModeActive) {
        reqControlMode = MTK_CONTROL_MODE_USE_SCENE_MODE;
    }
    updateEntry<MUINT8>(request , MTK_CONTROL_MODE , reqControlMode);

    MUINT8 reqSceneMode =
            sceneModeActive ? params.mSceneMode :
            (MUINT8)MTK_CONTROL_SCENE_MODE_DISABLED;
    updateEntry<MUINT8>(request , MTK_CONTROL_SCENE_MODE , reqSceneMode);

    MUINT8 reqFlashMode = MTK_FLASH_MODE_OFF;
    MUINT8 reqAeMode = params.mFlashMode;
    if(params.mUseParmsFlashMode) {
        switch (params.mParamFlashMode) {
            case ParamsManagerV3Imp::FLASH_MODE_OFF:
                reqAeMode = MTK_CONTROL_AE_MODE_ON; break;
            case ParamsManagerV3Imp::FLASH_MODE_AUTO:
                reqAeMode = MTK_CONTROL_AE_MODE_ON_AUTO_FLASH; break;
            case ParamsManagerV3Imp::FLASH_MODE_ON:
                if(!mFlashAsTorch){
                    reqAeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH; break;
                }
            case ParamsManagerV3Imp::FLASH_MODE_TORCH:
                reqAeMode = MTK_CONTROL_AE_MODE_ON;
                reqFlashMode = MTK_FLASH_MODE_TORCH;
                break;
            case ParamsManagerV3Imp::FLASH_MODE_RED_EYE:
                reqAeMode = MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE; break;
            default:
                MY_LOGE("Unknown flash mode %d", params.mParamFlashMode);
                return BAD_VALUE;
        }
    }
    updateEntry<MUINT8>(request , MTK_FLASH_MODE , reqFlashMode);
    updateEntry<MUINT8>(request , MTK_CONTROL_AE_MODE , reqAeMode);

    MUINT8 reqAeLock = params.mAutoExposureLock ?
            MTK_CONTROL_AE_LOCK_ON : MTK_CONTROL_AE_LOCK_OFF;
    updateEntry<MUINT8>(request , MTK_CONTROL_AE_LOCK , reqAeLock);

    updateEntry<MUINT8>(request , MTK_CONTROL_AWB_MODE , params.mWbMode);

    float reqFocusDistance = -1; // infinity focus in diopters
    MUINT8 reqFocusMode = params.mFocusMode;
    if(params.mUseParmsFocusMode) {
        switch (params.mParamFocusMode) {
            case ParamsManagerV3Imp::FOCUS_MODE_OFF:
            case ParamsManagerV3Imp::FOCUS_MODE_AUTO:
            case ParamsManagerV3Imp::FOCUS_MODE_MACRO:
            case ParamsManagerV3Imp::FOCUS_MODE_CONTINUOUS_VIDEO:
            case ParamsManagerV3Imp::FOCUS_MODE_CONTINUOUS_PICTURE:
            case ParamsManagerV3Imp::FOCUS_MODE_EDOF:
                reqFocusMode = params.mParamFocusMode;
                break;
            case ParamsManagerV3Imp::FOCUS_MODE_INFINITY:
            case ParamsManagerV3Imp::FOCUS_MODE_FIXED:
                reqFocusMode = MTK_CONTROL_AF_MODE_OFF;
                break;
        default:
                MY_LOGE("Unknown mParamFocusMode mode %d", params.mParamFocusMode);
                return BAD_VALUE;
        }
    }
    updateEntry<MFLOAT>(request , MTK_LENS_FOCUS_DISTANCE , reqFocusDistance);

    updateEntry<MUINT8>(request , MTK_CONTROL_AF_MODE , reqFocusMode);

    updateEntry<MINT32>(request , MTK_CONTROL_AE_EXPOSURE_COMPENSATION , params.mExposureCompensation);
    //
    MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
    MSize sensorSize;
    calculateCropRegion(sensorMode);
    getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
    updateEntry<MRect>(request , MTK_SCALER_CROP_REGION , reqCropRegion);

    // 3A region
    {
        // MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
        // getCropRegion(reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
        // af region
        {
            int max = mpParamsMgr->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
            String8 const s = mpParamsMgr->getStr(CameraParameters::KEY_FOCUS_AREAS);
            List<camera_area_t> areas;
            if ( OK == mpParamsMgr->parseCamAreas( s.string(), areas, max ) ) {
                IMetadata::IEntry entry(MTK_CONTROL_AF_REGIONS);
                List<camera_area_t>::iterator it = areas.begin();
                while ( it != areas.end() ) {
                    int xmin = ((*it).left   + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                    int ymin = ((*it).top    + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                    int xmax = ((*it).right  + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                    int ymax = ((*it).bottom + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                    //
                    entry.push_back( xmin, Type2Type< MINT32 >());
                    entry.push_back( ymin, Type2Type< MINT32 >());
                    entry.push_back( xmax, Type2Type< MINT32 >());
                    entry.push_back( ymax, Type2Type< MINT32 >());
                    entry.push_back( (*it).weight, Type2Type< MINT32 >());
                    //
                    it++;
                }
                request->update(entry.tag(), entry);
            }
        }
        // ae / awb region
        {
            int max = mpParamsMgr->getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS);
            String8 const s = mpParamsMgr->getStr(CameraParameters::KEY_METERING_AREAS);
            List<camera_area_t> areas;
            if ( OK == mpParamsMgr->parseCamAreas( s.string(), areas, max ) ) {
                IMetadata::IEntry entry(MTK_CONTROL_AE_REGIONS);
                IMetadata::IEntry entry1(MTK_CONTROL_AWB_REGIONS);
                //
                List<camera_area_t>::iterator it = areas.begin();
                while ( it != areas.end() ) {
                    int xmin = ((*it).left   + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                    int ymin = ((*it).top    + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                    int xmax = ((*it).right  + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                    int ymax = ((*it).bottom + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                    //
                    entry.push_back( xmin, Type2Type< MINT32 >());
                    entry.push_back( ymin, Type2Type< MINT32 >());
                    entry.push_back( xmax, Type2Type< MINT32 >());
                    entry.push_back( ymax, Type2Type< MINT32 >());
                    entry.push_back( (*it).weight, Type2Type< MINT32 >());
                    //
                    entry1.push_back( xmin, Type2Type< MINT32 >());
                    entry1.push_back( ymin, Type2Type< MINT32 >());
                    entry1.push_back( xmax, Type2Type< MINT32 >());
                    entry1.push_back( ymax, Type2Type< MINT32 >());
                    entry1.push_back( (*it).weight, Type2Type< MINT32 >());
                    //
                    it++;
                }
                request->update(entry.tag(), entry);
                request->update(entry1.tag(), entry1);
            }
        }
    }
    //
    //Move to updateRequestRecordEIS and updateRequestPreviewEIS
    //uint8_t reqVstabMode = params.mVideoStabilization ?
    //        MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON :
    //        MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    //updateEntry<MUINT8>(request , MTK_CONTROL_VIDEO_STABILIZATION_MODE , reqVstabMode);

    // 3dnr on/off parameter
    //MY_LOGD("this->m3dnr_mode: %d", this->m3dnr_mode);
    //updateEntry<MINT32>(request , MTK_NR_FEATURE_3DNR_MODE, this->m3dnr_mode);
    //
    {
        Mutex::Autolock _l(mMutex);
        mLastUpdateRequestMetadata = *request;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updateRequestJpeg(IMetadata *request, my_params const& params) const
{
    status_t res = OK;

    //IMetadata::IEntry thumbSizeEntry(MTK_JPEG_THUMBNAIL_SIZE);
    //thumbSizeEntry.push_back(MSize(mJpegThumbSize[0],mJpegThumbSize[1]), Type2Type< MSize >());
    updateEntry<MSize>(request , MTK_JPEG_THUMBNAIL_SIZE , MSize(params.mJpegThumbSize[0], params.mJpegThumbSize[1]));
    //request->update(MTK_JPEG_THUMBNAIL_SIZE, thumbSizeEntry);

    updateEntry<MUINT8>(request , MTK_JPEG_THUMBNAIL_QUALITY , params.mJpegThumbQuality);

    updateEntry<MUINT8>(request , MTK_JPEG_QUALITY , params.mJpegQuality);

    updateEntry<MINT32>(request , MTK_JPEG_ORIENTATION , params.mJpegRotation);

    if (params.mGgpsEnabled) {
        updateEntryArray(request, MTK_JPEG_GPS_COORDINATES, params.mGpsCoordinates, 3);

        updateEntry<MINT64>(request , MTK_JPEG_GPS_TIMESTAMP , params.mGpsTimestamp);

        MUINT8 uGPSProcessingMethod[64]={0};
        for( size_t i = 0; i < params.mGpsProcessingMethod.size(); i++ ) {
            uGPSProcessingMethod[i] = *(params.mGpsProcessingMethod+i);
        }
        //uGPSProcessingMethod[63] = '\0'; //null-terminating
        updateEntryArray<MUINT8>(request , MTK_JPEG_GPS_PROCESSING_METHOD , uGPSProcessingMethod, 64);

    } /*else {
        res = request->remove(MTK_JPEG_GPS_COORDINATES);
        if (res != OK) return res;
        res = request->remove(MTK_JPEG_GPS_TIMESTAMP);
        if (res != OK) return res;
        res = request->remove(MTK_JPEG_GPS_PROCESSING_METHOD);
        if (res != OK) return res;
    }*/
    return res;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
setEIS(/*MtkCameraParameters const& paramString, */eis_params& params) const
{
    params.mEISEnabled = mpParamsMgr->getVideoStabilization();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
setEng(/*MtkCameraParameters const& paramString, */eng_params& params) const
{

    MtkCameraParameters newParams;// = paramString;

    newParams.unflatten(mpParamsMgr->flatten());

    String8 const s1 = mpParamsMgr->getStr(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED);
    MBOOL const isSupportedManualFps = ( ! s1.isEmpty() && s1 == CameraParameters::TRUE ) ? MTRUE : MFALSE;
    MBOOL const isEnableManualFps = ((mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_ENABLE))==1)? MTRUE : MFALSE;
    MUINT32 const manualLowFps = mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_LOW);
    MUINT32 const manualHighFps = mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_HIGH);

    if (isSupportedManualFps && isEnableManualFps)
    {
        //mbFixFps = true;
        //cam3aParam.i4MinFps = manualLowFps*1000;
        //cam3aParam.i4MaxFps = manualHighFps*1000;
        //
        //minFPS of video raw dump
        params.mManualFpsRange[0] = manualLowFps; // params.mManualFpsRange[0] = manualLowFps*1000;
        //MaxFPS of video raw dump
        params.mManualFpsRange[1] = manualHighFps; //params.mManualFpsRange[1] = manualHighFps*1000;
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updatePreviewEISRequest(IMetadata *request, eis_params const& /*params*/) const
{
    uint8_t reqVstabMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;

    //EIS shall not be applied in camera preview.
    //Force disable EIS
    updateEntry<MUINT8>(request , MTK_CONTROL_VIDEO_STABILIZATION_MODE , reqVstabMode);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updateRecordEISRequest(IMetadata *request, eis_params const& params) const
{
    uint8_t reqVstabMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;

    if (params.mEISEnabled)
    {
        reqVstabMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
    }

    updateEntry<MUINT8>(request , MTK_CONTROL_VIDEO_STABILIZATION_MODE , reqVstabMode);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updateHDRDetectionRequest(IMetadata *request) const
{
    HDRMode kHDRMode = mpParamsMgr->getHDRMode();

    IMetadata::setEntry<MUINT8>(
            request, MTK_3A_HDR_MODE, static_cast<MUINT8>(kHDRMode));

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ParamsManagerV3Imp::
updateEngRequest(IMetadata *request, eng_params const& params) const
{

    //vdo manual fps
    String8 const s1 = mpParamsMgr->getStr(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED);
    MBOOL const isSupportedManualFps = ( ! s1.isEmpty() && s1 == CameraParameters::TRUE ) ? MTRUE : MFALSE;
    MBOOL const isEnableManualFps = ((mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_ENABLE))==1)? MTRUE : MFALSE;
    if (isSupportedManualFps && isEnableManualFps)
    {
        updateEntryArray<MINT32>(request, MTK_CONTROL_AE_TARGET_FPS_RANGE, params.mManualFpsRange, 2);
        MY_LOGD("vdo manual fps: min(%d),max(%d)", params.mManualFpsRange[0],params.mManualFpsRange[1]);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
#if 0
IMetadata::IEntry ParamsManagerV3Imp::staticInfo(uint32_t tag) const {

    if( ! mpMetadataProvider.get() ) {
            MY_LOGE(" ! mpMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
    IMetadata static_meta = mpMetadataProvider->getMtkStaticCharacteristics();
    IMetadata::IEntry entry = static_meta.entryFor(tag);
    return entry;
}
#endif
/******************************************************************************
 *
 ******************************************************************************/
ParamsManagerV3Imp::ParamsManagerV3Imp::flashMode_t ParamsManagerV3Imp::flashModeStringToEnum(
        const char *flashMode) {
    return
        !flashMode ?
            ParamsManagerV3Imp::FLASH_MODE_OFF :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_OFF) ?
            ParamsManagerV3Imp::FLASH_MODE_OFF :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_AUTO) ?
            ParamsManagerV3Imp::FLASH_MODE_AUTO :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_ON) ?
            ParamsManagerV3Imp::FLASH_MODE_ON :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_RED_EYE) ?
            ParamsManagerV3Imp::FLASH_MODE_RED_EYE :
        !strcmp(flashMode, CameraParameters::FLASH_MODE_TORCH) ?
            ParamsManagerV3Imp::FLASH_MODE_TORCH :
        ParamsManagerV3Imp::FLASH_MODE_INVALID;
}

/******************************************************************************
 *
 ******************************************************************************/
ParamsManagerV3Imp::ParamsManagerV3Imp::focusMode_t ParamsManagerV3Imp::focusModeStringToEnum(
        const char *focusMode) {
    return
        !focusMode ?
            ParamsManagerV3Imp::FOCUS_MODE_INVALID :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_AUTO) ?
            ParamsManagerV3Imp::FOCUS_MODE_AUTO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_INFINITY) ?
            ParamsManagerV3Imp::FOCUS_MODE_INFINITY :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_MACRO) ?
            ParamsManagerV3Imp::FOCUS_MODE_MACRO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_FIXED) ?
            ParamsManagerV3Imp::FOCUS_MODE_FIXED :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_EDOF) ?
            ParamsManagerV3Imp::FOCUS_MODE_EDOF :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO) ?
            ParamsManagerV3Imp::FOCUS_MODE_CONTINUOUS_VIDEO :
        !strcmp(focusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE) ?
            ParamsManagerV3Imp::FOCUS_MODE_CONTINUOUS_PICTURE :
        !strcmp(focusMode, "manual") ? //Eng mode manaul position part
            ParamsManagerV3Imp::FOCUS_MODE_OFF :
        !strcmp(focusMode, "fullscan") ? //Eng mode manaul position part
            ParamsManagerV3Imp::FOCUS_MODE_AUTO :
        ParamsManagerV3Imp::FOCUS_MODE_INVALID;
}


/******************************************************************************
 *
 ******************************************************************************/
int ParamsManagerV3Imp::wbModeStringToEnum(const char *wbMode) {
    return
        !wbMode ?
            MTK_CONTROL_AWB_MODE_AUTO :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_AUTO) ?
            MTK_CONTROL_AWB_MODE_AUTO :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_INCANDESCENT) ?
            MTK_CONTROL_AWB_MODE_INCANDESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_FLUORESCENT) ?
            MTK_CONTROL_AWB_MODE_FLUORESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT) ?
            MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_DAYLIGHT) ?
            MTK_CONTROL_AWB_MODE_DAYLIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT) ?
            MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_TWILIGHT) ?
            MTK_CONTROL_AWB_MODE_TWILIGHT :
        !strcmp(wbMode, CameraParameters::WHITE_BALANCE_SHADE) ?
            MTK_CONTROL_AWB_MODE_SHADE :
        -1;
}

/******************************************************************************
 *
 ******************************************************************************/
bool ParamsManagerV3Imp::boolFromString(const char *boolStr) const {
    return !boolStr ? false :
        !strcmp(boolStr, CameraParameters::TRUE) ? true :
        false;
}


/******************************************************************************
 *
 ******************************************************************************/
int ParamsManagerV3Imp::effectModeStringToEnum(const char *effectMode) {
    return
        !effectMode ?
            MTK_CONTROL_EFFECT_MODE_OFF :
        !strcmp(effectMode, CameraParameters::EFFECT_NONE) ?
            MTK_CONTROL_EFFECT_MODE_OFF :
        !strcmp(effectMode, CameraParameters::EFFECT_MONO) ?
            MTK_CONTROL_EFFECT_MODE_MONO :
        !strcmp(effectMode, CameraParameters::EFFECT_NEGATIVE) ?
            MTK_CONTROL_EFFECT_MODE_NEGATIVE :
        !strcmp(effectMode, CameraParameters::EFFECT_SOLARIZE) ?
            MTK_CONTROL_EFFECT_MODE_SOLARIZE :
        !strcmp(effectMode, CameraParameters::EFFECT_SEPIA) ?
            MTK_CONTROL_EFFECT_MODE_SEPIA :
        !strcmp(effectMode, CameraParameters::EFFECT_POSTERIZE) ?
            MTK_CONTROL_EFFECT_MODE_POSTERIZE :
        !strcmp(effectMode, CameraParameters::EFFECT_WHITEBOARD) ?
            MTK_CONTROL_EFFECT_MODE_WHITEBOARD :
        !strcmp(effectMode, CameraParameters::EFFECT_BLACKBOARD) ?
            MTK_CONTROL_EFFECT_MODE_BLACKBOARD :
        !strcmp(effectMode, CameraParameters::EFFECT_AQUA) ?
            MTK_CONTROL_EFFECT_MODE_AQUA :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_SEPIA_BLUE) ?
            MTK_CONTROL_EFFECT_MODE_SEPIABLUE :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_SEPIA_GREEN) ?
            MTK_CONTROL_EFFECT_MODE_SEPIAGREEN :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_NASHVILLE) ?
            MTK_CONTROL_EFFECT_MODE_NASHVILLE :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_HEFE) ?
            MTK_CONTROL_EFFECT_MODE_HEFE :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_VALENCIA) ?
            MTK_CONTROL_EFFECT_MODE_VALENCIA :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_XPROII) ?
            MTK_CONTROL_EFFECT_MODE_XPROII :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_LOFI) ?
            MTK_CONTROL_EFFECT_MODE_LOFI :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_SIERRA) ?
            MTK_CONTROL_EFFECT_MODE_SIERRA :
        !strcmp(effectMode, MtkCameraParameters::EFFECT_WALDEN) ?
            MTK_CONTROL_EFFECT_MODE_WALDEN :
        -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 ParamsManagerV3Imp::abModeStringToEnum(const char *abMode) {
    return
        !abMode ?
            MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO :
        !strcmp(abMode, CameraParameters::ANTIBANDING_AUTO) ?
            MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO :
        !strcmp(abMode, CameraParameters::ANTIBANDING_OFF) ?
            MTK_CONTROL_AE_ANTIBANDING_MODE_OFF :
        !strcmp(abMode, CameraParameters::ANTIBANDING_50HZ) ?
            MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ :
        !strcmp(abMode, CameraParameters::ANTIBANDING_60HZ) ?
            MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ :
        -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 ParamsManagerV3Imp::sceneModeStringToEnum(const char *sceneMode) {
    return
        !sceneMode ?
            MTK_CONTROL_SCENE_MODE_DISABLED :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_AUTO) ?
            MTK_CONTROL_SCENE_MODE_DISABLED :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_ACTION) ?
            MTK_CONTROL_SCENE_MODE_ACTION :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_PORTRAIT) ?
            MTK_CONTROL_SCENE_MODE_PORTRAIT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_LANDSCAPE) ?
            MTK_CONTROL_SCENE_MODE_LANDSCAPE :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_NIGHT) ?
            MTK_CONTROL_SCENE_MODE_NIGHT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT) ?
            MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_THEATRE) ?
            MTK_CONTROL_SCENE_MODE_THEATRE :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_BEACH) ?
            MTK_CONTROL_SCENE_MODE_BEACH :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SNOW) ?
            MTK_CONTROL_SCENE_MODE_SNOW :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SUNSET) ?
            MTK_CONTROL_SCENE_MODE_SUNSET :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_STEADYPHOTO) ?
            MTK_CONTROL_SCENE_MODE_STEADYPHOTO :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_FIREWORKS) ?
            MTK_CONTROL_SCENE_MODE_FIREWORKS :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_SPORTS) ?
            MTK_CONTROL_SCENE_MODE_SPORTS :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_PARTY) ?
            MTK_CONTROL_SCENE_MODE_PARTY :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_CANDLELIGHT) ?
            MTK_CONTROL_SCENE_MODE_CANDLELIGHT :
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_BARCODE) ?
            MTK_CONTROL_SCENE_MODE_BARCODE:
        !strcmp(sceneMode, CameraParameters::SCENE_MODE_HDR) ?
            MTK_CONTROL_SCENE_MODE_HDR:
        !strcmp(sceneMode, MtkCameraParameters::SCENE_MODE_NORMAL) ?
            MTK_CONTROL_SCENE_MODE_NORMAL:
        -1;
}

int ParamsManagerV3Imp::decideFOVMargin(MINT32 sensorMode, MINT32 ZoomRatio, MSize SensorSize, MRect& SensorCrop, MSize& Margin)
{
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    HwTransHelper hwTransHelper(getOpenId());
    HwMatrix matToActive, matFromActive;
    MRect ActiveArrayCrop;
    MSize ActiveArraySize;
    FOVHal::Configs Config;
    MRect RealRect;

    if( hwTransHelper.getMatrixToActive(sensorMode, matToActive) )
    {
        matToActive.transform(SensorSize, ActiveArraySize);
        matToActive.transform(SensorCrop, ActiveArrayCrop);
    }
    else
    {
        Margin.w = 0;
        Margin.h = 0;
        MY_LOGE("get matrix fail");
        goto EXIT;
    }

    Config.CamID = getOpenId();
    Config.SensorSize = ActiveArraySize;
    Config.ZoomRatio = ZoomRatio;
    Config.CropSize = ActiveArrayCrop.s;

    FOVHal::getInstance()->getFOVMarginandCrop(Config, Margin, mActiveShiftCrop, mCropScale);
    if (Margin.w == 0 || Margin.h == 0)
    {
        Margin.w = 0;
        Margin.h = 0;
        goto EXIT;
    }
    RealRect = ActiveArrayCrop;
    ActiveArrayCrop.p.x -= (Margin.w/2);
    ActiveArrayCrop.p.y -= (Margin.h/2);
    ActiveArrayCrop.s.w += Margin.w;
    ActiveArrayCrop.s.h += Margin.h;

    if (ActiveArrayCrop.p.x < 0)
    {
        ActiveArrayCrop.p.x = 0;
        Margin.w = (RealRect.p.x - ActiveArrayCrop.p.x) * 2;
        ActiveArrayCrop.s.w = RealRect.s.w + Margin.w;
        MY_LOGD("boundary under, new crop(x, w):(%d, %d)", ActiveArrayCrop.p.x, ActiveArrayCrop.s.w);
    }
    if (ActiveArrayCrop.p.y < 0)
    {
        ActiveArrayCrop.p.y = 0;
        Margin.h = (RealRect.p.y - ActiveArrayCrop.p.y)*2;
        ActiveArrayCrop.s.h = RealRect.s.h + Margin.h;
        MY_LOGD("boundary under, new crop(y, h):(%d, %d)", ActiveArrayCrop.p.y, ActiveArrayCrop.s.h);
    }
    if ((ActiveArrayCrop.s.w + ActiveArrayCrop.p.x) > ActiveArraySize.w)
    {
        Margin.w -= (ActiveArrayCrop.s.w + ActiveArrayCrop.p.x) - ActiveArraySize.w;
        Margin.w = Margin.w & ~(0x01);
        ActiveArrayCrop.p.x = RealRect.p.x - (Margin.w/2);
        ActiveArrayCrop.s.w = RealRect.s.w + Margin.w;
        MY_LOGD("boundary over, new crop(x, w):(%d, %d)", ActiveArrayCrop.p.x, ActiveArrayCrop.s.w);
    }
    if ((ActiveArrayCrop.s.h + ActiveArrayCrop.p.y) > ActiveArraySize.h)
    {
        Margin.h -= (ActiveArrayCrop.s.h + ActiveArrayCrop.p.y) - ActiveArraySize.h;
        Margin.h = Margin.h & ~(0x01);
        ActiveArrayCrop.p.y = RealRect.p.y - (Margin.h/2);
        ActiveArrayCrop.s.h = RealRect.s.h + Margin.h;
        MY_LOGD("boundary over, new crop(y, h):(%d, %d)", ActiveArrayCrop.p.y, ActiveArrayCrop.s.h);
    }
    mActiveShiftCrop.s = Margin;

    if( hwTransHelper.getMatrixFromActive(sensorMode, matFromActive) )
    {
        matFromActive.transform(ActiveArrayCrop, SensorCrop);
        matFromActive.transform(mActiveShiftCrop, mSensorShiftCrop);
    }
    else
    {
        Margin.w = 0;
        Margin.h = 0;
        MY_LOGE("get matrix fail");
        goto EXIT;
    }
EXIT:
    FOVHal::getInstance()->setCurrentCropInfo(getOpenId(), ZoomRatio, ActiveArrayCrop, Margin);
#else
    Margin.w = 0;
    Margin.h = 0;
#endif
    return 1;

}


/******************************************************************************
 *
 ******************************************************************************/
status_t ParamsManagerV3Imp::calculateCropRegion(MINT32 sensorMode)
{
    Mutex::Autolock _l(mMutex);
    //
    if( sensorMode == -1 )
        sensorMode = mLastUpdateSensorMode;
    //
    HwInfoHelper hwInfoHelper(getOpenId());
    if( !hwInfoHelper.updateInfos() ||
        !hwInfoHelper.getSensorSize(sensorMode, mSensorSize) )
    {
        MY_LOGE("updateInfos or getSensorSize fail");
        return UNKNOWN_ERROR;
    }

    uint32_t zoomRatio = mpParamsMgr->getZoomRatio(getOpenId());
    mZoomRatio = mpParamsMgr->getZoomRatio();
    //calculate mSensorSizeCrop
    mSensorSizeCrop.s.w = mSensorSize.w * 100 / zoomRatio;
    mSensorSizeCrop.s.h = mSensorSize.h * 100 / zoomRatio;
    mSensorSizeCrop.p.x = (mSensorSize.w - mSensorSizeCrop.s.w) >> 1;
    mSensorSizeCrop.p.y = (mSensorSize.h - mSensorSizeCrop.s.h) >> 1;

    decideFOVMargin(sensorMode, zoomRatio, mSensorSize, mSensorSizeCrop, mActiveMargin);

    //calculate mSensorSizePreviewCrop
    MSize prvSize;
    mpParamsMgr->getPreviewSize(&prvSize.w, &prvSize.h);
    if( mSensorSizeCrop.s.w * prvSize.h > mSensorSizeCrop.s.h * prvSize.w ) { // pillarbox
        mSensorSizePreviewCrop.s.w = mSensorSizeCrop.s.h * prvSize.w / prvSize.h;
        mSensorSizePreviewCrop.s.h = mSensorSizeCrop.s.h;
        mSensorSizePreviewCrop.p.x = mSensorSizeCrop.p.x + ((mSensorSizeCrop.s.w - mSensorSizePreviewCrop.s.w) >> 1);
        mSensorSizePreviewCrop.p.y = mSensorSizeCrop.p.y;
    }
    else { // letterbox
        mSensorSizePreviewCrop.s.w = mSensorSizeCrop.s.w;
        mSensorSizePreviewCrop.s.h = mSensorSizeCrop.s.w * prvSize.h / prvSize.w;
        mSensorSizePreviewCrop.p.x = mSensorSizeCrop.p.x;
        mSensorSizePreviewCrop.p.y = mSensorSizeCrop.p.y + ((mSensorSizeCrop.s.h - mSensorSizePreviewCrop.s.h) >> 1);
    }
    //crop shift to avoid bad pixel
    if(mCropShift > 0)
    {
        if( mSensorSizeCrop.p.x < mCropShift)
        {
            mSensorSizeCrop.s.w -= ((long)abs((long)(mSensorSizeCrop.p.x - mCropShift))*2);
            mSensorSizeCrop.p.x = mCropShift;
        }
        if( mSensorSizeCrop.p.y < mCropShift)
        {
            mSensorSizeCrop.s.h -= ((long)abs((long)(mSensorSizeCrop.p.y - mCropShift))*2);
            mSensorSizeCrop.p.y = mCropShift;
        }
        //
        if( mSensorSizePreviewCrop.p.x < mCropShift)
        {
            mSensorSizePreviewCrop.s.w -= ((long)abs((long)(mSensorSizePreviewCrop.p.x - mCropShift))*2);
            mSensorSizePreviewCrop.p.x = mCropShift;
        }
        if( mSensorSizePreviewCrop.p.y < mCropShift)
        {
            mSensorSizePreviewCrop.s.h -= ((long)abs((long)(mSensorSizePreviewCrop.p.y - mCropShift))*2);
            mSensorSizePreviewCrop.p.y = mCropShift;
        }
    }
    //crop size need 2x2 alignment
    mSensorSizeCrop.s.w &= (~1);
    mSensorSizeCrop.s.h &= (~1);
    mSensorSizeCrop.p.x &= (~1);
    mSensorSizeCrop.p.y &= (~1);
    mSensorSizePreviewCrop.s.w &= (~1);
    mSensorSizePreviewCrop.s.h &= (~1);
    mSensorSizePreviewCrop.p.x &= (~1);
    mSensorSizePreviewCrop.p.y &= (~1);

    //calculate mActiveArrayCrop, mActiveArrayPreviewCrop
    HwTransHelper hwTransHelper(getOpenId());
    HwMatrix matToActive;
    if( hwTransHelper.getMatrixToActive(sensorMode, matToActive) )
    {
        matToActive.transform(mSensorSizeCrop, mActiveArrayCrop);
        matToActive.transform(mSensorSizePreviewCrop, mActiveArrayPreviewCrop);
    }
    else
    {
        MY_LOGE("get matrix fail");
        return UNKNOWN_ERROR;
    }

    MY_LOGD("sensor mode(%d), sensor size(%dx%d), zoomRatio(%d)"
            ", sensor crop(%d,%d,%dx%d), active array(%d,%d,%dx%d)"
            ", sensor preview crop(%d,%d,%dx%d), active array preview(%d,%d,%dx%d)",
                sensorMode,
                mSensorSize.w, mSensorSize.h,
                zoomRatio,
                mSensorSizeCrop.p.x, mSensorSizeCrop.p.y, mSensorSizeCrop.s.w, mSensorSizeCrop.s.h,
                mActiveArrayCrop.p.x, mActiveArrayCrop.p.y, mActiveArrayCrop.s.w, mActiveArrayCrop.s.h,
                mSensorSizePreviewCrop.p.x, mSensorSizePreviewCrop.p.y,
                mSensorSizePreviewCrop.s.w, mSensorSizePreviewCrop.s.h,
                mActiveArrayPreviewCrop.p.x, mActiveArrayPreviewCrop.p.y,
                mActiveArrayPreviewCrop.s.w, mActiveArrayPreviewCrop.s.h);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MRect
ParamsManagerV3Imp::
getAAAPreviewCropRegion() const
{
    Mutex::Autolock _l(mMutex);
    return mPreviewCropRegion;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
getCropRegion(
    MSize& sensorSize,
    MRect& activeArrayCrop,
    MRect& sensorSizeCrop,
    MRect& activeArrayPreviewCrop,
    MRect& sensorSizePreviewCrop) const
{
    Mutex::Autolock _l(mMutex);
    sensorSize = mSensorSize;
    activeArrayCrop = mActiveArrayCrop;
    sensorSizeCrop = mSensorSizeCrop;
    activeArrayPreviewCrop = mActiveArrayPreviewCrop;
    sensorSizePreviewCrop = mSensorSizePreviewCrop;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
getCropRegion(
    MSize& sensorSize,
    MRect& activeArrayCrop,
    MRect& sensorSizeCrop,
    MRect& activeArrayPreviewCrop,
    MRect& sensorSizePreviewCrop,
    MSize& margin) const
{
    Mutex::Autolock _l(mMutex);
    sensorSize = mSensorSize;
    activeArrayCrop = mActiveArrayCrop;
    sensorSizeCrop = mSensorSizeCrop;
    activeArrayPreviewCrop = mActiveArrayPreviewCrop;
    sensorSizePreviewCrop = mSensorSizePreviewCrop;
    margin = mActiveMargin;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ParamsManagerV3Imp::
getCapCropRegion(
    MRect& activeArrayCrop,
    MRect& sensorSizeCrop,
    MRect& sensorSizePreviewCrop) const
{
    Mutex::Autolock _l(mMutex);
    activeArrayCrop = mActiveArrayCrop;
    sensorSizeCrop = mSensorSizeCrop;
    sensorSizePreviewCrop = mSensorSizePreviewCrop;
    //MY_LOGD("mActiveShiftCrop : %dx%d", mActiveShiftCrop.s.w, mActiveShiftCrop.s.h);
    if (mActiveShiftCrop.s.w != 0 && mActiveShiftCrop.s.h != 0)
    {
        // modify capture crop for fov alignment
        //MY_LOGD("activeArrayCrop : %d, %d, %dx%d", activeArrayCrop.p.x, activeArrayCrop.p.y, activeArrayCrop.s.w, activeArrayCrop.s.h);
        MSize Boundary;
        Boundary.w = activeArrayCrop.p.x + activeArrayCrop.s.w;
        Boundary.h = activeArrayCrop.p.y + activeArrayCrop.s.h;
        activeArrayCrop.s.w -= mActiveShiftCrop.s.w;
        activeArrayCrop.s.h -= mActiveShiftCrop.s.h;
        activeArrayCrop.s.w = activeArrayCrop.s.w * mCropScale;
        activeArrayCrop.s.h = activeArrayCrop.s.h * mCropScale;
        activeArrayCrop.p.x = activeArrayCrop.p.x + (mActiveShiftCrop.s.w/2) + mActiveShiftCrop.p.x;
        activeArrayCrop.p.y = activeArrayCrop.p.y + (mActiveShiftCrop.s.h/2) + mActiveShiftCrop.p.y;
        // boundary check
        if ((activeArrayCrop.p.x + activeArrayCrop.s.w) > Boundary.w)
        {
            activeArrayCrop.p.x = Boundary.w - activeArrayCrop.s.w;
        }
        if ((activeArrayCrop.p.y + activeArrayCrop.s.h) > Boundary.h)
        {
            activeArrayCrop.p.y = Boundary.h - activeArrayCrop.s.h;
        }
        activeArrayCrop.p.x = (activeArrayCrop.p.x < 0) ? 0 : activeArrayCrop.p.x;
        activeArrayCrop.p.y = (activeArrayCrop.p.y < 0) ? 0 : activeArrayCrop.p.y;
        //MY_LOGD("activeArrayCrop : %d, %d, %dx%d", activeArrayCrop.p.x, activeArrayCrop.p.y, activeArrayCrop.s.w, activeArrayCrop.s.h);

        Boundary.w = sensorSizeCrop.p.x + sensorSizeCrop.s.w;
        Boundary.h = sensorSizeCrop.p.y + sensorSizeCrop.s.h;
        sensorSizeCrop.s.w -= mSensorShiftCrop.s.w;
        sensorSizeCrop.s.h -= mSensorShiftCrop.s.h;
        sensorSizeCrop.s.w = sensorSizeCrop.s.w * mCropScale;
        sensorSizeCrop.s.h = sensorSizeCrop.s.h * mCropScale;
        sensorSizeCrop.p.x = sensorSizeCrop.p.x + (mSensorShiftCrop.s.w/2) + mSensorShiftCrop.p.x;
        sensorSizeCrop.p.y = sensorSizeCrop.p.y + (mSensorShiftCrop.s.h/2) + mSensorShiftCrop.p.y;
        // boundary check
        if ((sensorSizeCrop.p.x + sensorSizeCrop.s.w) > Boundary.w)
        {
            sensorSizeCrop.p.x = Boundary.w - sensorSizeCrop.s.w;
        }
        if ((sensorSizeCrop.p.y + sensorSizeCrop.s.h) > Boundary.h)
        {
            sensorSizeCrop.p.y = Boundary.h - sensorSizeCrop.s.h;
        }
        sensorSizeCrop.p.x = (sensorSizeCrop.p.x < 0) ? 0 : sensorSizeCrop.p.x;
        sensorSizeCrop.p.y = (sensorSizeCrop.p.y < 0) ? 0 : sensorSizeCrop.p.y;

        if( sensorSizeCrop.s.w * sensorSizePreviewCrop.s.h > sensorSizeCrop.s.h * sensorSizePreviewCrop.s.w ) { // pillarbox
            sensorSizePreviewCrop.s.w = sensorSizeCrop.s.h * sensorSizePreviewCrop.s.w / sensorSizePreviewCrop.s.h;
            sensorSizePreviewCrop.s.h = sensorSizeCrop.s.h;
            sensorSizePreviewCrop.p.x = sensorSizeCrop.p.x + ((sensorSizeCrop.s.w - sensorSizePreviewCrop.s.w) >> 1);
            sensorSizePreviewCrop.p.y = sensorSizeCrop.p.y;
        }
        else { // letterbox
            sensorSizePreviewCrop.s.w = sensorSizeCrop.s.w;
            sensorSizePreviewCrop.s.h = sensorSizeCrop.s.w * sensorSizePreviewCrop.s.h / sensorSizePreviewCrop.s.w;
            sensorSizePreviewCrop.p.x = sensorSizeCrop.p.x;
            sensorSizePreviewCrop.p.y = sensorSizeCrop.p.y + ((sensorSizeCrop.s.h - sensorSizePreviewCrop.s.h) >> 1);
        }

    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int
ParamsManagerV3Imp::
getZoomRatio()
{
    Mutex::Autolock _l(mMutex);
    return mZoomRatio;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ParamsManagerV3Imp::
updateBasedOnScenario(IMetadata* meta, MBOOL& bRepeat)
{
    Mutex::Autolock _l(mMutex);
    MUINT8 aeMode = MTK_CONTROL_AE_MODE_ON;
    IMetadata::getEntry<MUINT8>(meta, MTK_CONTROL_AE_MODE, aeMode);
    bool updateFlashMode = false;
    switch(mScenario) {
        case eScenario_Video_Preview:
        case eScenario_Video_Preview_Repeat: {
            // CameraParameters::FLASH_MODE_ON
            // set flash mode to MTK_FLASH_MODE_TORCH
            if ( aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ) {
                updateFlashMode = mNeedFlashLight = true;
            } else if(aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) {
                updateFlashMode = false;
                MUINT8 aeState = MTK_CONTROL_AE_STATE_INACTIVE;
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
                IMetadata metaLocal;
                #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
                int MasterId = -1;
                if (FOVHal::getInstance()->getMasterCamId(MasterId) && MasterId != -1)
                {
                    sp<IFrameInfo> pFrameInfo1 = IResourceContainer::getInstance(MasterId)->queryLatestFrameInfo();
                    if ( pFrameInfo1.get() )
                    {
                        pFrameInfo = pFrameInfo1;
                    }
                }
                #endif
                if ( pFrameInfo.get() ) {
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metaLocal);
                    MBOOL isBvTrigger = MFALSE;
                    MBOOL ret = MFALSE;
                    ret = IMetadata::getEntry<MBOOL>(&metaLocal, MTK_3A_AE_BV_TRIGGER, isBvTrigger);
                    if (ret == MTRUE) {
                        mNeedFlashLight = isBvTrigger;
                    } else {
                        MY_LOGD1("eScenario_Video_Preview, by pass update mNeedFlashLight, ret:%d", (int)ret);
                    }
                    MY_LOGD1("eScenario_Video_Preview, mNeedFlashLight:%d, ret:%d", mNeedFlashLight, (int)ret);
                } else {
                    MY_LOGD1("eScenario_Video_Preview, !get(), mNeedFlashLight:%d", mNeedFlashLight);
                }
            } else {
                mNeedFlashLight = false;
            }
            bRepeat = (mScenario == eScenario_Video_Preview_Repeat) ? bRepeat : false;
            mScenario = eScenario_Video_Preview_Repeat;
        } break;
        case eScenario_Video_Record_Start:
        case eScenario_Video_Record_Start_Repeat:{
            // CameraParameters::FLASH_MODE_AUTO & MTK_CONTROL_AE_STATE_FLASH_REQUIRED
            // set flash mode to MTK_FLASH_MODE_TORCH
            bRepeat = (mScenario == eScenario_Video_Record_Start_Repeat) ? bRepeat : false;
            updateFlashMode = mNeedFlashLight;
            mScenario = eScenario_Video_Record_Start_Repeat;
        } break;
        case eScenario_Video_Record_Stop:{
            mNeedFlashLight = false;
            mScenario = eScenario_Video_Preview;
        } break;
        case eScenario_Camera_Preview:
#if 0
            // regular
            sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
            IMetadata metaLocal;
            if ( pFrameInfo.get() ) pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metaLocal);
            MBOOL isBvTrigger = MFALSE;
            IMetadata::getEntry<MBOOL>(&metaLocal, MTK_3A_AE_BV_TRIGGER, isBvTrigger);
            mNeedFlashLight = isBvTrigger;
            MY_LOGD1("eScenario_Camera_Preview, mNeedFlashLight:%d", mNeedFlashLight);

            //mNeedFlashLight = false;
#endif
            // CameraParameters::FLASH_MODE_ON
            // set flash mode to MTK_FLASH_MODE_TORCH
            if ( aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ) {
                updateFlashMode = false;
                mNeedFlashLight = true;
            } else if(aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) {
                updateFlashMode = false;
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
                IMetadata metaLocal;
                #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
                int MasterId = -1;
                if (FOVHal::getInstance()->getMasterCamId(MasterId) && MasterId != -1)
                {
                    sp<IFrameInfo> pFrameInfo1 = IResourceContainer::getInstance(MasterId)->queryLatestFrameInfo();
                    if ( pFrameInfo1.get() )
                    {
                        pFrameInfo = pFrameInfo1;
                    }
                }
                #endif
                if ( pFrameInfo.get() ) {
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metaLocal);
                    MBOOL isBvTrigger = MFALSE;
                    MBOOL ret = MFALSE;
                    ret = IMetadata::getEntry<MBOOL>(&metaLocal, MTK_3A_AE_BV_TRIGGER, isBvTrigger);
                    if (ret == MTRUE) {
                        mNeedFlashLight = isBvTrigger;
                    } else {
                        MY_LOGD1("eScenario_Camera_Preview, by pass update mNeedFlashLight, ret:%d", (int)ret);
                    }
                    MY_LOGD1("eScenario_Camera_Preview, mNeedFlashLight:%d, ret:%d", mNeedFlashLight, (int)ret);
                } else {
                    MY_LOGD1("eScenario_Camera_Preview, !get(), mNeedFlashLight:%d", mNeedFlashLight);
                }
            } else {
                mNeedFlashLight = false;
            }
            break;
    };
    if ( updateFlashMode ) {
        IMetadata::setEntry<MUINT8>(meta, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_ON);
        IMetadata::setEntry<MUINT8>(meta, MTK_FLASH_MODE     , MTK_FLASH_MODE_TORCH);
    }
    MY_LOGD1("mode/m_state/repeat/mScenario/update = %d/%d/%d/%d/%d", aeMode, mNeedFlashLight, bRepeat, mScenario, updateFlashMode);

}

/******************************************************************************
 *
 ******************************************************************************/
void
ParamsManagerV3Imp::
getPreviewFpsRange(MINT32& minFps, MINT32& maxFps)
{
    minFps = mPreviewFpsRange[0];
    maxFps = mPreviewFpsRange[1];
}
/******************************************************************************
 *
 ******************************************************************************/
void
ParamsManagerV3Imp::
setPreviewFpsRange(MINT32& minFps, MINT32& maxFps)
{
    mPreviewFpsRange[0] = minFps;
    mPreviewFpsRange[1] = maxFps;
}


