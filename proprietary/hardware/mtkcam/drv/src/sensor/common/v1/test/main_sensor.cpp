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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////


#define LOG_TAG "SensorInfTest"

#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/def/common.h>

#include "kd_imgsensor_define.h"

#define PRINT_MSG(fmt, arg...)    printf("[%s]" fmt, __FUNCTION__, ##arg)
#define PRINT_WRN(fmt, arg...)    printf("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define PRINT_ERR(fmt, arg...)    printf("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

using namespace NSCam;
using namespace android;
void show_Sensors()
{
    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    if(pHalSensorList) {
        pHalSensorList->searchSensors();
        unsigned int const sensorNum = pHalSensorList->queryNumberOfSensors();

        PRINT_MSG("sensorNum %d \n",sensorNum);
        for( unsigned int i = 0; i < sensorNum; i++) {
            int sensorIndexDual = pHalSensorList->querySensorDevIdx(i);
            int sensorIndex = i;
            PRINT_MSG("name:%s type:%d \n", pHalSensorList->queryDriverName(sensorIndex), pHalSensorList->queryType(sensorIndex));
            PRINT_MSG("index:%d, SensorDevIdx:%d \n", sensorIndex, pHalSensorList->querySensorDevIdx(sensorIndex));
        }

    }

}


void test_SensorInterface(int sendev, int scenario, int input_fps)
{
    unsigned int i;
    char const *const szCallerName = "R1";
    IHalSensor *pHalSensor = NULL;
    IHalSensor::ConfigParam configParam;
    SensorStaticInfo sensorStaticInfo;
    SensorDynamicInfo sensorDynamicInfo;
    int width  = 0;
    int height = 0;
    int fps = 0;

    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();
    unsigned int const sensorNum = pHalSensorList->queryNumberOfSensors();

    for(i = 0; i < sensorNum; i++) {
        int sensorIndexDual = pHalSensorList->querySensorDevIdx(i);
        if(sendev & sensorIndexDual) {
            int sensorIndex = i;

            PRINT_MSG("name:%s type:%d \n", pHalSensorList->queryDriverName(sensorIndex), pHalSensorList->queryType(sensorIndex));
            PRINT_MSG("index:%d, SensorDevIdx:%d \n", sensorIndex, pHalSensorList->querySensorDevIdx(sensorIndex));

            pHalSensorList->querySensorStaticInfo(sensorIndexDual, &sensorStaticInfo);

            switch(scenario) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                width  = sensorStaticInfo.previewWidth;
                height = sensorStaticInfo.previewHeight;
                fps = sensorStaticInfo.previewFrameRate;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                width  = sensorStaticInfo.captureWidth;
                height = sensorStaticInfo.captureHeight;
                fps = sensorStaticInfo.captureFrameRate;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                width  = sensorStaticInfo.videoWidth;
                height = sensorStaticInfo.videoHeight;
                fps = sensorStaticInfo.videoFrameRate;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                width  = sensorStaticInfo.video1Width;
                height = sensorStaticInfo.video1Height;
                fps = sensorStaticInfo.video1FrameRate;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                width  = sensorStaticInfo.video2Width;
                height = sensorStaticInfo.video2Height;
                fps = sensorStaticInfo.video2FrameRate;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                width  = sensorStaticInfo.SensorCustom1Width;
                height = sensorStaticInfo.SensorCustom1Height;
                fps = sensorStaticInfo.custom1FrameRate;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                width  = sensorStaticInfo.SensorCustom2Width;
                height = sensorStaticInfo.SensorCustom2Height;
                fps = sensorStaticInfo.custom2FrameRate;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                width  = sensorStaticInfo.SensorCustom3Width;
                height = sensorStaticInfo.SensorCustom3Height;
                fps = sensorStaticInfo.custom3FrameRate;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                width  = sensorStaticInfo.SensorCustom4Width;
                height = sensorStaticInfo.SensorCustom4Height;
                fps = sensorStaticInfo.custom4FrameRate;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                width  = sensorStaticInfo.SensorCustom5Width;
                height = sensorStaticInfo.SensorCustom5Height;
                fps = sensorStaticInfo.custom5FrameRate;
                break;

            default:
                break;
            }

            PRINT_MSG("Sensor index:%d, scenarioId:%d\n", sensorIndex, scenario);
            PRINT_MSG("crop w:%d, h:%d\n", width, height);

            pHalSensor = pHalSensorList->createSensor(szCallerName, sensorIndex);

            if(!pHalSensor)
            {
              PRINT_ERR("createSensor fail");
              return;
            }

            PRINT_MSG("pHalSensor->powerOn start\n");
            pHalSensor->powerOn(szCallerName, 1, (MUINT *)&sensorIndex);

            configParam =
            {
              .index               = (MUINT)sensorIndex,
              .crop                = MSize(width, height),
              .scenarioId          = (MUINT)scenario,
              .isBypassScenario    = 0,
              .isContinuous        = 1,
            };
            if(input_fps != 0) {
                configParam.framerate = input_fps;
                fps = input_fps *10;
            }

            MUINT32 mipi_rate = 0;
            pHalSensor->configure(1, &configParam);
            pHalSensor->sendCommand(sensorIndexDual, SENSOR_CMD_SET_STREAMING_RESUME, 0, 0, 0);
            pHalSensor->sendCommand(sensorIndexDual, SENSOR_CMD_GET_MIPI_PIXEL_RATE, (MUINTPTR)&scenario,(MUINTPTR)&mipi_rate, (MUINTPTR)&fps);

            PRINT_MSG("main/atv-Continous=%u\n\n", configParam.isContinuous);
            PRINT_MSG("main/atv-BypassSensorScenario=%u\n\n", configParam.isBypassScenario);

            // Only can get current sensor information.
            pHalSensor->querySensorDynamicInfo(pHalSensorList->querySensorDevIdx(sensorIndex), &sensorDynamicInfo);
            PRINT_MSG("TgInfo[%d] = %d\n", sensorIndex, sensorDynamicInfo.TgInfo);
            PRINT_MSG("pixelMode[%d] = %d\n", sensorIndex, sensorDynamicInfo.pixelMode);
            PRINT_MSG("mipi rate = %d\n", mipi_rate);

        }
    }


    while(1) {
        unsigned int meter[4]= {0};
        double d_meter[4]= {0};
        double d_width = width;
        double d_fps = fps;
        if(pHalSensor != NULL)
            pHalSensor->sendCommand(IMGSENSOR_SENSOR_IDX_MAIN, SENSOR_CMD_DEBUG_GET_SENINF_METER, (MUINTPTR)meter, 0, 0);
        PRINT_MSG("---------------------------------------------------\n");
        PRINT_MSG("seninf_mux1 horizontal valid count 0x%x, horzontal blanking count 0x%x\n", meter[0], meter[1]);
        PRINT_MSG("blanking valid count 0x%x, blanking blanking count 0x%x, fps =%d\n\n", meter[2], meter[3], fps/10);

        d_meter[0] = meter[0];
        d_meter[1] = meter[1];
        d_meter[2] = meter[2];
        d_meter[3] = meter[3];

        if(meter[0] && meter[1] && meter[2] && meter[3] && fps) {
            double mipi_pixel_rate =
                d_width/((d_meter[0]/(d_meter[2]+d_meter[3]))*(10/d_fps));
            PRINT_MSG("mipi_pixel_rate = %fMpps \n",mipi_pixel_rate/1000000);
            double vertical_blanking = (d_meter[3]/(d_meter[2]+d_meter[3]))*(10/d_fps);
            double horizontal_blanking = (d_meter[1]/(d_meter[2]+d_meter[3]))*(10/d_fps);
            double line_time =  ((d_meter[1]+d_meter[0])/(d_meter[2]+d_meter[3]))*(10/d_fps);
            PRINT_MSG("vertical_blanking = %fms \n", vertical_blanking*1000);
            PRINT_MSG("horizontal_blanking = %fms \n", horizontal_blanking*1000);
            PRINT_MSG("line time = %fms \n", line_time*1000);
        }
        if(fps != 0)
            usleep((10000000/fps)*10);//update each 10 frames
    }
}

/*******************************************************************************
 *  Main Function
 ********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;
    int sensorDev = 0,scenario =0, fps = 0;
    PRINT_MSG("sizeof long : %d\n", (int)sizeof(long));
    if (argc < 2) {
        show_Sensors();
        PRINT_MSG("Param: 1 <sensorDev> <scenario> <fps>\n");
        PRINT_MSG("<sensorDev> : main(1), Sub(2), Main2(4), sub2(8), main3(16)\n");
        PRINT_MSG("<scenario>  : Pre(%d), Cap(%d), VD(%d), slim1(%d), slim2(%d)\n",\
                SENSOR_SCENARIO_ID_NORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_CAPTURE,\
                SENSOR_SCENARIO_ID_NORMAL_VIDEO, SENSOR_SCENARIO_ID_SLIM_VIDEO1, SENSOR_SCENARIO_ID_SLIM_VIDEO2);
        return -1;
    }
    if(argc > 1)
        sensorDev = atoi(argv[1]);
    if(argc > 2)
        scenario  = atoi(argv[2]);
    if(argc > 3)
        fps = atoi(argv[3]);

    PRINT_MSG("argc num:%d\n",argc);
    test_SensorInterface(sensorDev, scenario, fps);

    return ret;
}
