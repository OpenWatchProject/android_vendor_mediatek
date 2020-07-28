#ifndef _ME_COMM_H
#define _ME_COMM_H

#include "me_osal.h"

typedef void (*ME_ATInternalProc) (void *pData);
typedef int (*ME_URCInternalProc) (void *pData);

typedef enum me_fsm_event 
{
	AT_RESPONSE, 
	AT_TIMEOUT,
	AT_WRITEFAIL,
	AT_CANCEL

}FSMEVENT;

typedef enum me_fsm_state
{
	FSM_STATE_START, 
	FSM_STATE_STEP1,
	FSM_STATE_STEP2,
	FSM_STATE_STEP3,
	FSM_STATE_DONE

}FSFSTATE;


typedef struct tagURCTABLE
{
	int		id;
	ME_URCInternalProc	proc;
}URCTABLE;

typedef struct tagMEREQUEST
{
//	char	cmd[AT_CMD_LEN];
	char	expectedCmd[EXP_CMD_LEN];
	int		nFsmState;
	int		event;
	void	*context;   //at result
	OSAL_HANDLE  hBlockEvt;
	ME_ATInternalProc atproc;	//internal AT response proc
	ME_Callback reply;			//AT response callback
	
}MEREQUEST;

typedef struct tagURCLIST
{
	ME_Callback reply;			//urc callback
	void	*context;

}URCLIST;

typedef struct tagIO_CTL
{
	void *ioproxy; //for irp or com port.
	ME_TX_NOTIFY notify;
}IO_CTL;

typedef struct tagMEFSM
{
	// for parse data
	int state;
	int nHead;
	int nTail;
	int nReadIdx;
	int nWriteIdx;	
	int	nUrcState;
	int nUrcId;
	char rawdata[RAW_DATA_LEN];
	char ctldata[CTL_DATA_LEN];
	char pdu[CTL_DATA_LEN];
	BOOLEAN	bWaitPdu;
	BOOLEAN	bReadPdu;
	BOOLEAN bWriteDone;
	
	// for AT processing
	MEREQUEST req;
	OSALTIMER Timer;
	OSALLOCK  Lock;
	IO_CTL	  ioctl;
	void	*pRspObj;   // for AT response
	void	*pUrcObj;	// Only for CDS and CMT
	
	// for cancel and send AT operation
	OSALLOCK  LockSending;

	// for URC processing
	URCLIST urclist[ID_MAX];
	ME_URCInternalProc urcproc;	//internal urc proc
	
}MEFSM, *PMEFSM;

int GetHeader(char *header, char *src);
int GetSignIdx(char *src, char ch, int seq);
int GetInteger(char *src, int star, int end);
void GetString(char *src, char *dst, int star, int end);
int GetSignCount(char *src, char ch);
void trimright(char *src);
int DataCallBack(NTSTATUS status, const PUCHAR pTxt, ME_ULONG nLen, void *pData);

#endif