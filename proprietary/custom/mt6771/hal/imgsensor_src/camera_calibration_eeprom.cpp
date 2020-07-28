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

#include <cutils/log.h> //#include <utils/Log.h>
#include <fcntl.h>//define O_RDWR
#include <string.h> //memcpy
#include <bits/ioctl.h>
#include <unistd.h>

#include <mtkcam/drv/mem/cam_cal_drv.h>

#include "camera_custom_nvram.h"

//cam_cal
#include "cam_cal.h"
#include "cam_cal_define.h"

extern "C"{
#include "camera_custom_cam_cal.h"
}

#include "camera_calibration_cam_cal.h"	//get function pointer of original flow
#include "camera_calibration_eeprom.h"


#define CAM_CAL_SHOW_LOG 1
#define CAM_CAL_VER "camcal_ver9901~"	//2017.10.23

#ifdef CAM_CAL_SHOW_LOG
//#define CAM_CAL_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#define CAM_CAL_LOG(fmt, arg...)    ALOGI(CAM_CAL_VER " " fmt, ##arg)
#define CAM_CAL_ERR(fmt, arg...)    ALOGE(CAM_CAL_VER "Err: %5d: " fmt, __LINE__, ##arg)
#else
#define CAM_CAL_LOG(fmt, arg...)    void(0)
#define CAM_CAL_ERR(fmt, arg...)    void(0)
#endif


#define MAX_BUFFER_SIZE_OF_ONE_EEPROM_TABLE 6208
#define LENGTH_OF_LSC  CAM_CAL_SINGLE_LSC_SIZE	//defined on cam_cal_drv.h

typedef struct
{
    UINT8  S2aVer; //verson : 01
    UINT8  S2aBitEn; //bit enable: 03 Bit0: AF Bit1: WB
    UINT8  WB_R;
    UINT8  WB_Gr;
    UINT8  WB_Gb;
    UINT8  WB_B;
    UINT8  WB_Golden_R;
    UINT8  WB_Golden_Gr;
    UINT8  WB_Golden_Gb;
    UINT8  WB_Golden_B;
    UINT16 AF_infinite;
    UINT16 AF_macro;
}CAMERA_CALIBRATION_2A_STRUCT;

typedef struct
{
    UINT16 LSC_Length;
	UINT8  LSC_Table[LENGTH_OF_LSC];
}CAMERA_CALIBRATION_LSC_STRUCT;

typedef struct
{
	UINT8  PDAF_Table[CAM_CAL_PDAF_SIZE];//CAM_CAL_PDAF_SIZE is defined on cam_cal_drv.h
    UINT16 PDAF_Length;
}CAMERA_CALIBRATION_PDAF_STRUCT;


typedef struct
{
    UINT8   flag;
    UINT32  calibration_version;
    UINT16  serial_number;
    CAMERA_CALIBRATION_2A_STRUCT    INFO_2A;
    CAMERA_CALIBRATION_LSC_STRUCT   INFO_LSC;
    CAMERA_CALIBRATION_PDAF_STRUCT  PDAF;
    CAM_CAL_AF_STRUCT               INFO_AF;
}CAMERA_CALIBRATION_BUFFER;


//============== define sensor index ==============
enum
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP  = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
    SENSOR_DEV_SUB_2 = 0x08,
    SENSOR_DEV_MAIN_3 = 0x10,
};

enum
{
    IDX_MAIN_CAM	= 0x00,
    IDX_SUB_CAM		= 0x01,
    IDX_MAIN2_CAM	= 0x02,
    IDX_SUB2_CAM	= 0x03,
    IDX_MAIN3_CAM	= 0x04,
    IDX_MAX_CAM_NUMBER,
};

static CAMERA_CALIBRATION_BUFFER cam_cal_buffer_list[IDX_MAX_CAM_NUMBER];

//=================================================

static UINT32 Is_Read_To_Buffer = 0;	//it will |= deviceID 1,2,4,8,16



typedef struct
{
	UINT32 HeaderAddr; //Header Address
	UINT32 DeviceId;   //Device ID

	UINT16 StartAddr_SN;
	UINT16 BlockSize_SN;

	UINT16 StartAddr_AWB_AF;
	UINT16 BlockSize_AWB_AF;

	UINT16 StartAddr_LSC_TABLE;
	UINT16 BlockSize_LSC_TABLE;

	UINT16 StartAddr_PDAF_TABLE;
	UINT16 BlockSize_PDAF_TABLE;

	UINT16 StartAddr_AF_INFO_TABLE;
	UINT16 BlockSize_AF_INFO_TABLE;
} CAMERA_CALIBRATION_LAYOUT_STRUCT;
#define MAX_OF_CAM_CAL_LAYER 4
CAMERA_CALIBRATION_LAYOUT_STRUCT cam_cal_layer[MAX_OF_CAM_CAL_LAYER]=
{
	{0x010b00ff,0, 5,2, 7,14, 0x15,1868, 0x763,CAM_CAL_PDAF_SIZE, 0xf63,64},
	{0x020b00ff,0, 5,2, 7,14, 0x15,1868, 0x763,CAM_CAL_PDAF_SIZE, 0xf63,64},
	{0x040b00ff,0, 5,2, 7,14, 0x15,1868, 0x763,192, 0x823,64},
	{0x040b00ff,1, 5,2, 7,14, 0x15,1868, 0x763,CAM_CAL_PDAF_SIZE, 0xf63,64},
};



static int EEPROM_Fill_Table_From_IOCTL(INT32 CamcamFID, UINT32 idx_camera, UINT32 start_addr, UINT32* pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    /*********************1.ioctrl************************/
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    UINT8 info[MAX_BUFFER_SIZE_OF_ONE_EEPROM_TABLE];
    UINT32 ioctlerr;
    UINT32 idx = 0;

    cam_calCfg.u4Offset = (start_addr);
    cam_calCfg.u4Length = (MAX_BUFFER_SIZE_OF_ONE_EEPROM_TABLE);
    cam_calCfg.pu1Params = (UINT8 *)info ;
    cam_calCfg.sensorID = pCamCalData->sensorID;
    cam_calCfg.deviceID = pCamCalData->deviceID;
	CAM_CAL_LOG("EEPROM_Fill_Table_From_IOCTL... SensorID=%x DeviceID=%x and idx_camera=%d\n",
		pCamCalData->sensorID, pCamCalData->deviceID, idx_camera);
    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
    if(ioctlerr <= 0)
    {
        CAM_CAL_LOG("ioctl error!!!!!\n");
        return 0;
    }
    else {
        /*********************2.fill table************************/
        int StartAddr_SN = cam_cal_layer[0].StartAddr_SN;
        int BlockSize_SN = cam_cal_layer[0].BlockSize_SN;
        int StartAddr_AWB_AF = cam_cal_layer[0].StartAddr_AWB_AF;
        int BlockSize_AWB_AF = cam_cal_layer[0].BlockSize_AWB_AF;
        int StartAddr_LSC_TABLE = cam_cal_layer[0].StartAddr_LSC_TABLE;
        int BlockSize_LSC_TABLE = cam_cal_layer[0].BlockSize_LSC_TABLE;
        int StartAddr_PDAF_TABLE = cam_cal_layer[0].StartAddr_PDAF_TABLE;
        int BlockSize_PDAF_TABLE = cam_cal_layer[0].BlockSize_PDAF_TABLE;
        int StartAddr_AF_TABLE = cam_cal_layer[0].StartAddr_AF_INFO_TABLE;
        int BlockSize_AF_TABLE = cam_cal_layer[0].BlockSize_AF_INFO_TABLE;

        cam_cal_buffer_list[idx_camera].flag = info[0];

        memcpy((void*)&cam_cal_buffer_list[idx_camera].calibration_version,
               (const void*)(info + 1),
               sizeof(UINT32));
        CAM_CAL_LOG("eeprom calibration_version = %x\n", cam_cal_buffer_list[idx_camera].calibration_version);

        for(idx = 0; idx < MAX_OF_CAM_CAL_LAYER; idx++) {
            if(cam_cal_layer[idx].HeaderAddr == cam_cal_buffer_list[idx_camera].calibration_version) {
				if(cam_cal_layer[idx].DeviceId == idx_camera) {
					StartAddr_SN = cam_cal_layer[idx].StartAddr_SN;
					BlockSize_SN = cam_cal_layer[idx].BlockSize_SN;
					StartAddr_AWB_AF = cam_cal_layer[idx].StartAddr_AWB_AF;
					BlockSize_AWB_AF = cam_cal_layer[idx].BlockSize_AWB_AF;
					StartAddr_LSC_TABLE = cam_cal_layer[idx].StartAddr_LSC_TABLE;
					BlockSize_LSC_TABLE = cam_cal_layer[idx].BlockSize_LSC_TABLE;
					StartAddr_PDAF_TABLE = cam_cal_layer[idx].StartAddr_PDAF_TABLE;
					BlockSize_PDAF_TABLE = cam_cal_layer[idx].BlockSize_PDAF_TABLE;
					StartAddr_AF_TABLE = cam_cal_layer[idx].StartAddr_AF_INFO_TABLE;
					BlockSize_AF_TABLE = cam_cal_layer[idx].BlockSize_AF_INFO_TABLE;

					CAM_CAL_LOG("To use calibration table! idx = %x   length of LSC=%x\n", idx, BlockSize_LSC_TABLE);
				}
            }
        }
        memcpy((void*)&cam_cal_buffer_list[idx_camera].serial_number,
               (const void*)(info + StartAddr_SN),
               sizeof(UINT16));
        CAM_CAL_LOG("eeprom serial_number = %x\n", cam_cal_buffer_list[idx_camera].serial_number);

        memcpy((void*)&cam_cal_buffer_list[idx_camera].INFO_2A,
               (const void*)(info + StartAddr_AWB_AF),
               sizeof(CAMERA_CALIBRATION_2A_STRUCT));

        CAM_CAL_LOG("eeprom R=%d Gr=%d Gb=%d B=%d\n",
					cam_cal_buffer_list[idx_camera].INFO_2A.WB_R,
                    cam_cal_buffer_list[idx_camera].INFO_2A.WB_Gr,
                    cam_cal_buffer_list[idx_camera].INFO_2A.WB_Gb,
					cam_cal_buffer_list[idx_camera].INFO_2A.WB_B);
        CAM_CAL_LOG("eeprom [AFInf] = %d   [AFMacro] = %d \n",
					cam_cal_buffer_list[idx_camera].INFO_2A.AF_infinite,
					cam_cal_buffer_list[idx_camera].INFO_2A.AF_macro);

        memcpy((void*)&cam_cal_buffer_list[idx_camera].INFO_LSC,
                (const void*)(info + StartAddr_LSC_TABLE),
                sizeof(CAMERA_CALIBRATION_LSC_STRUCT));
        CAM_CAL_LOG("LSC Length = %d LSC[0]=%d \n",
					cam_cal_buffer_list[idx_camera].INFO_LSC.LSC_Length,
					cam_cal_buffer_list[idx_camera].INFO_LSC.LSC_Table[0]);

        memcpy((void*)cam_cal_buffer_list[idx_camera].PDAF.PDAF_Table,
               (const void*)(info + StartAddr_PDAF_TABLE),
               BlockSize_PDAF_TABLE);
        cam_cal_buffer_list[idx_camera].PDAF.PDAF_Length = BlockSize_PDAF_TABLE;
        CAM_CAL_LOG("PDAF size = %d PDAF[0]=%d \n",
					cam_cal_buffer_list[idx_camera].PDAF.PDAF_Length,
					cam_cal_buffer_list[idx_camera].PDAF.PDAF_Table[0]);

        CAM_CAL_LOG("Offset of AF info is 0x%x \n", StartAddr_AF_TABLE);
        if(StartAddr_AF_TABLE > 0) {
            unsigned char *AF_INFO = info + StartAddr_AF_TABLE;
            /*for(idx=0;idx<64;idx++) {
				CAM_CAL_LOG("AF[%d]=%d \n", idx, AF_INFO[idx]);
            }*/
            cam_cal_buffer_list[idx_camera].INFO_AF.Close_Loop_AF_Min_Position = (AF_INFO[0] | (AF_INFO[1]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.Close_Loop_AF_Max_Position = (AF_INFO[2] | (AF_INFO[3]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.Close_Loop_AF_Hall_AMP_Offset = AF_INFO[4];
            cam_cal_buffer_list[idx_camera].INFO_AF.Close_Loop_AF_Hall_AMP_Gain = AF_INFO[5];
            cam_cal_buffer_list[idx_camera].INFO_AF.AF_infinite_pattern_distance = (AF_INFO[6] | (AF_INFO[7]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.AF_Macro_pattern_distance = (AF_INFO[8] | (AF_INFO[9]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.AF_infinite_calibration_temperature = (AF_INFO[10]);
            cam_cal_buffer_list[idx_camera].INFO_AF.AF_macro_calibration_temperature = (AF_INFO[11]);

            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_infinite_calibration_temperature = (AF_INFO[16]);
            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_macro_calibration_temperature = (AF_INFO[17]);

            cam_cal_buffer_list[idx_camera].INFO_AF.AF_Middle_calibration = (AF_INFO[18] | (AF_INFO[19]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.AF_Middle_calibration_temperature = (AF_INFO[20]);
            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_Middle_calibration = (AF_INFO[21] | (AF_INFO[22]<<8));
            cam_cal_buffer_list[idx_camera].INFO_AF.Warm_AF_Middle_calibration_temperature = (AF_INFO[23]);
		}


        if(cam_cal_buffer_list[idx_camera].calibration_version == 0x040b00ff) {
			CAM_CAL_LOG("To fill main2 directly\n");
			int idx_camera2 = IDX_MAIN2_CAM;
			int StartAddr_AWB_AF = 2192;
			int StartAddr_LSC_TABLE = 2206;
			int StartAddr_PDAF_TABLE = 4076;
			int BlockSize_PDAF_TABLE = cam_cal_layer[0].BlockSize_PDAF_TABLE;
			int StartAddr_AF_TABLE = 6124;

			memcpy((void*)&cam_cal_buffer_list[idx_camera2].INFO_2A,
				   (const void*)(info + StartAddr_AWB_AF),
				   sizeof(CAMERA_CALIBRATION_2A_STRUCT));

			CAM_CAL_LOG("eeprom2 R=%d Gr=%d Gb=%d B=%d\n",
						cam_cal_buffer_list[idx_camera2].INFO_2A.WB_R,
						cam_cal_buffer_list[idx_camera2].INFO_2A.WB_Gr,
						cam_cal_buffer_list[idx_camera2].INFO_2A.WB_Gb,
						cam_cal_buffer_list[idx_camera2].INFO_2A.WB_B);
			CAM_CAL_LOG("eeprom2 [AFInf] = %d   [AFMacro] = %d \n",
						cam_cal_buffer_list[idx_camera2].INFO_2A.AF_infinite,
						cam_cal_buffer_list[idx_camera2].INFO_2A.AF_macro);
            memcpy((void*)&cam_cal_buffer_list[idx_camera2].INFO_LSC,
					(const void*)(info + StartAddr_LSC_TABLE),
					sizeof(CAMERA_CALIBRATION_LSC_STRUCT));
			CAM_CAL_LOG("LSC2 Length = %d LSC[0]=%d \n",
						cam_cal_buffer_list[idx_camera2].INFO_LSC.LSC_Length,
						cam_cal_buffer_list[idx_camera2].INFO_LSC.LSC_Table[0]);
			memcpy((void*)cam_cal_buffer_list[idx_camera2].PDAF.PDAF_Table,
				   (const void*)(info + StartAddr_PDAF_TABLE),
				   BlockSize_PDAF_TABLE);
			cam_cal_buffer_list[idx_camera2].PDAF.PDAF_Length = BlockSize_PDAF_TABLE;
			CAM_CAL_LOG("PDAF2 size = %d PDAF[0]=%d \n",
						cam_cal_buffer_list[idx_camera2].PDAF.PDAF_Length,
						cam_cal_buffer_list[idx_camera2].PDAF.PDAF_Table[0]);

			CAM_CAL_LOG("Offset2 of AF info is 0x%x \n", StartAddr_AF_TABLE);
			if(StartAddr_AF_TABLE > 0) {
				unsigned char *AF_INFO = info + StartAddr_AF_TABLE;

				cam_cal_buffer_list[idx_camera2].INFO_AF.Close_Loop_AF_Min_Position = (AF_INFO[0] | (AF_INFO[1]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.Close_Loop_AF_Max_Position = (AF_INFO[2] | (AF_INFO[3]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.Close_Loop_AF_Hall_AMP_Offset = AF_INFO[4];
				cam_cal_buffer_list[idx_camera2].INFO_AF.Close_Loop_AF_Hall_AMP_Gain = AF_INFO[5];
				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_infinite_pattern_distance = (AF_INFO[6] | (AF_INFO[7]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_Macro_pattern_distance = (AF_INFO[8] | (AF_INFO[9]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_infinite_calibration_temperature = (AF_INFO[10]);
				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_macro_calibration_temperature = (AF_INFO[11]);

				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_infinite_calibration_temperature = (AF_INFO[16]);
				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_macro_calibration_temperature = (AF_INFO[17]);

				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_Middle_calibration = (AF_INFO[18] | (AF_INFO[19]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.AF_Middle_calibration_temperature = (AF_INFO[20]);
				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_Middle_calibration = (AF_INFO[21] | (AF_INFO[22]<<8));
				cam_cal_buffer_list[idx_camera2].INFO_AF.Warm_AF_Middle_calibration_temperature = (AF_INFO[23]);
			}
			Is_Read_To_Buffer |= 4;
        }

//#define DUMP_ALL_EEPROM 1
#if DUMP_ALL_EEPROM
        for(idx = 0 ; idx < MAX_BUFFER_SIZE_OF_ONE_EEPROM_TABLE; idx++)
        {
            CAM_CAL_LOG("info[%04d]=%02x\n", idx, info[idx]);
        }
#endif
		if(idx_camera==IDX_MAIN_CAM) CAM_CAL_LOG("Fill EEPROM Table for Main sensor, success!\n");
		else if(idx_camera==IDX_SUB_CAM) CAM_CAL_LOG("Fill EEPROM Table for Sub sensor, success!\n");
		else if(idx_camera==IDX_MAIN2_CAM) CAM_CAL_LOG("Fill EEPROM Table for Main2 sensor, success!\n");

		CAM_CAL_LOG("Is_Read_To_Buffer = %d -> %d", Is_Read_To_Buffer, (Is_Read_To_Buffer | (1<<idx_camera)));
		Is_Read_To_Buffer |= (1<<idx_camera);

        return  cam_cal_buffer_list[idx_camera].calibration_version;
    }
}


/******************************************************************************
*   To fill eeprom buffer from CAMCAL ioctrl
*   1: Sucess to fill    0: Fail to fill
*******************************************************************************/
static unsigned int EEPROM_Buffer_Fill(unsigned int* pGetSensorCalData, unsigned int idx_camera)
{
		char cBuf[64] = "/dev/CAM_CAL_DRV";
		int CamcamFID = 0;
		UINT32 result = 0, CheckHeader;
		PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

		CAM_CAL_LOG("To fill eeprom buffer sensorID=%x deviceID=%x\n", pCamCalData->sensorID, pCamCalData->deviceID);

		pCamCalData->DataVer = CAM_CAL_SINGLE_OTP_DATA;

		CamcamFID = open(cBuf, O_RDWR);
		if(CamcamFID == -1) {
		    CAM_CAL_LOG("----error: can't open CAM_CAL : %s----\n",cBuf);
		    return 0;
		}
		else {
		    CheckHeader = EEPROM_Fill_Table_From_IOCTL(CamcamFID, idx_camera, 0, pGetSensorCalData);

			if(CheckHeader == 0) {
				CAM_CAL_LOG("CheckHeader error\n");
				close(CamcamFID);
				return 0;
			}
            result = 1;
		}

        close(CamcamFID);
		CAM_CAL_LOG("Fill_EEPROM_Buffer done!\n");
		return	1;


}


static void EEPROM_Read_2AGain(PCAM_CAL_DATA_STRUCT pCamCalData, int idx_camera)
{
	UINT8 AWBAFConfig = cam_cal_buffer_list[idx_camera].INFO_2A.S2aBitEn;
	UINT8 CalR, CalGb, CalGr, CalG, CalB, tempMax;
	UINT8 FacR, FacGr, FacGb, FacG, FacB;

	memset((void*)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));//To set init value

	pCamCalData->Single2A.S2aVer = cam_cal_buffer_list[idx_camera].INFO_2A.S2aVer;
	pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
	pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);

	CalR  = cam_cal_buffer_list[idx_camera].INFO_2A.WB_R;
	CalGr = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Gr;
	CalGb = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Gb;
	CalG = ((CalGr + CalGb) + 1) >> 1;
	CalB  = cam_cal_buffer_list[idx_camera].INFO_2A.WB_B;

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
	if (CalR!=0xFF &&
		CalR!=0 && CalG!=0 && CalB!=0 )
	{
		pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (UINT32)((tempMax*512 + (CalR >> 1))/CalR);
		pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (UINT32)((tempMax*512 + (CalG >> 1))/CalG);
		pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (UINT32)((tempMax*512 + (CalB >> 1))/CalB);
	}
	else
	{
		CAM_CAL_LOG("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
	}

	FacR  = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Golden_R;
	FacGr = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Golden_Gr;
	FacGb = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Golden_Gb;
	FacG  = ((FacGr + FacGb) + 1) >> 1;
	FacB  = cam_cal_buffer_list[idx_camera].INFO_2A.WB_Golden_B;

	if(FacR > FacG) {
		/* R > G */
		if(FacR > FacB)
			tempMax = FacR;
		else
			tempMax = FacB;
	} else {
		/* G > R */
		if(FacG > FacB)
			tempMax = FacG;
		else
			tempMax = FacB;
	}

	if (FacR!=0xFF &&
		FacR!=0 && FacG!=0 && FacB!=0 )
	{
		pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (UINT32)((tempMax*512 + (FacR >> 1))/FacR);
		pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (UINT32)((tempMax*512 + (FacG >> 1))/FacG);
		pCamCalData->Single2A.S2aAwb.rGoldGainu4B = (UINT32)((tempMax*512 + (FacB >> 1))/FacB);
	}
	else
	{
		CAM_CAL_LOG("There are something wrong on EEPROM, plz contact module vendor! Golden R=%d G=%d B=%d!!\n", FacR, FacG, FacB);
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
	CAM_CAL_LOG("======================AWB CAM_CAL %d==================\n", idx_camera);
	CAM_CAL_LOG("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
	CAM_CAL_LOG("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
	CAM_CAL_LOG("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
	CAM_CAL_LOG("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
	CAM_CAL_LOG("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
	CAM_CAL_LOG("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
	CAM_CAL_LOG("======================AWB CAM_CAL==================\n");


	//=========================AF============================
    pCamCalData->Single2A.S2aAf[0] = cam_cal_buffer_list[idx_camera].INFO_2A.AF_infinite;
    pCamCalData->Single2A.S2aAf[1] = cam_cal_buffer_list[idx_camera].INFO_2A.AF_macro;

    CAM_CAL_LOG("======================AF CAM_CAL %d==================\n", idx_camera);
    CAM_CAL_LOG("[AFInf] = %d\n", pCamCalData->Single2A.S2aAf[0]);
    CAM_CAL_LOG("[AFMacro] = %d\n", pCamCalData->Single2A.S2aAf[1]);
    CAM_CAL_LOG("======================AF CAM_CAL==================\n");

	//=========================AF addition============================
    memcpy(&pCamCalData->Single2A.S2aAF_t, &cam_cal_buffer_list[idx_camera].INFO_AF, 64);
    CAM_CAL_LOG("[AF_infinite_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance);
    CAM_CAL_LOG("[AF_Macro_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance);
    CAM_CAL_LOG("[AF_Middle_calibration] = %d\n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);

}


/******************************************************************************
*   Program Entry pointer
*******************************************************************************/
unsigned int EEPROMGetCalData(unsigned int* pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

    unsigned int result = CAM_CAL_ERR_NO_DEVICE;
	unsigned int idx_camera = 0;//To find the camera which will to set eeprom

    switch(pCamCalData->deviceID)
    {
        case SENSOR_DEV_MAIN:// 1
            idx_camera = IDX_MAIN_CAM;// 0
            break;
        case SENSOR_DEV_SUB:// 2
            idx_camera = IDX_SUB_CAM;// 1
            break;
        case SENSOR_DEV_MAIN_2:// 4
            idx_camera = IDX_MAIN2_CAM;// 2
            break;
        case SENSOR_DEV_SUB_2:// 8
            idx_camera = IDX_SUB2_CAM;// 3
            break;
        case SENSOR_DEV_MAIN_3:// 16
            idx_camera = IDX_MAIN3_CAM;// 4
            break;
    }
	CAM_CAL_LOG("EEPROMGetCalData Is_Read_To_Buffer = %d  pCamCalData->deviceID=%d lsCommand = %d\n", Is_Read_To_Buffer, pCamCalData->deviceID, pCamCalData->Command);

    if((Is_Read_To_Buffer & pCamCalData->deviceID) == 0)
    {
        result = EEPROM_Buffer_Fill(pGetSensorCalData, idx_camera);
        if(result == 0) //read fail
        {
            CAM_CAL_LOG("EEPROM_Buffer_Fill =0 \n");
            return CAM_CALGetCalData(pGetSensorCalData);    //original routing
        }
    }

	CAM_CAL_LOG("Table->3A Data\n");

	switch(pCamCalData->Command)
	{
		case CAMERA_CAM_CAL_DATA_SHADING_TABLE:
            pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2;
            pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 8;
			pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = cam_cal_buffer_list[idx_camera].INFO_LSC.LSC_Length;
			memcpy((void*)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType,	//The same as DoCamCalSingleLsc
				   (const void*) cam_cal_buffer_list[idx_camera].INFO_LSC.LSC_Table,
				   LENGTH_OF_LSC);

            pCamCalData->SingleLsc.TableRotation = 0;
			CAM_CAL_LOG("======================SingleLsc Data %d==================\n", idx_camera);
			CAM_CAL_LOG("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
    	                                             pCamCalData->SingleLsc.LscTable.Data[1],
    	                                             pCamCalData->SingleLsc.LscTable.Data[2],
    	                                             pCamCalData->SingleLsc.LscTable.Data[3]);
			CAM_CAL_LOG("[1st] = SensorLSC(1) MTKLSC(2)   %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);
			CAM_CAL_LOG("LSC Table size = 0x%x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize);
            CAM_CAL_LOG("======================SingleLsc Data==================\n");
            result = CAM_CAL_ERR_NO_ERR;
			break;

        case CAMERA_CAM_CAL_DATA_3A_GAIN:
		{
            EEPROM_Read_2AGain(pCamCalData, idx_camera);
            result = CAM_CAL_ERR_NO_ERR;
			break;
        }

		case CAMERA_CAM_CAL_DATA_PDAF:
		{
            memcpy((void*) pCamCalData->PDAF.Data,
				   (const void*) cam_cal_buffer_list[idx_camera].PDAF.PDAF_Table,
				   CAM_CAL_PDAF_SIZE);
			pCamCalData->PDAF.Size_of_PDAF = cam_cal_buffer_list[idx_camera].PDAF.PDAF_Length;
			CAM_CAL_LOG("======================PDAF Data %d==================\n", idx_camera);
			CAM_CAL_LOG("[1st] = %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
    	                                             pCamCalData->PDAF.Data[1],
    	                                             pCamCalData->PDAF.Data[2],
    	                                             pCamCalData->PDAF.Data[3]);
			CAM_CAL_LOG("PDAF size=%d\n========================================\n", pCamCalData->PDAF.Size_of_PDAF);
            result = CAM_CAL_ERR_NO_ERR;
			break;
        }
		default:
			result = CAM_CAL_ERR_NO_DEVICE;
	}

    return  result;
}

/******************************************************************************
   Fast to get PDAF pointer. (Care in applications!!)
*******************************************************************************/
unsigned int EEPROM_FastGetPDAF(UINT32 deviceID, unsigned char* pPDAF)
{
    int idx_camera = 0;
    CAM_CAL_LOG("EEPROM_GetPDAF Is_Read_To_Buffer = %d  deviceID = %d\n", Is_Read_To_Buffer, deviceID);

    switch(deviceID)
    {
        case SENSOR_DEV_MAIN:// 1
            idx_camera = IDX_MAIN_CAM;// 0
            break;
        case SENSOR_DEV_SUB:// 2
            idx_camera = IDX_SUB_CAM;// 1
            break;
        case SENSOR_DEV_MAIN_2:// 4
            idx_camera = IDX_MAIN2_CAM;// 2
            break;
        case SENSOR_DEV_SUB_2:// 8
            idx_camera = IDX_SUB2_CAM;// 3
            break;
        case SENSOR_DEV_MAIN_3:// 16
            idx_camera = IDX_MAIN3_CAM;// 4
            break;
        default:
            return 0;
    }
    if((Is_Read_To_Buffer&deviceID) == 0) {
        CAM_CAL_LOG("No Fill this PDAF Table! deviceID=%d\n", deviceID);
        return 0;
    }

    //pPDAF = cam_cal_buffer_list[idx_camera].PDAF.PDAF_Table; pointer->pointer

    memcpy(pPDAF, cam_cal_buffer_list[idx_camera].PDAF.PDAF_Table,
                  cam_cal_buffer_list[idx_camera].PDAF.PDAF_Length);


    return cam_cal_buffer_list[idx_camera].PDAF.PDAF_Length;
}
