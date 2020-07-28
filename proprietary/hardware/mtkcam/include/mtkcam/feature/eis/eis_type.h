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

/**
* @file EIS_Type.h
*
* EIS Type Header File
*/


#ifndef _EIS_TYPE_H_
#define _EIS_TYPE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/fsc/fsc_ext.h>

/*********************************************************************
* Define Value
*********************************************************************/

// EIS algo compile option
#define EIS_ALGO_READY (1)

//#if EIS_ALGO_READY
#if 1
#include <MTKGyro.h>
#include <MTKEis.h>
#include <MTKEisPlus.h>

#define EIS_MAX_WIN_NUM EIS_WIN_NUM

#else

#define EIS_MAX_WIN_NUM 32  // use when EIS algo not ready

#endif

#define CIF_WIDTH 352
#define CIF_HEIGHT 320

#define D1_WIDTH 792
#define D1_HEIGHT 528

#define HD_720P_WIDTH 1536
#define HD_720P_HEIGHT 864

#define HD_1080P_WIDTH 2112
#define HD_1080P_HEIGHT 1188

#define HD_8M_WIDTH 3264
#define HD_8M_HEIGHT 2200

#define HD_12M_WIDTH 4000
#define HD_12M_HEIGHT 3000

#define HD_16M_WIDTH 4608
#define HD_16M_HEIGHT 3456

#define HD_20M_WIDTH 5164
#define HD_20M_HEIGHT 3872

#define ALIGN_SIZE(in,align) (in & ~(align-1))


#define EIS_FE_MAX_INPUT_W  (2400)
#define EIS_FE_MAX_INPUT_H  (1360)

#define ALGOPT_FHD_THR_W    (2400)
#define ALGOPT_FHD_THR_H    (1800)

#define EIS_EISO_SYNC_FAIL -99

#define EISO_MEMORY_SIZE (256)    // 32 * 64 (bits) = 256 bytes

/*********************************************************************
* ENUM
*********************************************************************/

/**
*@brief Return enum of EIS
*/
typedef enum
{
    EIS_RETURN_NO_ERROR         = 0,       //! The function work successfully
    EIS_RETURN_UNKNOWN_ERROR    = 0x0001,  //! Unknown error
    EIS_RETURN_INVALID_DRIVER   = 0x0002,  //! invalid driver object
    EIS_RETURN_API_FAIL         = 0x0003,  //! api fail
    EIS_RETURN_INVALID_PARA     = 0x0004,  //! invalid parameter
    EIS_RETURN_NULL_OBJ         = 0x0005,  //! null object
    EIS_RETURN_MEMORY_ERROR     = 0x0006,  //! memory error
    EIS_RETURN_EISO_MISS        = 0x0007   //! EISO data missed
}EIS_ERROR_ENUM;

typedef enum
{
    LMV_RETURN_NO_ERROR         = 0,       //! The function work successfully
    LMV_RETURN_UNKNOWN_ERROR    = 0x0001,  //! Unknown error
    LMV_RETURN_INVALID_DRIVER   = 0x0002,  //! invalid driver object
    LMV_RETURN_API_FAIL         = 0x0003,  //! api fail
    LMV_RETURN_INVALID_PARA     = 0x0004,  //! invalid parameter
    LMV_RETURN_NULL_OBJ         = 0x0005,  //! null object
    LMV_RETURN_MEMORY_ERROR     = 0x0006,  //! memory error
    LMV_RETURN_EISO_MISS        = 0x0007   //! EISO data missed
}LMV_ERROR_ENUM;


/**
*@brief Sensor type
*/
typedef enum
{
    EIS_RAW_SENSOR,
    EIS_YUV_SENSOR,
    EIS_JPEG_SENSOR,
    EIS_NULL_SENSOR
}EIS_SENSOR_ENUM;

/**
*@brief EIS scenario enum
*/
typedef enum
{
    EIS_SCE_EIS,
    EIS_SCE_EIS_PLUS,
    EIS_SCE_MFBLL,      // multi-frame lowlight
    EIS_SCE_MT,         // motion track
    EIS_SCE_MAV,
    EIS_SCE_NONE,
    EIS_SCE_NUM
}EIS_SCENARIO_ENUM;

/**
*@brief EIS configure scenario enum
*/
typedef enum
{
    EIS_PASS_1,
    EIS_PASS_2
}EIS_PASS_ENUM;

/**
*@brief DMA PORT
*/
typedef enum
{
    EIS_DMA_EISO = 0,
    EIS_DMA_FEO
}EIS_DMA_ENUM;

/**
*@brief Flush enum
*/
typedef enum
{
    EIS_FLUSH_HW = 0,
    EIS_FLUSH_SW
}EIS_FLUSH_ENUM;

/**
*@brief EIS software state enum
*/
typedef enum EIS_SW_STATE
{
    EIS_SW_STATE_NONE = 0,
    EIS_SW_STATE_ALIVE,
    EIS_SW_STATE_UNINIT,
    EIS_SW_STATE_UNINIT_CHECK
}EIS_SW_STATE_ENUM;

/**
*@brief EIS command enum
*/
typedef enum EIS_CMD
{
    EIS_CMD_SET_STATE = 0,
    EIS_CMD_CHECK_STATE
}EIS_CMD_ENUM;

/*********************************************************************
* Struct
*********************************************************************/

/**
*@brief  EIS statistic data structure
*/
typedef struct
{
    MINT32 i4LMV_X[EIS_MAX_WIN_NUM];
    MINT32 i4LMV_Y[EIS_MAX_WIN_NUM];

    MINT32 i4LMV_X2[EIS_MAX_WIN_NUM];
    MINT32 i4LMV_Y2[EIS_MAX_WIN_NUM];

    MUINT32 NewTrust_X[EIS_MAX_WIN_NUM];
    MUINT32 NewTrust_Y[EIS_MAX_WIN_NUM];

    MUINT32 SAD[EIS_MAX_WIN_NUM];
} EIS_STATISTIC_T;

/**
  * @brief EIS config structure
  *
*/
typedef struct EIS_HAL_CONFIG
{
    MUINT32 imgiW;          // DMA IMGI input width,use in pass2 configure
    MUINT32 imgiH;          // DMA IMGI input height,use in pass2 configure
    MUINT32 crzOutW;        // use in pass2 configure
    MUINT32 crzOutH;        // use in pass2 configure
    MUINT32 srzOutW;        // use in pass2 configure
    MUINT32 srzOutH;        // use in pass2 configure
    MUINT32 rssoWidth;
    MUINT32 rssoHeight;
    MUINT32 feTargetW;      // use in pass2 configure
    MUINT32 feTargetH;      // use in pass2 configure
    MUINT32 gpuTargetW;     // use in pass2 configure
    MUINT32 gpuTargetH;     // use in pass2 configure
    MUINT32 cropX;          // MW crop setting of X,use in pass2 configure
    MUINT32 cropY;          // MW crop setting of Y,use in pass2 configure
    MINT32 gmv_X;
    MINT32 gmv_Y;
    MINT32 confX;
    MINT32 confY;
    MINT32 gmv_exist;
    MUINT32 sensorIdx;
    MUINT32 process_mode;
    MUINT32 process_idx;

    MINT32  sensor_Width;     //sensor
    MINT32  sensor_Height;    //sensor
    MINT32  rrz_crop_Width;   //sensor crop
    MINT32  rrz_crop_Height;  //sensor crop
    MINT32  rrz_crop_X;
    MINT32  rrz_crop_Y;
    MINT32  rrz_scale_Width;  //RRZ output
    MINT32  rrz_scale_Height; //RRZ output
    MINT32  is_multiSensor;
    MINT32  fov_wide_idx;
    MINT32  fov_tele_idx;
    MINT32  fov_align_Width; //fov aligned output width
    MINT32  fov_align_Height;//fov aligned output height
    MINT32  warp_precision;
    COORDINATE warp_grid[4]; //warp's gird (4 points: x0,y0 x1,y1)
                             //            (          x2,y2 x3,y3)

    MUINT32 vHDREnabled;
    MUINT32 lmvDataEnabled;
    EIS_STATISTIC_STRUCT*  lmv_data;
    //
    EIS_HAL_CONFIG()
    : imgiW(0)
    , imgiH(0)
    , crzOutW(0)
    , crzOutH(0)
    , srzOutW(0)
    , srzOutH(0)
    , rssoWidth(0)
    , rssoHeight(0)
    , feTargetW(0)
    , feTargetH(0)
    , gpuTargetW(0)
    , gpuTargetH(0)
    , cropX(0)
    , cropY(0)
    , gmv_X(0)
    , gmv_Y(0)
    , confX(0)
    , confY(0)
    , gmv_exist(0)
    , sensorIdx(0)
    , process_mode(0)
    , process_idx(0)
    , sensor_Width(0)
    , sensor_Height(0)
    , rrz_crop_Width(0)
    , rrz_crop_Height(0)
    , rrz_crop_X(0)
    , rrz_crop_Y(0)
    , rrz_scale_Width(0)
    , rrz_scale_Height(0)
    , is_multiSensor(0)
    , fov_wide_idx(0)
    , fov_tele_idx(0)
    , fov_align_Width(0)
    , fov_align_Height(0)
    , warp_precision(5)
    , vHDREnabled(0)
    , lmvDataEnabled(0)
    , lmv_data(NULL)
    {
        warp_grid[0].x = 0;
        warp_grid[0].y = 0;
        warp_grid[1].x = 0;
        warp_grid[1].y = 0;
        warp_grid[2].x = 0;
        warp_grid[2].y = 0;
        warp_grid[3].x = 0;
        warp_grid[3].y = 0;
    }
}EIS_HAL_CONFIG_DATA;


typedef struct LMV_HAL_CONFIG
{
    MUINT32 sensorType;     // use in pass1
    MUINT32 p1ImgW;         // use in pass1
    MUINT32 p1ImgH;         // use in pass1
    EIS_STATISTIC_T lmvData;

    //
    LMV_HAL_CONFIG()
    : sensorType(0)
    , p1ImgW(0)
    , p1ImgH(0)
    {
        for(MINT32 i=0; i<EIS_MAX_WIN_NUM; i++)
        {
            lmvData.i4LMV_X[i]=0;
            lmvData.i4LMV_Y[i]=0;
            lmvData.i4LMV_X2[i]=0;
            lmvData.i4LMV_Y2[i]=0;
            lmvData.NewTrust_X[i]=0;
            lmvData.NewTrust_Y[i]=0;
            lmvData.SAD[i]=0;
        }
    }
}LMV_HAL_CONFIG_DATA;

/**
  *@brief FEO register info
*/
#define MULTISCALE_FEFM (3)

typedef struct MULTISCALE_INFO_T
{
    MINT32 MultiScale_width[MULTISCALE_FEFM];
    MINT32 MultiScale_height[MULTISCALE_FEFM];
    MINT32 MultiScale_blocksize[MULTISCALE_FEFM];
}MULTISCALE_INFO;

typedef struct FEFM_PACKAGE_T
{
    MUINT16*  FE[MULTISCALE_FEFM];
    MUINT16*  LastFE[MULTISCALE_FEFM];

    MUINT16*  ForwardFM[MULTISCALE_FEFM];
    MUINT16*  BackwardFM[MULTISCALE_FEFM];
    MUINT32 ForwardFMREG[MULTISCALE_FEFM];
    MUINT32 BackwardFMREG[MULTISCALE_FEFM];

    FEFM_PACKAGE_T(MUINT32  _FMRegValue = 0)
    {
        MUINT32 i;
        for (i=0;i <MULTISCALE_FEFM;i++)
        {
            FE[i] =0;
            LastFE[i] =0;
            ForwardFM[i] =0;
            BackwardFM[i] =0;
            ForwardFMREG[i] = _FMRegValue;
            BackwardFMREG[i] = _FMRegValue;
        }
    }
}FEFM_PACKAGE;

typedef struct RSCME_PACKAGE_T
{
    MUINT8*  RSCME_mv;
    MUINT8*  RSCME_var;

    RSCME_PACKAGE_T()
    : RSCME_mv(0)
    , RSCME_var(0)
    {
    }
}RSCME_PACKAGE;

typedef struct LMV_PACKAGE_T
{
    MUINT32 enabled;
    EIS_STATISTIC_STRUCT data;

    LMV_PACKAGE_T(): enabled(0)
    {
        memset(&data, 0, sizeof(EIS_STATISTIC_STRUCT));
    }
} LMV_PACKAGE;

typedef struct FSC_INFO_T
{
    MINT32 numSlices;
    MBOOL isEnabled;
    FSC_INFO_T()
    : numSlices(0)
    , isEnabled(MFALSE)
    {
    }
}FSC_INFO;

typedef struct FSC_PACKAGE_T
{
    MINT32 procWidth;
    MINT32 procHeight;
    MINT32* scalingFactor;

    FSC_PACKAGE_T()
    : procWidth(0)
    , procHeight(0)
    , scalingFactor(NULL)
    {
    }
}FSC_PACKAGE;

typedef struct IMAGE_BASED_DATA_T
{
    FEFM_PACKAGE*   fefmData;
    RSCME_PACKAGE*  rscData;
    LMV_PACKAGE*    lmvData;
    FSC_PACKAGE*    fscData;

    IMAGE_BASED_DATA_T(): fefmData(NULL), rscData(NULL), lmvData(NULL), fscData(NULL)
    {}
} IMAGE_BASED_DATA;


/**
  * @brief EIS Plus GPU Info
  *
*/
typedef struct EIS_PLUS_GPU
{
    MUINT32 gridW;
    MUINT32 gridH;
}EIS_PLUS_GPU_INFO;

/**
  * @brief EIS Pass1 Result
  *
*/
typedef struct EIS_P1_RESULT
{
    MUINT32 DoEisCount;    //Vent@20140427: Add for EIS GMV Sync Check.
    MUINT32 cmvX_Int;
    MUINT32 cmvX_Flt;
    MUINT32 cmvY_Int;
    MUINT32 cmvY_Flt;
    MINT32 gmvX;
    MINT32 gmvY;
    MINT32 gmvConfidX;
    MINT32 gmvConfidY;
    MINT64 timeStamp;
}EIS_P1_RESULT_INFO;

#endif // _EIS_TYPE_H_

