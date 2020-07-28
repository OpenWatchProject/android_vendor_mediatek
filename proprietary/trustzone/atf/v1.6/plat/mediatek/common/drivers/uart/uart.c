/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <uart.h>
#include <plat_uart.h>
#include <debug.h>
#include <delay_timer.h>
#include <assert.h>

struct mtk_uart uart_save_addr[DRV_SUPPORT_UART_PORTS];

const uint32_t uart_base_addr[DRV_SUPPORT_UART_PORTS] = {
	UART_BASE0,
	UART_BASE1,
	0,
	0,
	0,
	0,
};

void mtk_uart_restore(void)
{
	uint32_t base;
	int uart_idx = UART_PORT0;
	struct mtk_uart *uart;

	/* Attention!! Must NOT print any debug log before UART restore */
	for (uart_idx = UART_PORT0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		uart = &uart_save_addr[uart_idx];
		assert(uart != NULL);
		base = uart->base;
		assert(base != 0);

		mmio_write_32(UART_LCR(base), 0xbf);
		mmio_write_32(UART_EFR(base), uart->registers.efr);
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		mmio_write_32(UART_FCR(base), uart->registers.fcr);

		/* baudrate */
		mmio_write_32(UART_HIGHSPEED(base), uart->registers.highspeed);
		mmio_write_32(UART_FRACDIV_L(base), uart->registers.fracdiv_l);
		mmio_write_32(UART_FRACDIV_M(base), uart->registers.fracdiv_m);
		mmio_write_32(UART_LCR(base), uart->registers.lcr | UART_LCR_DLAB);
		mmio_write_32(UART_DLL(base), uart->registers.dll);
		mmio_write_32(UART_DLH(base), uart->registers.dlh);
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		mmio_write_32(UART_SAMPLE_COUNT(base), uart->registers.sample_count);
		mmio_write_32(UART_SAMPLE_POINT(base), uart->registers.sample_point);
		mmio_write_32(UART_GUARD(base), uart->registers.guard);

		/* flow control */
		mmio_write_32(UART_ESCAPE_EN(base), uart->registers.escape_en);
		mmio_write_32(UART_MCR(base), uart->registers.mcr);
		mmio_write_32(UART_SCR(base), uart->registers.scr);
		mmio_write_32(UART_IER(base), uart->registers.ier);
	}
}

void mtk_uart_save(void)
{
	int uart_idx = UART_PORT0;
	uint32_t base;
	struct mtk_uart *uart;

	for (uart_idx = UART_PORT0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		uart_save_addr[uart_idx].base = uart_base_addr[uart_idx];
		base = uart_base_addr[uart_idx];
		assert(base != 0);
		uart = &uart_save_addr[uart_idx];
		assert(uart != NULL);
		uart->registers.lcr = mmio_read_32(UART_LCR(base));

		mmio_write_32(UART_LCR(base), 0xbf);
		uart->registers.efr = mmio_read_32(UART_EFR(base));
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		uart->registers.fcr = mmio_read_32(UART_FCR_RD(base));

		/* baudrate */
		uart->registers.highspeed = mmio_read_32(UART_HIGHSPEED(base));
		uart->registers.fracdiv_l = mmio_read_32(UART_FRACDIV_L(base));
		uart->registers.fracdiv_m = mmio_read_32(UART_FRACDIV_M(base));
		mmio_write_32(UART_LCR(base), uart->registers.lcr | UART_LCR_DLAB);
		uart->registers.dll = mmio_read_32(UART_DLL(base));
		uart->registers.dlh = mmio_read_32(UART_DLH(base));
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		uart->registers.sample_count = mmio_read_32(UART_SAMPLE_COUNT(base));
		uart->registers.sample_point = mmio_read_32(UART_SAMPLE_POINT(base));
		uart->registers.guard = mmio_read_32(UART_GUARD(base));

		/* flow control */
		uart->registers.escape_en = mmio_read_32(UART_ESCAPE_EN(base));
		uart->registers.mcr = mmio_read_32(UART_MCR(base));
		uart->registers.scr = mmio_read_32(UART_SCR(base));
		uart->registers.ier = mmio_read_32(UART_IER(base));
	}
}

uint32_t mtk_uart_sleep(void)
{
	uint32_t uart_idx = UART_PORT0;
	uint32_t base;
	uint32_t sleep_req = 0, i = 0;

	for (uart_idx = UART_PORT0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {

		base = uart_base_addr[uart_idx];
		assert(base != 0);
		/* request UART to sleep */
		sleep_req = mmio_read_32(UART_SLEEP_REQ(base));
		mmio_write_32(UART_SLEEP_REQ(base),
		    sleep_req | MTK_UART_SEND_SLEEP_REQ);

		/* wait for UART to ACK */
		while (!(mmio_read_32(UART_SLEEP_ACK(base))
			& MTK_UART_SLEEP_ACK_IDLE)) {
			if (i++ >= MTK_UART_WAIT_ACK_TIMES) {
				mmio_write_32(UART_SLEEP_REQ(base), sleep_req);
				ERROR("CANNOT GET UART%d SLEEP ACK\n", uart_idx);

				return UART_PM_ERROR;
			}

			udelay(10);
		}
	}
	return 0;
}

uint32_t mtk_uart_wakeup(void)
{
	uint32_t i = 0;
	int sleep_req = 0;
	uint32_t uart_idx = UART_PORT0;
	uint32_t base;

	for (uart_idx = 0; uart_idx < HW_SUPPORT_UART_PORTS; uart_idx++) {
		base = uart_base_addr[uart_idx];
		assert(base != 0);
		/* wakeup uart */
		sleep_req = mmio_read_32(UART_SLEEP_REQ(base));
		mmio_write_32(UART_SLEEP_REQ(base),
			sleep_req & (~MTK_UART_SEND_SLEEP_REQ));

		/* wait for UART to ACK */
		while (mmio_read_32(UART_SLEEP_ACK(base))
			& MTK_UART_SLEEP_ACK_IDLE) {
			if (i++ >= MTK_UART_WAIT_ACK_TIMES) {
				mmio_write_32(UART_SLEEP_REQ(base), sleep_req);
				ERROR("CANNOT GET UART%d WAKE ACK\n", uart_idx);
				return UART_PM_ERROR;
			}
			udelay(10);
		}
	}
	return 0;
}
