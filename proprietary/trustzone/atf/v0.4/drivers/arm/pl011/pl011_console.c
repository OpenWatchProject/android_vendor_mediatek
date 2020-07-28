/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <console.h>
#include <pl011.h>

extern void mtk_uart_init (unsigned int  uart_port, unsigned int  uartclk, unsigned int  baudrate);
extern void PutUARTByte (const char c);
extern int GetUARTBytes(unsigned char *buf, unsigned int size, unsigned int tmo_ms);

static unsigned long uart_base;
static unsigned long IsOutputToUARTFlag=1;

void set_uart_flag(void)
{
    IsOutputToUARTFlag=1;
}
void clear_uart_flag(void)
{
    IsOutputToUARTFlag=0;
}


void console_init(unsigned long base_addr)
{
	/* TODO: assert() internally calls printf() and will result in
	 * an infinite loop. This needs to be fixed with some kind of
	 * exception  mechanism or early panic support. This also applies
	 * to the other assert() calls below.
	 */
	assert(base_addr);

	/* Initialise internal base address variable */
	uart_base = base_addr;

#if 0   // do not init UART in ATF, preloader will do it
        // no platform dependent code
#define     CFG_LOG_BAUDRATE    921600

#if CFG_FPGA_PLATFORM
#define UART_SRC_CLK_FRQ                (12000000)
#else /* !CFG_FPGA_PLATFORM */
#define UART_SRC_CLK_FRQ                (0)         /* use default */
#endif

#if CFG_FPGA_PLATFORM
    /* init uart baudrate when pll on */
    mtk_uart_init(base_addr, UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
#endif
#endif
}

#define WAIT_UNTIL_UART_FREE(base) while ((pl011_read_fr(base)\
					& PL011_UARTFR_TXFF) == 1)
int console_putc(int c)
{
#if 1
    if(IsOutputToUARTFlag){
        PutUARTByte (c);
    }
#else
	assert(uart_base);

	if (c == '\n') {
		WAIT_UNTIL_UART_FREE(uart_base);
		pl011_write_dr(uart_base, '\r');
	}

	WAIT_UNTIL_UART_FREE(uart_base);
	pl011_write_dr(uart_base, c);
#endif
	return c;
}

int console_getc(void)
{
#if 1
    unsigned char c = 0;
    if(IsOutputToUARTFlag){
        GetUARTBytes(&c, 1, 10);
    }
    return c;
#else
	assert(uart_base);

	while ((pl011_read_fr(uart_base) & PL011_UARTFR_RXFE) != 0)
		;
	return pl011_read_dr(uart_base);
#endif
}
