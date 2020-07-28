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
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#include "TestDepthMap_Common.h"


typedef IImageBufferAllocator::ImgParam ImgParam;


StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();
MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};

ImgParam getImgParam_FD()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    MSize P2AFM_FD_IMG_SIZE = pass2SizeInfo.areaIMG2O;
    MUINT32 FD_STRIDES[3] = {P2AFM_FD_IMG_SIZE.w*2, 0, 0};
    return ImgParam(eImgFmt_YUY2, P2AFM_FD_IMG_SIZE, FD_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_MV_F()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    MSize VSDOF_MV_F_SIZE = pass2SizeInfo.areaWDMA.size;

    MUINT32 MAINIMG_STRIDES[3] = {VSDOF_MV_F_SIZE.w, VSDOF_MV_F_SIZE.w/2, VSDOF_MV_F_SIZE.w/2};

    return ImgParam(eImgFmt_YV12, VSDOF_MV_F_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_MV_F_CAP()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    MSize VSDOF_MV_F_CAP_SIZE = pass2SizeInfo.areaWDMA.size;

    MUINT32 MAINIMG_STRIDES[3] = {VSDOF_MV_F_CAP_SIZE.w, VSDOF_MV_F_CAP_SIZE.w/2, VSDOF_MV_F_CAP_SIZE.w/2};

    return ImgParam(eImgFmt_YV12, VSDOF_MV_F_CAP_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_WARPING_MATRIX()
{
    MUINT32 iWarpingMatrixSize = StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes();

    MUINT32 MAINIMG_STRIDES[3] = {iWarpingMatrixSize, 0, 0};

    return ImgParam(eImgFmt_Y8, MSize(iWarpingMatrixSize, 1), MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_DMP()
{
    MSize szDMP_SIZE = gpSizePrvder->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
    // Y16 : stride 1 = w*2
    MUINT32 MAINIMG_STRIDES[3] = {szDMP_SIZE.w*2, 0, 0};

    return ImgParam(eImgFmt_Y16, szDMP_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_DMW()
{
    MSize DMW_SIZE = gpSizePrvder->getBufferSize(E_DMW, eSTEREO_SCENARIO_RECORD);
    MUINT32 DMW_STRIDES[3] = {DMW_SIZE.w, 0, 0};

    return ImgParam(eImgFmt_Y8, DMW_SIZE, DMW_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_MY_S()
{
    MSize MY_S_SIZE = gpSizePrvder->getBufferSize(E_MY_S, eSTEREO_SCENARIO_RECORD);
    MUINT32 MY_S_STRIDES[3] = {MY_S_SIZE.w, MY_S_SIZE.w/2 , MY_S_SIZE.w/2};
    return ImgParam(eImgFmt_YV12, MY_S_SIZE, MY_S_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_DMBG()
{
    MSize DMBG_SIZE = gpSizePrvder->getBufferSize(E_DMBG, eSTEREO_SCENARIO_RECORD);
    MUINT32 DMBG_STRIDES[3] = {DMBG_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DMBG_SIZE, DMBG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_DEPTHMAP()
{
    MSize DEPTHMAP_SIZE = gpSizePrvder->getBufferSize(E_DEPTH_MAP, eSTEREO_SCENARIO_CAPTURE);
    MUINT32 DEPTHMAP_STRIDES[3] = {DEPTHMAP_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DEPTHMAP_SIZE, DEPTHMAP_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_JPSMain()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    MSize JPS_SIZE = pass2SizeInfo.areaWDMA.size;
    MUINT32 JPS_STRIDES[3] = {JPS_SIZE.w, JPS_SIZE.w/2, JPS_SIZE.w/2};
    return ImgParam(eImgFmt_YV12, JPS_SIZE, JPS_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_LDC()
{
    MSize LDC_SIZE = gpSizePrvder->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE);
    MUINT32 LDC_STRIDES[3] = {LDC_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, LDC_SIZE, LDC_STRIDES, FULL_RAW_BOUNDARY, 3);
}

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



android::Mutex WaitingListener::sMutex;
int WaitingListener::sCBCount = 0;
int WaitingListener::sDoneCount = 0;
String8 WaitingListener::sDoneStatus = String8(DEPTHMAP_COMPLETE_KEY);
Vector<sp<EffectRequest> > WaitingListener::vDoneReqVec;


MVOID WaitingListener::CB(MVOID *tag, String8 status, sp<EffectRequest> &request)
{
  android::Mutex::Autolock lock(sMutex);

  ++WaitingListener::sCBCount;
  MY_LOGD("++CB=%d", WaitingListener::sCBCount);
  if( status == sDoneStatus )
  {
    ++WaitingListener::sDoneCount;
    WaitingListener::vDoneReqVec.add(request);
    MY_LOGD("++Done=%d", WaitingListener::sDoneCount);
  }
}

void WaitingListener::resetCounter()
{
  android::Mutex::Autolock lock(sMutex);

  WaitingListener::sCBCount = 0;
  WaitingListener::sDoneCount = 0;
  WaitingListener::vDoneReqVec.clear();
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

MVOID ut_finishCB(bool status)
{
    if(status)
    {
        sp<EffectRequest> mEffReq = NULL;
        WaitingListener::CB(NULL, String8(DEPTHMAP_COMPLETE_KEY), mEffReq);
    }
}

