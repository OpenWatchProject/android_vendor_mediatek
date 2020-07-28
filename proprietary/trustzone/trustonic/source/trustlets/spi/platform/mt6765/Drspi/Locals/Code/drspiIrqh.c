#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drCommon.h"

#include "spi_debug.h"
#include "spi_regs.h"
#include "spi.h"

extern void SpiIrqDetach(void);
DECLARE_STACK(SpiIrqhStack, 50*1024);
/** 
 * tbase IRQ register note
 *  .secure world IRQ ID is same as normal world (platform depenedent)
 *  .tbase IRQ porting desen't cover General Interrupt Control's IRQ init currently, 
 *   whether the IRQ will be received in secure world only, 
 *   we have to register IRQ_id to GIC in normalworld to ensure correct IRQ init.
 *   otherwise, me may receive IRQ at strange timing
 */
#define SPI_GIC_PRIVATE_SIGNALS               (32)
#define MT6582_SPI_SECURE_IRQ_ID    (SPI_GIC_PRIVATE_SIGNALS+118)

static void spi_recv_check(struct spi_transfer *xfer)
{
	uint32_t cnt, i;
	
	cnt = (xfer->len%4)?(xfer->len/4 + 1):(xfer->len/4);
	for(i=0; i<cnt; i++)
	{
		if(*( ( uint32_t * ) xfer->rx_buf + i ) != *( ( uint32_t * ) xfer->tx_buf + i ))
		{ 					
			SPI_MSG("Error!! tx xfer %d is:%x\n",i,  *( ( uint32_t * ) xfer->tx_buf + i ) );
			SPI_MSG("Error!! rx xfer %d is:%x\n",i,  *( ( uint32_t * ) xfer->rx_buf + i ) ); 
		}
	}
	
}


void SpiHandleIRQ(void)
{
	//struct mt_spi_t *ms = (struct mt_spi_t *)dev_id;
	struct spi_message	*msg;
	struct spi_transfer	*xfer;
	struct mt_chip_conf *chip_config;

	unsigned long flags;
	uint32_t reg_val,cnt;
	uint32_t i;	
	SPI_MSG(" SpiHandleIRQ\n");

	xfer = GetSpiTransfer();
	chip_config = GetChipConfig();

	//reg_val = SPI_READ(SPI_REG_STATUS0);	
	//SPI_MSG("xfer:0x%p interrupt status:%x\n",xfer,reg_val & 0x3);
	
	if(NULL == chip_config) {
		SPI_ERR("chip_config is NULL!!\n");
		return;
	}

	/*pause mode*/
	if(chip_config->pause){
		if( GetPauseStatus() == INPROGRESS ){
			SetPauseStatus(PAUSED);
			SPI_MSG("IRQ set PAUSED state\n");
		}else{
			SPI_ERR("Wrong spi status.\n" );
		}
	}else 
	{
		SetPauseStatus(IDLE);
		SPI_MSG("IRQ set IDLE state\n");
	}
	
    SPI_MSG("start to read data !!\n");
	/*FIFO*/
	if((chip_config->com_mod == FIFO_TRANSFER) && xfer->rx_buf){	
		SPI_MSG("xfer->len:%d\n", xfer->len);
		cnt = (xfer->len%4)?(xfer->len/4 + 1):(xfer->len/4);
		for(i = 0; i < cnt; i++){			
			reg_val = SPI_READ(SPI_REG_RX_DATA); /*get the data from rx*/	
			SPI_MSG("SPI_RX_DATA_REG:0x%x\n", reg_val);
			*((uint32_t *)xfer->rx_buf + i) = reg_val;
		}
	}	
	else if((chip_config->com_mod == DMA_TRANSFER) && xfer->rx_buf){		
		cnt = (xfer->len%4)?(xfer->len/4 + 1):(xfer->len/4);
		for(i = 0; i < cnt; i++){	
			reg_val = SPI_READ(SPI_RX_DMA_VA_BASE+4*i); /*get the data from rx*/	
			//SPI_MSG("SPI_RX_ADDR_DMA:0x%x\n", SPI_RX_DMA_VA_BASE+get_pa_offset(xfer->rx_dma)+4*i);
			//SPI_MSG("SPI_RX_DATA_DMA:0x%x\n", reg_val);
			*((uint32_t *)xfer->rx_buf + i) = reg_val;
			//SPI_MSG("rx_buf:0x%x\n", *((uint32_t *)xfer->rx_buf + i));
		}
	}
	
    //if(chip_config->com_mod == DMA_TRANSFER)
	  //spi_recv_check(xfer);
    if(chip_config->com_mod == DMA_TRANSFER) {
		SPI_MSG("SpiDMAUnMapping\n");
	    SpiDMAUnMapping();
	}
	if(is_last_xfer == 1 && xfer->is_transfer_end == 1)
	{
		SetPauseStatus(IDLE);
	   #if 0
		if(chip_config->com_mod == DMA_TRANSFER) {
	          SpiDMAUnMapping();
		}
	   #endif
		ResetSpi();
		SPI_MSG("Pause set IDLE state & disable clk\n");
	}	

	/*set irq flag to ask SpiNextMessage continue to run*/
	//SPI_MSG("Peng set IRQ\n");
	
	SetIrqFlag(IRQ_IDLE);
	//SPI_MSG("Detach IRQ and IRQ status is %d\n", GetIrqFlag());
	const threadid_t threadId = drApiGetLocalThreadid(DRIVER_THREAD_NO_IPCH);
	drApiResult_t drRes = drApiIpcSignal(threadId);
	if (DRAPI_OK != drRes) {
		SPI_MSG("spi wakeup Signal fail, %x\n", drRes);
	}
	SPI_MSG("-----------------xfer end-----------------------\n");
	return;	
}

void spi_tz_irq_handler(void) 
{
    const uint32_t startIndex = 0;  
    uint32_t i;
    uint32_t irqStatus;
    drApiResult_t ret = DRAPI_OK; 
    
    irqStatus = SPI_READ(SPI_REG_STATUS0) & 0x00000003;
    
    SPI_MSG("IRQ: irqStatus[0x%08x]\n", irqStatus);          

	SpiHandleIRQ();

}

_NORETURN void SpiIrqhLoop( void )
{
    uint32_t timeout_us = 100; 
    intrNo_t spi_irq = 0;
    drApiResult_t drRet = DRAPI_OK;
    
    //entry loop to wait irq
    for (;;)
    {
        //drRet = drApiWaitForIntr(MT6582_SPI_SECURE_IRQ_ID, timeout_us, &spi_irq);
        drRet = drApiWaitForIntr(MT6582_SPI_SECURE_IRQ_ID, TIME_INFINITE, &spi_irq);
        
        if(DRAPI_OK != drRet)
        {
            #if 1
            // driver is loaded by mobicore daemon during bootup, disabled log to prevent performance degrade
            SPI_ERR(" drApiWaitForIntr err[0x%08x], error major code[0x%08x], irq[%d]\n", 
                drRet,
                DRAPI_ERROR_MAJOR(drRet),
                MT6582_SPI_SECURE_IRQ_ID);
            #endif
            
            continue;
        }

        // irq handle
        switch (spi_irq)
        {
            case (MT6582_SPI_SECURE_IRQ_ID):
                spi_tz_irq_handler();
                break;
            default: 
                SPI_ERR("never go here: dose not handle irq[%d]\n", spi_irq);
                break;
        }        
            
    }
}


_THREAD_ENTRY void SpiIrqhEntry( void )
{
    drApiResult_t attachedRet = drApiIntrAttach(MT6582_SPI_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL); //active low level-sensitive
    SPI_MSG("drApiIntrAttach, IRQ[%d], res = [%d]\n", MT6582_SPI_SECURE_IRQ_ID, attachedRet); 

    SpiIrqhLoop();
}


void SpiIrqhInit(void)
{
    SPI_MSG("SpiIrqhInit, IRQ thread number:%d\n", DRIVER_THREAD_NO_IRQH); 

    // ensure thread stack is clean
    clearStack(SpiIrqhStack);
}

void SpiIrqAttach(void)
{
     //drApiResult_t attachedRet = drApiIntrAttach(MT6582_SPI_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL); //active low level-sensitive
     //SPI_MSG("drApiIntrAttach, IRQ[%d], res = [%d]\n", MT6582_SPI_SECURE_IRQ_ID, attachedRet); 
	// start irq handler thread
    drApiResult_t status = drApiStartThread(
                    DRIVER_THREAD_NO_IRQH,
                    FUNC_PTR(SpiIrqhEntry),
                    getStackTop(SpiIrqhStack),
                    IRQH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH);
    if (DRAPI_OK != status)
    {
        SPI_MSG("SpiIrqAttach thread failed[%d]\n");
    }
}

void SpiIrqDetach(void)
{
	drApiResult_t status = drApiStopThread(DRIVER_THREAD_NO_IRQH);
    if (DRAPI_OK != status)
    {
        SPI_MSG("SpiIrqDetach thread failed[%d]\n");
    }
	drApiResult_t attachedRet = drApiIntrDetach(MT6582_SPI_SECURE_IRQ_ID); //active low level-sensitive
    SPI_MSG("drApiIntrDetach!!\n"); 
}

