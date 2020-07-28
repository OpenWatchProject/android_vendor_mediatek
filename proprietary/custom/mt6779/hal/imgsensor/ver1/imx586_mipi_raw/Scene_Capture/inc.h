/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _SCENE_CAPTURE_H
#define _SCENE_CAPTURE_H

#include "camera_custom_nvram.h"

#include "camera_custom_isp_nvram.h"

#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##_##B

#define AE_AE_BASE                                                   M_CONC(TARGET_SENSOR, AE_0000)
#define AF_AF_BASE                                                   M_CONC(TARGET_SENSOR, AF_0000)
#define AWB_AWB_BASE                                                 M_CONC(TARGET_SENSOR, AWB_0000)
#define Flash_AE_Flash_AE_BASE                                       M_CONC(TARGET_SENSOR, Flash_AE_0000)
#define Flash_AWB_Flash_AWB_BASE                                     M_CONC(TARGET_SENSOR, Flash_AWB_0000)
#define Flash_Calibration_Flash_Calibration_BASE                     M_CONC(TARGET_SENSOR, Flash_Calibration_0000)
#define LCE_TONE_BASE                                                M_CONC(TARGET_SENSOR, LCE_0000)
#define GMA_TONE_BASE                                                M_CONC(TARGET_SENSOR, GMA_0000)
#define DCE_TONE_BASE                                                M_CONC(TARGET_SENSOR, DCE_0000)
#define LTM_TONE_BASE                                                M_CONC(TARGET_SENSOR, LTM_0000)
#define HLR_TONE_BASE                                                M_CONC(TARGET_SENSOR, HLR_0000)
#define DM_ISP_YUV_BASE                                              M_CONC(TARGET_SENSOR, DM_0000)
#define YNR_ISP_YUV_BASE                                             M_CONC(TARGET_SENSOR, YNR_0000)
#define YNR_LCE_LINK_ISP_YUV_BASE                                    M_CONC(TARGET_SENSOR, YNR_LCE_LINK_0000)
#define YNR_FD_ISP_YUV_BASE                                          M_CONC(TARGET_SENSOR, YNR_FD_0000)
#define CNR_CNR_ISP_YUV_BASE                                         M_CONC(TARGET_SENSOR, CNR_CNR_0000)
#define EE_ISP_YUV_BASE                                              M_CONC(TARGET_SENSOR, EE_0000)
#define NR3D_ISP_YUV_BASE                                            M_CONC(TARGET_SENSOR, NR3D_0000)
#define HFG_ISP_YUV_BASE                                             M_CONC(TARGET_SENSOR, HFG_0000)
#define DM_ISP_MFNR_BASE                                             M_CONC(TARGET_SENSOR, DM_0008)
#define YNR_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, YNR_0016)
#define CNR_CNR_ISP_MFNR_BASE                                        M_CONC(TARGET_SENSOR, CNR_CNR_0008)
#define EE_ISP_MFNR_BASE                                             M_CONC(TARGET_SENSOR, EE_0016)
#define SWNR_THRES_ISP_MFNR_BASE                                     M_CONC(TARGET_SENSOR, SWNR_THRES_0001)
#define SWNR_ISP_MFNR_BASE                                           M_CONC(TARGET_SENSOR, SWNR_0008)
#define LPCNR_ISP_MFNR_BASE                                          M_CONC(TARGET_SENSOR, LPCNR_0008)
#define MFNR_THRES_ISP_MFNR_BASE                                     M_CONC(TARGET_SENSOR, MFNR_THRES_0000)
#define MFNR_ISP_MFNR_BASE                                           M_CONC(TARGET_SENSOR, MFNR_0000)
#define BSS_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, BSS_0000)
#define MFB_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, MFB_0000)
#define MIX_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, MIX_0000)
#define DM_ISP_AINR_BASE                                             M_CONC(TARGET_SENSOR, DM_0016)
#define YNR_ISP_AINR_BASE                                            M_CONC(TARGET_SENSOR, YNR_0040)
#define YNR_LCE_LINK_ISP_AINR_BASE                                   M_CONC(TARGET_SENSOR, YNR_LCE_LINK_0008)
#define YNR_FD_ISP_AINR_BASE                                         M_CONC(TARGET_SENSOR, YNR_FD_0008)
#define CNR_CNR_ISP_AINR_BASE                                        M_CONC(TARGET_SENSOR, CNR_CNR_0032)
#define EE_ISP_AINR_BASE                                             M_CONC(TARGET_SENSOR, EE_0024)
#define SWNR_THRES_ISP_AINR_BASE                                     M_CONC(TARGET_SENSOR, SWNR_THRES_0002)
#define SWNR_ISP_AINR_BASE                                           M_CONC(TARGET_SENSOR, SWNR_0016)
#define LPCNR_ISP_AINR_BASE                                          M_CONC(TARGET_SENSOR, LPCNR_0016)
#define AINR_ISP_AINR_BASE                                           M_CONC(TARGET_SENSOR, AINR_0000)
#define AINR_THRES_ISP_AINR_BASE                                     M_CONC(TARGET_SENSOR, AINR_THRES_0000)
#define YNR_ISP_DSDN_BASE                                            M_CONC(TARGET_SENSOR, YNR_0008)
#define EE_ISP_DSDN_BASE                                             M_CONC(TARGET_SENSOR, EE_0008)
#define DSDN_ISP_DSDN_BASE                                           M_CONC(TARGET_SENSOR, DSDN_0000)
#define OBC_ISP_RAW_BASE                                             M_CONC(TARGET_SENSOR, OBC_0000)
#define BPC_BPC_ISP_RAW_BASE                                         M_CONC(TARGET_SENSOR, BPC_BPC_0000)
#define BPC_CT_ISP_RAW_BASE                                          M_CONC(TARGET_SENSOR, BPC_CT_0000)
#define BPC_PDC_ISP_RAW_BASE                                         M_CONC(TARGET_SENSOR, BPC_PDC_0000)
#define LDNR_ISP_RAW_BASE                                            M_CONC(TARGET_SENSOR, LDNR_0000)
#define CNR_CCR_ISP_Other_BASE                                       M_CONC(TARGET_SENSOR, CNR_CCR_0000)
#define CNR_ABF_ISP_Other_BASE                                       M_CONC(TARGET_SENSOR, CNR_ABF_0000)
#define SWNR_THRES_ISP_Other_BASE                                    M_CONC(TARGET_SENSOR, SWNR_THRES_0000)
#define SWNR_ISP_Other_BASE                                          M_CONC(TARGET_SENSOR, SWNR_0000)
#define LPCNR_ISP_Other_BASE                                         M_CONC(TARGET_SENSOR, LPCNR_0000)
#define FUS_ISP_Other_BASE                                           M_CONC(TARGET_SENSOR, FUS_0000)
#define ZFUS_ISP_Other_BASE                                          M_CONC(TARGET_SENSOR, ZFUS_0000)
#define CA_LTM_CA_LTM_BASE                                           M_CONC(TARGET_SENSOR, CA_LTM_0000)
#define CCM_CCM_BASE                                                 M_CONC(TARGET_SENSOR, CCM_0000)
#define COLOR_PARAM_COLOR_BASE                                       M_CONC(TARGET_SENSOR, COLOR_PARAM_0000)
#define COLOR_COLOR_BASE                                             M_CONC(TARGET_SENSOR, COLOR_0000)
#define ClearZoom_ClearZoom_BASE                                     M_CONC(TARGET_SENSOR, ClearZoom_0000)
#define YNR_TBL_ISP_YNR_TBL_BASE                                     M_CONC(TARGET_SENSOR, YNR_TBL_0000)

extern const AE_NVRAM_T                                              AE_AE_BASE[1];
extern const NVRAM_LENS_DATA_PARA_STRUCT                             AF_AF_BASE[1];
extern const AWB_NVRAM_T                                             AWB_AWB_BASE[1];
extern const FLASH_AE_NVRAM_T                                        Flash_AE_Flash_AE_BASE[1];
extern const FLASH_AWB_NVRAM_T                                       Flash_AWB_Flash_AWB_BASE[1];
extern const FLASH_CALIBRATION_NVRAM_T                               Flash_Calibration_Flash_Calibration_BASE[1];
extern const ISP_NVRAM_LCE_TUNING_PARAM_T                            LCE_TONE_BASE[1];
extern const ISP_NVRAM_GMA_STRUCT_T                                  GMA_TONE_BASE[1];
extern const ISP_NVRAM_DCE_TUNING_PARAM_T                            DCE_TONE_BASE[1];
extern const ISP_NVRAM_LTM_TUNING_PARAM_T                            LTM_TONE_BASE[1];
extern const ISP_NVRAM_HLR_TUNING_PARAM_T                            HLR_TONE_BASE[1];
extern const ISP_NVRAM_DM_T                                          DM_ISP_YUV_BASE[8];
extern const ISP_NVRAM_YNR_T                                         YNR_ISP_YUV_BASE[8];
extern const ISP_NVRAM_YNR_LCE_LINK_T                                YNR_LCE_LINK_ISP_YUV_BASE[8];
extern const ISP_NVRAM_YNR_FD_T                                      YNR_FD_ISP_YUV_BASE[8];
extern const ISP_NVRAM_CNR_CNR_T                                     CNR_CNR_ISP_YUV_BASE[8];
extern const ISP_NVRAM_EE_T                                          EE_ISP_YUV_BASE[8];
extern const ISP_NVRAM_NR3D_T                                        NR3D_ISP_YUV_BASE[8];
extern const FEATURE_NVRAM_HFG_T                                     HFG_ISP_YUV_BASE[8];
extern const ISP_NVRAM_DM_T                                          DM_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_YNR_T                                         YNR_ISP_MFNR_BASE[24];
extern const ISP_NVRAM_CNR_CNR_T                                     CNR_CNR_ISP_MFNR_BASE[24];
extern const ISP_NVRAM_EE_T                                          EE_ISP_MFNR_BASE[8];
extern const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT                  SWNR_THRES_ISP_MFNR_BASE[1];
extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT                        SWNR_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_LPCNR_T                                       LPCNR_ISP_MFNR_BASE[8];
extern const NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT                  MFNR_THRES_ISP_MFNR_BASE[1];
extern const NVRAM_CAMERA_FEATURE_MFLL_STRUCT                        MFNR_ISP_MFNR_BASE[8];
extern const FEATURE_NVRAM_BSS_T                                     BSS_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_MFB_T                                         MFB_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_MIX_T                                         MIX_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_DM_T                                          DM_ISP_AINR_BASE[24];
extern const ISP_NVRAM_YNR_T                                         YNR_ISP_AINR_BASE[16];
extern const ISP_NVRAM_YNR_LCE_LINK_T                                YNR_LCE_LINK_ISP_AINR_BASE[16];
extern const ISP_NVRAM_YNR_FD_T                                      YNR_FD_ISP_AINR_BASE[16];
extern const ISP_NVRAM_CNR_CNR_T                                     CNR_CNR_ISP_AINR_BASE[16];
extern const ISP_NVRAM_EE_T                                          EE_ISP_AINR_BASE[16];
extern const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT                  SWNR_THRES_ISP_AINR_BASE[1];
extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT                        SWNR_ISP_AINR_BASE[8];
extern const ISP_NVRAM_LPCNR_T                                       LPCNR_ISP_AINR_BASE[8];
extern const FEATURE_NVRAM_AINR_T                                    AINR_ISP_AINR_BASE[8];
extern const NVRAM_CAMERA_FEATURE_AINR_THRES_STRUCT                  AINR_THRES_ISP_AINR_BASE[1];
extern const ISP_NVRAM_YNR_T                                         YNR_ISP_DSDN_BASE[8];
extern const ISP_NVRAM_EE_T                                          EE_ISP_DSDN_BASE[8];
extern const FEATURE_NVRAM_DSDN_T                                    DSDN_ISP_DSDN_BASE[1];
extern const ISP_NVRAM_OBC_T                                         OBC_ISP_RAW_BASE[8];
extern const ISP_NVRAM_BPC_BPC_T                                     BPC_BPC_ISP_RAW_BASE[8];
extern const ISP_NVRAM_BPC_CT_T                                      BPC_CT_ISP_RAW_BASE[8];
extern const ISP_NVRAM_BPC_PDC_T                                     BPC_PDC_ISP_RAW_BASE[8];
extern const ISP_NVRAM_LDNR_T                                        LDNR_ISP_RAW_BASE[8];
extern const ISP_NVRAM_CNR_CCR_T                                     CNR_CCR_ISP_Other_BASE[8];
extern const ISP_NVRAM_CNR_ABF_T                                     CNR_ABF_ISP_Other_BASE[8];
extern const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT                  SWNR_THRES_ISP_Other_BASE[1];
extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT                        SWNR_ISP_Other_BASE[8];
extern const ISP_NVRAM_LPCNR_T                                       LPCNR_ISP_Other_BASE[8];
extern const ISP_NVRAM_FUS_T                                         FUS_ISP_Other_BASE[8];
extern const ISP_NVRAM_ZFUS_T                                        ZFUS_ISP_Other_BASE[8];
extern const FEATURE_NVRAM_CA_LTM_T                                  CA_LTM_CA_LTM_BASE[1];
extern const ISP_NVRAM_MULTI_CCM_STRUCT                              CCM_CCM_BASE[6];
extern const ISP_NVRAM_COLOR_PARAM_T                                 COLOR_PARAM_COLOR_BASE[1];
extern const ISP_NVRAM_SCOLOR_PARAM_T                                COLOR_COLOR_BASE[60];
extern const ISP_NVRAM_CLEARZOOM_T                                   ClearZoom_ClearZoom_BASE[1];
extern const ISP_NVRAM_YNR_LUT_T                                     YNR_TBL_ISP_YNR_TBL_BASE[8];

#endif