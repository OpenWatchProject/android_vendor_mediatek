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

#include <mtkcam/utils/std/Log.h>
#include <utils/String8.h>
#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
 #include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <camera_custom_stereo.h>
#include <vector>

#include "../../flowOption/DepthMapFlowOption.h"
#include "../../DepthMapPipeNode.h"

using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;

#define MY_LOGV(fmt, arg...)                  CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)                  CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)                  CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)                  CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)                  CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_WITH_OPENID(fmt, arg...)      CAM_LOGD("[%s] (%d) " fmt, __FUNCTION__, getOpenId(), ##arg)

#define CONNECT_DATA_AND_SETUPDUMP(srcNode, tarNode, dataID)\
{\
    srcNode->connectData(dataID, dataID, tarNode, CONNECTION_DIRECT);\
    property_set(srcNode->getName(), "1");\
    char prop_name[256];\
    snprintf(prop_name, 256, "%s.%s", srcNode->getName(), DepthMapDataHandler::ID2Name(dataID));\
    property_set(prop_name, "1");\
}

#define SETUP_DUMP(srcNode, dataID)\
{\
    property_set(srcNode->getName(), "1");\
    char prop_name[256];\
    snprintf(prop_name, 256, "%s.%s", srcNode->getName(), DepthMapDataHandler::ID2Name(dataID));\
    property_set(prop_name, "1");\
}

#define SETUP_DUMP_REQ_START(start, size)\
{\
    ::property_set("vendor.depthmap.pipe.dump.size", size);\
    ::property_set("vendor.depthmap.pipe.dump.start", start);\
}

IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage);

IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage, MBOOL is_gb=MFALSE);


class WaitingListener
{
public:
    static android::Mutex sMutex;
    static int sCBCount;
    static int sDoneCount;

public:
    static MVOID CB(MVOID* tag, ResultState state, sp<IDualFeatureRequest>& request);
    static bool waitRequest(unsigned int targetTimes, unsigned int timeout_sec);
    static bool waitRequestAtLeast(unsigned int targetTimes, unsigned int timeout_sec, float least_wait_sec);
    static void resetCounter();
};

// finish CB which will be used to notifiy the WaitingListener
MVOID finishCB_forListener(bool status);

class UTEnvironmenSetup
{
public:
    struct Profile
    {
        ENUM_STEREO_RATIO imageRatio;
        ENUM_STEREO_SENSOR_PROFILE sensorProfile;
        MINT32 featureMode;
        MINT32 moduleType;
    };

public:
    UTEnvironmenSetup(char* username, Profile profile);
    ~UTEnvironmenSetup();
public:
    bool isReadyToUT()
    {
        return mIsReadyToUT;
    }
protected:
    bool powerOnSensor();
    bool powerOffSensor(MUINT index);
private:
    IHalSensor *mpHalSensor = nullptr;
    int mSensorCount;
    int mvSensorIndex[2];
    bool mIsReadyToUT;
    char *mUsername;
    Profile mProfile;
};

MBOOL setupUTDepthMapPipe(
        MBOOL isBayerMono,
        sp<DepthMapPipeSetting>& rSetting,
        sp<DepthMapPipeOption>& rOption,
        sp<DepthMapFlowOption>& rFlowOption,
        DepthMapFlowType flowType = eDEPTH_FLOW_TYPE_QUEUED_DEPTH
        );

MBOOL setupReqMetadata(MBOOL eisON, DepthMapRequestPtr& pRequest, MBOOL isQueueFlow=MFALSE);
MVOID releaseMetadata(std::vector<DepthMapRequestPtr> vRequestVec);
MBOOL prepareReqInputBuffer(DepthMapRequestPtr& pRequest);
MBOOL prepareReqOutputBuffer(DepthMapPipeOpState opType, DepthMapRequestPtr& pRequest);
MVOID finalizeTheNodes(std::vector<DepthMapPipeNode*> vNodes);

MSize getRRZOMain1Size();
