#include <errno.h>
#include <string.h>
#include <interrupt.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <spifreertos.h>
#include <wakelock.h>
#include <semphr.h>

#include <driver_api.h>
#include <FreeRTOS.h>


#define SPI0 0
#define SPI1 1

#define BUFFER_SIZE 256
static void spi_transfer_test(void)
{
#if 1
    int ret = 0;
	size_t len = 0;
	int i;
	char send_buffer[BUFFER_SIZE]/* = {0} {0x12, 0x34, 0x56, 0x78,
		                             0x13, 0x35, 0x57, 0x79,
		                             0x14, 0x36, 0x58, 0x89,
		                             0x15, 0x37, 0x59, 0x99,
		                             0x16}*/;
	char recv_buffer[BUFFER_SIZE];

    spi_init();
	for (i = 0; i< BUFFER_SIZE; i++)
		send_buffer[i] = (char)i + 1;
	while(1) {
        /**********MDAmode len > 1024 test & interrput*********/
		len = BUFFER_SIZE;
		ret = SCPSpiRxTxSync(SPI1, (void *)recv_buffer, (void *)send_buffer,
                      len, NULL);
		if (ret < 0)
			SPI_ERR("spi test fail ...............\n");
		for (i = 0; i < len; i++) {
			SPI_ERR("spi recv_buffer[%d] = 0x%x, send_buffer[%d] = 0x%x\n", i, recv_buffer[i], i, send_buffer[i]);
			if (recv_buffer[i] != send_buffer[i]) {
			    SPI_ERR("spi recv_buffer[%d] = 0x%x, send_buffer[%d] = 0x%x\n", i, recv_buffer[i], i, send_buffer[i]);
				configASSERT(0);
			}
		}
        memset(recv_buffer, 0, BUFFER_SIZE);
        SPI_ERR("spi dma test done ...............\n");
        /*****************FIFO test & interrupt***************************/
		len = 17;
		ret = SCPSpiRxTxSync(SPI1, (void *)recv_buffer, (void *)send_buffer,
                      len, NULL);
		if (ret < 0)
			SPI_ERR("spi test fail ...............\n");
		for (i = 0; i < len; i++) {
			SPI_ERR("spi recv_buffer[%d] = 0x%x, send_buffer[%d] = 0x%x\n", i, recv_buffer[i], i, send_buffer[i]);
			if (recv_buffer[i] != send_buffer[i]) {
			    SPI_ERR("spi recv_buffer[%d] = 0x%x, send_buffer[%d] = 0x%x\n", i, recv_buffer[i], i, send_buffer[i]);
				configASSERT(0);
			}
		}
        memset(recv_buffer, 0, BUFFER_SIZE);
		SPI_ERR("spi test done ...............\n");
		vTaskDelay(10000 / portTICK_RATE_MS);
	}
#endif
}

void mtk_spi_task_init(void);
void mtk_spi_task_init(void) {

    PRINTF_E("spi_task create ......................\n");

	xTaskHandle xHandle;

	xTaskCreate((TaskFunction_t) spi_transfer_test,
		"spi_test",
		2048,
		(void *) 0,
		2,
		&xHandle);
}
