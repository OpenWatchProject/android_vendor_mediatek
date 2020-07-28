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
#define LOG_TAG "test_frame3actrl"

    #include <stdio.h>
    #include <stdlib.h>
    #include <mtkcam/utils/std/Log.h>
    #include <mtkcam/drv/IHalSensor.h>
    #include <mtkcam/drv/IHwSyncDrv.h>
    #include <mtkcam/aaa/IHal3A.h>
    //#include <mtkcam/aaa/ISync3A.h>
    #include <mtkcam/feature/DualCam/IFrame3AControl.h>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt , ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg);
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

// for UT test
#define UT_TEST_ITEM(_name) { #_name, ut_ ## _name }
#define UT_REPORT(fmt, arg...)  MY_LOGI(fmt, ##arg)

typedef int (*ut_test_func)(void);

struct ut_test_item
{
    char*           name;
    ut_test_func    func;
};

using namespace NSCam;

namespace
{
static MUINT32 gSensorId_1 = 0;
static MUINT32 gSensorId_2 = 2;

IHalSensor* gpSensorHalObj = NULL;
MUINT32     gSensorDev[2] = {gSensorId_1, gSensorId_2};

// IFrame3ASyncControl
sp<IFrame3AControl> gpFrame3AControl;

}; // namespace

void configSensor(MUINT32 sensorId)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();

    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(sensorId);
    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    // get w, h, frame rate for each scenario (preview, capture, video)
    pHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

}

/******************************************************************************
 *
 ******************************************************************************/
void prepareSensorDual(MUINT32 sensorId_1, MUINT32 sensorId_2)
{
    // get available sensor count
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT32 num = pHalSensorList->searchSensors();
    MY_LOGD("searchSensors (%d)\n", num);

    // power on 2 sensors
    MUINT32    sensorArray[2] = {sensorId_1, sensorId_2};
    gpSensorHalObj = pHalSensorList->createSensor("Frame3ASync", 2, sensorArray);
    if (! gpSensorHalObj)
    {
        MY_LOGE("create dual sensors failed");
        exit(1);
        return;
    }
    if (! gpSensorHalObj->powerOn("Frame3ASycn", 2, &sensorArray[0]))
    {
        MY_LOGE("power on sensors failed");
        return;
    }

    // config sensor
    configSensor(sensorId_1);
    configSensor(sensorId_2);

    //exit(1);
}


/******************************************************************************
 *
 ******************************************************************************/
void closeSensorDual(MUINT32 sensorId_1, MUINT32 sensorId_2)
{
    MUINT32    sensorArray[2] = {sensorId_1, sensorId_2};
    gpSensorHalObj->powerOff("tester", 2, &sensorArray[0]);
}

int ut_none(void)
{
    int status = 0;
    return status;
}

int ut_sensor_sync(void)
{
    int status = -1;
    MY_LOGI("Frame3AControl test +");
    /* open 2 sensors */
    prepareSensorDual(gSensorId_1, gSensorId_2);

    /* enable frame and 3a sync */
    gpFrame3AControl = gpFrame3AControl->createInstance(gSensorId_1, gSensorId_2);
    if (gpFrame3AControl == NULL)
    {
        MY_LOGE("create Frame3AControl instance failed");
        goto lbExit;
    }
    if (! gpFrame3AControl->init())
    {
        MY_LOGE("init failed");
        goto lbExit;
    }

    /* check state and turn off frame and 3a sync */
    if (! gpFrame3AControl->enable3ASync())
    {
        MY_LOGE("enable3ASync failed");
        goto lbExit;
    }
    if (! gpFrame3AControl->enableFrameSync())
    {
        MY_LOGE("enableFrameSync failed");
        goto lbExit;
    }
    // test get and set
    Frame3ASetting_t setting;
    if (! gpFrame3AControl->get(&setting))
    {
        MY_LOGE("Can't get frame 3A settings");
        goto lbExit;
    }

    MY_LOGI("Frame3ASetting value: 3ASync(%d) FrameSync(%d)",
            setting.is3ASyncEnabled, setting.isFrameSyncEnabled);

    setting.is3ASyncEnabled = false;

    if (! gpFrame3AControl->set(&setting, F3A_TYPE_3A_SYNC))
    {
        MY_LOGE("Can't set 3A sync");
        goto lbExit;
    }

    if (! gpFrame3AControl->get(&setting))
    {
        MY_LOGE("Can't get frame 3A settings");
        goto lbExit;
    }

    MY_LOGI("Frame3ASetting value: 3ASync(%d) FrameSync(%d)",
            setting.is3ASyncEnabled, setting.isFrameSyncEnabled);

    if (! gpFrame3AControl->disableFrameSync())
    {
        MY_LOGE("disableFrameSync failed");
        goto lbExit;
    }
    if (! gpFrame3AControl->disable3ASync())
    {
        MY_LOGE("disable3ASync failed");
        goto lbExit;
    }
    if (! gpFrame3AControl->uninit())
    {
        MY_LOGE("uninit failed");
        goto lbExit;
    }
    status = 0;
lbExit:
    closeSensorDual(gSensorId_1, gSensorId_2);
    MY_LOGI("Frame3AControl test -\n");
    return status;
}

/******************************************************************************
 * Main
 ******************************************************************************/
static struct ut_test_item test_items[] =
{
    // sensor open, sync and close
    UT_TEST_ITEM(none),
    UT_TEST_ITEM(sensor_sync),
};

int main(int /*argc*/, char** /*argv*/)
{
    int cnt = sizeof(test_items) / sizeof(test_items[0]);
    int idx;
    struct ut_test_item* test_item;
    int ret;


    for (idx = 0; idx < cnt; idx++)
    {
        test_item = &test_items[idx];

        ret = test_item->func();
        if (0 == ret)
        {
            UT_REPORT("%s: PASSED...\n", test_item->name);
        }
        else
        {
            UT_REPORT("%s: FAILED with %d!!!\n", test_item->name, ret);
            return -1;
        }
    }

    UT_REPORT("all passed...\n");
    return 0;
}
