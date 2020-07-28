#ifndef _ME_FUNC_H
#define _ME_FUNC_H

#include "me_osal.h"
	
//Mobile engine control
ME_HANDLE ME_Init(void* ioproxy);
BOOLEAN ME_DeInit(ME_HANDLE handle);
BOOLEAN	ME_Cancel(ME_HANDLE handle);

//register URC calback function.
void ME_RegisterURC(ME_HANDLE handle, int nID/*URC_ID*/, ME_Callback reply, void *context);

//AT command control
//AT+CFUN=
NTSTATUS ME_Set_ModemFunctionality(ME_HANDLE handle, int fun, int rst, int common, ME_Callback reply, void* context);

//AT+ESLP
NTSTATUS ME_Set_SleepMode(ME_HANDLE handle, int mode, ME_Callback reply, void* context);

//AT+ESUO
NTSTATUS ME_Set_UartOwner(ME_HANDLE handle, int mode, ME_Callback reply, void* context);

//AT+CREG=
NTSTATUS ME_Set_NetwrokReg(ME_HANDLE handle, int n, ME_Callback reply, void* context);

//AT+ECPI=
NTSTATUS ME_Set_CallInfo(ME_HANDLE handle, char* value, ME_Callback reply, void* context);

//AT+EGTYPE=
NTSTATUS ME_Set_GPRSAttachMode(ME_HANDLE handle, int mode, int act, ME_Callback reply, void* context);

//ATE0Q0V1
NTSTATUS ME_Set_ModemFunc(ME_HANDLE handle, ME_Callback reply, void* context);

//AT
NTSTATUS ME_Query_ModemStatus(ME_HANDLE handle, ME_Callback reply, void* context);

//ATH
NTSTATUS ME_Terminate_Call(ME_HANDLE handle, ME_Callback reply, void* context);

//ATD
NTSTATUS ME_MakeMO_Call(ME_HANDLE handle, const char *number, ME_Callback reply, void* context);

//AT+ESIMS?
NTSTATUS ME_Query_SIMStatus(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+EGMR=0, 
NTSTATUS ME_Query_Revision(ME_HANDLE handle, int type, ME_Callback reply, void* context);

//AT+EGMR=1,
NTSTATUS ME_Set_Revision(ME_HANDLE handle, int type, char *str, ME_Callback reply, void* context);

//AT+CIMI
NTSTATUS ME_Query_IMSI(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+CGMR
NTSTATUS ME_Query_FWVersion(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+CPOF
NTSTATUS ME_TurnOff_Phone(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+EMDSTATUS
NTSTATUS ME_EMDSTATUS(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+ERAT=6
NTSTATUS ME_Change_Modem(ME_HANDLE handle, ME_Callback reply, void* context, int mode);

//AT+CPON
NTSTATUS ME_TurnOn_Phone(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+CSQ
NTSTATUS ME_DetectSignal(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+CHV
NTSTATUS ME_HangUpC2K(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+CDV=112
NTSTATUS ME_MakeC2KMO_Call(ME_HANDLE handle, const char *number, ME_Callback reply, void* context);

//AT^MEID
NTSTATUS ME_Query_MEID(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+ERFID
NTSTATUS ME_Query_RFChipID(ME_HANDLE handle, ME_Callback reply, void* context);

//ATZ
NTSTATUS ME_Reset_Config(ME_HANDLE handle, ME_Callback reply, void* context);

//AT+ES3G?
NTSTATUS ME_Query_3GStatus(ME_HANDLE handle, ME_Callback reply, void* context);
	
#endif