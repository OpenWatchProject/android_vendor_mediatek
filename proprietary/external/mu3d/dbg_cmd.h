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

extern INT32 _DbgReadPCI(INT32 i4Argc, const CHAR **szArgv);
extern INT32 _DbgReadXHCI(INT32 i4Argc, const CHAR **szArgv);
extern INT32 _DbgDumpXHCIRegs(INT32 i4Argc, const CHAR **szArgv);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------


CLI_EXEC_T _arDbgCmdTbl [] =
{
	{ (CHAR*)"read_pci" , (CHAR*)"rpci" , _DbgReadPCI , NULL, (CHAR*)"read PCI memory", CLI_GUEST },
	{ (CHAR*)"read_xhci" , (CHAR*)"r" , _DbgReadPCI , NULL, (CHAR*)"read XHCI memory", CLI_GUEST },
	{ (CHAR*)"dump_xhci_regs" , (CHAR*)"dr" , _DbgDumpXHCIRegs , NULL, (CHAR*)"dump XHCI registers", CLI_GUEST },
	{ (CHAR*)"printportstatus" , (CHAR*)"portstatus" , _DbgDumpXHCIRegs , NULL, (CHAR*)"print current attached port status", CLI_GUEST },
	{ (CHAR*)"debug_slt_ctx" , (CHAR*)"dbgslt" , _DbgDumpXHCIRegs , NULL, (CHAR*)"print slot output context", CLI_GUEST },	
	{ (CHAR*)"printhccparams" , (CHAR*)"hccparams" , _DbgDumpXHCIRegs , NULL, (CHAR*)"print HCCPARAMS", CLI_GUEST }, 
	{ (CHAR*)"scheduling1" , (CHAR*)"sch1" , _DbgDumpXHCIRegs , NULL, (CHAR*)"Test NEC scheduling", CLI_GUEST }, 
	{ (CHAR*)"scheduling2" , (CHAR*)"sch2" , _DbgDumpXHCIRegs , NULL, (CHAR*)"Test NEC scheduling", CLI_GUEST }, 
	{ (CHAR*)"scheduling3" , (CHAR*)"sch3" , _DbgDumpXHCIRegs , NULL, (CHAR*)"Test NEC scheduling", CLI_GUEST }, 
	{ (CHAR*)"setpls" , (CHAR*)"setpls" , _DbgDumpXHCIRegs , NULL, (CHAR*)"set port pls value", CLI_GUEST }, 	
	{ (CHAR*)"setped" , (CHAR*)"setped" , _DbgDumpXHCIRegs , NULL, (CHAR*)"set port ped bit", CLI_GUEST }, 		
	{ (CHAR*)"portreset" , (CHAR*)"portreset" , _DbgDumpXHCIRegs , NULL, (CHAR*)"hot reset or warm reset a port", CLI_GUEST },
	{ (CHAR*)"mdelay" , (CHAR*)"mdelay" , _DbgDumpXHCIRegs , NULL, (CHAR*)"delay msecs", CLI_GUEST }, 			
	{ (CHAR*)"u3w" , (CHAR*)"u3w" , _DbgDumpXHCIRegs , NULL, (CHAR*)"write phy register", CLI_GUEST }, 			
	{ (CHAR*)"u3r" , (CHAR*)"u3r" , _DbgDumpXHCIRegs , NULL, (CHAR*)"read phy register", CLI_GUEST }, 				
	{ (CHAR*)"u3i" , (CHAR*)"u3i" , _DbgDumpXHCIRegs , NULL, (CHAR*)"init u3 A60802 phy setting", CLI_GUEST },	
	{ (CHAR*)"u3ic" , (CHAR*)"u3ic" , _DbgDumpXHCIRegs , NULL, (CHAR*)"init u3 C60802 phy setting", CLI_GUEST },		
	{ (CHAR*)"u3" , (CHAR*)"u3" , _DbgDumpXHCIRegs , NULL, (CHAR*)"u3 A60802 phy calibration", CLI_GUEST },		
	{ (CHAR*)"u3c" , (CHAR*)"u3c" , _DbgDumpXHCIRegs , NULL, (CHAR*)"u3 C60802 phy calibration", CLI_GUEST },			
	{ (CHAR*)"u3eyescan" , (CHAR*)"u3eyescan" , _DbgDumpXHCIRegs , NULL, (CHAR*)"u3 phy eyescan", CLI_GUEST },			
	{ (CHAR*)"mw" , (CHAR*)"mw" , _DbgDumpXHCIRegs , NULL, (CHAR*)"memory write", CLI_GUEST },	
	{ (CHAR*)"mr" , (CHAR*)"mr" , _DbgDumpXHCIRegs , NULL, (CHAR*)"memory read", CLI_GUEST },	
	{ (CHAR*)"keyboard" , (CHAR*)"kb" , _DbgDumpXHCIRegs , NULL, (CHAR*)"keyboard class test", CLI_GUEST }, 
	{ (CHAR*)"sch" , (CHAR*)"sch" , _DbgDumpXHCIRegs , NULL, (CHAR*)"verify scheduler algorithm", CLI_GUEST }, 
	{ (CHAR*)"ewe" , (CHAR*)"ewe" , _DbgDumpXHCIRegs , NULL, (CHAR*)"verify ewe register", CLI_GUEST }, 	
    { (CHAR*)NULL , (CHAR*)NULL , NULL, NULL, (CHAR*) NULL, CLI_GUEST }
};

CLI_EXEC_T _rDbgCmdTbl =
{
    (CHAR*)"debug commands", (CHAR*)"dbg", NULL, _arDbgCmdTbl, (CHAR*)"debug commands for driver development", CLI_GUEST,
};

CLI_EXEC_T* GetDbgCmdTbl(void){
    return &_rDbgCmdTbl;
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
