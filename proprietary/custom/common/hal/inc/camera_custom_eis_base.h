/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _EIS_CUSTOM_BASE_H_
#define _EIS_CUSTOM_BASE_H_

#include <cutils/properties.h>

typedef int          MINT32;
typedef unsigned int MUINT32;
typedef float        MFLOAT;
typedef int          MBOOL;
typedef double       MDOUBLE;

#define EIS_DEFAULT_NONE_FACTOR         (100)
#define EIS_DEFAULT_FACTOR              (120)
#define EIS_DEFAULT_FHD_FACTOR          (125)

#define FWEIS_DEFAULT_START_FRAME        (18)
#define FWEIS_DEFAULT_MODE                (1)
#define FWEIS_DEFAULT_FRAMES             (25)
#define FWEIS_DEFAULT_FRAMES_4K          (25)
#define FWEIS_DEFAULT_4K_VR_FPS          (24)

#define EIS_DEFAULT_MV_WIDTH             (48)
#define EIS_DEFAULT_MV_HEIGHT            (27)

#define VAR_EIS_MV_WIDTH      "vendor.debug.eis.mvwidth"
#define VAR_EIS_MV_HEIGHT     "vendor.debug.eis.mvheight"
#define VAR_EIS_CUSTOM_FACTOR "vendor.debug.eis.factor"
#define EIS_VARIABLE_FPS      "vendor.debug.eis.variablefps"
#define EIS_FORCE_GYRO_ONLY   "vendor.debug.eis.gyroonly"
#define EIS_FORCE_IMAGE_ONLY  "vendor.debug.eis.imageonly"
#define EIS_GIS_SENSOR_WIDTH  "vendor.debug.eis.sensorwidth"
#define EIS_GIS_SENSOR_HEIGHT "vendor.debug.eis.sensorheight"
#define EIS_GIS_FOCAL_LENGTH  "vendor.debug.eis.fl"
#define EIS_GIS_TOFFSET       "vendor.debug.eis.toffset"
#define EIS_FORCE_PREVIEW_OFF "vendor.debug.eis.previewoff"

enum EIS_MODE
{
    EIS_MODE_OFF = 0,
    EIS_MODE_CALIBRATION,
    EIS_MODE_EIS_12,
    EIS_MODE_EIS_22,//GIS
    EIS_MODE_EIS_25,
    EIS_MODE_EIS_30,
    EIS_MODE_GYRO,
    EIS_MODE_IMAGE,
    EIS_MODE_EIS_QUEUE,
    EIS_MODE_EIS_DEJELLO,
};


#define EIS_MODE_ENABLE_CALIBRATION(x)           (x)|=(1<<EIS_MODE_CALIBRATION)
#define EIS_MODE_IS_CALIBRATION_ENABLED(x)       ((x& (1<<EIS_MODE_CALIBRATION))?true:false)

#define EIS_MODE_ENABLE_EIS_12(x)                (x)|=(1<<EIS_MODE_EIS_12)
#define EIS_MODE_IS_EIS_12_ENABLED(x)            ((x& (1<<EIS_MODE_EIS_12))?true:false)

#define EIS_MODE_ENABLE_EIS_22(x)                (x)|=(1<<EIS_MODE_EIS_22)
#define EIS_MODE_IS_EIS_22_ENABLED(x)            ((x& (1<<EIS_MODE_EIS_22))?true:false)

#define EIS_MODE_ENABLE_EIS_25(x)                (x)|=(1<<EIS_MODE_EIS_25)
#define EIS_MODE_IS_EIS_25_ENABLED(x)            ((x& (1<<EIS_MODE_EIS_25))?true:false)

#define EIS_MODE_ENABLE_EIS_30(x)                (x)|=(1<<EIS_MODE_EIS_30)
#define EIS_MODE_IS_EIS_30_ENABLED(x)            ((x& (1<<EIS_MODE_EIS_30))?true:false)

#define EIS_MODE_ENABLE_EIS_GYRO(x)              (x)|=(1<<EIS_MODE_GYRO)
#define EIS_MODE_IS_EIS_GYRO_ENABLED(x)          ((x& (1<<EIS_MODE_GYRO))?true:false)

#define EIS_MODE_ENABLE_EIS_IMAGE(x)             (x)|=(1<<EIS_MODE_IMAGE)
#define EIS_MODE_IS_EIS_IMAGE_ENABLED(x)         ((x& (1<<EIS_MODE_IMAGE))?true:false)

#define EIS_MODE_ENABLE_EIS_QUEUE(x)             (x)|=(1<<EIS_MODE_EIS_QUEUE)
#define EIS_MODE_IS_EIS_QUEUE_ENABLED(x)         ((x& (1<<EIS_MODE_EIS_QUEUE))?true:false)

#define EIS_MODE_ENABLE_EIS_DEJELLO(x)           (x)|=(1<<EIS_MODE_EIS_DEJELLO)
#define EIS_MODE_IS_EIS_DEJELLO_ENABLED(x)       ((x& (1<<EIS_MODE_EIS_DEJELLO))?true:false)

#define EIS_MODE_IS_EIS_ADVANCED_ENABLED(x)      ( \
        EIS_MODE_IS_EIS_22_ENABLED(x) || \
        EIS_MODE_IS_EIS_25_ENABLED(x) || \
        EIS_MODE_IS_EIS_30_ENABLED(x) )

enum Customize_EIS_SENSI
{
    CUSTOMER_EIS_SENSI_LEVEL_HIGH = 0,
    CUSTOMER_EIS_SENSI_LEVEL_NORMAL = 1,
    CUSTOMER_EIS_SENSI_LEVEL_ADVTUNE = 2
};

enum Customize_EIS_VOTE_METHOD_ENUM
{
    ABSOLUTE_HIST,
    SMOOTH_HIST
};

enum Customize_WARP_METHOD_ENUM
{
    EIS_WARP_METHOD_6_COEFF = 0,        // 6 coefficient
    EIS_WARP_METHOD_4_COEFF = 1,        // 4 coefficient
    EIS_WARP_METHOD_6_4_ADAPTIVE = 2,   // 6/4 adaptive (default)
    EIS_WARP_METHOD_2_COEFF = 3         //  2 coefficient
};


struct EIS_Customize_Para_t
{
    Customize_EIS_SENSI sensitivity;
    MUINT32 filter_small_motion;    // 0 or 1
    MUINT32 adv_shake_ext;          // 0 or 1
    MFLOAT  stabilization_strength; // 0.5~0.95
    MUINT32 new_tru_th;             // 0~100
    MUINT32 vot_th;                 // 1~16
    MUINT32 votb_enlarge_size;      // 0~1280
    MUINT32 min_s_th;               // 10~100
    MUINT32 vec_th;                 // 0~11   should be even
    MUINT32 spr_offset;             //0 ~ MarginX/2
    MUINT32 spr_gain1;              // 0~127
    MUINT32 spr_gain2;              // 0~127
    MUINT32 gmv_pan_array[4];       //0~5
    MUINT32 gmv_sm_array[4];        //0~5
    MUINT32 cmv_pan_array[4];       //0~5
    MUINT32 cmv_sm_array[4];        //0~5

    Customize_EIS_VOTE_METHOD_ENUM vot_his_method; //0 or 1
    MUINT32 smooth_his_step; // 2~6
    MUINT32 eis_debug;
};

struct EIS_PLUS_Customize_Para_t
{
    Customize_WARP_METHOD_ENUM warping_mode;
    MINT32 search_range_x;      // 32~64
    MINT32 search_range_y;      // 32~64
    MINT32 crop_ratio;          // 10~40
    MINT32 gyro_still_time_th; // 1~20  ,default :6
    MINT32 gyro_max_time_th; // 8~52 ,default : 14
    MINT32 gyro_similar_th; // 0~100 ,default : 77
    MFLOAT stabilization_strength;  // 0.5~0.95
};

struct EIS25_Customize_Tuning_Para_t
{
    MBOOL en_dejello;
    MFLOAT stabilization_strength;
    MINT32 stabilization_level;
    MFLOAT gyro_still_mv_th;
    MFLOAT gyro_still_mv_diff_th;
};

struct EIS30_Customize_Tuning_Para_t
{
    MFLOAT stabilization_strength;
    MINT32 stabilization_level;
    MFLOAT gyro_still_mv_th;
    MFLOAT gyro_still_mv_diff_th;
};

class EISCustomBase
{
protected:
    // DO NOT create instance
    EISCustomBase()
    {
    }

public:

    enum USAGE_MASK
    {
        USAGE_MASK_NONE      = 0x00,
        USAGE_MASK_VHDR      = 0x01,
        USAGE_MASK_4K2K      = 0x02,
        USAGE_MASK_DUAL_ZOOM = 0x04,
        USAGE_MASK_MULTIUSER = 0x08,
        USAGE_MASK_FSC       = 0x10,
    };

    enum VIDEO_CFG
    {
        VIDEO_CFG_FHD,
        VIDEO_CFG_4K2K,
    };

    /* Get EIS Mode
     */
    static MUINT32 getEISMode(MUINT32 mask)
    {
        (void)mask;
        return EIS_MODE_OFF;
    }

    /* EIS customized data
     */
    static void getEISData(EIS_Customize_Para_t *a_pDataOut)
    {
        a_pDataOut->sensitivity   = CUSTOMER_EIS_SENSI_LEVEL_ADVTUNE;
        a_pDataOut->filter_small_motion = 0;    // 0 or 1
        a_pDataOut->adv_shake_ext = 1;          // 0 or 1
        a_pDataOut->stabilization_strength = 0.9;   // 0.5~0.95
        a_pDataOut->new_tru_th = 25;            // 0~100
        a_pDataOut->vot_th = 4;                 // 1~16
        a_pDataOut->votb_enlarge_size = 0;      // 0~1280
        a_pDataOut->min_s_th = 40;              // 10~100
        a_pDataOut->vec_th = 0;                 // 0~11   should be even
        a_pDataOut->spr_offset = 0;             // 0 ~ MarginX/2
        a_pDataOut->spr_gain1 = 0;              // 0~127
        a_pDataOut->spr_gain2 = 0;              // 0~127
        a_pDataOut->gmv_pan_array[0] = 0;       // 0~5
        a_pDataOut->gmv_pan_array[1] = 0;       // 0~5
        a_pDataOut->gmv_pan_array[2] = 0;       // 0~5
        a_pDataOut->gmv_pan_array[3] = 1;       // 0~5

        a_pDataOut->gmv_sm_array[0] = 0;    //0~5
        a_pDataOut->gmv_sm_array[1] = 0;    //0~5
        a_pDataOut->gmv_sm_array[2] = 0;    //0~5
        a_pDataOut->gmv_sm_array[3] = 1;    //0~5

        a_pDataOut->cmv_pan_array[0] = 0;   //0~5
        a_pDataOut->cmv_pan_array[1] = 0;   //0~5
        a_pDataOut->cmv_pan_array[2] = 0;   //0~5
        a_pDataOut->cmv_pan_array[3] = 1;   //0~5

        a_pDataOut->cmv_sm_array[0] = 0;    //0~5
        a_pDataOut->cmv_sm_array[1] = 1;    //0~5
        a_pDataOut->cmv_sm_array[2] = 2;    //0~5
        a_pDataOut->cmv_sm_array[3] = 4;    //0~5

        a_pDataOut->vot_his_method = ABSOLUTE_HIST; //ABSOLUTE_HIST or SMOOTH_HIST
        a_pDataOut->smooth_his_step = 3; // 2~6
        a_pDataOut->eis_debug = 0;
    }

    static void getEISPlusData(EIS_PLUS_Customize_Para_t *a_pDataOut, MUINT32 config)
    {
        a_pDataOut->warping_mode = EIS_WARP_METHOD_6_4_ADAPTIVE;
        a_pDataOut->search_range_x = 64;        // 32~64
        a_pDataOut->search_range_y = 64;        // 32~64
        a_pDataOut->crop_ratio     = getEISFactor(config) - EIS_DEFAULT_NONE_FACTOR; // 10~40
        a_pDataOut->gyro_still_time_th = 0;
        a_pDataOut->gyro_max_time_th   = 0;
        a_pDataOut->gyro_similar_th    = 0;
        a_pDataOut->stabilization_strength = 0.9;   // 0.5~0.95
    }
    static void getEIS25Data(EIS25_Customize_Tuning_Para_t *a_pDataOut)
    {
        a_pDataOut->en_dejello = 0;
        a_pDataOut->stabilization_strength = 0.9;
        a_pDataOut->stabilization_level    = 4;
        a_pDataOut->gyro_still_mv_th       = 1;
        a_pDataOut->gyro_still_mv_diff_th  = 1;
    }

    static void getEIS30Data(EIS30_Customize_Tuning_Para_t *a_pDataOut)
    {
        // TODO: Update new tuning data
        a_pDataOut->stabilization_strength = 0.9;
        a_pDataOut->stabilization_level    = 4;
        a_pDataOut->gyro_still_mv_th       = 1;
        a_pDataOut->gyro_still_mv_diff_th  = 1;
    }

    /* EIS version support
     */
    static MBOOL isForcedEIS12()
    {
        return true;
    }

    static MBOOL isSupportAdvEIS_HAL3()
    {
        return false;
    }
    static MBOOL isEnabledEIS22()
    {
        return false;
    }
    static MBOOL isEnabledEIS25()
    {
        return false;
    }
    static MBOOL isEnabledEIS30()
    {
        return false;
    }
    static MBOOL isEnabledFixedFPS()
    {
        unsigned int variableFPS = ::property_get_int32(EIS_VARIABLE_FPS, 0);
        return variableFPS ? false : true;
    }
    static MBOOL isEnabledGyroMode()
    {
        return false;
    }
    static MBOOL isEnabledImageMode()
    {
        return false;
    }
    static MBOOL isEnabledForwardMode(MUINT32 cfg = 0)
    {
        (void)cfg;
        return false;
    }

    static MBOOL isEnabledLosslessMode()
    {
        return false;
    }

    static MBOOL isEnabledFOVWarpCombine(MUINT32 cfg = 0)
    {
        (void)cfg;
        return false;
    }

    static MBOOL isEnabledLMVData()
    {
        return false;
    }

    static MBOOL isEnabled4K2KMDP()
    {
        return true;
    }

    static MBOOL isSupportPreviewEIS()
    {
        return false;
    }

    /* EIS configurations
     */
    static double getEISRatio(MUINT32 cfg = 0, MUINT32 mask = 0)
    {
        (void)mask;
        return (cfg == VIDEO_CFG_4K2K) ?
               100.0/EIS_DEFAULT_FACTOR : 100.0/EIS_DEFAULT_FHD_FACTOR;
    }
    static MUINT32 getEIS12Factor()
    {
        return EIS_DEFAULT_FACTOR;
    }
    static MUINT32 getEISFactor(MUINT32 cfg = 0, MUINT32 mask = 0)
    {
        (void)mask;
        MUINT32 factor = (cfg == VIDEO_CFG_FHD) ? ::property_get_int32(VAR_EIS_CUSTOM_FACTOR, EIS_DEFAULT_FHD_FACTOR) :
                                                  ::property_get_int32(VAR_EIS_CUSTOM_FACTOR, EIS_DEFAULT_FACTOR);
        return factor;
    }
    static MFLOAT getHeightToWidthRatio()
    {
        return 0;
    }
    static MUINT32 get4K2KRecordFPS()
    {
        return FWEIS_DEFAULT_4K_VR_FPS;
    }
    static MUINT32 getForwardStartFrame()
    {
        return FWEIS_DEFAULT_START_FRAME;
    }
    static MUINT32 getForwardFrames(MUINT32 cfg = 0)
    {
        return (cfg == VIDEO_CFG_4K2K) ? FWEIS_DEFAULT_FRAMES_4K : FWEIS_DEFAULT_FRAMES;
    }
    static void getMVNumber(MINT32 width, MINT32 height, MINT32 *mvWidth, MINT32 *mvHeight)
    {
        (void)width;
        (void)height;
        MINT32 mv_width = ::property_get_int32(VAR_EIS_MV_WIDTH, 0);
        MINT32 mv_height = ::property_get_int32(VAR_EIS_MV_HEIGHT, 0);

        if (mv_width != 0 && mv_height != 0)
        {
            *mvWidth = mv_width;
            *mvHeight = mv_height;
            return;
        }

        if (mvWidth != 0 && mvHeight != 0)
        {
            *mvWidth = EIS_DEFAULT_MV_WIDTH;
            *mvHeight = EIS_DEFAULT_MV_HEIGHT;
        }
    }
};

#endif /* _EIS_CUSTOM_BASE_H_ */

