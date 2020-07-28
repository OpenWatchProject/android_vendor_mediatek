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

#include "BMDeNoisePipe_Common.h"

// Module header file
#define PIPE_MODULE_TAG "BMDeNoisePipe"
#define PIPE_CLASS_TAG "Common"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

/*******************************************************************************
* ImageBufInfoMap
********************************************************************************/
ImageBufInfoMap::
ImageBufInfoMap(sp<PipeRequest> ptr)
:mpReqPtr(ptr)
{

}

ssize_t
ImageBufInfoMap::
addIImageBuffer(const BMDeNoiseBufferID& key, const sp<IImageBuffer>& value)
{
    sp<IImageBuffer> spBuf = value;
    mvIImageBufData[key] = spBuf;
    return mvIImageBufData.size();
}

ssize_t
ImageBufInfoMap::
addSmartBuffer(const BMDeNoiseBufferID& key, const SmartImageBuffer& value)
{
    return mvSmartImgBufData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addGraphicBuffer(const BMDeNoiseBufferID& key, const SmartGraphicBuffer& value)
{
    return mvGraImgBufData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addMfllBuffer(const BMDeNoiseBufferID& key, const sp<mfll::IMfllImageBuffer>& value)
{
    sp<mfll::IMfllImageBuffer> spBuf = value;
    mvMfllImgBufData[key] = spBuf;
    return mvMfllImgBufData.size();
}

ssize_t
ImageBufInfoMap::
addMetadata(const BMDeNoiseBufferID& key, IMetadata* const value)
{
    return mvMetaPtrData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addSolidMetadata(const BMDeNoiseBufferID& key, IMetadata const value)
{
    return mvMetaData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addExtBufName(const BMDeNoiseBufferID& key, const string& value)
{
    return mvExtNameSet.add(key, value);
}

ssize_t
ImageBufInfoMap::
delIImageBuffer(const BMDeNoiseBufferID& key)
{
    return mvIImageBufData.erase(key);
}

ssize_t
ImageBufInfoMap::
delSmartBuffer(const BMDeNoiseBufferID& key)
{
    return mvSmartImgBufData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delGraphicBuffer(const BMDeNoiseBufferID& key)
{
    return mvGraImgBufData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delMfllBuffer(const BMDeNoiseBufferID& key)
{
    return mvMfllImgBufData.erase(key);
}

ssize_t
ImageBufInfoMap::
delMetadata(const BMDeNoiseBufferID& key)
{
    return mvMetaPtrData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delSolidMetadata(const BMDeNoiseBufferID& key)
{
    return mvMetaData.removeItem(key);
}

const sp<IImageBuffer>&
ImageBufInfoMap::
getIImageBuffer(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvIImageBufData.count(key) <= 0, "%d: key not found", key);
    return mvIImageBufData.count(key)>0? mvIImageBufData[key] : nullptr;
}

const SmartImageBuffer&
ImageBufInfoMap::
getSmartBuffer(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvSmartImgBufData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvSmartImgBufData.indexOfKey(key) >= 0) ? mvSmartImgBufData.valueFor(key) : nullptr;
}

const SmartGraphicBuffer&
ImageBufInfoMap::
getGraphicBuffer(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvGraImgBufData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvGraImgBufData.indexOfKey(key) >= 0) ? mvGraImgBufData.valueFor(key) : nullptr;
}

const sp<mfll::IMfllImageBuffer>&
ImageBufInfoMap::
getMfllBuffer(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvMfllImgBufData.count(key) <= 0, "%d: key not found", key);
    return mvMfllImgBufData.count(key)>0? mvMfllImgBufData[key] : nullptr;
}

IMetadata*
ImageBufInfoMap::
getMetadata(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvMetaPtrData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvMetaPtrData.indexOfKey(key) >= 0) ? mvMetaPtrData.valueFor(key) : nullptr;
}

IMetadata
ImageBufInfoMap::
getSolidMetadata(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvMetaData.indexOfKey(key) < 0, "%d: key not found", key);
    return mvMetaData.valueFor(key);
}

bool
ImageBufInfoMap::
getExtBufName(const BMDeNoiseBufferID& key, string& rString)
{
    MY_LOGW_IF(mvExtNameSet.indexOfKey(key) < 0, "%d: key not found", key);

    if(mvExtNameSet.indexOfKey(key) >= 0){
        rString = mvExtNameSet.valueFor(key);
        return true;
    }else{
        return false;
    }
}
/*******************************************************************************
* ScopeLogger
********************************************************************************/
ScopeLogger::
ScopeLogger(const char* text1, const char* text2)
  : mText1(text1)
  , mText2(text2)
{
    if(mText2 == nullptr){
        CAM_LOGD("[%s] +", mText1);
    }else{
        CAM_LOGD("[%s][%s] +", mText1, mText2);
    }
}

ScopeLogger::
~ScopeLogger()
{
    if(mText2 == nullptr){
        CAM_LOGD("[%s] -", mText1);
    }else{
        CAM_LOGD("[%s][%s] -", mText1, mText2);
    }
}
/*******************************************************************************
* SimpleTimer
********************************************************************************/
SimpleTimer::
SimpleTimer()
{}

SimpleTimer::
SimpleTimer(bool bStartTimer)
{
    if(bStartTimer)
        start = std::chrono::system_clock::now();
}

MBOOL
SimpleTimer::
startTimer()
{
    start = std::chrono::system_clock::now();
    return MTRUE;
}

float
SimpleTimer::
countTimer()
{
    std::chrono::time_point<std::chrono::system_clock> cur = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cur-start;
    return elapsed_seconds.count()*1000;
}
/*******************************************************************************
* FOVCropCalculator
********************************************************************************/
FOVCropCalculator::FOVCropCalculator(MBOOL isToCropMain) : mIsToCropMain(isToCropMain)
{
    StereoSettingProvider::getStereoCameraFOV(mMain1FOV, mMain2FOV);
    setRatio(StereoSettingProvider::imageRatio());
    mBaseline = StereoSettingProvider::getStereoBaseline();

    mModuleConfig = ::getSensorModuleConfig();
    mWorkingRangeNear = ::getWorkingRangeNear();
    mAdditionalToleranceCoeff = ::getAdditionalToleranceCoeff();
    mRelRotToleranceXY = ::getRelRotToleranceXY();
}

string FOVCropCalculator::toString()
{
    static char* moduleConfigString[] = {"MainAtBottom", "MainAtTop", "MainAtLeft", "MainAtRight"};

    std::ostringstream stringStream;
    stringStream << "cropMain: " << mIsToCropMain << ", ";
    stringStream << "main1FOV: " << "(" << mMain1FOV.fov_horizontal << ", " << mMain1FOV.fov_vertical << ")" << ", ";
    stringStream << "main1FOV: " << "(" << mMain2FOV.fov_horizontal << ", " << mMain2FOV.fov_vertical << ")" << ", ";
    stringStream << "baseline: " << mBaseline << ", ";
    stringStream << "workingRangeNear: " << mWorkingRangeNear << ", ";
    stringStream << "ratio: " << "(" << mOutputWidthFactor << ", " << mOutputHeightFactor << ")" << ", ";
    stringStream << "additionalToleranceCoeff: " << mAdditionalToleranceCoeff << ", ";
    stringStream << "relRotToleranceXY: " << mRelRotToleranceXY << ", ";
    stringStream << "moduleConfig: " << moduleConfigString[static_cast<int>(mModuleConfig)];
    return stringStream.str();
}

MRect FOVCropCalculator::getResultRegion(const MSize& srcImgSize)
{
    // Crop Main or Crop Sub
    float toCropFoV_W = degree2Radian((mIsToCropMain) ? mMain1FOV.fov_horizontal : mMain2FOV.fov_horizontal);
    float toCropFoV_H = degree2Radian((mIsToCropMain) ? mMain1FOV.fov_vertical : mMain2FOV.fov_vertical);
    float anotherFoV_W = degree2Radian((mIsToCropMain) ? mMain2FOV.fov_horizontal : mMain1FOV.fov_horizontal);
    float anotherFoV_H = degree2Radian((mIsToCropMain) ? mMain2FOV.fov_vertical : mMain1FOV.fov_vertical);

    //[To-Be-Cropped Camera] is at WHERE of [Another Camera]
    SensorModuleConfig cameraToCropAtBottom = (mIsToCropMain) ? SensorModuleConfig::MainAtBottom : SensorModuleConfig::MainAtTop;
    SensorModuleConfig cameraToCropAtTop = (mIsToCropMain) ? SensorModuleConfig::MainAtTop : SensorModuleConfig::MainAtBottom;
    SensorModuleConfig cameraToCropAtLeft = (mIsToCropMain) ? SensorModuleConfig::MainAtLeft : SensorModuleConfig::MainAtRight;
    SensorModuleConfig cameraToCropAtRight = (mIsToCropMain) ? SensorModuleConfig::MainAtRight : SensorModuleConfig::MainAtLeft;

    // Cropping Margin at 4 edges
    float croppedLeft = 0.0f;
    float croppedRight = 0.0f;
    float croppedTop = 0.0f;
    float croppedBottom = 0.0f;
    float topBottomCroppedTan, leftRightCroppedTan;
    float offsetX, offsetY, usageW, usageH;

    // After-Ratio-Cropping Width and Height
    float unitW, unitH, commonUnit;
    float additionalCropW, additionalCropH;

    const float relRotToleranceXY = degree2Radian(mRelRotToleranceXY);
    // (1) Compute Cropped Margin by [Relative Rotations] at Infinity
    topBottomCroppedTan = -std::min(0.0f, std::tan(anotherFoV_H/2.0f - relRotToleranceXY) - std::tan(toCropFoV_H/2.0f)*(1.0f));
    leftRightCroppedTan = -std::min(0.0f, std::tan(anotherFoV_W/2.0f - relRotToleranceXY) - std::tan(toCropFoV_W/2.0f)*(1.0f));

    croppedLeft += leftRightCroppedTan;
    croppedRight += leftRightCroppedTan;
    croppedTop += topBottomCroppedTan;
    croppedBottom += topBottomCroppedTan;

    // (2) Compute Cropped Margins by [Parallax] for Working Range : Near
    if ( mModuleConfig == cameraToCropAtBottom )
        croppedRight = std::max(croppedRight, -std::tan(anotherFoV_W/2.0f - relRotToleranceXY) + mBaseline/mWorkingRangeNear + std::tan(toCropFoV_W/2.0f));

    else if ( mModuleConfig == cameraToCropAtTop )
        croppedLeft = std::max(croppedLeft, -std::tan(anotherFoV_W/2.0f- relRotToleranceXY) + mBaseline/mWorkingRangeNear + std::tan(toCropFoV_W/2.0f));

    else if ( mModuleConfig == cameraToCropAtLeft )
        croppedBottom = std::max(croppedBottom, -std::tan(anotherFoV_H/2.0f - relRotToleranceXY) + mBaseline/mWorkingRangeNear + std::tan(toCropFoV_H/2.0f));

    else if ( mModuleConfig == cameraToCropAtRight )
        croppedTop = std::max(croppedTop, -std::tan(anotherFoV_H/2.0f - relRotToleranceXY) + mBaseline/mWorkingRangeNear + std::tan(toCropFoV_H/2.0f));

    // (3) Convert Unit From Tangent (Normalized Unit) to Pixel
    croppedLeft = croppedLeft/(2.0f*std::tan(toCropFoV_W/2.0f))*srcImgSize.w;
    croppedRight = croppedRight/(2.0f*std::tan(toCropFoV_W/2.0f))*srcImgSize.w;
    croppedTop = croppedTop/(2.0f*std::tan(toCropFoV_H/2.0f))*srcImgSize.h;
    croppedBottom = croppedBottom/(2.0f*std::tan(toCropFoV_H/2.0f))*srcImgSize.h;

    // (4) Do Rounding : 2-align, and apply an [Additional Tolerance Coefficient]
    croppedLeft = std::ceil(croppedLeft*mAdditionalToleranceCoeff/2.0f)*2.0f;
    croppedRight = std::ceil(croppedRight*mAdditionalToleranceCoeff/2.0f)*2.0f;
    croppedTop = std::ceil(croppedTop*mAdditionalToleranceCoeff/2.0f)*2.0f;
    croppedBottom = std::ceil(croppedBottom*mAdditionalToleranceCoeff/2.0f)*2.0f;

    // (5) Compute Offset and Usage
    offsetX = croppedLeft;
    offsetY = croppedTop;
    usageW = srcImgSize.w - croppedLeft - croppedRight;
    usageH = srcImgSize.h - croppedTop - croppedBottom;

    // (6) Find the To-Be-Cropped Side
    unitW = usageW/(mOutputWidthFactor*2.0f);
    unitH = usageH/(mOutputHeightFactor*2.0f);
    commonUnit = floor(std::min(unitW, unitH));

    // (7) Find How many pixel to be cropped additionally for RATIO Cropping
    additionalCropW = usageW - commonUnit*(mOutputWidthFactor*2.0f);
    additionalCropH = usageH - commonUnit*(mOutputHeightFactor*2.0f);

    // (8) Apply half additional crop to offset, that is,
    //     half additionalCropW for Left&Right and half additionalCropH for Top&Bottom
    //     Also,  Pass the Parameters Outt
    MRect result;
    result.p.x = offsetX + additionalCropW/2.0f;
    result.p.y = offsetY + additionalCropH/2.0f;
    result.s.w = usageW - additionalCropW;
    result.s.h = usageH - additionalCropH;
    return result;
}

inline void FOVCropCalculator::setRatio(float wFactor, float hFactor)
{
    mOutputWidthFactor = wFactor;
    mOutputHeightFactor =  hFactor;
}

inline void FOVCropCalculator::setRatio(ENUM_STEREO_RATIO eRatio)
{
    switch(eRatio)
    {
        case eRatio_16_9:
            mOutputWidthFactor = 16.0f;
            mOutputHeightFactor =  9.0f;
            break;
        case eRatio_4_3:
            mOutputWidthFactor = 4.0f;
            mOutputHeightFactor =  3.0f;
            break;
        default:
            MY_LOGW("unknow ENUM_STEREO_RATIO, %d, current ratio: (%d, %d)", eRatio, mOutputWidthFactor, mOutputHeightFactor);
            break;
    }
}

void FOVCropCalculator::setRatio(const MSize& size)
{
    const float ratio_4_3 = 1.33f;
    const float ratio_16_9 = 1.77f;

    float ratio = ((float)size.w)/size.h;
    float diff_4_3 = abs(ratio - ratio_4_3);
    float diff_16_9 = abs(ratio - ratio_16_9);

    if(diff_4_3 <= diff_16_9){
        mOutputWidthFactor  =  4.0f;
        mOutputHeightFactor =  3.0f;
    }else{
        mOutputWidthFactor  = 16.0f;
        mOutputHeightFactor =  9.0f;
    }
    MY_LOGD("ratio(%f) diff(%f,%f) ret(%.0f_%.0f)", ratio, diff_4_3, diff_16_9, mOutputWidthFactor, mOutputHeightFactor);
}

inline void FOVCropCalculator::setModuleConfig(SensorModuleConfig moduleConfig)
{
    mModuleConfig = moduleConfig;
}

inline void FOVCropCalculator::setWorkingRangeNear(float workingRangeNear)
{
    mWorkingRangeNear = workingRangeNear;
}

inline void FOVCropCalculator::setRelRotToleranceXY(float relRotToleranceXY)
{
    mRelRotToleranceXY = relRotToleranceXY;
}

inline void FOVCropCalculator::setIsToCropMain(MBOOL isToCropMain)
{
    mIsToCropMain = isToCropMain;
}

inline void FOVCropCalculator::setbaseLine(float baseLine)
{
    mBaseline = baseLine;
}

inline void FOVCropCalculator::setSensorFOV(const SensorFOV& main1FOV, const SensorFOV& main2FOV)
{
    mMain1FOV = main1FOV;
    mMain2FOV = main2FOV;
}

inline float FOVCropCalculator::degree2Radian(float degree)
{
    return  degree / 180 * M_PI;
}
/*******************************************************************************
* StopWatchCollection
********************************************************************************/
android::sp<IStopWatch>
StopWatchCollection::
GetStopWatch(MINT32 type, const std::string name)
{
    android::sp<ProfileUtil> pProfileUtil = new ProfileUtil("StopWatchCollection", name.c_str());
    switch(type)
    {
        case eStopWatchType::eSTOPWATCHTYPE_AUTO:
            mStopWatches.push_back(pProfileUtil);
            return new AutoStopWatch(pProfileUtil);
            break;
        case eStopWatchType::eSTOPWATCHTYPE_MANUAL:
            mStopWatches.push_back(pProfileUtil);
            return new ManualStopWatch(pProfileUtil);
            break;
        default:
            // error handle
            MY_LOGW("unknown stop watch type: %d", type);
            break;
    }

    return NULL;
}

void
StopWatchCollection
::PrintLog()
{
    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: " << mName << " Req#: " << mRequestID << std::endl;
    stringStream << "************************************" << std::endl;
    for(auto w : mStopWatches)
    {
        stringStream <<  w->getMessage() << " " << w->getDurationInMillisecond() << " ms" << std::endl;
    }
    stringStream << "************************************";

    __android_log_print(ANDROID_LOG_DEBUG, "STPWR", "%s", stringStream.str().c_str());
}

void
StopWatchCollection
::BeginStopWatch(const std::string name, void* cookie)
{
    auto iter = mProfileUtilTable.find(cookie);
    if(iter != mProfileUtilTable.end())
    {
        MY_LOGW("%d: key exist", cookie);
    }
    else
    {
        android::sp<ProfileUtil> pProfileUtil = new ProfileUtil("StopWatchCollection", name.c_str());
        mProfileUtilTable.insert(pair<void*, android::sp<ProfileUtil>>(cookie, pProfileUtil));
        pProfileUtil->beginProfile();
    }
}

void
StopWatchCollection
::EndStopWatch(void* cookie)
{
    auto iter = mProfileUtilTable.find(cookie);
    if(iter == mProfileUtilTable.end())
    {
        MY_LOGW("%d: key not found", cookie);
    }
    else
    {
        iter->second->endProfile(false);
        mStopWatches.push_back(iter->second);
        mProfileUtilTable.erase(iter);
    }
}

} //NSFeaturePipe
} //NSCamFeature
} //NSCam