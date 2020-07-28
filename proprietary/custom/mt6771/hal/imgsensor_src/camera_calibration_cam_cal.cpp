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
#include <unistd.h>

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
#include "camera_custom_cam_cal.h"
}
#include "camera_calibration_cam_cal.h"

/* file system */
#include <stdio.h>
#include <sys/stat.h>


#define DEBUG_CALIBRATION_LOAD

#define CUSTOM_CAM_CAL_ROTATION_00 CUSTOM_CAM_CAL_ROTATION_0_DEGREE
#define CUSTOM_CAM_CAL_ROTATION_01 CUSTOM_CAM_CAL_ROTATION_0_DEGREE
#define CUSTOM_CAM_CAL_COLOR_ORDER_00 CUSTOM_CAM_CAL_COLOR_SHIFT_00
#define CUSTOM_CAM_CAL_COLOR_ORDER_01 CUSTOM_CAM_CAL_COLOR_SHIFT_00

//#define CUSTOM_CAM_CAL_PART_NUMBERS_START_ADD 5
//#define CUSTOM_CAM_CAL_NEW_MODULE_NUMBER_CHECK 1 //

#define CAM_CAL_SHOW_LOG 1
#define CAM_CAL_VER "ver1210~"   //2017.12.10

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
UINT32 DoCamCalAWBGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalStereoData(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal_Dump(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

UINT32 DoCamCal2AGainOP(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAFOP(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

#if 1
//typedef enum
enum
{
	CALIBRATION_LAYOUT_SENSOR_OTP = 0,
	CALIBRATION_LAYOUT_STEREO_MAIN1,
	CALIBRATION_LAYOUT_STEREO_MAIN2,
	CALIBRATION_LAYOUT_STEREO_MAIN1_LEGACY,
	CALIBRATION_LAYOUT_STEREO_MAIN2_LEGACY,
	CALIBRATION_LAYOUT_EXT1,
	CALIBRATION_LAYOUT_EXT2,
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
//	                                                       0x20,0x53,0x6F,0x70,0x68,0x69,0x65,0x52,0x79,0x61,0x6E,0x00};

const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl[MAX_CALIBRATION_LAYOUT_NUM]=
{
    {//CALIBRATION_LAYOUT_SENSOR_OTP
        0x00000001, 0x010b00ff, CAM_CAL_SINGLE_OTP_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
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
    {//OP format 3p8
        0x00000006, 0x0000003a, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000C00, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x0000000C, 0x0000000E, DoCamCal2AGainOP}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00001400, 0x000000C0, DoCamCalPDAFOP},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
        }
    },
    {//OP format 576
        0x00000006, 0x00000053, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000C00, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x0000000C, 0x0000000E, DoCamCal2AGainOP}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00001400, 0x000000C0, DoCamCalPDAFOP},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData}  //CAMERA_CAM_CAL_DATA_STEREO_DATA
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
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    start_addr = 0;
    BlockSize = 24;

    CAM_CAL_LOG("DoCamCalPartNumber cam_calCfg.sensorID = %x\n", pCamCalData->sensorID);
    cam_calCfg.u4Offset = start_addr;
    cam_calCfg.u4Length = BlockSize;
    cam_calCfg.pu1Params= (u8 *)&pCamCalData->PartNumber[0];
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
    CAM_CAL_LOG("======================Part Number==================\n");
    CAM_CAL_LOG("[Part Number] = %d %d\n", pCamCalData->PartNumber[5], pCamCalData->PartNumber[6]);
    CAM_CAL_LOG("======================Part Number==================\n");
    #endif
    return err;
}

UINT32 DoCamCalAWBGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    {
        if(BlockSize!=CAM_CAL_SINGLE_AWB_COUNT_BYTE)
        {
            CAM_CAL_ERR("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_SINGLE_AWB_COUNT_BYTE);
            ShowCmdErrorLog(pCamCalData->Command);
        }
        else
        {
            ////Only AWB Gain without AF>////
            pCamCalData->Single2A.S2aVer = 0x01;
            pCamCalData->Single2A.S2aBitEn = CAM_CAL_AWB_BITEN;
            pCamCalData->Single2A.S2aAfBitflagEn = 0x0;// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
            memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));
            ////Only AWB Gain without AF<////
            ////Only AWB Gain Gathering >////
            cam_calCfg.u4Offset = start_addr|0xFFFF;
            cam_calCfg.u4Length = 4;
            cam_calCfg.pu1Params = (u8 *)&PregainFactor;
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
            cam_calCfg.u4Offset = start_addr+4;
            cam_calCfg.u4Length = 4;
            cam_calCfg.pu1Params = (u8 *)&PregainOffset;
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

            PregainFactorH = ((PregainFactor>>16)&0xFFFF);
            PregainOffsetH = ((PregainOffset>>16)&0xFFFF);
            if((PregainOffset==0)||(PregainOffsetH==0))
            {
                //pre gain
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = 512;
                CAM_CAL_LOG_IF(dumpEnable,"Pegain has no Calinration Data!!!\n");
            }
            else
            {
                //pre gain
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R =
                                                                           (((PregainFactor&0xFF)<<8)|
                                                                       ((PregainFactor&0xFF00)>>8))*512 /
                                                                           (((PregainOffset&0xFF)<<8)|
                                                                       ((PregainOffset&0xFF00)>>8));
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  =
                                                                           (((PregainFactorH&0xFF)<<8)|
                	                                                 ((PregainFactorH&0xFF00)>>8))*512/
                	                                                     (((PregainOffsetH&0xFF)<<8)|
                	                                                 ((PregainOffsetH&0xFF00)>>8));
            	err=0;
            }

            if((pCamCalData->Single2A.S2aAwb.rUnitGainu4R==0)||(pCamCalData->Single2A.S2aAwb.rUnitGainu4B==0))
            {
                //pre gain
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = 512;
                CAM_CAL_ERR("RGB Gain is not reasonable!!!\n");
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                ShowCmdErrorLog(pCamCalData->Command);
            }
            ////Only AWB Gain Gathering <////
            #ifdef DEBUG_CALIBRATION_LOAD
            CAM_CAL_LOG_IF(dumpEnable,"======================AWB CAM_CAL==================\n");
            CAM_CAL_LOG_IF(dumpEnable,"[CAM_CAL PREGAIN VALUE] = 0x%x\n", PregainFactor);
            CAM_CAL_LOG_IF(dumpEnable,"[CAM_CAL PREGAIN OFFSET] = 0x%x\n", PregainOffset);
            CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
            CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4G] = %d\n",  pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
            CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
            CAM_CAL_LOG_IF(dumpEnable,"======================AWB CAM_CAL==================\n");
            #endif
////////////////////////////////////////////////////////////////////////////////
        }
    }
    return err;
}



/***********************************************************************************

	Function : To read 2A infomation. Please put your AWB+AF data funtion, here.

************************************************************************************/

UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{

	stCAM_CAL_INFO_STRUCT  cam_calCfg;
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	UINT32 ioctlerr;
	UINT32 err =  CamCalReturnErr[pCamCalData->Command];

	UINT32 CalGain, FacGain;
	UINT8 AWBAFConfig;
	u16 AFInf, AFMacro;
	UINT8 tempMax = 0;
	UINT8 CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;
	CAM_CAL_LOG("DoCamCal2AGain is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

#ifdef MTK_LOAD_DEBUG
	dumpEnable = 1;
#else
	dumpEnable = 0;
#endif

	//Structure
	/*
	Byte[0]:Version
	Byte[1]: [x,x,x,x,enbAFMacro, enbAFInf, enbAF, enbWB]
	Byte[9:2]: {GoldenB, GoldenGb, GoldenGr, GoldenR, UnitB, UnitGb, UnitGr, UnitR}
	Byte[11:10]: AF inf.
	Byte[13:12]: AF Macro
	*/
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
                CAM_CAL_LOG_IF(dumpEnable, "======================AWB CAM_CAL==================\n");
                CAM_CAL_LOG_IF(dumpEnable, "[CalGain] = 0x%x\n", CalGain);
                CAM_CAL_LOG_IF(dumpEnable, "[FacGain] = 0x%x\n", FacGain);
                CAM_CAL_LOG("[UnitGain] R=%d G=%d B=%d\n",
                            pCamCalData->Single2A.S2aAwb.rUnitGainu4R,
                            pCamCalData->Single2A.S2aAwb.rUnitGainu4G,
                            pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
                CAM_CAL_LOG("[GoldenGain] R=%d G=%d B=%d\n",
                            pCamCalData->Single2A.S2aAwb.rGoldGainu4R,
                            pCamCalData->Single2A.S2aAwb.rGoldGainu4G,
                            pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                CAM_CAL_LOG_IF(dumpEnable, "======================AWB CAM_CAL==================\n");
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
                CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
                CAM_CAL_LOG("[AF] Inf=%d Marco=%d\n", AFInf, AFMacro);
                CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
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
                CAM_CAL_LOG_IF(dumpEnable, "======================AF addition CAM_CAL==================\n");
                CAM_CAL_LOG("[AF distance] Inf=%dmm Marco=%dmm\n",
                            pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance,
                            pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance);
                CAM_CAL_LOG("[AF_Middle_calibration] = %d \n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
                CAM_CAL_LOG_IF(dumpEnable, "======================AF addition CAM_CAL==================\n");
                #endif

            }
////////////////////////////////////////////////////////////////////////////////
        }
    }
    return err;
}




/***********************************************************************************

   Function : To read 2A infomation. Please put your AWB+AF data funtion, here.

************************************************************************************/

UINT32 DoCamCal2AGainOP(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{

    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    long long CalGain, FacGain;
    UINT8 AWBAFConfig = 0xf;
    UINT16 AFInf, AFMacro;
    UINT16 tempMax = 0;
    UINT16 CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;
    CAM_CAL_LOG("DoCamCal2AGainOP is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

    (void) start_addr;

    dumpEnable = 1;

    memset((void*)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));//To set init value

    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    {
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;

        pCamCalData->Single2A.S2aVer = 0x01;
        pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
        pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);

        if(0x1&AWBAFConfig){
            ////AWB////
            cam_calCfg.u4Offset = 0x0c;
            CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", cam_calCfg.u4Offset);
            cam_calCfg.u4Length = 8;
            cam_calCfg.pu1Params = (u8 *)&CalGain;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

            if(ioctlerr>0)
            {
                // Get min gain
                CalR  = CalGain&0xFFFF;
                CalGr = (CalGain>>16)&0xFFFF;
                CalGb = (CalGain>>32)&0xFFFF;
                CalG = ((CalGr + CalGb) + 1) >> 1;
                CalB  = (CalGain>>48)&0xFFFF;

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
            cam_calCfg.u4Offset = 0x14;
            cam_calCfg.u4Length = 8;
            cam_calCfg.pu1Params = (u8 *)&FacGain;
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
            CAM_CAL_LOG_IF(dumpEnable,"Read FacGain OK\n");
            if(ioctlerr>0)
            {
                // Get min gain
                FacR  = FacGain&0xFFFF;
                FacGr = (FacGain>>16)&0xFFFF;
                FacGb = (FacGain>>32)&0xFFFF;
                FacG = ((FacGr + FacGb) + 1) >> 1;
                FacB  = (FacGain>>48)&0xFFFF;

                CAM_CAL_LOG_IF(dumpEnable,"FacGain=%lld\n", FacGain);

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
            CAM_CAL_LOG_IF(dumpEnable, "======================AWB CAM_CAL==================\n");
            CAM_CAL_LOG_IF(dumpEnable, "[CalGain] = %lld\n", CalGain);
            CAM_CAL_LOG_IF(dumpEnable, "[FacGain] = %lld\n", FacGain);
            CAM_CAL_LOG("[UnitGain] R=%d G=%d B=%d\n",
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4R,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4G,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
            CAM_CAL_LOG("[GoldenGain] R=%d G=%d B=%d\n",
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4R,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4G,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
            CAM_CAL_LOG_IF(dumpEnable, "======================AWB CAM_CAL==================\n");
            #endif
        }
        if(0x2&AWBAFConfig){
            ////AF////
            cam_calCfg.u4Offset = 0x26;
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

            cam_calCfg.u4Offset = 0x24;
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
            CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
            CAM_CAL_LOG("[AF] Inf=%d Marco=%d\n", AFInf, AFMacro);
            CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
            #endif
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
        //if(pCamCalData->sensorID == 0x3108) DoCamCal_Dump(CamcamFID, 0, 6759, pGetSensorCalData);
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
        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=0x%x u4Length=%d", cam_calCfg.u4Offset, cam_calCfg.u4Length);
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        if(!ioctlerr)
        {
            err = CAM_CAL_ERR_NO_SHADING;
        }

        if(pCamCalData->sensorID == 0x0386 || pCamCalData->sensorID == 0x2107 || pCamCalData->sensorID == 0x3108)
        {
                table_size = 1868;
        }
        if(start_addr == 0x0C00)//OP case
        {
                table_size = 1868;
        }
        CAM_CAL_LOG("lsc table_size %d\n", table_size);
        pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
        if(table_size>0)
        {
            pCamCalData->SingleLsc.TableRotation=CUSTOM_CAM_CAL_ROTATION_00;
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
    CAM_CAL_LOG_IF(dumpEnable, "======================SingleLsc Data==================\n");
    CAM_CAL_LOG("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
                                             pCamCalData->SingleLsc.LscTable.Data[1],
                                             pCamCalData->SingleLsc.LscTable.Data[2],
                                             pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
    CAM_CAL_LOG_IF(dumpEnable, "RETURN = 0x%x \n", err);
    CAM_CAL_LOG_IF(dumpEnable, "======================SingleLsc Data==================\n");
    #endif

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

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal_Stereo_Data cam_calCfg.sensorID = %x\n", pCamCalData->sensorID);
    pCamCalData->Stereo_Data.Size_of_Stereo_Data = CAM_CAL_Stereo_Data_SIZE;
    cam_calCfg.u4Offset = start_addr;
    cam_calCfg.u4Length = BlockSize;
    cam_calCfg.pu1Params= (u8 *)&pCamCalData->Stereo_Data.Data;
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
    CAM_CAL_LOG_IF(dumpEnable, "======================DoCamCal_Stereo_Data==================\n");
    CAM_CAL_LOG("Stereo_Data=%x %x %x %x\n",
        pCamCalData->Stereo_Data.Data[0], pCamCalData->Stereo_Data.Data[1],
        pCamCalData->Stereo_Data.Data[2], pCamCalData->Stereo_Data.Data[3]);
#endif
    return err;
}

/******************************************************************************
*In order to get data one block by one block instead of overall data in one time
*It must extract FileID and LayoutType from CAM_CALGetCalData()
*******************************************************************************/
UINT32 DoCamCalLayoutCheck(UINT32* pGetSensorCalData)
{
    MINT32 lCamcamFID = -1;

    UCHAR cBuf[128] = "/dev/CAM_CAL_DRV";
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    stCAM_CAL_INFO_STRUCT  cam_calCfg;

    UINT32 CheckID = 0, i;
    INT32 err;
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

            if(i >= CALIBRATION_LAYOUT_EXT1 ) { //op format
                CheckID &= 0xffff;
            }

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
    INT32 CamcamFID = 0;

    CAMERA_CAM_CAL_TYPE_ENUM lsCommand;

    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

#ifdef MTK_LOAD_DEBUG
    dumpEnable = 1;
#else
    dumpEnable = 0;
#endif



    CAM_CAL_LOG("sensorID=%x deviceID=%x\n", pCamCalData->sensorID, pCamCalData->deviceID);


    lsCommand = pCamCalData->Command;
    CAM_CAL_LOG_IF(dumpEnable,"pCamCalData->Command = 0x%x \n",pCamCalData->Command);
    CAM_CAL_LOG_IF(dumpEnable,"lsCommand = 0x%x \n",lsCommand);
    //Make sure Layout is confirmed, first
    if(DoCamCalLayoutCheck(pGetSensorCalData)==CAM_CAL_ERR_NO_ERR)
    {
        pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[LayoutType].DataVer;
        CamcamFID = open(cBuf, O_RDWR);

        if(CamcamFID == -1)
        {
            CAM_CAL_LOG_IF(dumpEnable,"--CAM_CALGetCalData--error: can't open CAM_CAL %s----\n",cBuf);
            result =  CamCalReturnErr[lsCommand];
            ShowCmdErrorLog(lsCommand);
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


/***********************************************************************************
 Function : To read PDAF infomation.
************************************************************************************/
UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,
	UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr;
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
    CAM_CAL_LOG_IF(dumpEnable, "======================PDAF Data==================\n");
    CAM_CAL_LOG("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
                                                    pCamCalData->PDAF.Data[1],
                                                    pCamCalData->PDAF.Data[2],
                                                    pCamCalData->PDAF.Data[3],
                                                    pCamCalData->PDAF.Data[4]);

    CAM_CAL_LOG_IF(dumpEnable, "RETURN = 0x%x \n", err);
    CAM_CAL_LOG_IF(dumpEnable, "======================PDAF Data==================\n");
#endif

    return err;

}


/***********************************************************************************
 Function : To read PDAF infomation for OP
************************************************************************************/

UINT32 DoCamCalPDAFOP(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,
	UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    (void) start_addr;
    (void) BlockSize;

    pCamCalData->PDAF.Size_of_PDAF = 496;
    CAM_CAL_LOG("PDAF read proc1 496 byte");

    cam_calCfg.u4Offset = 0x1400;
    cam_calCfg.u4Length = 496;
    cam_calCfg.pu1Params= (u8 *)&pCamCalData->PDAF.Data[0];
    cam_calCfg.sensorID = pCamCalData->sensorID;
    cam_calCfg.deviceID = pCamCalData->deviceID;

    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
    if(ioctlerr > 0) {
        CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[0], pCamCalData->PDAF.Data[1]);
        CAM_CAL_LOG("PDAF read proc2 806 byte");
        pCamCalData->PDAF.Size_of_PDAF += 806;

        cam_calCfg.u4Offset = 0x1600;
        cam_calCfg.u4Length = 806;
        cam_calCfg.pu1Params= (u8 *)&pCamCalData->PDAF.Data[496];
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;

        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

        if(ioctlerr > 0) {
             CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[496], pCamCalData->PDAF.Data[497]);
             CAM_CAL_LOG("PDAF read proc3 102 byte");
             pCamCalData->PDAF.Size_of_PDAF += 102;
             cam_calCfg.u4Offset = 0x1A00;
             cam_calCfg.u4Length = 102;
             cam_calCfg.pu1Params= (u8 *)&pCamCalData->PDAF.Data[1302];
             cam_calCfg.sensorID = pCamCalData->sensorID;
             cam_calCfg.deviceID = pCamCalData->deviceID;

             ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);

             if(ioctlerr > 0) {
                 CAM_CAL_LOG("PDAF %x %x  all size=%d",
                     pCamCalData->PDAF.Data[1302], pCamCalData->PDAF.Data[1303],
                     pCamCalData->PDAF.Size_of_PDAF);
                 err = CAM_CAL_ERR_NO_ERR;
             }
        }
    }
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

static int Is_dump = 0;
#define MAX_NUM_EEPROM 4096
UINT32 DoCamCal_Dump_All(INT32 CamcamFID, UINT32* pGetSensorCalData)
{/* dump all EEPROM info */
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    char info[MAX_NUM_EEPROM];
    UINT32 ioctlerr;
    UINT32 err =  CAM_CAL_ERR_NO_DEVICE, idx;

    int m_bDebugEnable = property_get_int32("vendor.debug.eeprom.enable", 0);

    CAM_CAL_LOG_IF(dumpEnable, "To dump all EEPROM info.. m_bDebugEnable=%x Is_dump=%d\n", m_bDebugEnable, Is_dump);

/* Step 1: check if you want to dump */
    if (m_bDebugEnable == 0) return err;

    ioctlerr = mkdir("/sdcard/EEPROM", S_IRWXU | S_IRWXG | S_IXOTH);
    CAM_CAL_LOG( "create folder /sdcard/EEPROM (%d)", ioctlerr);

/* Step 2: if we have dump, then don't to dump again  */
    CAM_CAL_LOG("Is_dump=%d pCamCalData->deviceID=%d &=%d ", Is_dump, pCamCalData->deviceID, (Is_dump & pCamCalData->deviceID) );

    if ((Is_dump & pCamCalData->deviceID) == 0) {
        cam_calCfg.u4Offset = 0;
        cam_calCfg.u4Length = MAX_NUM_EEPROM;
        cam_calCfg.pu1Params = (u8 *)info ;
        cam_calCfg.sensorID = pCamCalData->sensorID;
        cam_calCfg.deviceID = pCamCalData->deviceID;
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
        if (ioctlerr > 0) {
           FILE *fp = NULL;
/* Step 3: if we get data, set flag, and ready to dump */
            Is_dump |= pCamCalData->deviceID;
/* Step 4: Wrtie basic data to file */
            if (cam_calCfg.deviceID == 1)
              fp= fopen("/sdcard/EEPROM/Main_Sensor", "w");
            else if (cam_calCfg.deviceID == 2)
              fp = fopen("/sdcard/EEPROM/Sub_Sensor", "w");
            else if (cam_calCfg.deviceID == 4)
              fp = fopen("/sdcard/EEPROM/Main2_Sensor", "w");
            else if (cam_calCfg.deviceID == 8)
              fp = fopen("/sdcard/EEPROM/Sub2_Sensor", "w");
            else if (cam_calCfg.deviceID == 16)
              fp = fopen("/sdcard/EEPROM/Main3_Sensor", "w");

            if(fp == NULL) {
                 CAM_CAL_LOG( "open file fail!");
                 return err;
            }
/* Step 5: Wrtie dump data to file */
            fprintf( fp, "SensorID=0x%x\n", pCamCalData->sensorID);
            //print info
            for(idx = 0 ; idx < MAX_NUM_EEPROM; idx++) {
               fprintf(fp, "0x%04d,0x%02x\n", idx, info[idx]);
            }
/* Step 6: Close the file */
            Is_dump |= pCamCalData->deviceID;
            CAM_CAL_LOG( "dump data is ok! Is_dump=%d", Is_dump);
            err = CAM_CAL_ERR_NO_ERR;
            fclose(fp);
        }
        else
        {
            CAM_CAL_ERR("ioctl err\n");
            ShowCmdErrorLog(pCamCalData->Command);
            return err;
        }
    }
    else
       CAM_CAL_LOG("There is exist data for device %d\n", pCamCalData->deviceID);


    return err;
}

