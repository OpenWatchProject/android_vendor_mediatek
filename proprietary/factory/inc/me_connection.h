#ifndef MTK_CONNECTION_H
#define MTK_CONNECTION_H

#include "me_func.h"
#include "me_comm.h"
#include "me_result.h"
#include "me_device.h"

class Connection
{

public:
	Connection();
	~Connection();
	
	//For connect device (ccci/usb/uart/etc.)
	int  Conn_Init(char *path, int mdIdx, ME_Callback urc_cb);   
	int  Conn_DeInit();           
	
	//For ATE mode
	int Open_ATEChannel(char *path, int mdIdx, ME_ATECallback ate_cb);
	int Close_ATEChannel();
	int Write_ATEChannel(char *buf, int len);
	int GetHandle();
	//For 2/3/4G modem AT Command
	int SetNormalRFMode(int mode);			//AT+CFUN
	int SetMTKRFMode(int mode);				//AT+EFUN
	int SetSleepMode(int mode);				//AT+ESLP
	int SetUARTOwner(int mode);				//AT+ESUO
	int SetNetworkRegInd(int n);			//AT+CREG
	int SetCallInfo(char* value);			//AT+ECPI
	int SetPSType(int mode);				//AT+EGTYPE	
	int SetModemFunc(int value);			//ATE0Q0V1
	int SetModemRevision(int type, char* revision);		//AT+EGMR=1,x
	int QueryModemStatus();					//AT
	int MakeMOCall(char *number); 			//ATD
	int TerminateCall();					//ATH

	int QuerySIMStatus(int& Isable);		//AT+ESIMS
	int QueryModemRevision(int type, char* revision);	//AT+EGMR=0,X
	int QueryIMSI(char *imsi);			//AT+CIMI
	int QueryFWVersion(char *ver);		//AT+CGMR
		
	//For C2K modem AT command
	int TurnOffPhone();                                     //AT+CPOF
	int TurnOnPhone();                                      //AT+CPON
	int EMDSTATUS();                                        //AT+EMDSTATUS
    int ChangeMode(int mode);                                       //AT+ERAT=6
	int DetectSignal();                                     //AT+CSQ
	int HangUpC2K();                                        //AT+CHV
	int C2KCall(char *number);                              //AT+CDV=112
	int QueryMEID(char *meid);                              //AT^MEID
	int QueryRFChipID(int &rfid);                          //AT+ERFID
	int ResetConfig();                                      //ATZ
	int Query3GStatus(int& capa);                         //AT+ES3G?
	
private:
	int Internal_Conn_Init(char *port, int mdIdx, ME_Callback urc_cb, ME_ATECallback ate_cb=NULL);

private:
	OSAL_HANDLE m_hDeInitMutex;
	ME_HANDLE m_medrv;	
	CDevice *m_dev;  //for device; ccci/usb/uart/etc... 
	int		m_ate_mode;
};

#endif