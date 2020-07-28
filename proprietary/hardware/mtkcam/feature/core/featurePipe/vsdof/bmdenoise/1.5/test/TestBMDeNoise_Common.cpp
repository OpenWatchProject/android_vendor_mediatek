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
#include "TestBMDeNoise_Common.h"

#define USER_NAME "BMDNPipeUT"

#define MY_LOGGER(fmt, arg...) \
    CAM_LOGD("[%s][%s]" fmt, USER_NAME, __func__, ##arg); \
    printf("[D][%s][%s]" fmt"\n", USER_NAME, __func__, ##arg);

typedef IImageBufferAllocator::ImgParam ImgParam;


StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();

ImgParam getImgParam_out_YUV()
{
    MSize SIZE(3072, 1728);
    int STRIDES[2] = {SIZE.w, SIZE.w};
    int BOUNDARY[2] = {0, 0};
    return ImgParam(eImgFmt_NV21, SIZE, STRIDES, BOUNDARY, 2);
}

ImgParam getImgParam_out_Thumb()
{
    MSize SIZE(160, 90);
    int STRIDES[2] = {SIZE.w, SIZE.w};
    int BOUNDARY[2] = {0, 0};
    return ImgParam(eImgFmt_NV21, SIZE, STRIDES, BOUNDARY, 2);
}

IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->loadFromFile(path);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}

IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}


android::Mutex WaitingListener::sMutex;
int WaitingListener::sCBCount = 0;
int WaitingListener::sDoneCount = 0;
String8 WaitingListener::sDoneStatus = String8(BMDENOISE_COMPLETE_KEY);
Vector<sp<PipeRequest> > WaitingListener::vDoneReqVec;


MVOID WaitingListener::CB(MVOID *tag, String8 status, sp<PipeRequest> &request)
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

	for(int i = 0 ; i < timeout_sec ; i ++){
		{
			android::Mutex::Autolock lock(sMutex);
			MY_LOGGER("waitRequest CB(%d/%d)  Sec:(%d/%d)", WaitingListener::sDoneCount, targetTimes, i, timeout_sec);

			if(WaitingListener::sDoneCount >= targetTimes){
				MY_LOGGER("waitRequest Wait DONE!");
				return true;
			}
		}
        usleep(1000*1000);
    }

	MY_LOGGER("waitRequest timeout!");
	return false;
}