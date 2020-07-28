#include "Win2L.h"

#define NULL_FILE_DESCRIPTOR (-1)

typedef int (*DataNotify) (int status, const char* pTxt, unsigned int nLen, void *pData);
typedef void (*ATECallback) (const char *pData, int len);

class CDevice 
{
private:
	int m_handle;
	OSAL_HANDLE m_hThread;
	unsigned int m_nThreadId;
	OSAL_HANDLE m_hStopEvt;
	void *m_pData;
	DataNotify m_datacb;
	ATECallback m_ate_cb;
	int m_mdIdx;
	int m_ate_mode;

	unsigned int dumpDataInHexString(const char* con, int length, unsigned int bytesPerRow);
	int Initernal_Start();
	void Initernal_Stop();
	void ClosePort();
	int StartThread();
	static DWORD __stdcall WatchProc(LPVOID param);
	DWORD ThreadFunc();

public:
	CDevice(int mdIdx);
	~CDevice();
	
	int Init_Port(const char *bsdPath);
	void Deinit_Port();
	int GetHandle();
	unsigned int WriteData(const char *buf, const int &size);
	void SetCallBack(DataNotify reply, void *pData);
	void SetATECallBack(ATECallback ate_cb);
	
};