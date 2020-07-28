/*
 * Copyright (c) 2014 - 2016 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <msee_fwk/drv_fwk.h>
#include <msee_fwk/drv_defs.h>
#include <msee_fwk/drv_error.h>
#include <tz_private/system.h>
#include <tz_private/log.h>

#define LOG_TAG "MSEE_DRV_IRQ"
#define INFINITE_TIME UINT32_MAX

static unsigned int gTimeout = INFINITE_TIME;

MSEE_Result msee_request_irq(unsigned int irq, msee_irq_handler_t handler, unsigned long flags, unsigned int timeout_ms /*ms*/, void *data)
{
	// fix me: todo add callback handler and parameter to finish this function

	gTimeout = timeout_ms;
	return MTEE_RequestIrq(irq, flags);
}

void msee_free_irq(unsigned int irq)
{
	MTEE_ReleaseIrq(irq);
}

MSEE_Result msee_wait_for_irq_complete(void)
{
	return MTEE_WaitIrq(gTimeout);
}


