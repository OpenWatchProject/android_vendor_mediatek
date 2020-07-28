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
 * @file   drfooIpch.c
 * @brief  Implements IPC handler of the driver.
 *
 */

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "DrError.h"
#include "drCommon.h"
#include "drspi_Api.h"
#include "tlApispi.h"
#include "drApiMarshal.h"
#include "drSmgmt.h"
#include "spi.h"
#include "spi_debug.h"

#define NO_DMA_LENGTH_LIMIT

extern int spi_tz_irq_handler(void);

DECLARE_STACK(drIpchStack, 50*1024);

/* Static variables */
static dciMessage_t *message;
int timeout_flag = 0;
uint8_t is_last_xfer = 1;

uint64_t excute_beg;
uint64_t map_spiData_end;
uint64_t map_tx_buf_end;
uint64_t map_rx_buf_end;
uint64_t map_chip_config_end;
uint64_t setup_reg_end;
uint64_t sngl_trsf_sw;
uint64_t sngl_trsf_hw;
uint64_t fst_trsf_sw;
uint64_t fst_trsf_hw;
uint64_t scnd_trsf_sw;
uint64_t scnd_trsf_hw;

uint64_t scnd_poll_beg;
uint64_t scnd_poll_end;

extern int32_t SpiInit(void);
extern void SpiIrqAttach(void);
extern void SpiIrqDetach(void);


void Set_timeout_flag(void)
{
	timeout_flag = -1 ;

}
int Get_timeout_flag(void)
{
	return timeout_flag;
}


/*
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void )
{
	/* Set IPC parameters for initial MSG_RD to IPCH */
	threadid_t              ipcClient = NILTHREAD;
	message_t               ipcMsg    = MSG_RD;
	uint32_t                ipcMsgId  = MSG_RD;
	uint32_t                ipcData   = 0;
	tlApiResult_t           tlRet     = TLAPI_OK;
	drApiResult_t           drRet     = DRAPI_OK;
	drMarshalingParam_ptr   pMarshal;
	taskid_t                taskid = drApiGetTaskid();
	uint8_t                 *text = NULL;
	uint32_t                x, y, result;
	struct spi_transfer    *spiData=NULL;
	struct spi_transfer     xfer;
	struct mt_chip_conf     spi_ipc_chip_config;
	struct spi_transfer    *xfer_p = &xfer;
	uint32_t		total_len;
	for(;;) {
/*************************************************************
* When called first time sends ready message to IPC server and
* then waits for IPC requests
*************************************************************/
		if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData)) {
			SPI_ERR("drIpchLoop(): drApiIpcCallToIPCH failed\n");
			continue;
		}

		SPI_MSG("Received command from trustlet (MSG_RQ_EX, API LEVEL:%d)\n", TBASE_API_LEVEL);
		ipcMsgId = drApiExtractMsgCmd(ipcMsg);
/* Dispatch request */

		switch(ipcMsgId) {
		case MSG_CLOSE_TRUSTLET:
/*************************
* Trustlet close message
*************************/
			SPI_MSG("drIpchLoop(): Acknowledging trustlet close\n");
/****************************************************************
*Close active sessions owned by trustlet, which is being shutdown
*For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
****************************************************************/
			drSmgmtCloseSessionForThread(ipcData);
			ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
			ipcData = TLAPI_OK;
		break;
		case MSG_CLOSE_DRIVER:
/*************************
* Trustlet close message
*************************/
			ipcMsg = MSG_CLOSE_DRIVER_ACK;
			ipcData = TLAPI_OK;
		break;
		case MSG_RQ:
		case MSG_RQ_EX:
			drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
						(addr_t)ipcData,
						sizeof(drMarshalingParam_t),
						MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
						(void**)&pMarshal);
			if(drRet != DRAPI_OK) {
				tlRet = E_DRAPI_CANNOT_MAP;
				ipcMsg  = MSG_RS;
				SPI_ERR("drIpchLoop(): map task buffer failed");
				break;
			}
			if(pMarshal) {
				/* Process the request */
				switch (pMarshal->functionId) {
					case FID_DR_OPEN_SESSION:
						SPI_MSG("FID_DR_OPEN_SESSION & Attach FIQ\n");
#ifndef SPI_TRANSFER_POLLING
						SpiIrqAttach();
#endif
						pMarshal->sid = drSmgmtOpenSession(ipcClient);
/**************************************************************************
* Update IPC status as success. If there is an error with opening session
* invalid sid will indicatethat opening session failed
*************************************************************************/
						tlRet = TLAPI_OK;
					break;
					case FID_DR_CLOSE_SESSION:
/**********************************************************************
* Handle close session request based on the sid provided by the client
*********************************************************************/
						SPI_MSG("FID_DR_CLOSE_SESSION\n");
#ifndef SPI_TRANSFER_POLLING
						SpiIrqDetach();
#endif
						drSmgmtCloseSession(pMarshal->sid);
						tlRet = TLAPI_OK;
					break;
					case FID_DR_INIT_DATA:
/*************************
* Initialize sesion data
*************************/
						SPI_MSG("FID_DR_INIT_DATA\n");
						if (E_DR_SMGMT_OK == drSmgmtSetSessionData(
								pMarshal->sid,
								ipcClient,
								(pMarshal->payload.spi_data))) {
							tlRet = TLAPI_OK;
						}
					break;
					case FID_DR_SPI_EXECUTE:
						SPI_MSG("FID_DR_EXECUTE\n");
						/* map the transfer struct */
#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&excute_beg) != DRAPI_OK)
							SPI_PERFORMANCE("get excute_beg time failed\n");
#endif
						SetIrqFlag(IRQ_IDLE);
						timeout_flag = 0 ;
						drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
							(addr_t)pMarshal->payload.spi_data,
							sizeof(xfer),
							MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
							(void**)&spiData);
						if(drRet != DRAPI_OK) {
							SPI_ERR("[Driver Drspi] FID_DR_EXECUTE: map spiData task buffer failed");
							tlRet = E_DRAPI_CANNOT_MAP;
							break;
						}
#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&map_spiData_end)!= DRAPI_OK)
							SPI_PERFORMANCE("get map_spiData_end time failed\n");
#endif
						if(CMD_SPI_DRV_SEND == spiData->commandId) {
							uint32_t  packet_loop, rest_size;
							uint32_t i;
							/* ----map the tx_buf from user -----------*/
							drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
								(addr_t)spiData->tx_buf,
								spiData->len,
								MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
								(void**)&(xfer_p->tx_buf));
							if(drRet != DRAPI_OK) {
								SPI_ERR("CMD_SPI_DRV_SEND: map tx_buf failed,drRet=%d\n", drRet);
								tlRet = E_DRAPI_CANNOT_MAP;
								break;
						}
#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&map_tx_buf_end) != DRAPI_OK)
							SPI_PERFORMANCE("get map_tx_buf_end time failed\n");
#endif
						/* ----map the rx_buf from user ------------*/
						drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
								(addr_t)spiData->rx_buf,
								spiData->len,
								MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
								(void**)&(xfer_p->rx_buf));
						if(drRet != DRAPI_OK) {
							SPI_ERR("CMD_SPI_DRV_SEND: map rx_buf failed, drRet=%d\n", drRet);
							tlRet = E_DRAPI_CANNOT_MAP;
							break;
						}
#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&map_rx_buf_end) != DRAPI_OK)
							SPI_PERFORMANCE("get map_rx_buf_end time failed\n");
#endif
						xfer.tx_dma = spiData->tx_dma;
						xfer.rx_dma = spiData->rx_dma;
						xfer.len = spiData->len;
						xfer.is_dma_used = spiData->is_dma_used;
						xfer.is_transfer_end = spiData->is_transfer_end;
						total_len = xfer.len;
						SPI_MSG("tx_dma=0x%x,rx_dma=0x%x,xfer.len=%d\n",
										xfer.tx_dma,
										xfer.rx_dma,
										xfer.len);

						while(IRQ_BUSY == GetIrqFlag()) {
						/*need a pause instruction to avoid unknow exception*/
							SPI_MSG("IPC wait IRQ handle finish\n");
						}
			            /* ------map the chip config struct from user -----------*/
						drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
								(addr_t)spiData->chip_config,
								sizeof(spi_ipc_chip_config),
								MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
								(void**)&(xfer_p->chip_config));
						if(drRet != DRAPI_OK) {
							SPI_ERR("CMD_SPI_DRV_SEND: map chip_config task buffer failed, drRet=%d\n", drRet);
							tlRet = E_DRAPI_CANNOT_MAP;
							break;
						}
#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&map_chip_config_end) != DRAPI_OK)
							SPI_PERFORMANCE("get map_chip_config_end time failed\n");
#endif
						SpiChipConfig(xfer.chip_config);
						SpiSetupTransfer(&xfer);

#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&setup_reg_end) != DRAPI_OK)
							SPI_PERFORMANCE("get setup_reg_end time failed\n");
#endif

#ifdef NO_DMA_LENGTH_LIMIT
						packet_loop = spiData->len/1024;
						rest_size = spiData->len % 1024;
						SPI_MSG("packet_loop=%d,rest_size=%d\n",packet_loop,rest_size);

						if(xfer.len <=1024 || rest_size == 0) {
							SpiTransfer(&xfer);

#ifdef SPI_PERF
						if (drApiGetSecureTimestamp(&sngl_trsf_sw)!= DRAPI_OK)
							SPI_PERFORMANCE("get sngl_trsf_sw time failed\n");
#endif
		 				SPI_MSG("single transfer SigWait\n");
#ifndef SPI_TRANSFER_POLLING
				drRet = drApiIpcSigWait();
		                if(DRAPI_OK != drRet) {
		                	SPI_ERR("single transfer SigWait fail:%x\n", drRet);
		                }
#else
				spi_tz_irq_handler();
#endif

#ifdef SPI_PERF
				if (drApiGetSecureTimestamp(&sngl_trsf_hw)!= DRAPI_OK)
					SPI_PERFORMANCE("get sngl_trsf_hw time failed\n");
#endif
				} else {
					chip_config->pause = 1;
					is_last_xfer = 0;
		                    xfer.len = 1024*packet_loop;
		                    SpiTransfer(&xfer);

#ifdef SPI_PERF
				if (drApiGetSecureTimestamp(&fst_trsf_sw)!= DRAPI_OK)
					SPI_PERFORMANCE("get fst_trsf_sw time failed\n");
#endif
				SPI_MSG("twice transfer fst SigWait\n");
#ifndef SPI_TRANSFER_POLLING
				drRet = drApiIpcSigWait();
				if(DRAPI_OK != drRet) {
					SPI_ERR("twice transfer fst SigWait fail:%x\n", drRet);
				}
#else
				spi_tz_irq_handler();
#endif

#ifdef SPI_PERF
				if (drApiGetSecureTimestamp(&fst_trsf_hw)!= DRAPI_OK)
					SPI_PERFORMANCE("get fst_trsf_hw time failed\n");
#endif
				if(Get_timeout_flag()== -1) {
					SPI_MSG("dma goto out of wait\n");
					goto exc_fail ;
				}
				is_last_xfer = 1;
				xfer.len = spiData->len % 1024;
				xfer.tx_buf = xfer.tx_buf + 1024*packet_loop;
				xfer.rx_buf = xfer.rx_buf + 1024*packet_loop;

				SpiTransfer(&xfer);

#ifdef SPI_PERF
				if (drApiGetSecureTimestamp(&scnd_trsf_sw)!= DRAPI_OK)
					SPI_PERFORMANCE("get scnd_trsf_sw time failed\n");
#endif
					SPI_MSG("twice transfer scnd SigWait\n");
#ifndef SPI_TRANSFER_POLLING
					drRet = drApiIpcSigWait();
					if(DRAPI_OK != drRet) {
						SPI_ERR("twice transfer scnd SigWait fail:%x\n", drRet);
					}
#else
					spi_tz_irq_handler();
#endif

#ifdef SPI_PERF
					if (drApiGetSecureTimestamp(&scnd_trsf_hw)!= DRAPI_OK)
						SPI_PERFORMANCE("get scnd_trsf_hw time failed\n");
#endif
					}
#else
					SpiTransfer(&xfer);
#endif

#ifdef SPI_PERF
					if (packet_loop == 0) {
						SPI_PERFORMANCE("[Time1]: %d (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus \n",
		                                            total_len,

						(unsigned int)(sngl_trsf_hw>>32)-(unsigned int)(excute_beg>>32),
						(unsigned int)(sngl_trsf_hw)-(unsigned int)(excute_beg),
						(unsigned int)(sngl_trsf_hw)-(unsigned int)(excute_beg),

						(unsigned int)(map_chip_config_end>>32)-(unsigned int)(excute_beg>>32),
						(unsigned int)(map_chip_config_end)-(unsigned int)(excute_beg),
						(unsigned int)(map_chip_config_end)-(unsigned int)(excute_beg),

						(unsigned int)(sngl_trsf_sw>>32)-(unsigned int)(map_chip_config_end>>32),
						(unsigned int)(sngl_trsf_sw)-(unsigned int)(map_chip_config_end),
						(unsigned int)(sngl_trsf_sw)-(unsigned int)(map_chip_config_end),

						(unsigned int)(sngl_trsf_hw>>32)-(unsigned int)(sngl_trsf_sw>>32),
						(unsigned int)(sngl_trsf_hw)-(unsigned int)(sngl_trsf_sw),
						(unsigned int)(sngl_trsf_hw)-(unsigned int)(sngl_trsf_sw),

						(unsigned int)(scnd_poll_end>>32)-(unsigned int)(scnd_poll_beg>>32),
						(unsigned int)(scnd_poll_end)-(unsigned int)(scnd_poll_beg),
						(unsigned int)(scnd_poll_end)-(unsigned int)(scnd_poll_beg));
					}else{
						SPI_PERFORMANCE("[Time2]: %d (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus (0x%x,0x%x)%dus \n",
		                                            total_len,

						(unsigned int)(scnd_trsf_hw>>32)-(unsigned int)(excute_beg>>32),
						(unsigned int)(scnd_trsf_hw)-(unsigned int)(excute_beg),
						(unsigned int)(scnd_trsf_hw)-(unsigned int)(excute_beg),

						(unsigned int)(map_chip_config_end>>32)-(unsigned int)(excute_beg>>32),
						(unsigned int)(map_chip_config_end)-(unsigned int)(excute_beg),
						(unsigned int)(map_chip_config_end)-(unsigned int)(excute_beg),

						(unsigned int)(fst_trsf_sw>>32)-(unsigned int)(map_chip_config_end>>32),
						(unsigned int)(fst_trsf_sw)-(unsigned int)(map_chip_config_end),
						(unsigned int)(fst_trsf_sw)-(unsigned int)(map_chip_config_end),

						(unsigned int)(fst_trsf_hw>>32)-(unsigned int)(fst_trsf_sw>>32),
						(unsigned int)(fst_trsf_hw)-(unsigned int)(fst_trsf_sw),
						(unsigned int)(fst_trsf_hw)-(unsigned int)(fst_trsf_sw),

						(unsigned int)(scnd_trsf_sw>>32)-(unsigned int)(fst_trsf_hw>>32),
						(unsigned int)(scnd_trsf_sw)-(unsigned int)(fst_trsf_hw),
						(unsigned int)(scnd_trsf_sw)-(unsigned int)(fst_trsf_hw),

						(unsigned int)(scnd_trsf_hw>>32)-(unsigned int)(scnd_trsf_sw>>32),
						(unsigned int)(scnd_trsf_hw)-(unsigned int)(scnd_trsf_sw),
						(unsigned int)(scnd_trsf_hw)-(unsigned int)(scnd_trsf_sw),

						(unsigned int)(scnd_poll_end>>32)-(unsigned int)(scnd_poll_beg>>32),
						(unsigned int)(scnd_poll_end)-(unsigned int)(scnd_poll_beg),
						(unsigned int)(scnd_poll_end)-(unsigned int)(scnd_poll_beg));
					}
#endif
				}else {
					SPI_MSG("FID_DR_EXECUTE spiData->commandId is wrong !!\n");
				}
exc_fail:
				spiData->exc_flag = Get_timeout_flag() ;
		 		SPI_MSG("spiData->timeout_flag = %d\n" ,spiData->exc_flag);
			break;
			case FID_DR_SPI_HW_CONFIG:
				SPI_MSG("FID_DR_SPI_HW_CONFIG & detach IRQ\n");
#ifndef SPI_TRANSFER_POLLING
				SpiIrqDetach();
#endif
			break;
			case FID_DR_SPI_DEBUG:
				SPI_MSG("FID_DR_SPI_DEBUG\n");
 				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
								(addr_t)pMarshal->payload.spi_data,
								sizeof(xfer),
								MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
								(void**)&spiData);
				if(drRet != DRAPI_OK) {
					SPI_ERR("FID_DR_SPI_DEBUG: map spiData task buffer failed");
					tlRet = E_DRAPI_CANNOT_MAP;
					break;
				}
				if(CMD_SPI_DRV_DEBUG == spiData->commandId) {
					dump_chip_config_default();
					dump_reg_config();
					SPI_MSG("CMD_SPI_DRV_DEBUG");
				}else {
					SPI_MSG("FID_DR_SPI_DEBUG spiData->commandId is wrong !!\n");
				}
			break;
			case FID_DR_SPI_TEST:
				SPI_MSG("FID_DR_SPI_TEST\n");
				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
							(addr_t)pMarshal->payload.spi_data,
							sizeof(xfer),
							MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
							(void**)&spiData);
				if(drRet != DRAPI_OK) {
					SPI_MSG("[Driver Drspi] FID_DR_SPI_TEST: map spiData task buffer failed");
					tlRet = E_DRAPI_CANNOT_MAP;
					break;
				}
				if(CMD_SPI_DRV_TEST == spiData->commandId) {
					if(SpiInit() != 0) {
						SPI_MSG("FID_DR_SPI_TEST SPI Init fail");
						break;
					}
					SpiFifoTest();
					SPI_MSG("FID_DR_SPI_TEST");
					}else {
						SPI_MSG("FID_DR_SPI_TEST spiData->commandId is wrong !!\n");
					}
			break;
			case FID_DR_GETONEIMG_EXECUTE:
				SPI_MSG("FID_DR_GETONEIMG_EXECUTE\n");

				SetIrqFlag(IRQ_IDLE);
				timeout_flag = 0;

				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
							(addr_t)pMarshal->payload.spi_data,
							sizeof(xfer),
							MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
							(void**)&spiData);
				if(drRet != DRAPI_OK) {
					SPI_ERR("[Driver Drspi] FID_DR_EXECUTE: map spiData task buffer failed");
					tlRet = E_DRAPI_CANNOT_MAP;
					break;
				}

				if(CMD_SPI_DRV_SEND == spiData->commandId) {
				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
							(addr_t)spiData->rx_buf,
							spiData->len,
							MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
							(void**)&(xfer_p->rx_buf));
				if(drRet != DRAPI_OK) {
					SPI_ERR("CMD_SPI_DRV_SEND: map rx_buf failed, drRet=%d\n", drRet);
					tlRet = E_DRAPI_CANNOT_MAP;
					break;
				}

				xfer.tx_dma = spiData->tx_dma;
				xfer.rx_dma = spiData->rx_dma;
				xfer.len = spiData->len;
				xfer.is_dma_used = spiData->is_dma_used;
				xfer.is_transfer_end = spiData->is_transfer_end;
				xfer.enable_custom = spiData->enable_custom;

				SPI_MSG("tx_dma=0x%x,rx_dma=0x%x,xfer.len=%d\n",
                                        xfer.tx_dma, xfer.rx_dma, xfer.len);
				while(IRQ_BUSY == GetIrqFlag()) {
					/*need a pause instruction to avoid unknow exception*/
					SPI_MSG("IPC wait IRQ handle finish\n");
				}

				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
							(addr_t)spiData->chip_config,
							sizeof(spi_ipc_chip_config),
							MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
							(void**)&(xfer_p->chip_config));
				if(drRet != DRAPI_OK) {
					SPI_ERR("CMD_SPI_DRV_SEND: map chip_config task buffer failed, drRet=%d\n", drRet);
					tlRet = E_DRAPI_CANNOT_MAP;
					break;
				}

				SpiChipConfig(xfer.chip_config);

				SpiSetupTransfer(&xfer);
				spi_read_image(xfer.rx_buf, xfer.len, xfer.enable_custom);
			}else {
				SPI_MSG("FID_DR_GETONEIMG_EXECUTE spiData->commandId is wrong !!\n");
			}
				spiData->exc_flag = Get_timeout_flag() ;
				SPI_MSG("spiData->timeout_flag = %d\n" ,spiData->exc_flag);
				break;
			default:
				break;
			}
			pMarshal->payload.retVal = tlRet;
			SPI_MSG("spi drApiUnmapTaskBuffers\n");
			drApiUnmapTaskBuffers(THREADID_TO_TASKID(ipcClient));
		}
		ipcMsg  = MSG_RS;
		ipcData = 0;
		break;
		default:
			drApiIpcUnknownMessage(&ipcClient, &ipcMsg, &ipcData);
			/* Unknown message has been received*/
			ipcMsg = MSG_RS;
			ipcData = (message_t) E_TLAPI_DRV_UNKNOWN;
		break;
		}
	}
}

/********************************
 * IPC handler thread entry point
********************************/
_THREAD_ENTRY void drIpch( void )
{
	drIpchLoop();
}


void drIpchInit( const addr_t dciBuffer,
		 const uint32_t  dciBufferLen)
{
	clearStack(drIpchStack);

/* DCI message buffer */
	message = (dciMessage_t*) dciBuffer;

/***********************************************
* DCI buffer data can be accessed via message
* pointer.
**********************************************/

/*******************************************************************
* Start IPC handler thread. Exception handler thread becomes local
* exception handler of IPC handler thread
*******************************************************************/
	if (DRAPI_OK != drApiStartThread(
					DRIVER_THREAD_NO_IPCH,
					FUNC_PTR(drIpch),
					getStackTop(drIpchStack),
					IPCH_PRIORITY,
					DRIVER_THREAD_NO_EXCH)) {
		SPI_MSG("[Driver Drspi] drIpchInit(): drApiStartThread failed");
	}

}
