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
#include <imem_drv.h>

#include "kd_imgsensor_define.h"
#include "seninf_drv_imp.h"

#include "FakeSensor.h"


#define LOG_MSG(fmt, arg...)    printf("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    printf("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    printf("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

using namespace NSCam;
using namespace android;

IHalSensor::ConfigParam configParam[2];
SensorStaticInfo sensorStaticInfo[3];
SensorDynamicInfo sensorDynamicInfo[3];

void test_SensorInterface(int sendev, int scenario)
{
    int err = 0;

    MINT32 index = 0, subIndex = 0, sensorFound = 0;
    MUINT32 sensorDevId = SENSOR_DEV_MAIN;
    MUINT   scenarioId = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    MUINT32  mSupportedSensorDev = 0;
    static int  mSensorDev;
    //sensor Info
    MUINT32 msensorArray[1];
    //IHalSensor *m_pSensorHalObj;
    //char const *mSensorCallerName;
    bool   mSensorInit;
    int i,j;
    MUINT32 sensorArray[1] = {0}, sensorArray2[1] = {1};
    char const*const szCallerName = "R1";
    IHalSensor* pHalSensor = NULL;
    //IHalSensor* pHalSensor2 = NULL;


    //====== Sensor Setting =====
    //search sensor & get object
    IHalSensorList*const pHalSensorList = IHalSensorList::get();//TS_FakeSensorList::getTestModel();//IHalSensorList::get();
    pHalSensorList->searchSensors();
    unsigned int const sensorNum = pHalSensorList->queryNumberOfSensors();

    //Get sensor information
    for (MUINT i = 0; i < sensorNum; i++){
        LOG_MSG("name:%s type:%d \n", pHalSensorList->queryDriverName(i), pHalSensorList->queryType(i));//debug
        LOG_MSG("index:%d, SensorDevIdx:%d \n",i ,pHalSensorList->querySensorDevIdx(i));//debug
        mSupportedSensorDev |= pHalSensorList->querySensorDevIdx(i);
    }

    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
    LOG_MSG("Main preview width = %d, height = %d\n",sensorStaticInfo[0].previewWidth,sensorStaticInfo[0].previewHeight);
    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_SUB,&sensorStaticInfo[1]);
    LOG_MSG("Sub preview width = %d, height = %d\n",sensorStaticInfo[1].previewWidth,sensorStaticInfo[1].previewHeight);
    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN_2,&sensorStaticInfo[2]);
    LOG_MSG("Main2 preview width = %d, height = %d\n",sensorStaticInfo[2].previewWidth,sensorStaticInfo[2].previewHeight);

    LOG_MSG("mSupportedSensorDev:0x%x \n", mSupportedSensorDev);

    if(sensorNum > 0)
    {
        sensorFound = 1;
    }
    
    if(sendev == 1){    
        sensorArray[0] = 0;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
    }
    else if(sendev == 2){
        sensorArray[0] = 1;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_SUB,&sensorStaticInfo[0]);
    }
    else if(sendev == 4){
        sensorArray[0] = 2;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN_2,&sensorStaticInfo[0]);
    }
    else if(sendev == 3){
        sensorArray[0] = 0;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
        sensorArray2[0] = 1;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_SUB,&sensorStaticInfo[1]);
    }
    else if(sendev == 5){
        sensorArray[0] = 0;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
        sensorArray2[0] = 2;
        pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN_2,&sensorStaticInfo[1]);
    }

    scenarioId = scenario;
    if(scenarioId == 1)
    {
        sensorStaticInfo[0].previewWidth = sensorStaticInfo[0].captureWidth;
        sensorStaticInfo[0].previewHeight = sensorStaticInfo[0].captureHeight;
    }
    else if(scenarioId == 2)
    {
        sensorStaticInfo[0].previewWidth = sensorStaticInfo[0].videoWidth;
        sensorStaticInfo[0].previewHeight = sensorStaticInfo[0].videoHeight;
    }
    else
    {
        sensorStaticInfo[0].previewWidth = sensorStaticInfo[0].previewWidth;
        sensorStaticInfo[0].previewHeight = sensorStaticInfo[0].previewWidth;
    }
    LOG_MSG("Sensor index:%d, scenarioId:%d\n",sensorArray[0],scenarioId);
    LOG_MSG("crop w:%d, h:%d\n",sensorStaticInfo[0].previewWidth,sensorStaticInfo[0].previewHeight);

    pHalSensor = pHalSensorList->createSensor(szCallerName, 1, &sensorArray[0]);//modify
    if(! pHalSensor )
    {
      LOG_ERR("createSensor fail");
      return;
    }
    else
    {
		LOG_MSG("pHalSensor->powerOn start\n");
		if(sendev == 3)
		{
			pHalSensor->powerOn(szCallerName, 1, &sensorArray[0]);
			pHalSensor->powerOn(szCallerName, 1, &sensorArray2[0]);
		}
		else if(sendev == 5)
		{
			pHalSensor->powerOn(szCallerName, 1, &sensorArray[0]);
			pHalSensor->powerOn(szCallerName, 1, &sensorArray2[0]);
		}
		else
		{
			pHalSensor->powerOn(szCallerName, 1, &sensorArray[0]);
		}
	}
    configParam[0] =
    {
      index               : sensorArray[0],
      crop                : MSize(sensorStaticInfo[0].previewWidth, sensorStaticInfo[0].previewHeight),
      scenarioId          : scenarioId,
      isBypassScenario    : 0,
      isContinuous        : 1,
    };
    configParam[1] =
    {
      index               : sensorArray2[0],
      crop                : MSize(sensorStaticInfo[1].previewWidth, sensorStaticInfo[1].previewHeight),
      scenarioId          : scenarioId,
      isBypassScenario    : 0,
      isContinuous        : 1,
    };
    //LOG_ERR("pHalSensor->powerOn end 2 (%p)\n\n",m_pSensorHalObj);
    if((sendev == 3) || (sendev == 5))
    {
        pHalSensor->configure(1, &configParam[0]);
        pHalSensor->configure(1, &configParam[1]);
    }
    else
    {
        pHalSensor->configure(1, &configParam[0]);
    }

     LOG_MSG("main/atv-Continous=%u\n\n",configParam[0].isContinuous);
     LOG_MSG("main/atv-BypassSensorScenario=%u\n\n",configParam[0].isBypassScenario);


     // Only can get current sensor information.
     pHalSensor->querySensorDynamicInfo(mSensorDev, &sensorDynamicInfo[0]);
     LOG_MSG("TgInfo[0] = %d, TgInfo[1] = %d, TgInfo[2] = %d\n", sensorDynamicInfo[0].TgInfo, sensorDynamicInfo[1].TgInfo, sensorDynamicInfo[2].TgInfo);
     LOG_MSG("pixelMode[0] = %d, pixelMode[1] = %d, pixelMode[2] = %d\n", sensorDynamicInfo[0].pixelMode, sensorDynamicInfo[1].pixelMode, sensorDynamicInfo[2].pixelMode);





    for(int ii=0;ii<500;ii++) {
      LOG_MSG("RRRRR");
      usleep(5000000);
    }

}

static void usage()
{
    printf("Usage: iopipetest <0: not spported, 1: camio, 3:pass2, 4:IT>\n");
}
/*******************************************************************************
 *  Main Function
 ********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;
    int sensorDev,scenario;
    LOG_MSG("sizeof long : %d\n", (int)sizeof(long));
    if (argc < 2) {
        LOG_MSG("Param: 1 <sensorDev> <scenario>\n");
        LOG_MSG("<sensorDev> : main(1), Sub(2), Main2(4)\n");
        LOG_MSG("<scenario>  : Pre(%d), Cap(%d), VD(%d), slim1(%d), slim2(%d)\n",\
                SENSOR_SCENARIO_ID_NORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_CAPTURE,\
                SENSOR_SCENARIO_ID_NORMAL_VIDEO, SENSOR_SCENARIO_ID_SLIM_VIDEO1, SENSOR_SCENARIO_ID_SLIM_VIDEO2);
        return -1;
    }
    sensorDev = atoi(argv[1]);
    scenario  = atoi(argv[2]);

    LOG_MSG("argc num:%d\n",argc);
    test_SensorInterface(sensorDev, scenario);

    return ret;
}
