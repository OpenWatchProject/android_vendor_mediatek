#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/properties.h>
#include <dirent.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <unistd.h>
#include "PortHandle.h"
#include "PortInterface.h"
#include "LogDefine.h"
#include "SerPort.h"
#include "Context.h"

#if defined(MTK_TC1_FEATURE)
	#define DEV_USB_PATH	"/dev/ttyGS4"
#else
    #define DEV_USB_PATH    "/dev/ttyGS0"
#endif

#define COM_PORT_TYPE_FILE "/sys/bus/platform/drivers/meta_com_type_info/meta_com_type_info"
#define COM_PORT_TYPE_STR_LEN 1

#define UART_PORT_INFO_FILE "/sys/bus/platform/drivers/meta_uart_port_info/meta_uart_port_info"
#define UART_PORT_INFO_STR_LEN 1

#define MAX_DEVICES 32
#define MAX_LENGTH  1024


class PortHandle
{
private:
	PortHandle(void);
public:
	~PortHandle(void);

	static PortHandle *instance();
	SerPort * createPort();
	void destroyPort();
	SerPort * getPort() const;
	META_COM_TYPE getComType();
	void setComType(META_COM_TYPE comType);
	void querySerPortStatus();
	void FTMuxPrimitiveData(META_RX_DATA *pMuxBuf);
	int WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);
	int getMetaUartPort(void);
	void getMetaConnectType();
	void destroy();
	void usbMutexLock(bool bLock);

private:
	META_COM_TYPE		m_comType;
	SerPort *			m_serPort;
private:
	static PortHandle *	m_myInst;

	unsigned int ev_count;
	unsigned int ev_touch;
	struct pollfd ev_fds[MAX_DEVICES];
	pthread_mutex_t m_usbMutex;

};

PortHandle *PortHandle::m_myInst = NULL;

PortHandle::PortHandle(void)
	: m_comType(META_UNKNOWN_COM),
	  m_serPort(NULL)
{
    ev_count = 0;
	ev_touch = 0;
	memset(&ev_fds, 0, sizeof(pollfd)*MAX_DEVICES);
	m_usbMutex = PTHREAD_MUTEX_INITIALIZER;
}

PortHandle::~PortHandle(void)
{
	if (m_serPort != NULL)
	{
		delete m_serPort;
		m_serPort = NULL;
	}
}


PortHandle *PortHandle::instance()
{
	return (m_myInst==NULL) ? ((m_myInst=new PortHandle)) : m_myInst;
}

void PortHandle::destroy()
{
	delete m_myInst;
	m_myInst = NULL;
}

void PortHandle::destroyPort()
{
	if(m_serPort != NULL)
	{
		delete m_serPort;
		m_serPort = NULL;			
	}
}

SerPort * PortHandle::createPort()
{
    if (m_serPort != NULL)
    {
        assert(false); // repeated create
    }
    else
    {
        META_COM_TYPE eComType = getComType();
        if (eComType == META_USB_COM)
        {
            m_serPort = new UsbPort(DEV_USB_PATH);
        }
        else if (eComType == META_UART_COM)
        {
            char szDevUartPath[256] = {0};
            int ret;
            switch(getMetaUartPort())
            {
                case 1:      //UART1
                    ret = access(UART1_PATH_TTYMT, F_OK);
                    if(ret == 0)
                        strncpy(szDevUartPath, UART1_PATH_TTYMT, strlen(UART1_PATH_TTYMT));
                    else
                        strncpy(szDevUartPath, UART1_PATH_TTYS, strlen(UART1_PATH_TTYS));
                    break;
                case 2:      //UART2
                    ret = access(UART2_PATH_TTYMT, F_OK);
                    if(ret == 0)
                        strncpy(szDevUartPath, UART2_PATH_TTYMT, strlen(UART2_PATH_TTYMT));
                    else
                        strncpy(szDevUartPath, UART2_PATH_TTYS, strlen(UART2_PATH_TTYS));
                    break;
                case 3:      //UART3
                    ret = access(UART3_PATH_TTYMT, F_OK);
                    if(ret == 0)
                        strncpy(szDevUartPath, UART3_PATH_TTYMT, strlen(UART3_PATH_TTYMT));
                    else
                        strncpy(szDevUartPath, UART3_PATH_TTYS, strlen(UART3_PATH_TTYS));
                    break;
                case 4:      //UART4
                    ret = access(UART4_PATH_TTYMT, F_OK);
                    if(ret == 0)
                        strncpy(szDevUartPath, UART4_PATH_TTYMT, strlen(UART4_PATH_TTYMT));
                    else
                        strncpy(szDevUartPath, UART4_PATH_TTYS, strlen(UART4_PATH_TTYS));
                    break;
                default:     //default use UART1
                    ret = access(UART1_PATH_TTYMT, F_OK);
                    if(ret == 0)
                        strncpy(szDevUartPath, UART1_PATH_TTYMT, strlen(UART1_PATH_TTYMT));
                    else
                        strncpy(szDevUartPath, UART1_PATH_TTYS, strlen(UART1_PATH_TTYS));
                    break;
            }
            META_LOG("[Meta] uart port path: %s", szDevUartPath);
            m_serPort = new UartPort(szDevUartPath);
        }
        else if (eComType == META_SOCKET)
        {
            META_LOG("[Meta] eComType == META_SOCKET");
            char wcn_ready[128] = {0};
            //check WCN driver has ready
            int retry_prop = 20;
            while(retry_prop > 0)
            {
                property_get("vendor.connsys.formeta.ready", wcn_ready, "no");
                if(!strcmp(wcn_ready, "yes"))
                {
                    META_LOG("[Meta] createPort WCN driver ready");
                    break;;
                }
                else
                {
                    META_LOG("[Meta] createPort get service.wcn.driver.ready fail, retry_prop: %d", retry_prop);
                    usleep(100*1000);
                    retry_prop--;
                }
            }
            char tempPath[64] = {0}; //no use

            m_serPort = new MetaSocket(tempPath);

        }
    }

	assert(m_serPort != NULL);

	return m_serPort;
}

SerPort * PortHandle::getPort() const
{
	return m_serPort;
}

static int getBootMode_local()
{
    int bootMode;

    bootMode = readSys_int(BOOTMODE_PATH);

    if(NORMAL_BOOT== bootMode)
        META_LOG("[Meta] Normal mode boot!");
    else if(META_BOOT== bootMode)
        META_LOG("[Meta] Meta mode boot!");
    else {
		META_LOG("[Meta] Not Support boot mode! BootMode=%d",bootMode);
        bootMode = -1;       
    }     
    return bootMode;   
}

void PortHandle::setComType(META_COM_TYPE comType)
{
	META_LOG("[META] setComType %d",comType);	
	m_comType = comType;		
}

META_COM_TYPE PortHandle::getComType()
{
	if (m_comType == META_UNKNOWN_COM)
	{
		if(NORMAL_BOOT == getBootMode_local())
		{
			getMetaConnectType();	
		}
		else
		{
			char buf[COM_PORT_TYPE_STR_LEN + 1];
			int bytes_read = 0;
			int res = 0;
			int fd = open(COM_PORT_TYPE_FILE, O_RDONLY);
			if (fd != -1)
			{
				memset(buf, 0, COM_PORT_TYPE_STR_LEN + 1);
				while (bytes_read < COM_PORT_TYPE_STR_LEN)
				{
					res = read(fd, buf + bytes_read, COM_PORT_TYPE_STR_LEN);
					if (res > 0)
						bytes_read += res;
					else
						break;
				}
			    close(fd);
			    m_comType = (META_COM_TYPE)atoi(buf);
			    if (m_comType == META_UNKNOWN_COM)
			    {
                     getMetaConnectType();
			    }
		    }
		    else     //if (fd != -1)
		    {
			    META_LOG("[Meta] Failed to open com port type file %s", COM_PORT_TYPE_FILE);
		    }
		    META_LOG("[Meta] com port type: %d", m_comType);
	    }
    }
	return m_comType;
}

void PortHandle::getMetaConnectType()
{
	//FBE:full disk encryption -> [ro.crypto.type]: [file]
	//FDE:file based encryption-> [ro.crypto.type]: [block]   

	char tempstr[128]      = {0};
	char crypto_state[128] = {0};
	char crypto_type[128]  = {0};
	char vold_decrypt[128] = {0};
	int retry = 20;

	//Before get property value, we need wait for data decryption ready.
	property_get("ro.crypto.state", crypto_state, "");
	if (!strcmp(crypto_state, "unencrypted") || !strcmp(crypto_state, "unsupported"))
	{
	     META_LOG("[Meta] getMetaConnectType data ready, unencrypted");
         goto DATA_READY;
	}
	else if(!strcmp(crypto_state, "encrypted"))
	{
		property_get("ro.crypto.type", crypto_type, "");
		META_LOG("[Meta] getMetaConnectType: ro.crypto.type is %s", crypto_type);

		if(!strcmp(crypto_type, "file"))
		{
			META_LOG("[Meta] getMetaConnectType: FBE");
            goto DATA_READY;
		}

		//only FDE need to wait trigger_restart_framework
        while(retry > 0)
        {
            property_get("vold.decrypt", vold_decrypt, "");
		    if(!strcmp(vold_decrypt, "trigger_restart_framework"))
			{
                META_LOG("[Meta] getMetaConnectType data ready, retry: %d", retry);
                goto DATA_READY;
            }
			else
			{
			    usleep(1000 * 1000);
			    retry--;
			}
         }
		 META_LOG("[Meta] getMetaConnectType check data ready timeout, vold_decrypt: %s", vold_decrypt);
		 m_comType = META_UNKNOWN_COM;
		 return;
     }
	 else
	 {
	     META_LOG("[Meta] getMetaConnectType error crypto_state: %s", crypto_state);
		 m_comType = META_UNKNOWN_COM;
		 return;
	 }

DATA_READY:
    int ret = property_get("persist.vendor.meta.connecttype",tempstr,"unknown");
    META_LOG("[Meta] ret:%d, persist.vendor.meta.connecttype: %s", ret,tempstr);
	if (strcmp(tempstr,"uart") == 0)
	{
	    m_comType = META_UART_COM;  //UART
	}
	else if (strcmp(tempstr,"usb") == 0)
	{
	    m_comType = META_USB_COM;  //USB
	}
	else if (strcmp(tempstr,"wifi") == 0)
	{
	    m_comType = META_SOCKET;  //WiFi
	}

}

void PortHandle::querySerPortStatus()
{
	usbMutexLock(true);
	if (m_comType == META_USB_COM)
	{
		SerPort * pPort = getSerPort();
		if (pPort != NULL)
		{
			pPort->update();
		}
	}
	usbMutexLock(false);
}

int PortHandle::getMetaUartPort(void)
{
    int nPort = 1;
    if (m_comType == META_UART_COM)
    {
	    char buf[UART_PORT_INFO_STR_LEN + 1] = {0};
	    int fd = open(UART_PORT_INFO_FILE, O_RDONLY);
	    if (fd != -1)
	    {
			if (read(fd, buf, sizeof(char)*COM_PORT_TYPE_STR_LEN) <= 0)
			{
			    META_LOG("[Meta] ERROR can not read meta uart port ");
		    }
			else
			{
			    nPort = atoi(buf);
			}
		    close(fd);

	    }
	    else
	    {
		    META_LOG("[Meta] Failed to open meta uart port file %s", UART_PORT_INFO_FILE);
	    }
	    META_LOG("[Meta] uart com port: %d", nPort);
    }
	else
	{
	    META_LOG("[Meta] com port type is not uart");
	}
	return nPort;
}

void PortHandle::FTMuxPrimitiveData(META_RX_DATA *pMuxBuf)
{
    /* This primitive is logged by TST */
    unsigned char *pTempBuf = NULL;
    unsigned char *pTempDstBuf = NULL;
    unsigned char *pMamptrBase = NULL;
    unsigned char *pDestptrBase = NULL;
    int iCheckNum = 0;
    int dest_index=0;
    unsigned char cCheckSum = 0;
    int cbWriten = 0;
    int cbTxBuffer = 0;
    SerPort * pPort = getSerPort();

    if(pMuxBuf == NULL)
    {
        META_LOG("[Meta] (FTMuxPrimitiveData) Err: pMuxBuf is NULL");
        return;
    }

    cbTxBuffer = pMuxBuf->LocalLen + pMuxBuf->PeerLen + 9;
    if (cbTxBuffer>FRAME_MAX_LEN)
    {
        META_LOG("[Meta] (FTMuxPrimitiveData) error frame size is too big!! ");
        return;
    }
    else
        META_LOG("[Meta] (FTMuxPrimitiveData) Type = %d Local_len = %d, Peer_len = %d", pMuxBuf->eFrameType, pMuxBuf->LocalLen, pMuxBuf->PeerLen);

    //META_LOG("[Meta] (FTMuxPrimitiveData) total size = %d", cbTxBuffer);
    pMamptrBase = (unsigned char *)malloc(cbTxBuffer);

    if(pMamptrBase == NULL)
    {
        META_LOG("[Meta] (FTMuxPrimitiveData) Err: malloc pMamptrBase Fail");
        return;
    }
    pDestptrBase = (unsigned char *)malloc(FRAME_MAX_LEN);
    if(pDestptrBase == NULL)
    {
        META_LOG("[Meta] (FTMuxPrimitiveData) Err: malloc pDestptrBase Fail");
        free(pMamptrBase);
        return;
    }


    pTempDstBuf = pDestptrBase;
    pTempBuf = pMamptrBase;

    /* fill the frameheader */
    *pTempBuf++ = 0x55;
    *pTempBuf++=((pMuxBuf->LocalLen + pMuxBuf->PeerLen +5)&0xff00)>>8;
    *pTempBuf++= (pMuxBuf->LocalLen + pMuxBuf->PeerLen +5)&0xff;
    *pTempBuf++ = 0x60;

    /*fill the local and peer data u16Length and its data */
    *pTempBuf++ = ((pMuxBuf->LocalLen)&0xff); /// pMuxBuf->LocalLen ;
    *pTempBuf++ = ((pMuxBuf->LocalLen)&0xff00)>>8;
    *pTempBuf++ = (pMuxBuf->PeerLen )&0xff;   ///pMuxBuf->PeerLen ;
    *pTempBuf++ = ((pMuxBuf->PeerLen)&0xff00)>>8;

    memcpy((pTempBuf), pMuxBuf->pData, pMuxBuf->LocalLen + pMuxBuf->PeerLen);

    pTempBuf = pMamptrBase;

    /* 0x5a is start data, so we use 0x5a and 0x01 inidcate 0xa5, use 0x5a and 0x5a indicate 0x5a
    the escape is just for campatiable with feature phone */
    while (iCheckNum != (cbTxBuffer-1))
    {
        cCheckSum ^= *pTempBuf;
        *pTempDstBuf = *pTempBuf;
        iCheckNum++;

        if (*pTempBuf ==0xA5 )
        {
            *pTempDstBuf++ = 0x5A;
            *pTempDstBuf++ = 0x01;
            dest_index++;		//do the escape, dest_index should add for write to uart or usb
        }
        else if (*pTempBuf ==0x5A )
        {
            *pTempDstBuf++ = 0x5A;
            *pTempDstBuf++ = 0x5A;
            dest_index++;		//do the escape, dest_index should add for write to uart or usb
        }
        else
            pTempDstBuf++;

        dest_index++;
        pTempBuf++;
    }

    /* 0x5a is start data, so we use 0x5a and 0x01 inidcate 0xa5 for check sum, use 0x5a and 0x5a indicate 0x5a
    the escape is just for campatiable with feature phone */
    if ( cCheckSum ==0xA5 )
    {
        dest_index++;		//do the escape, dest_index should add for write to uart or usb
        //Wayne replace 2048 with MAX_TST_RECEIVE_BUFFER_LENGTH
        if ((dest_index) > FRAME_MAX_LEN)//2048)
        {
            META_LOG("[Meta] (FTMuxPrimitiveData) Data is too big: index = %d cbTxBuffer = %d ",dest_index, cbTxBuffer);
            goto TSTMuxError;
        }

        *pTempDstBuf++= 0x5A;
        *pTempDstBuf = 0x01;
    }
    else if ( cCheckSum ==0x5A )
    {
        dest_index++;		//do the escape, dest_index should add for write to uart or usb
        if ((dest_index) > FRAME_MAX_LEN)
        {
            META_LOG("[Meta] (FTMuxPrimitiveData) Data is too big: index = %d cbTxBuffer = %d ",dest_index, cbTxBuffer);
            goto TSTMuxError;
        }
        *pTempDstBuf++= 0x5A;
        *pTempDstBuf = 0x5A;
    }
    else
        *pTempDstBuf =(char )cCheckSum;

    dest_index++;

    //write to PC
    //cbWriten = write(getPort(), (void *)pDestptrBase, dest_index);

	pPort->write(pDestptrBase, dest_index);
    pTempDstBuf = pDestptrBase;

    META_LOG("[Meta] FTMuxPrimitiveData: %d  %d %d  cChecksum: %d ",cbWriten, cbTxBuffer, dest_index,cCheckSum);

    TSTMuxError:

    free(pMamptrBase);
    free(pDestptrBase);
}


int PortHandle::WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len)
{
	META_RX_DATA metaRxData;
	memset(&metaRxData,0, sizeof(META_RX_DATA));
	unsigned int dataLen = Local_len+Peer_len+8+1;
	unsigned char *metaRxbuf = (unsigned char *)malloc(dataLen);
        if(metaRxbuf==NULL)
        {
	    return 0;
        }
	memset(metaRxbuf,0, dataLen);
	unsigned char *cPeerbuf = &metaRxbuf[Local_len+8];

	metaRxData.eFrameType = AP_FRAME;
	metaRxData.pData = metaRxbuf;
	metaRxData.LocalLen = Local_len;
	metaRxData.PeerLen = Peer_len >0 ? Peer_len+8 : Peer_len;

    if (((Local_len + Peer_len)> FT_MAX_LEN)||(Peer_len >PEER_BUF_MAX_LEN))
    {
        META_LOG("[Meta] (WriteDataToPC) Err: Local_len = %hu, Peer_len = %hu", Local_len,Peer_len);
		free(metaRxbuf);
		metaRxbuf = NULL;
        return 0;
    }

    if ((Local_len == 0) && (Local_buf == NULL))
    {
        META_LOG("[Meta] (WriteDataToPC) Err: Local_len = %hu, Peer_len = %hu", Local_len,Peer_len);
		free(metaRxbuf);
		metaRxbuf = NULL;
        return 0;
    }

    // copy to the temp buffer, and send it to the tst task.
    memcpy(metaRxbuf, Local_buf, Local_len);
    if ((Peer_len >0)&&(Peer_buf !=NULL))
        memcpy(cPeerbuf, Peer_buf, Peer_len);

    FTMuxPrimitiveData(&metaRxData);
	free(metaRxbuf);
	metaRxbuf = NULL;
    return 1;
}

void PortHandle::usbMutexLock(bool bLock)
{
	if(bLock)
		pthread_mutex_lock(&m_usbMutex);
	else
		pthread_mutex_unlock(&m_usbMutex);
}
/////////////////////////////////////////////////////////////////////////////////

void destroyPortHandle()
{
	return PortHandle::instance()->destroy();
}

META_COM_TYPE getComType()
{
	return PortHandle::instance()->getComType();
}

SerPort * createSerPort()
{
	return PortHandle::instance()->createPort();
}

void destroySerPort()
{
	return PortHandle::instance()->destroyPort();
}

SerPort * getSerPort()
{
	return PortHandle::instance()->getPort();
}

void querySerPortStatus()
{
     return PortHandle::instance()->querySerPortStatus();
}

int WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len)
{
	return PortHandle::instance()->WriteDataToPC(Local_buf,Local_len,Peer_buf,Peer_len);
}

void setComType(META_COM_TYPE comType)
{
	return 	PortHandle::instance()->setComType(comType);
}

void usbMutexLock(bool bLock)
{
	return 	PortHandle::instance()->usbMutexLock(bLock);
}


