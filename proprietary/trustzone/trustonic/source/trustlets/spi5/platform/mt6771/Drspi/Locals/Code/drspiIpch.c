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

DECLARE_STACK(drIpchStack, 50*1024);

/* Static variables */
static dciMessage_t *message;

extern int32_t SpiInit(void);
extern void SpiIrqAttach(void);
extern void SpiIrqDetach(void);

int timeout_flag = 0;


void Set_timeout_flag(void)
{
	timeout_flag = -1 ;

}
int Get_timeout_flag(void)
{
	return timeout_flag;
}

uint8_t is_last_xfer = 1;
static uint8_t spi_tx_buf_p[1024] = {0x0};
static uint8_t spi_rx_buf_p[1024] = {0x0};

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
    //spi_transfer_ptr            spiData = NULL;
    struct spi_transfer    *spiData=NULL;
	struct spi_transfer     xfer;
	struct spi_transfer     xfer_connect;
	struct mt_chip_conf     spi_ipc_chip_config;
	struct spi_transfer    *xfer_p = &xfer;

    for(;;)
    {
        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData))
        {
            SPI_MSG("[Driver Drspi] drIpchLoop(): drApiIpcCallToIPCH failed\n");
            continue;
        }

		SPI_MSG("[Driver Drspi] Received command from trustlet (MSG_RQ_EX, API LEVEL:%d)\n", TBASE_API_LEVEL);
        ipcMsgId = drApiExtractMsgCmd(ipcMsg);
        /* Dispatch request */
        switch(ipcMsgId)
        {
            case MSG_CLOSE_TRUSTLET:
                /*
                 * Trustlet close message
                 */
                SPI_MSG("[Driver Drspi] drIpchLoop(): Acknowledging trustlet close\n");
                /* Close active sessions owned by trustlet, which is being shutdown */
                //For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
                drSmgmtCloseSessionForThread(ipcData);
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_CLOSE_DRIVER:
                /*
                 * Driver close message
                 */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_RQ:
            case MSG_RQ_EX:
//#ifdef DR_FEATURE_TL_API                
                /*
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshalling data
                 */
          /*
                pMarshal = (drMarshalingParam_ptr)drApiMapClientAndParams(
                                            ipcClient,
                                            ipcData);
                  */
                drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
                                           (addr_t)ipcData,
                                           sizeof(drMarshalingParam_t),
                                           MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
                                           (void**)&pMarshal);
                if(drRet != DRAPI_OK) {
					tlRet = E_DRAPI_CANNOT_MAP;
					ipcMsg  = MSG_RS;
                    SPI_MSG("[Driver Drspi] drIpchLoop(): map task buffer failed");
					break;
                }
				
                if(pMarshal)
                {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {

                    case FID_DR_OPEN_SESSION:
                        /*
                         * Handle open session request
                         */
						SPI_MSG("[Driver Drspi] FID_DR_OPEN_SESSION & Attach FIQ\n");
						/*confrim IRQ had been done, and Detach IRQ had been done*/
						#if 0
						while(IRQ_BUSY == GetIrqFlag()) {
		                        /*need a pause instruction to avoid unknow exception*/
		                        SPI_MSG("IPC wait IRQ handle finish at open\n");
                        }
						#endif
						/*Attach FIQ when open SPI driver*/
						SpiIrqAttach();
                        pMarshal->sid = drSmgmtOpenSession(ipcClient);
                        /*
                         * Update IPC status as success. If there is an error with opening session
                         * invalid sid will indicatethat opening session failed
                         */
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_CLOSE_SESSION:
                        /*
                         * Handle close session request based on the sid provided by the client
                         */
						SPI_MSG("[Driver Drspi] FID_DR_CLOSE_SESSION\n"); 
					    SpiIrqDetach();
                        drSmgmtCloseSession(pMarshal->sid);
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_INIT_DATA:
                        /*
                         * Initialize sesion data
                         */
                        SPI_MSG("[Driver Drspi] FID_DR_INIT_DATA\n");  	
                        if (E_DR_SMGMT_OK == drSmgmtSetSessionData(
                                pMarshal->sid,
                                ipcClient,
                                (pMarshal->payload.spi_data)))
                        {
                            tlRet = TLAPI_OK;
                        }
                        break;
                    case FID_DR_SPI_EXECUTE:
						SPI_MSG("[Driver Drspi] FID_DR_EXECUTE\n");
              /*
                         * TODO: Read registry data compare threadids to make sure that client is allowed to use
                         * registry data. Then execute the command and update tlRet accordingly
                         */
                     //   ResetSpi();	
                		SetIrqFlag(IRQ_IDLE);
						timeout_flag = 0 ;
                     //   SetPauseStatus(IDLE);
                    #if 1
                        //spiData = drApiAddrTranslateAndCheck(pMarshal->payload.spi_data);
                        drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)pMarshal->payload.spi_data,
												    sizeof(xfer),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&spiData);
					    if(drRet != DRAPI_OK) {
							    SPI_MSG("[Driver Drspi] FID_DR_EXECUTE: map spiData task buffer failed");
                                tlRet = E_DRAPI_CANNOT_MAP;
							    break;
					    }
                        if(CMD_SPI_DRV_SEND == spiData->commandId)
                        {
                
							uint32_t  packet_loop, rest_size;
							uint32_t i;
								
                        	//xfer.tx_buf = drApiAddrTranslateAndCheck(spiData->tx_buf);
							//xfer.rx_buf = drApiAddrTranslateAndCheck(spiData->rx_buf);
							drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)spiData->tx_buf,
												       //sizeof(xfer.tx_buf),
												       spiData->len,
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->tx_buf));
							if(drRet != DRAPI_OK) {
								SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND: map tx_buf task buffer failed");
                                tlRet = E_DRAPI_CANNOT_MAP;
								break;
							}
							xfer.tx_buf = xfer_p->tx_buf;
							xfer_connect.tx_buf = xfer_p->tx_buf;
							drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)spiData->rx_buf,
												       //sizeof(xfer.rx_buf),
												       spiData->len,
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->rx_buf));
							if(drRet != DRAPI_OK) {
								SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND: map tx_buf task buffer failed, drRet=%d\n", drRet);
                                tlRet = E_DRAPI_CANNOT_MAP;
								break;
							}
							xfer.rx_buf = xfer_p->rx_buf;
							xfer_connect.rx_buf = xfer_p->rx_buf;
							xfer.tx_dma = spiData->tx_dma;
							xfer.rx_dma = spiData->rx_dma;
							xfer.len = spiData->len;
							xfer.is_dma_used = spiData->is_dma_used;
							xfer.is_transfer_end = spiData->is_transfer_end;
							
 						    SPI_MSG("[Driver Drspi] tx_dma=0x%x,rx_dma=0x%x\n",xfer.tx_dma ,xfer.rx_dma );
							SPI_MSG("[Driver Drspi] xfer.len=%d, xfer.is_dma_used=%d\n ",xfer.len,xfer.is_dma_used);
							SPI_MSG("[Driver Drspi] IRQ status=%d\n ",GetIrqFlag());
                            while(IRQ_BUSY == GetIrqFlag()) {
		                        /*need a pause instruction to avoid unknow exception*/
		                        SPI_MSG("IPC wait IRQ handle finish\n");
                            }
							//SpiIrqAttach();
							/*spi HW setting*/
							//SpiChipConfig(spiData->chip_config);
							drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)spiData->chip_config,
												       //sizeof(xfer),
												       sizeof(spi_ipc_chip_config),
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->chip_config));
							if(drRet != DRAPI_OK) {
								SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND: map chip_config task buffer failed, drRet=%d\n", drRet);
                                tlRet = E_DRAPI_CANNOT_MAP;
								break;
							}
							xfer.chip_config = xfer_p->chip_config;
							//SpiChipConfig(drApiAddrTranslateAndCheck(spiData->chip_config));
							SpiChipConfig(xfer.chip_config);
							
							SpiSetupTransfer(&xfer);
							//dump_reg_config();
							
                            SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND\n");
							//SpiNextXfer(&xfer);
							#ifdef NO_DMA_LENGTH_LIMIT
							packet_loop = spiData->len/1024;
							rest_size = spiData->len % 1024;
							SPI_MSG("[Driver Drspi] packet_loop=%d,rest_size=%d\n",packet_loop,rest_size);
							
							if(xfer.len <=1024 || rest_size == 0)
							{
								//if(xfer.is_dma_used == 1)
								//	ResetSpi();
								SpiTransfer(&xfer);
							    //if(IRQ_BUSY == GetIrqFlag()) {
						           SPI_MSG("spi drApiIpcSigWait3\n");
						           drRet = drApiIpcSigWait();
                            
						           if(DRAPI_OK != drRet) {
				                       SPI_MSG("spi drApiIpcSigWait fail3, %x\n", drRet);
	                               }
					            //}
							}
							else
							{
								chip_config->pause = 1;
								is_last_xfer = 0;	
								xfer.len = 1024*packet_loop;
								SpiTransfer(&xfer);
								//while(IRQ_BUSY == GetIrqFlag()); 
//								if(IRQ_BUSY == GetIrqFlag()) {
									SPI_MSG("spi drApiIpcSigWait1\n");
						            drRet = drApiIpcSigWait();
                            
						            if(DRAPI_OK != drRet) {
				                       SPI_MSG("spi drApiIpcSigWait fail1, %x\n", drRet);
	                                }
//					             }

								if(Get_timeout_flag()== -1)
								{
									SPI_MSG("dma goto out of wait\n");
									goto exc_fail ;
								}	
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)(spiData->tx_buf + packet_loop*1024),
												       //sizeof(xfer.tx_buf),
												       (spiData->len % 1024),
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->tx_buf));
							    if(drRet != DRAPI_OK) {
								    SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND: map tx_buf task buffer failed\n");
                                    tlRet = E_DRAPI_CANNOT_MAP;
								    break;
							    }
							    xfer.tx_buf = xfer_p->tx_buf;
								
								for(i=0;i<rest_size;i++)
								{
									spi_tx_buf_p[i] =	*( ( uint8_t * ) xfer.tx_buf + i); 
								    //SPI_MSG("[Driver Drspi] xfer.tx_buf[%d]=%x\n",i,spi_tx_buf_p[i]);
								}	
								is_last_xfer = 1;	
								xfer.len = spiData->len % 1024;
								xfer.tx_buf = spi_tx_buf_p;	
								xfer.rx_buf = spi_rx_buf_p;	
								//SpiIrqAttach();
								SpiTransfer(&xfer);
								//while(IRQ_BUSY == GetIrqFlag()); 
//								if(IRQ_BUSY == GetIrqFlag()) {						
									SPI_MSG("spi drApiIpcSigWait2\n");
						            drRet = drApiIpcSigWait();
                            
						            if(DRAPI_OK != drRet) {
				                       SPI_MSG("spi drApiIpcSigWait fail2, %x\n", drRet);
	                                }
//					             }

								//xfer.rx_buf = drApiAddrTranslateAndCheck(spiData->rx_buf);
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)(spiData->rx_buf + packet_loop*1024),
												       //sizeof(xfer.rx_buf),
												       (spiData->len % 1024),
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->rx_buf));
							    if(drRet != DRAPI_OK) {
								    SPI_MSG("[Driver Drspi] CMD_SPI_DRV_SEND: map tx_buf task buffer failed drRet=%d\n", drRet);
                                    tlRet = E_DRAPI_CANNOT_MAP;
								    break;
							    }
								xfer.rx_buf = xfer_p->rx_buf;
								
								for(i=0;i<rest_size;i++)
								{
									*( ( uint8_t * ) xfer.rx_buf + i)=spi_rx_buf_p[i]; 
		                            //SPI_MSG("[Driver Drspi] xfer.rx_buf[%d]=%x",i,spi_rx_buf[i]);
								}	
							}
							#else
							    //SpiIrqAttach();
								SpiTransfer(&xfer);
							#endif
                        }else {
							SPI_MSG("FID_DR_EXECUTE spiData->commandId is wrong !!\n");
                        }
                    #endif
					exc_fail:
							spiData->exc_flag = Get_timeout_flag() ;
							SPI_MSG("spiData->timeout_flag = %d\n" ,spiData->exc_flag);
                        break;  
					case FID_DR_SPI_HW_CONFIG:
						SPI_MSG("[Driver Drspi] FID_DR_SPI_HW_CONFIG & detach IRQ\n");
						SpiIrqDetach();
						#if 0
                        //spiData = drApiAddrTranslateAndCheck(pMarshal->payload.spi_data);
						drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)pMarshal->payload.spi_data,
												    sizeof(xfer),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&spiData);
						if(drRet != DRAPI_OK) {
							SPI_MSG("[Driver Drspi] FID_DR_SPI_HW_CONFIG: map spiData task buffer failed");
                            tlRet = E_DRAPI_CANNOT_MAP;
							break;
						}
						if(CMD_SPI_DRV_CONFIG == spiData->commandId)
                        {
                        	struct spi_transfer xfer;
                            //xfer.chip_config = drApiAddrTranslateAndCheck(spiData->chip_config);
							drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										               (addr_t)spiData->chip_config,
												       sizeof(xfer),
													   MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													   (void**)&(xfer_p->chip_config));
							if(drRet != DRAPI_OK) {
								SPI_MSG("[Driver Drspi] CMD_SPI_DRV_CONFIG: map chip_config task buffer failed");
                                tlRet = E_DRAPI_CANNOT_MAP;
								break;
							}
							/*spi HW setting*/
							SpiChipConfig(xfer.chip_config);
							SpiChipConfigSet();						
                            SPI_MSG("[Driver Drspi] CMD_SPI_DRV_CONFIG");
                   
                        }else {
							SPI_MSG("FID_DR_SPI_HW_CONFIG spiData->commandId is wrong !!\n");
                        }
						#endif
					    break;
					case FID_DR_SPI_DEBUG:
					    SPI_MSG("[Driver Drspi] FID_DR_SPI_DEBUG\n");
						//spiData = drApiAddrTranslateAndCheck(pMarshal->payload.spi_data);
						drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)pMarshal->payload.spi_data,
												    sizeof(xfer),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&spiData);
						if(drRet != DRAPI_OK) {
							SPI_MSG("[Driver Drspi] FID_DR_SPI_DEBUG: map spiData task buffer failed");
                            tlRet = E_DRAPI_CANNOT_MAP;
							break;
						}
						if(CMD_SPI_DRV_DEBUG == spiData->commandId) {
                            dump_chip_config_default();
					        dump_reg_config();
							SPI_MSG("[Driver Drspi] CMD_SPI_DRV_DEBUG");
						}else {
							SPI_MSG("FID_DR_SPI_DEBUG spiData->commandId is wrong !!\n");
                        }
					    break;
					case FID_DR_SPI_TEST:
					    SPI_MSG("[Driver Drspi] FID_DR_SPI_TEST\n");
						//spiData = drApiAddrTranslateAndCheck(pMarshal->payload.spi_data);
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
							  SPI_MSG("[Driver Drspi] FID_DR_SPI_TEST SPI Init fail");
							  break;
							}
							SpiFifoTest();
							SPI_MSG("[Driver Drspi] FID_DR_SPI_TEST");
						}else {
							SPI_MSG("FID_DR_SPI_TEST spiData->commandId is wrong !!\n");
                        }
					    break;
                    default:
                        /* Unknown message has been received*/
                        break;
                    }
					pMarshal->payload.retVal = tlRet;
					SPI_MSG("spi drApiUnmapTaskBuffers\n");
					drApiUnmapTaskBuffers(THREADID_TO_TASKID(ipcClient));
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = 0;
//#endif                
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

/*
 * IPC handler thread entry point
 */
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}

/**
 * IPC handler thread init
 */
void drIpchInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /* DCI message buffer */
    message = (dciMessage_t*) dciBuffer;

    /*
     * DCI buffer data can be accessed via message
     * pointer.
     */

    /*
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_IPCH,
                    FUNC_PTR(drIpch),
                    getStackTop(drIpchStack),
                    IPCH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
        SPI_MSG("[Driver Drspi] drIpchInit(): drApiStartThread failed");
    }

}
