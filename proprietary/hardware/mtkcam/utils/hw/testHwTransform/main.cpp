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
#define LOG_TAG "HwTransHelperTest"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <unistd.h>
//
#include <mtkcam/utils/hw/HwTransform.h>
//
#include <mtkcam/drv/IHalSensor.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);
//
using namespace NSCam;
using namespace NSCamHW;

/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#endif
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");

#define P2_SUPPORT 0


/******************************************************************************
 *
 ******************************************************************************/
void help()
{
    printf("Pipeline <test>\n");
}

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    //
    IHalSensor* mpSensorHalObj = NULL;
    //
}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
void prepareSensor(int openId)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (pHalSensorList == nullptr)
    {
        return;
    }
    mpSensorHalObj = pHalSensorList->createSensor("tester", openId);
    if( ! mpSensorHalObj ) {
        MY_LOGE("create sensor failed");
        exit(1);
        return;
    }
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(openId)};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);
}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor(int openId)
{
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(openId)};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
}


/******************************************************************************
 *
 ******************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    MY_LOGD("[HwTransHelperTest] start to test\n");
    //
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (pHalSensorList == nullptr)
    {
        return 0;
    }
    MUINT num = pHalSensorList->searchSensors();
    MY_LOGD("searchSensors (%d)\n", num);

    for( MUINT id = 0; id < num; id++ )
    {
        //sensor
        MY_LOGD("[HwTransHelperTest] prepareSensor openId=%d...\n", id);
        prepareSensor(id);
        //
        HwTransHelper helper(id);
        //
        for( int i = SENSOR_SCENARIO_ID_NORMAL_PREVIEW; i <= SENSOR_SCENARIO_ID_SLIM_VIDEO2; i++ )
        {
            MY_LOGD("[HwTransHelperTest] sensor mode=%d +\n", i);
            //
            HwMatrix mat, mat_inv;
            if( ! helper.getMatrixFromActive(i, mat) ) {
                MY_LOGE("failed at mode %d\n", i);
            }
            //
            if( ! helper.getMatrixToActive(i, mat_inv) ) {
                MY_LOGE("failed at mode %d\n", i);
            }
            //
            mat.dump("HwTransHelperTestResult");
            mat_inv.dump("HwTransHelperTestResult");
            //
            MY_LOGD("[HwTransHelperTest] sensor mode=%d -\n", i);
        }
        //
        MY_LOGD("[HwTransHelperTest] closeSensor openId=%d...\n", id);
        closeSensor(id);
    }

    MY_LOGD("[HwTransHelperTest] end of test\n");
    return 0;
}

