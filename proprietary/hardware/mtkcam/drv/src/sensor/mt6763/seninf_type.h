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

/*******************************************************************************
*
********************************************************************************/
typedef enum{
    SENINF_TOP_TG1 = 0x0,
    SENINF_TOP_TG2 = 0x1,
    SENINF_TOP_SV1 = 0x2,
    SENINF_TOP_SV2 = 0x3,
    SENINF_TOP_SV3 = 0x4,
    SENINF_TOP_SV4 = 0x5,
    SENINF_TOP_SV5 = 0x6,
    SENINF_TOP_SV6 = 0x7,
    SENINF_P1_NUM,
}SENINF_TOP_P1_ENUM;

typedef enum{
	SENINF_MUX1 = 0x0,
	SENINF_MUX2 = 0x1,
	SENINF_MUX3 = 0x2,
	SENINF_MUX4 = 0x3,
	SENINF_MUX5 = 0x4,
	SENINF_MUX6 = 0x5,
	SENINF_MUX7 = 0x6,
	SENINF_MUX8 = 0x7,
	SENINF_MUX_NUM,
}SENINF_MUX_ENUM;


typedef enum{
    SENINF_1 = 0x0,
    SENINF_2 = 0x1,
    SENINF_3 = 0x2,
    SENINF_4 = 0x3,
    SENINF_5 = 0x4,
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
    VIRTUAL_VC_12		= 0x20,
}SENINF_SOURCE_ENUM;

typedef enum { //0:CSI2(2.5G), 1:NCSI2(1.5G)
    CSI2_1_5G           = 0x0, /* 1.5G support */
    CSI2_2_5G           = 0x1, /* 2.5G support*/
    CSI2_2_5G_CPHY      = 0x2, /* 2.5G support*/
}SENINF_CSI2_ENUM;

typedef enum {
	SENINF1_CSI0	= 0x1, /*4 Lane*/
	SENINF3_CSI1	= 0x2, /*4 Lane*/
	SENINF5_CSI2	= 0x3, /*4 Lane*/
	SENINF1_CSI0A	= 0x4,
	SENINF2_CSI0B	= 0x5,
	SENINF3_CSI1A	= 0x6,
	SENINF4_CSI1B	= 0x7,
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
    ACTIVE_HIGH     = 0x0,
    ACTIVE_LOW      = 0x1,
}CCIR656_OUTPUT_POLARITY_ENUM;

typedef enum {
    IMMIDIANT_TRIGGER   = 0x0,
    REFERENCE_VS1       = 0x1,
    I2C1_BEFORE_I2C2    = 0x2,
    I2C2_BEFORE_I2C1    = 0x3
}N3D_I2C_TRIGGER_MODE_ENUM;

typedef enum drvSeninfCmd_s {
    CMD_SET_DEVICE              = 0x1000,
    CMD_GET_SENINF_ADDR         = 0x2001,
    CMD_DEBUG_TASK				= 0x3001,
    CMD_DEBUG_TASK_CAMSV	    = 0x4001,
    CMD_DRV_SENINF_MAX			= 0xFFFF
} drvSeninfCmd_e;

/* To Remove bk_ */
#define CAM_PLL_48_GROUP        (1)
#define CAM_PLL_52_GROUP        (2)
#define CAM_PLL_24_GROUP        (1)
#define CAM_PLL_26_GROUP        (2)

typedef enum {
	MCLK_26MHZ,
	MCLK_24MHZ,
	MCLK_52MHZ,
	MCLK_48MHZ,
	MCLK_26MHZ_2,
	MCLK_13MHZ,
	MCLK_12MHZ,
	MCLK_6MHZ,
	MCLK_MAX,
}MCLK_TG_SRC_ENUM;

typedef enum {
	FREQ_26MHZ = 26,
	FREQ_24MHZ = 24,
	FREQ_52MHZ = 52,
	FREQ_48MHZ = 48,
	FREQ_26MHZ_2 = 0xff,
	FREQ_13MHZ = 13,
	FREQ_12MHZ = 12,
	FREQ_6MHZ = 6,
}MCLK_FREQ_ENUM;


typedef struct {
	SENINF_ENUM             seninfSrc;
	SENINF_CSI2_IP_ENUM		CSI2_IP;
	PAD2CAM_DATA_ENUM 		padSel;
	SENINF_SOURCE_ENUM 		inSrcTypeSel;
	SENINF_CSI2_ENUM        csi_type;
	unsigned int 			dataTermDelay;
	unsigned int 			dataSettleDelay;
	unsigned int 			clkTermDelay;
	unsigned int 			vsyncType;
	unsigned int 			dlaneNum;
	unsigned int 			Enable;
	unsigned int 			dataheaderOrder;
	SENINF_CSI2_ENUM 		mipi_type;
	unsigned int 			HSRXDE;
	unsigned int 			dpcm;
	unsigned int 			MIPIDeskew; /*0: no suport hw deskew, 1: DPHY1.2 deskew*/
} seninf_csi_para;


#endif // _ISP_DRV_H_

