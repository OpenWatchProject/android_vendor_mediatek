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
#define MT6582_SPI_SECURE_IRQ_ID    (SPI_GIC_PRIVATE_SIGNALS+147)

static void spi_recv_check(struct spi_transfer *xfer)
{
	uint32_t cnt, i;
	cnt = (xfer->len%4)?(xfer->len/4 + 1):(xfer->len/4);
	for(i=0; i<cnt; i++) {
		if(*( ( uint32_t * ) xfer->rx_buf + i ) != *( ( uint32_t * ) xfer->tx_buf + i )) {
			SPI_MSG("Error!! tx xfer %d is:%08x\n",i,  *( ( uint32_t * ) xfer->tx_buf + i ) );
			SPI_MSG("Error!! rx xfer %d is:%08x\n",i,  *( ( uint32_t * ) xfer->rx_buf + i ) );
		}
	}
}


void SpiHandleIRQ(void)
{
	//struct mt_spi_t *ms = (struct mt_spi_t *)dev_id;
	struct spi_message  *msg;
	struct spi_transfer *xfer;
	struct mt_chip_conf *chip_config;

	unsigned long flags;
	uint32_t reg_val, i, cnt_quotient, cnt_remainder;
	uint8_t *p8_Tmp;
	SPI_MSG("------------------<3> SpiHandleIRQ Start read data---------------\n");

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

	/*Using FIFO/DMA to receive data*/
	/*Default mode: LITTLE-ENDIAN*/
	cnt_quotient = xfer->len / 4;
	cnt_remainder = xfer->len % 4;
	SPI_MSG("cnt_quotient = %d, cnt_remainder = %d\n", cnt_quotient, cnt_remainder);

	if(chip_config->com_mod == FIFO_TRANSFER) {
		SPI_MSG("FIFO start to read data xfer->len:%d!!\n", xfer->len);

		/*Handling 4bytes data*/
		for(i = 0; i < cnt_quotient; i++) {
				reg_val = SPI_READ(SPI_REG_RX_DATA);
				if(i < 4)
					SPI_MSG("rx_buf[%d] data(4B Align) is:%08x\n", i, reg_val);
				*((uint32_t *)xfer->rx_buf + i) = reg_val;
		}

		/*Handling less than 4bytes data*/
		if (cnt_remainder) {
			reg_val = SPI_READ(SPI_REG_RX_DATA);
			SPI_MSG("rx_buf data(None 4B Align):%08x\n", reg_val);

			/*Convert the 32bits point(point to less than 4bytes data) to 8bits point*/
			p8_Tmp = (uint8_t *)((uint32_t *)xfer->rx_buf + cnt_quotient);

			for(i = 0; i < cnt_remainder; i++) {
				*(p8_Tmp + i) = (uint8_t)(reg_val >> (i * 8));
				SPI_MSG("rx_buf[%d] data(disintegrate):%02x\n", i, *(p8_Tmp + i));
			}
		}
	} else if(chip_config->com_mod == DMA_TRANSFER) {
		SPI_MSG("DMA start to read data xfer->len:%d!!\n", xfer->len);

		/*Handling 4bytes data*/
		for(i = 0; i < cnt_quotient; i++) {
			reg_val = SPI_READ(SPI_RX_DMA_VA_BASE + 4 * i);
			if(i < 4)
				SPI_MSG("rx_buf[%d] data(4B Align) is:%08x\n", i, reg_val);
			*((uint32_t *)xfer->rx_buf + i) = reg_val;
		}

		/*Handling less than 4bytes data*/
		if (cnt_remainder) {

			/*Convert the 32bits point(point to less than 4bytes data) to 8bits point*/
			p8_Tmp = (uint8_t *)((uint32_t *)xfer->rx_buf + cnt_quotient);

			for(i = 0; i < cnt_remainder; i++) {
				*(p8_Tmp + i) = SPI_READ8((SPI_RX_DMA_VA_BASE + 4 * cnt_quotient) + i);
				SPI_MSG("rx_buf[%d] data(None 4B Align):%02x\n", i, *(p8_Tmp + i));
			}
		}
	}
    if(chip_config->com_mod == DMA_TRANSFER) {
		SPI_MSG("SpiDMAUnMapping\n");
	    SpiDMAUnMapping();
	}
	if(is_last_xfer == 1 && xfer->is_transfer_end == 1) {
		SetPauseStatus(IDLE);
	   #if 0
		if(chip_config->com_mod == DMA_TRANSFER) {
	          SpiDMAUnMapping();
		}
	   #endif
		ResetSpi();
		SPI_MSG("Pause set IDLE state & disable clk\n");
	}

	SetIrqFlag(IRQ_IDLE);

#ifndef SPI_TRANSFER_POLLING
	const threadid_t threadId = drApiGetLocalThreadid(DRIVER_THREAD_NO_IPCH);
	drApiResult_t drRes = drApiIpcSignal(threadId);
	if (DRAPI_OK != drRes) {
		SPI_MSG("spi wakeup Signal fail, %x\n", drRes);
	}
#endif
	SPI_MSG("------------------<4> SPI HandleIRQ/Transfer end------------------\n");
	return;
}

extern uint64_t scnd_poll_beg;
extern uint64_t scnd_poll_end;
int spi_tz_irq_handler(void)
{
    const uint32_t startIndex = 0;
    uint32_t i;
    uint32_t irqStatus;
    drApiResult_t ret = DRAPI_OK;

#ifdef SPI_TRANSFER_POLLING
#ifdef SPI_PERF
	if (drApiGetSecureTimestamp(&scnd_poll_beg)!= DRAPI_OK)
                                   SPI_PERFORMANCE("get scnd_poll_beg time failed\n");
#endif
	do {
		irqStatus = SPI_READ(SPI_REG_STATUS1);
	} while (irqStatus == 0);
#ifdef SPI_PERF
	if (drApiGetSecureTimestamp(&scnd_poll_end)!= DRAPI_OK)
                                   SPI_PERFORMANCE("get scnd_poll_end time failed\n");
#endif
#else
	irqStatus = SPI_READ(SPI_REG_STATUS0) & 0x00000003;
	SPI_MSG("IRQ: irqStatus[0x%08x]\n", irqStatus);
#endif
	SpiHandleIRQ();

	return 0;
}

_NORETURN void SpiIrqhLoop( void )
{
    uint32_t timeout_us = 100;
    intrNo_t spi_irq = 0;
    drApiResult_t drRet = DRAPI_OK;

    //entry loop to wait irq
    for (;;) {
        //drRet = drApiWaitForIntr(MT6582_SPI_SECURE_IRQ_ID, timeout_us, &spi_irq);
        drRet = drApiWaitForIntr(MT6582_SPI_SECURE_IRQ_ID, TIME_INFINITE, &spi_irq);

        if(DRAPI_OK != drRet) {
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
        switch (spi_irq) {
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
    drApiResult_t attachedRet = drApiIntrAttach(MT6582_SPI_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL);
    //active low level-sensitive
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
     //drApiResult_t attachedRet = drApiIntrAttach(MT6582_SPI_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL);
     //active low level-sensitive
     //SPI_MSG("drApiIntrAttach, IRQ[%d], res = [%d]\n", MT6582_SPI_SECURE_IRQ_ID, attachedRet);
     // start irq handler thread
    drApiResult_t status = drApiStartThread(
                    DRIVER_THREAD_NO_IRQH,
                    FUNC_PTR(SpiIrqhEntry),
                    getStackTop(SpiIrqhStack),
                    IRQH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH);
    if (DRAPI_OK != status) {
        SPI_MSG("SpiIrqAttach thread failed[%d]\n");
    }
}

void SpiIrqDetach(void)
{
    drApiResult_t status = drApiStopThread(DRIVER_THREAD_NO_IRQH);
    if (DRAPI_OK != status) {
        SPI_MSG("SpiIrqDetach thread failed[%d]\n");
    }
    drApiResult_t attachedRet = drApiIntrDetach(MT6582_SPI_SECURE_IRQ_ID); //active low level-sensitive
    SPI_MSG("drApiIntrDetach!!\n");
}

