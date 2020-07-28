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
#include <unistd.h>
#include <log/log.h> //#include <utils/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>

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

#include "kd_imgsensor.h"
#include "kd_camera_feature.h"

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
#define CAM_CAL_VER "ver0723" //2018.07.22

#ifdef CAM_CAL_SHOW_LOG
//#define CAM_CAL_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#define CAM_CAL_LOG(fmt, arg...)    ALOGI(CAM_CAL_VER " " fmt, ##arg)
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

#define SUPPORT_FOR_BAYER_BAYER_FORMAT 1
#if SUPPORT_FOR_BAYER_BAYER_FORMAT
static UINT32 DoCamCal2AGain_2P7(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
static UINT32 DoCamCalPDAF_2P7(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);

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
	CALIBRATION_LAYOUT_4CELL,
	CALIBRATION_LAYOUT_EXT1,
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
    {//CALIBRATION_LAYOUT_STEREO_MAIN2
        0x00001001, 0x030b00ff, CAM_CAL_DOUBLE_EEPROM_DATA,
        {
            {0x00000001, 0x00001000, 0x00001000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000000, 0x00001005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00001017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00001007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000001, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
    {// 4 CELL SENSOR
        0x00000001, 0x050b00ff, CAM_CAL_SINGLE_OTP_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
            {0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
            {0x00000001, 0x00000017, 0x0000074C, DoCamCalSingleLsc}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
            {0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain}, //CAMERA_CAM_CAL_DATA_3A_GAIN
            {0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF},
            {0x00000000, 0x00000FAE, 0x00000550, DoCamCalStereoData},  //CAMERA_CAM_CAL_DATA_STEREO_DATA
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    },
#ifdef SUPPORT_FOR_BAYER_BAYER_FORMAT
    {//refer spec
        0x00000006, 0x0062001a, CAM_CAL_SINGLE_EEPROM_DATA,
        {
            {0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion},
            {0x00000000, 0x00000005, 0x00000002, DoCamCalPartNumber},
            {0x00000001, 0x00000043, 0x0000074C, DoCamCalSingleLsc},//Check by spec, only modify size(LH)
            {0x00000001, 0x00000030, 0x0000000E, DoCamCal2AGain_2P7},//Check by spec
            {0x00000001, 0x00000900, 0x00000587, DoCamCalPDAF_2P7},//Check by spec
            {0x00000000, 0x00000000, 0x00000000, DoCamCalStereoData},
            {0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All},
            {0x00000000, 0x00000F80, 0x0000000A, DoCamCalLensId}
        }
    }
#endif

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
    CAM_CAL_LOG("[Part Number] = %x %x %x %x\n",
            pCamCalData->PartNumber[0], pCamCalData->PartNumber[1],
            pCamCalData->PartNumber[2], pCamCalData->PartNumber[3]);
    CAM_CAL_LOG("[Part Number] = %x %x %x %x\n",
            pCamCalData->PartNumber[4], pCamCalData->PartNumber[5],
            pCamCalData->PartNumber[6], pCamCalData->PartNumber[7]);
    CAM_CAL_LOG("[Part Number] = %x %x %x %x\n",
            pCamCalData->PartNumber[8], pCamCalData->PartNumber[9],
            pCamCalData->PartNumber[10], pCamCalData->PartNumber[11]);
    CAM_CAL_LOG("======================Part Number==================\n");
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
                CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
                CAM_CAL_LOG("[AF] Inf=%d Marco=%d\n", AFInf, AFMacro);
                CAM_CAL_LOG_IF(dumpEnable, "======================AF CAM_CAL==================\n");
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

        if(pCamCalData->sensorID == 0x0386 || pCamCalData->sensorID == 0x2107 || pCamCalData->sensorID == 0x3108)
        {
                table_size = 1868;
        }
        CAM_CAL_LOG("lsc table_size %d\n", table_size);
        pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
        if(table_size>0)
        {
            pCamCalData->SingleLsc.TableRotation=CUSTOM_CAM_CAL_ROTATION_00;
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
UINT32 DoCamCalStereoData(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = 0;

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal_Stereo_Data sensorID = %x\n", pCamCalData->sensorID);
    pCamCalData->Stereo_Data.Size_of_Stereo_Data = CAM_CAL_Stereo_Data_SIZE;
    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)&pCamCalData->Stereo_Data.Data);
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
                if(err <= 0)
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
//===================it's have to put on last!!20170719=======================
#ifdef SUPPORT_FOR_BAYER_BAYER_FORMAT
            if(pCamCalData->sensorID == 0x2107 && pCamCalData->deviceID == 1)
            {
                CAM_CAL_LOG("S5K2P7 EEPROM Programming Guide 0719\n");
                LayoutType = 6;
                gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_PASS;
                result = CAM_CAL_ERR_NO_ERR;
                break;
            }
#endif
//============================================================================
        }
        CAM_CAL_LOG_IF(dumpEnable,"LayoutType= 0x%x",LayoutType);
        CAM_CAL_LOG_IF(dumpEnable,"result= 0x%x",result);

    }

    CAM_CAL_LOG("result = %x, LayoutType = %u", result, LayoutType);
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


/***********************************************************************************
 Function : To read PDAF infomation.
************************************************************************************/
UINT32 DoCamCalPDAF(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];
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
    CAM_CAL_LOG("[Lens Id] = %x %x %x %x %x\n",
            pCamCalData->LensDrvId[0], pCamCalData->LensDrvId[1],
            pCamCalData->LensDrvId[2], pCamCalData->LensDrvId[3],
            pCamCalData->LensDrvId[4]);
    CAM_CAL_LOG("[Lens Id] = %x %x %x %x %x\n",
            pCamCalData->LensDrvId[5], pCamCalData->LensDrvId[6],
            pCamCalData->LensDrvId[7], pCamCalData->LensDrvId[8],
            pCamCalData->LensDrvId[9]);
    CAM_CAL_LOG("======================Lens Id==================\n");
#endif
    return err;
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


#if SUPPORT_FOR_BAYER_BAYER_FORMAT
/***********************************************************************************

 Function : To read 2A infomation. This function is special case.

************************************************************************************/
static UINT32 DoCamCal2AGain_2P7(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];

    UINT8 AWB_Flag = 0x0;
    UINT8 AF_Flag = 0;
    u8 AFInf[2], AFMacro[2];
    u8 rgbGain[12];
    u16 rGain = 0,bGain = 0,gGain = 0;
    u16 GOLDEN_rGain = 0,GOLDEN_bGain = 0,GOLDEN_gGain = 0;

#define GOLDEN_RGAIN  293 // R/Gr
#define GOLDEN_GGAIN  512 // B/Gr
#define GOLDEN_BGAIN  349 // Gr/Gb

    CAM_CAL_LOG_IF(dumpEnable,"DoCamCal2AGain_2P7 is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
         err = CAM_CAL_ERR_NO_DEVICE;
         CAM_CAL_ERR("878 ioctl err\n");
         ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    {
        /*Check the config. for AWB*/
        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                start_addr, 1, (u8 *)&AWB_Flag);
        if(ioctlerr>0)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }
        else
        {
             pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
             CAM_CAL_ERR("898 ioctl err\n");
             ShowCmdErrorLog(pCamCalData->Command);
        }

        pCamCalData->Single2A.S2aVer = 0x01;
        pCamCalData->Single2A.S2aBitEn = CAM_CAL_AWB_BITEN;

        if(0x1 == AWB_Flag){
             ////AWB////
             CAM_CAL_LOG_IF(dumpEnable,"AWB offset=%d\n", start_addr + 1);
             ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                     start_addr + 1, 12, (u8 *)&rgbGain[0]);
             CAM_CAL_LOG_IF(dumpEnable,"Read CalGain OK %x\n",ioctlerr);

             if(ioctlerr>0)
             {
                 err = CAM_CAL_ERR_NO_ERR;
                 rGain = ((rgbGain[0]<<8) | rgbGain[1]);
                 bGain = ((rgbGain[2]<<8) | rgbGain[3]);
                 gGain = ((rgbGain[4]<<8) | rgbGain[5]);
                 CAM_CAL_LOG_IF(dumpEnable,"[R/Gr] = %d\n", rGain);
                 CAM_CAL_LOG_IF(dumpEnable,"[B/Gr] = %d\n", bGain);
                 CAM_CAL_LOG_IF(dumpEnable,"[Gb/Gr] = %d\n", gGain);

                 GOLDEN_rGain = ((rgbGain[6]<<8) | rgbGain[7]);
                 GOLDEN_bGain = ((rgbGain[8]<<8) | rgbGain[9]);
                 GOLDEN_gGain = ((rgbGain[10]<<8) | rgbGain[11]);

                 CAM_CAL_LOG_IF(dumpEnable,"GOLDEN_rGain[R/Gr] = %d\n", GOLDEN_rGain);
                 CAM_CAL_LOG_IF(dumpEnable,"GOLDEN_bGain[B/Gr] = %d\n", GOLDEN_bGain);
                 CAM_CAL_LOG_IF(dumpEnable,"GOLDEN_gGain[Gb/Gr] = %d\n", GOLDEN_gGain);

                 if((GOLDEN_rGain == 0) ||(GOLDEN_gGain == 0) ||(GOLDEN_bGain == 0)){
                     GOLDEN_rGain = GOLDEN_RGAIN;
                     GOLDEN_gGain = GOLDEN_GGAIN;
                     GOLDEN_bGain = GOLDEN_BGAIN;
                 }

                 pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32) ((512.0 / rGain) *512);
                 pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (u32) ((512.0 / bGain) *512);
                 pCamCalData->Single2A.S2aAwb.rUnitGainu4G = GOLDEN_GGAIN;//(u32) ((512.0 / gGain) *512); Comment by MTK's AWB owner

                 pCamCalData->Single2A.S2aAwb.rGoldGainu4R= (u32) ((512.0/GOLDEN_rGain)*512);
                 pCamCalData->Single2A.S2aAwb.rGoldGainu4B= (u32) ((512.0/GOLDEN_bGain)*512);
                 pCamCalData->Single2A.S2aAwb.rGoldGainu4G= (u32) ((512.0/GOLDEN_gGain)*512);

                 #ifdef DEBUG_CALIBRATION_LOAD
                    CAM_CAL_LOG("======================AWB CAM_CAL 2P7==================\n");
                    CAM_CAL_LOG("[rCalGainUnit.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
                    CAM_CAL_LOG("[rCalGainUnit.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
                    CAM_CAL_LOG("[rCalGainUnit.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
                    CAM_CAL_LOG("[rCalGainGold.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
                    CAM_CAL_LOG("[rCalGainGold.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                    CAM_CAL_LOG("[rCalGainGold.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
                    CAM_CAL_LOG("======================AWB CAM_CAL 2P7==================\n");
                 #endif
                 }
                 else
                 {
                     pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                     CAM_CAL_ERR("ioctl err\n");
                     ShowCmdErrorLog(pCamCalData->Command);
                 }
            }

            ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                    0x0020, 1, (u8 *)&AF_Flag);

            if(0x1 == AF_Flag){
                ////AF////
                pCamCalData->Single2A.S2aAfBitflagEn = CAM_CAL_AF_BITEN;/*Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7*/
                CAM_CAL_LOG_IF(dumpEnable,"S2aAfBitflagEn=0x%x", pCamCalData->Single2A.S2aAfBitflagEn);

                ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                        0x0022, 2, (u8 *)&AFInf[0]);
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
                        0x0024, 2, (u8 *)&AFMacro[0]);
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


                pCamCalData->Single2A.S2aAf[0] = ((AFInf[0]<<8)|AFInf[1]);
                pCamCalData->Single2A.S2aAf[1] = ((AFMacro[0]<<8)|AFMacro[1]);

                ////Only AWB Gain Gathering <////
            #ifdef DEBUG_CALIBRATION_LOAD
               CAM_CAL_LOG("======================AF CAM_CAL 2P7==================\n");
               CAM_CAL_LOG("[AFInf] = %x\n", pCamCalData->Single2A.S2aAf[0]);
               CAM_CAL_LOG("[AFMacro] = %x\n", pCamCalData->Single2A.S2aAf[1]);
               CAM_CAL_LOG("======================AF CAM_CAL 2P7==================\n");
            #endif
         }
     ////////////////////////////////////////////////////////////////////////////////
     }
     return err;
}


static UINT32 DoCamCalPDAF_2P7(ICamCalHelper *pCamCalHelper, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CAM_CAL_ERR_NO_ERR;//CamCalReturnErr[pCamCalData->Command];
    u8 pdaf_valid;

    (void) BlockSize;

    CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", start_addr + 4, 1);
    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr + 4, 1, (u8 *)&pdaf_valid);
    if(!ioctlerr)
    {
         err = CAM_CAL_ERR_NO_PDAF;
    }

    if(pdaf_valid == 1)
    {
        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", start_addr + 7, 496);
        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                start_addr + 7, 496, (u8 *)&pCamCalData->PDAF.Data[0]);
        if(!ioctlerr)
        {
            err = CAM_CAL_ERR_NO_PDAF;
        }
    }

    CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", start_addr + 0x1F8, 1);
    ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr + 0x1F8, 1, (u8 *)&pdaf_valid);
    if(!ioctlerr)
    {
        err = CAM_CAL_ERR_NO_PDAF;
    }

    if(pdaf_valid == 1)
    {
        CAM_CAL_LOG_IF(dumpEnable,"u4Offset=%d u4Length=%d", start_addr + 0x1FB, 908);
        ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
                start_addr + 0x1FB, 908, (u8 *)&pCamCalData->PDAF.Data[496]);
        if(!ioctlerr)
        {
            err = CAM_CAL_ERR_NO_PDAF;
        }
    }


    pCamCalData->PDAF.Size_of_PDAF = 0x57C;
    #ifdef DEBUG_CALIBRATION_LOAD
    CAM_CAL_LOG("======================PDAF Data 2P7==================\n");
    CAM_CAL_LOG("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
                    pCamCalData->PDAF.Data[1],pCamCalData->PDAF.Data[2],pCamCalData->PDAF.Data[3],
                    pCamCalData->PDAF.Data[4]);
    CAM_CAL_LOG("RETURN = 0x%x \n", err);
    CAM_CAL_LOG("======================PDAF Data 2P7==================\n");
    #endif
    return err;

}

#endif


CamCalCustomImpl::CamCalCustomImpl(ICamCalHelper *pCamCalHelper)
    : ICamCalCustom(pCamCalHelper)
{
}

bool
CamCalCustomImpl::hasCalData(unsigned int const sensorDev, unsigned int sensorId)
{
    if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR && sensorId == OV8856_SENSOR_ID) {
        CAM_CAL_LOG("No cal data for sensor Id 0x%x\n", sensorId);
        return false;
    }
    return true;
}

unsigned int
CamCalCustomImpl::getCalData(unsigned int* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    unsigned int result = CAM_CAL_ERR_NO_DEVICE;

    CAM_CAL_LOG("pCamCalData->deviceID=%d\n", pCamCalData->deviceID);


    pthread_mutex_lock(&mEEPROM_Mutex);

    CAMERA_CAM_CAL_TYPE_ENUM lsCommand = pCamCalData->Command;

    if(hasCalData(pCamCalData->deviceID, pCamCalData->sensorID) &&
        DoCamCalLayoutCheck(mpCamCalHelper, pGetSensorCalData) == CAM_CAL_ERR_NO_ERR)
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
    .enableAutoPowerOn = false,
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

