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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "MtkCam/DualCamZoomPolicy/AAAJudgeStrategy"

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_dualzoom.h>
// mtkcam global header file

// Module header file

// Local header file
#include "AAAJudgeStrategy.h"
#include "CameraContext.h"

/******************************************************************************
*
*******************************************************************************/
#define COMMON_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGA(fmt, arg...)    CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGF(fmt, arg...)    CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define COMMON_FUN_IN()             COMMON_LOGD("+")
#define COMMON_FUN_OUT()            COMMON_LOGD("-")

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define COMMON_SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
using AAASettings   = AAAJudgeSettings;
using Helper        = DualZoomPolicyHelper;
/******************************************************************************
 * AAAJudgeStrategy
******************************************************************************/
AAAJudgeStrategy::
AAAJudgeStrategy(OpenId openId, const std::string& name)
: mOpenId(openId)
, mName(name)
, mAELv(-1)
, mAFDac(-1)
, mIsAFDone(false)
{

}
//
AAAJudgeStrategy::
~AAAJudgeStrategy()
{

}
//
OpenId
AAAJudgeStrategy::
calculate(sp<IFrame3AControl>& frame3AControl)
{
    COMMON_SCOPED_TRACER();

    OpenId ret = -1;
    if(!ForceFlow::isSkip3ACheck())
    {
        if (frame3AControl != nullptr)
        {
            const OpenId openId = getOpenId();
            Frame3ASetting_t settings
            {
                .openId = openId
            };
            frame3AControl->get(&settings, F3A_TYPE_3A_INFO);
            {
                mAFDac = settings.AFDAC;
                mAELv = settings.AELv_x10;
                mIsAFDone = settings.isAFDone;
            }
            ret = onCalculate(settings);
        }
        else
        {
            COMMON_LOGD("failed to calculate 3A judge, name: %s, the argmuent is nullptr", getName().c_str());
        }
    }
    return ret;
}
//
OpenId
AAAJudgeStrategy::
getOpenId() const
{
    return mOpenId;
}
//
const std::string&
AAAJudgeStrategy::
getName() const
{
    return mName;
}
//
MINT32
AAAJudgeStrategy::
getAELv() const
{
    return mAELv;
}
//
MINT32
AAAJudgeStrategy::
getAFDac() const
{
    return mAFDac;
}
//
MINT32
AAAJudgeStrategy::
getIsAFDone() const
{
    return mIsAFDone;
}
//
OpenId
AAAJudgeStrategy::
onCalculate(const Frame3ASetting_t& settings)
{
    return -1;
}
//
MVOID
AAAJudgeStrategy::
setIsAFDone(MBOOL value)
{
    mIsAFDone = value;
}
/******************************************************************************
 * DefauleAAAJudgeStrategy
******************************************************************************/
sp<AAAJudgeStrategy>
DefauleAAAJudgeStrategy::
createInstance(OpenId openId)
{
    return new DefauleAAAJudgeStrategy(openId);
}
//
DefauleAAAJudgeStrategy::
DefauleAAAJudgeStrategy(OpenId openId)
: AAAJudgeStrategy(openId, "default")
{

}
//
/******************************************************************************
 * WideAAAJudgeStrategy
******************************************************************************/
sp<AAAJudgeStrategy>
WideAAAJudgeStrategy::
createInstance()
{
    return new WideAAAJudgeStrategy();
}
//
WideAAAJudgeStrategy::
WideAAAJudgeStrategy()
: AAAJudgeStrategy(DUALZOOM_WIDE_CAM_ID, "wide")
, mAEIsoCount(0)
, mAFDacCount(0)
, mPreviousAFDac(0)
{

}
//
OpenId
WideAAAJudgeStrategy::
onCalculate(const Frame3ASetting_t& settings)
{
    COMMON_SCOPED_TRACER();

    OpenId ret = -1;

    MBOOL isSatisfiedAEIso = false;
    MBOOL isSatisfiedAFDac = false;
    MBOOL isSatisfiedAELvDiff = false;

    const MTK_SYNC_CAEMRA_STATE wideStatus = CameraContext::getStatus(DUALZOOM_WIDE_CAM_ID);
    if(wideStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY)
    {
        COMMON_LOGD("[%s] wied in standby status, doesn't do the 3A judge", getName().c_str());
    }
    else
    {
        MBOOL isAFDone = settings.isAFDone;
        if(settings.AFDAC != mPreviousAFDac)
        {
            isAFDone= false;
            setIsAFDone(isAFDone);
            COMMON_LOGD("[%s] af done is different, previous: %d, current: %d", getName().c_str(), mPreviousAFDac, settings.AFDAC);
        }

        if (settings.isoValue > AAASettings::getWideAEIsoHighThreshold())
        {
            mAEIsoCount++;
        }
        else if (settings.isoValue < AAASettings::getWideAEIsoLowThreshold())
        {
            mAEIsoCount = 0;
        }

        if (isAFDone)
        {
            // [Daniel] check this item is correct ot not
            if (settings.AFDAC > AAASettings::getWideAFDacHighThreshold())
            {
                mAFDacCount++;
            }
            else if (settings.AFDAC < AAASettings::getWideAFDacLowThreshold())
            {
                mAFDacCount = 0;
            }
        }

        MINT32 currentTeleAELv = -1;
        sp<CameraContext> temp = CameraContext::getCameraContext(DUALZOOM_TELE_CAM_ID).promote();
        if(temp != nullptr)
        {
            currentTeleAELv = temp->getAELv();
        }
        else
        {
            COMMON_LOGD("[%s] falied to get tele AElv, tele camera context is nullptr", getName().c_str());
        }


        isSatisfiedAEIso = (mAEIsoCount > AAASettings::getWideAEIsoCountThreshold());
        isSatisfiedAFDac = (mAFDacCount > AAASettings::getWideAFDacCountThreshold());
        isSatisfiedAELvDiff = ((currentTeleAELv != -1) && ((settings.AELv_x10 - currentTeleAELv) > AAASettings::getAELVDifferenceThreshold()));

        ret = (isSatisfiedAFDac || isSatisfiedAEIso || isSatisfiedAELvDiff) ? DUALZOOM_WIDE_CAM_ID : -1;

        COMMON_LOGD("[%s] prevDac/ curDac: %d, %d, afDone: %d",
            getName().c_str(), mPreviousAFDac, settings.AFDAC, isAFDone);

        COMMON_LOGD("[%s] afDac : %d/ %d/ %d/, afDacCount: %d (%d)",
            getName().c_str(), AAASettings::getWideAEIsoLowThreshold(), settings.AFDAC, AAASettings::getWideAEIsoHighThreshold(),
            mAFDacCount, AAASettings::getWideAFDacCountThreshold());

        COMMON_LOGD("[%s] iso : %d/ %d/ %d, aeIsoCount: %d (%d)",
            getName().c_str(), AAASettings::getWideAEIsoLowThreshold(), settings.isoValue, AAASettings::getWideAEIsoHighThreshold(),
            mAEIsoCount, AAASettings::getWideAEIsoCountThreshold());

        COMMON_LOGD("[%s] aeLV wide/ tele: %d/ %d, diffTh: %d",
            getName().c_str(), settings.AELv_x10, currentTeleAELv, AAASettings::getAELVDifferenceThreshold());

        mPreviousAFDac = settings.AFDAC;
    }

    if(ret != -1)
    {
        COMMON_LOGD("[%s] calculate reault: %d, isSatisfiedAELvDiff: %d, isSatisfiedAEIso: %d, isSatisfiedAELvDiff: %d",
            getName().c_str(), ret, isSatisfiedAFDac, isSatisfiedAEIso, isSatisfiedAELvDiff);
    }
    return ret;
}
//
/******************************************************************************
 * TeleAAAJudgeStrategy
******************************************************************************/
sp<AAAJudgeStrategy>
TeleAAAJudgeStrategy::
createInstance()
{
    return new TeleAAAJudgeStrategy();
}
//
TeleAAAJudgeStrategy::
TeleAAAJudgeStrategy()
: AAAJudgeStrategy(DUALZOOM_TELE_CAM_ID, "tele")
, mIsPreviousAFDone(false)
, mAEIsoCount(0)
{

}
//
OpenId
TeleAAAJudgeStrategy::
onCalculate(const Frame3ASetting_t& settings)
{
    COMMON_SCOPED_TRACER();

    OpenId ret = -1;

    MBOOL isSatisfiedAEIso = false;
    MBOOL isSatisfiedAFStrategy = false;

    const MTK_SYNC_CAEMRA_STATE teleStatus = CameraContext::getStatus(DUALZOOM_TELE_CAM_ID);
    if(teleStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        COMMON_LOGD("[%s] tele is in standby status, doesn't do the 3A judge", getName().c_str());
        mIsPreviousAFDone = false;
        mAEIsoCount = 0;
    }
    else if(Helper::getWideStandbySupported())
    {
        const MTK_SYNC_CAEMRA_STATE wideStatus = CameraContext::getStatus(DUALZOOM_WIDE_CAM_ID);

        COMMON_LOGD("wide support standby, wide status: %s(%u)", DualZoomPolicyConverter::statusNameMap[wideStatus], static_cast<MUINT32>(wideStatus));

        // because wide is in standby mode and tele check the 3A judge condition
        if ((wideStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY) || (wideStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY))
        {
            if (settings.isoValue > AAASettings::getTeleAEIsoHighThreshold())
            {
                mAEIsoCount++;
            }
            else if (settings.isoValue < AAASettings::getTeleAEIsoLowThreshold())
            {
                mAEIsoCount = 0;
            }

            if (mAEIsoCount > AAASettings::getTeleAEIsoCountThreshold())
            {
                isSatisfiedAEIso = true;
            }

            COMMON_LOGD("[%s] iso: %d/ %d/ %d, aeIsoCount: %d, isSatisfiedAEIso: %d",
                getName().c_str(), AAASettings::getTeleAEIsoLowThreshold(), settings.isoValue,
                AAASettings::getTeleAEIsoHighThreshold(), AAASettings::getTeleAEIsoCountThreshold(), isSatisfiedAEIso);
        }
        else
        {
            mAEIsoCount = 0;
        }

        // AF strategy judge
        {
            if (AAASettings::getAFStrategy() == 0)
            {
                if (mIsPreviousAFDone == true && settings.isAFDone == false)
                {
                    isSatisfiedAFStrategy = true;
                }
            }
            else if (AAASettings::getAFStrategy() == 1)
            {
                if ((mIsPreviousAFDone == false) && (settings.isAFDone == true) && (settings.AFDAC > AAASettings::getTeleAFDacThreshold()))
                {
                    isSatisfiedAFStrategy = true;
                }
            }

            COMMON_LOGD("[%s] afStrategy: %d, afDac: %d (%d), afTeleState (prev/cur): %d/ %d, aeLV: %d, isSatisfiedAFStrategy: %d",
                getName().c_str(), AAASettings::getAFStrategy(), settings.AFDAC, AAASettings::getTeleAFDacThreshold(),
                mIsPreviousAFDone, settings.isAFDone, settings.AELv_x10, isSatisfiedAFStrategy);
        }

        ret = (isSatisfiedAEIso || isSatisfiedAFStrategy) ? DUALZOOM_WIDE_CAM_ID : -1;

        mIsPreviousAFDone = settings.isAFDone;
    }

    if(ret != -1)
    {
        COMMON_LOGD("[%s] return: %d, isSatisfiedAEIso: %d, isSatisfiedAFStrategy: %d",
            getName().c_str(), ret, isSatisfiedAEIso, isSatisfiedAFStrategy);
    }
    return ret;
}

} // NSCam
