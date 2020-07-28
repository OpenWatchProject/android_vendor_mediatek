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

/**
 * @file af_algo_if.h
 * @brief AF algorithm interface, for raw sensor.
 */
#ifndef _AF_ALGO_IF_C_H_
#define _AF_ALGO_IF_C_H_

#include "af_param.h"

#define AF_ALGO_IF_REVISION 8365001

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    AF single API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * @brief Trigger focusing algorithm.
 */
MRESULT afc_trigger(ext_ESensorDev_T c_sensorDev);

/**
 * @brief Cancel focusing
 */
MRESULT afc_cancel(ext_ESensorDev_T c_sensorDev);

/**
 * @brief Lock AF behavior
 */
MRESULT afc_lock(ext_ESensorDev_T c_sensorDev);

/**
 * @brief Unlock pairing with lock
 */
MRESULT afc_unlock(ext_ESensorDev_T c_sensorDev);

/**
 * @brief Set AF algorithm mode.
 * @param [in] a_eAFMode Set AF mode for single/continous/Fullscan/MF; Please refer LIB3A_AF_MODE_T in af_feature.h
 */
MRESULT afc_setAFMode(ext_ESensorDev_T c_sensorDev, LIB3A_AF_MODE_T a_eAFMode);

/**
 * @brief HW Initial AF algorithm.
 * @param [in] a_sAFHwInitInput Hw Init Input AF settings from af manager; Please refer AF_HW_INIT_INPUT_T in af_param.h
 * @param [in] a_sAFHwInitOutput Hw Init Onput AF settings to af manager; Please refer AF_HW_INIT_OUTPUT_T in af_param.h
 */
MRESULT afc_initHwConfig(ext_ESensorDev_T c_sensorDev, const AF_HW_INIT_INPUT_T *a_sAFHwInitInput, AF_HW_INIT_OUTPUT_T *a_sAFHwInitOutput);

/**
 * @brief Initial AF algorithm.
 * @param [in] a_sAFInitInput Init Input AF algorithm settings from af manager; Please refer AF_INIT_INPUT_T in af_param.h
 * @param [in] a_sAFInitOutput Init Onput AF algorithm settings to af manager; Please refer AF_INIT_OUTPUT_T in af_param.h
 */
MRESULT afc_initAF(ext_ESensorDev_T c_sensorDev, const AF_INIT_INPUT_T *a_sAFInitInput, AF_INIT_OUTPUT_T *a_sAFInitOutput);

/**
 * @brief Handle AF algorithm tasks.
 * @param [in] a_sAFInput Input AF algorithm settings from af manager; Please refer AF_INPUT_T in af_param.h
 * @param [in] a_sAFOutput Onput AF algorithm settings to af manager; Please refer AF_OUTPUT_T in af_param.h
 */
MRESULT afc_handleAF(ext_ESensorDev_T c_sensorDev, const AF_INPUT_T *a_sAFInput, AF_OUTPUT_T *a_sAFOutput);

/**
 * @brief setup AFO parsing information
 */
void    afc_setupAFO(ext_ESensorDev_T c_sensorDev, const AF_INPUT_T *a_sAFInput);

/**
 * @brief do AFO parsing and bank preprocess
 */
void    afc_doBankAFO(ext_ESensorDev_T c_sensorDev, void *sram_addr, MINT32 bank_idx, MINT32 bank_sz_byte);

/**
 * @get ROI for PD Algorithm, Input AF ROI
 * @param [in] inAfRoi: AF ROI for calculate PD ROI
 * @param [in] inAfRoiNum: number of input AF ROI
 * @param [in] inWinBufSz: maximal number of PD ROI  can be generate
 * @param [out] outWins: output PD ROI
 * @param [out] outWinNum: number of output PD ROI
 */
MRESULT afc_getPdBlocks(ext_ESensorDev_T c_sensorDev, AFPD_BLOCK_ROI_T* inAfRoi, MINT32 inAfRoiNum, MINT32 inWinBufSz, FD_INFO_T inFdInfo, AFPD_BLOCK_ROI_T* outWins, MINT32* outWinNum);

/**
 * @brief Send debug information to AF manager. For internal debug information.
 * @param [in] a_sAFDebugInfo debug information data pointer.;Please refer AF_DEBUG_INFO_T in dbg_af_param.h
 */
MRESULT afc_getDebugInfo(ext_ESensorDev_T c_sensorDev, EXT_AF_DEBUG_INFO_T *a_DebugInfo);

/**
 * @brief Set manual focus position in AF algorithm. When AF mode is MF, use it to set lens position.
 * @param [in] a_i4Pos Lens position. Usually value in 0~1023.
 */
void    afc_setMFPos(ext_ESensorDev_T c_sensorDev, MINT32 a_i4Pos);

/**
 * @brief TAF speed up
 */
MRESULT afc_targetAssistMove(ext_ESensorDev_T c_sensorDev);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    AF sync API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void    afc_SetSyncMode(ext_ESensorDev_T c_sensorDev, MINT32 const a_i4SyncMode );   // Use in Main1 / Main2
void    afc_MotorRange(ext_ESensorDev_T c_sensorDev, AF_SyncInfo_T *sCamInfo);  // Use in Main2
MINT32  afc_GetSyncInfo(ext_ESensorDev_T c_sensorDev, AF_SyncInfo_T *sCamInfo); //used in main1
void    afc_GetMain2CalibPos(ext_ESensorDev_T c_sensorDev, AF_SyncInfo_T *sCamInfo);    //used in main2,
void    afc_CalibAFPos(ext_ESensorDev_T c_sensorDev, AF_SyncInfo_T *sCamInfo)  ;   //used in main1, calculate pos pair
void    afc_SetSyncInfo(ext_ESensorDev_T c_sensorDev, MINT32 a_i4Pos, AF_SyncInfo_T *sSlaveHisCamInfo);
AREA_T  afc_DepthMapAdjustROI(ext_ESensorDev_T c_sensorDev, void* a_pDepthmap, AREA_T a_sMapSize, AREA_T a_sMapSrcInfo, AREA_T a_sFrameSize, AREA_T a_sCurAFROI);


#endif

