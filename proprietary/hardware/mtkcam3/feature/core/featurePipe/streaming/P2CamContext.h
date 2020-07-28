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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2_CAM_CONTEXT_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2_CAM_CONTEXT_H_

#include <utility>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <mtkcam/def/common.h>
#include <common/3dnr/3dnr_hal_base.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/IHalISP.h>
#include "StreamingFeatureData.h"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


class P2CamContext : public virtual android::RefBase
{
public:
    static constexpr int SENSOR_INDEX_MAX = 10;

private:
    static const char* MODULE_NAME;

    static android::Mutex sMutex;
    static android::sp<P2CamContext> spInstance[SENSOR_INDEX_MAX];

    MINT32 mRefCount;
    MUINT32 mSensorIndex;
    MBOOL mIsInited;

    NSCam::NSIoPipe::NSPostProc::Hal3dnrBase* mp3dnr = NULL;
    ImgBuffer mPrevFullImg = NULL;
    ImgBuffer mPrevFullImg2 = NULL;
    NS3Av3::IHal3A *mp3A = NULL;
    NS3Av3::IHalISP *mpISP = NULL;

    P2CamContext(MUINT32 sensorIndex);
    virtual ~P2CamContext();

    void init(const StreamingFeaturePipeUsage &pipeUsage);
    void uninit();

public:
    static android::sp<P2CamContext> createInstance(
        MUINT32 sensorIndex,
        const StreamingFeaturePipeUsage &pipeUsage);

    static void destroyInstance(MUINT32 sensorIndex);
    static android::sp<P2CamContext> getInstance(MUINT32 sensorIndex);

    MUINT32 getSensorIndex()
    {
        return mSensorIndex;
    }

    NSCam::NSIoPipe::NSPostProc::Hal3dnrBase* get3dnr()
    {
        return mp3dnr;
    }

    ImgBuffer getPrevFullImg()
    {
        return mPrevFullImg;
    }

    ImgBuffer getPrevFullImg2()
    {
        return mPrevFullImg2;
    }

    NS3Av3::IHal3A* get3A()
    {
        return mp3A;
    }
    NS3Av3::IHalISP* getISP()
    {
        return mpISP;
    }

    void setPrevFullImg(ImgBuffer buffer)
    {
        mPrevFullImg = buffer;
    }

    void setPrevFullImg2(ImgBuffer buffer)
    {
        mPrevFullImg2 = buffer;
    }

    template <typename _Func, typename ... _Arg>
    static void forAllInstances(_Func &&func, _Arg&& ... arg);

    // No copy
    P2CamContext(const P2CamContext &) = delete;
    P2CamContext& operator=(const P2CamContext &) = delete;
};


// NOTE: Do not invoke destoryInstance() or other function
// which may acquire the mutex in the callback function
template <typename _Func, typename ... _Arg>
void P2CamContext::forAllInstances(_Func &&func, _Arg&& ... arg)
{
    android::Mutex::Autolock lock(sMutex);

    for (MUINT32 i = 0; i < SENSOR_INDEX_MAX; i++)
    {
        android::sp<P2CamContext> instance = spInstance[i];
        if (instance != NULL)
        {
            func(instance, std::forward<_Arg>(arg)...);
        }
    }
}


android::sp<P2CamContext> getP2CamContext(MUINT32 sensorIndex);


};
};
};

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2_CAM_CONTEXT_H_

