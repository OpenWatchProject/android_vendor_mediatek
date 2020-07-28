/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
 **************************************************/


#ifndef _ICAM_TYPE_H_
#define _ICAM_TYPE_H_


/*This namedspace for old enum use, new enum no need declare in here*/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

typedef enum{
    CAM_UNKNOWN         = 0x00000000,
    CAM_BAYER8          = 0x00000001,
    CAM_BAYER10         = 0x00000002,
    CAM_BAYER12         = 0x00000004,
    CAM_BAYER14         = 0x00000008,
    CAM_MIPI10_BAYER8   = 0x00000010,
    CAM_MIPI10_BAYER10  = 0x00000020,
    CAM_MIPI10_BAYER12  = 0x00000040,
    CAM_MIPI10_BAYER14  = 0x00000080,
    CAM_FG_BAYER8       = 0x00000100,
    CAM_FG_BAYER10      = 0x00000200,
    CAM_FG_BAYER12      = 0x00000400,
    CAM_FG_BAYER14      = 0x00000800,
    CAM_UFEO_BAYER8     = 0x00001000,
    CAM_UFEO_BAYER10    = 0x00002000,
    CAM_UFEO_BAYER12    = 0x00004000,
    CAM_UFEO_BAYER14    = 0x00008000
}E_CAM_FORMAT;

/******************************************************************************
 * @enum E_CAM_PipelineBitDepth_SEL
 *
 * @Pipeline bit depth format
 *
 ******************************************************************************/
typedef enum {
    CAM_Pipeline_10BITS = 0x0,
    CAM_Pipeline_12BITS = 0x1,
    CAM_Pipeline_14BITS = 0x2,
    CAM_Pipeline_16BITS = 0x4,
} E_CAM_PipelineBitDepth_SEL;

/**
    type for p1hwcfg module.
    note: Data type for ISP3.0
*/
enum EModule
{
    //raw
    EModule_OB          = 00,
    EModule_BNR         = 05,
    EModule_LSC         = 10,
    EModule_RPG         = 15,
    EModule_AE          = 20,
    EModule_AWB         = 25,
    EModule_SGG1        = 30,
    EModule_FLK         = 35,
    EModule_AF          = 40,
    EModule_SGG2        = 45,
    EModule_SGG3        = 46,
    EModule_EIS         = 50,
    EModule_LCS         = 55,
    EModule_BPCI        = 60,
    EModule_LSCI        = 65,
    EModule_AAO         = 70,
    EModule_ESFKO       = 75,
    EModule_AFO         = 80,
    EModule_EISO        = 85,
    EModule_LCSO        = 90,
    EModule_LCESHO        = 91,
    EModule_vHDR        = 95,
    EModule_CAMSV_IMGO  = 100,
    EModule_CAMSV2_IMGO  = 101,
    //raw_d
    EModule_OB_D        = 1000,
    EModule_BNR_D       = 1005,
    EModule_LSC_D       = 1010,
    EModule_RPG_D       = 1015,
    EModule_BPCI_D      = 1020,
    EModule_LSCI_D      = 1025,
    EModule_AE_D        = 1030,
    EModule_AWB_D       = 1035,
    EModule_SGG1_D      = 1040,
    EModule_AF_D        = 1045,
    EModule_LCS_D       = 1050,
    EModule_AAO_D       = 1055,
    EModule_AFO_D       = 1060,
    EModule_LCSO_D      = 1065,
    EModule_vHDR_D      = 1070
};

/******************************************************************************
 * @enum E_CamPixelMode
 *
 * @Pixel mode:
 *
 ******************************************************************************/
typedef enum{
    ePixMode_NONE = 0,
    ePixMode_1,
    ePixMode_2,
    ePixMode_4,
    ePixMode_8,
    ePixMode_MAX,
    _UNKNOWN_PIX_MODE = ePixMode_NONE,
    _1_PIX_MODE = ePixMode_1,
    _2_PIX_MODE = ePixMode_2,
    _4_PIX_MODE = ePixMode_4,
    _8_PIX_MODE = ePixMode_8,
    _MAX_PIX_MODE = ePixMode_MAX,
}E_CamPixelMode;


/******************************************************************************
 * @enum E_CamPipeBitdepthMode
 *
 * @Pipeline bit mode:
 *  1 for low bit on: pipeline bitdepth--
 *  0 for low bit off: pipeline bitdepth++
 *
 ******************************************************************************/
typedef enum {
    eBitdepthMode_Lowbit_AfterTG = 0x01,  //G1
    eBitdepthMode_Lowbit_AfterFUS = 0x02  //G2
} E_CamPipeBitdepthMode;

typedef E_CamPixelMode Normalpipe_PIXMODE;
}
}
}

/******************************************************************************
 * @enum E_INPUT
 *
 * @TG input index
 *
 ******************************************************************************/
typedef enum{
    TG_A         = 0,  //mapping to hw module CAM_A      0
    TG_B         = 1,  //mapping to hw module CAM_B      1
    TG_C         = 2,  //mapping to hw module CAM_C      2
    TG_CAM_MAX,
    TG_CAMSV_0   = 10, //mapping to hw module CAMSV_0  4
    TG_CAMSV_1   = 11, //mapping to hw module CAMSV_1  5
    TG_CAMSV_2   = 12, //mapping to hw module CAMSV_2  6
    TG_CAMSV_3   = 13, //mapping to hw module CAMSV_3  7
    TG_CAMSV_4   = 14, //mapping to hw module CAMSV_4  8
    TG_CAMSV_5   = 15, //mapping to hw module CAMSV_5  9
    TG_CAMSV_6   = 16, //mapping to hw module CAMSV_6  10
    TG_CAMSV_7   = 17, //mapping to hw module CAMSV_7  11
    TG_CAMSV_MAX,
    RAWI,
}E_INPUT;

/******************************************************************************
 * @enum E_CAMIO_Pattern
 *
 * @input data pattern
 *
 ******************************************************************************/
typedef enum{
    eCAM_NORMAL   = 0,
    eCAM_DUAL_PIX,      //dual pd's pattern
    eCAM_QuadCode,      //for MT6763
    eCAM_4CELL,
    eCAM_MONO,
    eCAM_IVHDR,
    eCAM_ZVHDR,
    eCAM_4CELL_IVHDR,
    eCAM_4CELL_ZVHDR,
    eCAM_DUAL_PIX_IVHDR,
    eCAM_DUAL_PIX_ZVHDR,
    eCAM_YUV, //driver internal use
    eCAM_NORMAL_PD,     //bayer with PD, drv internal use
}E_CamPattern;

/******************************************************************************
 * @enum E_CamIQLevel
 *
 * @Image Quality level for dynamic bin
 *
 ******************************************************************************/
typedef enum {
    eCamIQ_L = 0,
    eCamIQ_H,
    eCamIQ_MAX,
} E_CamIQLevel;

//For ENPipeCmd_SET_QUALITY
typedef union
{
	struct
	{
		E_CamIQLevel Qlvl_0     : 2; //L, H, MAX
		MUINT32 SensorIdx_0     : 4; //CurrentMax IOPIPE_MAX_SENSOR_CNT=5
		E_CamIQLevel Qlvl_1     : 2; //L, H, MAX
		MUINT32 SensorIdx_1     : 4; //CurrentMax IOPIPE_MAX_SENSOR_CNT=5
		MUINT32 rsv             : 20;
	} Bits = {eCamIQ_MAX, 0, eCamIQ_MAX, 0, 0};
	MUINT32 Raw;
}CAM_QUALITY;

/******************************************************************************
 * @enum E_SuspendLevel
 *
 * @Type1: suspend with sensor is on
 *    Type2: suspend with sensor is off
 *
 ******************************************************************************/
typedef enum{
    eSuspend_Unknow = 0,
    eSuspend_Type1,
    eSuspend_Type2,
}E_SUSPEND_TPYE;

/******************************************************************************
 * @struct _UFDG_META_INFO
 *
 * @UF format meta info
 *
 ******************************************************************************/
typedef struct _UFDG_META_INFO{
    MUINT32 bUF;
    MUINT32 UFDG_BITSTREAM_OFST_ADDR;
    MUINT32 UFDG_BS_AU_START;
    MUINT32 UFDG_AU2_SIZE;
    MUINT32 UFDG_BOND_MODE;
} UFDG_META_INFO;


/******************************************************************************
*  @ enum for supported RAWI path.
*******************************************************************************/
typedef enum {
    noRAWI          = 0x0,
    isDCIF          = 0x1,
    isFakeStagger   = 0x2,
    isM2MBasic      = 0x4,
} E_CamRawiPath;

/******************************************************************************
*  @ union resConfig for ISP CAM resource config
*******************************************************************************/
typedef union
{
        struct
        {
                MUINT32 targetTG        : 6;
                MUINT32 isOffTwin       : 1;
                MUINT32 rsv             : 25;
        } Bits = {TG_CAM_MAX, 0, 0};
        MUINT32 Raw;
}CAM_RESCONFIG;

typedef struct _tImgSize{
    MUINT32 w;
    MUINT32 h;
}tImgSize;

using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_NONE;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_1;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_2;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_4;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_8;
using NSCam::NSIoPipe::NSCamIOPipe::E_CamPixelMode::ePixMode_MAX;


#define REG_DUMP_NOP    0xDEADDEAD

#define REG_DUMP_DUMMY_8    (0xFE)
//((REG_DUMP_DUMMY_8<<24)|(REG_DUMP_DUMMY_8<<16)|(REG_DUMP_DUMMY_8<<8)|REG_DUMP_DUMMY_8)
#define REG_DUMP_DUMMY      (0xFEFEFEFE)
#endif
