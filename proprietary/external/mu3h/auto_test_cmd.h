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

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

CLI_EXEC_T _arAutoTestCmdTbl [] =
{
	{ (CHAR*)"handshake" , (CHAR*)"handshake" , _AutoDefault , NULL, (CHAR*)"Init handshake a test case", CLI_GUEST },
	{ (CHAR*)"getresult" , (CHAR*)"getresult" , _AutoDefault , NULL, (CHAR*)"Get result after test case", CLI_GUEST },		
	{ (CHAR*)"quit" , (CHAR*)"quit" , _AutoDefault , NULL, (CHAR*)"Quit autotest", CLI_GUEST },			
	{ (CHAR*)"u3handshake" , (CHAR*)"u3handshake" , _AutoDefault , NULL, (CHAR*)"u3 auto test POC", CLI_GUEST },			
	{ (CHAR*)"u3getresult" , (CHAR*)"u3getresult" , _AutoDefault , NULL, (CHAR*)"Quit autotest", CLI_GUEST },			
	{ (CHAR*)"u3quit" , (CHAR*)"u3quit" , _AutoDefault , NULL, (CHAR*)"Quit autotest", CLI_GUEST },	
	{ (CHAR*)"lbctrl" , (CHAR*)"lbctrl" , _AutoDefault , NULL, (CHAR*)"Ctrl transfer loopback", CLI_GUEST },	
	{ (CHAR*)"lb" , (CHAR*)"lb" , _AutoDefault , NULL, (CHAR*)"BULK/INTR/ISOC transfer loopback", CLI_GUEST },	
	{ (CHAR*)"lbscan" , (CHAR*)"lbscan" , _AutoDefault , NULL, (CHAR*)"BULK/INTR/ISOC transfer loopback scan all parameters", CLI_GUEST },	
	{ (CHAR*)"lbsg" , (CHAR*)"lbsg" , _AutoDefault , NULL, (CHAR*)"BULK transfer scatter-gather loopback", CLI_GUEST },	
	{ (CHAR*)"lbsgscan" , (CHAR*)"lbsgscan" , _AutoDefault , NULL, (CHAR*)"BULK transfer scatter-gather loopback scan all parameters", CLI_GUEST },	
	{ (CHAR*)"devrandomstop" , (CHAR*)"devrandomstop" , _AutoDefault , NULL, (CHAR*)"test device random stop function", CLI_GUEST },	
	{ (CHAR*)"randomsuspend" , (CHAR*)"randomsuspend" , _AutoDefault , NULL, (CHAR*)"randomly suspend for n times", CLI_GUEST },	
	{ (CHAR*)"randomwakeup" , (CHAR*)"randomwakeup" , _AutoDefault , NULL, (CHAR*)"randomly remote wakeup for n times", CLI_GUEST },		
	{ (CHAR*)"stress" , (CHAR*)"stress" , _AutoDefault , NULL, (CHAR*)"stress test", CLI_GUEST },			
	{ (CHAR*)"isofrm" , (CHAR*)"isofrm" , _AutoDefault , NULL, (CHAR*)"test iso transfer with frameid instead of SIA", CLI_GUEST },				
	{ (CHAR*)"conresume" , (CHAR*)"conresume" , _AutoDefault , NULL, (CHAR*)"concurrent resume", CLI_GUEST },					
	{ (CHAR*)"conu1u2" , (CHAR*)"conu1u2" , _AutoDefault , NULL, (CHAR*)"concurrent u1/u2 enter", CLI_GUEST }, 				
	{ (CHAR*)"conu1u2exit" , (CHAR*)"conu1u2exit" , _AutoDefault , NULL, (CHAR*)"concurrent u1/u2 exit", CLI_GUEST }, 					
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
