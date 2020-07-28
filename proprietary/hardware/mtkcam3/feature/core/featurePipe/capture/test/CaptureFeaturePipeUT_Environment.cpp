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

// Standard C header file
#include <string>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/drv/IHalSensor.h>
// Local header file
#include "CaptureFeaturePipeUT_Environment.h"
// Log
#define LOG_TAG "CapturePipeUT/Environment"
#include "CaptureFeaturePipeUT_log.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {
namespace UnitTest {


/*******************************************************************************
* Class Define
*******************************************************************************/
class MySensor
{
public:
    MySensor();
    ~MySensor();

public:
    MUINT32 getMain1Index() { return mSensorIndex[0]; }
    MUINT32 getMain2Index() { return mSensorIndex[1]; }

    MBOOL init();

private:
    const std::string   mName;
    MUINT32             mSensorIndex[2];
    IHalSensor*         mHalSensor;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MySensor Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MySensor::
MySensor()
: mName {"MySansor"}
, mSensorIndex {0, 1}
, mHalSensor {nullptr}
{

}

MySensor::
~MySensor()
{
    SCOPED_TRACER();

    if(mHalSensor)
    {
        const MUINT32 size = sizeof(mSensorIndex)/sizeof(mSensorIndex[0]);
        mHalSensor->powerOff(mName.c_str(), size, mSensorIndex);
        mHalSensor->destroyInstance(mName.c_str());
        mHalSensor = nullptr;
        UT_LOGD("powerOffSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
    }
}

MBOOL
MySensor::
init()
{
    SCOPED_TRACER();

    const MUINT32 size = sizeof(mSensorIndex)/sizeof(mSensorIndex[0]);

    IHalSensor* pHalSensor = nullptr;
    NSCam::IHalSensorList* sensorList = MAKE_HalSensorList();

    if(!sensorList)
    {
        UT_LOGD("failed to get sensor list, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
        return MFALSE;
    }

    const MUINT32 count = sensorList->searchSensors();
    mHalSensor = sensorList->createSensor(mName.c_str(), size, mSensorIndex);

    if(!mHalSensor)
    {
        UT_LOGD("failed to createSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
        return MFALSE;
    }
    UT_LOGD("createSensor, count=%u index0=%u, index1=%u", count, mSensorIndex[0], mSensorIndex[1]);

    UT_LOGD("powerOnSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
    mHalSensor->powerOn(mName.c_str(), size, mSensorIndex);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MYEnvironment Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MYEnvironment::gMain1Index = -1;
MUINT32 MYEnvironment::gMain2Index = -1;

MYEnvironment::
MYEnvironment()
: mMySensor{new MySensor()}
{
    SCOPED_TRACER();
}

void
MYEnvironment::
SetUp()
{
    SCOPED_TRACER();

    mMySensor->init();
    gMain1Index = mMySensor->getMain1Index();
    gMain2Index = mMySensor->getMain2Index();
}

void
MYEnvironment::
TearDown()
{
    SCOPED_TRACER();

    // I don't know what, we need in this function to destroy the MYSensor object
    mMySensor = nullptr;
}

MYEnvironment::
~MYEnvironment()
{
    SCOPED_TRACER();
}

} // UnitTest
} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
