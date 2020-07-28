/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/*
 * @file   tlfoo.c
 * @brief  Trustlet implementation
 *
 * TCI data is processed and handled according to the command id and data
 * given by the associated TLC. Normal world is notified accordingly with
 * relevant return code after processing TCI command data
 *
 */

#include "tlStd.h"
#include "TlApi/TlApi.h"
//#include "drtlspi_api.h"

#include "tlspi_Api.h"
#include "drtlspi_api.h"

// Reserve 2048 byte for stack.
DECLARE_TRUSTLET_MAIN_STACK(8192);

/*
 * Trustlet entry.
 */
_TLAPI_ENTRY void tlMain( const addr_t tciBuffer, const uint32_t tciBufferLen )
{
    tciReturnCode_t ret;
    tciCommandId_t commandId;

    tlApiLogPrintf("[<t Trustlet TlSecSPI], Build " __DATE__ ", " __TIME__ EOL);

    dbgSN("SPI trustlet is starting");
	
    {
        uint32_t tlApiVersion;
        mcVersionInfo_t versionInfo;

        ret = tlApiGetVersion(&tlApiVersion);
        if (TLAPI_OK != ret) 
        {
            tlDbgPrintLnf("tlApiGetVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("tlApi version 0x%08X, exit", tlApiVersion);

        ret = tlApiGetMobicoreVersion(&versionInfo);
        if (TLAPI_OK != ret) 
        {
            tlDbgPrintLnf("tlApiGetMobicoreVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("productId        = %s",     versionInfo.productId);
        tlDbgPrintLnf("versionMci       = 0x%08X", versionInfo.versionMci);
        tlDbgPrintLnf("versionSo        = 0x%08X", versionInfo.versionSo);
        tlDbgPrintLnf("versionMclf      = 0x%08X", versionInfo.versionMclf);
        tlDbgPrintLnf("versionContainer = 0x%08X", versionInfo.versionContainer);
        tlDbgPrintLnf("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
        tlDbgPrintLnf("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
        tlDbgPrintLnf("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
        tlDbgPrintLnf("versionCmp       = 0x%08X", versionInfo.versionCmp);
    }

    /* Check if the size of the given TCI is sufficient */
    if ((NULL == tciBuffer) || (sizeof(tciSpiMessage_t) > tciBufferLen))
    {
        dbgSN("TCI error");
        dbgSPN("TCI buffer: ", tciBuffer);
        dbgSPN("TCI buffer length: ", tciBufferLen);
        dbgSDN("sizeof(tciSpiMessage_t): ", sizeof(tciSpiMessage_t));
        tlApiExit(EXIT_ERROR);
    }

    tlDbgPrintLnf("SEC SPI trustlet is processing the command");
    tciSpiMessage_t* message = (tciSpiMessage_t*) tciBuffer;

    //drSecMemQueryPool(&pool_addr, &pool_size);
    //tlDbgPrintLnf("SEC MEM [POOL] addr (0x%x) size(0x%x)", pool_addr, pool_size);


    for (;;)
    {
        tlDbgPrintLnf("SEC SPI trustlet is waiting for a notification to arrive");

        /* Wait for a notification to arrive */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /* Dereference commandId once for further usage */
        commandId = message->cmd_spi.header.commandId;

        tlDbgPrintLnf("[Trustlet TlSPi...]Got a message, commandId=0x%08X", commandId);

        /* Check if the message received is (still) a response */
        if (!IS_CMD(commandId)) 
        {
            tlApiNotify();
            continue;
        }

        /* Process command message */
        switch (commandId) 
        {
            /*SPI Read/Write APIs*/
			case CMD_SPI_SEND:
				tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_SEND+");
				/*defined in drspi_Api.c*/
            	ret = drSpiSend1(message->tx_buf,message->rx_buf,message->len,message->tl_chip_config,message->is_transfer_end);
				tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_SEND-");
              break;
            case CMD_SPI_CONFIG:
			  	tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_CONFIG+");
				/*defined in drspi_Api.c*/
            	ret = drSpiHwConfig(message->tl_chip_config);
				tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_CONFIG-");
              break;
			case CMD_SPI_DEBUG:
                tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_DEBUG+");
				/*defined in drspi_Api.c*/
            	ret = drSpiDebug();
				tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_DEBUG-");
              break;
			case CMD_SPI_TEST:
                tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_TEST+");
				/*defined in drspi_Api.c*/
            	ret = drSpiTest();
				tlDbgPrintLnf("[Trustlet TlSPi] CMD_SPI_TEST-");
              break;
            default:
              /* Unknown command ID */
              ret = RET_ERR_UNKNOWN_CMD;
              break;
	    }
	
		/* Set up response header */
		message->rsp_spi.header.responseId = RSP_ID(commandId);
		message->rsp_spi.header.returnCode = ret;
		
		tlDbgPrintLnf("SEC SPI trustlet is exiting with status %d", ret);
		
		/* Notify back the TLC */
		tlApiNotify();
	}
}
