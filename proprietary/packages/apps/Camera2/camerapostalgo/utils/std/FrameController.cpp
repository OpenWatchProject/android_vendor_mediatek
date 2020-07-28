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

#define LOG_TAG "MtkCam/FrameController"
//
#include <cutils/properties.h>
#include <utils/def/common.h>
#include <utils/std/Log.h>
#include <utils/std/FrameController.h>
using namespace android;
using namespace NSCam::Utils;


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s][%s] " fmt, __FUNCTION__, getName().string(), ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOG_ID(cond, ...)        do { \
                                        if(cond) \
                                        { \
                                            MY_LOGI(__VA_ARGS__);\
                                        }\
                                        else \
                                        { \
                                            MY_LOGD(__VA_ARGS__); \
                                        } \
                                    }while(0)

/******************************************************************************
*
*******************************************************************************/
FrameController::
FrameController(String8 userName)
    : mName(userName)
    , miLogLevel(1)
    , miDisplayControlStage(1)
    , mAverageDisplayTime(0)
    , mAverageCount(0)
    , miFirstDelayTime(0)
    , miAdjDelay(0)
    , miP2interval(0)
    , mbFirstReturnFrame(true)
    , mnLastFrameTime(0)
    , mnFrameWaitTime(0)
    , mnCurrentFrameTime(0)
    , mnLastEnqueSystemTime(0)
    , mnOldDisplayDelayTime(0)
    , mnNewDisplayDelayTime(0)
    , mnMinAdjDisplay(0)
    , mnMaxAdjDisplay(0)
    , mbApplyFrameControl(false)
    , mnTimeInterval(0)
    , mnFrameInterval(0)
    , mnFrameMaxPlusDelay(30000000) //ns
    , mnFrameMaxSleep(1000000000)   //ns
    , mnFrameMinSleep(200)          //us
    , mnAdjSleepTime(250000)        //ns
    , mbFrameControlReset(false)
    , mbFrameControlAdj(false)
{
    int32_t iLogLevel = property_get_int32("debug.camera.fctrl.loglevel", -1);
    if(iLogLevel != -1)
    {
        miLogLevel = iLogLevel;
    }

    int64_t iFrameMaxDelay = property_get_int32("debug.camera.fctrl.fmd", -1);
    if(iFrameMaxDelay != -1)
    {
        mnFrameMaxPlusDelay = iFrameMaxDelay * 1000000;
    }

    int64_t iFrameMaxSleep = property_get_int32("debug.camera.fctrl.smaxms", -1);
    if(iFrameMaxSleep != -1)
    {
        mnFrameMaxSleep = iFrameMaxSleep * 1000000;
    }

    int64_t iFrameMinSleep = property_get_int32("debug.camera.fctrl.sminus", -1);
    if(iFrameMinSleep != -1)
    {
        mnFrameMinSleep = iFrameMinSleep;
    }

    int64_t iAdjSleepTime = property_get_int32("debug.camera.fctrl.adjsleep", -1);
    if(iAdjSleepTime != -1)
    {
        mnAdjSleepTime = iAdjSleepTime;
    }
    MY_LOGI("- miLogLevel(%d), max delay(%lld), max sleep(%lld ns), min sleep(%lld us), adj sleep(%lld ns)"
            , miLogLevel, mnFrameMaxPlusDelay, mnFrameMaxSleep, mnFrameMinSleep, mnAdjSleepTime);
}

/******************************************************************************
*
*******************************************************************************/
FrameController::
~FrameController()
{
}

/******************************************************************************
*
*******************************************************************************/
String8
FrameController::
getName()
{
    return mName;
}

/******************************************************************************
*
*******************************************************************************/
void
FrameController::
bufferControl(nsecs_t frameTime)
{
    mnCurrentFrameTime = frameTime;
    switch(miDisplayControlStage)
    {
        case 1:
            {
                MY_LOGD("framecontrol stage(%d)",miDisplayControlStage);
                nsecs_t currentTime = systemTime();
                nsecs_t currentFrameTime = mnCurrentFrameTime;
                nsecs_t currentDisplayDelay = currentTime - currentFrameTime;
                mAverageDisplayTime += currentDisplayDelay;
                mAverageCount += 1;
                if(mAverageCount == 5)
                {
                    mAverageDisplayTime/=5;
                    miDisplayControlStage = 2;
                    MY_LOGD("mAverageDisplayTime(%lld)",mAverageDisplayTime);
                }
            }
            break;
        case 2:
            {
                MY_LOGD("framecontrol stage(%d)",miDisplayControlStage);
                nsecs_t currentTime = systemTime();
                nsecs_t currentDisplayDelay = currentTime - mnCurrentFrameTime;
                MY_LOGD("currentDisplayDelay(%lld),mAverageDisplayTime(%lld)",currentDisplayDelay,mAverageDisplayTime);
                if(currentDisplayDelay<=mAverageDisplayTime)
                {
                    //int64_t middleDisplayDelay=0;;
                    mnLastFrameTime = mnCurrentFrameTime;
                    miFirstDelayTime = property_get_int32("debug.displaydelay.ms", 15);
                    miAdjDelay = property_get_int32("debug.displaydelay.adjdelay", 5000000);
                    miP2interval = property_get_int32("debug.displaydelay.p2", 30000000);
                    MY_LOGI("FirstFrameDelay(%lld)ms",miFirstDelayTime);
                    miFirstDelayTime = miFirstDelayTime * 1000; // ms to us
                    usleep(miFirstDelayTime);
                    MY_LOGI("FirstFrameDelay(%lld)ms",miFirstDelayTime);
                    miFirstDelayTime = miFirstDelayTime*1000;
                    miDisplayControlStage = 3;
                }
            }
            break;
        case 3:
            {
                nsecs_t currentTime = systemTime();
                nsecs_t currentFrameTime = mnCurrentFrameTime;
                //mnTimeInterval = currentTime - mnLastEnqueSystemTime;
                nsecs_t lastFrameTimeBK = mnLastFrameTime;
                mnFrameInterval = currentFrameTime - mnLastFrameTime;
                mnLastFrameTime = currentFrameTime;
                //
                mnOldDisplayDelayTime = mnFrameInterval + miP2interval + miFirstDelayTime;
                mnNewDisplayDelayTime = mnOldDisplayDelayTime + mnFrameMaxPlusDelay;
                mnMaxAdjDisplay = mnOldDisplayDelayTime + miAdjDelay;
                mnMinAdjDisplay = mnOldDisplayDelayTime;
                // use max delay reset method
                nsecs_t currentDisplayDelay = currentTime - currentFrameTime;
                nsecs_t currentDisplayDelayAfterSleep;
                // time delay range base on frame rate


                if(!mbFrameControlReset)
                {
                    bool doSleep = false;
                    int64_t sleepus = 0;
                    mnTimeInterval = systemTime() - mnLastEnqueSystemTime;
                    mnFrameWaitTime = (mnFrameInterval) - (mnTimeInterval);
                    int64_t mnWaitTime = mnFrameWaitTime - mnAdjSleepTime;
                    if (mnWaitTime > 0 && mnWaitTime < mnFrameMaxSleep)
                    {
                        sleepus = mnWaitTime / 1000;
                        if(mbFrameControlAdj)
                        {
                            sleepus = sleepus - 1000;
                        }
                        // only do sleep when sleep more than 500us
                        if (sleepus > mnFrameMinSleep)
                        {
                            doSleep = true;
                        }
                    }
                    if(!doSleep)
                    {
                        sleepus = 0;
                    }
                    currentDisplayDelayAfterSleep = currentDisplayDelay + (sleepus*1000);
                    // if reach to max display delay, disable frame control
                    if(currentDisplayDelayAfterSleep >= mnNewDisplayDelayTime)
                    {
                        mbFrameControlReset = true;
                        mbFrameControlAdj = false;
                        MY_LOG_ID((2<=miLogLevel),"start to reset frame delay");
                    }

                    if(!mbFrameControlReset)
                    {
                        // if display delay reach to defined value, start to sleep less for next frame to decrease display delay
                        if(!mbFrameControlAdj)
                        {
                            //MY_LOGD("currentDisplayDelayAfterSleep(%lld),mnMaxAdjDisplay(%lld),dynamicDelayTimeAdjust(%lld),mnMinAdjDisplay(%lld)",
                            //        currentDisplayDelayAfterSleep,mnMaxAdjDisplay,dynamicDelayTimeAdjust,mnMinAdjDisplay);
                            if(currentDisplayDelayAfterSleep >= mnMaxAdjDisplay)
                            {
                                mbFrameControlAdj = true;
                                MY_LOG_ID((2<=miLogLevel),"start to adj frame delay");
                            }
                        }
                        else
                        {
                            if(currentDisplayDelayAfterSleep <= mnMinAdjDisplay)
                            {
                                mbFrameControlAdj = false;
                                MY_LOG_ID((2<=miLogLevel),"stop adj frame delay");
                            }
                        }
                    }
                    if(doSleep)
                    {
                        MY_LOG_ID((2<=miLogLevel),"start sleep");
                        usleep(sleepus);
                    }
                    MY_LOG_ID((2<=miLogLevel),"off:ct(%lld),cft(%lld),cddt(%lld),oddt(%lld),nddt(%lld),max(%lld),min(%lld),fi(%lld),ti(%lld),stc(%d),st(%lld us)",
                                                currentTime,currentFrameTime,currentDisplayDelayAfterSleep,
                                                mnOldDisplayDelayTime,mnNewDisplayDelayTime, mnMaxAdjDisplay, mnMinAdjDisplay,
                                                mnFrameInterval,mnTimeInterval,mbFrameControlAdj,sleepus);
                    if (mnWaitTime >=  mnFrameMaxSleep)
                    {
                        MY_LOGW("Check Frame conrol parameters, something wrong");
                        MY_LOGW("ct(%lld),cft(%lld),lft(%lld),cddt(%lld),nddt(%lld),fi(%lld),ti(%lld)",
                                                currentTime,currentFrameTime,lastFrameTimeBK,currentDisplayDelay+(sleepus*1000),
                                                mnNewDisplayDelayTime,mnFrameInterval,mnTimeInterval);
                    }
                }
                else
                {
                    MY_LOG_ID((2<=miLogLevel)," on:ct(%lld),cft(%lld),cddt(%dd),oddt(%lld),fi(%lld),ti(%lld)",
                                                currentTime, currentFrameTime, currentDisplayDelay,
                                                mnOldDisplayDelayTime,mnFrameInterval, mnTimeInterval);

                    if(currentDisplayDelay <= ((mnOldDisplayDelayTime+mnNewDisplayDelayTime)/2))
                    {
                        mbFrameControlReset = false;
                        mbFrameControlAdj = false;
                        MY_LOG_ID((2<=miLogLevel),"stop reset frame delay");
                    }
                }
            }
            break;
        default:
            MY_LOGE("Display control stage error, should not happened");
            break;
    }
    mnLastEnqueSystemTime = systemTime();
}
