/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

#include "cli.h"

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------

extern INT32 _HubDefault(INT32 i4Argc, const CHAR **szArgv);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------


CLI_EXEC_T _arHubTestCmdTbl [] =//
{
	{ (CHAR*)"config" , (CHAR*)"config" , _HubDefault , NULL, (CHAR*)"config hub attach to roothub port", CLI_GUEST },
	{ (CHAR*)"subhub" , (CHAR*)"subhub" , _HubDefault , NULL, (CHAR*)"config hub attach to any subhub", CLI_GUEST },
	{ (CHAR*)"dev" , (CHAR*)"dev" , _HubDefault , NULL, (CHAR*)"config device attach to any subhub", CLI_GUEST },
	{ (CHAR*)"deveth" , (CHAR*)"deveth" , _HubDefault , NULL, (CHAR*)"config ethernet adapter device attach to any subhub", CLI_GUEST },
	{ (CHAR*)"loop" , (CHAR*)"loop" , _HubDefault , NULL, (CHAR*)"start loop back test", CLI_GUEST },
	{ (CHAR*)"lbstress" , (CHAR*)"lpstress" , _HubDefault , NULL, (CHAR*)"start loop back stress test", CLI_GUEST },
	{ (CHAR*)"ixia" , (CHAR*)"ixia" , _HubDefault , NULL, (CHAR*)"start ixia stress test", CLI_GUEST },
	{ (CHAR*)"reset" , (CHAR*)"reset" , _HubDefault , NULL, (CHAR*)"Reset a device attach to hub", CLI_GUEST },
	{ (CHAR*)"wakeup" , (CHAR*)"wakeup" , _HubDefault , NULL, (CHAR*)"Assert device to remote wakeup", CLI_GUEST },
	{ (CHAR*)"selsuspend" , (CHAR*)"selsuspend" , _HubDefault , NULL, (CHAR*)"Selective suspend a hub port", CLI_GUEST },
	{ (CHAR*)"selresume" , (CHAR*)"selresume" , _HubDefault , NULL, (CHAR*)"Selective resume a hub port", CLI_GUEST },
	{ (CHAR*)"intr" , (CHAR*)"intr" , _HubDefault , NULL, (CHAR*)"queue an intr in urb", CLI_GUEST },
	{ (CHAR*)"u1u2" , (CHAR*)"u1u2" , _HubDefault , NULL, (CHAR*)"set u1/u2 timeout to hub port", CLI_GUEST },
	{ (CHAR*)"forcepm" , (CHAR*)"forcepm" , _HubDefault , NULL, (CHAR*)"force hub to accept u1/u2 lpm", CLI_GUEST },	
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};

CLI_EXEC_T _rHubTestCmdTbl =
{
    (CHAR*)"hub commands", (CHAR*)"hub", NULL, _arHubTestCmdTbl, (CHAR*)"hub commands for driver development", CLI_GUEST,
};

CLI_EXEC_T* GetHubTestCmdTbl(void){
    return &_rHubTestCmdTbl;
}

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// extern variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Imported variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Imported functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// extern functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *  @param
 *  @retval
 */
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------
//
