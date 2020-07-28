#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <cutils/sockets.h>

#include "Modem.h"
#include "SerPort.h"
#include "MSocket.h"
#include "UsbRxWatcher.h"
#include "Context.h"
#include "PortInterface.h"
#include "LogDefine.h"

int main(int argc, char** argv)
{
	META_LOG("[Meta] Enter meta_tst init flow! V1.01.00");

      	if(argv[0] != NULL)
        {
            META_LOG("argc = %d, argv= %s",argc,argv[0]);
        }

	umask(007);

	UsbRxWatcher hostRx;
        setVirtualRxWatcher(&hostRx);

	SerPort *pPort = NULL;

	if(NORMAL_BOOT == getBootMode())
	{
		META_LOG("[Meta] is normal mode");
		
		queryNormalModeTestFlag();
		if(1 == getNormalModeTestFlag())
		{
			
			sleep(1); //waiting for USBdeviceManager ready.

			META_LOG("[Meta] To set vendor.usb.config");
#ifndef MTK_ATM_METAWIFIONLY
			property_set("vendor.usb.config","atm_gs0gs3");
		
#else
			property_set("vendor.usb.config","atm_gs0");

#endif
			sleep(5);
			META_LOG("[Meta] To set persist.meta.connecttype");
			property_set("persist.vendor.meta.connecttype", "usb");
			META_LOG("[Meta] To set persist.vendor.atm.mdmode");
			property_set("persist.vendor.atm.mdmode", "normal");
			setMDMode(1);
    	    
			if(getModemHwVersion(0) >= MODEM_6293)  //only support one 93 and subsequent Modem
			{
				if(getLoadType()==2 || getLoadType()==3) //eng:1 user:2 user debug:3
				{
					META_LOG("[Meta] To set atci properties");
					property_set("persist.vendor.service.atci.atm_mode", "1");
				}
#ifdef ATM_PCSENDAT_SUPPORT
				META_LOG("[Meta] Define ATM_PCSENDAT_SUPPORT");

				MSocket *pSocket = getSocket(SOCKET_ATCI_CLIENT);
				if(pSocket == NULL)
				{
					pSocket = createSocket(SOCKET_ATCI_CLIENT);
					if(pSocket != NULL)
					{
						int bInit = pSocket->initServer("meta-atci", ANDROID_SOCKET_NAMESPACE_RESERVED);
						if(bInit == 0)
						{
							delSocket(SOCKET_ATCI_CLIENT);
						}
					}
				}
#else
                META_LOG("[Meta] Not Define ATM_PCSENDAT_SUPPORT");
                
                sleep(5);
	#ifndef MTK_ATM_METAWIFIONLY
                pPort = createSerPort();		
                if (pPort != NULL)
                {
	                pPort->pumpAsync(&hostRx);
				}
                else
                {
                    META_LOG("[Meta] Enter meta_tst init fail");
				}
	#endif
				//createAllModemThread();
#endif
				//Add for communication between APP and meta_tst
				MSocket *pSocket = getSocket(SOCKET_ATM_COMM);
				if(pSocket == NULL)
				{
					pSocket = createSocket(SOCKET_ATM_COMM);
					if(pSocket != NULL)
					{
						int bInit = pSocket->initClient("ATMWiFiHidlServer", ANDROID_SOCKET_NAMESPACE_ABSTRACT);
						if(bInit == 0)
						{
							delSocket(SOCKET_ATM_COMM);
						}
					}
				}
			}
			else // Modem is not 93
			{			
                sleep(5);
			
				pPort = createSerPort();
			
				if (pPort != NULL)
				{
					pPort->pumpAsync(&hostRx);
				}
				else
				{
					META_LOG("[Meta] Enter meta_tst normal mode init fail");
				}
			}
			
		}
		else
		{
			META_LOG("[Meta] Normal mode flag is not 1,exist!");
			return 0;
		}
	}
	else
	{

		META_LOG("[Meta] is meta mode");
		pPort = createSerPort();		
		if (pPort != NULL)
		{
			pPort->pumpAsync(&hostRx);
		}
		else
		{
			META_LOG("[Meta] Enter meta_tst init fail");
		}
		
		createAllModemThread();
	}

	while (1)
	{
		usleep(100*1000);
		querySerPortStatus(); //query port type change every 100ms
	}

	// infinite loop above; it'll never get here...



	destroyContext();

	return 0;
}
