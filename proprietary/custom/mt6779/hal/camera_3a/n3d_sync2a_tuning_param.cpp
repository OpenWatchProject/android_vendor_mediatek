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



//=== Sync AE Tuning Parameters === //

typedef enum
{
    SYNC_AE_FOLLOW_MAIN = 0,
    SYNC_AE_FOLLOW_LDR
}SYNC_AE_POLICY;


static strSyncAEInitInfo gSyncAEInitInfo =
{
    false,
	{0,  0},
	{10, 10},
    // bayer standard RGB to Y 5:9:2
    {4174, 4952, 5940},
    // bayer to mono white
	//{4028, 4281, 5904},
	{10000, 10000, 10000},
	1057,
	0,
	false,
	SYNC_AE_FOLLOW_MAIN,
    {   //SYNC_AE_DUAL_CAM_DENOISE_BMDN
        {
        1000,  // EV   0
        1072,   //EV    0.1
        1149,   //EV    0.2
        1231,   //EV    0.3
        1320,   //EV    0.4
        1414,   //EV    0.5
        1516,   //EV    0.6
        1625,   //EV    0.7
        1625,   //EV    0.8
        1625,   //EV    0.9
        1625,   //EV    1
        1625,   //EV    1.1
        1625,   //EV    1.2
        1625,   //EV    1.3
        1625,   //EV    1.4
        1625,   //EV    1.5
        1625,   //EV    1.6
        1625,   //EV    1.7
        1625,   //EV    1.8
        1625,   //EV    1.9
        1625,   //EV    2
        1625,   //EV    2.1
        1625,   //EV    2.2
        1625,   //EV    2.3
        1625,   //EV    2.4
        1625,   //EV    2.5
        1625,   //EV    2.6
        1625,   //EV    2.7
        1625,   //EV    2.8
        1625,   //EV    2.9   
        },
        //SYNC_AE_DUAL_CAM_DENOISE_MFNR
        {
        1000,  // EV   0
        1072,   //EV    0.1
        1149,   //EV    0.2
        1231,   //EV    0.3
        1320,   //EV    0.4
        1414,   //EV    0.5
        1516,   //EV    0.6
        1625,   //EV    0.7
        1625,   //EV    0.8
        1625,   //EV    0.9
        1625,   //EV    1
        1625,   //EV    1.1
        1625,   //EV    1.2
        1625,   //EV    1.3
        1625,   //EV    1.4
        1625,   //EV    1.5
        1625,   //EV    1.6
        1625,   //EV    1.7
        1625,   //EV    1.8
        1625,   //EV    1.9
        1625,   //EV    2
        1625,   //EV    2.1
        1625,   //EV    2.2
        1625,   //EV    2.3
        1625,   //EV    2.4
        1625,   //EV    2.5
        1625,   //EV    2.6
        1625,   //EV    2.7
        1625,   //EV    2.8
        1625,   //EV    2.9   
        },
    },
};


const strSyncAEInitInfo*
getSyncAEInitInfo()
{
	return &gSyncAEInitInfo;
}




//===Sync AWB Tuning Parameters === //

typedef enum
{
	SYNC_AWB_ADV_PP_METHOD,
	SYNC_AWB_BLENDING_METHOD,
    SYNC_AWB_WHITPOINT_MAP_METHOD,
	SYNC_AWB_FREE_RUN
    
}eSYNC_AWB_METHOD;


static strSyncAWBInitInfo gSyncAWBInitInfo =
{
    SYNC_AWB_ADV_PP_METHOD,
    {
        MTRUE,
        //MFALSE,
		10,
        230,
        5,
        10, //10%
        20, //diff should less than 20%
		96, //64 Enhance Sub gain change base =512, range = 0~512

    },
    
    {8, 10, 5, 80},
    {
        {80,120},  //100 = 1x
        {80,120},
        
        {80,120},
        {80,120},

        10,
        
        {1,5},
        {5,250},
    },
    MTRUE,
    64,
    {
        MTRUE,
        {1,2,4}, 
    },
	{
		{512, 512, 512}, //Tungsten WB sync preference, base = 512
		{512, 512, 512}, //WARM_FLUORESCENT sync preference, base = 512
		{512, 512, 512}, //FLUORESCENT WB sync preference, base = 512
		{512, 512, 512}, //CWF WB sync preference, base = 512
		{512, 512, 512}, //DAYLIGHT WB sync preference, base = 512
		{512, 512, 512}, //Shade WB sync preference, base = 512
		{512, 512, 512}, //DAYLIGHT FLUORESCENT WB sync preference, base = 512
    },
	{
		{50,100},
		{512, 512}, //Tungsten WB sync preference channel R, base = 512
		{512, 512}, //Tungsten WB sync preference channel G, base = 512
		{512, 512}, //Tungsten WB sync preference channel B, base = 512
		{512, 512}, //WARM_FLUORESCENT sync preference channel R, base = 512
		{512, 512}, //WARM_FLUORESCENT sync preference channel G, base = 512
		{512, 512}, //WARM_FLUORESCENT sync preference channel B, base = 512
		{512, 512}, //FLUORESCENT WB sync preference channel R, base = 512
		{512, 512}, //FLUORESCENT WB sync preference channel G, base = 512
		{512, 512}, //FLUORESCENT WB sync preference channel B, base = 512
		{512, 512}, //CWF WB sync preference channel R, base = 512
		{512, 512}, //CWF WB sync preference channel G, base = 512
		{512, 512}, //CWF WB sync preference channel B, base = 512
		{512, 512}, //DAYLIGHT WB sync preference channel R, base = 512
		{512, 512}, //DAYLIGHT WB sync preference channel G, base = 512
		{512, 512}, //DAYLIGHT WB sync preference channel B, base = 512
		{512, 512}, //Shade WB sync preference channel R, base = 512
		{512, 512}, //Shade WB sync preference channel G, base = 512
		{512, 512}, //Shade WB sync preference channel B, base = 512
		{512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel R, base = 512
		{512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel G, base = 512
		{512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel B, base = 512
    },
};


const strSyncAWBInitInfo*
getSyncAWBInitInfo()
{
	return &gSyncAWBInitInfo;
}



