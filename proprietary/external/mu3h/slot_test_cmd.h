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

extern INT32 _SlotDefault(INT32 i4Argc, const CHAR **szArgv);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

CLI_EXEC_T _arSlotTestCmdTbl [] =
{
//	{ (CHAR*)"address" , (CHAR*)"add" , _SlotDefault , NULL, (CHAR*)"attach and address device", CLI_GUEST },
	{ (CHAR*)"configure" , (CHAR*)"config" , _SlotDefault , NULL, (CHAR*)"attach and configure device", CLI_GUEST },
	{ (CHAR*)"disconnect" , (CHAR*)"discon" , _SlotDefault , NULL, (CHAR*)"disconnect, disable slot", CLI_GUEST },
	{ (CHAR*)"configep" , (CHAR*)"configep" , _SlotDefault , NULL, (CHAR*)"config a bulk endpoint", CLI_GUEST },
	{ (CHAR*)"resetslot" , (CHAR*)"reset" , _SlotDefault , NULL, (CHAR*)"reset device slot", CLI_GUEST },
	{ (CHAR*)"resetport" , (CHAR*)"resetp" , _SlotDefault , NULL, (CHAR*)"reset port", CLI_GUEST },
	{ (CHAR*)"reconfigslot" , (CHAR*)"reconfig" , _SlotDefault , NULL, (CHAR*)"reconfig slot", CLI_GUEST },
	{ (CHAR*)"enableslot" , (CHAR*)"enable" , _SlotDefault , NULL, (CHAR*)"attache, reset and enable slot", CLI_GUEST },
	{ (CHAR*)"disableslot" , (CHAR*)"disable" , _SlotDefault , NULL, (CHAR*)"disable slot", CLI_GUEST },
	{ (CHAR*)"addressslot" , (CHAR*)"addslt" , _SlotDefault , NULL, (CHAR*)"address an enabled slot", CLI_GUEST },	
	{ (CHAR*)"getdescriptor" , (CHAR*)"getdesc" , _SlotDefault , NULL, (CHAR*)"Do get_descriptor ctrl transfer", CLI_GUEST },	
	{ (CHAR*)"getAlldescriptor" , (CHAR*)"getAlldesc" , _SlotDefault , NULL, (CHAR*)"Do get more descriptor ctrl transfer", CLI_GUEST },	
	{ (CHAR*)"getbos" , (CHAR*)"getbos" , _SlotDefault , NULL, (CHAR*)"Do get_descriptor ctrl request of BOS descriptor", CLI_GUEST },	
	{ (CHAR*)"setconfiguration" , (CHAR*)"setconf" , _SlotDefault , NULL, (CHAR*)"Do set configuration", CLI_GUEST },		
	{ (CHAR*)"setu1u2enable" , (CHAR*)"setu1u2" , _SlotDefault , NULL, (CHAR*)"set feature - u1 enable, u2 enable", CLI_GUEST },			
	{ (CHAR*)"getdevstatus" , (CHAR*)"devstat" , _SlotDefault , NULL, (CHAR*)"Get status - device status", CLI_GUEST },	
	{ (CHAR*)"evaluatecontext" , (CHAR*)"evalctx" , _SlotDefault , NULL, (CHAR*)"evaluate context test", CLI_GUEST },				
	{ (CHAR*)"ped" , (CHAR*)"ped" , _SlotDefault , NULL, (CHAR*)"test port enable/disable(ped) register", CLI_GUEST },					
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};

CLI_EXEC_T _rSlotTestCmdTbl =
{
    (CHAR*)"slog_management", (CHAR*)"slt", NULL, _arSlotTestCmdTbl, (CHAR*)"slot managements", CLI_GUEST,
};

CLI_EXEC_T* GetSlotTestCmdTbl(void){
    return &_rSlotTestCmdTbl;
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
