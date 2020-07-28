#include "me_connection.h"
#include "me_result.h"
#include "me_osal.h"

Connection::Connection()
{
	m_dev    = NULL;
	m_medrv  = NULL;
	m_ate_mode = 0;
	m_hDeInitMutex = CreateMutex(NULL, OSAL_FALSE, NULL);
}

Connection::~Connection()
{
	CloseHandle(m_hDeInitMutex);
}

int Connection::Internal_Conn_Init(char *port, int mdIdx, ME_Callback urc_cb, ME_ATECallback ate_cb)
{
	if(m_dev != NULL)
		Conn_DeInit();

	m_dev = new CDevice(mdIdx);

	if(m_dev == NULL)
		return 0;

	if(NULL != ate_cb)
	{
		m_dev->SetATECallBack(ate_cb);
	}
	else
	{
		m_medrv = ME_Init((void*)m_dev);
		if(m_medrv == NULL)
		{
			Conn_DeInit();
			return 0;
		}

		m_dev->SetCallBack(DataCallBack, m_medrv);
		ME_RegisterURC(m_medrv, 0, urc_cb, NULL);
	}

	if(INVALID_HANDLE_VALUE == m_dev->Init_Port(port))
	{
		Conn_DeInit();
		return 0;
	}

	return 1;
}

int Connection::Conn_Init(char *path, int mdIdx, ME_Callback urc_cb) 
{
	WaitForSingleObject(m_hDeInitMutex, INFINITE);
	Internal_Conn_Init(path, mdIdx, urc_cb);
	ReleaseMutex(m_hDeInitMutex);
	return 1;
}

int Connection::Conn_DeInit()
{
	WaitForSingleObject(m_hDeInitMutex, INFINITE);
	if(m_dev != NULL)
	{
		m_dev->Deinit_Port();
		delete m_dev;
		m_dev = NULL;
	}

	if(m_medrv != NULL)
	{
		ME_DeInit(m_medrv);
		m_medrv = NULL;
	}
	ReleaseMutex(m_hDeInitMutex);
	return 1;
}

//For ATE mode
int Connection::Open_ATEChannel(char *path, int mdIdx, ME_ATECallback ate_cb)
{
	return Internal_Conn_Init(path, mdIdx, NULL, ate_cb);
}

int Connection::Close_ATEChannel()
{
	return Conn_DeInit();
}

int Connection::Write_ATEChannel(char *buf, int len)
{
	if(m_dev != NULL)
	{
		m_dev->WriteData((const char*)buf,  len);
    		return 1;
	}

	return 0;
}
int Connection::GetHandle()
{
	return m_dev->GetHandle();
}

//For Sending AT Command
int Connection::SetNormalRFMode(int mode)			//AT+CFUN
{
	ATResult atret;
	return ME_Set_ModemFunctionality(m_medrv, mode, -1, 0, NULL, &atret);
}

int Connection::SetMTKRFMode(int mode)				//AT+EFUN
{
	ATResult atret;
	return ME_Set_ModemFunctionality(m_medrv, mode, -1, 1, NULL, &atret);
}

int Connection::SetSleepMode(int mode)				//AT+ESLP
{
	ATResult atret;
	return ME_Set_SleepMode(m_medrv, mode, NULL, &atret);
}

int Connection::SetUARTOwner(int mode)				//AT+ESUO
{
	ATResult atret;
	return ME_Set_UartOwner(m_medrv, mode, NULL, &atret);
}

int Connection::SetNetworkRegInd(int n)				//AT+CREG
{
	ATResult atret;
	return ME_Set_NetwrokReg(m_medrv, n, NULL, &atret);
}

int Connection::SetCallInfo(char* value)				//AT+ECPI
{
	ATResult atret;
	return ME_Set_CallInfo(m_medrv, value, NULL, &atret);
}

int Connection::SetPSType(int mode)					//AT+EGTYPE
{
	ATResult atret;
	return ME_Set_GPRSAttachMode(m_medrv, mode, -1, NULL, &atret);
}

int Connection::SetModemFunc(int value)				//ATE
{
	ATResult atret;
	if (value == 0) //reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] Warning: value = 0\n");
	}
	return ME_Set_ModemFunc(m_medrv, NULL, &atret);
}

int Connection::QuerySIMStatus(int& Isable) 			//AT+ESIMS
{
	ATResult atret;

	int ret = ME_Query_SIMStatus(m_medrv, NULL, &atret);
	if(ER_OK == ret)
	{
		if(atret.check_key("+ESIMS"))
			Isable = atret.get_integer(0, 1);
		else
			ret = ER_UNKNOWN;
	}

	return ret;
}

int Connection::QueryModemStatus()					//AT
{
	ATResult atret;
	return ME_Query_ModemStatus(m_medrv, NULL, &atret);
}

int Connection::MakeMOCall(char *number)			//ATD
{
	ATResult atret;
	return ME_MakeMO_Call(m_medrv, number, NULL, &atret);
}

int Connection::TerminateCall()						//ATH
{
	ATResult atret;
	return ME_Terminate_Call(m_medrv, NULL, &atret);
}

int Connection::QueryModemRevision(int type, char* revision)	//AT+EGMR
{
	ATResult atret;

	int ret = ME_Query_Revision(m_medrv, type, NULL, &atret);
	if(ER_OK != ret)
		return ret;

	if(!atret.check_key("+EGMR"))
		return ER_UNKNOWN;
	if (!atret.get_string(revision, 0, 1))
		return ER_UNKNOWN;

	return ER_OK;
}

int Connection::SetModemRevision(int type, char* revision)		//AT+EGMR
{
	ATResult atret;
	return ME_Set_Revision(m_medrv, type, revision, NULL, &atret);
}

int Connection::QueryIMSI(char *imsi)		//AT+CIMI
{
	ATResult atret;
	int ret = ME_Query_IMSI(m_medrv, NULL, &atret);

	if (ER_OK != ret)
	{
		return ret;
	}

	if(atret.resultLst.size() >= 1)
	{
		atret.get_string(imsi, 0, 0);
	}

	return ER_OK;
}

int Connection::QueryFWVersion(char *ver)	//AT+CGMR
{
	ATResult atret;

	int ret = ME_Query_FWVersion(m_medrv, NULL, &atret);
	if(ER_OK != ret)
		return ret;

	if (!atret.get_string(ver, 0, 1))
		return ER_UNKNOWN;

	return ER_OK;
}

int Connection::TurnOffPhone()
{
    ATResult atret;
	return ME_TurnOff_Phone(m_medrv, NULL, &atret);
}

int Connection::TurnOnPhone()
{
    ATResult atret;
	return ME_TurnOn_Phone(m_medrv, NULL, &atret);
}

int Connection::EMDSTATUS()
{
    ATResult atret;
	return ME_EMDSTATUS(m_medrv, NULL, &atret);
}

int Connection::ChangeMode(int mode)
{
    ATResult atret;
	return ME_Change_Modem(m_medrv, NULL, &atret, mode);
}

int Connection::DetectSignal()
{
    ATResult atret;
	return ME_DetectSignal(m_medrv, NULL, &atret);
}

int Connection::HangUpC2K()
{
    ATResult atret;
	return ME_HangUpC2K(m_medrv, NULL, &atret);
}


int Connection::C2KCall(char *number)
{
    ATResult atret;
	return ME_MakeC2KMO_Call(m_medrv, number, NULL, &atret);
}

int Connection::ResetConfig()
{
    ATResult atret;
	return ME_Reset_Config(m_medrv, NULL, &atret);
}

int Connection::QueryMEID(char *meid)
{
	ATResult atret;

	int ret = ME_Query_MEID(m_medrv, NULL, &atret);
	if(ER_OK != ret)
		return ret;

	if (!atret.get_string(meid, 0, 1))
		return ER_UNKNOWN;

    return ER_OK;
}


int Connection::Query3GStatus(int& capa) { //AT+ES3G?
    ATResult atret;

	int ret = ME_Query_3GStatus(m_medrv, NULL, &atret);
	
	if(ER_OK == ret)
	{
		if(atret.check_key("+ES3G"))
			capa = atret.get_integer(0, 1);
		else
			ret = ER_UNKNOWN;
	}
	
	return ret;
}
