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

//
#include "inc/Local.h"
#include "inc/ParamsManager.h"
#include <utils/String16.h>
#include <utils/Mutex.h>
#include <mtkcam/drv/IHalSensor.h>
#include <math.h>
#include <cutils/properties.h>

// STL
#include <type_traits> //static_assert
// MTKCAM
#include <mtkcam/feature/hdrDetection/Defs.h>
// CUSTOM
#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <map>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

pid_t MyRWLock::mWriteTID = -1;
pid_t MyRWLock::mWritePID = -1;

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1) || (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT==1)
#define DUAL_CAM 1
#else
#define DUAL_CAM 0
#endif

using namespace NSCam;

/******************************************************************************
*
*******************************************************************************/
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

#define VALUES_POSTFIX "-values"

//the result get from mParameters.get() may be release after lock scope,
//we have to copy it before leaving lock scope
const char * __getSafeString(const char *p, String8 &s)
{
    if(p) {
        s.setTo(p);
        return s.string();
    }

    return NULL;
}

/******************************************************************************
*
*******************************************************************************/
ParamsManager::
ParamsManager(String8 const& rName, int32_t const i4OpenId)
    : IParamsManager()
    //
    , mRWLock()
    //
    , mpFeatureKeyedMap(NULL)
    //
    , mParameters()
    , mName(rName)
    , mi4OpenId(i4OpenId)
    //
    , ms8ClientAppMode(rName)
    , mi4HalAppMode(0)
    //
    , mIsAppendPreviewSize(false)
    , mIsDualZoomMode(0)
    , mMain2CamID(0)
    , mStaticVhdrMode(MtkCameraParameters::VIDEO_HDR_MODE_NONE)
{
    mDefaultPreviewFps = 30;
    mDefaultPreviewFpsRange[0] = 5;
    mDefaultPreviewFpsRange[1] = 30;

    mRecordLikely = false;
    updateLogStatus();
    mSetParametersByString = property_get_int32("vendor.paramsmgr.string", 1);
    //
    mbSetParametersChangeNothing.store(false);
}

void
ParamsManager::updateLogStatus()
{
    mLogLevel = property_get_int32("vendor.paramsmgr.log", PARAMS_MGR_NO_LOG);
    mEnableDebugLog = IS_SHOW_DEBUG(mLogLevel);
}

/******************************************************************************
*
*******************************************************************************/
ParamsManager::
~ParamsManager()
{
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
init()
{
    return   updateDefaultParams()
          && updateDefaultEngParams()
          && updateHalAppMode()
             ;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
uninit()
{
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getValuesKeyName(char const aKeyName[])
{
    return  getValuesKeyName(String8(aKeyName));
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getSupportedKeyName(String8 const& s8KeyName)
{
    return  s8KeyName + "-supported";
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getValuesKeyName(String8 const& s8KeyName)
{
    return  s8KeyName + VALUES_POSTFIX;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
splitInt(String8 const& s8Input, Vector<int>& rOutput)
{
    rOutput.clear();
    //
    if  ( s8Input.isEmpty() )
    {
        MY_LOGW("empty string");
        return  false;
    }
    //
    char const*start = s8Input.string();
    char *end = NULL;
    do {
        int value = ::strtol(start, &end, 10);
        if  ( start == end ) {
            MY_LOGW_IF(0, "no digits in str:%s", s8Input.string());
            return  false;
        }
        rOutput.push_back(value);
        MY_LOGD_IF(0, "%d", value);
        start = end + 1;
    } while ( end && *end );
    //
    return  (rOutput.size() > 0);
}


/******************************************************************************
*   Splits a comma delimited string to a List of int Vector.
*   Example string: "(10000,26623),(10000,30000)"
*******************************************************************************/
bool
ParamsManager::
splitRange(String8 const& s8Input, List< Vector<int> >& rOutput)
{
    rOutput.clear();
    //
    if  ( s8Input.isEmpty() )
    {
        MY_LOGW("empty string");
        return  false;
    }
    //
    int endIndex, fromIndex = 1;
    int endIndex_input = s8Input.length()-1;
    if  ( s8Input[0] != '(' || s8Input[endIndex_input] != ')' )
    {
        MY_LOGW("Invalid range list string=%s", s8Input.string());
        return  false;
    }
    //
    do  {
        endIndex = s8Input.find("),(", fromIndex);
        if (endIndex == -1) endIndex = endIndex_input;
        //
        Vector<int> vOut;
        String8 const s8SubString(s8Input.string()+fromIndex, endIndex-fromIndex);
        if  ( splitInt(s8SubString, vOut) ) {
            rOutput.push_back(vOut);
        }
        //
        fromIndex = endIndex + 3;
    } while (endIndex != endIndex_input);
    //
    return  (rOutput.size() > 0);
}


/******************************************************************************
*
*******************************************************************************/
void
IParamsManager::
showParameters(String8 const& rs8Param)
{
    AutoParamsMgrProfileUtil profile(LOG_TAG, "Show Parameters");

    String8 s8Log;
    ssize_t const max_cpy_len = 767;

    size_t loop = 0;
    size_t const len = rs8Param.length();
    char const* pHead = rs8Param.string();
    char const*const pEnd = pHead + len;

    for (; pHead < pEnd;)
    {
        ssize_t cpy_len = pEnd - pHead;
        if  ( cpy_len > max_cpy_len )
        {
            cpy_len = max_cpy_len;
        }
        s8Log.setTo(pHead, cpy_len);
        CAM_LOGD("%s", s8Log.string());
        pHead += cpy_len;
        loop++;
    }
    // MY_LOGD_IF(mEnableDebugLog, "%zu %zu", loop, len);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
showParameters() const
{
    IParamsManager::showParameters(flatten());
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getDiffParameters(const String8 &stringParams,
                  DefaultKeyedVector<string, string> &diffParams,
                  vector<string> &removedKeys)
{
    AutoParamsMgrProfileUtil profile(LOG_TAG, "getDiffParameters");
    const char *a = stringParams.string();
    const char *b;

    const char *currentValue = NULL;

    //Mark all exist keys to not exist, we'll set during pasing parameters
    for(auto& k : mExistKeys) {
        k.second = false;
    }

    #define ADD_PARAMS(k, v) \
                do { \
                    size_t srcSize = v.length(); \
                    size_t dstSize = (NULL == currentValue) ? 0 : strlen(currentValue); \
                    if(NULL == currentValue || \
                       srcSize != dstSize || \
                       !!memcmp(v.c_str(), currentValue, srcSize)) \
                    { \
                        diffParams.add(k, v); \
                        MY_LOGD_IF(mEnableDebugLog, "Change %s: %s->%s", k.c_str(), currentValue, v.c_str()); \
                    } \
                } while(0)

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        string k(a, (size_t)(b-a));
        currentValue = mParameters.get(k.c_str());
        mExistKeys[k] = true;

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            string v(a);
            ADD_PARAMS(k, v);
            break;
        }

        string v(a, (size_t)(b-a));
        ADD_PARAMS(k, v);

        a = b+1;
    }

    // Find keys to remove
    for(auto &key : mExistKeys) {
        if(!key.second) {
            removedKeys.push_back(key.first);
            MY_LOGD_IF(mEnableDebugLog, "Remove key: %s", key.first.c_str());
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
isSetParametersChangeNothing()
{
    return mbSetParametersChangeNothing.load();
}

/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
setParameters(String8 const& paramsIn)
{
    if(paramsIn.size() < 3) {   //at least "a=b"
        return BAD_VALUE;
    }

    if(!mSetParametersByString) {
        return setParameters(CameraParameters(paramsIn));
    }

    AutoParamsMgrProfileUtil profile(LOG_TAG, "=== setParameters(string8) ===");
    if(IS_SHOW_PARAMS(mLogLevel)) {
        IParamsManager::showParameters(paramsIn);
    }

    MY_LOGD_IF(mEnableDebugLog, "%d: + AutoWLock", getOpenId());
    MyRWLock::AutoWLock _lock(mRWLock, __FUNCTION__);

    DefaultKeyedVector<string, string> params;
    vector<string> removedKeys;
    getDiffParameters(paramsIn, params, removedKeys);

    for(auto& k : removedKeys) {
        mParameters.remove(k.c_str());
    }

    status_t    status = OK;
    bool        bIsSceneChanged = false;

    //
    //  (1) Check to see if new parameters are valid or not.
    CameraParameters camParams;
    size_t paramsSize = params.size();
    const char *key;
    {
        int count = 0;
        // AutoParamsMgrProfileUtil profile(LOG_TAG, "Prepare params");
        for(size_t i = 0; i < paramsSize; i++) {
            key = params.keyAt(i).c_str();
            camParams.set(key, params.valueAt(i).c_str());
            count++;
        }
        MY_LOGD_IF(mEnableDebugLog && count > 0, "Set %d parameters", count);

        if(0 == count) {
            MY_LOGD_IF(mEnableDebugLog, "No parameter to update");
            mbSetParametersChangeNothing.store(true);
            return status;
        }
        mbSetParametersChangeNothing.store(false);
    }

    if  ( OK != (status = checkParams(camParams)) )
    {
        MY_LOGE("checkParams() return %d", status);
        return  status;
    }

    //  Check to see if Preview Size Changes or not. Get old preview size
    if(camParams.get(CameraParameters::KEY_PREVIEW_SIZE))
    {
        // AutoParamsMgrProfileUtil profile(LOG_TAG, "Update Preview Size");
        Size oldPrvSize, tmpPrvSize, newPrvSize;
        mParameters.getPreviewSize(&oldPrvSize.width, &oldPrvSize.height);
        camParams.getPreviewSize(&tmpPrvSize.width, &tmpPrvSize.height);

        //  Update Parameter: preview size
        mParameters.setPreviewSize(tmpPrvSize.width, tmpPrvSize.height);
        updatePreviewSize();
        mParameters.getPreviewSize(&newPrvSize.width, &newPrvSize.height);
        MY_LOGD_IF(mEnableDebugLog && ( oldPrvSize.width != newPrvSize.width || oldPrvSize.height != newPrvSize.height ),
                   "Preview Size change: %dx%d/%dx%d -> (%dx%d)",
                   oldPrvSize.width, oldPrvSize.height,
                   tmpPrvSize.width, tmpPrvSize.height,
                   newPrvSize.width, newPrvSize.height
        );

        camParams.remove(CameraParameters::KEY_PREVIEW_SIZE);
    }

    //  Get old&new picture size
    if(camParams.get(CameraParameters::KEY_PICTURE_SIZE)) {
        // AutoParamsMgrProfileUtil profile(LOG_TAG, "Update Picture Size");
        Size oldPicSize, newPicSize;

        mParameters.getPictureSize(&oldPicSize.width, &oldPicSize.height);
        camParams.getPictureSize(&newPicSize.width, &newPicSize.height);
        mParameters.setPictureSize(newPicSize.width, newPicSize.height);

        // Update FOV according to picture size
        if(oldPicSize.width != newPicSize.width || oldPicSize.height != newPicSize.height)
        {
            MY_LOGI_IF(mEnableDebugLog, "picSZ(%d,%d)->(%d,%d)",oldPicSize.width,oldPicSize.height,newPicSize.width,newPicSize.height);
            updateFov(newPicSize);
        }

        camParams.remove(CameraParameters::KEY_PICTURE_SIZE);
    }

    char forceSceneMode[PROPERTY_VALUE_MAX] = {"-1"};
    if(mLogLevel != PARAMS_MGR_NO_LOG)
    {
        property_get("vendor.forceset.scenemode", forceSceneMode, "-1");
        if(strcmp(forceSceneMode,"-1"))
        {
            MY_LOGD("Force set scene mode: %s", forceSceneMode);
            camParams.set(CameraParameters::KEY_SCENE_MODE, forceSceneMode);
        }
    }

    //  Check to see if Scene Changes or not.
    char const*const pOldScene = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    char const*const pNewScene =   camParams.get(CameraParameters::KEY_SCENE_MODE);
    if  ( NULL != pOldScene &&
          NULL != pNewScene &&
          0 != ::strcmp(pOldScene, pNewScene) )
    {
        //  scene mode has changed
        MY_LOGD_IF(mEnableDebugLog, "Scene change: %s -> %s", pOldScene, pNewScene);
        bIsSceneChanged = true;
    }

    // Save original KEY_VIDEO_HDR_MODE
    char const*const pVideoHdrMode = mParameters.get(MtkCameraParameters::KEY_VIDEO_HDR_MODE);

    //  Update rest parameters
    {
        AutoParamsMgrProfileUtil profile(LOG_TAG, "Update parameters");
        for(size_t i = 0; i < paramsSize; i++) {
            mParameters.set(params.keyAt(i).c_str(), params.valueAt(i).c_str());
        }
    }
    // Keep original KEY_VIDEO_HDR_MODE
    char const*const pVideoHdrModeNew = mParameters.get(MtkCameraParameters::KEY_VIDEO_HDR_MODE);
    if(pVideoHdrMode != NULL && pVideoHdrModeNew != NULL && 0 != ::strcmp(pVideoHdrMode, pVideoHdrModeNew))
    {
        MY_LOGD_IF(mEnableDebugLog, "Keep the original KEY_VIDEO_HDR_MODE: %s -> %s", pVideoHdrModeNew, pVideoHdrMode);
        mParameters.set(MtkCameraParameters::KEY_VIDEO_HDR_MODE, pVideoHdrMode);
    }

    if(mLogLevel != PARAMS_MGR_NO_LOG && strcmp(forceSceneMode,"-1"))
    {
        mParameters.set(CameraParameters::KEY_SCENE_MODE, forceSceneMode);
    }

    //  Update Parameters
    if  ( bIsSceneChanged ) {
        if  ( ! updateSceneAndParams() ) {
            status = BAD_VALUE;
        }
    }

    //  Add preview size for display rotation
    AddPreviewSizeForPanel();
    MY_LOGD_IF(1, "length %d", paramsIn.length());
    return status;
}

status_t
ParamsManager::
setParameters(CameraParameters const& params)
{
    AutoParamsMgrProfileUtil profile(LOG_TAG, "=== setParameters(Params)  ===");
    if(IS_SHOW_PARAMS(mLogLevel)) {
        IParamsManager::showParameters(params.flatten());
    }

    MY_LOGD_IF(mEnableDebugLog, "%d: + AutoWLock", getOpenId());
    MyRWLock::AutoWLock _lock(mRWLock, __FUNCTION__);
    //
    status_t    status = OK;
    bool        bIsSceneChanged = false;
    Size        oldPrvSize, tmpPrvSize, newPrvSize;
    Size        oldPicSize, newPicSize;
    //
    //  (1) Check to see if new parameters are valid or not.
    if  ( OK != (status = checkParams(params)) )
    {
        MY_LOGE("checkParams() return %d", status);
        return  status;
    }

    //  (2) Check to see if Scene Changes or not.
    char const*const pOldScene = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    char const*const pNewScene =      params.get(CameraParameters::KEY_SCENE_MODE);
    if  ( 0 != ::strcmp(pOldScene, pNewScene) )
    {
        //  scene mode has changed
        MY_LOGD_IF(mEnableDebugLog, "Scene change: %s -> %s", pOldScene, pNewScene);
        bIsSceneChanged = true;
    }
    //
    //  (3.1) Check to see if Preview Size Changes or not. Get old preview size
    mParameters.getPreviewSize(&oldPrvSize.width, &oldPrvSize.height);
    params.getPreviewSize(&tmpPrvSize.width, &tmpPrvSize.height);

    //  (3.2) Get old&new picture size
    mParameters.getPictureSize(&oldPicSize.width, &oldPicSize.height);
    params.getPictureSize(&newPicSize.width, &newPicSize.height);

    //  (4) Update Parameter
    {
        AutoParamsMgrProfileUtil profile(LOG_TAG, "Copy Parameters");
        mParameters = params;
    }

    if  ( bIsSceneChanged ) {
        if  ( ! updateSceneAndParams() ) {
            status = BAD_VALUE;
        }
    }

    //  (5) Update Parameter: preview size
    updatePreviewSize();

    // (6) Update FOV according to picture size
    if(oldPicSize.width != newPicSize.width || oldPicSize.height != newPicSize.height)
    {
        MY_LOGI_IF(mEnableDebugLog, "picSZ(%d,%d)->(%d,%d)",oldPicSize.width,oldPicSize.height,newPicSize.width,newPicSize.height);
        updateFov(newPicSize);
    }

    //  (3.2) Check to see if Preview Size Changes or not. Get new preview size
    mParameters.getPreviewSize(&newPrvSize.width, &newPrvSize.height);

    if  ( oldPrvSize.width != newPrvSize.width || oldPrvSize.height != newPrvSize.height )
    {
        MY_LOGD_IF(mEnableDebugLog,
            "Preview Size change: %dx%d/%dx%d -> (%dx%d)",
            oldPrvSize.width, oldPrvSize.height,
            tmpPrvSize.width, tmpPrvSize.height,
            newPrvSize.width, newPrvSize.height
        );
    }
    // (6) Add preview size for display rotation
    AddPreviewSizeForPanel();

    MY_LOGD_IF(mEnableDebugLog, "-");
    return status;
}

/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
flatten() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    return mParameters.flatten();
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getPreviewFormat() const
{
    return  getStr(CameraParameters::KEY_PREVIEW_FORMAT);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getPreviewSize(int *width, int *height) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    mParameters.getPreviewSize(width, height);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getVideoSize(int *width, int *height) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    mParameters.getVideoSize(width, height);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getPictureSize(int *width, int *height) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    mParameters.getPictureSize(width, height);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getIsoSpeed(int *iso) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    char const *pIso = mParameters.get(MtkCameraParameters::KEY_ISO_SPEED);
    *iso = mParameters.getInt(MtkCameraParameters::KEY_ISO_SPEED);
    if(pIso == NULL)
    {
        MY_LOGD_IF(mEnableDebugLog, "Iso Sepeed is NULL");
    }
    else if (0 == ::strcmp(pIso, MtkCameraParameters::ISO_SPEED_AUTO))
    {
        MY_LOGD_IF(mEnableDebugLog, "Iso Sepeed is Auto==(%s) (%d)",pIso,*iso);
    }
    else
    {
        MY_LOGD_IF(mEnableDebugLog, "Iso Sepeed is (%s) (%d)",pIso,*iso);
    }
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getManualCap() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

    bool ret = false;
    char const *p = mParameters.get(MtkCameraParameters::KEY_MANUAL_CAP);
    if(p)
    {
        MY_LOGD_IF(mEnableDebugLog, "KEY_MANUAL_CAP = %s", p);
        ret = (::strcmp(p, MtkCameraParameters::ON) == 0);
    }
    else
    {
        ret = false;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDynamicSwitchSensorMode() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    //
    bool ret = false;
#if (1==STEREO_CAMERA_SUPPORTED)
    if( StereoSettingProvider::isDualCamMode() == false)
#else
    if(1)
#endif
    {
        int switchMode = property_get_int32("vendor.camera.switchmode", -1);
        if(switchMode == 0)
        {
            MY_LOGD("forced to set getDynamicSwitchSensorMode()==false");
            return false;
        }
        else if(switchMode > 0)
        {
            MY_LOGD("forced to set getDynamicSwitchSensorMode==true");
            return true;
        }
        else
        {
            char const *p = mParameters.get(MtkCameraParameters::KEY_DYNAMIC_SWITCH_SENSOR_MODE);
            if(p)
            {
                MY_LOGD_IF(mEnableDebugLog, "KEY_DYNAMIC_SWITCH_SENSOR_MODE = %s", p);
                ret = (::strcmp(p, MtkCameraParameters::ON) == 0);
                MY_LOGD("KEY_DYNAMIC_SWITCH_SENSOR_MODE = %s", p);
            }
            else
            {
                ret = false;
                MY_LOGD("KEY_DYNAMIC_SWITCH_SENSOR_MODE = NULL");
            }
        }
    }
    else
    {
        MY_LOGD("dual cam mode don't support : KEY_DYNAMIC_SWITCH_SENSOR_MODE");
        ret = false;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getHighFpsCapture() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    //
    bool ret = false;
#if (1==STEREO_CAMERA_SUPPORTED)
    if( StereoSettingProvider::isDualCamMode() == false)
#else
    if(1)
#endif
    {
        int highFpsCapture = property_get_int32("vendor.camera.highfps.cap", -1);
        if(highFpsCapture == 0)
        {
            MY_LOGD("forced to set getHighFpsCapture()==false");
            return false;
        }
        else if(highFpsCapture > 0)
        {
            MY_LOGD("forced to set getHighFpsCapture()==true");
            return true;
        }
        else
        {
            char const *p = mParameters.get(MtkCameraParameters::KEY_HIGH_FPS_CAPTURE);
            if(p)
            {
                MY_LOGD_IF(mEnableDebugLog, "KEY_HIGH_FPS_CAPTURE = %s", p);
                ret = (::strcmp(p, MtkCameraParameters::ON) == 0);
                MY_LOGD("KEY_HIGH_FPS_CAPTURE = %s", p);
            }
            else
            {
                ret = false;
                MY_LOGD("KEY_HIGH_FPS_CAPTURE = NULL");
            }
        }
    }
    else
    {
        MY_LOGD("dual cam mode don't support : KEY_HIGH_FPS_CAPTURE");
        ret = false;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getNonSlowmotionPreviewLimitMaxFps(int *fps) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    //
    bool ret = false;
#if (1==STEREO_CAMERA_SUPPORTED)
    if( StereoSettingProvider::isDualCamMode() == false)
#else
    if(1)
#endif
    {
        char const *p = mParameters.get(MtkCameraParameters::KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS);
        if(p)
        {
            MY_LOGD_IF(mEnableDebugLog, "KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS = %s", p);
            mParameters.getInt(MtkCameraParameters::KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS);
            MY_LOGD("KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS = %s (%d)", p, *fps);
            ret = true;
        }
        else
        {
            ret = false;
            MY_LOGD("KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS = NULL");
        }
    }
    else
    {
        MY_LOGD("dual cam mode don't support : KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS");
        ret = false;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getLowLightCaptureUsePreviewMode() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    //
    bool ret = false;
#if (1==STEREO_CAMERA_SUPPORTED)
    if( StereoSettingProvider::isDualCamMode() == false)
#else
    if(1)
#endif
    {
        int lowLightCaptureUsePreviewMode = property_get_int32("vendor.camera.lowlv.cap", -1);
        if(lowLightCaptureUsePreviewMode == 0)
        {
            MY_LOGD("forced to set getLowLightCaptureUsePreviewMode()==false");
            return false;
        }
        else if(lowLightCaptureUsePreviewMode > 0)
        {
            MY_LOGD("forced to set getLowLightCaptureUsePreviewMode()==true");
            return true;
        }
        else
        {
            char const *p = mParameters.get(MtkCameraParameters::KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE);
            if(p)
            {
                MY_LOGD_IF(mEnableDebugLog, "KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE = %s", p);
                ret = (::strcmp(p, MtkCameraParameters::ON) == 0);
                MY_LOGD("KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE = %s", p);
            }
            else
            {
                ret = false;
                MY_LOGD("KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE = NULL");
            }
        }
    }
    else
    {
        MY_LOGD("dual cam mode don't support : KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE");
        ret = false;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getExposureTime(int *exptime) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    char const *pExpTime = mParameters.get(MtkCameraParameters::KEY_EXPOSURE_TIME);

    if(pExpTime == NULL)
    {
        *exptime = 0;
        MY_LOGD_IF(mEnableDebugLog, "Exposure Time is NULL");
    }
    else if (0 == ::strcmp(pExpTime, MtkCameraParameters::EXPOSURE_TIME_AUTO))
    {
        *exptime = mParameters.getInt(MtkCameraParameters::KEY_EXPOSURE_TIME);
        MY_LOGD_IF(mEnableDebugLog, "Exposure Time is Auto==(%s) (%d)",pExpTime,*exptime);
    }
    else
    {
        *exptime = mParameters.getInt(MtkCameraParameters::KEY_EXPOSURE_TIME);
        MY_LOGD_IF(mEnableDebugLog, "Exposure Time is %s (%d)",pExpTime,*exptime);
    }
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getDualZoomInfo(int *Main2ID, int *Supported) const
{
    // MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    // MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    if (Main2ID != NULL) {
        *Main2ID = mMain2CamID;
    }
    if (Supported != NULL) {
        *Supported = mIsDualZoomMode;
    }
}

/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getZoomRatio(int id __attribute__((unused))) const  //id only used in dual cam
{
    ssize_t index = getInt(CameraParameters::KEY_ZOOM);

    if  (index < 0)
    {
        index = 0;
    }
    else if(index >= (ssize_t)mvZoomRatios.size())
    {
        index = mvZoomRatios.size() - 1;
    }
#if DUAL_CAM
    if (id != -1)
    {
        MY_LOGD_IF(mEnableDebugLog, "WillDBG DualSupported : %d, SupCamId : %d", mIsDualZoomMode, mMain2CamID);
        if (mIsDualZoomMode && id == mMain2CamID) {
            MY_LOGD_IF(mEnableDebugLog, "WillDBG get tele zoom ratio");
            return mvZoomRatios_Sup[index];
        }
    }
#endif // DUAL_CAM

    return  mvZoomRatios[index];
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getZoomRatioByIndex(uint32_t index) const
{
    // MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    // MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    return  mvZoomRatios[index];
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDualZoomSupported() const
{
    return mIsDualZoomMode;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getRecordLikely() const
{
    return mRecordLikely;
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
setRecordLikely(bool Recording)
{
    mRecordLikely = Recording;
}



/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getRecordingHint() const
{
    return isEnabled(CameraParameters::KEY_RECORDING_HINT);
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getVideoStabilization() const
{
    return isEnabled(CameraParameters::KEY_VIDEO_STABILIZATION);
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getVideoSnapshotSupport() const
{
    return isEnabled(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED);
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getMultFrameBlending() const
{
    uint32_t mfbMode = MTK_MFB_MODE_OFF; // 0:off, 1:mfll, 2:ais

    char const* pMfb   = NULL;
    char const* pScene = NULL;
    char const *pIso   = NULL;
    String8 s8Mfb;
    String8 s8Scene;
    String8 s8Iso;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        pMfb   = __getSafeString(mParameters.get(MtkCameraParameters::KEY_MFB_MODE), s8Mfb);
        pScene = __getSafeString(mParameters.get(CameraParameters::KEY_SCENE_MODE), s8Scene);
        pIso   = __getSafeString(mParameters.get(MtkCameraParameters::KEY_ISO_SPEED), s8Iso);
    }

    if(!pMfb)
    {
        MY_LOGD_IF(mEnableDebugLog, "No KEY_MFB_MODE");
        mfbMode = MTK_MFB_MODE_OFF;
    }
    else if (0 == ::strcmp(pMfb, MtkCameraParameters::KEY_MFB_MODE_MFLL))
    {
        // default MFB off, leave for CCT MFLL
        MY_LOGD_IF(mEnableDebugLog, " MFB enable for CCT");
        mfbMode = MTK_MFB_MODE_MFLL;
    }
    else if (0 == ::strcmp(pMfb, MtkCameraParameters::KEY_MFB_MODE_AIS))
    {
        /* AIS only can be enable if only if ISO is auto mode */
        if (0 == ::strcmp(pIso, MtkCameraParameters::ISO_SPEED_AUTO))
        {
            MY_LOGD_IF(mEnableDebugLog, " MFB enable for AIS");
            mfbMode = MTK_MFB_MODE_AIS;
        }
        else
        {
            MY_LOGD_IF(mEnableDebugLog, " Discard AIS due to manual ISO");
            mfbMode = MTK_MFB_MODE_OFF;
        }
    }
    else if (0 == ::strcmp(pMfb, MtkCameraParameters::KEY_MFB_MODE_AUTO))
    {
#ifdef CUST_MFLL_AUTO_MODE
        static_assert( ((CUST_MFLL_AUTO_MODE >= MTK_MFB_MODE_OFF)&&(CUST_MFLL_AUTO_MODE < MTK_MFB_MODE_NUM)),
                       "CUST_MFLL_AUTO_MODE is invalid in custom/feature/mfnr/camera_custom_mfll.h" );

        MY_LOGD_IF(mEnableDebugLog, "CUST_MFLL_AUTO_MODE:%d", CUST_MFLL_AUTO_MODE);
        mfbMode = CUST_MFLL_AUTO_MODE;
#else
#error "CUST_MFLL_AUTO_MODE is no defined in custom/feature/mfnr/camera_custom_mfll.h"
#endif
    }
    else if (0 == ::strcmp(pMfb, MtkCameraParameters::OFF))
    {
        if  (pScene && (0 == ::strcmp(pScene, CameraParameters::SCENE_MODE_NIGHT)))
        {
            MY_LOGD_IF(mEnableDebugLog, " MFB enable for NightMode");
            mfbMode = MTK_MFB_MODE_MFLL;
        }
    }

    return mfbMode;
}

/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getCustomHint() const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

    uint32_t customHint = MTK_CUSTOM_HINT_0;

    char const* pCustomHint = mParameters.get(MtkCameraParameters::KEY_CUSTOM_HINT);
    if(!pCustomHint)
    {
        MY_LOGD("No KEY_CUSTOM_HINT");
        customHint = MTK_CUSTOM_HINT_0;
    }
    else if (0 == ::strcmp(pCustomHint, MtkCameraParameters::KEY_CUSTOM_HINT_0))
    {
        customHint = MTK_CUSTOM_HINT_0;
    }
    else if (0 == ::strcmp(pCustomHint, MtkCameraParameters::KEY_CUSTOM_HINT_1))
    {
        customHint = MTK_CUSTOM_HINT_1;
    }
    else if (0 == ::strcmp(pCustomHint, MtkCameraParameters::KEY_CUSTOM_HINT_2))
    {
        customHint = MTK_CUSTOM_HINT_2;
    }
    else if (0 == ::strcmp(pCustomHint, MtkCameraParameters::KEY_CUSTOM_HINT_3))
    {
        customHint = MTK_CUSTOM_HINT_3;
    }
    else if (0 == ::strcmp(pCustomHint, MtkCameraParameters::KEY_CUSTOM_HINT_4))
    {
        customHint = MTK_CUSTOM_HINT_4;
    }
    else
    {
        // for flexible, if custom hint value not use dedicated string
        customHint = getInt(MtkCameraParameters::KEY_CUSTOM_HINT);
    }

    MY_LOGD("%s: %d",MtkCameraParameters::KEY_CUSTOM_HINT, customHint);

    int forceCustomHint = property_get_int32("vendor.custom.hint", -1);
    if (forceCustomHint >= 0) {
        MY_LOGD("force customHint: %d",forceCustomHint);
        customHint = forceCustomHint;
    }

    return customHint;
}

/******************************************************************************
*
*******************************************************************************/
bool ParamsManager::getCShotIndicator() const
{
    return isEnabled(MtkCameraParameters::KEY_CSHOT_INDICATOR);
}

/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getVHdr() const
{
    if(mLogLevel != PARAMS_MGR_NO_LOG)
    {
        int forceSetVHDRMode = property_get_int32("vendor.forceset.vhdrmode", -1);
        if(forceSetVHDRMode != -1)
        {
            if(forceSetVHDRMode == 0)
                return SENSOR_VHDR_MODE_NONE;
            else if(forceSetVHDRMode == 1)
                return SENSOR_VHDR_MODE_IVHDR;
            else if(forceSetVHDRMode == 2)
                return SENSOR_VHDR_MODE_MVHDR;
            else if(forceSetVHDRMode == 9)
                return SENSOR_VHDR_MODE_ZVHDR;
            else{
                MY_LOGE("forceSetVHDRMode(%d) not support", forceSetVHDRMode);
                return SENSOR_VHDR_MODE_NONE;
            }
        }
    }

    const char* strSceneMode  = NULL;
    char const* strVideoHDR   = NULL;
    char const* strVHDRMode   = NULL;
    String8 s8SceneMode;
    String8 s8VideoHDR;
    String8 s8VHDRMode;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

        strSceneMode   = __getSafeString(mParameters.get(CameraParameters::KEY_SCENE_MODE), s8SceneMode);
        strVideoHDR    = __getSafeString(mParameters.get(MtkCameraParameters::KEY_VIDEO_HDR), s8VideoHDR);
        strVHDRMode    = __getSafeString(mParameters.get(MtkCameraParameters::KEY_VIDEO_HDR_MODE), s8VHDRMode);
    }

    MY_LOGD_IF(mEnableDebugLog, "VhdrMode Parameters: %s:%s, %s:%s, %s:%s",
        CameraParameters::KEY_SCENE_MODE, strSceneMode,
        MtkCameraParameters::KEY_VIDEO_HDR, strVideoHDR,
        MtkCameraParameters::KEY_VIDEO_HDR_MODE, strVHDRMode);

    if (!strSceneMode || (strcmp(strSceneMode, CameraParameters::SCENE_MODE_HDR) && strcmp(strSceneMode, CameraParameters::SCENE_MODE_AUTO)))
    {
        MY_LOGD_IF(mEnableDebugLog, "scene mode(%s) not SCENE_MODE_HDR and SCENE_MODE_AUTO", s8SceneMode.c_str());
        return SENSOR_VHDR_MODE_NONE;
    }

    if(!strVideoHDR)
    {
        MY_LOGD_IF(mEnableDebugLog, "No KEY_VIDEO_HDR");
        return SENSOR_VHDR_MODE_NONE;
    }

    if(!strVHDRMode)
    {
        MY_LOGD_IF(mEnableDebugLog, "No KEY_VIDEO_HDR_MODE");
        return SENSOR_VHDR_MODE_NONE;
    }

    //> check vhdr mode

    if(::strcmp(strVideoHDR, MtkCameraParameters::OFF) == 0)
    {
        return SENSOR_VHDR_MODE_NONE;
    }
    else
    {
        if(::strcmp(strVHDRMode, MtkCameraParameters::VIDEO_HDR_MODE_NONE) == 0)
        {
            MY_LOGW_IF(mEnableDebugLog, "No VHDR, VIDEO_HDR_MODE_NONE");
            return SENSOR_VHDR_MODE_NONE;
        }
        else if(::strcmp(strVHDRMode, MtkCameraParameters::VIDEO_HDR_MODE_IVHDR) == 0)
        {
            // FIXME currently ivHDR not ready yet. When we complete ivHDR developement, we should remove this comment.
            //return SENSOR_VHDR_MODE_IVHDR;
            MY_LOGW("IVHDR not supported yet!!");
            return SENSOR_VHDR_MODE_NONE;
        }
        else if(::strcmp(strVHDRMode, MtkCameraParameters::VIDEO_HDR_MODE_MVHDR) == 0)
        {
            return SENSOR_VHDR_MODE_MVHDR;
        }
        else if(::strcmp(strVHDRMode, MtkCameraParameters::VIDEO_HDR_MODE_ZVHDR) == 0)
        {
            return SENSOR_VHDR_MODE_ZVHDR;
        }
        else
        {
            MY_LOGE("wrong mode(%s)",strVHDRMode);
            return SENSOR_VHDR_MODE_NONE;
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getSingleFrameCaptureHDR() const
{
    return isEnabled(MtkCameraParameters::KEY_SINGLE_FRAME_CAPTURE_HDR_SUPPORTED);
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getHDRDetectionSupported() const
{
    // return the default value if parameter does not exist
    if (getStr(MtkCameraParameters::KEY_HDR_DETECTION_SUPPORTED).isEmpty())
        return false;

    return isEnabled(MtkCameraParameters::KEY_HDR_DETECTION_SUPPORTED);
}

/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getStaticVhdrMode() const
{
    MY_LOGW_IF(mEnableDebugLog, "get static vhdr mode: %s", mStaticVhdrMode.string());
    return mStaticVhdrMode;
}

/******************************************************************************
*
*******************************************************************************/
NSCam::HDRMode
ParamsManager::
getHDRMode() const
{
    if(mLogLevel != PARAMS_MGR_NO_LOG)
    {
        int forceSetHDRMode = property_get_int32("vendor.forceset.hdrmode", -1);
        if(forceSetHDRMode != -1)
        {
            if(forceSetHDRMode == 0)
                return HDRMode::OFF;
            else if(forceSetHDRMode == 1)
                return HDRMode::ON;
            else if(forceSetHDRMode == 2)
                return HDRMode::AUTO;
            else if(forceSetHDRMode == 3)
                return HDRMode::VIDEO_ON;
            else if(forceSetHDRMode == 4)
                return HDRMode::VIDEO_AUTO;
            else{
                MY_LOGE("forceSetHDRMode(%d) not support", forceSetHDRMode);
                return HDRMode::OFF;
            }
        }
    }

    // TODO: use KEY_HDR_MODE to replace KEY_HDR_AUTO_MODE and KEY_VIDEO_HDR
#if 1
    const char* strSceneMode   = NULL;
    const char* strHDRAutoMode = NULL;

    uint32_t vhdrMode = getVHdr();
    String8 s8SceneMode;
    String8 s8HDRAutoMode;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

        strSceneMode   = __getSafeString(mParameters.get(CameraParameters::KEY_SCENE_MODE), s8SceneMode);
        strHDRAutoMode = __getSafeString(mParameters.get(MtkCameraParameters::KEY_HDR_AUTO_MODE), s8HDRAutoMode);
    }

    MY_LOGD_IF(mEnableDebugLog, "HdrMode Parameters: %s:%s, %s:%s, vhdrMode:%d",
        CameraParameters::KEY_SCENE_MODE, strSceneMode,
        MtkCameraParameters::KEY_HDR_AUTO_MODE, strHDRAutoMode,
        vhdrMode);

    // scene-mode != SCENE_MODE_HDR and scene-mode != SCENE_MODE_AUTO
    if (!strSceneMode || (strcmp(strSceneMode, CameraParameters::SCENE_MODE_HDR) && strcmp(strSceneMode, CameraParameters::SCENE_MODE_AUTO)))
        return HDRMode::OFF;

    // SCENE_MODE_AUTO only allow for VHDR used
    if(!strcmp(strSceneMode, CameraParameters::SCENE_MODE_AUTO) && (vhdrMode == SENSOR_VHDR_MODE_NONE))
        return HDRMode::OFF;

    // hdr-auto-mode != on
    if (!strHDRAutoMode || strcmp(strHDRAutoMode, MtkCameraParameters::ON))
        return
            (vhdrMode == SENSOR_VHDR_MODE_NONE) ? HDRMode::ON : HDRMode::VIDEO_ON;

    // hdr-auto-mode = on
    return
        (vhdrMode == SENSOR_VHDR_MODE_NONE) ? HDRMode::AUTO : HDRMode::VIDEO_AUTO;

#else
    const char* strHDRMode =
        mParameters.get(CameraParameters::KEY_HDR_MODE);

    return
        !strHDRMode ? HDRMode::OFF :
        !strcmp(strHDRMode, MtkCameraParameters::HDR_MODE_ON) ? HDRMode::ON :
        !strcmp(strHDRMode, MtkCameraParameters::HDR_MODE_AUTO) ? HDRMode::AUTO :
        !strcmp(strHDRMode, MtkCameraParameters::HDR_MODE_VIDEO_ON) ? HDRMode::VIDEO_ON :
        !strcmp(strHDRMode, MtkCameraParameters::HDR_MODE_VIDEO_AUTO) ? HDRMode::VIDEO_AUTO :
        HDRMode::OFF;
#endif
}

/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getShotModeStr() const
{
    String8 s8ShotMode;
    char const* pScene       = NULL;
    char const* pCaptureMode = NULL;
    String8 s8Scene;
    String8 s8CaptureMode;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

        pScene       = __getSafeString(mParameters.get(CameraParameters::KEY_SCENE_MODE), s8Scene);
        pCaptureMode = __getSafeString(mParameters.get(MtkCameraParameters::KEY_CAPTURE_MODE), s8CaptureMode);
    }
    //
    if  ( pScene && 0 == ::strcmp(pScene, CameraParameters::SCENE_MODE_HDR) )
    {
        s8ShotMode = MtkCameraParameters::CAPTURE_MODE_HDR_SHOT;
        MY_LOGD_IF(mEnableDebugLog, "%s - scene(hdr)", s8ShotMode.string());
    }
    else
    if  ( pCaptureMode )
    {
        s8ShotMode = pCaptureMode;
        MY_LOGD_IF(0, "%s", s8ShotMode.string());
    }
    else
    {
        s8ShotMode = MtkCameraParameters::CAPTURE_MODE_NORMAL;
        MY_LOGD_IF(mEnableDebugLog, "%s by default - no shot mode", s8ShotMode.string());
    }
    //
    return  s8ShotMode;
}

/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getShotMode() const
{
    return  PARAMSMANAGER_MAP_INST(eMapShotMode)->valueFor(getShotModeStr());
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getFaceBeautyTouchPosition(FACE_BEAUTY_POS& pos) const
{
    char* p = NULL;
    String8 s;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        p = (char*) __getSafeString(mParameters.get(MtkCameraParameters::KEY_FB_TOUCH_POS), s);
    }

    if (p == NULL)
    {
        return;
    }
    char* q;
    pos.x = strtol(p,&q,10);
    p = q+1;
    pos.y = strtol(p,&q,10);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getFaceBeautyBeautifiedPosition(Vector<FACE_BEAUTY_POS>& pos) const
{
    char* p = NULL;
    String8 s;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        p = (char*) __getSafeString(mParameters.get(MtkCameraParameters::KEY_FB_FACE_POS), s);
    }

    if (p == NULL)
    {
        return;
    }
    char* end = p + strlen(p);
    char* q;
    FACE_BEAUTY_POS xy;
    while(p < end) {
        xy.x = strtol(p,&q,10);
        p = q+1;
        xy.y = strtol(p,&q,10);
        p = q+1;
        pos.push_back(xy);
    }
    return;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getPDAFSupported() const
{
    return isEnabled(MtkCameraParameters::KEY_PDAF);
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDNGSupported() const
{
    return isEnabled(MtkCameraParameters::KEY_DNG_SUPPORTED);
}
/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateBrightnessValue(int const iBV)
{
    set(MtkCameraParameters::KEY_BRIGHTNESS_VALUE, iBV);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewFPS(int const fps)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_FPS, fps);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewFrameInterval(int const interval)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_FRAME_INTERVAL_IN_US, interval);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getBokehPictureSize(int &width, int &height) const
{
    // Get the current string
    const char *p = NULL;
    String8 s;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        p = __getSafeString(mParameters.get(MtkCameraParameters::KEY_REFOCUS_PICTURE_SIZE), s);
    }
    if (p == 0) return;
    sscanf(p, "%dx%d", &width, &height);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewAEIndexShutterGain(int const index, int const shutter, int const isp_gain, int const sensor_gain)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SHUTTER_SPEED, shutter);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_ISP_GAIN, isp_gain);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SENSOR_GAIN, sensor_gain);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_AE_INDEX, index);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewAEIndexShutterGain(int const index, int const shutter, int const isp_gain, int const sensor_gain, int const iso)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SHUTTER_SPEED, shutter);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_ISP_GAIN, isp_gain);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SENSOR_GAIN, sensor_gain);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_AE_INDEX, index);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_ISO, iso);
}
/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateCaptureShutterGain(int const shutter, int const isp_gain, int const sensor_gain)
{
    set(MtkCameraParameters::KEY_ENG_CAPTURE_SHUTTER_SPEED, shutter);
    set(MtkCameraParameters::KEY_ENG_CAPTURE_ISP_GAIN, isp_gain);
    set(MtkCameraParameters::KEY_ENG_CAPTURE_SENSOR_GAIN, sensor_gain);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateEngMsg(char const* msg)
{
    set(MtkCameraParameters::KEY_ENG_MSG, msg);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
set(char const* key, char const* value)
{
    MY_LOGD_IF(mEnableDebugLog, "%d: + AutoWLock %s=%s", getOpenId(), key, value);
    MyRWLock::AutoWLock _lock(mRWLock, __FUNCTION__);
    mParameters.set(key, value);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
set(char const* key, int value)
{
    MY_LOGD_IF(mEnableDebugLog, "%d: + AutoWLock %s=%d", getOpenId(), key, value);
    MyRWLock::AutoWLock _lock(mRWLock, __FUNCTION__);
    mParameters.set(key, value);
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getStr(char const* key) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock %s", key);
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    char const* p = mParameters.get(key);
    return  (0 != p) ? String8(p) : String8::empty();
}


/******************************************************************************
*
*******************************************************************************/
int
ParamsManager::
getInt(char const*key) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock %s", key);
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    return mParameters.getInt(key);
}


/******************************************************************************
*
*******************************************************************************/
float
ParamsManager::
getFloat(char const* key) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock %s", key);
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
    return mParameters.getFloat(key);
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
isEnabled(char const* key) const
{
    if  ( ! key )
    {
        MY_LOGW("Null key");
        return  false;
    }
    //
    char const* p = NULL;
    String8 s;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock %s", key);
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        p = __getSafeString(mParameters.get(key), s);
    }

    if ( ! p ) {
        MY_LOGE_IF(mEnableDebugLog, "Key not found: %s", key);
        return false;
    }
    //
    return ( ::strcmp(p, CameraParameters::TRUE) != 0 ) ? false : true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
dump(int fd, Vector<String8>const& args)
{
    MY_LOGD_IF(mEnableDebugLog, "%d: + AutoWLock", getOpenId());
    MyRWLock::AutoWLock _lock(mRWLock, __FUNCTION__);
    //
    if  ( args.empty() ) {
        mParameters.dump(fd, Vector<String16>());
        MY_LOGD_IF(mEnableDebugLog, "-");
        return  OK;
    }
    //
    MY_LOGD_IF(mEnableDebugLog, "args(%zu)=%s", args.size(), (*args.begin()).string());
    //  Parse command: "s1=s2"
    if  ( args.size() == 1 )
    {
        String8 s8OutputTemp;
        String8 const& s8Arg1 = args[0];
        //  find pattern in s8Arg1: "s1=s2"
        ssize_t const AssignIndex = s8Arg1.find("=");

        //  (1) No assignment. Just show the old value.
        if  ( -1 == AssignIndex ) {
            String8 const key(s8Arg1.string());
            char const *value_old = mParameters.get(key);
            //
            s8OutputTemp = String8::format("\t%s: ", key.string());
            if  ( value_old ) {
                s8OutputTemp += value_old;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
        }
        //  (2) Assignment. Show both old/new values and then set the new value.
        else
        {
            String8 const key(s8Arg1.string(), AssignIndex);
            char const *value_old = mParameters.get(key);
            char const *value_new = s8Arg1.string() + AssignIndex + 1;
            //
            //  show old key-value
            s8OutputTemp = String8::format("\t[old] %s: ", key.string());
            if  ( value_old ) {
                s8OutputTemp += value_old;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
            //
            //  show new key-value
            s8OutputTemp = String8::format("\t[new] %s: ", key.string());
            if  ( value_new ) {
                s8OutputTemp += value_new;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
            //
            //  set new key-value
            mParameters.set(key, value_new);
        }
    }
    //
    MY_LOGD_IF(mEnableDebugLog, "-");
    return  OK;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDisplayRotationSupported() const
{
    char const* p = NULL;
    String8 s;
    {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);
        p = __getSafeString(mParameters.get(MtkCameraParameters::KEY_DISPLAY_ROTATION_SUPPORTED), s);
    }
    //
    if ( ! p ) {
        MY_LOGI_IF(mEnableDebugLog, "No KEY_DISPLAY_ROTATION_SUPPORTED");
        return false;
    }
    //
    MY_LOGD_IF(mEnableDebugLog, "KEY_DISPLAY_ROTATION_SUPPORTED = %s", p);
    const int CAM_MODE = [&] {
        MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
        MyRWLock::AutoRLock _lock(mRWLock, "getDisplayRotationSupported(CAM_MODE)");
        return mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    }();
    bool ret = (::strcmp(p, CameraParameters::TRUE) == 0) && CAM_MODE != MtkCameraParameters::CAMERA_MODE_NORMAL;
    if(  ret &&
        (ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_DEFAULT ||
         ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO ||
         ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO ||
         ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD ||
         ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_DUALCAM ))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    //
    int panelWidth = 0, panelHeight = 0;
    getPanelSize(&panelWidth, &panelHeight);
    if( panelWidth > 0 &&
        panelHeight > 0)
    {
        int prvWidth = 0, prvHeight = 0;
        getPreviewSize(&prvWidth, &prvHeight);
        //
        if( panelWidth  != prvWidth &&
            panelHeight != prvHeight)
        {
            MY_LOGD_IF(mEnableDebugLog, "Panel(%d x %d) != Preview(%d x %d)",
                    panelWidth,
                    panelHeight,
                    prvWidth,
                    prvHeight);
            ret = false;
        }
    }
    //
    MY_LOGD_IF(mEnableDebugLog, "getDisplayRotationSupported ret = %d (KEY_CAMERA_MODE=%d)(ms8ClientAppMode=%s)", ret ? 1 : 0, CAM_MODE, ms8ClientAppMode.string());
    //
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
setDisplayRotationSupported(bool isSupport)
{
    //Don't add write lock here, it will unlock the write lock in setParameters
    mParameters.set(MtkCameraParameters::KEY_DISPLAY_ROTATION_SUPPORTED, isSupport ? CameraParameters::TRUE : CameraParameters::FALSE);
    MY_LOGD_IF(1, "setDisplayRotationSupported = %s", isSupport ? CameraParameters::TRUE : CameraParameters::FALSE);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getPanelSize(int *width, int *height) const
{
    MY_LOGD_IF(mEnableDebugLog, "AutoRLock");
    MyRWLock::AutoRLock _lock(mRWLock, __FUNCTION__);

    char* p = (char*) mParameters.get(MtkCameraParameters::KEY_PANEL_SIZE);
    if (p == NULL)
    {
        return;
    }
    char* q;
    *width = strtol(p,&q,10);
    p = q+1;
    *height = strtol(p,&q,10);
}


/******************************************************************************
*
*******************************************************************************/
// Parse string like "640x480" or "10000,20000"
int
ParamsManager::
parse_pair(const char *str, int *first, int *second, char delim,
                      char **endptr)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);
    // If a delimeter does not immediately follow, give up.
    if (*end != delim) {
        ALOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = (int)strtol(end+1, &end, 10);

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
parseSizesList(const char *sizesStr, Vector<Size> &sizes)
{
    if (sizesStr == 0) {
        return;
    }

    char *sizeStartPtr = (char *)sizesStr;

    while (true) {
        int width, height;
        int success = parse_pair(sizeStartPtr, &width, &height, 'x',
                                 &sizeStartPtr);
        if (success == -1 || (*sizeStartPtr != ',' && *sizeStartPtr != '\0')) {
            ALOGE("Picture sizes string \"%s\" contains invalid character.", sizesStr);
            return;
        }
        sizes.push(Size(width, height));

        if (*sizeStartPtr == '\0') {
            return;
        }
        sizeStartPtr++;
    }
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
AddPreviewSizeForPanel()
{
    AutoParamsMgrProfileUtil profile(LOG_TAG, "AddPreviewSizeForPanel");

    //Don't add write lock here, it will unlock the write lock in setParameters

    bool bDisplayRotationSupported = false;
    //
    //TODO: replace with getDisplayRotationSupported?
    char const *p = mParameters.get(MtkCameraParameters::KEY_DISPLAY_ROTATION_SUPPORTED);
    if(p)
    {
        MY_LOGD_IF(mEnableDebugLog, "KEY_DISPLAY_ROTATION_SUPPORTED = %s", p);
        const int CAM_MODE = mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        bool ret = ((::strcmp(p, CameraParameters::TRUE) == 0) && CAM_MODE != MtkCameraParameters::CAMERA_MODE_NORMAL);
        if(  ret &&
            (ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_DEFAULT ||
             ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO ||
             ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO ||
             ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD ||
             ms8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_DUALCAM ))
        {
            bDisplayRotationSupported = true;
        }
        else
        {
            bDisplayRotationSupported = false;
        }
        MY_LOGD_IF(mEnableDebugLog, "DisplayRotationSupported ret = %d (KEY_CAMERA_MODE=%d)(ms8ClientAppMode=%s)", bDisplayRotationSupported ? 1 : 0, CAM_MODE, ms8ClientAppMode.string());
    }
    //
    if(bDisplayRotationSupported)
    {
        if( !mIsAppendPreviewSize )
        {
            int width=0;
            int height=0;
            getPanelSize(&width,&height);
            MY_LOGD_IF(mEnableDebugLog, "Get Panel sizes : %dx%d", width,height);
            if( width != 0 && height != 0 )
            {
                String8 supportedPreviewSizes = String8(mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES));
                MY_LOGD_IF(mEnableDebugLog, "supportedPreviewSizes : %s", supportedPreviewSizes.string());
                //
                int newWidth1 = ceil(((float)height * 16 / 9) / 16) * 16;  // Based on panel size height, for 16:9 and align width to 16x
                int newHight1 = ceil((float)height / 8) * 8;               // Based on panel size height, for 16:9 and align hight to 16x
                //
                int newWidth2 = ceil(((float)height * 4 / 3) / 16) * 16;   // Based on panel size height, for 4:3 and align width to 16x
                int newHight2 = ceil((float)height / 8) * 8;               // Based on panel size height, for 4:3 and align hight to 8x
                //
                // For special panel size (not 16:9 or not 4:3)
                float ratioRange = 0.001f;
                int newWidth3 = 0 ;
                int newHight3 = 0;
                if(fabs((float)width/height - ((float)16 / 9)) > ratioRange &&
                   fabs((float)width/height - ((float)4 / 3)) > ratioRange )
                {
                    newWidth3 = ceil((float)width / 16) * 16;   // align width to 16x
                    newHight3 = ceil((float)height / 8) * 8;    // hight to 8x
                }
                //
                char str1[30];
                sprintf(str1, "%dx%d", newWidth1,newHight1);
                MY_LOGD_IF(mEnableDebugLog, "prev size 1 : %s", str1);
                char str2[30];
                sprintf(str2, "%dx%d", newWidth2,newHight2);
                MY_LOGD_IF(mEnableDebugLog, "prev size 2 : %s", str2);
                char str3[30];
                sprintf(str3, "%dx%d", newWidth3,newHight3);
                MY_LOGD_IF(mEnableDebugLog, "prev size 3 : %s", str3);
                //
                Vector<Size> previewSizes;
                parseSizesList(supportedPreviewSizes.string(),previewSizes);
                int diffRange = 0;
                bool existNewSize1 = false;
                bool existNewSize2 = false;
                bool existNewSize3 = (newWidth3==0 && newHight3==0) ? true : false;
                for(size_t i=0; i<previewSizes.size(); i++)
                {
                    if(!existNewSize1 &&
                        abs(previewSizes[i].width-newWidth1) <= diffRange &&
                        abs(previewSizes[i].height-newHight1) <= diffRange )
                    {
                        existNewSize1 = true;
                    }
                    if(!existNewSize2 &&
                        abs(previewSizes[i].width-newWidth2) <= diffRange &&
                        abs(previewSizes[i].height-newHight2) <= diffRange )
                    {
                        existNewSize2 = true;
                    }
                    if(!existNewSize3 &&
                        abs(previewSizes[i].width-newWidth3) <= diffRange &&
                        abs(previewSizes[i].height-newHight3) <= diffRange )
                    {
                        existNewSize3 = true;
                    }
                    if(existNewSize1 && existNewSize2 && existNewSize3)
                        break;
                }
                if(!existNewSize1)
                {
                    supportedPreviewSizes += ",";
                    supportedPreviewSizes += (String8)str1;
                    MY_LOGD_IF(mEnableDebugLog, "add prev size 1");
                }
                if(!existNewSize2)
                {
                    supportedPreviewSizes += ",";
                    supportedPreviewSizes += (String8)str2;
                    MY_LOGD_IF(mEnableDebugLog, "add prev size 2");
                }
                if(!existNewSize3)
                {
                    supportedPreviewSizes += ",";
                    supportedPreviewSizes += (String8)str3;
                    MY_LOGD_IF(mEnableDebugLog, "add prev size 3");
                }
                //
                //
                mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, supportedPreviewSizes);
                //
                MY_LOGD_IF(mEnableDebugLog, "New Supported preview sizes : %s", mParameters.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES));
                //
                mIsAppendPreviewSize = true;
            }
            else
            {
                setDisplayRotationSupported(false);
                MY_LOGW("DisplayRotationSupported = true but didn't set panel size");
            }
        }
    }

}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDefaultPreviewFps(int& fps)
{
    fps = mDefaultPreviewFps;
    return true;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getDefaultPreviewFpsRange(int& minFps, int& maxFps)
{
    minFps = mDefaultPreviewFpsRange[0];
    maxFps = mDefaultPreviewFpsRange[1];
    return true;
}

