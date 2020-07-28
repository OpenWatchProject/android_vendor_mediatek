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
#ifndef _DBG_AF_PARAM_H_
#define _DBG_AF_PARAM_H_

// --- AF debug info ---
#define AF_EXIF_VERSION       (4)
#define AF_DEBUG_TAG_SIZE     (1050)
#define DBG_AF_PARAM_REVISION 6162003
#define DBG_PD_PARAM_REVISION 5343001

typedef struct
{
    AAA_DEBUG_TAG_T Tag[AF_DEBUG_TAG_SIZE];

} AF_DEBUG_INFO_T;

typedef enum
{
    IDX = 0,	// search idx
    POS,		// lens position
    VLU,		// focus value
    MINL,		// idx for min FV in inf side
    MAX,		// idx for max FV
    MINR,		// idx for min FV in macro side

    FIN_3P, 	// finish: Peak found from 3 points
    FIN_ICL,	// finish: Incline case
    FIN_BND,	// finish: Full search case

    BEST_POS,   // lens target position from curve fitting
    FOCUS_POS,  // focused lens position

    FIN_DAF1,	// finish: DAF 1 step
    FIN_DAFn,	// finish: DAF + D-seek
    FAIL_DAF,	// can not find peak by daf

    LV,         // light value
    FAIL,       // can not find peak
    FAIL_BND,   // can not find peak     (boundary)
    MAIN_THRES, // main threshold percent
    SUB_THRES,  // sub threshold percent
    THRES_OFFSET,          // spot threshold offset
    MATRIX_THRES_OFFSET,   // matrix threshold offset

    THRES_VLU_MAIN,        // main threshold value
    STATE,      // AF state
    AFMODE,     // 1: AFS, 2: AFC, 3: Macro, 4: Inf, 5: MF, 6: Cal, 7: Fullscan
    METER_MENU,    // 1: Spot, 2: Matrix
    METER_REAL,    // real meter
    WIN_X,    // AF window location  - left
    WIN_Y,    // AF window location  - top
    WIN_W,    // AF window location  - width
    WIN_H,    // AF window location  - height

    AFTIME,     // AF process time

    FD_STATUS,  // 0: no face, 1: face detected

    SCAN_START, // fullscan start position
    SCAN_STEP,  // fullscan step interval
    SCAN_NUM,

    STEP_L,     // lnfinity boundary in AF table
    STEP_H,     // macro boundary in AF table

    AF_AREA_PERCENT_W,  // AF window width vs image width percetage
    AF_AREA_PERCENT_H,  // AF window height vs image height percetage

    FOCUSED_IDX_NEAREST,   // nearest focused window idx
    FOCUSED_IDX,           // focused window idx
    FOCUSED_IDX_FARTHEST,  // farthest focused window idx
    DOF,                   // DOF

    VERSION,               // version

    ZOOM_W,                // image width after zoom
    ZOOM_H,                // image higght after zoom
    ZOOM_X,                // image left,top position after zoom
    ZOOM_Y,                // image left,top position after zoom

    FIRST_FV,              // first focus value for scene change compare
    CHANGE_FV,             // scene change focus value
    HW_TH,                 // af hardware threshold
    FV_DC,                 // DC focus value
    MIN_TH,                // minimum threshold

    ZSD,                   // is ZSD mode
    ZSD_FIN_BND,           // peak found in ZSD mode
    ZSD_FAIL,              // peak not found in ZSD mode
    ZSD_MONO_VLU,          // monotous focus value in ZSD mode
    AE_STABLE,             // is AE stable
    ISO,                   // ISO value
    GSUM,                  // G sum value
    XCURR,                 // 2D interpolation Xcurr
    ZCURR,                 // 2D interpolation Zcurr
    X0,                    // 2D interpolation X0
    X1,                    // 2D interpolation X1
    Z0,                    // 2D interpolation Z0
    Z1,                    // 2D interpolation Z1
    Y00,                   // 2D interpolation Y00
    Y01,                   // 2D interpolation Y01
    Y10,                   // 2D interpolation Y10
    Y11,                   // 2D interpolation Y11
    FIRST_GS,              // first gsum value for scene change compare
    CHANGE_GS,             // scene change gsum value
    POST_COMP_OFFSET,      // posture compensation offset
    DAMPING_COMP,          // damping compensation enable
    FV_EXTRACT,	           // fv extraction enable
    DBG_IDX,               // findpeak idx
    DBG_POS,               // findpeak pos
    DBG_VLU,               // findpeak vlu
    DBG_COMP_DIR,          // damping comp dir
    DBG_COMP_STEP,         // damping comp step
    DBG_COMP_VLU,          // damping comp vlu
    DBG_EXT_BIN,           // fv extract bin values
    DBG_EXT_FITCNT,        // fv extract fitcnt
    DBG_COMP_PRERUN,       // prerun
    DBG_COMP_FRAME,        // frametime
    DBG_COMP_SHUTTER,      // shutter
    DBG_COMP_SIDX,         // shutter idx
    DBG_COMP_SRATIO,	   // shutter ratio
    DBG_PD_ROI_X,
    DBG_PD_ROI_Y,
    DBG_PD_ROI_W,
    DBG_PD_ROI_H,
    DBG_PD_CUR_POS,
    DBG_PD_TAR_POS,
    DBG_PD_PDVAL,
    DBG_PD_CONF,
    DBG_PD_CORR,
    DBG_PD_EVAL1,
    DBG_PD_EVAL2,
    DBG_PD_EVAL3,
    DBG_PD_EVAL4,
    DBG_PD_CONV,
    DBG_PD_CALI_STEP1_RATIO,
    DBG_PD_CALI_STEP2_CONV,
    DBG_PD_CALI_STEP2_POS,
    DBG_PD_CALI_STEP3_POS,
	DBG_PD_WIN_ORIENTATION,
    DBG_HYF_AFMODE,
    DBG_HYF_STATE,
    DBG_HYF_CNT,
    DBG_HYF_DAC,
    DBG_HYF_CONF,
    DBG_HYT_CNT,
    DBG_HYT_CONVERGE,
    DBG_HYT_WAIT_CAF,
    DBG_HYT_CAF,
    DBG_HYT_WAIT_FSH,
    DBG_HYT_FINESEARCH,
    DBG_HYT_WAIT_MOV,
    DBG_HYT_MOVING,
    DBG_HYT_DAC,
    DBG_HYT_VLU,
    DBG_HYT_NOTE,
    DBG_RV_VLU,
    YLU,
    PLSEL,
    PLSTATE,
    PLRATE,
    // merge pdaf exif here
    DISTANCE,

    OVER_PATH_LENGTH,
    DBG_PD_INFO_0,
    DBG_PD_INFO_1,
    DBG_PD_INFO_2,
    DBG_PD_INFO_3,
    DBG_PD_INFO_4,

    MGR_TG_W,
    MGR_TG_H,
    MGR_BIN_W,
    MGR_BIN_H,
    MGR_CROP_WIN_X,
    MGR_CROP_WIN_Y,
    MGR_CROP_WIN_W,
    MGR_CROP_WIN_H,
    MGR_DZ_CFG,
    MGR_DZ_FACTOR,
    MGR_FOCUSING_WIN_X,
    MGR_FOCUSING_WIN_Y,
    MGR_FOCUSING_WIN_W,
    MGR_FOCUSING_WIN_H,
    MGR_OTFD_WIN_X,
    MGR_OTFD_WIN_Y,
    MGR_OTFD_WIN_W,
    MGR_OTFD_WIN_H,
    MGR_CENTER_WIN_X,
    MGR_CENTER_WIN_Y,
    MGR_CENTER_WIN_W,
    MGR_CENTER_WIN_H,
    MGR_CMD_WIN_X,
    MGR_CMD_WIN_Y,
    MGR_CMD_WIN_W,
    MGR_CMD_WIN_H,
    MGR_PD_BUF_TYPE,
    MGR_PD_SEN_TYPE,
    MGR_LASER_VAL,
    MGR_FOCUSING_POS,
    MGR_FOCUSING_DST,
    MGR_CURRENT_POS,
    MGR_TS,
    MGR_GYRO_SENSOR_X,
    MGR_GYRO_SENSOR_Y,
    MGR_GYRO_SENSOR_Z,

    MZ_WIN_NUM,
    MZ_WIN_X,
    MZ_WIN_Y,
    MZ_WIN_W,
    MZ_WIN_H,
    MZ_WIN_RES,

    MZ_CLUSTER,
    MZ_PD_RELIABLE,
    MZ_NEAR_CENTER,
    MZ_SAT_CNT,
    MZ_ZONE_STATUS,
    MZ_ZONE_FINISH,
    MZ_ZONE_PEAK,
    MZ_FV,
    MZ_GSUM,
    MZ_OVERLAP_SCALE,
    MZ_H_ZONE_NUM,
    MZ_V_ZONE_NUM,
    
    CALI_TEMP_START,
    CALI_TEMP_END,
    CALI_TEMP_FOCUS,
    CALI_OTP_MAC,
    CALI_OTP_INF,
    
    DBG_GYRO,
    DBG_ACCE,
    DBG_TIME,
    DBG_FD_X,
    DBG_FD_Y,
    DBG_FD_W,
    DBG_FD_H,
    DBG_FD_COUNT,
    DBG_FD_NON_COUNT,
    DBG_FD_STATUS,
    DBG_FACE_EXTRACT_VOTE,
    DBG_FACE_EXTRACT_VOTE_TH,
    DBG_FACE_EXTRACT_FV
    
} AF_DEBUG_TAG_T;

#endif // _DBG_AF_PARAM_H_

