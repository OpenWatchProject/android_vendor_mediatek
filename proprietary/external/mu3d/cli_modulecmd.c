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
#include "assert.h"

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

// cli module table
extern CLI_EXEC_T* GetHcdTestCmdTbl(void);
extern CLI_EXEC_T* GetSlotTestCmdTbl(void);
extern CLI_EXEC_T* GetLoopTestCmdTbl(void);
extern CLI_EXEC_T* GetPowerTestCmdTbl(void);
extern CLI_EXEC_T* GetAutoTestCmdTbl(void);
extern CLI_EXEC_T* GetRingTestCmdTbl(void);
extern CLI_EXEC_T* GetStressTestCmdTbl(void);
extern CLI_EXEC_T* GetHubTestCmdTbl(void);
extern CLI_EXEC_T* GetDbgCmdTbl(void);
extern CLI_EXEC_T* GetDevTestCmdTbl(void);
extern CLI_EXEC_T* GetFsgTestCmdTbl(void);

CLI_GET_CMD_TBL_FUNC _pfCliGetTbl[] =
{
	GetHcdTestCmdTbl,	
	GetFsgTestCmdTbl,
	GetAutoTestCmdTbl
};


#define CLI_MOD_NS (sizeof(_pfCliGetTbl)/sizeof(CLI_GET_CMD_TBL_FUNC))

static CLI_EXEC_T _rNullTbl = {NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR};
static CLI_EXEC_T _arUserCmdTbl[CLI_MOD_NS + 1];

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static variables
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
// Static functions
//-----------------------------------------------------------------------------




//----------------------------------------------------------------------------- 
/** 
 * 
 *  @param  
 *  @retval   
 */
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------
// 
void CLI_ModuleCmd_Install()
{
    UINT32 u4Idx;
    UINT32 u4CmdIdx;
    CLI_EXEC_T* prModCmdTbl;

    // initialize module command table
    for (u4Idx = 0; u4Idx < (UINT32)(CLI_MOD_NS + 1); u4Idx++)
    {
        _arUserCmdTbl[u4Idx] = _rNullTbl;
    }

    // install module command table
    u4CmdIdx = 0;
    for (u4Idx = 0; u4Idx < (UINT32)CLI_MOD_NS; u4Idx++)
    {
//        ASSERT(_pfCliGetTbl[u4Idx] != NULL);
        assert(_pfCliGetTbl[u4Idx] != NULL);
        prModCmdTbl = _pfCliGetTbl[u4Idx]();

        if ((prModCmdTbl != NULL) &&
            (prModCmdTbl->pszCmdStr != NULL) &&
            ((prModCmdTbl->pfExecFun != NULL) || (prModCmdTbl->prCmdNextLevel != NULL)))
        {
            _arUserCmdTbl[u4CmdIdx] = *prModCmdTbl;
            u4CmdIdx++;
        }
    }

    CLI_CmdTblAttach(_arUserCmdTbl);
}
