/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _SENINF_TYPE_BASE_H_
#define _SENINF_TYPE_BASE_H_

#include "kd_imgsensor_define.h"
#include <camera_custom_imgsensor_cfg.h>

/*******************************************************************************
*
********************************************************************************/
#define MCLK_24MHZ 24

#define HALSENSOR_CAM_PIXEL_MODE TWO_PIXEL_MODE
#define HALSENSOR_CAMSV_PIXEL_MODE TWO_PIXEL_MODE

typedef enum{
	SENINF_MUX1 = 0x0,
	SENINF_MUX2 = 0x1,
	SENINF_MUX3 = 0x2,
	SENINF_MUX4 = 0x3,
	SENINF_MUX_NUM,
}SENINF_MUX_ENUM;

typedef enum{
    SENINF_TOP_TG1 = SENINF_MUX1,
    SENINF_TOP_TG2 = SENINF_MUX2,
    SENINF_TOP_SV1 = SENINF_MUX3,
    SENINF_TOP_SV2 = SENINF_MUX4,
    SENINF_P1_NUM,
}SENINF_TOP_P1_ENUM;


typedef enum{
    SENINF_1 = 0x0,
    SENINF_2 = 0x1,
    SENINF_3 = 0x2,
    SENINF_4 = 0x3,
    SENINF_NUM,
}SENINF_ENUM;

typedef enum {
    PAD_10BIT       = 0x0,
    PAD_8BIT_7_0    = 0x3,
    PAD_8BIT_9_2    = 0x4,
}PAD2CAM_DATA_ENUM;


typedef enum { //0:CSI2(2.5G), 3: parallel, 8:NCSI2(1.5G)
    CSI2            = 0x0, /* 2.5G support */
    TEST_MODEL      = 0x1,
    CCIR656         = 0x2,
    PARALLEL_SENSOR = 0x3,
    SERIAL_SENSOR   = 0x4,
    HD_TV           = 0x5,
    EXT_CSI2_OUT1   = 0x6,
    EXT_CSI2_OUT2   = 0x7,
    MIPI_SENSOR     = 0x8,/* 1.5G support */
    VIRTUAL_CHANNEL_1 	= 0x9,
    VIRTUAL_CHANNEL_2 	= 0xA,
    VIRTUAL_CHANNEL_3 	= 0xB,
	VIRTUAL_CHANNEL_4 	= 0xC,
    VIRTUAL_CHANNEL_5 	= 0xD,
    VIRTUAL_CHANNEL_6 	= 0xE,
}SENINF_SOURCE_ENUM;

typedef enum { //0:CSI2(2.5G), 1:NCSI2(1.5G)
    CSI2_1_5G           = MIPI_OPHY_NCSI2, /* 1.5G support */
    CSI2_2_5G           = MIPI_OPHY_CSI2, /* 2.5G support*/
    CSI2_2_5G_CPHY      = MIPI_CPHY, /* 2.5G support*/
}SENINF_CSI2_ENUM;
/*
MIPI_OPHY_NCSI2 = 0,
MIPI_OPHY_CSI2	= 1,
MIPI_CPHY		= 2,
*/
typedef enum {
	SENINF1_CSI0	= 0x0, /*4 Lane*/
	SENINF2_CSI1	= 0x1, /*4 Lane*/
	SENINF_CSI2_IP_NUM,
}SENINF_CSI2_IP_ENUM;

typedef enum {
    TG_12BIT    = 0x0,
    TG_10BIT    = 0x1,
    TG_8BIT     = 0x2
}SENSOR_DATA_BITS_ENUM;

typedef enum {
    RAW_8BIT_FMT        = 0x0,
    RAW_10BIT_FMT       = 0x1,
    RAW_12BIT_FMT       = 0x2,
    YUV422_FMT          = 0x3,
    RAW_14BIT_FMT       = 0x4,
    RGB565_MIPI_FMT     = 0x5,
    RGB888_MIPI_FMT     = 0x6,
    JPEG_FMT            = 0x7
}TG_FORMAT_ENUM;


typedef enum {
    MIPI_DESKEW_NONE = 0,
    MIPI_DESKEW_SW = 1,
	MIPI_DESKEW_HW = 2,
} MIPI_DESKEW_ENUM;


typedef struct {
	MUINT32 mipiPad;
	SENINF_ENUM             seninfSrc;
	SENINF_CSI2_IP_ENUM		CSI2_IP;
	MUINT32 			dataTermDelay;
	MUINT32 			dataSettleDelay;
	MUINT32 			clkTermDelay;
	MUINT32 			vsyncType;
	MUINT32 			dlaneNum;
	MUINT32 			Enable;
	MUINT32 			dataHeaderOrder;
	SENSOR_MIPI_TYPE_ENUM    mipi_type;
	MUINT32 			HSRXDE;
	MUINT32 			dpcm;
	MIPI_DESKEW_ENUM    mipi_deskew; /*0: no suport hw deskew, 1: DPHY1.2 deskew*/
	MUINT64 			mipi_pixel_rate;
    SENSOR_VC_INFO_STRUCT vcInfo;
} SENINF_CSI_PARA;

typedef struct {
	MUINT32     mclkIdx;
	MUINT32		mclkFreq;
	MBOOL		mclkPolarityLow;
	MUINT8		mclkRisingCnt;
	MUINT8		mclkFallingCnt;
	MUINT32		pclkInv;
	MUINT32		mclkPLL;
	IMGSENSOR_SENSOR_IDX sensorIdx;
} SENINF_MCLK_PARA;

typedef struct {
    MUINT SCAM_DataNumber; // 0: NCSI2 , 1:CSI2
    MUINT SCAM_DDR_En; // 0: Enable HS Detect, 1: disable HS Detect
    MUINT SCAM_CLK_INV; // Enable DPCM mode type
    MUINT SCAM_DEFAULT_DELAY; // default delay for calibration
    MUINT SCAM_CRC_En;
    MUINT SCAM_SOF_src;
    MUINT SCAM_Timout_Cali;
}SENINF_SCAM_PARA;


typedef struct {
    MUINT u1HsyncPol;
    MUINT u1VsyncPol;
}SENINF_PARALLEL_CAM_PARA;


typedef struct {
    PAD2CAM_DATA_ENUM padSel;
    MUINT scenarioId;
    MUINT u1IsContinuous;
    MUINT u1IsBypassSensorScenario;
    MUINT u4PixelX0;
    MUINT u4PixelX1;
    MUINT u4PixelY0;
    MUINT u4PixelY1;
    TG_FORMAT_ENUM inDataType;
    MUINT cropWidth;
    MUINT cropHeight;
    MUINT frameRate;
    MUINT twopixelOn;
    MUINT debugMode;
    MUINT HDRMode;
    MUINT PDAFMode; /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode*/
    MUINT senInLsb;
    MUINT64 pixel_rate;
    SENINF_SOURCE_ENUM inSrcTypeSel;
    SENINF_CSI_PARA csi_para;
    SENINF_PARALLEL_CAM_PARA parallel_para;
    SENINF_SCAM_PARA scam_para;
    IMGSENSOR_SENSOR_IDX sensor_idx;
}SENINF_CONFIG_STRUCT;


#endif // _ISP_DRV_H_

