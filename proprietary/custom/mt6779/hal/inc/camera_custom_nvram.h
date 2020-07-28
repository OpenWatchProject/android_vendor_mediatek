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
 ************************************************************************************************/
#ifndef _CAMERA_CUSTOM_NVRAM_H_
#define _CAMERA_CUSTOM_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "CFG_Camera_File_Max_Size.h"
#include "camera_custom_3a_nvram.h"
#include "camera_custom_ae_nvram.h"
#include "camera_custom_ae_sync_nvram.h"
#include "camera_custom_awb_nvram.h"
#include "camera_custom_awb_sync_nvram.h"
#include "camera_custom_af_nvram.h"
#include "camera_custom_flash_nvram.h"
#include "camera_custom_tsf_tbl.h"
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "camera_custom_tuning_size.h"
//#include "../../../../../hardware/mtkcam/aaa/include/private/aaa_hal_private.h"
#define SENSOR_IDX_MAX (20)

using namespace NSIspTuning;

#define NVRAM_CAMERA_SHADING_FILE_VERSION       1
#define NVRAM_CAMERA_PARA_FILE_VERSION          1
#define NVRAM_CAMERA_3A_FILE_VERSION            1
#define NVRAM_CAMERA_LENS_FILE_VERSION          1
#define NVRAM_CAMERA_STROBE_FILE_VERSION          2 // SC MODIFY
#define NVRAM_CAMERA_FEATURE_FILE_VERSION          1
#define NVRAM_CAMERA_GEOMETRY_FILE_VERSION          1
#define NVRAM_CAMERA_FOV_FILE_VERSION           1
#define NVRAM_CAMERA_PLINE_FILE_VERSION          1
#define NVRAM_CAMERA_AF_FILE_VERSION          1
#define NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION          1
#ifndef MTK_STEREO_KERNEL_NVRAM_LENGTH
//May defined in algo header
#define MTK_STEREO_KERNEL_NVRAM_LENGTH (8400) // refer to the same define in MtkStereoKernel.h
#endif

/*******************************************************************************
* ANR table
********************************************************************************/
#define ANR_SUPPORT_CT_NUM              (2)
#define ANR_SUPPORT_ISO_NUM             (6)
#define ANR_HW_TBL_NUM                  (256)

/*******************************************************************************
* shading
********************************************************************************/
#define SHADING_SUPPORT_CT_NUM          (4)
#define SHADING_SUPPORT_OP_NUM          (10)
#define SHADING_SUPPORT_CH_NUM          (4)
#define MAX_FRM_GRID_NUM                (16)
#define MAX_TIL_GRID_NUM                (32)
#define COEFF_BITS_PER_CH               (192)
#define COEFF_PER_CH_U32                (COEFF_BITS_PER_CH>>5)
#define COEFF_PER_CH_U8                 (COEFF_BITS_PER_CH>>3)
#define MAX_SHADING_SIZE                (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1024) //INT32
#define MAX_SHADING_CapFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(4096) //INT32
#define MAX_SHADING_CapTil_SIZE         (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(4096) //INT32
#define MAX_SHADING_PvwFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SHADING_PvwTil_SIZE         (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SHADING_VdoFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SVD_SHADING_SIZE            (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*sizeof(UINT32))//(1024) //Byte
#define MAX_SENSOR_CAL_SIZE             (2048)//(1024) //Byte

#define member_size(type, member) sizeof(((type *)0)->member)
#define struct_size(type, start, end) \
    ((offsetof(type, end) - offsetof(type, start) + member_size(type, end)))

#define SIZEOF  sizeof

typedef struct {
    MUINT8     PixId; //0,1,2,3: B,Gb,Gr,R
    MUINT32    SlimLscType; //00A0  FF 00 02 01 (4 bytes)       4
    MUINT16    Width; //00A8    Capture Width (2 bytes) Capture Height (2 bytes)    2
    MUINT16    Height; //00A8    Capture Width (2 bytes) Capture Height (2 bytes)    2
    MUINT16    OffsetX; //00AA    Capture Offset X (2 bytes)  Capture Offfset Y (2 bytes) 2
    MUINT16    OffsetY; //00AA    Capture Offset X (2 bytes)  Capture Offfset Y (2 bytes) 2
    MUINT32    TblSize; //00B0   Capture Shading Table Size (4 bytes)        4
    MUINT32    IspLSCReg[5]; //00C8 Capture Shading Register Setting (5x4 bytes)        20
    MUINT8     GainTable[2312]; //00DC   Capture Shading Table (17 X 17 X 2 X 4 bytes)       2312
    //MUINT8     UnitGainTable[2048]; //2048
} SHADING_GOLDEN_REF;

#define SHADING_DATA                                                                \
    struct {                                                                        \
        UINT32 Version;                                                             \
        UINT32 SensorId;                                                            \
        UINT8  GridXNum;                                                            \
        UINT8  GridYNum;                                                            \
        UINT16 Width;                                                               \
        UINT16 Height;                                                              \
        UINT32 CapTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        SHADING_GOLDEN_REF SensorGoldenCalTable;                                    \
        }


struct _ISP_SHADING_STRUCT
{
    SHADING_DATA;
};

typedef struct
{
    SHADING_DATA;
    UINT8 CameraData[MAXIMUM_CAMERA_SHADING_SIZE-
                     sizeof(struct _ISP_SHADING_STRUCT)];
} ISP_SHADING_STRUCT, *PISP_SHADING_STRUCT;

typedef struct
{
    ISP_SHADING_STRUCT  Shading;
} NVRAM_CAMERA_SHADING_STRUCT, *PNVRAM_CAMERA_SHADING_STRUCT;

/*******************************************************************************
* 3A
********************************************************************************/



// AE NVRAM Mapping Table by Scenario
static MUINT32 AENVRAMMapping[CAM_SCENARIO_NUM] = {
    static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW
    static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO
    static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO
};

//____3A NVRAM____
//typedef unsigned char  UINT8;
// AWB NVRAM Mapping Table by Scenario
static MUINT32 AWBNVRAMMapping[CAM_SCENARIO_NUM] = {
    static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW
    static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO
    static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO
};

/*******************************************************************************
*
********************************************************************************/
//____AF NVRAM____
#define AF_CAM_SCENARIO_NUM CAM_SCENARIO_NUM      // TO_BE_REPLACED by scenario_num_2
#define AF_CAM_SCENARIO_NUM_2             (10)

// AF NVRAM Mapping Table by Scenario
static MUINT32 AFNVRAMMapping[AF_CAM_SCENARIO_NUM] = {
    static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW (binning-size preview)
    static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO   (full-size vdo record)
    static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE (full-size preview)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR     (N/A)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR(N/A)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT      (binning-size vdo record)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO  (N/A)
};

typedef struct
{
    MUINT8          rFilePath[256];
    AF_NVRAM_T      rAFNVRAM;
    PD_NVRAM_T      rPDNVRAM;
} NVRAM_LENS_DATA_PARA_STRUCT, *PNVRAM_LENS_DATA_PARA_STRUCT;

typedef struct
{
    UINT32 Version;
    FOCUS_RANGE_T   rFocusRange;
    NVRAM_LENS_DATA_PARA_STRUCT AF[AF_CAM_SCENARIO_NUM_2];
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(NVRAM_LENS_DATA_PARA_STRUCT)*AF_CAM_SCENARIO_NUM_2];
} NVRAM_LENS_PARA_STRUCT, *PNVRAM_LENS_PARA_STRUCT;

typedef union
{
    struct
    {
        MUINT8    LaserOffsetCalEn;
        MUINT16   LaserOffsetCalVal;
        MUINT8    LaserXTalkCalEn;
        MUINT16   LaserXTalkCalVal;

        PD_CALIBRATION_DATA_T rPdCaliData;
    };
    UINT8 Data[MAXIMUM_NVRAM_CAMERA_AF_FILE_SIZE];

} NVRAM_AF_PARA_STRUCT, *PNVRAM_AF_PARA_STRUCT;



typedef struct
{
    //data structure version, update once structure been modified.
    UINT32 u4Version;

    // ID of sensor module
    UINT32 SensorId;

    //data content
    AE_NVRAM_T                      AE[AE_CAM_SCENARIO_NUM];
    AWB_NVRAM_T                     AWB[AWB_CAM_SCENARIO_NUM];
    AESYNC_NVRAM_T                  AESYNC;
    AWBSYNC_NVRAM_T                 AWBSYNC;
//    FLASH_AE_NVRAM_T                Flash_AE[FlASH_AE_NUM_2];
//    FLASH_AWB_NVRAM_T               Flash_AWB[FlASH_AWB_NUM_2];
//    FLASH_CALIBRATION_NVRAM_T       Flash_Calibration[FlASH_CALIBRATION_NUM_2];
    //SSS(reserved unused spaces(bytes)) = total-used;,
    //ex. SSS = 4096-sizeof(UINT32)--sizeof(NVRAM_AAA_T)-sizeof(NVRAM_bbb_T);
    //    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)-sizeof(AF_NVRAM_T)-sizeof(AWB_NVRAM_T)];
//    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)*AE_CAM_SCENARIO_NUM_2-sizeof(AWB_NVRAM_T)*AWB_CAM_SCENARIO_NUM_2-sizeof(NVRAM_LENS_DATA_PARA_STRUCT)*AF_CAM_SCENARIO_NUM_2-sizeof(FLASH_AE_NVRAM_T)*FlASH_AE_NUM_2-sizeof(FLASH_AWB_NVRAM_T)*FlASH_AWB_NUM_2-sizeof(FLASH_CALIBRATION_NVRAM_T)*FlASH_CALIBRATION_NUM_2];
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)*AE_CAM_SCENARIO_NUM-sizeof(AWB_NVRAM_T)*AWB_CAM_SCENARIO_NUM - sizeof(AESYNC_NVRAM_T)- sizeof(AWBSYNC_NVRAM_T)];
} NVRAM_CAMERA_3A_STRUCT, *PNVRAM_CAMERA_3A_STRUCT;

typedef struct NVRAM_CAMERA_IDX_STRUCT_T
{
    unsigned short idx_factor_ns[EDim_NUM];
    void * modules[EModule_NUM];
    unsigned short scenario_ns;
    char (*scenarios)[][64];
} NVRAM_CAMERA_IDX_STRUCT, *PNVRAM_CAMERA_IDX_STRUCT;


/*******************************************************************************
*
********************************************************************************/
#define CAL_INFO_IN_COMM_LOAD   34

#define CAL_GET_DEFECT_FLAG     0x01
#define CAL_GET_3ANVRAM_FLAG    0x02
#define CAL_GET_SHADING_FLAG    0x04
#define CAL_GET_PARA_FLAG       0x08
#define CAL_DATA_LOAD           0x6C6F6164//"load"
#define CAL_DATA_UNLOAD         0x00000000
#define CAL_SHADING_TYPE_SENSOR 0x216D746B//"!mtk"
#define CAL_SHADING_TYPE_ISP    0x3D6D746B//"=mtk"


/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    CAMERA_DATA_TYPE_START=0,
    CAMERA_NVRAM_DATA_ISP = CAMERA_DATA_TYPE_START,
    CAMERA_NVRAM_DATA_3A,
    CAMERA_NVRAM_DATA_SHADING,
    CAMERA_NVRAM_DATA_LENS,
    CAMERA_DATA_AE_PLINETABLE,
    CAMERA_NVRAM_DATA_STROBE,
    CAMERA_DATA_TSF_TABLE,
    CAMERA_DATA_PDC_TABLE,
    CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_DATA_FOV,
    CAMERA_NVRAM_IDX_TBL,
    CAMERA_NVRAM_DATA_FEATURE,
    CAMERA_NVRAM_DATA_AF,
    CAMERA_NVRAM_DATA_AF_CALIBRATION = CAMERA_NVRAM_DATA_AF,
    CAMERA_NVRAM_DATA_FLASH_CALIBRATION,
    CAMERA_NVRAM_VERSION,
    CAMERA_DATA_TYPE_NUM
} CAMERA_DATA_TYPE_ENUM;

typedef enum
{
    CAMSV_CUSTOM_DECODE_START = 0,
    CAMSV_CUSTOM_DECODE_AE = CAMSV_CUSTOM_DECODE_START,
    CAMSV_CUSTOM_DECODE_FLK,
    CAMSV_CUSTOM_DECODE_NUM
} CAMSV_CUSTOM_DECODE_ENUM;

typedef struct
{
    MUINT32 width;
    MUINT32 height;
    MUINT32 size;
} CAMSV_CUSTOM_BUFFER_INFO;

typedef enum
{
    AE_CUSTOM_TRANSFORM_START = 0,
    AE_CUSTOM_TRANSFORM_BINSUM = AE_CUSTOM_TRANSFORM_START,
    AE_CUSTOM_TRANSFORM_ATR,
    AE_CUSTOM_TRANSFORM_NUM
} AE_CUSTOM_TRANSFORM_ENUM;


typedef enum
{
    GET_CAMERA_DATA_NVRAM,
    GET_CAMERA_DATA_DEFAULT,
    SET_CAMERA_DATA_NVRAM,
} MSDK_CAMERA_NVRAM_DATA_CTRL_CODE_ENUM;

typedef union
{
    struct
    {
            short ispVer[SENSOR_IDX_MAX]; //main, sub, main2
            short aaaVer[SENSOR_IDX_MAX];
            short shadingVer[SENSOR_IDX_MAX];
            short lensVer[SENSOR_IDX_MAX];
            short aePlineVer[SENSOR_IDX_MAX];
            short strobeVer[SENSOR_IDX_MAX];
            short tsfVer[SENSOR_IDX_MAX];
            short pdcVer[SENSOR_IDX_MAX];
            short geometryVer[SENSOR_IDX_MAX];
            short fovVer[SENSOR_IDX_MAX];
            short featureVer[SENSOR_IDX_MAX];
            short afVer[SENSOR_IDX_MAX];
            short flashCalibrationVer[SENSOR_IDX_MAX];
            short idxTblVer[SENSOR_IDX_MAX];
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_VERSION_FILE_SIZE];
}NVRAM_CAMERA_VERSION_STRUCT;


typedef struct // maximum 2048 bytes
{
    // MFLL
    MUINT8  capture_M;       //capture frame number, default=6, range=2~8, step=1
    MUINT8  blend_N;         //blending frame number, default=6, range=2~8, step=1, blend_frame_number< capture_frame_number

	MUINT32 memc_bad_mv_range;          //default=255
    MUINT32 memc_bad_mv_rate_th;        //default=12707

	MUINT32 me_noise_lv; //noise level for ME algorithm. The value can equal to ISO speed for IMX519 sensor.
	MUINT32 conf_noise_lv; //noise level for ME confidence caculation

	//DSDN related settings
	MUINT8 dsdn_ratio; //The scaling ratio of DSDN (1~16, default 16). dsdn_ratio/16. If dsdn_ratio=16, dsdn is disabled.
	MUINT8 memc_dsus_mode; //The mode of DSDN (called DSUS in ME).

	//AE variation compensation
	MUINT8 aevc_ae_en; //Enable AE variation compensation. 0:disable, 1:enable. default = 1
	MUINT8 aevc_lcso_en; //Enable AE variation compensation for lcso table. 0:disable, 1:enable. default = 1

	//Post refine
	MUINT8 post_refine_en; //Enable post refine. default = 1.
	MUINT8 post_me_refine_en; //Enable post refine parameter by me output. default =1. (if post_refine_en=0, don't care this variable)

    //Global drop frm
    MUINT16 post_me_refine_mv_ratio; // parameter for me output index calculation. range=0~256, default = 128;
	MUINT16 post_me_refine_face_ratio_ThL; // range: 0 ~ 10000, default = 1000
	MUINT16 post_me_refine_face_ratio_ThH; // range: 0 ~ 10000, default = 3200
	MUINT16 post_me_refine_full_ratio_ThL; // range: 0 ~ 10000, default = 1000
	MUINT16 post_me_refine_full_ratio_ThH; // range: 0 ~ 10000, default = 3200
	MUINT16 post_me_refine_edge_ccl_Th[2]; // range: 0 ~ 65535, default = {600, 600}
	MUINT8 post_me_refine_edge_cclnum_Th;  // range: 0 ~   255, default = 3
    MUINT16 post_me_refine_edge_FDAreaThL; // range: 0 ~ 65535, default = 0
	MUINT16 post_me_refine_edge_FDAreaThH; // range: 0 ~ 65535, default = 16384

    //Local decrease
    MUINT8  lcl_deconf_en; //Enable local confidence fallback. 0:disable, 1:enable. default = 0
    MUINT32 lcl_deconf_noise_lv;    //parameter for confidence decrease calculation.
    MUINT16 lcl_deconf_bg_bss_ratio;//parameter for confidence decrease calculation. range=0~256, default = 256;
    MUINT16 lcl_deconf_fd_bss_ratio;//parameter for confidence decrease calculation. range=0~256, default = 243;
    MUINT8 lcl_deconf_dltvar_en;   //parameter for confidence decrease calculation. default = 0

    // reserved space
    MUINT8 ext_setting; // use extension settings of reserved block. disable = 0. default = 0, 1: parameter add to post_refine_int

    ///////////////////////////////////////////////////////
    //extension version 1 Start
    //##### Eye
    MUINT32  Blink_eye_en;
    MUINT32  Blink_eye_ThL;
    MUINT32  Blink_eye_ThH;
    MUINT32  Blink_eye_qstep;
    MUINT32  post_refine_int;
    MUINT32  me_large_mv_thd;
    MUINT32  me_large_mv_SAD_thd;
    MUINT32  me_large_mv_ratio;
    MUINT32  me_large_mv_txtr_en;
    MUINT32  me_large_mv_txtr_wei;
    MUINT32  me_large_mv_txtr_thd;
    //extension version 1 End
    ///////////////////////////////////////////////////////

    // reserved space
    MUINT32 reserved[102];

} NVRAM_CAMERA_FEATURE_MFLL_STRUCT, *PNVRAM_CAMERA_FEATURE_MFLL_STRUCT;

static_assert( sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT) <= 2048,
        "NVRAM_CAMERA_FEATURE_MFLL_STRUCT is greater 2048 bytes, please make sure " \
        "it's smaller 2048 bytes");

#if 0 // debug NVRAM_CAMERA_FEATURE_MFLL_STRUCT structure size usage
template<int s> struct NVRAM_CAMERA_FEATURE_MFLL_STRUCT_SIZE_IS;
NVRAM_CAMERA_FEATURE_MFLL_STRUCT_SIZE_IS
<sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT)> n_______n;
#endif

#define GIS_MAXSUPPORTED_SMODE (20)

typedef struct NVRAM_CAMERA_FEATURE_GIS_STRUCT_t
{
    MUINT32 gis_defWidth;
    MUINT32 gis_defHeight;
    MUINT32 gis_defCrop;
    double   gis_defParameter1[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter2[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter3[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_deftRS[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved1[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved2[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
} NVRAM_CAMERA_FEATURE_GIS_STRUCT, *PNVRAM_CAMERA_FEATURE_GIS_STRUCT;

struct NR_PARAM
{
    MINT32 NR_K;
    MINT32 NR_S;
    MINT32 NR_SD;
    MINT32 NR_BLD_W;
    MINT32 NR_BLD_TH;
    MINT32 NR_SMTH;
    MINT32 NR_NTRL_TH_1_Y;
    MINT32 NR_NTRL_TH_2_Y;
    MINT32 NR_NTRL_TH_1_UV;
    MINT32 NR_NTRL_TH_2_UV;
};

struct HFG_PARAM
{
    MINT32 HFG_ENABLE;
    MINT32 HFG_GSD;
    MINT32 HFG_SD0;
    MINT32 HFG_SD1;
    MINT32 HFG_SD2;
    MINT32 HFG_TX_S;
    MINT32 HFG_LCE_LINK_EN;
    MINT32 HFG_LUMA_CPX1;
    MINT32 HFG_LUMA_CPX2;
    MINT32 HFG_LUMA_CPX3;
    MINT32 HFG_LUMA_CPY0;
    MINT32 HFG_LUMA_CPY1;
    MINT32 HFG_LUMA_CPY2;
    MINT32 HFG_LUMA_CPY3;
    MINT32 HFG_LUMA_SP0;
    MINT32 HFG_LUMA_SP1;
    MINT32 HFG_LUMA_SP2;
    MINT32 HFG_LUMA_SP3;
};

struct CCR_PARAM
{
    MINT32 CCR_ENABLE;
    MINT32 CCR_CEN_U;
    MINT32 CCR_CEN_V;
    MINT32 CCR_Y_CPX1;
    MINT32 CCR_Y_CPX2;
    MINT32 CCR_Y_CPY1;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_X1;
    MINT32 CCR_UV_X2;
    MINT32 CCR_UV_X3;
    MINT32 CCR_UV_GAIN1;
    MINT32 CCR_UV_GAIN2;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_UV_GAIN_SP2;
    MINT32 CCR_Y_CPX3;
    MINT32 CCR_Y_CPY0;
    MINT32 CCR_Y_CPY2;
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP2;
    MINT32 CCR_UV_GAIN_MODE;
    MINT32 CCR_MODE;
    MINT32 CCR_OR_MODE;
    MINT32 CCR_HUE_X1;
    MINT32 CCR_HUE_X2;
    MINT32 CCR_HUE_X3;
    MINT32 CCR_HUE_X4;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;
    MINT32 CCR_HUE_GAIN1;
    MINT32 CCR_HUE_GAIN2;
};

typedef struct
{
    NR_PARAM NR;
    HFG_PARAM HFG;
    CCR_PARAM CCR;
} NVRAM_CAMERA_FEATURE_SWNR_STRUCT;

// ClearZoom
typedef struct
{
    MINT32  ultraResEnable;
    MINT32  IBSE_clip_ratio;
    MINT32  IBSE_clip_thr;
    MINT32  IBSE_gaincontrol_gain;
    MINT32  tapAdaptSlope;
    MINT32  dynIBSE_gain;
} ISP_NVRAM_CLZ_RSZSWREG, *PISP_NVRAM_CLZ_RSZSWREG;

typedef struct
{
    MINT32  IBSE_gaincontrol_coring_value;
    MINT32  IBSE_gaincontrol_coring_thr;
    MINT32  IBSE_gaincontrol_coring_zero;
    MINT32  IBSE_gaincontrol_softlimit_ratio;
    MINT32  IBSE_gaincontrol_bound;
    MINT32  IBSE_gaincontrol_limit;
    MINT32  IBSE_gaincontrol_softcoring_gain;
} ISP_NVRAM_CLZ_RSZHWREG, *PISP_NVRAM_CLZ_RSZHWREG;

typedef struct
{
    MINT32  tdshp_en;
    MINT32  tdshp_gain_mid;
    MINT32  tdshp_gain_high;
    MINT32  tdshp_softcoring_gain;

    MINT32  tdshp_coring_thr;
    MINT32  tdshp_coring_zero;
    MINT32  tdshp_gain;
    MINT32  tdshp_limit_ratio;
    MINT32  tdshp_limit;
    MINT32  tdshp_bound;
    MINT32  tdshp_coring_value;

    MINT32  tdshp_clip_en;
    MINT32  tdshp_clip_ratio;
    MINT32  tdshp_clip_thr;

    MINT32  pbc1_gain;
    MINT32  pbc1_lpf_gain;

    MINT32  pbc2_gain;
    MINT32  pbc2_lpf_gain;

    MINT32  pbc3_gain;
    MINT32  pbc3_lpf_gain;

    MINT32  edf_flat_gain;
    MINT32  edf_detail_gain;
    MINT32  edf_edge_gain;
} ISP_NVRAM_CLZ_DSHWREG, *PISP_NVRAM_CLZ_DSHWREG;

typedef struct
{
    ISP_NVRAM_CLZ_RSZSWREG  RszEntrySWReg;
    ISP_NVRAM_CLZ_RSZHWREG  RszEntryHWReg;
    ISP_NVRAM_CLZ_DSHWREG  iDSHWRegEntry;
} ISP_NVRAM_CLZ_TUNINGREG_T, *PISP_NVRAM_CLZ_TUNINGREG_T;

typedef struct
{
    MINT32  ZOOM[6];
    MINT32  ISO[11];
    MINT32 defaultuptable;
    MINT32 defaultdowntable;
    MINT32 switchratio6tap6ntap;
    MINT32 switchratio6ntapacc;
    ISP_NVRAM_CLZ_TUNINGREG_T   CLZ_TUNING[50];
} ISP_NVRAM_CLEARZOOM_T, *PISP_NVRAM_CLEARZOOM_T;

// CA_LTM
typedef struct{
    MUINT32  ca_ltm_s_lower;
    MUINT32  ca_ltm_s_upper;
    MUINT32  ca_ltm_y_lower;
    MUINT32  ca_ltm_y_upper;
    MUINT32  ca_ltm_h_lower;
    MUINT32  ca_ltm_h_upper;
    MUINT32  ca_ltm_max_hist_mode;
    MUINT32  ca_ltm_bitplus_contour_range_th;
    MUINT32  ca_ltm_bitplus_contour_range_slope;
    MUINT32  ca_ltm_bitplus_diff_count_th;
    MUINT32  ca_ltm_bitplus_diff_count_slope;
    MUINT32  ca_ltm_bitplus_pxl_diff_th;
    MUINT32  ca_ltm_bitplus_pxl_diff_slope;
    MUINT32  ca_ltm_pxl_diff_th_for_flat_pxl;
    MUINT32  ca_ltm_pxl_diff_slope_for_flat_pxl;
    MUINT32  ca_ltm_pxl_diff_th;
    MUINT32  ca_ltm_pxl_diff_slope;
}FEATURE_NVRAM_CA_LTM_INITUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_INITUNINGREG_T;

typedef struct{
    MINT32 ca_ltm_fw_en;
    MINT32 ca_ltm_curve_en;
    MINT32 ca_ltm_gain_flt_en;
    MINT32 ca_ltm_iir_force_range;
    MUINT32 bADLWeight1;
    MUINT32 bADLWeight2;
    MUINT32 bADLWeight3;
    MUINT32 bBSDCGain;
    MUINT32 bBSACGain;
    MUINT32 bBSLevel;
    MUINT32 bMIDDCGain;
    MUINT32 bMIDACGain;
    MUINT32 bWSDCGain;
    MUINT32 bWSACGain;
    MUINT32 bWSLevel;
    MUINT32 ca_ltm_dync_spike_wgt_min;
    MUINT32 ca_ltm_dync_spike_wgt_max;
    MUINT32 ca_ltm_dync_spike_th;
    MUINT32 ca_ltm_dync_spike_slope;
    MUINT32 bSpikeBlendmethod;
    MUINT32 bSkinWgtSlope;
    MUINT32 bSkinBlendmethod;
    MUINT32 ca_ltm_dync_flt_coef_min;
    MUINT32 ca_ltm_dync_flt_coef_max;
    MUINT32 ca_ltm_dync_flt_ovr_pxl_th;
    MUINT32 ca_ltm_dync_flt_ovr_pxl_slope;
    MINT32 LLPValue;
    MINT32 LLPRatio;
    MINT32 APLCompRatioLow;
    MINT32 APLCompRatioHigh;
    MINT32 FltConfSlope;
    MINT32 FltConfTh;
    MINT32 BlkHistCountRatio;
    MINT32 BinIdxDiffSlope;
    MINT32 BinIdxDiffTh;
    MINT32 BinIdxDiffWgtOft;
    MINT32 APLTh;
    MINT32 APLSlope;
    MINT32 APLWgtOft;
    MINT32 APL2Th;
    MINT32 APL2Slope;
    MINT32 APL2WgtOft;
    MINT32 APL2WgtMax;
    MINT32 BlkSpaFltEn;
    MINT32 BlkSpaFltType;
    MINT32 LoadBlkCurveEn;
    MINT32 SaveBlkCurveEn;
    MINT32 bGreenWgtSlope;
    MINT32 dre_dync_compact_wgt_min;
    MINT32 dre_dync_compact_wgt_max;
    MINT32 dre_dync_compact_th;
    MINT32 dre_dync_compact_slope;
    MINT32 bCompactBlendmethod;
    MINT32 dre_dync_compact_LowBoundBin;
    MINT32 dre_dync_compact_HighBoundBin;
    MINT32 dre_dync_compact_BinMinWeight;
    MINT32 dre_dync_compact_BinMaxWeight;
}FEATURE_NVRAM_CA_LTM_TUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_TUNINGREG_T;

typedef struct{
    MINT32 Enabled;
    MINT32 Strength;
    MINT32 AdaptiveMethod;
    MINT32 AdaptiveType;
    MINT32 CustomParametersSearchMode;
}FEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T;

typedef struct{
    FEATURE_NVRAM_CA_LTM_INITUNINGREG_T CA_LTM_INITUNINGRE;
    FEATURE_NVRAM_CA_LTM_TUNINGREG_T CA_LTM_TUNINGREG;
    FEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T CA_LTM_ADAPTTUNINGREG;
}FEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T;

typedef struct{
    MINT32 ISO[16];
    MINT32 LV[16];
    FEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T CA_LTM_ALLTUNINGREG[15];
} FEATURE_NVRAM_CA_LTM_T, *PFEATURE_NVRAM_CA_LTM_T;

typedef struct{
    // HFG reg
    MUINT32 HFG_NOISE_GSEED;
    MUINT32 HFG_IMAGE_WD;
    MUINT32 HFG_NOISE_SEED0;
    MUINT32 HFG_NOISE_SEED1;
    MUINT32 HFG_NOISE_SEED2;

    // HFC reg
    MUINT32 HFC_NOISE_STD;
    MUINT32 HFC_SL2_LINK_EN;
    MUINT32 HFC_LUMA_CPX1;
    MUINT32 HFC_LUMA_CPX2;
    MUINT32 HFC_LUMA_CPX3;
    MUINT32 HFC_LUMA_CPY0;
    MUINT32 HFC_LUMA_CPY1;
    MUINT32 HFC_LUMA_CPY2;
    MUINT32 HFC_LUMA_CPY3;
    MUINT32 HFC_LUMA_SP0;
    MUINT32 HFC_LUMA_SP1;
    MUINT32 HFC_LUMA_SP2;
    MUINT32 HFC_LUMA_SP3;
    MUINT32 HFC_SL2_CPX1;
    MUINT32 HFC_SL2_CPX2;
    MUINT32 HFC_SL2_CPX3;
    MUINT32 HFC_SL2_CPY0;
    MUINT32 HFC_SL2_CPY1;
    MUINT32 HFC_SL2_CPY2;
    MUINT32 HFC_SL2_CPY3;
    MUINT32 HFC_SL2_SP0;
    MUINT32 HFC_SL2_SP1;
    MUINT32 HFC_SL2_SP2;
    MUINT32 HFC_SL2_SP3;
}FEATURE_NVRAM_HFG_TUNINGREG_T, *PFEATURE_NVRAM_HFG_TUNINGREG_T;

typedef struct{
    FEATURE_NVRAM_HFG_TUNINGREG_T HFG_TUNINGREG;
} FEATURE_NVRAM_HFG_T, *PFEATURE_NVRAM_HFG_T;

typedef struct{
    MUINT32 dsdn_dividend;
    MUINT32 dsdn_divisor;
    MINT32  downscale_thres;
} FEATURE_NVRAM_DSDN_T, *PFEATURE_NVRAM_DSDN_T;

typedef struct
{
    MINT32 x;
    MINT32 downscale_thres;
    MINT32 downscale_ratio;
} NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT, *PNVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT;

typedef struct
{
    MINT32 iso_th;
    MINT32 ext_setting;
    MINT32 reserved;
} NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT, *PNVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT;

typedef struct
{
// AINR
    MUINT32 chi_R;
    MUINT32 chi_GR;
    MUINT32 chi_GB;
    MUINT32 chi_B;
    MUINT32 std_R;
    MUINT32 std_GR;
    MUINT32 std_GB;
    MUINT32 std_B;
    MUINT32 AINR_blend_R;
    MUINT32 AINR_blend_GR;
    MUINT32 AINR_blend_GB;
    MUINT32 AINR_blend_B;
    MUINT32 MIX_blend_Y;
    MUINT32 MIX_blend_UV;
    MUINT32 Lambda;
    // reserved space
    MUINT32 rsv1;
    MUINT32 rsv2;
    MUINT32 rsv3;
    MUINT32 rsv4;
    MUINT32 rsv5;
    MUINT32 rsv6;
    MUINT32 rsv7;
} FEATURE_NVRAM_AINR_T, *PFEATURE_NVRAM_AINR_T;

typedef struct
{
    // Gives N, for capturing N frames
    MUINT32 capture_frame_number;
     
    // The threshold of doing ainr    
    MUINT32 ainr_iso_th;
    
    // iso range for adaptive frame number mechanism 
    MUINT32 iso_level1; 
    MUINT32 iso_level2;
    MUINT32 iso_level3;
    MUINT32 iso_level4;
    MUINT32 iso_level5;
    MUINT32 iso_level6;
    MUINT32 iso_level7;
    MUINT32 iso_level8;
    
    // blend frame number
    MUINT32 frame_num1; 
    MUINT32 frame_num2;
    MUINT32 frame_num3;
    MUINT32 frame_num4;
    MUINT32 frame_num5;
    MUINT32 frame_num6;
    MUINT32 frame_num7;
    MUINT32 frame_num8;
} NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT, *PNVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT;

typedef struct
{
	//basic setting
	MUINT8 bss_ver; //bss version. default=3
	MUINT8 scale_factor; //bss scaling factor. default = 8.
	MUINT8 clip_th0; //low clip threshold for image. default = 10.
	MUINT8 clip_th1; //high clip threshold for image. default = 20.
	MUINT8 clip_th2; //low clip threshold for face. default = 10.
	MUINT8 clip_th3; //high clip threshold for face. default = 20.
	MUINT8 zero_gmv; //the threshold for zero gmv. default = 12.
	MUINT8 adf_th; //The adf treshold for drop frame. default = 18.
	MUINT8 sdf_th; //The sdf threshold for drop frame. default = 80.

	//YPF
	MUINT8 ypf_en; //enable ypf. 0:disable. 1: enable. default = 1
	MUINT8 ypf_fac; //default = 50
	MUINT8 ypf_adj_th; //default = 12
	MUINT8 ypf_dfmed0; //default = 20
	MUINT8 ypf_dfmed1; //default = 32
	MUINT8 ypf_th[8]; //default {102,104,98,96,96,96,96,96}

	//FD
	MUINT8 fd_en; //enable face scoring. 0:disable. 1: enable. default = 1
	MUINT8 fd_fac; //default = 2
	MUINT8 fd_fnum; //default = 1

	//EYE blinking
	MUINT8 eye_en; //enable eye blinking scoring. 0:disable. 1: enable. default = 1
	MUINT8 eye_cfth; //default = 60
	MUINT8 eye_ratio0; //default = 60
	MUINT8 eye_ratio1; //default = 40
	MUINT8 eye_fac; //default = 50

	// BSS 3.0
	MUINT8 FaceCVTh;        // range: 0 ~   100, default = 25
	MUINT8 GradThL;         // range: 0 ~   255, default = 0
	MUINT8 GradThH;         // range: 0 ~   255, default = 128
	MUINT16 FaceAreaThL[2]; // range: 0 ~ 65535, default = {0, 0}
	MUINT16 FaceAreaThH[2]; // range: 0 ~ 65535, default = {16384, 32768}
	MUINT16 APLDeltaTh[33]; // range: 0 ~  4095, default curve is below:
	                        //                   {20, 20, 20, 20, 20, 30, 40, 50, 60, 70, 80, 95,110,130,150,170,190,
							//                       210,230,260,290,320,320,320,320,320,320,320,320,320,320,320,320}
	MUINT16 GradRatioTh[8]; // range: 0 ~ 10000, default curve: {5000, 4000, 3000, 2500, 2000, 1000, 1000, 1000}
	MUINT8 EyeDistThL;      // range: 0 ~   255, default = 0
	MUINT8 EyeDistThH;      // range: 0 ~   255, default = 35
	MUINT16 EyeMinWeight;   // range: 0 ~ 10000, default = 5000

    // reserved space
	MUINT8 ext_setting; // use extension settings of reserved block. disable = 0. default = 0
    MUINT32 reserved[71];
} FEATURE_NVRAM_BSS_T, *PFEATURE_NVRAM_BSS_T;

typedef struct NVRAM_CAMERA_FEATURE_STRUCT_t
{
    NVRAM_CAMERA_FEATURE_GIS_STRUCT         gis;
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT        SWNR[NVRAM_SWNR_TBL_NUM];
    NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT  SWNR_THRES[NVRAM_SWNR_THRES_TBL_NUM];
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT        MFNR[NVRAM_MFNR_TBL_NUM];
    NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT  MFNR_THRES[NVRAM_MFNR_THRES_TBL_NUM];
    ISP_NVRAM_CLEARZOOM_T                   ClearZoom[NVRAM_ClearZoom_TBL_NUM];
    FEATURE_NVRAM_DSDN_T                    DSDN[NVRAM_DSDN_TBL_NUM];
    FEATURE_NVRAM_CA_LTM_T                  CA_LTM[NVRAM_CA_LTM_TBL_NUM];
    FEATURE_NVRAM_HFG_T                     HFG[NVRAM_HFG_TBL_NUM];
    FEATURE_NVRAM_AINR_T                    AINR[NVRAM_AINR_TBL_NUM];
    NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT  AINR_THRES[NVRAM_AINR_THRES_TBL_NUM];
    FEATURE_NVRAM_BSS_T                     BSS[NVRAM_BSS_TBL_NUM];
} NVRAM_CAMERA_FEATURE_STRUCT, *PNVRAM_CAMERA_FEATURE_STRUCT;

static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT) <= MAXIMUM_NVRAM_CAMERA_FEATURE_FILE_SIZE,
        "nvram feature size is not enough"
        );

typedef union
{
    struct
    {
        MUINT32 StereoData[MTK_STEREO_KERNEL_NVRAM_LENGTH];
        float DepthAfData[ (MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE - MTK_STEREO_KERNEL_NVRAM_LENGTH * sizeof(MUINT32) ) / sizeof(float) ];
    } StereoNvramData;

    UINT8   Data[MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE];
}NVRAM_CAMERA_GEOMETRY_STRUCT;

typedef struct NVRAM_CAMERA_FOV_STRUCT_t
{
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE];
} NVRAM_CAMERA_FOV_STRUCT, *PNVRAM_CAMERA_FOV_STRUCT;

static_assert(
        sizeof(NVRAM_CAMERA_FOV_STRUCT) <= MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE,
        "nvram fov size is not enough"
        );

#endif // _CAMERA_CUSTOM_NVRAM_H_

