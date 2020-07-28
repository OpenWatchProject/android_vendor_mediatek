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
#ifndef _FACE_CAPTURE_H
#define _FACE_CAPTURE_H

#include "camera_custom_nvram.h"

#include "camera_custom_isp_nvram.h"

#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##_##B

#define DM_ISP_YUV_BASE                                              M_CONC(TARGET_SENSOR, DM_0032)
#define YNR_ISP_YUV_BASE                                             M_CONC(TARGET_SENSOR, YNR_0056)
#define YNR_LCE_LINK_ISP_YUV_BASE                                    M_CONC(TARGET_SENSOR, YNR_LCE_LINK_0024)
#define YNR_FD_ISP_YUV_BASE                                          M_CONC(TARGET_SENSOR, YNR_FD_0024)
#define CNR_CNR_ISP_YUV_BASE                                         M_CONC(TARGET_SENSOR, CNR_CNR_0048)
#define EE_ISP_YUV_BASE                                              M_CONC(TARGET_SENSOR, EE_0040)
#define NR3D_ISP_YUV_BASE                                            M_CONC(TARGET_SENSOR, NR3D_0008)
#define HFG_ISP_YUV_BASE                                             M_CONC(TARGET_SENSOR, HFG_0008)
#define DM_ISP_MFNR_BASE                                             M_CONC(TARGET_SENSOR, DM_0040)
#define YNR_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, YNR_0072)
#define CNR_CNR_ISP_MFNR_BASE                                        M_CONC(TARGET_SENSOR, CNR_CNR_0056)
#define EE_ISP_MFNR_BASE                                             M_CONC(TARGET_SENSOR, EE_0056)
#define SWNR_THRES_ISP_MFNR_BASE                                     M_CONC(TARGET_SENSOR, SWNR_THRES_0003)
#define SWNR_ISP_MFNR_BASE                                           M_CONC(TARGET_SENSOR, SWNR_0024)
#define LPCNR_ISP_MFNR_BASE                                          M_CONC(TARGET_SENSOR, LPCNR_0024)
#define MFNR_ISP_MFNR_BASE                                           M_CONC(TARGET_SENSOR, MFNR_0008)
#define BSS_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, BSS_0008)
#define MFB_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, MFB_0008)
#define MIX_ISP_MFNR_BASE                                            M_CONC(TARGET_SENSOR, MIX_0008)
#define MFNR_THRES_ISP_MFNR_BASE                                     M_CONC(TARGET_SENSOR, MFNR_THRES_0001)
#define YNR_ISP_DSDN_BASE                                            M_CONC(TARGET_SENSOR, YNR_0064)
#define EE_ISP_DSDN_BASE                                             M_CONC(TARGET_SENSOR, EE_0048)
#define DSDN_ISP_DSDN_BASE                                           M_CONC(TARGET_SENSOR, DSDN_0001)
#define CCM_CCM_BASE                                                 M_CONC(TARGET_SENSOR, CCM_0006)
#define COLOR_PARAM_COLOR_BASE                                       M_CONC(TARGET_SENSOR, COLOR_PARAM_0001)
#define COLOR_COLOR_BASE                                             M_CONC(TARGET_SENSOR, COLOR_0060)

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
extern const NVRAM_CAMERA_FEATURE_MFLL_STRUCT                        MFNR_ISP_MFNR_BASE[8];
extern const FEATURE_NVRAM_BSS_T                                     BSS_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_MFB_T                                         MFB_ISP_MFNR_BASE[8];
extern const ISP_NVRAM_MIX_T                                         MIX_ISP_MFNR_BASE[8];
extern const NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT                  MFNR_THRES_ISP_MFNR_BASE[1];
extern const ISP_NVRAM_YNR_T                                         YNR_ISP_DSDN_BASE[8];
extern const ISP_NVRAM_EE_T                                          EE_ISP_DSDN_BASE[8];
extern const FEATURE_NVRAM_DSDN_T                                    DSDN_ISP_DSDN_BASE[1];
extern const ISP_NVRAM_MULTI_CCM_STRUCT                              CCM_CCM_BASE[6];
extern const ISP_NVRAM_COLOR_PARAM_T                                 COLOR_PARAM_COLOR_BASE[1];
extern const ISP_NVRAM_SCOLOR_PARAM_T                                COLOR_COLOR_BASE[60];

#endif