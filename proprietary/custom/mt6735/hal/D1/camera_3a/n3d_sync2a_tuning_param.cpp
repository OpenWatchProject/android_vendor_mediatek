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

/*******************************************************************************
*
********************************************************************************/
#include <aaa_types.h>
#include "n3d_sync2a_tuning_param.h"

#define SYNC_AWB_STAT_Y_TH 20
#define SYNC_AWB_STAT_BLK_NUM_RAIO 5

static MUINT32 GainRatioTh[4] = {80, 150, 80, 150};
static MUINT32 CCTDiffTh[5] = {2000, 2000, 2000, 2000, 2000};

//=== Sync AE Tuning Parameters ===//
typedef enum
{
    SYNC_AE_STEREO_QUALITY_PRIORITY = 0,
    SYNC_AE_LUMA_QUALITY_PRIORITY
}SYNC_AE_POLICY;

MUINT32 
getSyncAePolicy()
{
    return SYNC_AE_STEREO_QUALITY_PRIORITY;
    //return SYNC_AE_LUMA_QUALITY_PRIORITY;
}


//===Sync AWB 
typedef enum
{
	SYNC_AWB_CCT_TH_METHOD = 0,
	SYNC_AWB_GAIN_INTERPOLATION_METHOD,
	SYNC_AWB_ADV_PP_METHOD,
	SYNC_AWB_FREE_RUN
}eSYNC_AWB_METHOD;


MUINT32
getSyncAwbMode()
{
    return SYNC_AWB_ADV_PP_METHOD;//SYNC_AWB_GAIN_INTERPOLATION_METHOD; 
}

const MUINT32*
getSyncGainRatioTh()
{
	return GainRatioTh;
}

const MUINT32*
getSyncCCTDiffTh()
{
	return CCTDiffTh;
}

MUINT32 getSynAwbStatYTh()
{
	return SYNC_AWB_STAT_Y_TH;
}

MUINT32 getSyncAwbStatNumRatio()
{
	return SYNC_AWB_STAT_BLK_NUM_RAIO;
}





