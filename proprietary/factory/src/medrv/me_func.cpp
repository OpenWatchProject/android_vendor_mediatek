#include "me_comm.h"
#include "me_func_cb.h"

typedef void (*ME_RspCallBack) (void *pData);

#define FILLREQUEST(a, b, c, d)	\
do{								\
	a->req.reply = me_respond_ready;\
	a->req.atproc  = c;			\
	a->req.nFsmState = FSM_STATE_START;\
	a->req.event = AT_RESPONSE;	\
	a->req.context = d;			\
	a->pRspObj = NULL;			\
}while(0)

void TimeoutProc(ME_HANDLE handle)
{
	PMEFSM pfsm = (PMEFSM)handle;
	if((pfsm->req.atproc != NULL) && (pfsm->req.nFsmState != FSM_STATE_DONE))
	{
		pfsm->req.event = AT_TIMEOUT;
		ME_Print(ME_TAG "[Medrv] time out proc -> todo");
		pfsm->req.atproc(pfsm);
		ME_Print(ME_TAG "[Medrv] time out proc done");
	}
	ME_Print(ME_TAG "[Medrv] Leave time out proc");
}

NTSTATUS TX_Callback(NTSTATUS status, int len, void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;
    if (len == 0) // reserved parameter, for resolving warning
    {
        ME_Print(ME_TAG "[Medrv] Warning: len =0\n");
	}
	pfsm->bWriteDone = OSAL_TRUE;
	if(status != STATUS_SUCCESS)
	{
		ME_Print(ME_TAG "[Medrv] Enter TX_Callback, write failed");
		pfsm->req.event = AT_WRITEFAIL;
		pfsm->req.atproc(pfsm);
		ME_Print(ME_TAG "[Medrv] Leave TX_Callback");
	}
	else
	{
		if((pfsm->req.nFsmState == FSM_STATE_DONE) && (pfsm->req.atproc != NULL))
		{
			ME_Print(ME_TAG "[Medrv] Enter TX_Callback, write successful");
			pfsm->req.atproc(pfsm);
			ME_Print(ME_TAG "[Medrv] Leave TX_Callback");
		}
	}

	return 1;
}


ME_HANDLE ME_Init(void* ioproxy)
{
	PMEFSM pfsm = (PMEFSM )me_malloc(sizeof(MEFSM));
	if(pfsm == NULL)
		return NULL;
	pfsm->state = 0;
	pfsm->nHead = 0;
	pfsm->nTail = 0;
	pfsm->nReadIdx  = 0;
	pfsm->nWriteIdx = 0;
	pfsm->nUrcId = -1;
	pfsm->bWaitPdu = OSAL_FALSE;
	pfsm->bReadPdu = OSAL_FALSE;
	pfsm->bWriteDone = OSAL_FALSE;
	pfsm->nUrcState = FSM_STATE_START;
	pfsm->ioctl.ioproxy = ioproxy;
	pfsm->ioctl.notify = TX_Callback;
	pfsm->pRspObj = NULL;
	pfsm->req.atproc = NULL;
	pfsm->req.nFsmState = FSM_STATE_DONE;

	me_initlock(&pfsm->Lock);
	me_initlock(&pfsm->LockSending);


	pfsm->Timer.timer_cb = TimeoutProc;
	pfsm->Timer.pData = (void *)pfsm;
	me_inittimer(&pfsm->Timer);

	pfsm->urcproc = ME_URC_Parser;
	for(int i=0; i<ID_MAX; i++)
	{
		pfsm->urclist[i].reply = NULL;
		pfsm->urclist[i].context = NULL;
	}

	pfsm->req.hBlockEvt = CreateEvent(NULL, OSAL_FALSE, OSAL_FALSE, NULL);
	ResetEvent(pfsm->req.hBlockEvt);

	return (ME_HANDLE)pfsm;
}

BOOLEAN ME_DeInit(ME_HANDLE handle)
{
	if(handle != NULL)
	{
		ME_Print(ME_TAG "[Medrv] Enter ME_DeInit");
		PMEFSM pfsm = (PMEFSM)handle;

		if(pfsm->req.nFsmState != FSM_STATE_DONE)
		{
			ME_Print(ME_TAG "[Medrv] Leave ME_DeInit -> not done");
			return OSAL_FALSE;
		}

		me_exittimer(&pfsm->Timer);
		me_free(handle);
		ME_Print(ME_TAG "[Medrv] Leave ME_DeInit");

		return OSAL_TRUE;
	}

	return OSAL_FALSE;
}

BOOLEAN	ME_Cancel(ME_HANDLE handle)
{
	if(handle != NULL)
	{
		PMEFSM pfsm = (PMEFSM)handle;
		ME_Print(ME_TAG "[Medrv] Enter ME_Cancel");

		me_lock(&pfsm->Lock); 
		if (pfsm->req.atproc != NULL)
		{
			pfsm->req.event = AT_CANCEL;
			pfsm->req.atproc(pfsm);
			ME_Print(ME_TAG "[Medrv] Cancel proc done");
		}

		me_unlock(&pfsm->Lock);
		ME_Print(ME_TAG "[Medrv] Leave ME_Cancel");

		return OSAL_TRUE;
	}

	return OSAL_FALSE;
}

NTSTATUS SendCommand(PMEFSM pfsm, const char *cmd)
{
	NTSTATUS status;
	pfsm->bWriteDone = OSAL_FALSE;

	status = me_send_at(pfsm->ioctl.ioproxy, cmd, pfsm->ioctl.notify, pfsm);
	ME_Print(ME_TAG "[Medrv] SendCommand completed......\n");
	return status;
}

NTSTATUS ME_Fill_FSM(ME_HANDLE handle, ME_Callback reply, void* context, 
					 ME_RspCallBack rspcb, const char *cmd, const char *exCmd, int timout)
{
	PMEFSM pfsm = NULL;
	if (reply == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] reply is NULL");
	}
	if(handle == NULL)
		return STATUS_INVALID_PARAMETER;

	pfsm = (PMEFSM)handle;
//	me_lock(&pfsm->LockSending); 

	if(pfsm->req.nFsmState != FSM_STATE_DONE)
		return STATUS_INVALID_PARAMETER;

	FILLREQUEST(pfsm, reply, rspcb, context);

//	me_strcpy(pfsm->req.cmd, cmd);
	if(exCmd != NULL)
		me_strcpy(pfsm->req.expectedCmd, exCmd);	
	else
		pfsm->req.expectedCmd[0] = '\0';

	ME_Print(ME_TAG "[Medrv] To set timer: <AT>%s,  timeout = %d\n", cmd, timout);
	me_settimer(&pfsm->Timer, timout);

	if(STATUS_SUCCESS != SendCommand(pfsm, cmd))
		return 100;//ER_UNKNOWN;
//	me_unlock(&pfsm->LockSending);

	return me_get_rettype(handle);
}

/////////////////////////////////////////////////////URC////////////////////////////////////////////

void ME_RegisterURC(ME_HANDLE handle, int nID/*URC_ID*/, ME_Callback reply, void *context)
{
	PMEFSM pfsm;
	if (nID == 0) // reserved parameter, for resolving waring
	{
        ME_Print(ME_TAG "[Medrv] Warning: nID =0\n");
	}
	if(context == NULL) // reserved parameter, for resolving waring
	{
        ME_Print(ME_TAG "[Medrv] context is NULL");
	}
	if(handle == NULL)
		return;
	pfsm = (PMEFSM)handle;

	for(int i=ID_ECSQ; i<ID_MAX; i++)
		pfsm->urclist[i].reply = reply;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//AT+CFUN=
NTSTATUS ME_Set_ModemFunctionality(ME_HANDLE handle, int fun, int rst, int common, ME_Callback reply, void* context)
{	
	//common 0:standard command, 1:MTK command

	char cmd[AT_CMD_LEN] = {0};
	char c = 0;

	if(common == 0)
		c = 'C';
	else
		c = 'E';

	if(rst < 0)
		me_sprintf(cmd, "AT+%cFUN=%d\r", c, fun);
	else
		me_sprintf(cmd, "AT+%cFUN=%d,%d\r", c, fun, rst);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE*3);
}

//AT+ESLP
NTSTATUS ME_Set_SleepMode(ME_HANDLE handle, int mode, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};

	me_sprintf(cmd, "AT+ESLP=%d\r", mode);
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+ESUO
NTSTATUS ME_Set_UartOwner(ME_HANDLE handle, int mode, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};

	me_sprintf(cmd, "AT+ESUO=%d\r", mode);
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CREG=
NTSTATUS ME_Set_NetwrokReg(ME_HANDLE handle, int mode, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd, "AT+CREG=%d\r", mode);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+ECPI=
NTSTATUS ME_Set_CallInfo(ME_HANDLE handle, char* value, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd, "AT+ECPI=%s\r", value);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+EGTYPE=
NTSTATUS ME_Set_GPRSAttachMode(ME_HANDLE handle, int mode, int act, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};

	if (act >= 0)
		me_sprintf(cmd, "AT+EGTYPE=%d,%d\r", mode, act);
	else
		me_sprintf(cmd, "AT+EGTYPE=%d\r", mode);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//ATE0Q0V1
NTSTATUS ME_Set_ModemFunc(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "ATE0Q0V1\r", NULL, SHORT_TIMEOUT_ELAPSE);
}


//AT
NTSTATUS ME_Query_ModemStatus(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "AT\r", NULL, SHORT_TIMEOUT_ELAPSE);	
}

//ATH
NTSTATUS ME_Terminate_Call(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "ATH\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//ATD
NTSTATUS ME_MakeMO_Call(ME_HANDLE handle, const char *number, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd,"ATD%s;\r", number);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);	
}

//AT+ESIMS
NTSTATUS ME_Query_SIMStatus(ME_HANDLE handle, ME_Callback reply, void* context)
{
	const char *cmd   = "AT+ESIMS\r"; 
	const char *exCmd = "+ESIMS";

	return ME_Fill_FSM(handle, reply, context, ME_Query_SIMStatus_CB, cmd, exCmd, MIDDLE_TIMEOUT_ELAPSE);
}

//AT+EGMR
NTSTATUS ME_Query_Revision(ME_HANDLE handle, int type, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd, "AT+EGMR=0,%d\r", type);
	const char *exCmd = "+EGMR";

	return ME_Fill_FSM(handle, reply, context, ME_Query_Revision_CB, cmd, exCmd, SHORT_TIMEOUT_ELAPSE);	

}

//AT+EGMR=
NTSTATUS ME_Set_Revision(ME_HANDLE handle, int type, char *str, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN];
	me_sprintf(cmd, "AT+EGMR=1,%d,\"%s\"\r", type, str);
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);	
}

//AT+CIMI
NTSTATUS ME_Query_IMSI(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Query_IMSI_CB, "AT+CIMI\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CGMR
NTSTATUS ME_Query_FWVersion(ME_HANDLE handle, ME_Callback reply, void* context)
{
	const char *cmd   = "AT+CGMR\r";
	const char *exCmd = "+CGMR";

	return ME_Fill_FSM(handle, reply, context, ME_Query_FWVersion_CB, cmd, exCmd, SHORT_TIMEOUT_ELAPSE);
}
////////////////////////////////////////For C2K //////////////////////////////
//ATZ
NTSTATUS ME_Reset_Config(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "ATZ\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CPOF
NTSTATUS ME_TurnOff_Phone(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "AT+CPOF\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CPON
NTSTATUS ME_TurnOn_Phone(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "AT+CPON\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+EMDSTATUS
NTSTATUS ME_EMDSTATUS(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "AT+EMDSTATUS=1,1\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+ERAT=6
NTSTATUS ME_Change_Modem(ME_HANDLE handle, ME_Callback reply, void* context, int mode)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd,"AT+ERAT=%d\r", mode);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CHV
NTSTATUS ME_HangUpC2K(ME_HANDLE handle, ME_Callback reply, void* context)
{
	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, "AT+CHV\r", NULL, SHORT_TIMEOUT_ELAPSE);
}

//AT+CDV=112
NTSTATUS ME_MakeC2KMO_Call(ME_HANDLE handle, const char *number, ME_Callback reply, void* context)
{
	char cmd[AT_CMD_LEN] = {0};
	me_sprintf(cmd,"AT+CDV=%s\r", number);

	return ME_Fill_FSM(handle, reply, context, ME_Set_ModemFunc_CB, cmd, NULL, SHORT_TIMEOUT_ELAPSE);	
}

//AT+CSQ
NTSTATUS ME_DetectSignal(ME_HANDLE handle, ME_Callback reply, void* context)
{
	const char *cmd   = "AT+CSQ\r";
	const char *exCmd = "+CSQ";
	return ME_Fill_FSM(handle, reply, context, ME_Query_Signal_CB, cmd, exCmd, SHORT_TIMEOUT_ELAPSE);
}

//AT^MEID
NTSTATUS ME_Query_MEID(ME_HANDLE handle, ME_Callback reply, void* context)
{
	const char *cmd   = "AT^MEID\r";
	const char *exCmd = "^MEID";

	return ME_Fill_FSM(handle, reply, context, ME_Query_FWVersion_CB, cmd, exCmd, SHORT_TIMEOUT_ELAPSE);
}

NTSTATUS ME_Query_3GStatus(ME_HANDLE handle, ME_Callback reply, void* context)
{
    const char *cmd   = "AT+ES3G?\r";
	const char *exCmd = "+ES3G";
	return ME_Fill_FSM(handle, reply, context, ME_Query_3G_Status_CB, cmd, exCmd, SHORT_TIMEOUT_ELAPSE);
}
