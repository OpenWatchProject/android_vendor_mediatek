/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG "CamCalCamCal"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h> //#include <utils/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <math.h>
#include <bits/ioctl.h>
#include "unistd.h"
//mutex
#include <pthread.h>

#include <mtkcam/drv/mem/cam_cal_drv.h>


#include "camera_custom_nvram.h"
#include "string.h"

//cam_cal
#include "cam_cal.h"
#include "cam_cal_define.h"
extern "C"{
//#include "cam_cal_layout.h"
//#include "camera_custom_cam_cal.h"
}
#include "camera_calibration_cam_cal.h"

#include <stdio.h> //for rand?
#include <stdlib.h>
//COMMON


#define DEBUG_CALIBRATION_LOAD

#define CUSTOM_CAM_CAL_ROTATION_00 0
#define CUSTOM_CAM_CAL_ROTATION_01 CUSTOM_CAM_CAL_ROTATION_0_DEGREE
#define CUSTOM_CAM_CAL_COLOR_ORDER_00 CUSTOM_CAM_CAL_COLOR_SHIFT_00
#define CUSTOM_CAM_CAL_COLOR_ORDER_01 CUSTOM_CAM_CAL_COLOR_SHIFT_00

#define CAM_CAL_ERR_NO_DEVICE   0x8FFFFFFF

//#define CUSTOM_CAM_CAL_NEW_MODULE_NUMBER_CHECK 1 //

#define CAM_CAL_SHOW_LOG 1
#define CAM_CAL_VER "ver0329~"   //2018.03.29 For MT6761

#ifdef CAM_CAL_SHOW_LOG
//#define CAM_CAL_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#define CAM_CAL_LOG(fmt, arg...)    ALOGI(CAM_CAL_VER " " fmt, ##arg)
#define CAM_CAL_ERR(fmt, arg...)    ALOGE(CAM_CAL_VER "Err: %5d: " fmt, __LINE__, ##arg)
#else
#define CAM_CAL_LOG(fmt, arg...)    void(0)
#define CAM_CAL_ERR(fmt, arg...)    void(0)
#endif
#define CAM_CAL_LOG_IF(cond, ...)      do { if ( (cond) ) { CAM_CAL_LOG(__VA_ARGS__); } }while(0)


UINT32 DoCamCalModuleVersion(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPartNumber(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
//UINT32 DoCamCalShadingTable(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalStereoData(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal_Dump(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

UINT32 S5K3L6_DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 S5K3L6_DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 S5K3L6_DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);


static pthread_mutex_t mEEPROM_Mutex = PTHREAD_MUTEX_INITIALIZER;


#if 1
//typedef enum
enum
{
    CALIBRATION_LAYOUT_SENSOR_OTP = 0,
    CALIBRATION_LAYOUT_STEREO_MAIN1,
    CALIBRATION_LAYOUT_STEREO_MAIN2,
    CALIBRATION_LAYOUT_STEREO_MAIN1_LEGACY,
    CALIBRATION_LAYOUT_STEREO_MAIN2_LEGACY,
    CALIBRATION_LAYOUT_STEREO_MAIN_MTK_1_4,
    CALIBRATION_LAYOUT_STEREO_SUB_MTK_1_4,
    MAX_CALIBRATION_LAYOUT_NUM
};
//}CAM_CAL_MODULE_TYPE;

/*
typedef enum
{
    CAM_CAL_LAYOUT_PASS,
    CAM_CAL_LAYOUT_FAILED,
    CAM_CAL_LAYOUT_QUEUE
}CAM_CAL_LAYOUT_T;
*/
#else
#define 	CALIBRATION_LAYOUT_SLIM_LSC1  0 //Legnacy module for 657x
#define 	CALIBRATION_LAYOUT_N3D_DATA1 1 //N3D module for 658x
#define 	CALIBRATION_LAYOUT_SUNNY_Q8N03D_LSC1 2  //SL 110317
#define 	MAX_CALIBRATION_LAYOUT_NUM 3
#endif
#if 1
typedef enum // : MUINT32
{
    CAM_CAL_LAYOUT_RTN_PASS = 0x0,
    CAM_CAL_LAYOUT_RTN_FAILED = 0x1,
    CAM_CAL_LAYOUT_RTN_QUEUE = 0x2
} CAM_CAL_LAYOUT_T;
#else
#define CAM_CAL_LAYOUT_RTN_PASS  0x0
#define CAM_CAL_LAYOUT_RTN_FAILED  0x1
#define CAM_CAL_LAYOUT_RTN_QUEUE  0x2

#endif
/*
#define CAL_DATA_SIZE_SLIM_LSC1_CC 	(0x290)//656
#define CAL_DATA_SIZE_N3D_DATA1_CC	(0x1C84)
#define CAL_DATA_SIZE_SUNNY_LSC1_CC   (0x290)//656 //SL 110317
*/

typedef struct
{
    UINT16 Include; //calibration layout include this item?
    UINT32 StartAddr; // item Start Address
    UINT32 BlockSize;   //BlockSize
    UINT32 (*GetCalDataProcess)(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,
    UINT32* pGetSensorCalData);
    //(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
} CALIBRATION_ITEM_STRUCT;

typedef struct
{
    UINT32 HeaderAddr; //Header Address
    UINT32 HeaderId;   //Header ID
    UINT32 DataVer;   ////new for 658x CAM_CAL_SINGLE_EEPROM_DATA, CAM_CAL_SINGLE_OTP_DATA,CAM_CAL_N3D_DATA
    CALIBRATION_ITEM_STRUCT CalItemTbl[CAMERA_CAM_CAL_DATA_LIST];
} CALIBRATION_LAYOUT_STRUCT;

/*
//Const variable
*/

//const MUINT8 CamCalPartNumber[24]={0x57,0x61,0x6E,0x70,0x65,0x69,0x20,0x4C,0x69,0x61,0x6E,0x67,
//                                   0x20,0x53,0x6F,0x70,0x68,0x69,0x65,0x52,0x79,0x61,0x6E,0x00};

const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl[MAX_CALIBRATION_LAYOUT_NUM]=
{
    {//CALIBRATION_LAYOUT_SENSOR_OTP
        0x00000001, 0x1b0c1204, CAM_CAL_SINGLE_OTP_DATA,
        {
            {0x00000000, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000041, 0x0000074C, S5K3L6_DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000009, 0x00000037, S5K3L6_DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000791, 0x0000057C, S5K3L6_DoCamCalPDAF},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN1_LEGACY:
        0x00000001, 0x020b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN2_LEGACY:
        0x00001001, 0x020b00ff, CAM_CAL_DOUBLE_EEPROM_DATA,
        {
            {0x00000001, 0x00001000, 0x00001000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00001005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00001017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00001007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN1
        0x00000001, 0x030b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN2
        0x00001001, 0x030b00ff, CAM_CAL_DOUBLE_EEPROM_DATA,
        {
            {0x00000001, 0x00001000, 0x00001000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00001005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00001017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00001007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//4cell sensor
        0x00000001, 0x050b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x000000C0, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },

};

/****************************************************************
//Global variable
****************************************************************/
static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1); //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
static MINT32 dumpEnable = 0;

static CAM_CAL_LAYOUT_T  gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//(CAM_CAL_LAYOUT_T)CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.

//MUINT32 gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//(CAM_CAL_LAYOUT_T)CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
static UINT32 g_lastSensorID = 0;
UINT32 ShowCmdErrorLog(CAMERA_CAM_CAL_TYPE_ENUM cmd)
{
    CAM_CAL_ERR("Return ERROR %s\n",CamCalErrString[cmd]);
    return 0;
}

UINT32 DoCamCalModuleVersion(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,
    UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr, err=  0;
    int CalModuleVersion;

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCalModuleVersion cam_calCfg.sensorID = %x\n", pCamCalData->sensorID);

    if(BlockSize ==	sizeof(int)) {
        cam_calCfg.u4Offset = start_addr;
        cam_calCfg.u4Length = BlockSize;
        cam_calCfg.pu1Params= (u8 *)&CalModuleVersion;
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        if(ioctlerr>0)
        {
                err = CAM_CAL_ERR_NO_ERR;
        }
        else
        {
            CAM_CAL_ERR("ioctl err\n");
            ShowCmdErrorLog(pCamCalData->Command);
        }
    }

    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG_IF(dumpEnable,"======================Module version==================\n");
    CAM_CAL_LOG_IF(dumpEnable,"[DataVer] = 0x%x\n", pCamCalData->DataVer);
    CAM_CAL_LOG_IF(dumpEnable,"RETURN = 0x%x \n", err);
    CAM_CAL_LOG_IF(dumpEnable,"======================Module version==================\n");
    #endif
    return err;
}

UINT32 DoCamCalPartNumber(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    (void) CamcamFID;
    (void) start_addr;
    (void) BlockSize;
    (void) pGetSensorCalData;

    return 0;
}


/***********************************************************************************
    Function : To read 2A infomation. Please put your AWB+AF data funtion, here.
************************************************************************************/

UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{


    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    UINT32 CalGain, FacGain;
    INT8 AWBAFConfig;


    u16 AFInf, AFMacro;
    int tempMax = 0;
    int CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;
    CAM_CAL_LOG("DoCamCal2AGain is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

#ifdef MTK_LOAD_DEBUG
    dumpEnable = 1;
#else
    dumpEnable = 0;
#endif

    memset((void*)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));//To set init value

    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    {
        if(BlockSize!=14)
        {
            CAM_CAL_ERR("BlockSize(%d) is not correct (%d)\n",BlockSize,14);
            ShowCmdErrorLog(pCamCalData->Command);
        }
        else
        {
            // Check the config. for AWB & AF
            cam_calCfg.u4Offset = (start_addr+1);
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWBAFConfig;//0x0f
            cam_calCfg.sensorID = pCamCalData->sensorID;
            cam_calCfg.deviceID = pCamCalData->deviceID;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
            if(ioctlerr>0)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }

            pCamCalData->Single2A.S2aVer = 0x01;
            pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
            //CAM_CAL_LOG_IF(dumpEnable,"S2aBitEn=0x%x", pCamCalData->Single2A.S2aBitEn);
            pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
            //memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));

            if(0x1&AWBAFConfig){
                ////AWB////
                cam_calCfg.u4Offset = (start_addr+2);
                CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", cam_calCfg.u4Offset);
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&CalGain;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                CAM_CAL_LOG_IF(dumpEnable,"Read CalGain OK %x\n",ioctlerr);

                if(ioctlerr>0)
                {
                    // Get min gain
                    CalR  = CalGain&0xFF;
                    CalGr = (CalGain>>8)&0xFF;
                    CalGb = (CalGain>>16)&0xFF;
                    CalG = ((CalGr + CalGb) + 1) >> 1;
                    CalB  = (CalGain>>24)&0xFF;

                    if(CalR > CalG) {
                        /* R > G */
                        if(CalR > CalB)
                            tempMax = CalR;
                        else
                            tempMax = CalB;
                    }
                    else {
                        /* G > R */
                        if(CalG > CalB)
                            tempMax = CalG;
                        else
                            tempMax = CalB;
                    }
                    CAM_CAL_LOG_IF(dumpEnable,"UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

                    err = CAM_CAL_ERR_NO_ERR;

                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                if (CalGain!=0 &&
                    CalGain!=0xFFFFFFFF &&
                    CalR!=0 &&
                    CalG!=0 &&
                    CalB!=0 )
                {
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax*512 + (CalR >> 1))/CalR);
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax*512 + (CalG >> 1))/CalG);
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = (u32)((tempMax*512 + (CalB >> 1))/CalB);
                }
                else
                {
                    CAM_CAL_LOG("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
                }
                cam_calCfg.u4Offset = (start_addr+6);
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&FacGain;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                CAM_CAL_LOG_IF(dumpEnable,"Read FacGain OK\n");
                if(ioctlerr>0)
                {
                    // Get min gain
                    FacR  = FacGain&0xFF;
                    FacGr = (FacGain>>8)&0xFF;
                    FacGb = (FacGain>>16)&0xFF;
                    FacG = ((FacGr + FacGb) + 1) >> 1;
                    FacB  = (FacGain>>24)&0xFF;

                    CAM_CAL_LOG_IF(dumpEnable,"Extract CalGain OKK\n");

                    if(FacR > FacG) {
                        /* R > G */
                        if(FacR > FacB)
                            tempMax = FacR;
                        else
                            tempMax = FacB;
                    }
                    else {
                        /* G > R */
                        if(FacG > FacB)
                            tempMax = FacG;
                        else
                            tempMax = FacB;
                    }

                    CAM_CAL_LOG_IF(dumpEnable,"GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

                    err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
                CAM_CAL_LOG_IF(dumpEnable,"Start assign value\n");

               if (FacGain!=0 &&
                    FacGain!=0xFFFFFFFF &&
                    FacR!=0 &&
                    FacG!=0 &&
                    FacB!=0 )
                {
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) /FacR);
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) /FacG);
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4B  = (u32)((tempMax * 512 + (FacB >> 1)) /FacB);
                }
                else
                {
                    CAM_CAL_LOG("There are something wrong on EEPROM, plz contact module vendor!! Golden R=%d G=%d B=%d\n", FacR, FacG, FacB);
                }
                //Set original data to 3A Layer
                pCamCalData->Single2A.S2aAwb.rValueR = CalR;
                pCamCalData->Single2A.S2aAwb.rValueGr = CalGr;
                pCamCalData->Single2A.S2aAwb.rValueGb = CalGb;
                pCamCalData->Single2A.S2aAwb.rValueB = CalB;
                pCamCalData->Single2A.S2aAwb.rGoldenR = FacR;
                pCamCalData->Single2A.S2aAwb.rGoldenGr = FacGr;
                pCamCalData->Single2A.S2aAwb.rGoldenGb = FacGb;
                pCamCalData->Single2A.S2aAwb.rGoldenB = FacB;
                ////Only AWB Gain Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================AWB CAM_CAL==================\n");
                CAM_CAL_LOG("[CalGain] = 0x%x\n", CalGain);
                CAM_CAL_LOG("[FacGain] = 0x%x\n", FacGain);
                CAM_CAL_LOG("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
                CAM_CAL_LOG("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
                CAM_CAL_LOG("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
                CAM_CAL_LOG("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
                CAM_CAL_LOG("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
                CAM_CAL_LOG("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                CAM_CAL_LOG("======================AWB CAM_CAL==================\n");
                #endif
            }
            if(0x2&AWBAFConfig){
                ////AF////
                cam_calCfg.u4Offset = (start_addr+10);
                cam_calCfg.u4Length = 2;
                cam_calCfg.pu1Params = (u8 *)&AFInf;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                if(ioctlerr>0)
                {
                    err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                cam_calCfg.u4Offset = (start_addr+12);
                cam_calCfg.u4Length = 2;
                cam_calCfg.pu1Params = (u8 *)&AFMacro;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                if(ioctlerr>0)
                {
                    err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                pCamCalData->Single2A.S2aAf[0] = AFInf;
                pCamCalData->Single2A.S2aAf[1] = AFMacro;

                ////Only AF Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================AF CAM_CAL==================\n");
                CAM_CAL_LOG("[AFInf] = %d\n", AFInf);
                CAM_CAL_LOG("[AFMacro] = %d\n", AFMacro);
                CAM_CAL_LOG("======================AF CAM_CAL==================\n");
                #endif
            }

            if(0x4&AWBAFConfig){ //load AF addition info
                int EEPROM_Header;
                UINT8 AF_INFO[64];

                memset(AF_INFO, 0, 64);
                cam_calCfg.u4Offset = 1;
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *) &EEPROM_Header;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                CAM_CAL_LOG_IF(dumpEnable,"EEPROM Header = %x\n", EEPROM_Header);

                if(EEPROM_Header==0x040b00ff) {
                    //print main2 AF info, only for EEPROM 0x040b00ff version
                    UINT16 AF_Inf_main2 = 0, AF_Marco_main2 = 0;
                    cam_calCfg.u4Offset = 0x89a;
                    cam_calCfg.u4Length = 2;
                    cam_calCfg.pu1Params = (u8 *)&AF_Inf_main2;
                    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                    cam_calCfg.u4Offset = 0x89c;
                    cam_calCfg.u4Length = 2;
                    cam_calCfg.pu1Params = (u8 *)&AF_Marco_main2;
                    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                    pCamCalData->Single2A.S2aAf[6] = AF_Inf_main2;
                    pCamCalData->Single2A.S2aAf[7] = AF_Marco_main2;
                    CAM_CAL_LOG_IF(dumpEnable,"[AFInf_main2] = %d\n", AF_Inf_main2);
                    CAM_CAL_LOG_IF(dumpEnable,"[AFMacro_main2] = %d\n", AF_Marco_main2);

                    cam_calCfg.u4Offset = 0x823;
                }
                else cam_calCfg.u4Offset = 0xf63;
                CAM_CAL_LOG_IF(dumpEnable,"cam_calCfg.u4Offset = %d\n", cam_calCfg.u4Offset);

                cam_calCfg.u4Length = 64;
                cam_calCfg.pu1Params = (u8 *) AF_INFO;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
                if(ioctlerr>0)
                {
                    err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
                CAM_CAL_LOG_IF(dumpEnable,"AF Test = %x %x %x %x\n", AF_INFO[6], AF_INFO[7], AF_INFO[8], AF_INFO[9]);

                pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Min_Position = (AF_INFO[0] | (AF_INFO[1]<<8));
                pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Max_Position = (AF_INFO[2] | (AF_INFO[3]<<8));
                pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Hall_AMP_Offset = AF_INFO[4];
                pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Hall_AMP_Gain = AF_INFO[5];
                pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance = (AF_INFO[6] | (AF_INFO[7]<<8));
                pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance = (AF_INFO[8] | (AF_INFO[9]<<8));
                pCamCalData->Single2A.S2aAF_t.AF_infinite_calibration_temperature = (AF_INFO[10]);
                pCamCalData->Single2A.S2aAF_t.AF_macro_calibration_temperature = (AF_INFO[11]);

                pCamCalData->Single2A.S2aAF_t.Warm_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13]<<8));
                pCamCalData->Single2A.S2aAF_t.Warm_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15]<<8));
                pCamCalData->Single2A.S2aAF_t.Warm_AF_infinite_calibration_temperature = (AF_INFO[16]);
                pCamCalData->Single2A.S2aAF_t.Warm_AF_macro_calibration_temperature = (AF_INFO[17]);

                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration = (AF_INFO[18] | (AF_INFO[19]<<8));
                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration_temperature = (AF_INFO[20]);
                pCamCalData->Single2A.S2aAF_t.Warm_AF_Middle_calibration = (AF_INFO[21] | (AF_INFO[22]<<8));
                pCamCalData->Single2A.S2aAF_t.Warm_AF_Middle_calibration_temperature = (AF_INFO[23]);

                ////AF addition info////
                #ifdef DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================AF addition CAM_CAL==================\n");
                CAM_CAL_LOG("[AF_infinite_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance);
                CAM_CAL_LOG("[AF_Macro_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance);
                CAM_CAL_LOG("[AF_Middle_calibration] = %d \n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
                CAM_CAL_LOG("======================AF addition CAM_CAL==================\n");
                #endif

            }
////////////////////////////////////////////////////////////////////////////////
        }
    }
    return err;
}



/***********************************************************************************

    Function : To read LSC Table

************************************************************************************/
UINT32 DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];
    u16 table_size;

#ifdef MTK_LOAD_DEBUG
    dumpEnable = 1;
#else
    dumpEnable = 0;
#endif

    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else
    {
        //if(pCamCalData->sensorID == 0x0386) DoCamCal_Dump(CamcamFID, 0, 6207, pGetSensorCalData);
        if(BlockSize!=CAM_CAL_SINGLE_LSC_SIZE)
        {
            CAM_CAL_LOG("BlockSize(%d) is not match (%d)\n", BlockSize, CAM_CAL_SINGLE_LSC_SIZE);
        }
        pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2;//mtk type
        pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 8;//hardcode.... need to fix

        cam_calCfg.u4Offset = (start_addr-2);
        cam_calCfg.u4Length = sizeof(table_size);
        cam_calCfg.pu1Params= (u8 *)&table_size;
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;
        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        if(!ioctlerr)
        {
            err = CAM_CAL_ERR_NO_SHADING;
        }

        if(pCamCalData->sensorID == 0x0386 || pCamCalData->sensorID == 0x0398)
        {
                table_size = 1868;
        }
        CAM_CAL_LOG("lsc table_size %d\n", table_size);
        pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
        if(table_size>0)
        {
            pCamCalData->SingleLsc.TableRotation=0;
            cam_calCfg.u4Offset = (start_addr);//|0xFFFF);
            cam_calCfg.u4Length = table_size; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType;
            CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d ", cam_calCfg.u4Offset, cam_calCfg.u4Length);
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
            if(table_size == ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];
                ShowCmdErrorLog(pCamCalData->Command);
            }
        }
    }
    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================SingleLsc Data==================\n");
    CAM_CAL_LOG("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
                                             pCamCalData->SingleLsc.LscTable.Data[1],
                                             pCamCalData->SingleLsc.LscTable.Data[2],
                                             pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG("[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);
    CAM_CAL_LOG("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
    CAM_CAL_LOG("RETURN = 0x%x \n", err);
    CAM_CAL_LOG("======================SingleLsc Data==================\n");
    #endif
//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data
    return err;
}

/******************************************************************************
This function will add after sensor support FOV data
*******************************************************************************/
UINT32 DoCamCalStereoData(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr, err=  0;
    char Stereo_Data[1360];

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal_Stereo_Data cam_calCfg.sensorID = %x\n", pCamCalData->sensorID);
    cam_calCfg.u4Offset = start_addr;
    cam_calCfg.u4Length = BlockSize;
    cam_calCfg.pu1Params= (u8 *)Stereo_Data;
    cam_calCfg.sensorID = pCamCalData->sensorID;
    cam_calCfg.deviceID = pCamCalData->deviceID;
    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
    if(ioctlerr>0)
    {
        err = CAM_CAL_ERR_NO_ERR;
    }
    else
    {
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }

#ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================DoCamCal_Stereo_Data==================\n");
    CAM_CAL_LOG("======================DoCamCal_Stereo_Data==================\n");
#endif
    return err;
}

/******************************************************************************
*seanlin 121017, MT658x
*In order to get data one block by one block instead of overall data in one time
*It must extract FileID and LayoutType from CAM_CALGetCalData()
*******************************************************************************/
UINT32 DoCamCalLayoutCheck(UINT32* pGetSensorCalData)
{
    MINT32 lCamcamFID = -1;  //seanlin 121017 01 local for layout check

    UCHAR cBuf[128] = "/dev/CAM_CAL_DRV";
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    stCAM_CAL_INFO_STRUCT  cam_calCfg;

    UINT32 CheckID = 0;
    INT32 err, i;
    switch(gIsInitedCamCal)
    {
    case CAM_CAL_LAYOUT_RTN_PASS:
        result =  CAM_CAL_ERR_NO_ERR;
        break;
    case CAM_CAL_LAYOUT_RTN_QUEUE:
    case CAM_CAL_LAYOUT_RTN_FAILED:
    default:
        result =  CAM_CAL_ERR_NO_DEVICE;
        break;
    }
    CAM_CAL_LOG_IF(dumpEnable,"g_lastSensorID=%x  pCamCalData->sensorID=%x\n", g_lastSensorID, pCamCalData->sensorID);
    if ((g_lastSensorID!=pCamCalData->sensorID))
    {
        g_lastSensorID = pCamCalData->sensorID;
        lCamcamFID = open(cBuf, O_RDWR);
        CAM_CAL_LOG_IF(dumpEnable,"lCamcamFID= 0x%x", lCamcamFID);
        if(lCamcamFID == -1)
        {
            CAM_CAL_ERR("--DoCamCalLayoutCheck--error: can't open CAM_CAL %s----\n",cBuf);
            gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_FAILED;
            result =  CAM_CAL_ERR_NO_DEVICE;
            return result;//0;
        }
        //read ID
        cam_calCfg.u4Offset = 0xFFFFFFFF;
        for (i = 0; i< MAX_CALIBRATION_LAYOUT_NUM; i++)
        {
            if (cam_calCfg.u4Offset != CalLayoutTbl[i].HeaderAddr)
            {
                CheckID = 0x00000000;
                cam_calCfg.u4Offset = CalLayoutTbl[i].HeaderAddr;
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&CheckID;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d sensorID=%x",
                                cam_calCfg.u4Offset, cam_calCfg.u4Length, cam_calCfg.sensorID);

                err= ioctl(lCamcamFID, CAM_CALIOC_G_READ , &cam_calCfg);
                if(err< 0)
                {
                    CAM_CAL_ERR("ioctl err\n");
                    CAM_CAL_ERR("Read header ID fail err = 0x%x \n",err);
                    gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_FAILED;
                    result =  CAM_CAL_ERR_NO_DEVICE;
                    break;
                }
            }
            CAM_CAL_LOG_IF(dumpEnable,"Table[%d] ID= 0x%x, CID = 0x%x", i, CalLayoutTbl[i].HeaderId, CheckID);
            if(CheckID == CalLayoutTbl[i].HeaderId)
            {
                CAM_CAL_LOG_IF(dumpEnable,"CID Matched! DevID=%d, DataVer=%d\n", pCamCalData->deviceID,
                    CalLayoutTbl[i].DataVer);

                LayoutType = i;
                gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_PASS;
                result =  CAM_CAL_ERR_NO_ERR;
                break;
            }
        }
        CAM_CAL_LOG_IF(dumpEnable,"LayoutType= 0x%x",LayoutType);
        CAM_CAL_LOG_IF(dumpEnable,"result= 0x%x",result);
        ////
        close(lCamcamFID);
    }

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCalLayoutCheck result= %x", result);
    return result;
}

/******************************************************************************
* Program Enter function
*******************************************************************************/
UINT32 CAM_CALGetCalData(UINT32* pGetSensorCalData)
{
    UCHAR cBuf[128] = "/dev/CAM_CAL_DRV";
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
//    static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1); //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    INT32 CamcamFID = 0;  //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    CAMERA_CAM_CAL_TYPE_ENUM lsCommand;

    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

#ifdef MTK_LOAD_DEBUG
        dumpEnable = 1;
#else
        dumpEnable = 0;
#endif


    CAM_CAL_LOG("pCamCalData->sensorID=%x\n", pCamCalData->sensorID);
    CAM_CAL_LOG("pCamCalData->deviceID=%x\n", pCamCalData->deviceID);


    lsCommand = pCamCalData->Command;
    CAM_CAL_LOG_IF(dumpEnable,"pCamCalData->Command = 0x%x \n",pCamCalData->Command);
    CAM_CAL_LOG_IF(dumpEnable,"lsCommand = 0x%x \n",lsCommand);
    //Make sure Layout is confirmed, first
    if(DoCamCalLayoutCheck(pGetSensorCalData)==CAM_CAL_ERR_NO_ERR)
    {
        pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[LayoutType].DataVer;
        if (1)
        {
            pthread_mutex_lock(&mEEPROM_Mutex);
            CamcamFID = open(cBuf, O_RDWR);

            if(CamcamFID == -1)
            {
                CAM_CAL_LOG_IF(dumpEnable,"--CAM_CALGetCalData--error: can't open CAM_CAL %s----\n",cBuf);
                result =  CamCalReturnErr[lsCommand];
                ShowCmdErrorLog(lsCommand);
                pthread_mutex_unlock(&mEEPROM_Mutex);
                return result;
            }
            /*********************************************/
            if ((CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].Include != 0)
            	&&(CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL))
            {
                CAM_CAL_LOG_IF(dumpEnable,"CalLayoutTbl sensorID = 0x%x \n", pCamCalData->sensorID);
                result =  CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
                                        CamcamFID,
                                        CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].StartAddr,
                                        CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].BlockSize,
                                        pGetSensorCalData);
            }
            else
            {
                result =  CamCalReturnErr[lsCommand];
                ShowCmdErrorLog(lsCommand);
            }
            /*********************************************/
            close(CamcamFID);
            pthread_mutex_unlock(&mEEPROM_Mutex);
        }
    }
    else
    {

       result =  CamCalReturnErr[lsCommand];
       ShowCmdErrorLog(lsCommand);
        return result;
    }
    CAM_CAL_LOG_IF(dumpEnable,"result = 0x%x\n",result);
    return  result;
}


UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];
    {
#ifdef MTK_LOAD_DEBUG
        dumpEnable = 1;
#else
        dumpEnable = 0;
#endif
        pCamCalData->PDAF.Size_of_PDAF = BlockSize;
        CAM_CAL_LOG_IF(dumpEnable,"PDAF start_addr =%x table_size=%d\n",start_addr, BlockSize);

        cam_calCfg.u4Offset = start_addr;
        cam_calCfg.u4Length = BlockSize;
        cam_calCfg.pu1Params= (u8 *)&pCamCalData->PDAF.Data[0];
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;
        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        if(ioctlerr > 0)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }
    }
#ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================PDAF Data==================\n");
    CAM_CAL_LOG("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
                                                    pCamCalData->PDAF.Data[1],
                                                    pCamCalData->PDAF.Data[2],
                                                    pCamCalData->PDAF.Data[3],
                                                    pCamCalData->PDAF.Data[4]);

    CAM_CAL_LOG("RETURN = 0x%x \n", err);
    CAM_CAL_LOG("======================PDAF Data==================\n");
#endif

    return err;

}

/***********************************************************************************

	Function : To read LSC Table for S5K3L6

************************************************************************************/
UINT32 S5K3L6_DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;
    int ioctlerr;	//Dream: ioctl return value is int
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    UINT8 lsc_flag;
    int i;

    unsigned char lsc_checksum;
    UINT32 Temp = 0;
    UINT8 * Value;

    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else
    {
        pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2;//mtk type
        pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 2;// 0:B,1:Gb,2:Gr,3:R

        cam_calCfg.u4Offset = start_addr - 1;
        cam_calCfg.u4Length = 1;//sizeof(table_size);
        cam_calCfg.pu1Params= (u8 *)&lsc_flag;//cam_calCfg.pu1Params = (u8 *)&CheckID;
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;

        CAM_CAL_LOG("u4Offset=%d u4Length=%d pu1Params= 0x%x ", cam_calCfg.u4Offset, cam_calCfg.u4Length, cam_calCfg.pu1Params);

        ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        CAM_CAL_LOG("ioctl=%d", ioctlerr);

        if(ioctlerr == 1)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }

        CAM_CAL_LOG("3L6 lsc_flag 0x%x\n",lsc_flag);
        pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = 1868;
        if(lsc_flag>0)
        {
            pCamCalData->SingleLsc.TableRotation=CUSTOM_CAM_CAL_ROTATION_00;
            cam_calCfg.u4Offset = start_addr;
            cam_calCfg.u4Length = BlockSize;//table_size; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType;
            CAM_CAL_LOG("u4Offset=%d u4Length=%d pu1Params= 0x%x ", cam_calCfg.u4Offset, cam_calCfg.u4Length, cam_calCfg.pu1Params);
            ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            CAM_CAL_LOG("ioctl=%d", ioctlerr);

            if(ioctlerr == BlockSize)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];
                ShowCmdErrorLog(pCamCalData->Command);
            }

        //for lsc checksum
        Value = (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType;
	    for(i = 0; i < BlockSize; i++)
		{
			//CAM_CAL_LOG("LSC CAM_CAL Addr = 0x%x, value = 0x%x\n", cam_calCfg.u4Offset++, Value[i]);
			Temp = Temp + Value[i];
		}

        cam_calCfg.u4Offset = start_addr + BlockSize;
        cam_calCfg.u4Length = 1;//sizeof(table_size);
        cam_calCfg.pu1Params= (u8 *)&lsc_checksum;//cam_calCfg.pu1Params = (u8 *)&CheckID;
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;
        CAM_CAL_LOG("u4Offset=%d u4Length=%d pu1Params= 0x%x ", cam_calCfg.u4Offset, cam_calCfg.u4Length, cam_calCfg.pu1Params);
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

        if(ioctlerr == 1)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }
		else {
			CAM_CAL_LOG("ioctl fail");
		}

        CAM_CAL_LOG("3L6 lsc_checksum 0x%x\n",lsc_checksum);

		if( (Temp % 256) == lsc_checksum )
			CAM_CAL_LOG("S5K3L6 lsc_checksum = %d is ok \n", lsc_checksum);
		else
			CAM_CAL_LOG("S5K3L6 lsc_checksum = %d isn't ok, Sum = %d, Sum%256 = %d \n", lsc_checksum, Temp, (Temp%256));

        }
    }
    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("====================SingleLsc Data for S5K3L6==================\n");
    CAM_CAL_LOG("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[1],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[2],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG("[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);

	CAM_CAL_LOG( "CapIspReg[0]=%x CapIspReg[1]=%x CapIspReg[2]=%x CapIspReg[3]=%x\n",
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3]);

    CAM_CAL_LOG("RETURN = 0x%x \n", err);
    CAM_CAL_LOG("===================SingleLsc Data for S5K3L6==================\n");
    #endif

    return err;
}

/******************************************************************************
	Function : To read AWB Table for S5K3L6
*******************************************************************************/
UINT32 S5K3L6_DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
	stCAM_CAL_INFO_STRUCT  cam_calCfg;
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	MUINT32 idx;
	int  ioctlerr; //Dream: it's value
	UINT32 err =  CamCalReturnErr[pCamCalData->Command];
	UINT32 GainValue;
	UINT8 AWB_R_G_High, AWB_R_G_Low, AWB_B_G_High, AWB_B_G_Low,AWB_R_G_Golden_High,AWB_R_G_Golden_Low,AWB_B_G_Golden_High,AWB_B_G_Golden_Low;
	u16 AWB_R_G, AWB_B_G,AWB_R_G_Golden,AWB_B_G_Golden;

	UINT8 AWBAFConfig;
	UINT8 AFInf_High,AFInf_Low, AFMacro_High,AFMacro_Low;
	u16 AFInf,AFMacro;
	UINT8 tempMin = 255, tempMax = 0;
	UINT8 tempGain, CalR, CalGr, CalGb, CalG, CalB, FacR=1, FacGr, FacGb, FacG=1, FacB=1;

	AWBAFConfig = 0x3;

    //Structure
    /*
    Byte[0]:Version
    Byte[1]: [x,x,x,x,enbAFMacro, enbAFInf, enbAF, enbWB]
    Byte[9:2]: {GoldenB, GoldenGb, GoldenGr, GoldenR, UnitB, UnitGb, UnitGr, UnitR}
    Byte[11:10]: AF inf.
    Byte[13:12]: AF Macro
    */
    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    {
        if(BlockSize!=0x37)
        {
            CAM_CAL_ERR("BlockSize(%d) is not correct (%d)\n",BlockSize,0x37);
            ShowCmdErrorLog(pCamCalData->Command);
        }
        else
        {
	     if(0x1&AWBAFConfig){
		/*Check the config. for AWB & AF*/
			// R/Gr_HIGH
	     cam_calCfg.u4Offset = 0x000A;
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWB_R_G_High;
            cam_calCfg.sensorID = pCamCalData->sensorID;
            cam_calCfg.deviceID = pCamCalData->deviceID;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            if(ioctlerr == 1)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }
			CAM_CAL_LOG("OTPdebug S5K3L6:AWB_R_G_High = 0x%x\n",AWB_R_G_High);
			// R/Gr_LOW
            cam_calCfg.u4Offset = 0x000B;
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWB_R_G_Low;
	        cam_calCfg.sensorID = pCamCalData->sensorID;
    	    cam_calCfg.deviceID = pCamCalData->deviceID;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            if(ioctlerr == 1)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }
			CAM_CAL_LOG("OTPdebug S5K3L6:AWB_R_G_Low = 0x%x\n",AWB_R_G_Low);
			// B/Gr_HIGH
            cam_calCfg.u4Offset = 0x000C;
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWB_B_G_High;
            cam_calCfg.sensorID = pCamCalData->sensorID;
            cam_calCfg.deviceID = pCamCalData->deviceID;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            if(ioctlerr == 1)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }

			CAM_CAL_LOG("OTPdebug S5K3L6:AWB_B_G_High = 0x%x\n",AWB_B_G_High);
			// B/Gr_LOW
			cam_calCfg.u4Offset = 0x000D;
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWB_B_G_Low;
            cam_calCfg.sensorID = pCamCalData->sensorID;
            cam_calCfg.deviceID = pCamCalData->deviceID;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            if(ioctlerr == 1)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }

			CAM_CAL_LOG("OTPdebug S5K3L6:AWB_B_G_Low = 0x%x\n",AWB_B_G_Low);

			AWB_R_G =((AWB_R_G_High << 8) & 0xff00) | (AWB_R_G_Low & 0xff);
			AWB_B_G =((AWB_B_G_High << 8) & 0xff00) | (AWB_B_G_Low & 0xff);
			CAM_CAL_LOG("3L6 AWB_R_G = %d\n", AWB_R_G);
			CAM_CAL_LOG("3L6 AWB_B_G = %d\n", AWB_B_G);
			//modify for 3L6 AWB OTP by wei.miao@reallytek.com 20180510 start
			pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((1024.0/ AWB_R_G)*512);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)( 512);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (u32)((1024.0/ AWB_B_G)*512);
				// Golden_R/Gr_HIGH
				cam_calCfg.u4Offset = 0x0010;
				cam_calCfg.u4Length = 1;
				cam_calCfg.pu1Params = (u8 *)&AWB_R_G_Golden_High;
		              cam_calCfg.sensorID = pCamCalData->sensorID;
		              cam_calCfg.deviceID = pCamCalData->deviceID;
				ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

				if(ioctlerr == 1)
				{
					err = CAM_CAL_ERR_NO_ERR;
				}
				else
				{
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					CAM_CAL_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				CAM_CAL_LOG("OTPdebug S5K3L6:AWB_R_G_Golden_High = 0x%x\n",AWB_R_G_Golden_High);
				// Golden_R/Gr_LOW
				cam_calCfg.u4Offset = 0x0011;
				cam_calCfg.u4Length = 1;
				cam_calCfg.pu1Params = (u8 *)&AWB_R_G_Golden_Low;
                             cam_calCfg.sensorID = pCamCalData->sensorID;
                             cam_calCfg.deviceID = pCamCalData->deviceID;
				ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

				if(ioctlerr == 1)
				{
					err = CAM_CAL_ERR_NO_ERR;
				}
				else
				{
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					CAM_CAL_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				CAM_CAL_LOG("OTPdebug S5K3L6:AWB_R_G_Golden_Low = 0x%x\n",AWB_R_G_Golden_Low);
				// Golden_B/Gr_HIGH
				cam_calCfg.u4Offset = 0x0012;
				cam_calCfg.u4Length = 1;
				cam_calCfg.pu1Params = (u8 *)&AWB_B_G_Golden_High;
		               cam_calCfg.sensorID = pCamCalData->sensorID;
		               cam_calCfg.deviceID = pCamCalData->deviceID;
				ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

				if(ioctlerr == 1)
				{
					err = CAM_CAL_ERR_NO_ERR;
				}
				else
				{
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					CAM_CAL_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				CAM_CAL_LOG("OTPdebug S5K3L6:AWB_B_G_Golden_High = 0x%x\n",AWB_B_G_Golden_High);
				// Golden_B/Gr_LOW
				cam_calCfg.u4Offset = 0x0013;
				cam_calCfg.u4Length = 1;
				cam_calCfg.pu1Params = (u8 *)&AWB_B_G_Golden_Low;
		               cam_calCfg.sensorID = pCamCalData->sensorID;
		               cam_calCfg.deviceID = pCamCalData->deviceID;
				ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

				if(ioctlerr == 1)
				{
					err = CAM_CAL_ERR_NO_ERR;
				}
				else
				{
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					CAM_CAL_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				CAM_CAL_LOG("OTPdebug S5K3L6:AWB_B_G_Golden_Low = 0x%x\n",AWB_B_G_Golden_Low);

				AWB_R_G_Golden =((AWB_R_G_Golden_High << 8) & 0xff00) | (AWB_R_G_Golden_Low & 0xff);
				AWB_B_G_Golden =((AWB_B_G_Golden_High << 8) & 0xff00) | (AWB_B_G_Golden_Low& 0xff);
				CAM_CAL_LOG_IF(dumpEnable,"S5K3L6 AWB_R_G_Golden = %d\n", AWB_R_G_Golden);
				CAM_CAL_LOG_IF(dumpEnable,"S5K3L6 AWB_B_G_Golden = %d\n", AWB_B_G_Golden);

                pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((1024.0/ AWB_R_G_Golden)*512);
                pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)( 512);
                pCamCalData->Single2A.S2aAwb.rGoldGainu4B  = (u32)((1024.0/ AWB_B_G_Golden)*512);

                /*Only AWB Gain Gathering <*/
                #ifdef DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================AWB CAM_CAL S5K3L6==================\n");
                CAM_CAL_LOG("awbotp>>S5K3L6>>[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
                CAM_CAL_LOG("awbotp>>S5K3L6>>[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
                CAM_CAL_LOG("awbotp>>S5K3L6>>[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
    			CAM_CAL_LOG("awbotp>>S5K3L6>>[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
                CAM_CAL_LOG("awbotp>>S5K3L6>>[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
                CAM_CAL_LOG("awbotp>>S5K3L6>>[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                CAM_CAL_LOG("======================AWB CAM_CAL S5K3L6==================\n");
                #endif

			}
			if(0x2&AWBAFConfig){
                ////AF////
                cam_calCfg.u4Offset = 0x003B;//(start_addr);
                cam_calCfg.u4Length = 1;
                cam_calCfg.pu1Params = (u8 *)&AFInf_High;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

                if(ioctlerr == 1)
                {
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
				CAM_CAL_LOG_IF(dumpEnable,"OTPdebug 3l6:AFInf_High = 0x%x\n",AFInf_High);

                cam_calCfg.u4Offset = 0x003C;//(start_addr);
                cam_calCfg.u4Length = 1;
                cam_calCfg.pu1Params = (u8 *)&AFInf_Low;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

                if(ioctlerr == 1)
                {
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
				CAM_CAL_LOG_IF(dumpEnable,"OTPdebug 3l6:AFInf_Low = 0x%x\n",AFInf_Low);

                cam_calCfg.u4Offset = 0x0037;//(start_addr+2);
                cam_calCfg.u4Length = 1;
                cam_calCfg.pu1Params = (u8 *)&AFMacro_High;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

                if(ioctlerr == 1)
                {
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
				CAM_CAL_LOG_IF(dumpEnable,"OTPdebug 3l6:AFMacro_High = 0x%x\n",AFMacro_High);

		  cam_calCfg.u4Offset = 0x0038;//(start_addr+2);
                cam_calCfg.u4Length = 1;
                cam_calCfg.pu1Params = (u8 *)&AFMacro_Low;
                cam_calCfg.sensorID = pCamCalData->sensorID;
                cam_calCfg.deviceID = pCamCalData->deviceID;
                ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

                if(ioctlerr == 1)
                {
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
//ITD:modify CBQHLES-582 by quan.chang 181104 start
                CAM_CAL_LOG_IF(dumpEnable,"OTPdebug 3l6:AFMacro_Low = 0x%x\n",AFMacro_Low);
#if 0
                int macroextoffset = 0;
                int infextoffset = 0;

                AFInf = ((AFInf_Low & 0x00ff) | ((AFInf_High << 8) & 0x300));
                AFMacro = ((AFMacro_Low & 0x00ff) | ((AFMacro_High << 8) & 0x300));
                if(getAFTblExtoffset)getAFTblExtoffset(1,S5K3L6_SENSOR_ID,&macroextoffset,&infextoffset);//weakref
                pCamCalData->Single2A.S2aAf[0] = AFInf + infextoffset;
                pCamCalData->Single2A.S2aAf[1] = AFMacro + macroextoffset;
                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration = 0; //lili modify

                ////Only AWB Gain Gathering <////
                #if 1//def DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================s5k3l6 AF CAM_CAL==================\n");
                CAM_CAL_LOG("s5k3l6>>>[AFInf/extoffset] = %d/%d\n", AFInf,infextoffset);
                CAM_CAL_LOG("s5k3l6>>>>>[AFMacro/extoffset] = %d/%d\n", AFMacro,macroextoffset);
                CAM_CAL_LOG("s5k3l6>>>>>[AFMiddle] = %d\n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
                CAM_CAL_LOG("======================s5k3l6 AF CAM_CAL==================\n");
                #endif
#endif
                AFInf = ((AFInf_Low & 0x00ff) | ((AFInf_High << 8) & 0x300));
                AFMacro = ((AFMacro_Low & 0x00ff) | ((AFMacro_High << 8) & 0x300));
                pCamCalData->Single2A.S2aAf[0] = AFInf;
                pCamCalData->Single2A.S2aAf[1] = AFMacro;
                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration = 0; //lili modify

								////Only AWB Gain Gathering <////
                #if 1//def DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================s5k3l6 AF CAM_CAL==================\n");
                CAM_CAL_LOG("s5k3l6>>>[AFInf/extoffset] = %d\n", AFInf);
                CAM_CAL_LOG("s5k3l6>>>>>[AFMacro/extoffset] = %d\n", AFMacro);
                CAM_CAL_LOG("s5k3l6>>>>>[AFMiddle] = %d\n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
                CAM_CAL_LOG("======================s5k3l6 AF CAM_CAL==================\n");
                #endif

//ITD:modify CBQHLES-582 by quan.chang 181104 end
            }

        }
    }
    return err;
}

/***********************************************************************************

	Function : To read PDAF for S5K3L6

************************************************************************************/
UINT32 S5K3L6_DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
	stCAM_CAL_INFO_STRUCT  cam_calCfg;
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	MUINT32 idx;
	INT32 ioctlerr;
	INT8 PDAFConfig;

	UINT32 err =  CamCalReturnErr[pCamCalData->Command];

		pCamCalData->PDAF.Size_of_PDAF = BlockSize;
		CAM_CAL_LOG("PDAF start_addr =%x table_size=%d\n",start_addr, BlockSize);

		cam_calCfg.u4Offset = 0x0790;
		cam_calCfg.u4Length = 1;
		cam_calCfg.pu1Params= (u8 *)&PDAFConfig;
		cam_calCfg.sensorID = pCamCalData->sensorID;
		cam_calCfg.deviceID = pCamCalData->deviceID;
		CAM_CAL_LOG("u4Offset=%d u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
		ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
		CAM_CAL_LOG("ioctl=%d", ioctlerr);
		if ( 1 == ioctlerr )
		{
			err = CAM_CAL_ERR_NO_ERR;
		}

		if ( 0x1&PDAFConfig )
		{
		       unsigned char Pdaf_CheckSum;
			UINT32 i, Temp1 = 0;
			UINT8 * Value;

			cam_calCfg.u4Offset = 0x0791;
			cam_calCfg.u4Length = 1404;
			cam_calCfg.pu1Params= (u8 *)&pCamCalData->PDAF.Data[0];
			cam_calCfg.sensorID = pCamCalData->sensorID;
			cam_calCfg.deviceID = pCamCalData->deviceID;
			CAM_CAL_LOG("u4Offset=%d u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
			ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
			CAM_CAL_LOG("ioctl=%d", ioctlerr);
			if( 1404 == ioctlerr )
			{
				Value = (u8 *)&pCamCalData->PDAF.Data[0];
				for(i=0; i<BlockSize; i++)
				{
					Temp1 = Temp1 + Value[i];
					//CAM_CAL_LOG("s5k3l6_DATA_PDAF_%d 0x%x  0x%x\n", i, (0x791+i), pCamCalData->PDAF.Data[i]);
				}
				CAM_CAL_LOG("Read PDAF data OK!\n");
				err = CAM_CAL_ERR_NO_ERR;
			} else {
				CAM_CAL_ERR("ioctl err\n");
				ShowCmdErrorLog(pCamCalData->Command);
			}

            cam_calCfg.u4Offset = 0x0D0D;
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params= (u8 *)&Pdaf_CheckSum;
            cam_calCfg.sensorID = pCamCalData->sensorID;
            cam_calCfg.deviceID = pCamCalData->deviceID;
            CAM_CAL_LOG("u4Offset=%d u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
			CAM_CAL_LOG("ioctl=%d", ioctlerr);
            if ( 1 == ioctlerr )
            {
                if ( Pdaf_CheckSum == (Temp1%256) )
                {
                    CAM_CAL_LOG("PDAF checksum succesd!!\n");
                    err = CAM_CAL_ERR_NO_ERR;
                } else {
                    CAM_CAL_LOG("PDAF checksum error!! CheckSum(%d)->(%d)\n", Pdaf_CheckSum, (Temp1%256));
                    err = CAM_CAL_ERR_NO_PDAF;
                }
            } else {
                CAM_CAL_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }
        }

#ifdef DEBUG_CALIBRATION_LOAD
	CAM_CAL_LOG("=====================S5K3L6 PDAF Data==================\n");
	CAM_CAL_LOG("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
													pCamCalData->PDAF.Data[1],
													pCamCalData->PDAF.Data[2],
													pCamCalData->PDAF.Data[3],
													pCamCalData->PDAF.Data[4]);

	CAM_CAL_LOG("RETURN = 0x%x \n", err);
	CAM_CAL_LOG("=====================S5K3L6 PDAF Data==================\n");
#endif

	return err;

}

UINT32 DoCamCal_Dump(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{/* dump all EEPROM info */
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    char info[8192];
    UINT32 ioctlerr;
    UINT32 err =  CAM_CAL_ERR_NO_DEVICE, idx;

    if(BlockSize > 8192) return err;

    CAM_CAL_LOG("To dump all EEPROM info.. SensorID=%x\n", pCamCalData->sensorID);

    cam_calCfg.u4Offset = (start_addr);
    cam_calCfg.u4Length = (BlockSize);
    cam_calCfg.pu1Params = (u8 *)info ;
    cam_calCfg.sensorID = pCamCalData->sensorID;
    cam_calCfg.deviceID = pCamCalData->deviceID;
    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
    if(ioctlerr>0)
    {
        err = CAM_CAL_ERR_NO_ERR;
    }
    else
    {
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
        return err;
    }
    //print info
    for(idx = 0 ; idx < BlockSize; idx++)
    {
        CAM_CAL_LOG("info[%04d]=%02x\n", idx, info[idx]);
    }

    return err;
}

