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
#ifndef __SENINF_DRV_DEF_H__
#define __SENINF_DRV_DEF_H__

#include <mtkcam/drv/IHalSensor.h>
#include "camera_custom_imgsensor_cfg.h"
#include "kd_imgsensor_define.h"

using namespace NSCamCustomSensor;

/*******************************************************************************
*
********************************************************************************/
typedef enum{
    SENINF_MUX1 = 0x0,
    SENINF_MUX2 = 0x1,
    SENINF_MUX3 = 0x2,
    SENINF_MUX4 = 0x3,
    SENINF_MUX5 = 0x4,
    SENINF_MUX6 = 0x5,
    SENINF_MUX_NUM,
    SENINF_MUX_ERROR = -1,
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
}SENINF_SOURCE_ENUM;

typedef enum { //0:CSI2(2.5G), 1:NCSI2(1.5G)
    CSI2_1_5G           = 0x0, /* 1.5G support */
    CSI2_2_5G           = 0x1, /* 2.5G support*/
    CSI2_2_5G_CPHY      = 0x2, /* 2.5G support*/
}SENINF_CSI2_ENUM;

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


typedef struct {
    CUSTOM_CFG_CSI_PORT port;
    SENINF_ENUM         seninf;
    SENINF_SOURCE_ENUM  srcType;
} SENINF_CSI_INFO;

typedef struct {
    unsigned int     enable;
    SENINF_CSI_INFO  *pCsiInfo;
    SENINF_CSI2_ENUM csi_type;
    unsigned int     dlaneNum;
    unsigned int     dpcm;
    unsigned int     dataheaderOrder;
    unsigned int     padSel;
    unsigned int     line_length;
    unsigned int     pclk;
    unsigned int     mipi_pixel_rate;
    TG_FORMAT_ENUM   inDataType;
} SENINF_CSI_MIPI;

typedef struct {
    MUINT enable;
    MUINT SCAM_DataNumber; // 0: NCSI2 , 1:CSI2
    MUINT SCAM_DDR_En; // 0: Enable HS Detect, 1: disable HS Detect
    MUINT SCAM_CLK_INV; // Enable DPCM mode type
    MUINT SCAM_DEFAULT_DELAY; // default delay for calibration
    MUINT SCAM_CRC_En;
    MUINT SCAM_SOF_src;
    MUINT SCAM_Timout_Cali;
} SENINF_CSI_SCAM;

typedef struct {
    MUINT enable;
} SENINF_CSI_PARALLEL;

typedef struct {
    MUINT32     mclkIdx;
    MUINT32	mclkFreq;
    MBOOL	mclkPolarityLow;
    MUINT8	mclkRisingCnt;
    MUINT8	mclkFallingCnt;
    MUINT32	pclkInv;
    MUINT32	mclkPLL;
    IMGSENSOR_SENSOR_IDX sensorIdx;
} SENINF_MCLK_PARA;

#define SENINF_CAM_MUX_MIN      SENINF_MUX1
#define SENINF_CAM_MUX_MAX      SENINF_MUX3
#define SENINF_CAMSV_MUX_MIN    SENINF_MUX3
#define SENINF_CAMSV_MUX_MAX    SENINF_MUX_NUM

#define SENINF_PIXEL_MODE_CAM   FOUR_PIXEL_MODE
#define SENINF_PIXEL_MODE_CAMSV FOUR_PIXEL_MODE

#define SENINF_TIMESTAMP_CLK    1000

#define IS_MUX_HW_BUFFERED(x) (x < SENINF_MUX3)
#define HW_BUF_EFFECT 80
#define OFFSET_START_EXPOSURE 3000000
#define MCLK_DRIVE_CURRENT_BY_PINCTRL

#endif

