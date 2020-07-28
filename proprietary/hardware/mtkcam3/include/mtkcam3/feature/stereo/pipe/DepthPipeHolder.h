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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/**
 * @file DepthPipeHolder.h
 * @brief singleton holder for DepthMap Pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_INTERFACE_SINGLETON_DEPTH_MAP_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_INTERFACE_SINGLETON_DEPTH_MAP_PIPE_H_

// Standard C header file
#include <atomic>
#include <mutex>
// Android system/core header file
#include <utils/RefBase.h>
#include <utils/RWLock.h>
#include <utils/Condition.h>
#include <utils/StrongPointer.h>
// mtkcam custom header file
// mtkcam global header file
// Module header file
// Local header file
#include "IDepthMapPipe.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::sp;

class SmartDepthMapPipe;
/**
 * @class DepthPipeHolder
 * @brief singleton interface for depth pipe
 */
class DepthPipeHolder
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static sp<SmartDepthMapPipe> getInstance();
    static sp<SmartDepthMapPipe> waitInstance();
    static sp<SmartDepthMapPipe> createPipe(
                                android::sp<DepthMapPipeSetting> pSetting,
                                android::sp<DepthMapPipeOption> pPipeOption);

    static void clearHolder();
private:
    static std::atomic<SmartDepthMapPipe*> mpInstance;
    static android::Condition mCond;
    static android::Mutex mMutex;
};

/**
 * @class SmartDepthMapPipe
 * @brief SmartDepthMapPipe
 */
class SmartDepthMapPipe
: public IDepthMapPipe
, public android::RefBase
{
    friend class DepthPipeHolder;
public:
    virtual ~SmartDepthMapPipe() {};
    MBOOL destroyInstance() { return MTRUE;}
private:
    static sp<SmartDepthMapPipe> createInstance(
                                android::sp<DepthMapPipeSetting> pSetting,
                                android::sp<DepthMapPipeOption> pPipeOption);

    void onLastStrongRef(const void* id);
};


}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif
