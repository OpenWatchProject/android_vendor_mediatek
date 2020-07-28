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
    return mvIImageBufData.add(key, value);
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
addMetadata(const BMDeNoiseBufferID& key, IMetadata* const value)
{
    return mvMetaData.add(key, value);
}

ssize_t
ImageBufInfoMap::
delIImageBuffer(const BMDeNoiseBufferID& key)
{
    return mvIImageBufData.removeItem(key);
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
delMetadata(const BMDeNoiseBufferID& key)
{
    return mvMetaData.removeItem(key);
}

const sp<IImageBuffer>&
ImageBufInfoMap::
getIImageBuffer(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvIImageBufData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvIImageBufData.indexOfKey(key) >= 0) ? mvIImageBufData.valueFor(key) : nullptr;
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

IMetadata*
ImageBufInfoMap::
getMetadata(const BMDeNoiseBufferID& key)
{
    MY_LOGW_IF(mvMetaData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvMetaData.indexOfKey(key) >= 0) ? mvMetaData.valueFor(key) : nullptr;
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

} //NSFeaturePipe
} //NSCamFeature
} //NSCam