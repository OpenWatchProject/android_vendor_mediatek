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


//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------
static char _aszArgv[CLI_MAX_ARGU][CLI_MAX_ARGU_LEN];           // temp argument buffer

static CLI_EXEC_T* _aprCmdTblBuf[CLI_INPUT_MAX_CMD_TBL_NUM];    // root command table pointer
static UINT32 _u4CmdTblNum = 0;                                 // number of root command table

static CLI_EXEC_T* _aprCmdTblLinkBuf[CLI_MAX_CMD_TBL_LEVEL];    // for 'cd' command
static UINT32 _u4CmdTblLinkIdx = 0;                             // for 'cd' command
static CLI_EXEC_T* _prCurrentCmdTbl = NULL;                     // current command table level, for 'cd' command

static char* _szCliPromptStr = NULL;

//static CLI_ACCESS_RIGHT_T _eCliMode = CC_CLI_PERMISSION;            // global permission variable
static CLI_ACCESS_RIGHT_T _eCliMode = CLI_ADMIN;                      // global permission variable

/******************************************************************************
* Function prototype
******************************************************************************/
static void _CliShowHelp(const CLI_EXEC_T * prCmdTbl);
static INT32 _CliStringParser(const CHAR *szCmdBuf, UINT32 u4ArgNum, UINT32 u4ArgLen, CHAR * const * szArgv);
static CLI_EXEC_T* _CliCmdSearch(const CHAR*  szArgv, CLI_EXEC_T * prCmdTbl);
static INT32 _CliCmdParser(INT32 i4Argc, const CHAR**  szArgv, CLI_EXEC_T * prCmdTbl, INT32 fgNoChk);
static void _CliGeneratePrompt(void);
//INT32 CLI_CmdRepeat(INT32 i4Argc, const CHAR ** szArgv);
INT32 CLI_CmdChangeDirectory(INT32 i4Argc, const CHAR ** szArgv);
static INT32 CLI_CmdList(INT32 i4Argc, const CHAR ** szArgv);
static INT32 _CmdTestExitAPI(INT32 i4Argc, const char ** szArgv);
static INT32 _CmdDoExitCli(INT32 i4Argc, const char ** szArgv);
int CLI_Parser_Arg(char *fmt,...);

#if 1
extern int write_cmd(char *buf);
extern int read_msg(char *buf);
#endif

/******************************************************************************
* Variable      : cli mandatory command table
******************************************************************************/
//const CLI_EXEC_T _arMandatoryCmdTbl[] CLI_MAIN_COMMAND =
const CLI_EXEC_T _arMandatoryCmdTbl[] =
{
    {
        "cd",                       //pszCmdStr
        "cd",
        CLI_CmdChangeDirectory,     //execution function
        NULL,
        "Change current directory",
        CLI_GUEST
    },
#if 0
    {
        "do",                       //pszCmdStr
        NULL,
        CLI_CmdRepeat,              //execution function
        NULL,
        "Repeat command",
        CLI_GUEST
    },
#endif
	{
		"ls",						//pszCmdStr
		"ls",
		CLI_CmdList,		//execution function
		NULL,
		"Recursive list all commands",
		CLI_GUEST
	},
    {
		"exitapi",						//pszCmdStr
		"exitapi",
		_CmdTestExitAPI,		//execution function
		NULL,
		"exitapi",
		CLI_GUEST
	},
    {
		"exit",						//pszCmdStr
		"exit",
		_CmdDoExitCli,		//execution function
		NULL,
		"exit user space CLI Task",
		CLI_GUEST
	},
    // last cli command record, NULL
    {
        NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
    }
};


//-----------------------------------------------------------------------------
// global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Imported variables
//-----------------------------------------------------------------------------
extern UINT8 u1UserCLiExitFlag;
//-----------------------------------------------------------------------------
// Imported functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------
//#if 0
#define HELP_STR_POS            24
static BOOL _FormatHelpString(char* szDst, UINT32 u4DstSize, const char * szCmd, const char * szAbbr, const char* szHelp)
{
    UINT32 u4CmdLen, u4AbbrLen, u4HelpLen;
    BOOL fgOut;

//    ASSERT(szCmd != NULL);
    assert(szCmd != NULL);

    //cli_print("szCmd=%s, szAbbr=%s, szHelp=%s\r\n", szCmd,szAbbr,szHelp);
    // Check if destination buffer is big enough
    u4CmdLen = strlen(szCmd);
    u4AbbrLen = strlen(szAbbr);
    u4HelpLen = strlen(szHelp);

    fgOut = FALSE;
//    fgOut = KAL_FALSE;
    if (szHelp != NULL)
    {
        if ((u4CmdLen + (u4AbbrLen + 4)) > HELP_STR_POS)
        {
            if ((u4CmdLen + (u4AbbrLen + (u4HelpLen + 5))) > u4DstSize)
            {
                fgOut = TRUE;
                //fgOut = KAL_TRUE;
            }
        }
        else
        {
            if ((HELP_STR_POS + (u4HelpLen + 2)) > u4DstSize)
            {
                fgOut = TRUE;
                //fgOut = KAL_TRUE;
            }
        }
    }
    else
    {
        if ((u4CmdLen + (u4AbbrLen + 4)) > u4DstSize)
        {
            fgOut = TRUE;
            //fgOut = KAL_TRUE;
        }
    }
    if (fgOut)
    {
        return FALSE;
        //return KAL_FALSE;
    }

    // Format command and abbreviation strings
    if (szAbbr != NULL)
    {
        sprintf(szDst, "%s(%s):", szCmd, szAbbr);
    }
    else
    {
        sprintf(szDst, "%s:", szCmd);
    }

    // Append spaces and help string
    if (szHelp != NULL)
    {
        UINT32 i;

        //ASSERT(szDst!=NULL);
        assert(szDst!=NULL);
        for (i = strlen(szDst); i < HELP_STR_POS; i++)
        {
            szDst[i] = ' ';
        }
        //LINT_SUPPRESS_NEXT_EXPRESSION(534);
        strcpy(szDst + HELP_STR_POS, szHelp);
    }

    return TRUE;
    //return KAL_TRUE;
}

#define MAX_CLI_PATH_LENGTH 64
static void _CLI_PrintCmd(const char *cmdPath,CLI_EXEC_T  *cli_cmd)
{
	char *pszStr;

    //ASSERT(cli_cmd != NULL);
    assert(cli_cmd != NULL);
    //UNUSED(cli_cmd);
	pszStr = cli_cmd->pszCmdStr;
	if (cli_cmd->pszCmdAbbrStr!=NULL)
    {
		pszStr = cli_cmd->pszCmdAbbrStr;
    }

	if (strlen(cmdPath)>0)
	{
		cli_print("%s.%s",cmdPath,pszStr);
	}
	else
	{
		cli_print("%s",pszStr);
	}
	if (cli_cmd->pszCmdHelpStr!=NULL)
    {
		cli_print("\t\t:%s",cli_cmd->pszCmdHelpStr);
    }
	cli_print("\n");
}

static void _CLI_RecursiveListCliCmd(const char *cmdPath,CLI_EXEC_T * prCmdTbl,CLI_ACCESS_RIGHT_T access_right)
{
	UINT32 u4Idx;
	char szBuffer[MAX_CLI_PATH_LENGTH+1];
	char *pszStr;
	UINT32 u4Tmp1;

	if (prCmdTbl != NULL)
	{
		for (u4Idx = 0;prCmdTbl[u4Idx].pszCmdStr != NULL;u4Idx++) //print command first
		{
			if ((prCmdTbl[u4Idx].eAccessRight >= access_right) &&
				(prCmdTbl[u4Idx].prCmdNextLevel == NULL ))
			{
				_CLI_PrintCmd(cmdPath,&prCmdTbl[u4Idx]);
			}
		}
		for (u4Idx = 0;prCmdTbl[u4Idx].pszCmdStr != NULL;u4Idx++) //recursive into directory
		{
			if ((prCmdTbl[u4Idx].eAccessRight >= access_right) &&
				(prCmdTbl[u4Idx].prCmdNextLevel != NULL))
			{

				pszStr = prCmdTbl[u4Idx].pszCmdStr;
				if (prCmdTbl[u4Idx].pszCmdAbbrStr!=NULL)
                {
					pszStr = prCmdTbl[u4Idx].pszCmdAbbrStr;
                }

				if ((strlen(cmdPath)+strlen(pszStr)+1)>MAX_CLI_PATH_LENGTH) //+1 for '.'
				{
					//cli_print("%s: %s.%s too long can't recursive\r\n",__FUNCTION__,cmdPath,pszStr);
					cli_print("%s.%s too long can't recursive\r\n",cmdPath,pszStr);
					continue;
				}
				u4Tmp1 = strlen(cmdPath);
				if (u4Tmp1>0)
                {
					sprintf(szBuffer,"%s.%s",cmdPath,pszStr);
                }
				else
                {
					sprintf(szBuffer,"%s",pszStr);
                }
				_CLI_RecursiveListCliCmd(szBuffer,prCmdTbl[u4Idx].prCmdNextLevel,access_right);
			}
		}
	}

}


//-----------------------------------------------------------------------------
/**
 *
 *  @param
 *  @retval
 */
//-----------------------------------------------------------------------------
INT32 _CmdTestExitAPI(INT32 i4Argc, const char ** szArgv)
{
    CLI_Parser_Arg("exit");
    return 0;
}
//-----------------------------------------------------------------------------
/**
 *
 *  @param
 *  @retval
 */
//-----------------------------------------------------------------------------
static INT32 _CmdDoExitCli(INT32 i4Argc, const char ** szArgv)
{
    //UNUSED(i4Argc);
    //UNUSED(szArgv);
    u1UserCLiExitFlag=1;
    return 0;
}
static INT32 CLI_CmdList(INT32 i4Argc, const char ** szArgv)
{
	const CHAR *pszStr;
	UINT32 u4Idx,u4CmdIdx;
	CLI_EXEC_T * prCmdTbl;

    //UNUSED(i4Argc);
    //UNUSED(szArgv);
	if (_prCurrentCmdTbl == NULL)	// root
	{
		for (u4Idx = CLI_MANDA_CMD_TBL_IDX; u4Idx < _u4CmdTblNum; u4Idx++) //print command first
		{
			for (u4CmdIdx = 0;_aprCmdTblBuf[u4Idx][u4CmdIdx].pszCmdHelpStr != NULL;u4CmdIdx++)
			{
				prCmdTbl = &(_aprCmdTblBuf[u4Idx][u4CmdIdx]);
				if ((prCmdTbl->eAccessRight >= _eCliMode) &&
					(prCmdTbl->prCmdNextLevel == NULL))
				{
					_CLI_PrintCmd("",prCmdTbl);
				}

			}
		}
		for (u4Idx = CLI_MANDA_CMD_TBL_IDX; u4Idx < _u4CmdTblNum; u4Idx++) //recursive into directory
		{
			for (u4CmdIdx = 0;_aprCmdTblBuf[u4Idx][u4CmdIdx].pszCmdHelpStr != NULL;u4CmdIdx++)
			{
				prCmdTbl = &(_aprCmdTblBuf[u4Idx][u4CmdIdx]);
				if ((prCmdTbl->eAccessRight >= _eCliMode) &&
					(prCmdTbl->prCmdNextLevel != NULL))
				{
					pszStr = prCmdTbl->pszCmdStr;
					if (prCmdTbl->pszCmdAbbrStr!=NULL)
                    {
						pszStr = prCmdTbl->pszCmdAbbrStr;
                    }
					_CLI_RecursiveListCliCmd(pszStr,prCmdTbl->prCmdNextLevel,_eCliMode);
				}

			}
		}
	}
	else
	{
		_CLI_RecursiveListCliCmd("",_prCurrentCmdTbl,_eCliMode);
	}
	return CLI_COMMAND_OK;
}

#define MAX_STR_SIZE            256
/****************************************************************************
Function        : _CliShowHelp
Description     : Show CLI command table help string
*****************************************************************************/
static void _CliShowHelp(const CLI_EXEC_T * prCmdTbl)
{
    UINT32 u4CmdIdx;
    UINT32 u4Idx;

    CHAR szBuf[MAX_STR_SIZE];

	if (prCmdTbl == NULL)	// root command table
	{
		cli_print("[Help.]\r\n");
		for (u4Idx = CLI_MANDA_CMD_TBL_IDX; u4Idx < _u4CmdTblNum; u4Idx++)
		{
			u4CmdIdx = 0;

			while (_aprCmdTblBuf[u4Idx][u4CmdIdx].pszCmdHelpStr != NULL)
			//while (_aprCmdTblBuf[u4Idx][u4CmdIdx].ps_cmd_help_str != NULL)
			{
			    const CLI_EXEC_T* prCmd;

			    prCmd = &_aprCmdTblBuf[u4Idx][u4CmdIdx];

			    //VERIFY(_FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->pszCmdStr, prCmd->pszCmdAbbrStr, prCmd->pszCmdHelpStr));
			    _FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->pszCmdStr, prCmd->pszCmdAbbrStr, prCmd->pszCmdHelpStr);
			    //_FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->ps_cmd_str, prCmd->ps_cmd_abbr_str, prCmd->ps_cmd_help_str);

                if (((prCmd->prCmdNextLevel == NULL) || (prCmd->prCmdNextLevel->pszCmdStr != NULL)) && (prCmd->eAccessRight >= _eCliMode)) {
                //if (((prCmd->pt_next_level == NULL) || (prCmd->pt_next_level->ps_cmd_str != NULL)) && (prCmd->e_access_right >= _eCliMode)) {
			        cli_print("%s\r\n", szBuf);
                }

				u4CmdIdx++;
			}
		}
	}
	else
	{
		cli_print("[Help..]\r\n");

		u4CmdIdx = 0;
		while (prCmdTbl[u4CmdIdx].pszCmdHelpStr != NULL)
		//while (prCmdTbl[u4CmdIdx].ps_cmd_help_str != NULL)
		{
		    const CLI_EXEC_T* prCmd;

		    prCmd = &prCmdTbl[u4CmdIdx];

		    //VERIFY(_FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->pszCmdStr, prCmd->pszCmdAbbrStr, prCmd->pszCmdHelpStr));
		    _FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->pszCmdStr, prCmd->pszCmdAbbrStr, prCmd->pszCmdHelpStr);
		    //_FormatHelpString(szBuf, MAX_STR_SIZE, prCmd->ps_cmd_str, prCmd->ps_cmd_abbr_str, prCmd->ps_cmd_help_str);

            if (((prCmd->prCmdNextLevel == NULL) || (prCmd->prCmdNextLevel->pszCmdStr != NULL)) && (prCmd->eAccessRight >= _eCliMode)) {
            //if (((prCmd->pt_next_level == NULL) || (prCmd->pt_next_level->ps_cmd_str != NULL)) && (prCmd->e_access_right >= _eCliMode)) {
		        cli_print("%s\r\n", szBuf);
            }
			u4CmdIdx++;
		}
	}

}
//#endif
/****************************************************************************
Function        : _CliStringParser
Description     : String Parser
*****************************************************************************/
static INT32 _CliStringParser(const char *szCmdBuf, UINT32 u4ArgNum, UINT32 u4ArgLen, char * const * szArgv)
{
    INT32 i4Argc = 0;
    char cChar;
    char * pcStr;
    UINT32 u4Cnt;
    INT32 u4State; /* 1 is single quote, 2, is double quota. */

    if ((szCmdBuf == NULL) || (u4ArgNum == 0) || (u4ArgLen == 0) || (szArgv == NULL))
    {
        return 0;
    }

    cChar = *szCmdBuf;

    while (cChar != ASCII_NULL)
    {
        pcStr = szArgv[i4Argc];

        // skip space
        while (!IsPrintable(cChar) || IsSpace(cChar))
        {
            cChar = *(++szCmdBuf);
        }

        // copy non-space characters
        u4Cnt = 0;
        u4State = 0;
        while (IsPrintable(cChar) && (((u4State==0) && !IsSpace(cChar)) ||
                ((u4State==1) && (cChar!=ASCII_KEY_QUOTE)) ||
                ((u4State==2) && (cChar!=ASCII_KEY_DBL_QUOTE))))
        {

            if (cChar == ASCII_NULL)    // end of string
            {
                *pcStr = ASCII_NULL;    // end of string
                break;
            }

            if (u4Cnt >= (u4ArgLen - 1))    // maximum argument length
            {
                *pcStr = ASCII_NULL;    // end of string
                break;
            }

            if ((u4State!=2) && (cChar==0x27)) {
                if (u4State==0) {
                    u4State = 1;
                    cChar = *(++szCmdBuf);
                    u4Cnt++;
                } else if (u4State==1) {
                    u4State = 0;
                }
                continue;
            } else if ((u4State!=1) && (cChar==0x22)) {
                if (u4State==0) {
                    u4State = 2;
                    cChar = *(++szCmdBuf);
                    u4Cnt++;
                } else if (u4State==2) {
                    u4State = 0;
                }
                continue;
            }

            *pcStr = cChar;
            u4Cnt++;
            pcStr++;
            cChar = *(++szCmdBuf);
        }

        if ((cChar==ASCII_KEY_QUOTE) || (cChar==ASCII_KEY_DBL_QUOTE)) {
            cChar = *(++szCmdBuf);
            u4Cnt++;
        }

        if (u4Cnt > 0)
        {
            *pcStr = ASCII_NULL;    // end of string
            i4Argc++;
        }

        if ((UINT32)i4Argc >= u4ArgNum)
        {
            break;
        }
    }

    return i4Argc;
}
/****************************************************************************
Function        : _CliCmdSearch
Description     : Search Command
*****************************************************************************/
static CLI_EXEC_T* _CliCmdSearch(const char*  szArgv, CLI_EXEC_T * prCmdTbl)
{
    UINT32 u4CmdIdx;
    UINT32 u4StrLen;
    UINT32 u4CmdLen;
    UINT32 u4CmdAbbrLen;
    const char *szStr;
    UINT8 ucFound = 0;

    if ((szArgv == NULL) || (prCmdTbl == NULL))
    {
        return NULL;
    }

    szStr = szArgv;
    u4StrLen = 0;
    while (!IsDot(*szStr) &&
        (*szStr != ASCII_NULL))
    {
        szStr++;
        u4StrLen++;
    }

    // search commmad from command table. If found, run command
    u4CmdIdx = 0;
    while (prCmdTbl[u4CmdIdx].pszCmdStr != NULL)
    {
        if (prCmdTbl[u4CmdIdx].pszCmdAbbrStr != NULL)
        {
            u4CmdAbbrLen = strlen(prCmdTbl[u4CmdIdx].pszCmdAbbrStr);

            if ((u4StrLen == u4CmdAbbrLen) &&
                (strncmp(prCmdTbl[u4CmdIdx].pszCmdAbbrStr, szArgv, u4StrLen) == 0))
            {
                ucFound = 1;
                break;
            }
        }

        u4CmdLen = strlen(prCmdTbl[u4CmdIdx].pszCmdStr);

        if ((u4StrLen == u4CmdLen) &&
            (strncmp(prCmdTbl[u4CmdIdx].pszCmdStr, szArgv, u4StrLen) == 0))
        {
            ucFound = 1;
            break;
        }

        u4CmdIdx++;
    }

    // execute command
    if (ucFound)
    {
        return &prCmdTbl[u4CmdIdx];
    }

    return NULL;
}

/****************************************************************************
Function        : _CliCmdParser
Description     : Command Parser
*****************************************************************************/
static INT32 _CliCmdParser(INT32 i4Argc, const CHAR**  szArgv, CLI_EXEC_T * prCmdTbl, INT32 fgNoChk)
{
    const CHAR *szStr;
    CLI_EXEC_T *pCmdTbl;

    if ((szArgv == NULL) || (prCmdTbl == NULL))
    {
        return CLI_UNKNOWN_CMD;
    }

    szStr = szArgv[0];
    while (!IsDot(*szStr) &&
        (*szStr != ASCII_NULL))
    {
        szStr++;
    }

    pCmdTbl = _CliCmdSearch(szArgv[0], prCmdTbl);

    // execute command
    if ((pCmdTbl != NULL) &&
        (fgNoChk || (pCmdTbl->eAccessRight >= _eCliMode)))
    {
        if ((pCmdTbl->prCmdNextLevel == NULL) && (pCmdTbl->pfExecFun == NULL))
        {
            //ASSERT(0);
            assert(0);
        }

        if (pCmdTbl->pfExecFun != NULL)
        {
            if ((pCmdTbl->prCmdNextLevel == NULL) ||
                (!IsDot(*szStr) && (i4Argc > 1)))       // entry can be a directory or a command
            {
                INT32 i4Return = pCmdTbl->pfExecFun(i4Argc, szArgv);

                return i4Return;
            }
        }

        if (pCmdTbl->prCmdNextLevel != NULL)
        {
            if (*szStr == ASCII_NULL)
            {
                // cli help
                _CliShowHelp(pCmdTbl->prCmdNextLevel);
                return CLI_COMMAND_OK;
            }

            szStr++;
            szArgv[0] = szStr;  // skip dot
            return _CliCmdParser(i4Argc, szArgv, pCmdTbl->prCmdNextLevel, fgNoChk); // go to next level
        }
    }

    return CLI_UNKNOWN_CMD;
}

//-----------------------------------------------------------------------------
/**
 *
 *  @param
 *  @retval
 */
//-----------------------------------------------------------------------------

/******************************************************************************
* Function      : CLI_CmdChangeDirectory
* Description   : CLI Command to change current command level
******************************************************************************/
INT32 CLI_CmdChangeDirectory(INT32 i4Argc, const char ** szArgv)
{
    const char *szStr;
    const char *szCmd;
    UINT32 u4Idx;
    CLI_EXEC_T* pCmdTbl = NULL;

    if ((i4Argc < 2) || (szArgv == NULL) || (szArgv[1] == NULL))    // show alias
    {
        _CliShowHelp(_prCurrentCmdTbl);
        return 1;
    }

    szStr = szArgv[1];

    while (*szStr != ASCII_NULL)
    {
        if (IsDot(*szStr))
        {
            szStr++;
            if (IsDot(*szStr))  // cd ..
            {
                szStr++;
                if (_u4CmdTblLinkIdx > 0)
                {
                    _u4CmdTblLinkIdx --;

                    if (_u4CmdTblLinkIdx == 0)
                    {
                        _prCurrentCmdTbl = NULL;
                    }
                    else
                    {
                        _prCurrentCmdTbl = _aprCmdTblLinkBuf[_u4CmdTblLinkIdx]->prCmdNextLevel;
                    }
                    _CliGeneratePrompt();
                }
            }
            else
            {
                return -1;
            }
        }
        else
        if (IsRoot(*szStr))     // cd root
        {
            szStr++;

            _u4CmdTblLinkIdx = 0;
            _prCurrentCmdTbl = NULL;
            _CliGeneratePrompt();
        }
        else
        {
            szCmd = szStr;
            while (!IsDot(*szStr) &&
                (*szStr != ASCII_NULL))
            {
                szStr++;
            }

            if (_prCurrentCmdTbl == NULL)   // root
            {
				for (u4Idx = (CLI_MANDA_CMD_TBL_IDX + 1); u4Idx < _u4CmdTblNum; u4Idx++)
				{
					pCmdTbl = _CliCmdSearch(szCmd, _aprCmdTblBuf[u4Idx]);

					if (pCmdTbl != NULL)
					{
						break;
					}
				}
            }
            else
            {
                pCmdTbl = _CliCmdSearch(szCmd, _prCurrentCmdTbl);
            }

            if ((pCmdTbl != NULL) && (pCmdTbl->prCmdNextLevel != NULL) &&
                (pCmdTbl->eAccessRight >= _eCliMode))
            {
                if (_u4CmdTblLinkIdx < (CLI_MAX_CMD_TBL_LEVEL - 2))
                {
                    _u4CmdTblLinkIdx++;
                    _aprCmdTblLinkBuf[_u4CmdTblLinkIdx] = pCmdTbl;
                    _prCurrentCmdTbl = _aprCmdTblLinkBuf[_u4CmdTblLinkIdx]->prCmdNextLevel;

                    _CliGeneratePrompt();
                }
                else
                {
                    cli_print("CLI fatal error!\n");
                    while(1) {}
                }
            }
            else
            {
                return -1;
            }

            if (IsDot(*szStr))
            {
                szStr++;    // skip 'dot' character
            }
        }
    }

    return 1;
}

//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------
/******************************************************************************
* Function      : CLI_CmdTblClear
* Description   : cli command table clear
******************************************************************************/
void CLI_CmdTblClear(void)
{
    UINT32 u4TblIdx;

    for (u4TblIdx = 0; u4TblIdx < CLI_INPUT_MAX_CMD_TBL_NUM; u4TblIdx++)
    {
        _aprCmdTblBuf[u4TblIdx] = NULL;
    }

    _u4CmdTblNum = 0;
    //VERIFY( CLI_CmdTblAttach((CLI_EXEC_T *)_arMandatoryCmdTbl) != -1);
    CLI_CmdTblAttach((CLI_EXEC_T *)_arMandatoryCmdTbl);

    _aprCmdTblLinkBuf[0] = NULL;
    _u4CmdTblLinkIdx = 0;
    _prCurrentCmdTbl = NULL;
}

/******************************************************************************
* Function      : _CliGeneratePrompt
* Description   : Generate CLI prompt string according to current directory
******************************************************************************/
static void _CliGeneratePrompt(void)
{
    UINT32 u4Idx;
    UINT32 u4TotalLen;
    UINT32 u4StrLen;
    char* szCmd;

    _szCliPromptStr[0] = ASCII_NULL;
    u4TotalLen = 0;
    for (u4Idx = 1; u4Idx <= _u4CmdTblLinkIdx; u4Idx++)
    {
        if (_aprCmdTblLinkBuf[u4Idx]->pszCmdAbbrStr != NULL)
        {
            szCmd = _aprCmdTblLinkBuf[u4Idx]->pszCmdAbbrStr;
        }
        else
        {
            szCmd = _aprCmdTblLinkBuf[u4Idx]->pszCmdStr;
        }

        u4StrLen = strlen(szCmd);

        if ((u4TotalLen + u4StrLen + 1) > CLI_INPUT_BUF_SIZE)
        {
            cli_print("CLI fatal error!\n");
            while(1) {}
        }

        strncat(_szCliPromptStr, szCmd, (u4StrLen + 1));
        if (u4StrLen)
        	strncat(_szCliPromptStr, ".", 2);


        u4TotalLen += (u4StrLen + 1);
    }
}

/****************************************************************************
Function        : CLI_ParserSetPromptStr
Description     : Set Prompt String Pointer
*****************************************************************************/
void CLI_ParserSetPromptStr(char* szPtr)
{
    _szCliPromptStr = szPtr;
}

//-----------------------------------------------------------------------------
/**
 *
 *  @param
 *  @param
 *  @param
 *  @retval
 *  @retval
 */
//-----------------------------------------------------------------------------
/******************************************************************************
* Function      : CLI_CmdTblAttach(CLI_EXEC_T* pTbl, UINT32 u4Len)
* Description   : cli command table attach
******************************************************************************/
INT32 CLI_CmdTblAttach(CLI_EXEC_T* pTbl)
{
    if (_u4CmdTblNum >= CLI_INPUT_MAX_CMD_TBL_NUM)
    {
        cli_print("CLI_CmdTblAttach(): _u4CmdTblNum >= CLI_INPUT_MAX_CMD_TBL_NUM\r\n");
        return -1;
    }

    if (pTbl == NULL)
    {
        cli_print("pTbl == NULL\r\n");
        return -1;
    }

    _aprCmdTblBuf[_u4CmdTblNum] = pTbl;
    _u4CmdTblNum++;

    return 0;
}

/****************************************************************************
Function        : CLI_Parser
Description     : CLI Parer main function
*****************************************************************************/
INT32 CLI_PromptParser(const char *szCmdBuf)
{
    INT32 i4ReturnValue;
    INT32 i4Argc;
    UINT32 u4Argc,i;
    UINT32 u4InCmdLen;
    char* aszTemp[CLI_MAX_ARGU];


    char buf[200];


    if (szCmdBuf == NULL)
    {
        return CLI_UNKNOWN_CMD;
    }
    // prepare argument
    for (u4InCmdLen = 0; u4InCmdLen < CLI_MAX_ARGU; u4InCmdLen++)
    {
        aszTemp[u4InCmdLen] = _aszArgv[u4InCmdLen];
    }

    i4Argc = _CliStringParser(szCmdBuf, CLI_MAX_ARGU, CLI_MAX_ARGU_LEN, aszTemp);
    u4Argc = (UINT32) i4Argc;

    cli_print("argc=%d,  ", (int)u4Argc);
    for(i=0;i<u4Argc;i++){
        cli_print("argv(%d)=%s ",(int)i,aszTemp[i]);
    }
    cli_print("\r\n");

    if (u4Argc > 0)
    {
		#if 0 //do not porting alias related code
        // alias handle, replace alias with corresponding string
        u4InCmdLen = strlen(aszTemp[0]);
        u4CmdLen = strlen(CLI_ALIAS_CMD_STR);
        u4CmdAbbrLen = strlen(CLI_ALIAS_CMD_ABBR_STR);

        if (!(((u4InCmdLen == u4CmdLen) && (x_strncmp(aszTemp[0], CLI_ALIAS_CMD_STR, u4InCmdLen) == 0)) ||
            ((u4InCmdLen == u4CmdAbbrLen) && (x_strncmp(aszTemp[0], CLI_ALIAS_CMD_ABBR_STR, u4InCmdLen) == 0))))
        {
            UINT32 u4Idx;
            u4Idx = 0;
            while (u4Idx < u4Argc)
            {
                pszAliasString = CLI_AliasCompare(aszTemp[u4Idx]);

                if (pszAliasString)
                {
                    UINT32 u4AliasIdx;
                    UINT32 u4AliasArgc;
                    CHAR* aszAliasTemp[CLI_MAX_ARGU];

                    // prepare alias argument
                    for (u4AliasIdx = 0; u4AliasIdx < CLI_MAX_ARGU; u4AliasIdx++)
                    {
                        aszAliasTemp[u4AliasIdx] = _aszAliasArgv[u4AliasIdx];
                    }

                    u4AliasArgc = (UINT32) _CliStringParser(pszAliasString, CLI_MAX_ARGU, CLI_MAX_ARGU_LEN, aszAliasTemp);

                    // too many argument after alias parser
                    if (u4AliasArgc > (CLI_MAX_ARGU - (u4Argc + 1)))
                    {
                        u4AliasArgc = CLI_MAX_ARGU - (u4Argc + 1);
                    }

                    // move back string argument after alias
                    for (u4AliasIdx = (u4Argc - 1); u4AliasIdx > u4Idx; u4AliasIdx--)
                    {
                        VERIFY(x_strncpy(aszTemp[u4AliasIdx + (u4AliasArgc - 1)],
                                aszTemp[u4AliasIdx],
                                CLI_MAX_ARGU_LEN) == aszTemp[u4AliasIdx + (u4AliasArgc - 1)]);
                    }

                    // copy alias string arguemnt
                    for (u4AliasIdx = 0; u4AliasIdx < u4AliasArgc; u4AliasIdx++)
                    {
                        VERIFY(x_strncpy(aszTemp[u4Idx+u4AliasIdx],
                                aszAliasTemp[u4AliasIdx],
                                CLI_MAX_ARGU_LEN) == aszTemp[u4Idx+u4AliasIdx]);
                    }

                    u4Argc += (u4AliasArgc - 1);
                    u4Idx--;
                }
                u4Idx++;
            }
        }
		#endif

        i4Argc = (INT32) u4Argc;

        // command parsing, for all other user command table
        if (i4Argc > 0)
        {
        	//IOCTL_WRITE command string
			#if 1
        	//command
          	_CliGeneratePrompt();
			strcpy(buf, _szCliPromptStr);
			strcat(buf, _aszArgv[0]);

			//arguments
			for (i = 1; i < u4Argc; i++)
			{
				strcat(buf, " ");
				strcat(buf, _aszArgv[i]);

			}
			strcat(buf, "\0");
			#endif

            if (_prCurrentCmdTbl == NULL)   // root
            {
				UINT32 u4Idx;

				for (u4Idx = (CLI_MANDA_CMD_TBL_IDX + 1); u4Idx < _u4CmdTblNum; u4Idx++)
				{
					i4ReturnValue = _CliCmdParser(i4Argc, (const CHAR**)aszTemp, _aprCmdTblBuf[u4Idx], 0);
					if (i4ReturnValue != CLI_UNKNOWN_CMD)
					{
						//IOCTL_WRITE command when arguments are valid
						i4ReturnValue = write_cmd(buf);

						return i4ReturnValue;
					}
				}
            }
            else                            // sub-directory
            {
                i4ReturnValue = _CliCmdParser(i4Argc, (const CHAR**)aszTemp, _prCurrentCmdTbl, 0);
                if (i4ReturnValue != CLI_UNKNOWN_CMD)
                {
                	//IOCTL_WRITE command when arguments are valid
					i4ReturnValue = write_cmd(buf);

                    return i4ReturnValue;
                }
            }
        }

        // command parsing, mandatory table
        i4ReturnValue = _CliCmdParser(i4Argc, (const CHAR**)aszTemp, (CLI_EXEC_T *)_arMandatoryCmdTbl, 0);
        if (i4ReturnValue != CLI_UNKNOWN_CMD)
        {
            return i4ReturnValue;
        }
    }

    // cli help
    _CliShowHelp(_prCurrentCmdTbl);

    return CLI_UNKNOWN_CMD;
//    return 0;
}


int CLI_Parser_Arg(char *fmt,...)
{
	UINT16 len;
	va_list ap;
    CHAR szBuf[64];
    UINT32 i4ReturnValue;

	va_start (ap, fmt);
	len=vsnprintf(szBuf,64,fmt,ap);
	va_end (ap);

    i4ReturnValue = CLI_PromptParser(&szBuf[0]);
	if ((i4ReturnValue < CLI_COMMAND_OK) && (i4ReturnValue != CLI_UNKNOWN_CMD)){
		cli_print("CLI_Parser_Arg Return Value (%d)\n", (int)i4ReturnValue);
	}
	return len;
}
