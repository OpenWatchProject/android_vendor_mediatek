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

/* file system */
#include <stdio.h>
#include <sys/stat.h>


#define DEBUG_CALIBRATION_LOAD

#define CUSTOM_CAM_CAL_ROTATION_00 0
#define CUSTOM_CAM_CAL_ROTATION_01 CUSTOM_CAM_CAL_ROTATION_0_DEGREE
#define CUSTOM_CAM_CAL_COLOR_ORDER_00 CUSTOM_CAM_CAL_COLOR_SHIFT_00
#define CUSTOM_CAM_CAL_COLOR_ORDER_01 CUSTOM_CAM_CAL_COLOR_SHIFT_00

#define CAM_CAL_ERR_NO_DEVICE   0x8FFFFFFF

//#define CUSTOM_CAM_CAL_NEW_MODULE_NUMBER_CHECK 1 //

#define CAM_CAL_SHOW_LOG 1
#define CAM_CAL_VER "ver0723" //2018.07.22

#ifdef CAM_CAL_SHOW_LOG
//#define CAM_CAL_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#define CAM_CAL_LOG(fmt, arg...)    ALOGD(CAM_CAL_VER " " fmt, ##arg)
#define CAM_CAL_ERR(fmt, arg...)    ALOGE(CAM_CAL_VER "Err: %5d: " fmt, __LINE__, ##arg)
#else
#define CAM_CAL_LOG(fmt, arg...)    void(0)
#define CAM_CAL_ERR(fmt, arg...)    void(0)
#endif
#define CAM_CAL_LOG_IF(cond, ...)      do { if ( (cond) ) { CAM_CAL_LOG(__VA_ARGS__); } }while(0)


UINT32 DoCamCalModuleVersion(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPartNumber(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalSingleLsc(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGain(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalStereoData(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAF(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal_Dump(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal_Dump_All(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalLensId(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

UINT32 DoCamCalSingleLscOP(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGainOP519(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGainOP586(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGainOP576(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAFOP586_T0(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAFOP586_T1(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalPDAFOP576(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalLensIdOP(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

#define SUPPORT_FOR_QUAM_FORMAT 1

#if SUPPORT_FOR_QUAM_FORMAT
UINT32 DoCamCal2AGainQ(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
#endif



static pthread_mutex_t mEEPROM_Mutex = PTHREAD_MUTEX_INITIALIZER;


#if 1
enum
{
    CALIBRATION_LAYOUT_SENSOR_OTP = 0,
    CALIBRATION_LAYOUT_STEREO_MAIN1,
    CALIBRATION_LAYOUT_STEREO_MAIN2,
    CALIBRATION_LAYOUT_STEREO_MAIN1_LEGACY,
    CALIBRATION_LAYOUT_STEREO_MAIN2_LEGACY,
    CALIBRATION_LAYOUT_STEREO_MAIN_MTK_1_4,
    CALIBRATION_LAYOUT_STEREO_SUB_MTK_1_4,
    CALIBRATION_LAYOUT_EXT_OP_519,
    CALIBRATION_LAYOUT_EXT_OP_586_T1,
    CALIBRATION_LAYOUT_EXT_OP_586_T0,
    CALIBRATION_LAYOUT_EXT_OP_576,
    MAX_CALIBRATION_LAYOUT_NUM
};

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
    UINT32 (*GetCalDataProcess)(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize,
            UINT32* pGetSensorCalData);
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
        0x00000001, 0x010b00ff, CAM_CAL_SINGLE_OTP_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN1_LEGACY:
        0x00000001, 0x020b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN2_LEGACY:
        0x00001001, 0x020b00ff, CAM_CAL_DOUBLE_EEPROM_DATA,
        {
            {0x00000001, 0x00001000, 0x00001000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00001005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00001017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00001007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN1
        0x00000001, 0x030b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {//CALIBRATION_LAYOUT_STEREO_MAIN_MTK_1_4
        0x00000001, 0x040b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x000000C0, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {//Four Cell
        0x00000001, 0x050b00ff, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x000000C0, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
#ifdef SUPPORT_FOR_QUAM_FORMAT
    {//QUAM FORMAT
        0x00000006, 0x0062001a, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion},
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber},
            {0x00000001, 0x00000C00, 0x0000074C, DoCamCalSingleLsc},
            {0x00000001, 0x0000000C, 0x0000000E, DoCamCal2AGainQ},
            {0x00000001, 0x00001400, 0x00000800, DoCamCalPDAF},
            {0x00000000, 0x00000000, 0x00000000, DoCamCalStereoData}
        }
    },
#endif
    {//OP format 519
        0x00000006, 0x00820052, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000000, 0x0000000C, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000E50, 0x0000074C, DoCamCalSingleLscOP}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x0000000C, 0x0000000E, DoCamCal2AGainOP519}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000B06, 0x00000060, DoCamCalPDAF},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000008, 0x00000002, DoCamCalLensIdOP}
        }
    },
    {//OP format 586 (T1)
        0x00000006, 0x00B40056, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000000, 0x0000000C, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000530, 0x0000074C, DoCamCalSingleLscOP}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000010, 0x0000000E, DoCamCal2AGainOP586}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00001E90, 0x000001F0, DoCamCalPDAFOP586_T1},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x000024CA, DoCamCal_Dump_All},
            {0x00000001, 0x00000008, 0x00000002, DoCamCalLensIdOP}
        }
    },
    {//OP format 586 (T0)
        0x00000006, 0x00D20056, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000000, 0x0000000C, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000530, 0x0000074C, DoCamCalSingleLscOP}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000010, 0x0000000E, DoCamCal2AGainOP586}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00001E90, 0x000001F0, DoCamCalPDAFOP586_T0},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x000024CA, DoCamCal_Dump_All},
            {0x00000001, 0x00000008, 0x00000002, DoCamCalLensIdOP}
        }
    },
    {//OP format 576
        0x00000006, 0x006D0053, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000000, 0x0000000C, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000C00, 0x0000074C, DoCamCalSingleLscOP}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x0000000C, 0x0000000E, DoCamCal2AGainOP576}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00001400, 0x000000C0, DoCamCalPDAFOP576},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001F00, DoCamCal_Dump_All},
            {0x00000001, 0x00000008, 0x00000002, DoCamCalLensIdOP}
        }
    },
};

/****************************************************************
//Global variable
****************************************************************/

static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1);
static MINT32 dumpEnable = 0;
static CAM_CAL_LAYOUT_T  gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;
static UINT32 g_lastSensorID = 0;

UINT32 ShowCmdErrorLog(CAMERA_CAM_CAL_TYPE_ENUM cmd)
{
    CAM_CAL_ERR("Return ERROR %s\n",CamCalErrString[cmd]);
    return 0;
}

INT32 getMtkFormatVersion(ICamCalHelper *pCamCalHelper, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ret = 0;

    if (pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                0xFA3, 1, (u8 *) &ret) > 0 ) {
        CAM_CAL_LOG("Mtk format version = 0x%x\n", ret);
    }
    else {
        CAM_CAL_ERR("ioctl err\n");
        ret = -1;
    }

    return ret;
}

/*********************************************************************/

UINT32 DoCamCalModuleVersion(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize,
    UINT32* pGetSensorCalData)
{
    (void) pCamCalHelper;
    (void) start_addr;
    (void) BlockSize;
    (void) pGetSensorCalData;

    return 0;
}

UINT32 DoCamCalPartNumber(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];
    UINT32 sizeLimit = sizeof(pCamCalData->PartNumber);

    memset(&pCamCalData->PartNumber[0], 0, sizeLimit);

    if (BlockSize > sizeLimit) {
        CAM_CAL_ERR("part number size can't larger than %u\n", sizeLimit);
        return err;
    }

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)&pCamCalData->PartNumber[0]);
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
    CAM_CAL_LOG("[Part Number] = %x %x %x %x %x %x %x %x %x %x %x %x\n",
            pCamCalData->PartNumber[0], pCamCalData->PartNumber[1],
            pCamCalData->PartNumber[2], pCamCalData->PartNumber[3],
            pCamCalData->PartNumber[4], pCamCalData->PartNumber[5],
            pCamCalData->PartNumber[6], pCamCalData->PartNumber[7],
            pCamCalData->PartNumber[8], pCamCalData->PartNumber[9],
            pCamCalData->PartNumber[10], pCamCalData->PartNumber[11]);
#endif
    return err;
}


/***********************************************************************************
    Function : To read 2A infomation. Please put your AWB+AF data funtion, here.
************************************************************************************/

UINT32 DoCamCal2AGain(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];

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
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    start_addr + 1, 1, (u8 *)&AWBAFConfig);
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
                CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", start_addr + 2);
                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        start_addr + 2, 4, (u8 *)&CalGain);
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
                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        start_addr + 6, 4, (u8 *)&FacGain);
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
                CAM_CAL_LOG("[CalGain] = 0x%x, [FacGain] = 0x%x\n", CalGain, FacGain);
                CAM_CAL_LOG("[rCalGain.u4R] = %d, [rCalGain.u4G] = %d, [rCalGain.u4B] = %d\n",
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4R,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4G,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
                CAM_CAL_LOG("[rFacGain.u4R] = %d, [rFacGain.u4G] = %d, [rFacGain.u4B] = %d\n",
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4R,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4G,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                #endif
            }
            if(0x2&AWBAFConfig){
                ////AF////
                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        start_addr + 10, 2, (u8 *)&AFInf);
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

                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        start_addr + 12, 2, (u8 *)&AFMacro);
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
                CAM_CAL_LOG("[AFInf] = %d, [AFMacro] = %d\n", AFInf, AFMacro);
                #endif
            }

            if(0x4&AWBAFConfig){ //load AF addition info
                int EEPROM_Header;
                UINT8 AF_INFO[64];
                unsigned int af_info_offset;

                memset(AF_INFO, 0, 64);
                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        1, 4, (u8 *) &EEPROM_Header);
                CAM_CAL_LOG_IF(dumpEnable,"EEPROM Header = %x\n", EEPROM_Header);

                if(EEPROM_Header==0x040b00ff) {
                    //print main2 AF info, only for EEPROM 0x040b00ff version
                    UINT16 AF_Inf_main2 = 0, AF_Marco_main2 = 0;
                    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                            0x89a, 2, (u8 *)&AF_Inf_main2);
                    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                            0x89c, 2, (u8 *)&AF_Marco_main2);
                    pCamCalData->Single2A.S2aAf[6] = AF_Inf_main2;
                    pCamCalData->Single2A.S2aAf[7] = AF_Marco_main2;
                    CAM_CAL_LOG_IF(dumpEnable,"[AFInf_main2] = %d\n", AF_Inf_main2);
                    CAM_CAL_LOG_IF(dumpEnable,"[AFMacro_main2] = %d\n", AF_Marco_main2);

                    af_info_offset = 0x823;
                }
                else af_info_offset = 0xf63;
                CAM_CAL_LOG_IF(dumpEnable,"af_info_offset = %d\n", af_info_offset);

                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        af_info_offset, 64, (u8 *) AF_INFO);
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

                if (getMtkFormatVersion(pCamCalHelper, pGetSensorCalData) == 0x18) {
                    pCamCalData->Single2A.S2aAF_t.Posture_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13]<<8));
                    pCamCalData->Single2A.S2aAF_t.Posture_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15]<<8));
                    pCamCalData->Single2A.S2aAF_t.Posture_AF_infinite_calibration_temperature = (AF_INFO[16]);
                    pCamCalData->Single2A.S2aAF_t.Posture_AF_macro_calibration_temperature = (AF_INFO[17]);
                } else {
                    pCamCalData->Single2A.S2aAF_t.Warm_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13]<<8));
                    pCamCalData->Single2A.S2aAF_t.Warm_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15]<<8));
                    pCamCalData->Single2A.S2aAF_t.Warm_AF_infinite_calibration_temperature = (AF_INFO[16]);
                    pCamCalData->Single2A.S2aAF_t.Warm_AF_macro_calibration_temperature = (AF_INFO[17]);

                    pCamCalData->Single2A.S2aAF_t.Warm_AF_Middle_calibration = (AF_INFO[21] | (AF_INFO[22]<<8));
                    pCamCalData->Single2A.S2aAF_t.Warm_AF_Middle_calibration_temperature = (AF_INFO[23]);
                }

                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration = (AF_INFO[18] | (AF_INFO[19]<<8));
                pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration_temperature = (AF_INFO[20]);

                ////AF addition info////
                #ifdef DEBUG_CALIBRATION_LOAD
                CAM_CAL_LOG("======================AF addition CAM_CAL==================\n");
                CAM_CAL_LOG("[AF_infinite_pattern_distance] = %dmm, [AF_Macro_pattern_distance] = %dmm\n",
                        pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance,
                        pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance);
                CAM_CAL_LOG("[AF_Middle_calibration] = %d \n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
                #endif

            }
////////////////////////////////////////////////////////////////////////////////
        }
    }
    return err;
}

#if SUPPORT_FOR_QUAM_FORMAT
/***********************************************************************************

    Function : To read 2A infomation. This function is special case.

************************************************************************************/
UINT32 DoCamCal2AGainQ(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];

    char CalGain[16];
    INT8 AWBAFConfig = 0x0f;
    INT16 AFInf, AFMacro;
    INT8 tempMax = 0;
    INT8 CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal2AGainQ is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

    pCamCalData->Single2A.S2aVer = 0x01;
    pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
    pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7

    if(0x1&AWBAFConfig){
      ////AWB////
      CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", start_addr);
      ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
              start_addr, 16, (u8 *)CalGain);
      CAM_CAL_LOG_IF(dumpEnable,"Read CalGain OK %x\n",ioctlerr);

      if(ioctlerr>0)
      {
        // Get min gain
        CalR  = CalGain[0];
        CalGr = CalGain[2];
        CalGb = CalGain[4];
        CalG = ((CalGr + CalGb) + 1) >> 1;
        CalB  = CalGain[6];

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
        if (CalR!=0 &&
            CalG!=0 &&
            CalB!=0 ) {
            pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax*512 + (CalR >> 1))/CalR);
            pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax*512 + (CalG >> 1))/CalG);
            pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = (u32)((tempMax*512 + (CalB >> 1))/CalB);
        }

        FacR  = CalGain[8];
        FacGr = CalGain[10];
        FacGb = CalGain[12];
        FacG = ((FacGr + FacGb) + 1) >> 1;
        FacB  = CalGain[14];
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
        if (FacR!=0 &&
            FacG!=0 &&
            FacB!=0 )
        {
            pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) /FacR);
            pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) /FacG);
            pCamCalData->Single2A.S2aAwb.rGoldGainu4B = (u32)((tempMax * 512 + (FacB >> 1)) /FacB);
        }
        err = CAM_CAL_ERR_NO_ERR;

      }
      else {
        pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
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
      CAM_CAL_LOG_IF(dumpEnable,"======================AWB CAM_CAL==================\n");
      CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
      CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
      CAM_CAL_LOG_IF(dumpEnable,"[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
      CAM_CAL_LOG_IF(dumpEnable,"[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
      CAM_CAL_LOG_IF(dumpEnable,"[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
      CAM_CAL_LOG_IF(dumpEnable,"[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
      CAM_CAL_LOG_IF(dumpEnable,"======================AWB CAM_CAL==================\n");
      #endif
    }
    if(0x2&AWBAFConfig){
      ////AF////
      ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
              0x0024, 2, (u8 *)&AFInf);
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

      ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
              0x0026, 2, (u8 *)&AFMacro);
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

      pCamCalData->Single2A.S2aAf[0] = AFMacro;
      pCamCalData->Single2A.S2aAf[1] = AFInf;

      ////Only AF Gathering <////
      #ifdef DEBUG_CALIBRATION_LOAD
      CAM_CAL_LOG_IF(dumpEnable,"======================AF CAM_CAL==================\n");
      CAM_CAL_LOG_IF(dumpEnable,"[AFInf] = %d\n", AFInf);
      CAM_CAL_LOG_IF(dumpEnable,"[AFMacro] = %d\n", AFMacro);
      CAM_CAL_LOG_IF(dumpEnable,"======================AF CAM_CAL==================\n");
      #endif
    }

    return err;
}
#endif



/***********************************************************************************

    Function : To read LSC Table

************************************************************************************/
UINT32 DoCamCalSingleLsc(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];
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

        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%lu", start_addr - 2, sizeof(table_size));
        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                start_addr - 2, sizeof(table_size), (u8 *)&table_size);
        if(ioctlerr <= 0)
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
            CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d ", start_addr, table_size);
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    start_addr, table_size, (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType);
            if(table_size == ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR("ioctl err\n");
                err = CamCalReturnErr[pCamCalData->Command];
                ShowCmdErrorLog(pCamCalData->Command);
            }
        }
    }
    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================SingleLsc Data==================\n");
    CAM_CAL_LOG("RETURN = 0x%x, [1st] = %x, %x, %x, %x\n",
            err,
            pCamCalData->SingleLsc.LscTable.Data[0],
            pCamCalData->SingleLsc.LscTable.Data[1],
            pCamCalData->SingleLsc.LscTable.Data[2],
            pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
    #endif
//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data
    return err;
}

/******************************************************************************
This function will add after sensor support FOV data
*******************************************************************************/
UINT32 DoCamCalStereoData(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err=  0;
    char Stereo_Data[1360];

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal_Stereo_Data sensorID = %x\n", pCamCalData->sensorID);
    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)Stereo_Data);
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
    CAM_CAL_LOG("RETURN = 0x%x\n", err);
#endif
    return err;
}

/******************************************************************************
*In order to get data one block by one block instead of overall data in one time
*It must extract FileID and LayoutType from CAM_CALGetCalData()
*******************************************************************************/
UINT32 DoCamCalLayoutCheck(ICamCalHelper *pCamCalHelper, UINT32* pGetSensorCalData)
{
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 header_offset;

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

        //read ID
        header_offset = 0xFFFFFFFF;
        for (i = 0; i< MAX_CALIBRATION_LAYOUT_NUM; i++)
        {
            if (header_offset != CalLayoutTbl[i].HeaderAddr)
            {
                CheckID = 0x00000000;
                header_offset = CalLayoutTbl[i].HeaderAddr;
                CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d sensorID=%x",
                                CalLayoutTbl[i].HeaderAddr, 4, pCamCalData->sensorID);

                err = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        header_offset, 4, (u8 *)&CheckID);
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

    }

    CAM_CAL_LOG_IF(dumpEnable,"result = %x, LayoutType = %u", result, LayoutType);
    return result;
}

/******************************************************************************
* Depredicated entry function
*******************************************************************************/
UINT32 CAM_CALGetCalData(UINT32* pGetSensorCalData)
{
    (void) pGetSensorCalData;
    return 0;
}

UINT32 DoCamCalPDAF(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    INT32 ioctlerr;
    INT32 err =  CamCalReturnErr[pCamCalData->Command];
    {
#ifdef MTK_LOAD_DEBUG
        dumpEnable = 1;
#else
        dumpEnable = 0;
#endif
        pCamCalData->PDAF.Size_of_PDAF = BlockSize;
        CAM_CAL_LOG_IF(dumpEnable,"PDAF start_addr =%x table_size=%d\n",start_addr, BlockSize);

        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                start_addr, BlockSize, (u8 *)&pCamCalData->PDAF.Data[0]);
        if(ioctlerr > 0)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }
    }
#ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================PDAF Data==================\n");
    CAM_CAL_LOG("RETURN = 0x%x, First five %x, %x, %x, %x, %x \n",
            err,
            pCamCalData->PDAF.Data[0],
            pCamCalData->PDAF.Data[1],
            pCamCalData->PDAF.Data[2],
            pCamCalData->PDAF.Data[3],
            pCamCalData->PDAF.Data[4]);
#endif

    return err;

}

/***********************************************************************************

    Function : To read LSC Table

************************************************************************************/
UINT32 DoCamCalSingleLscOP(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];
    u16 table_size = (u16)BlockSize;

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
        if(BlockSize!=CAM_CAL_SINGLE_LSC_SIZE)
        {
            CAM_CAL_LOG("BlockSize(%d) is not match (%d)\n", BlockSize, CAM_CAL_SINGLE_LSC_SIZE);
        }
        pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2;//mtk type
        pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 8;//hardcode.... need to fix

        CAM_CAL_LOG("lsc table_size %d\n", table_size);
        pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
        if(table_size>0)
        {
            pCamCalData->SingleLsc.TableRotation=0;
            CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d ", start_addr, table_size);
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    start_addr, table_size, (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType);
            if(table_size == ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR("ioctl err\n");
                err = CamCalReturnErr[pCamCalData->Command];
                ShowCmdErrorLog(pCamCalData->Command);
            }
        }
    }
    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================SingleLsc Data==================\n");
    CAM_CAL_LOG("RETURN = 0x%x, [1st] = %x, %x, %x, %x\n",
            err,
            pCamCalData->SingleLsc.LscTable.Data[0],
            pCamCalData->SingleLsc.LscTable.Data[1],
            pCamCalData->SingleLsc.LscTable.Data[2],
            pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
         pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
    #endif
//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data
    return err;
}

/***********************************************************************************

   Function : To read 2A infomation. Please put your AWB+AF data funtion, here.

************************************************************************************/

static UINT32 _DoCamCal2AGainOP(ICamCalHelper *pCamCalHelper, UINT32 awb_offset, UINT32 golden_offset,
        UINT32 af_infinity_offset, UINT32 af_macro_offset,
        UINT32* pGetSensorCalData)
{

    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    long long CalGain, FacGain;
    UINT8 AWBAFConfig = 0xf;
    UINT16 AFInf, AFMacro;
    UINT16 tempMax = 0;
    UINT16 CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;
    CAM_CAL_LOG("_DoCamCal2AGainOP is enter.. SensorID=%x\n", pCamCalData->sensorID);

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
        pCamCalData->Single2A.S2aVer = 0x01;
        pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
        pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);

        if(0x1&AWBAFConfig){
            ////AWB////
            CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", awb_offset);
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    awb_offset, 8, (u8 *)&CalGain);

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
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    golden_offset, 8, (u8 *)&FacGain);
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
            CAM_CAL_LOG_IF(dumpEnable, "[CalGain] = %lld, [FacGain] = %lld\n", CalGain, FacGain);
            CAM_CAL_LOG("[UnitGain] R=%d G=%d B=%d\n",
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4R,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4G,
                        pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
            CAM_CAL_LOG("[GoldenGain] R=%d G=%d B=%d\n",
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4R,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4G,
                        pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
            #endif
        }
        if(0x2&AWBAFConfig){
            ////AF////
            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    af_infinity_offset, 2, (u8 *)&AFInf);
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

            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    af_macro_offset, 2, (u8 *)&AFMacro);
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
            #endif
        }

    }
    return err;
}

UINT32 DoCamCal2AGainOP519(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    (void) start_addr;
    (void) BlockSize;

    return _DoCamCal2AGainOP(pCamCalHelper, 0x0c, 0x14, 0x26, 0x24, pGetSensorCalData);
}

UINT32 DoCamCal2AGainOP586(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    (void) start_addr;
    (void) BlockSize;

    return _DoCamCal2AGainOP(pCamCalHelper, 0x10, 0x18, 0x94, 0x92, pGetSensorCalData);
}

UINT32 DoCamCal2AGainOP576(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    (void) start_addr;
    (void) BlockSize;

    return _DoCamCal2AGainOP(pCamCalHelper, 0x0c, 0x14, 0x26, 0x24, pGetSensorCalData);
}

UINT32 DoCamCalPDAFOP586_T0(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize,
	UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    (void) start_addr;
    (void) BlockSize;

    pCamCalData->PDAF.Size_of_PDAF = 496;
    CAM_CAL_LOG("PDAF read proc1 496 byte");

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            0x1E90, 496, (u8 *)&pCamCalData->PDAF.Data[0]);
    if(ioctlerr > 0) {
        CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[0], pCamCalData->PDAF.Data[1]);
        CAM_CAL_LOG("PDAF read proc2 917 byte");
        pCamCalData->PDAF.Size_of_PDAF += 917;

        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                0x2110, 917, (u8 *)&pCamCalData->PDAF.Data[496]);

        if(ioctlerr > 0) {
             CAM_CAL_LOG("PDAF %x %x  all size=%d",
                     pCamCalData->PDAF.Data[496], pCamCalData->PDAF.Data[497],
                     pCamCalData->PDAF.Size_of_PDAF);
             err = CAM_CAL_ERR_NO_ERR;
        }
    }
    return err;
}

UINT32 DoCamCalPDAFOP586_T1(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize,
	UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    (void) start_addr;
    (void) BlockSize;

    pCamCalData->PDAF.Size_of_PDAF = 496;
    CAM_CAL_LOG("PDAF read proc1 496 byte");

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            0x1E90, 496, (u8 *)&pCamCalData->PDAF.Data[0]);
    if(ioctlerr > 0) {
        CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[0], pCamCalData->PDAF.Data[1]);
        CAM_CAL_LOG("PDAF read proc2 886 byte");
        pCamCalData->PDAF.Size_of_PDAF += 886;

        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                0x2110, 886, (u8 *)&pCamCalData->PDAF.Data[496]);

        if(ioctlerr > 0) {
             CAM_CAL_LOG("PDAF %x %x  all size=%d",
                     pCamCalData->PDAF.Data[496], pCamCalData->PDAF.Data[497],
                     pCamCalData->PDAF.Size_of_PDAF);
             err = CAM_CAL_ERR_NO_ERR;
        }
    }
    return err;
}

UINT32 DoCamCalPDAFOP576(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize,
	UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];

    (void) start_addr;
    (void) BlockSize;

    pCamCalData->PDAF.Size_of_PDAF = 496;
    CAM_CAL_LOG("PDAF read proc1 496 byte");

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            0x1400, 496, (u8 *)&pCamCalData->PDAF.Data[0]);
    if(ioctlerr > 0) {
        CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[0], pCamCalData->PDAF.Data[1]);
        CAM_CAL_LOG("PDAF read proc2 806 byte");
        pCamCalData->PDAF.Size_of_PDAF += 806;

        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                0x1600, 806, (u8 *)&pCamCalData->PDAF.Data[496]);

        if(ioctlerr > 0) {
             CAM_CAL_LOG("PDAF %x %x", pCamCalData->PDAF.Data[496], pCamCalData->PDAF.Data[497]);
             CAM_CAL_LOG("PDAF read proc3 102 byte");
             pCamCalData->PDAF.Size_of_PDAF += 102;

             ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                     0x1A00, 102, (u8 *)&pCamCalData->PDAF.Data[1302]);

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

UINT32 DoCamCalLensId_Base(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];
    UINT32 sizeLimit = sizeof(pCamCalData->LensDrvId);

    memset(&pCamCalData->LensDrvId[0], 0, sizeLimit);

    if (BlockSize > sizeLimit) {
        CAM_CAL_ERR("lens id size can't larger than %u\n", sizeLimit);
        return err;
    }

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)&pCamCalData->LensDrvId[0]);
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
    CAM_CAL_LOG("======================Lens Id==================\n");
    CAM_CAL_LOG("[Lens Id] = %x %x %x %x %x %x %x %x %x %x\n",
            pCamCalData->LensDrvId[0], pCamCalData->LensDrvId[1],
            pCamCalData->LensDrvId[2], pCamCalData->LensDrvId[3],
            pCamCalData->LensDrvId[4], pCamCalData->LensDrvId[5],
            pCamCalData->LensDrvId[6], pCamCalData->LensDrvId[7],
            pCamCalData->LensDrvId[8], pCamCalData->LensDrvId[9]);
#endif
    return err;
}

UINT32 DoCamCalLensIdOP(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    return DoCamCalLensId_Base(pCamCalHelper, start_addr, BlockSize, pGetSensorCalData);
}

UINT32 DoCamCalLensId(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];

    if (getMtkFormatVersion(pCamCalHelper, pGetSensorCalData) != 0x18) {
        CAM_CAL_ERR("No lens id data\n");
        return err;
    }

    return DoCamCalLensId_Base(pCamCalHelper, start_addr, BlockSize, pGetSensorCalData);
}

UINT32 DoCamCal_Dump(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{/* dump all EEPROM info */
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    char info[8192];
    INT32 ioctlerr;
    UINT32 err =  CAM_CAL_ERR_NO_DEVICE, idx;

    if(BlockSize > 8192) return err;

    CAM_CAL_LOG("To dump all EEPROM info.. SensorID=%x\n", pCamCalData->sensorID);

    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)info);
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

UINT32 DoCamCal_Dump_All(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    char *info;
    INT32 ioctlerr;
    UINT32 err =  CAM_CAL_ERR_NO_DEVICE, idx;
    UINT32 dumpSize = BlockSize;

    // specify dump size if property set
    int debugDumpSize = property_get_int32("vendor.debug.eeprom.dumpsize", 0);
    if (debugDumpSize > 0) {
        dumpSize = (UINT32) debugDumpSize;
    }

    // create folder
    ioctlerr = mkdir("/sdcard/EEPROM", S_IRWXU | S_IRWXG | S_IXOTH);
    CAM_CAL_LOG( "create folder /sdcard/EEPROM (%d)", ioctlerr);

    // open file
    char targetFile[50];
    snprintf(targetFile, sizeof(targetFile), "/sdcard/EEPROM/SensorDev%d", pCamCalData->deviceID);
    FILE *fp = fopen(targetFile, "w");

    if(fp == NULL) {
        CAM_CAL_LOG("open file fail!");
        return err;
    }

    // get data
    info = new char[dumpSize];
    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, dumpSize, (u8 *)info);
    if (ioctlerr > 0) {
        fprintf(fp, "SensorID=0x%x\n", pCamCalData->sensorID);

        for(idx = 0 ; idx < dumpSize; idx++) {
            fprintf(fp, "0x%04x,0x%02x\n", idx, info[idx]);
        }
        err = CAM_CAL_ERR_NO_ERR;
    } else {
        CAM_CAL_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
        err = CAM_CAL_ERR_DUMP_FAILED;
    }

    // release resource
    delete info;
    fclose(fp);

    return err;
}

CamCalCustomImpl::CamCalCustomImpl(ICamCalHelper *pCamCalHelper)
    : ICamCalCustom(pCamCalHelper)
{
}

unsigned int
CamCalCustomImpl::getCalData(unsigned int* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    unsigned int result = CAM_CAL_ERR_NO_DEVICE;

    CAM_CAL_LOG("pCamCalData->deviceID=%d\n", pCamCalData->deviceID);


    pthread_mutex_lock(&mEEPROM_Mutex);

    CAMERA_CAM_CAL_TYPE_ENUM lsCommand = pCamCalData->Command;

    if(DoCamCalLayoutCheck(mpCamCalHelper, pGetSensorCalData)==CAM_CAL_ERR_NO_ERR)
    {
        pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[LayoutType].DataVer;

        if ((CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].Include != 0)
            &&(CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL))
        {
            result = CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
                mpCamCalHelper,
                CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].StartAddr,
                CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].BlockSize,
                pGetSensorCalData);
        }
        else
        {
            result = CamCalReturnErr[lsCommand];
            ShowCmdErrorLog(lsCommand);
        }
    }

    pthread_mutex_unlock(&mEEPROM_Mutex);
    return result;
}

/**
 * Please override this function when
 * the EEPROM data is stored at another sensor dev
 */
unsigned int
CamCalCustomImpl::getEepromSensorDev(unsigned int const sensorDev, unsigned int sensorId)
{
    (void) sensorId;

    /* Implementation example. The cal data of Main2 S5K3M3 is store at main cam's EEPROM.
     * if (sensorDev == 0x4 && sensorId == S5K3M3_SENSOR_ID)
     *     return 0x1;
     * else
     *     return sensorDev;
     */

    return sensorDev;
}

/**
 * Configuration of CamCalDrv
 */
struct CamCalDrvConfig ICamCalCustom::sDrvConfig = {
    .enableAutoPowerOn = true,
    .enablePreload = true,
    .preloadSizeLimit = 0x1500,
};

ICamCalCustom*
ICamCalCustom::createInstance(ICamCalHelper *pCamCalHelper)
{
    if (pCamCalHelper == NULL)
        return NULL;
    return new CamCalCustomImpl(pCamCalHelper);
}

