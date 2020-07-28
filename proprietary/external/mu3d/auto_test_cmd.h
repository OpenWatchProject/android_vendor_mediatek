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

extern INT32 _AutoDefault(INT32 i4Argc, const CHAR **szArgv);
extern INT32 _FsgDefault(INT32 i4Argc, const CHAR **szArgv);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

CLI_EXEC_T _arAutoTestCmdTbl [] =
{
	{ (CHAR*)"dev" , (CHAR*)"dev" , _AutoDefault , NULL, (CHAR*)"enter device test mode", CLI_GUEST },
	{ (CHAR*)"u3i" , (CHAR*)"u3i" , _AutoDefault , NULL, (CHAR*)"initialize phy module", CLI_GUEST },
	{ (CHAR*)"u3w" , (CHAR*)"u3w" , _AutoDefault , NULL, (CHAR*)"write phy register", CLI_GUEST },
	{ (CHAR*)"u3r" , (CHAR*)"u3r" , _AutoDefault , NULL, (CHAR*)"read phy register", CLI_GUEST },
	{ (CHAR*)"u3d" , (CHAR*)"u3d" , _AutoDefault , NULL, (CHAR*)"PIPE phase scan", CLI_GUEST },
	{ (CHAR*)"link" , (CHAR*)"link" , _AutoDefault , NULL, (CHAR*)"u3 device link up", CLI_GUEST },
	{ (CHAR*)"eyeinit" , (CHAR*)"eyeinit" , _AutoDefault , NULL, (CHAR*)"initialize eyescan", CLI_GUEST },	
	{ (CHAR*)"eyescan" , (CHAR*)"eyescan" , _AutoDefault , NULL, (CHAR*)"eyescan", CLI_GUEST },	
	{ (CHAR*)"stop" , (CHAR*)"stop" , _AutoDefault , NULL, (CHAR*)"Call TS_AUTO_TEST_STOP", CLI_GUEST },
	{ (CHAR*)"otg" , (CHAR*)"otg" , _AutoDefault , NULL, (CHAR*)"call otg_top", CLI_GUEST },
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};

CLI_EXEC_T _rAutoTestCmdTbl =
{
    (CHAR*)"auto test management", (CHAR*)"auto", NULL, _arAutoTestCmdTbl, (CHAR*)"Auto test configuration", CLI_GUEST,
};

CLI_EXEC_T* GetAutoTestCmdTbl(void){
    return &_rAutoTestCmdTbl;
}



//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

CLI_EXEC_T _arFsgTestCmdTbl [] =
{
	{ (CHAR*)"init" , (CHAR*)"init" , _FsgDefault , NULL, (CHAR*)"start to run normal driver", CLI_GUEST },
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};



CLI_EXEC_T _rFsgTestCmdTbl =
{
    (CHAR*)"musb driver initialization", (CHAR*)"fsg", NULL, _arFsgTestCmdTbl, (CHAR*)"Start musb driver", CLI_GUEST,
};

CLI_EXEC_T* GetFsgTestCmdTbl(void){
    return &_rFsgTestCmdTbl;
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
