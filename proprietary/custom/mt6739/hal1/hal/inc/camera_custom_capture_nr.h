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
#ifndef _CAMERA_CUSTOM_CAPTURE_NR_H_
#define _CAMERA_CUSTOM_CAPTURE_NR_H_

#include "camera_custom_types.h"

#define DISABLE_CAPTURE_NR (12800)

typedef struct
{
    int hwth;
    int swth;
}Capture_NR_Th_t;

enum EIdxSwNR
{
    eIDX_SWNR_SINGLE_ISO_100 = 0,
    eIDX_SWNR_SINGLE_ISO_200,
    eIDX_SWNR_SINGLE_ISO_400,
    eIDX_SWNR_SINGLE_ISO_800,
    eIDX_SWNR_SINGLE_ISO_1200,
    eIDX_SWNR_SINGLE_ISO_1600,
    eIDX_SWNR_SINGLE_ISO_2000,
    eIDX_SWNR_SINGLE_ISO_2400,
    eIDX_SWNR_SINGLE_ISO_2800,
    eIDX_SWNR_SINGLE_ISO_3200,
    eIDX_SWNR_SINGLE_ISO_ANR_ENC_OFF,
    eIDX_SWNR_SINGLE_ISO_ANR_ENC_ON,
    eIDX_SWNR_MFLL_ISO_100,
    eIDX_SWNR_MFLL_ISO_200,
    eIDX_SWNR_MFLL_ISO_400,
    eIDX_SWNR_MFLL_ISO_800,
    eIDX_SWNR_MFLL_ISO_1200,
    eIDX_SWNR_MFLL_ISO_1600,
    eIDX_SWNR_MFLL_ISO_2000,
    eIDX_SWNR_MFLL_ISO_2400,
    eIDX_SWNR_MFLL_ISO_2800,
    eIDX_SWNR_MFLL_ISO_3200,
    eIDX_SWNR_MFLL_ISO_ANR_ENC_OFF,
    eIDX_SWNR_MFLL_ISO_ANR_ENC_ON,
    eNUM_OF_SWNR_IDX,
};

enum ESWNRType
{
    eSWNRType_Default,
    eSWNRType_IIR,
};

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        Capture_NR_Th_t* pTh
        );

// get performance level: 2 > 1 > 0, -1: default level
MINT32 get_performance_level(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        MBOOL const isMfll,
        MBOOL const isMultiOpen
        );

// method: <0: nearest, 1: lower, 2: upper>
EIdxSwNR map_ISO_value_to_index(MUINT32 const u4Iso, MBOOL const isMfll, MUINT32 method = 0);
MUINT32  map_index_to_ISO_value(EIdxSwNR const u4Idx);

MBOOL is_to_invoke_swnr_interpolation(MBOOL const isMfll, MUINT32 const u4Iso);

MINT32 get_swnr_type();

#endif /* _CAMERA_CUSTOM_CAPTURE_NR_H_ */
