#include "me_comm.h"
#include "me_func_cb.h"
#include "me_parser.h"
#include "me_result.h"

typedef enum rsp_type
{
	RSP_OK,
	RSP_ERROR,
	RSP_CMEERROR,
	RSP_CMSERROR,
	RSP_CONNECT
	
}RSPTYPE;

typedef enum rsp_parser_state
{
	PARSER_DOING,
	PARSER_DONE,
	PARSER_EXIT

}RSPPARSERSTATE;
 
const char *RSP_String[] = 
{
	"OK",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
	"CONNECT",
	NULL
};

URCTABLE urc_proc_t[] = 
{
	{ID_ECSQ,		ME_URC_ECSQ_Parser},
	{ID_CREG,		ME_URC_CREG_Parser},
	{ID_CGREG,		ME_URC_CGREG_Parser},
	{ID_PSBEARER,	ME_URC_PSBEARER_Parser},
	{ID_EIND,		ME_URC_EIND_Parser},
	{ID_RING,		ME_URC_RING_Parser},
	{ID_ESIMS,		ME_URC_ESIMS_Parser},
	{ID_ECPI,		ME_URC_ECPI_Parser},
	{ID_ESPEECH,	ME_URC_ESPEECH_Parser},
	{ID_VPUP,		ME_URC_VPUP_Parser},
  {ID_CONN,		ME_URC_CONN_Parser},
  {ID_VPON,		ME_URC_VPON_Parser},
  {ID_EUSIM,		ME_URC_EUSIM_Parser},
  {ID_UIMST,		ME_URC_UIMST_Parser},
	{ID_MAX, NULL}
};

int ME_URC_Parser(void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;
	int i = 0;
	
	if(pfsm == NULL)
		return 0;
	
	while(urc_proc_t[i].proc!= NULL)
	{
		if(urc_proc_t[i++].proc(pfsm) != 0)
			return 1;
	}
	
	return 0;
}

static int IsFinalResultCode(char* buf, unsigned int buflen)
{
	int i = 0;

	if ( buflen == 0 )
		return -1;
	
	while (RSP_String[i] != 0)
	{
		if (buflen < me_strlen(RSP_String[i]))
		{
			i++;
			continue;
		}
		
		if (me_memcmp(RSP_String[i], buf, me_strlen(RSP_String[i])) == 0 )
			return i;
		
		i++;
	}
	
	return -1;
}

static int IsExpectedCmd(PMEFSM pfsm)
{
	if(pfsm->req.expectedCmd[0] == 0)
		return 0;

	return me_memcmp(pfsm->req.expectedCmd, pfsm->ctldata, me_strlen(pfsm->req.expectedCmd));
}

static int GetErrorCode(char* buf, unsigned int buflen)
{
	int retval = IsFinalResultCode(buf, buflen);

	switch(retval)
	{
	case RSP_OK:
		retval = ER_OK;
		break;
	case RSP_ERROR:
		retval = ER_UNKNOWN;
		break;
	case RSP_CMEERROR:
		retval = ER_CMEERROR;
		break;
	case RSP_CMSERROR:
		retval = ER_CMEERROR;
		break;
	case RSP_CONNECT:
		retval = ER_OK;
		break;
	}
	
	return retval;
}

int GetCMEErrorNum(int err, PMEFSM pfsm)
{
	if(err == ER_CMEERROR)
	{
		int begin;
		begin = GetSignIdx(pfsm->ctldata, ':', 1);
		return GetInteger(pfsm->ctldata, begin, me_strlen(pfsm->ctldata));
	}
	
	return err;
}

static int AnalyingRespString(PMEFSM pfsm, int *errcode)
{
	int retval;

	if(pfsm->req.nFsmState == FSM_STATE_DONE)
		return PARSER_EXIT;

	switch(pfsm->req.event)
	{
		case AT_RESPONSE:
			{
				retval = GetErrorCode(pfsm->ctldata, sizeof(pfsm->ctldata));
				if(retval < 0)
				{
					if(!IsExpectedCmd(pfsm))
					{
						//parse data
						return PARSER_DOING;
					}
					else
					{
						//URC or other error response
						return PARSER_EXIT;
					}	
				}
				else
				{
					//final result code (e.g. OK, ERROR, +CME ERROR)
					*errcode = GetCMEErrorNum(retval, pfsm);
				}
			}
			break;
		case AT_TIMEOUT:
			{
				ME_Print(ME_TAG "[Medrv] time out\r\n");
				*errcode = ER_TIMEOUT;
			}
			break;
		case AT_WRITEFAIL:
			{
				ME_Print(ME_TAG "[Medrv] failed to write");
				*errcode = ER_UNKNOWN;
			}
			break;
		case AT_CANCEL:
			{
				ME_Print(ME_TAG "[Medrv] cancel operation");
				*errcode = ER_USERABORT;
			}
			break;
	}

	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	pRet->retType = *errcode;

	return PARSER_DONE;
}

void ME_Singleline_Response_Parser(void *pData, PARSERFUNC func)
{
	PMEFSM pfsm = NULL;
	int errcode;
	
	pfsm = (PMEFSM)pData;	

	ME_Print(ME_TAG "[Medrv] Begin lock AT proc#################");
	me_lock(&pfsm->Lock);	

	ME_Print(ME_TAG "[Medrv] AT proc locking**************");
	if(pfsm->req.nFsmState != FSM_STATE_DONE)
	{
		switch(AnalyingRespString(pfsm, &errcode))
		{
		case PARSER_DOING:
			{
				if(func != NULL)
				{
					func(pfsm, NULL);
				}
                                //notice there is NO break
			}
		case PARSER_EXIT: 
			{
				ME_Print(ME_TAG "[Medrv] End lock AT proc#################");
				me_unlock(&pfsm->Lock);
				return;
			}
		case PARSER_DONE:
			{
				pfsm->req.nFsmState = FSM_STATE_DONE;
				ME_Print(ME_TAG "[Medrv] To kill timer");
				me_killtimer(&pfsm->Timer, pfsm->req.event);				
				break;
			}
		}//end switch
		
	}
	
	if(pfsm->bWriteDone)
	{
//		me_lock(&pfsm->LockSending); 
		pfsm->req.atproc = NULL;	
//		me_unlock(&pfsm->LockSending); 

		pfsm->req.reply(pData);
	}
	
	ME_Print(ME_TAG "[Medrv] End lock AT proc#################");
	me_unlock(&pfsm->Lock);
}

void ME_Set_ModemFunc_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, NULL);
}

void ME_Query_SIMStatus_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_SIMStatus_Parser);
}

void ME_Query_IMSI_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_IMSI_Parser);
}

void ME_Query_FWVersion_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_FWVersion_Parser);
}

void ME_Query_Revision_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_Revision_Parser);
}

void ME_Query_Signal_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_Signal_Parser);
}
void ME_Query_3G_Status_CB(void *pData)
{
	ME_Singleline_Response_Parser(pData, ME_Query_3G_Status_Parser);
}
