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

#define LOG_TAG "test/TestDepthMap_Common"

#include <time.h>
#include <gtest/gtest.h>

#include <vector>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/drv/IHalSensor.h>
#include <featurePipe/core/include/SyncUtil.h>
#include <featurePipe/core/include/CamGraph.h>

#include "../../DepthMapFactory.h"
#include "TestDepthMap_Common.h"
#include "ImgParamSetting.h"
#include "../../DepthMapEffectRequest.h"
using namespace NSCam::v1::Stereo;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
typedef IImageBufferAllocator::ImgParam ImgParam;

#define LOG_TAG "MtkCam/DepthPipeUT"

IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->loadFromFile(path);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}

IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage, MBOOL is_gb)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}

MSize getRRZOMain1Size()
{
    return MSize(960, 720);
}

android::Mutex WaitingListener::sMutex;
int WaitingListener::sCBCount = 0;
int WaitingListener::sDoneCount = 0;

MVOID WaitingListener::CB(MVOID* tag, ResultState state, sp<IDualFeatureRequest>& request)
{
  android::Mutex::Autolock lock(sMutex);

  ++WaitingListener::sCBCount;
  MY_LOGD("++CB=%d", WaitingListener::sCBCount);
  if( state == eRESULT_COMPLETE || state == eRESULT_DEPTH_NOT_READY)
  {
    ++WaitingListener::sDoneCount;
    MY_LOGD("++Done=%d", WaitingListener::sDoneCount);
  }
}

void WaitingListener::resetCounter()
{
  android::Mutex::Autolock lock(sMutex);
  WaitingListener::sCBCount = 0;
  WaitingListener::sDoneCount = 0;
  MY_LOGD("resetCounter()");
}


bool WaitingListener::waitRequest(unsigned int targetTimes, unsigned int timeout_sec)
{
  timespec t;

  if(targetTimes<=0)
      return true;

  // divide to ten loop
  int interval = 10;

  float time_slice = timeout_sec*1.0/interval;
  int waitSec = floor(time_slice);
  long waitNSec = (time_slice - waitSec ) * pow(10, 9);

  MY_LOGD("waitRequest timeout_sec=%d  waitSec=%d waitNSec= %d",timeout_sec, waitSec, waitNSec);
  do
  {
    {
      android::Mutex::Autolock lock(sMutex);
      if( WaitingListener::sDoneCount >= targetTimes )
      {

        MY_LOGD("waitRequest: return true!!");
        return true;
      }
    }

    t.tv_sec = waitSec;
    t.tv_nsec = waitNSec;
    if( nanosleep(&t, NULL) != 0 )
    {
      break;
    }
  }while( interval-- );

  MY_LOGD("waitRequest: return false!!");
  return false;
}


bool
WaitingListener::
waitRequestAtLeast(unsigned int targetTimes, unsigned int timeout_sec, float least_wait_sec)
{
    timespec t;
    // divide to ten loop
    int interval = 10;

    float time_slice = timeout_sec*1.0/interval;
    int waitSec = floor(time_slice);
    long waitNSec = (time_slice - waitSec ) * pow(10, 9);

    MY_LOGD("waitRequest timeout_sec=%d  waitSec=%d waitNSec= %d least_wait_sec=%f",timeout_sec, waitSec, waitNSec, least_wait_sec);
    do
    {
        {
            MY_LOGD("==> interval=%d (10-interval)*time_slice =%f ", interval, (10-interval)*time_slice);
            android::Mutex::Autolock lock(sMutex);
            MY_LOGD("WaitingListener::sDoneCount=%d targetTimes=%d", WaitingListener::sDoneCount, targetTimes);
            if( WaitingListener::sDoneCount >= targetTimes &&
                (10-interval)*time_slice >= least_wait_sec)
            {
                MY_LOGD("waitRequest: return true!!");
                return true;
            }
        }

        t.tv_sec = waitSec;
        t.tv_nsec = waitNSec;
        if( nanosleep(&t, NULL) != 0 )
        {
          break;
        }
    }while( interval-- );

    MY_LOGD("waitRequest: return false!!");
    return false;
}

MVOID finishCB_forListener(bool status)
{
    if(status)
    {
        sp<IDualFeatureRequest> pRequest = nullptr;
        WaitingListener::CB(NULL, eRESULT_COMPLETE, pRequest);
    }
}

UTEnvironmenSetup::
UTEnvironmenSetup(char* username, Profile profile)
: mvSensorIndex{999, 999}
, mProfile(profile)
{
    mUsername = username;
    //
    MAKE_HalLogicalDeviceList()->searchDevices();
    //
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    mSensorCount = pHalSensorList->queryNumberOfSensors();
    MY_LOGD("mSensorCount  :%d", mSensorCount);
    if(mSensorCount <= 2)
    {
        mIsReadyToUT = MFALSE;
        MY_LOGE("Failed to init the environment setup! Sensor count=%d", mSensorCount);
        return;
    }

    //
    StereoSettingProvider::setLogicalDeviceID(3);
    StereoSettingProvider::setStereoFeatureMode(profile.featureMode);
    StereoSettingProvider::setStereoModuleType(profile.moduleType);
    StereoSettingProvider::setImageRatio(profile.imageRatio);
    //
    mIsReadyToUT = powerOnSensor();
    DepthPipeLoggingSetup::mbProfileLog = MTRUE;
    #ifdef GTEST_PROFILE
    DepthPipeLoggingSetup::mbDebugLog = MFALSE;
    #else
    DepthPipeLoggingSetup::mbDebugLog = MTRUE;
    #endif

    MY_LOGD("mIsReadyToUT: %d", mIsReadyToUT);
    if(!mIsReadyToUT)
        MY_LOGE("Failed to init the environment setup!");


}

UTEnvironmenSetup::
~UTEnvironmenSetup()
{
    MY_LOGD("+");
    // close sensor
    for(int i = 0; i < 2; i++)
    {
        if(mvSensorIndex[i] >= 0)
        {
            powerOffSensor(mvSensorIndex[i]);
        }
    }

    if(mpHalSensor_Main1)
    {
        mpHalSensor_Main1->destroyInstance(mUsername);
        mpHalSensor_Main1 = NULL;
    }

    if(mpHalSensor_Main2)
    {
        mpHalSensor_Main2->destroyInstance(mUsername);
        mpHalSensor_Main2 = NULL;
    }
    MY_LOGD("-");
}

bool
UTEnvironmenSetup::
powerOnSensor()
{
    MY_LOGD("+");
    if(mSensorCount < 2)
    {
        MY_LOGE("mSensorCount:%d  < 2", mSensorCount);
        return false;
    }

    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    mvSensorIndex[0] = main1Idx;
    mvSensorIndex[1] = main2Idx;
    MY_LOGD("mvSensorIndex =%d %d", mvSensorIndex[0], mvSensorIndex[1]);

    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        return false;
    }
    //
    mpHalSensor_Main1 = pHalSensorList->createSensor( mUsername,mvSensorIndex[0]);
    mpHalSensor_Main2 = pHalSensorList->createSensor( mUsername,mvSensorIndex[1]);
    if(mpHalSensor_Main1 == NULL)
    {
       MY_LOGE("mpHalSensor Main1 is NULL");
       return false;
    }
    if(mpHalSensor_Main2 == NULL)
    {
       MY_LOGE("mpHalSensor Main2 is NULL");
       return false;
    }
    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    if( !mpHalSensor_Main1->powerOn(mUsername, 1, &mvSensorIndex[0]) )
    {
        MY_LOGE("sensor main1 power on failed: %d", mvSensorIndex[0]);
        return false;
    }
    if( !mpHalSensor_Main2->powerOn(mUsername, 1, &mvSensorIndex[1]) )
    {
        MY_LOGE("sensor main2 power on failed: %d", mvSensorIndex[1]);
        powerOffSensor(mvSensorIndex[0]);
        return false;
    }

    MY_LOGD("-");
    return true;
}

bool
UTEnvironmenSetup::
powerOffSensor(MUINT index)
{
    if(index == mvSensorIndex[0])
    {
        if( !mpHalSensor_Main1->powerOff(mUsername, 1, &mvSensorIndex[0]) )
        {
            MY_LOGE("sensor main1 power off failed: %d", index);
            return false;
        }
    }
    else if(index == mvSensorIndex[1])
    {
        if( !mpHalSensor_Main2->powerOff(mUsername, 1, &mvSensorIndex[1]) )
        {
            MY_LOGE("sensor main2 power off failed: %d", index);
            return false;
        }
    }

    return true;
}

MBOOL setupUTDepthMapPipe(
    MBOOL bIsBayerMono,
    sp<DepthMapPipeSetting>& rSetting,
    sp<DepthMapPipeOption>& rOption,
    sp<DepthMapFlowOption>& rFlowOption,
    DepthMapFlowType flowType
)
{
    MINT32 vSensorIndex[2];
    StereoSettingProvider::getStereoSensorIndex(vSensorIndex[0], vSensorIndex[1]);

    SeneorModuleType moduleType;
    if(bIsBayerMono)
        moduleType = BAYER_AND_MONO;
    else
        moduleType = BAYER_AND_BAYER;

    rSetting = new DepthMapPipeSetting();
    rSetting->miSensorIdx_Main1 = vSensorIndex[0];
    rSetting->miSensorIdx_Main2 = vSensorIndex[1];
    rSetting->mszRRZO_Main1 = MSize(1440, 1080);
    rOption = new DepthMapPipeOption(moduleType, eDEPTHNODE_MODE_VSDOF, flowType);

    if(!queryDepthMapFlowOption(rSetting, rOption, nullptr, rFlowOption))
    {
        MY_LOGE("Failed to query flow option!!");
        return MFALSE;
    }
    return MTRUE;
}

MVOID copyMeata(DepthMapRequestPtr& pRequest, BufferIOType ioType, DepthMapBufferID srcID, DepthMapBufferID tarID)
{
    IMetadata* pMeta = nullptr;
    MBOOL bRet = pRequest->getRequestMetadata({.bufferID=srcID,
                                        .ioType=ioType}, pMeta);

    if(bRet)
    {
        IMetadata* pCopyMeta = new IMetadata(*pMeta);
        pRequest->pushRequestMetadata({.bufferID=tarID,
                                        .ioType=ioType}, pCopyMeta);
    }
}

MBOOL setupReqMetadata(MBOOL eisON, sp<IDepthMapEffectRequest> pRequest)
{
    IMetadata* pMetadata;
    // InAppMeta
    pMetadata = new IMetadata();
    pRequest->pushRequestMetadata({BID_META_IN_APP, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // InAppMeta - EIS on
    if(eisON)
    {
        trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE,
                                MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON);
    }
    else
    {
        trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE,
                                MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF);
    }
    trySetMetadata<MINT32>(pMetadata, MTK_JPEG_ORIENTATION, 0);
    trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_AF_TRIGGER, 0);
    trySetMetadata<MINT32>(pMetadata, MTK_STEREO_FEATURE_DOF_LEVEL, 0);
    trySetMetadata<MINT64>(pMetadata, MTK_SENSOR_EXPOSURE_TIME, 100);

    // InHalMeta Main1
    pMetadata = new IMetadata();
    pRequest->pushRequestMetadata({BID_META_IN_HAL_MAIN1, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // InHalMeta-EIS region
    if(eisON)
    {
        MSize rrzoSize = getRRZOMain1Size();
        IMetadata::IEntry entry(MTK_EIS_REGION);
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(rrzoSize.w, Type2Type< MINT32 >());
        entry.push_back(rrzoSize.h, Type2Type< MINT32 >());
        // the following is  MVtoCenterX, MVtoCenterY, IsFromRRZ
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(0, Type2Type< MINT32 >());
        entry.push_back(MTRUE, Type2Type< MBOOL >());
        pMetadata->update(MTK_EIS_REGION, entry);
    }
    // magic num
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    // sensor mode
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_SENSOR_MODE, SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
    // conv offset
    trySetMetadata<MFLOAT>(pMetadata, MTK_CONVERGENCE_DEPTH_OFFSET, 0);
    // timestamp
    trySetMetadata<MINT64>(pMetadata, MTK_P1NODE_FRAME_START_TIMESTAMP, 99999999);
    // unique key
    trySetMetadata<MINT32>(pMetadata, MTK_PIPELINE_UNIQUE_KEY, 555555);
    // InHalMain2
    pMetadata = new IMetadata();
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    pRequest->pushRequestMetadata({BID_META_IN_HAL_MAIN2, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // RetMeta
    pMetadata = new IMetadata();
    pRequest->pushRequestMetadata({BID_META_IN_P1_RETURN, eBUFFER_IOTYPE_INPUT}, pMetadata);
    trySetMetadata<MINT32>(pMetadata, MTK_SENSOR_SENSITIVITY, 100);
    // OutAppMeta BID_META_OUT_APP
    pMetadata = new IMetadata();
    pRequest->pushRequestMetadata({BID_META_OUT_APP, eBUFFER_IOTYPE_OUTPUT}, pMetadata);
    // OutHalMeta
    pMetadata = new IMetadata();
    pRequest->pushRequestMetadata({BID_META_OUT_HAL, eBUFFER_IOTYPE_OUTPUT}, pMetadata);

    return MTRUE;
}

MVOID releaseMetadata(std::vector<sp<IDepthMapEffectRequest>> vRequestVec)
{
    IMetadata* pMetadata;

    for(auto pRequest : vRequestVec)
    {
        if(pRequest->getRequestMetadata({BID_META_IN_APP, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_APP, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_IN_HAL_MAIN1, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_HAL_MAIN1, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_IN_HAL_MAIN2, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_HAL_MAIN2, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_OUT_APP, eBUFFER_IOTYPE_OUTPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_OUT_APP, eBUFFER_IOTYPE_OUTPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_OUT_HAL, eBUFFER_IOTYPE_OUTPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_OUT_HAL, eBUFFER_IOTYPE_OUTPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_IN_APP_QUEUED, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_APP_QUEUED, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_IN_HAL_MAIN1_QUEUED, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_HAL_MAIN1_QUEUED, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_IN_HAL_MAIN2_QUEUED, eBUFFER_IOTYPE_INPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_IN_HAL_MAIN2_QUEUED, eBUFFER_IOTYPE_INPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_OUT_APP_QUEUED, eBUFFER_IOTYPE_OUTPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_OUT_APP_QUEUED, eBUFFER_IOTYPE_OUTPUT});
            delete pMetadata;
        }

        if(pRequest->getRequestMetadata({BID_META_OUT_HAL_QUEUED, eBUFFER_IOTYPE_OUTPUT}, pMetadata))
        {
            pRequest->popRequestMetadata({BID_META_OUT_HAL_QUEUED, eBUFFER_IOTYPE_OUTPUT});
            delete pMetadata;
        }
    }

}

MVOID loadFullResizRawsImgBuf(
    sp<IImageBuffer>& rpFSMain1ImgBuf,
    sp<IImageBuffer>& rpFSMain2ImgBuf,
    sp<IImageBuffer>& rpRSMain1ImgBuf,
    sp<IImageBuffer>& rpRSMain2ImgBuf,
    sp<IImageBuffer>& rpRectInMain2ImgBuf
)
{
    const char * sMain1Filename = "/sdcard/vsdof_data/BID_P2A_IN_RSRAW1_1808_reqID_12__960x720_10_0.raw";
    const char * sMain2Filename = "/sdcard/vsdof_data/BID_P2A_IN_RSRAW2_1808_reqID_12__960x720_10_0.raw";
    //const char * sMain1FSFilename = "/sdcard/vsdof_data/BID_P2A_IN_FSRAW1_4640x3488_5800.raw";

    MUINT32 bufStridesInBytes[3] = {1808, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamRRZO_Main1 = IImageBufferAllocator::ImgParam((eImgFmt_FG_BAYER10), getRRZOMain1Size(), bufStridesInBytes, bufBoundaryInBytes, 1);

    MUINT32 bufStridesInBytes2[3] = {1808, 0, 0};
    IImageBufferAllocator::ImgParam imgParamRRZO_Main2 = IImageBufferAllocator::ImgParam((eImgFmt_FG_BAYER10), getRRZOMain1Size(), bufStridesInBytes2, bufBoundaryInBytes, 1);

    MUINT32 bufStridesInBytesFS[3] = {5800, 0, 0};
    MINT32 bufBoundaryInBytesFS[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamIMGO = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(4640, 3488), bufStridesInBytesFS, bufBoundaryInBytesFS, 1);

    rpRSMain1ImgBuf = createImageBufferFromFile(imgParamRRZO_Main1, sMain1Filename, "RsRaw1",  eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
    rpRSMain2ImgBuf = createImageBufferFromFile(imgParamRRZO_Main2, sMain2Filename, "RsRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
    //rpFSMain1ImgBuf = createImageBufferFromFile(imgParamIMGO, sMain1FSFilename, "FsRaw",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpFSMain1ImgBuf = createEmptyImageBuffer(imgParamIMGO, "FsRaw",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    //
    const char * sRect1Filename = "/sdcard/vsdof_data/BID_P2A_OUT_RECT_IN1_reqID_12__368x480_8_s0.yuy2";
    IImageBufferAllocator::ImgParam p1YUV_param = getImgParam_P1PVYUV();

    //rpRectInMain2ImgBuf = createImageBufferFromFile(p1YUV_param, sRect1Filename, "p1yuv", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpRectInMain2ImgBuf = createEmptyImageBuffer(p1YUV_param, "p1yuvv", eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
}


MVOID loadCaptureYUVImgBuf(
    sp<IImageBuffer>& rpMain1YUVImgBuf,
    sp<IImageBuffer>& rpMain2YUVImgBuf
)
{
    const char * sMain1Filename = "/sdcard/vsdof_data/BID_P2A_IN_RSRAW1_1808_reqID_12__960x720_10_0.raw";
    const char * sMain2Filename = "/sdcard/vsdof_data/BID_P2A_IN_RSRAW2_1808_reqID_12__960x720_10_0.raw";

    IImageBufferAllocator::ImgParam captureYUV1Param = getImgParam_CaptureYUVMain1();
    rpMain1YUVImgBuf = createEmptyImageBuffer(captureYUV1Param, "Main1YUVCap", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);

    IImageBufferAllocator::ImgParam captureYUV2Param = getImgParam_CaptureYUVMain2();
    rpMain2YUVImgBuf = createEmptyImageBuffer(captureYUV2Param, "Main2YUVCap", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
}

MBOOL prepareReqInputBuffer(sp<IDepthMapEffectRequest> pRequest)
{
    sp<IImageBuffer> buf_fs_Main1, buf_fs_Main2, buf_rs_Main1, buf_rs_Main2, buf_rect1;
    loadFullResizRawsImgBuf(buf_fs_Main1, buf_fs_Main2, buf_rs_Main1, buf_rs_Main2, buf_rect1);
    // prepare input frame info: RSRAW1
    pRequest->pushRequestImageBuffer({BID_P2A_IN_RSRAW1, eBUFFER_IOTYPE_INPUT}, buf_rs_Main1);
    // prepare input frame info: RSRAW2
    pRequest->pushRequestImageBuffer({BID_P2A_IN_RSRAW2, eBUFFER_IOTYPE_INPUT}, buf_rs_Main2);
    // prepare input frame info: FSRAW1
    pRequest->pushRequestImageBuffer({BID_P2A_IN_FSRAW1, eBUFFER_IOTYPE_INPUT}, buf_fs_Main1);
    // prepare p1 yuv
    pRequest->pushRequestImageBuffer({BID_P2A_IN_RECT_IN1, eBUFFER_IOTYPE_INPUT}, buf_rect1);


    return MTRUE;
}


MBOOL prepareReqInputBuffer_Capture(sp<IDepthMapEffectRequest> pRequest)
{
    sp<IImageBuffer> buf_yuv_Main1, buf_yuv_Main2;
    loadCaptureYUVImgBuf(buf_yuv_Main1, buf_yuv_Main2);
    // prepare input frame info: Main1 YUV
    pRequest->pushRequestImageBuffer({BID_P2A_IN_YUV1, eBUFFER_IOTYPE_INPUT}, buf_yuv_Main1);
    // prepare input frame info: Main2 YUV
    pRequest->pushRequestImageBuffer({BID_P2A_IN_YUV2, eBUFFER_IOTYPE_INPUT}, buf_yuv_Main2);
    return MTRUE;
}

MBOOL prepareReqOutputBuffer(
    DepthMapPipeOpState eState,
    sp<IDepthMapEffectRequest> pRequest
)
{
    // prepare output frame: MV_F
    sp<IImageBuffer> buf;

    if(eState == eSTATE_CAPTURE)
    {
        ImgParam imgParam_depthMap = getImgParam_DEPTHMAP();
        buf = createEmptyImageBuffer(imgParam_depthMap, "imgParam_depthMap",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        pRequest->pushRequestImageBuffer({PBID_OUT_DEPTHMAP, eBUFFER_IOTYPE_OUTPUT},buf);

    }
    else
    {
        ImgParam imgParam_MainImageCAP = getImgParam_MV_F_CAP();
        ImgParam imgParam_MainImage = getImgParam_MV_F();

        buf = createEmptyImageBuffer(imgParam_MainImage, "MainImg",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        pRequest->pushRequestImageBuffer({BID_P2A_OUT_MV_F, eBUFFER_IOTYPE_OUTPUT},buf);

        ImgParam imgParam_FD = getImgParam_FD();
        buf = createEmptyImageBuffer(imgParam_FD, "FD", eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE |  eBUFFER_USAGE_SW_WRITE_RARELY);
        pRequest->pushRequestImageBuffer({BID_P2A_OUT_FDIMG, eBUFFER_IOTYPE_OUTPUT},buf);
        //
        ImgParam imgParam_DMBG = getImgParam_DMBG();
        buf = createEmptyImageBuffer(imgParam_DMBG, "imgParam_DMBG", eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN);
        pRequest->pushRequestImageBuffer({BID_GF_OUT_DMBG, eBUFFER_IOTYPE_OUTPUT},buf);

    }

    return MTRUE;
}

MVOID finalizeTheNodes(std::vector<DepthMapPipeNode*> vNodes)
{
    MY_LOGD("finalizeTheNodes +");
    for(auto pNode : vNodes)
    {
        pNode->setDataFlow(MFALSE);
    }
    MY_LOGD("setDataFlow to false");
    CountDownLatch counter(vNodes.size());
    android::sp<NotifyCB> flushCB = new CounterCBWrapper(&counter);
    for(auto pNode : vNodes)
    {
        pNode->flush(flushCB);
    }
    counter.wait();
    MY_LOGD("flushed all nodes");
    android::sp<CountDownLatch> counter2 = new CountDownLatch(vNodes.size());
    for(auto pNode : vNodes)
    {
        pNode->registerSyncCB(counter2);
    }
    counter2->wait();
    MY_LOGD("synced all nodes");
    for(auto pNode : vNodes)
    {
        pNode->registerSyncCB(NULL);
        MY_LOGD("Node:%d stop!!", pNode->getName());
        pNode->stop();
        MY_LOGD("Node:%d uninit!!", pNode->getName());
        pNode->uninit();
    }
    MY_LOGD("finalizeTheNodes -");
}