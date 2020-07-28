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

#define LOG_TAG "nvram_test"

#include <gtest/gtest.h>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <nvbuf_util.h>
#include <nvram_drv.h>
#include <camera_custom_nvram.h>
#include <mtkcam/drv/IHalSensor.h>

#include <log/log.h>

// ---------------------------------------------------------------------------

namespace NSCam {

// ---------------------------------------------------------------------------

class NvramTest : public ::testing::Test
{
protected:
    // function
    NvramTest() {}

    virtual ~NvramTest() {}

    virtual void SetUp()
    {
        ALOGD("SetUp +");
    }

    virtual void TearDown()
    {
        ALOGD("TearDown +");
    }

protected:
    // member
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

// ---------------------------------------------------------------------------

using namespace NSCam;

TEST_F(NvramTest, Init)
{
    ALOGD("Init +");
    int err = 0;
    IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();
    pIHalSensorList->searchSensors();
    int sensorCount = pIHalSensorList->queryNumberOfSensors();
    ALOGD("sensorCount:%d", sensorCount);

    ALOGD("err=%d", err);
    NVRAM_CAMERA_3A_STRUCT* buf;
    ALOGD("DUAL_CAMERA_MAIN_SENSOR=%d", DUAL_CAMERA_MAIN_SENSOR);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, DUAL_CAMERA_MAIN_SENSOR, (void*&)buf);
    ALOGD("qq ln=%d 0x%x err=%d ver=%d sensorId=%d, MinGain=%d",__LINE__, (int*)buf, err,
    buf->u4Version, buf->SensorId, (buf->rAENVRAM[0]).rDevicesInfo.u4MinGain);
    // buf->temp=10;
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, DUAL_CAMERA_MAIN_SENSOR);
    // logI("qq ln=%d 0x%x err=%d",__LINE__, (int)buf, err);

    // ALOGD("%d", NVRAM_NO_ERROR);
}