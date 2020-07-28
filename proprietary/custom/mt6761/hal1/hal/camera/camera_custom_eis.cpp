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

#include "camera_custom_eis.h"

/* EIS Hal1 Configurations
 *
 *  => DO NOT modify
 */
#if GMO_RAM_OPTIMIZE
#define EIS_ADV_SUPPORT_HAL_ONE     (0)
#else
#define EIS_ADV_SUPPORT_HAL_ONE     (1)
#endif

#define EIS_SUPPORT_FRAME_HAL_ONE   (25)

#define EIS_MV_WIDTH_WIDE               (48)
#define EIS_MV_HEIGHT_WIDE              (27)
#define EIS_MV_WIDTH                    (48)
#define EIS_MV_HEIGHT                   (36)

void get_EIS_CustomizeData(EIS_Customize_Para_t *a_pDataOut)
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

void get_EIS_PLUS_CustomizeData(EIS_PLUS_Customize_Para_t *a_pDataOut)
{
    a_pDataOut->warping_mode = EIS_WARP_METHOD_6_4_ADAPTIVE;
    a_pDataOut->search_range_x = 64;        // 32~64
    a_pDataOut->search_range_y = 64;        // 32~64
    a_pDataOut->crop_ratio     = 20;        // 10~40
    a_pDataOut->gyro_still_time_th = 6; // 1~20
    a_pDataOut->gyro_max_time_th = 14; // 8~52
    a_pDataOut->gyro_similar_th = 77; // 0~100
    a_pDataOut->stabilization_strength = 0.9;   // 0.5~0.95
}

void getEIS30Data(EIS30_Customize_Tuning_Para_t *a_pDataOut)
{
    // TODO: Update new tuning data
    a_pDataOut->stabilization_strength = 0.9;
    a_pDataOut->stabilization_level    = 4;
    a_pDataOut->gyro_still_mv_th       = 1;
    a_pDataOut->gyro_still_mv_diff_th  = 1;
}


bool GetEisLinkWithDfr()
{
    return true;    // true : dynamic frame rate off when eis on; false : dynamic frame rate on when eis on
}

bool isAdvEisSupport_Hal1()
{
    return EIS_ADV_SUPPORT_HAL_ONE;
}

MUINT32 getEisSupFrames_Hal1()
{
    if( !isAdvEisSupport_Hal1())
        return 0;
    return EIS_SUPPORT_FRAME_HAL_ONE;
}

void getMVNumber_Hal1(MINT32 width, MINT32 height, MINT32 *mvWidth, MINT32 *mvHeight)
{
    float ratio = (width != 0) ? (float)height/width : 0;

    if (mvWidth != 0 && mvHeight != 0)
    {
        if(ratio < (((3.0/4)+(9.0/16))/2))//16:9
        {
            *mvWidth = EIS_MV_WIDTH_WIDE;
            *mvHeight = EIS_MV_HEIGHT_WIDE;
        }
        else //4:3 and others
        {
            *mvWidth = EIS_MV_WIDTH;
            *mvHeight = EIS_MV_HEIGHT;
        }
    }
}

