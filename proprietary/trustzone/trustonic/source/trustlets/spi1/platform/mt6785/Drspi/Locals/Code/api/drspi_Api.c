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

/**
 * @file   tlDriverApi.c
 * @brief  Implements driver APIs
 *
 * The APIs allow trustlets to make requests to the driver
 *
 */

#include "drSPIApi.h"
#include "drApiMarshal.h"
#include "drspi_Api.h"
#include "drSecMemApi.h"
#include "tlApispi.h"
#include "tlSecDriverApi.h"
#include "tlDapcIndex.h"

//#define DMA_TEST
//#define FIFO_TEST
//#define DRSPI_DEBUG_LOG

#ifdef  DRSPI_DEBUG_LOG
#define TLSPIMSG(fmt, args...) tlApiLogPrintf("[TL-SPI]"fmt, ##args)
#else
#define TLSPIMSG(fmt, args...)
#endif

#define SPI_DATA_LENGTH    16642
#ifdef DMA_TEST
static uint32_t spi_tx_buf[SPI_DATA_LENGTH];
static uint32_t spi_rx_buf[SPI_DATA_LENGTH];
static struct mt_chip_conf mt_chip_conf_dma_test = {
	.setuptime = 10,
	.holdtime = 10,
	.high_time = 20,
	.low_time = 20,
	.cs_idletime = 20,
	.ulthgh_thrsh = 0,
	.cpol = 0,
	.cpha = 1,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = DMA_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0
};

#endif

#ifdef FIFO_TEST
//static uint32_t spi_tx_buf[SPI_DATA_LENGTH];
//static uint32_t spi_rx_buf[SPI_DATA_LENGTH];
static struct mt_chip_conf mt_chip_conf_fifo_test = {
	.setuptime = 10,
	.holdtime = 10,
	.high_time = 20,
	.low_time = 20,
	.cs_idletime = 20,
	.ulthgh_thrsh = 0,
	.cpol = 0,
	.cpha = 1,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = FIFO_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0
};

#endif

_TLAPI_EXTERN_C int32_t tlset_master_transaction(unsigned int master_index ,unsigned int transaction_type);

/* Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param pMarParam MPointer to marshaling parameters to send to the driver.
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriver(
        uint32_t driver_ID,
        void* pMarParam);


/***********************************************
 * Open session to the driver with given data
 *
 * @return  session id
 ***********************************************/
_TLAPI_EXTERN_C uint32_t tlSpiOpenSession()
{
	uint32_t sid = DR_SID_INVALID;
	drMarshalingParam_t marParam = {
		.functionId = FID_DR_OPEN_SESSION,
	};
	TLSPIMSG("tlSpiOpenSession");
	if (TLAPI_OK == tlApi_callDriver(SPI_DRV_ID, &marParam)) {
	/* Retrieve session id provided by the driver */
	sid = marParam.sid;
	}
	return sid;
}

/**************************************************
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 *************************************************/
_TLAPI_EXTERN_C tlApiResult_t tlSpiCloseSession(
        uint32_t sid)
{
	drMarshalingParam_t marParam = {
	.functionId = FID_DR_CLOSE_SESSION,
	.sid = sid
	};
	TLSPIMSG("tlSpiCloseSession\n");
	tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
	return ret;
}

/****************************************************
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 ***************************************************/
_TLAPI_EXTERN_C tlApiResult_t tlSpiExecute(
        uint32_t sid,
        struct spi_transfer* spiData)
{
	drMarshalingParam_t marParam =
	{
	.functionId = FID_DR_EXECUTE,
	.sid        = sid,
	.payload    =
	{
		.spi_data = spiData
	}
	};

	TLSPIMSG("tlSpiExecute\n");
	tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
	// return ret;
	TLSPIMSG("marParam.payload.spi_data->exc_flag = %d \n",marParam.payload.spi_data->exc_flag);

	return marParam.payload.spi_data->exc_flag;
}

_TLAPI_EXTERN_C tlApiResult_t tlSpiGetOneImgExecute(
        uint32_t sid,
        struct spi_transfer* spiData)
{
    drMarshalingParam_t marParam =
    {
        .functionId = FID_DR_GETONEIMG_EXECUTE,
        .sid        = sid,
        .payload    =
        {
            .spi_data = spiData
        }
    };

	TLSPIMSG("tlSpiGetOneImgExecute\n");
    tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
   // return ret;
   TLSPIMSG("marParam.payload.spi_data->exc_flag = %d \n",marParam.payload.spi_data->exc_flag);
	return marParam.payload.spi_data->exc_flag;
}

_TLAPI_EXTERN_C tlApiResult_t tlSpiConfig(
        uint32_t sid,
        struct spi_transfer* spiData)
{
    drMarshalingParam_t marParam =
    {
        .functionId = FID_DR_SPI_HW_CONFIG,
        .sid        = sid,
        .payload    =
        {
            .spi_data = spiData
        }
    };

    TLSPIMSG("tlSpiConfig\n");
    tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
    return ret;
}

_TLAPI_EXTERN_C tlApiResult_t tlSpiDebug(
        uint32_t sid,
        struct spi_transfer* spiData)
{
    drMarshalingParam_t marParam =
    {
        .functionId = FID_DR_SPI_DEBUG,
        .sid        = sid,
        .payload    =
        {
            .spi_data = spiData
        }
    };

    TLSPIMSG("tlSpiDebug\n");
    tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
    return ret;
}

_TLAPI_EXTERN_C tlApiResult_t tlSpiTest(
        uint32_t sid,
        struct spi_transfer* spiData)
{
    drMarshalingParam_t marParam =
    {
        .functionId = FID_DR_SPI_TEST,
        .sid        = sid,
        .payload    =
        {
            .spi_data = spiData
        }
    };

    TLSPIMSG("tlSpiTest\n");
    tlApiResult_t ret = tlApi_callDriver(SPI_DRV_ID, &marParam);
    return ret;
}

static _TLAPI_EXTERN_C tlApiResult_t spi_test(struct spi_transfer* spiData)
{
    uint32_t  crSession;
    tlApiResult_t  tlRet;
    uint32_t handle, size, alignment;
    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);

    do {
        crSession = tlSpiOpenSession();
        tlRet = tlSpiTest(crSession, spiData); /*this will trigger SpiIpc thead*/
        tlSpiCloseSession(crSession);
        break;
    }while(false);
    return tlRet;
}

static _TLAPI_EXTERN_C tlApiResult_t spi_debug(struct spi_transfer* spiData)
{
    uint32_t  crSession;
    tlApiResult_t  tlRet;
    uint32_t handle, size, alignment;

    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);
    do {
        crSession = tlSpiOpenSession();
        tlRet = tlSpiDebug(crSession, spiData); /*this will trigger SpiIpc thead*/
        tlSpiCloseSession(crSession);
        break;
    }while(false);

    return tlRet;
}

static _TLAPI_EXTERN_C tlApiResult_t spi_hw_config(struct spi_transfer* spiData)
{
    uint32_t  crSession;
    tlApiResult_t  tlRet;
    uint32_t handle, size, alignment;

    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);
    do {
        crSession = tlSpiOpenSession();
        tlRet = tlSpiConfig(crSession, spiData); /*this will trigger SpiIpc thead*/
        tlSpiCloseSession(crSession);
        break;
    }while(false);

    return tlRet;
}

#if 0
static _TLAPI_EXTERN_C tlApiResult_t spi_handle(struct spi_transfer* spiData)
{
    uint32_t  crSession;
    tlApiResult_t  tlRet;
    uint32_t handle, size, alignment;

    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);
    do {
        crSession = tlSpiOpenSession();
        tlRet = tlSpiExecute(crSession, spiData); /*this will trigger SpiIpc thead*/
        tlSpiCloseSession(crSession);
        break;
    }while(false);

    return tlRet;
}
#endif

uint32_t  crSession;
static _TLAPI_EXTERN_C tlApiResult_t spi_handle(struct spi_transfer* spiData)
{
    tlApiResult_t  tlRet;
    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);
    do {
        tlRet = tlSpiExecute(crSession, spiData); /*this will trigger SpiIpc thead*/
        break;
    }while(false);
    return tlRet;
}

static _TLAPI_EXTERN_C tlApiResult_t spi_getOneImg_handle(struct spi_transfer* spiData)
{
    tlApiResult_t  tlRet;

    TLSPIMSG("Proceeding with spi operation type(%d) in SWd\n", spiData->commandId);

    do
    {
		tlRet = tlSpiGetOneImgExecute(crSession, spiData); /*this will trigger SpiIpc thead*/
		break;
    }while(false);

    return tlRet;
}

_TLAPI_EXTERN_C uint32_t spiOpenSession()
{
    TLSPIMSG("spiOpenSession...\n");
    do {
        crSession = tlSpiOpenSession();
        break;
    }while(false);

    return crSession;
}


_TLAPI_EXTERN_C tlApiResult_t spiCloseSession()
{
    tlApiResult_t  tlRet;
    TLSPIMSG("spiCloseSession...\n");
    do {
        tlRet = tlSpiCloseSession(crSession);
        break;
    }while(false);

    return tlRet;
}

//#define SPI_DATA_LENGTH 4

//uint64_t tx_dma_addr, rx_dma_addr;
uint32_t SharedMemAddr, SharedMemSize;
uint32_t UseSecureSpi = 0;
_TLAPI_EXTERN_C tlApiResult_t drSpiSend(const void *tx_buf, void *rx_buf, uint32_t len,
                                           struct mt_chip_conf *chip_conf, uint32_t flag)
{
	struct spi_transfer spiData;
	tlApiResult_t  ret;
	uint32_t tx_refcount,rx_refcount;
	uint32_t i;
	static uint32_t SharedMemStatus;
	int32_t tl_ret;
	uint32_t retryTimes = 10;

	/*tlset_module_apc(DAPC_INDEX_SPI1,0,1); // lock SPI, only secure world can access*/
	if(UseSecureSpi == 0) {
		i = retryTimes;
		do {
			ret = drSecMemQuerySharedMem(&SharedMemStatus);
			i--;
		} while ((ret!= TLAPI_OK || (!SharedMemStatus)) && i);

		if (ret != TLAPI_OK || (!SharedMemStatus)) {
			tlApiLogPrintf("[Trustlet TlSPi]ERROR!!spi drSecMemQuerySharedMem fail(retry:%d)! ret:%d\n",
						retryTimes, ret);
			return ret;
		} else {
			i = retryTimes;
			do {
				ret = drSecMemQueryPlReservedMem(SECMEM_PL_RESERVED_SPI,
                                                         &SharedMemAddr, &SharedMemSize);
				i--;
			} while(ret!=TLAPI_OK && i);
			if(ret != TLAPI_OK) {
				tlApiLogPrintf("[Trustlet TlSPi] spi drSecMemQueryPlReservedMem Failed(retry:%d)!Addr:0x%x,Size:0x%x,ret:%d\n",
							retryTimes, SharedMemAddr, SharedMemSize,ret);
				return ret;
			}
			TLSPIMSG("[Trustlet TlSPi] spi drSecMemQueryPlReservedMem Success! Addr:0x%x,Size:0x%x\n",
                                      SharedMemAddr, SharedMemSize);
			//spi measter use secure transaction
			//tl_ret = tlset_master_transaction(DAPC_MASTER_INDEX_SPI1,SECURE_TRANSACTION);
			tl_ret = tlset_devapc_master_trans(DEVAPC_MASTER_REQ_SPI, DEVAPC_PROTECT_ENABLE, SPI1);
			if (tl_ret == TLAPI_OK)
				UseSecureSpi = 1 ;
			else {
				tlApiLogPrintf("[Trustlet TlSPi] tlset_master_transaction fail.Exit spi transfer.tl_ret:%d\n", tl_ret);
				return tl_ret;
			}
		}
	}
	TLSPIMSG("[Trustlet TlSPi] spi measter use secure transaction , UseSecureSpi = %d  \n" ,UseSecureSpi);
	TLSPIMSG("[Trustlet TlSPi] this is not SPI_PIN_SWITCH_MODE API \n");

	spiData.commandId = CMD_SPI_DRV_SEND;
	spiData.tx_buf = tx_buf;
	spiData.rx_buf = rx_buf;
	spiData.chip_config = chip_conf;
	spiData.len = len;
	spiData.is_transfer_end = flag;
	spiData.is_dma_used = 0;

	if(spiData.chip_config != NULL) {
		if(DMA_TRANSFER == (spiData.chip_config->com_mod)) {
			spiData.is_dma_used = 1;
		}else {
			spiData.is_dma_used = 0;
		}
	}

#ifdef FIFO_TEST
	uint32_t send_data = 0x12341234;
	uint32_t recv_data = 0;
/*1 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0xf8;
		spiData.tx_buf = &send_data;
		spiData.rx_buf = &recv_data;
		spiData.len = 4;
		spiData.is_dma_used = 0;
		spiData.is_transfer_end = 1;
		spiData.chip_config = &mt_chip_conf_fifo_test;
		ret = spi_handle(&spiData);
		/*2 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0x1c;
		spiData.len = 4;
		ret = spi_handle(&spiData);

		/*1 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0xc0;
		spiData.len = 4;
		ret = spi_handle(&spiData);

		/*2 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0x1c;
		spiData.len = 4;
		ret = spi_handle(&spiData);

		/*1 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0xc0;
		spiData.len = 4;
		ret = spi_handle(&spiData);

		/*2 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0x1c;
		spiData.len = 4;
		ret = spi_handle(&spiData);

		/*2 byte*/
		//memset(send_data,0,10);
		//memset(recv_data,0,20);
		//send_data[0] = 0x1c;
		spiData.len = 4;
		ret = spi_handle(&spiData);
		//spiData.is_transfer_end = 1;
#endif

#ifdef DMA_TEST
	for(i = 0; i < SPI_DATA_LENGTH; i++) {
		spi_tx_buf[i]= 0x12341234;
	}

	spiData.tx_buf = spi_tx_buf;
	spiData.rx_buf = spi_rx_buf;
	spiData.len = SPI_DATA_LENGTH;
	spiData.is_dma_used = 1;
	spiData.is_transfer_end = 1;
	spiData.chip_config = &mt_chip_conf_dma_test;
#endif

	if (spiData.is_dma_used == 1) {
		if (SharedMemStatus) {
			spiData.tx_dma = SharedMemAddr;
			spiData.rx_dma = SharedMemAddr + SharedMemSize*1/4 ;
			TLSPIMSG("[Trustlet TlSPi] SECMEM_PL_RESERVED_SPI\n");
		} else {
			/*Shared mem not enabled*/
			//spiData.tx_dma = (tx_dma_addr);
			//spiData.rx_dma = (rx_dma_addr);
			TLSPIMSG("[Trustlet TlSPi] drSecMemAllocPA\n");
		}
	}

	TLSPIMSG("[Trustlet TlSPi] spiData.len=%d\n", spiData.len);
	TLSPIMSG("[Trustlet TlSPi] spi_handle+\n");
	/*defined in drspi_Api.c*/
	ret = spi_handle(&spiData);
	TLSPIMSG("[Trustlet TlSPi] spi_handle-\n");

	TLSPIMSG("SPI TL: tx_buf(0x%x)\n", spiData.tx_buf);
	TLSPIMSG("SPI TL: rx_buf(0x%x)\n", spiData.rx_buf);
	if (spiData.is_dma_used == 1) {
		/*drSecMemUnrefPA(tx_dma_addr,&tx_refcount);
		drSecMemUnrefPA(rx_dma_addr,&rx_refcount);*/
	}

	//tlset_master_transaction(9,0);
	//spi measter use non-secure transaction
	//tlset_module_apc(DAPC_INDEX_SPI1, 0, 0);
	// unlock SPI make normal world can access
	return ret;
}

_TLAPI_EXTERN_C tlApiResult_t drSpiHwConfig(struct mt_chip_conf *chip_conf)
{
	struct mt_chip_conf *chip_config;
	struct spi_transfer spiData;
	tlApiResult_t  ret;
	uint32_t tx_refcount,rx_refcount;
	spiData.chip_config = chip_conf;
	spiData.commandId = CMD_SPI_DRV_CONFIG;
	TLSPIMSG("[Trustlet TlSPi] spi_hw_config+\n");
	/*defined in drspi_Api.c*/
	ret = spi_hw_config(&spiData);
	TLSPIMSG("[Trustlet TlSPi] spi_hw_config-\n");
	return ret;
}

_TLAPI_EXTERN_C tlApiResult_t drSpiDebug(void)
{
	struct spi_transfer spiData;
	tlApiResult_t  ret;
	spiData.commandId = CMD_SPI_DRV_DEBUG;
	TLSPIMSG("[Trustlet TlSPi] spi_debug+\n");
	/*defined in drspi_Api.c*/
	ret = spi_debug(&spiData);
	TLSPIMSG("[Trustlet TlSPi] spi_debug-\n");
	return ret;
}

_TLAPI_EXTERN_C tlApiResult_t drSpiTest(void)
{
	struct spi_transfer spiData;
	tlApiResult_t  ret;
	spiData.commandId = CMD_SPI_DRV_TEST;
	TLSPIMSG("[Trustlet TlSPi] spi_test+\n");
	/*defined in drspi_Api.c*/
	ret = spi_test(&spiData);
	TLSPIMSG("[Trustlet TlSPi] spi_test-\n");
	return ret;
}


_TLAPI_EXTERN_C tlApiResult_t drGetOneImage(void *rx_buf, uint32_t len, uint32_t enable_custom, struct mt_chip_conf *chip_conf)
{
	struct spi_transfer spiData;
	tlApiResult_t  ret;
	uint32_t tx_refcount,rx_refcount;
	uint32_t i;
	static uint32_t SharedMemStatus;
	int32_t tl_ret;
	uint32_t retryTimes = 10;

	TLSPIMSG("rx_buf = 0x%x, len = %d, enable_custom = %d \n" , rx_buf, len, enable_custom);

	if(UseSecureSpi == 0)
	{
		i = retryTimes;
		do {
			ret = drSecMemQuerySharedMem(&SharedMemStatus);
			i--;
		} while ((ret!= TLAPI_OK || (!SharedMemStatus)) && i);

		if (ret != TLAPI_OK || (!SharedMemStatus)) {
			tlApiLogPrintf("[Trustlet TlSPi] ERROR!!spi drSecMemQuerySharedMem fail(retry:%d)! ret:%d\n",
						retryTimes, ret);
			return ret;
		} else {
			i = retryTimes;
			do {
				ret = drSecMemQueryPlReservedMem(SECMEM_PL_RESERVED_SPI, &SharedMemAddr, &SharedMemSize);
				i--;
			} while(ret!=TLAPI_OK && i);

			if(ret != TLAPI_OK) {
				tlApiLogPrintf("[Trustlet TlSPi] spi drSecMemQueryPlReservedMem Failed(retry:%d)! Addr:0x%x,Size:0x%x,ret:%d\n",
							retryTimes, SharedMemAddr, SharedMemSize,ret);
				return ret;
			}
			TLSPIMSG("[Trustlet TlSPi] spi drSecMemQueryPlReservedMem Success! Addr:0x%x,Size:0x%x\n",
							SharedMemAddr, SharedMemSize);

			//tl_ret = tlset_master_transaction(DAPC_MASTER_INDEX_SPI5,SECURE_TRANSACTION);	//spi measter use secure transaction
			tl_ret = tlset_devapc_master_trans(DEVAPC_MASTER_REQ_SPI, DEVAPC_PROTECT_ENABLE, SPI1);
			if (tl_ret == TLAPI_OK)
				UseSecureSpi = 1 ;
			else {
				tlApiLogPrintf("[Trustlet TlSPi] tlset_master_transaction fail.Exit spi transfer.tl_ret:%d\n", tl_ret);
				return tl_ret;
			}
		}
	}

	TLSPIMSG("[Trustlet TlSPi] spi measter use secure transaction , UseSecureSpi = %d  \n" ,UseSecureSpi);
	TLSPIMSG("[Trustlet TlSPi] this is not SPI_PIN_SWITCH_MODE API \n");

	spiData.commandId = CMD_SPI_DRV_SEND;
	spiData.rx_buf = rx_buf;
	spiData.chip_config = chip_conf;
	spiData.len = len;
	spiData.is_dma_used = 0;
	spiData.is_transfer_end = 1;
	spiData.enable_custom = enable_custom;

	spiData.tx_dma = SharedMemAddr + SharedMemSize*2/4;
	spiData.rx_dma = SharedMemAddr + SharedMemSize*3/4;
	TLSPIMSG("---------------------spiData.tx_dma = 0x%x, spiData.rx_dma = 0x%x \n" , spiData.tx_dma, spiData.rx_dma);

#if 0
	if(spiData.chip_config != NULL) {
       if(DMA_TRANSFER == (spiData.chip_config->com_mod)) {
	       spiData.is_dma_used = 1;
	   }else {
           spiData.is_dma_used = 0;
	   }
    }

	if (spiData.is_dma_used == 1) {
		if (SharedMemStatus) {
			spiData.tx_dma = SharedMemAddr + SharedMemSize*2/4;
			spiData.rx_dma = SharedMemAddr + SharedMemSize*3/4;
			TLSPIMSG("[Trustlet TlSPi] SECMEM_PL_RESERVED_SPI\n");
		} else {
			TLSPIMSG("[Trustlet TlSPi] drSecMemAllocPA\n");
		}
	}
#endif

	TLSPIMSG("[Trustlet TlSPi] spiData.len=%d\n", spiData.len);
	TLSPIMSG("[Trustlet TlSPi] spi_handle+\n");

	/*defined in drspi_Api.c*/
    ret = spi_getOneImg_handle(&spiData);
	TLSPIMSG("[Trustlet TlSPi] spi_handle-\n");

    TLSPIMSG("SPI TL: rx_buf(0x%x)\n", spiData.rx_buf);
	if (spiData.is_dma_used == 1) {

	}

	return ret;

}

