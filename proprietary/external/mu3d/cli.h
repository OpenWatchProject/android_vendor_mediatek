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

/** @file cli.h
 *  Add your description here.
 */

#ifndef _CLI_H
#define _CLI_H

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
//#include "kal_release.h"
#include "x_typedef.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "assert.h"
// -----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
#define CLI_INPUT_BUF_ROW_NUM					3				// number of DOS key row
//#define CLI_INPUT_BUF_ROW_NUM					1				// number of DOS key row, jackson: now only use one row
#define CLI_INPUT_BUF_SIZE						256				// number of character per row
                                                                //
#define CLI_INPUT_MAX_CMD_TBL_NUM				4				// maximum number of attach command root table

#define CLI_PROMPT_STR							"U_CLI>"			// prompt string of CLI

// ASCII key define
#define ASCII_NULL								0x00
#define ASCII_KEY_BS							0x08
#define ASCII_KEY_NL							0x0A
#define ASCII_ENTER								0x0D
#define ASCII_KEY_CR							0x0D
#define ASCII_KEY_ESC							0x1B
#define ASCII_KEY_SPACE							0x20
#define ASCII_KEY_DBL_QUOTE						0x22
#define ASCII_KEY_QUOTE							0x27
#define ASCII_KEY_DOT							0x2e
#define ASCII_KEY_DOLLAR						0x24
#define ASCII_KEY_UP							0x41
#define ASCII_KEY_DOWN							0x42
#define ASCII_KEY_RIGHT							0x43
#define ASCII_KEY_LEFT							0x44
#define ASCII_KEY_ARROW							0x5B
#define ASCII_KEY_ROOT							0x5c

#define ASCII_KEY_PRINTABLE_MIN					0x20
#define ASCII_KEY_PRINTABLE_MAX					0x7E

#define IsPrintable(c) ((((c) > ASCII_NULL) && ((c) < ASCII_KEY_SPACE)) ? 0 : 1)
#define IsSpace(c) (((c)==' ') || ((c)=='\n') || ((c)=='\t') || ((c)=='\r') || ((c)=='\a'))
#define IsDot(c) (((c) == ASCII_KEY_DOT) ? 1 : 0)
#define IsRoot(c) (((c) == ASCII_KEY_ROOT) ? 1 : 0)

#define CLI_MAX_ARGU							20				// maximum number of argument
#define CLI_MAX_ARGU_LEN						256				//length of each argument, modify 32 to 256, due to argument need fit http url  length
                                                                // length of each argument
#define CLI_MANDA_CMD_TBL_IDX					0				// mandatory command root table index
#define CLI_MAX_CMD_TBL_LEVEL					8				// maximum level of command table
//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
//#define CLI_COMMAND_OK					0
#define CLI_UNKNOWN_CMD					-2147483647

/******************************************************************************
* cli command access right
******************************************************************************/
typedef enum
{
	CLI_SUPERVISOR = 0,
	CLI_ADMIN,
	CLI_GUEST,
	CLI_HIDDEN
} CLI_ACCESS_RIGHT_T;


/******************************************************************************
* cli command structure
******************************************************************************/
typedef struct _CLI_EXEC
{
	CHAR*				pszCmdStr;													// command string
	CHAR*				pszCmdAbbrStr;												// command abbreviation
	INT32				(*pfExecFun) (INT32 i4Argc, const CHAR ** szArgv);			// execution function
	struct _CLI_EXEC	*prCmdNextLevel;											// next level command table
	CHAR*				pszCmdHelpStr;												// command description string
	CLI_ACCESS_RIGHT_T	eAccessRight;												// command access right
} CLI_EXEC_T;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------
typedef enum _ENUM_CLI_ERR_CODE_T
{
    CLI_COMMAND_OK = 0,
    //E_CLI_COMMAND_OK = 0,
    E_CLI_ERR_GENERAL = -1,
}
ENUM_CLI_ERR_CODE_T;
//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------
extern INT32 CLI_CmdTblAttach(CLI_EXEC_T* pTbl);
//jackson temp remove extern CLI_EXEC_T* CLI_GetDefaultCmdTbl(void);


typedef CLI_EXEC_T* (*CLI_GET_CMD_TBL_FUNC)(void);

#define cli_print printf
//#define cli_print(x) printf(x)
//do not porting alias related code
/*
// CLI Alias Related

#define CLI_ALIAS_NUM							0x10			// maximum number of alias support
#define CLI_ALIAS_SIZE							0x10			// maximum number of alias characters

#define CLI_ALIAS_CMD_STR						"alias"
#define CLI_ALIAS_CMD_ABBR_STR					"a"

extern void CLI_AliasInit(void);
extern const CHAR* CLI_AliasCompare(const CHAR* szAlias);
extern INT32 CLI_CmdAlias(INT32 i4Argc, const CHAR ** szArgv);
*/
#endif /* _CLI_H */

#ifdef __cplusplus
}
#endif


