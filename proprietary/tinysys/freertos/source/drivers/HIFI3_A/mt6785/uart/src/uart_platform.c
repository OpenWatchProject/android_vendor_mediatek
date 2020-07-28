
#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <mt_uart.h>



void uart_platform_setting(void)
{
    /* Hifi3 CG settings enable uart*/
    UART_SET_BITS(ADSP_CK_CTRL_BASE, UART0_CG_ENABLE);
    /* enable clock and release reset*/
    UART_SET_BITS(UART_CLK_CG_CTRL,
                  (UART0_CLK | UART0_RST));
#ifndef CONFIG_MTK_FPGA
/* adspuart0 GPIO 200/201 */
    DRV_WriteReg32(0x10005490, (DRV_Reg32(0x10005490)&0xFFFFFF88)|0x00000022);

#endif
}
