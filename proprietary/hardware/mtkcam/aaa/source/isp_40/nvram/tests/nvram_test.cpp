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

#include <log/log.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <nvbuf_util.h>
#include <nvram_drv.h>
#include <camera_custom_nvram.h>
#include <mtkcam/drv/IHalSensor.h>

#include "myArgs.h"
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
        ALOGD("SetUp -");
    }

    virtual void TearDown()
    {
        ALOGD("TearDown +");
        ALOGD("TearDown -");
    }

protected:
    // member
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

// ---------------------------------------------------------------------------

using namespace NSCam;
using namespace android;
using namespace std;

enum
{
    e_NvramTypeNum = 13,
    e_SensorTypeNum = 4,
};

static int g_nvramTypeEnum[e_NvramTypeNum]=
{
    (int)CAMERA_NVRAM_DATA_ISP,
    (int)CAMERA_NVRAM_DATA_3A,
    (int)CAMERA_NVRAM_DATA_SHADING,
    (int)CAMERA_NVRAM_DATA_LENS,
    (int)CAMERA_DATA_AE_PLINETABLE,
    (int)CAMERA_NVRAM_DATA_STROBE,
    (int)CAMERA_DATA_TSF_TABLE,
    (int)CAMERA_DATA_PDC_TABLE,
    (int)CAMERA_NVRAM_DATA_GEOMETRY,
    (int)CAMERA_NVRAM_DATA_FOV,
    (int)CAMERA_NVRAM_DATA_FEATURE,
    (int)CAMERA_NVRAM_DATA_AF,
    (int)CAMERA_NVRAM_DATA_FLASH_CALIBRATION
};

static int g_nvramSensorEnum[e_SensorTypeNum]=
{
    (int)DUAL_CAMERA_MAIN_SENSOR,
    (int)DUAL_CAMERA_SUB_SENSOR,
    (int)DUAL_CAMERA_MAIN_2_SENSOR,
    (int)DUAL_CAMERA_SUB_2_SENSOR,
};

TEST_F(NvramTest, Init)
{

    ALOGD("Parse arguments +");
    String8 dev, type, u4data;
    MUINT32 u4TestArg = 0;
    for (int i = 1; i < myArgc; i+=2) {
        if (i + 1 < myArgc){ // Check that we haven't finished parsing already
            if (!strcmp(*(myArgv+i), "--dev")){
                dev = String8(*(myArgv+i+1));
            } else if (!strcmp(*(myArgv+i), "--type")) {
                type = String8(*(myArgv+i+1));
            } else if (!strcmp(*(myArgv+i), "--u4data")) {
                u4data = String8(*(myArgv+i+1));
            } else {
                ALOGD("Not enough or invalid arguments, please try again.\n");
            }
        }
    }
    if (dev.isEmpty() || type.isEmpty())
    {
        ALOGD("Parse arguments error");
        return;
    }

    if (!u4data.isEmpty())
        ALOGD("Parse arguments -, dev(%s), type(%s), u4data(%s)", dev.string(), type.string(), u4data.string());
    else
        ALOGD("Parse arguments -, dev(%s), type(%s)", dev.string(), type.string());

    ALOGD("Init sensor +");
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    pIHalSensorList->searchSensors();
    int sensorCount = pIHalSensorList->queryNumberOfSensors();
    ALOGD("Init sensor -, sensorCount(%d)", sensorCount);

    void* buf = NULL;
    int err = 0;
    int sensorDev = g_nvramSensorEnum[atoi(dev.string())];
    CAMERA_DATA_TYPE_ENUM nvRamId = (CAMERA_DATA_TYPE_ENUM)g_nvramTypeEnum[atoi(type.string())];

    ALOGD("read buf +, nvRamId(%d), sensorDev(%d)", nvRamId, sensorDev);
    err = NvBufUtil::getInstance().getBufAndRead(nvRamId, sensorDev, (void*&)buf);
    ALOGD("read buf -, buf(0x%p), err(%d)", buf, err);

    if (!u4data.isEmpty())
    {
        MUINT32 u4dataInt = atoi(u4data.string());
        ::memcpy((MUINT32*)buf, &u4dataInt, 4);
    }

    ALOGD("write buf +");
    err = NvBufUtil::getInstance().write(nvRamId, sensorDev);
    ALOGD("write buf -, err(%d)", err);

    MUINT32 bufData = 9999;
    ::memcpy(&bufData, (MUINT32*)buf, 4);

    ALOGD("First 4 byte data(%d), data_16(0x%08x)", bufData, bufData);
}