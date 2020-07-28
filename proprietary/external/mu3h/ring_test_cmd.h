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

extern INT32 _RingDefault(INT32 i4Argc, const CHAR **szArgv);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

CLI_EXEC_T _arRingTestCmdTbl [] =
{
	{ (CHAR*)"erfull" , (CHAR*)"erfull" , _RingDefault , NULL, (CHAR*)"Test event ring full error", CLI_GUEST },
	{ (CHAR*)"stopcmd" , (CHAR*)"stopcmd" , _RingDefault , NULL, (CHAR*)"Stop command ring", CLI_GUEST },
	{ (CHAR*)"abortcmd" , (CHAR*)"abortcmd" , _RingDefault , NULL, (CHAR*)"Abort command ring", CLI_GUEST },
	{ (CHAR*)"stopep" , (CHAR*)"stopep" , _RingDefault , NULL, (CHAR*)"Stop ep ring", CLI_GUEST },
	{ (CHAR*)"rrd" , (CHAR*)"rrd" , _RingDefault , NULL, (CHAR*)"Add a random ring doorbell thread", CLI_GUEST },
	{ (CHAR*)"rstp" , (CHAR*)"rstp" , _RingDefault , NULL, (CHAR*)"Add a random stop ep thread", CLI_GUEST },	
	{ (CHAR*)"enlarge" , (CHAR*)"enlarge" , _RingDefault , NULL, (CHAR*)"Enlarge a ep ring", CLI_GUEST },
	{ (CHAR*)"shrink" , (CHAR*)"shrink" , _RingDefault , NULL, (CHAR*)"Shrink a ep ring", CLI_GUEST },
	{ (CHAR*)"intrmod" , (CHAR*)"intrmod" , _RingDefault , NULL, (CHAR*)"Test interrupt moderation", CLI_GUEST },
	{ (CHAR*)"bei" , (CHAR*)"bei" , _RingDefault , NULL, (CHAR*)"Test bei of TRB with normal bulk transfer", CLI_GUEST },	
	{ (CHAR*)"idt" , (CHAR*)"idt" , _RingDefault , NULL, (CHAR*)"Test idt of TRB with normal bulk transfer", CLI_GUEST },		
	{ (CHAR*)"noop" , (CHAR*)"noop" , _RingDefault , NULL, (CHAR*)"Test noop transfer trb", CLI_GUEST },		
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};

CLI_EXEC_T _rRingTestCmdTbl =
{
    (CHAR*)"ring mgt function", (CHAR*)"ring", NULL, _arRingTestCmdTbl, (CHAR*)"Ring mgt function management", CLI_GUEST,
};

CLI_EXEC_T* GetRingTestCmdTbl(void){
    return &_rRingTestCmdTbl;
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
