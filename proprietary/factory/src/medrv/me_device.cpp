#include <stdlib.h>
#include <unistd.h>
#include <termios.h> 
#include <fcntl.h>
#include <errno.h>
#include "me_device.h"
#include "me_osal.h"

CDevice::CDevice(int mdIdx)
{
	m_handle    = NULL_FILE_DESCRIPTOR;
	m_nThreadId = 0;
	m_hThread   = NULL;
	m_hStopEvt  = NULL;
	m_datacb    = NULL;
	m_ate_cb    = NULL;
	m_pData     = NULL;
	m_ate_mode = 0;
	m_mdIdx     = mdIdx;
}

CDevice::~CDevice()
{
	
}

void CDevice::SetCallBack(DataNotify reply, void *pData)
{
	m_datacb = reply;
	m_pData = pData;
}
int CDevice::GetHandle()
{
    return m_handle;
}
void CDevice::SetATECallBack(ME_ATECallback ate_cb)
{
	m_ate_mode = 1;
	m_ate_cb     = ate_cb;
}

int CDevice::Init_Port(const char *bsdPath)
{
	int retry = 3;
	
	while(m_handle == NULL_FILE_DESCRIPTOR && retry != 0)
	{
		m_handle = open(bsdPath, O_RDWR|O_NOCTTY|O_NONBLOCK);
		ME_Print(ME_TAG "[Medrv] Open modem[%d] m_handle = %d", m_mdIdx, m_handle);
		if (m_handle != NULL_FILE_DESCRIPTOR)
		{
			ME_Print(ME_TAG "[Medrv] Open modem[%d]:(%s) success.", m_mdIdx, bsdPath);
			break;
		}
		else
		{
			ME_Print(ME_TAG "[Medrv] Open modem[%d]:(%s) fail.", m_mdIdx, bsdPath);
			usleep(100*1000);
			retry--;
		}
	}
	
	if(Initernal_Start() == OSAL_FALSE)
		goto error;
	
	// Success
	return m_handle;
	
	// Failure path
error:
	if (m_handle != -1)
	{
		close(m_handle);
		m_handle = -1;
	}
	
	return -1;
}

void CDevice::Deinit_Port()
{
	Initernal_Stop();
	ClosePort();
}

void CDevice::ClosePort()
{
	close(m_handle);
	m_handle = -1;
}

unsigned int CDevice::WriteData(const char *buf, const int &size)
{
	int nRetryCount = 0;
	size_t nSentBytes = 0;
	size_t wbytes = 0;
	size_t nPerSend = 0;

	while((int)nSentBytes < size)
	{
		wbytes = 0;
		nPerSend = size - nSentBytes;
		wbytes = write(m_handle, buf+nSentBytes, nPerSend);

		if((int)wbytes == -1)
		{
			ME_Print(ME_TAG "[Medrv] modem[%d] Error writting to device", m_mdIdx);
			return 0;
		}
		else if(wbytes < nPerSend)
		{
			nSentBytes += wbytes;
		}
		else
		{
			nSentBytes += wbytes;
			nRetryCount =0;
			continue;
		}
	}

	ME_Print(ME_TAG "[Medrv] modem[%d][txd] %s", m_mdIdx, buf);

	return 1;
}

int CDevice::Initernal_Start()
{
	m_hStopEvt = CreateEvent(NULL, OSAL_TRUE, OSAL_FALSE, NULL);

	if(!StartThread())
	{
		CloseHandle(m_hStopEvt);
		m_hStopEvt = NULL;
		return 0;
	}
	return 1;
}

void CDevice::Initernal_Stop()
{
	SetEvent(m_hStopEvt);
	if(m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	CloseHandle(m_hStopEvt);
	m_hStopEvt = NULL;
}

int CDevice::StartThread()
{
	m_hThread = _beginthreadex(NULL, 0, WatchProc, this, 0, &m_nThreadId);

	if(m_hThread != NULL)
		return 1;

	return 0;
}

DWORD CDevice::WatchProc(LPVOID param)
{
	CDevice *pto = (CDevice*)param;
	if(pto != NULL)
		return pto->ThreadFunc();

	return 0;
}

DWORD CDevice::ThreadFunc()
{
	int numBytes = 0;
	char buf[1024] = {0};

	while(1)
	{
		if(WaitForSingleObject(m_hStopEvt, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		numBytes = read(m_handle, buf, sizeof(buf));
		//ME_Print(ME_TAG "[Medrv] [modem%d][rxd] len = %d", m_mdIdx, numBytes);
		if (numBytes <= 0)
		{
			if(errno == EAGAIN)
			{
				usleep(100000);
				continue;
			}
			return 0;
		}
		else if (numBytes > 0)
		{
			if (numBytes == 1024)
			{// The data from md is too long
				buf[numBytes-1] = 0;
			}
			else
			    buf[numBytes] = 0;
			dumpDataInHexString(buf, numBytes, 16);

			if(m_datacb || m_ate_cb)
			{
				ME_Print(ME_TAG "[Medrv] [rxd]:%s", buf);
				if(!m_ate_mode)
					m_datacb(true, buf, numBytes, m_pData);
				else
				{
						m_ate_cb(buf, numBytes);
				}
			}
		}
	}

	return 1;
}

unsigned int CDevice::dumpDataInHexString(const char* con, int length, unsigned int bytesPerRow)
{

	int i = 0;
	unsigned int j = 0;
	unsigned int rowLength = 3 * bytesPerRow + 1;
	unsigned char hex[rowLength];
	unsigned char high;
	unsigned char low;

	ME_Print(ME_TAG "[Medrv] Dump data begin!");
	for(i = 0; i < length; i++)
	{
		high = (con[i] >> 4);
		low = (con[i] & 0x0f);

		if(high < 0x0a)
			high += 0x30;
		else
			high += 0x37;

		if(low < 0x0a)
			low += 0x30;
		else
			low += 0x37;

		hex[j++] = high;
		hex[j++] = low;
		hex[j++] = ' ';

		if (j == rowLength - 1 || i == length - 1)
		{
			hex[j] = '\0';
			j = 0;
			ME_Print("%s", hex);
		}
	}

	ME_Print(ME_TAG "[Medrv] Dump data finished!");

	return 0;
}
