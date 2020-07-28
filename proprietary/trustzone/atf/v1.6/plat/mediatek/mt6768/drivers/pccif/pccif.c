/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <debug.h>
#include <mmio.h>
#include <mtk_plat_common.h>
#include <platform.h>
#include <platform_def.h>
#include <console.h>
#include <amms.h>
#include <string.h>

#define read32(addr)		mmio_read_32((addr))
#define write32(addr, val)	mmio_write_32((addr), (val))
#define sync_write32(addr, val)	do { mmio_write_32((addr), (val)); dsbsy(); } while (0)

/*******************************************************************************
 * PCCIF1
 ******************************************************************************/
#define PCCIF1_AP_BASE		(0x1020B000)
#define PCCIF1_AP_SIZE		(0x1000)
#define PCCIF1_DATA_SIZE	(0x100)
#define PCCIF1_CH_NUM		(4)

/* PCCIF1 releate register */
#define PCCIF1_CON		(PCCIF1_AP_BASE + 0x00000000)
#define PCCIF1_BUSY		(PCCIF1_AP_BASE + 0x00000004)
#define PCCIF1_START		(PCCIF1_AP_BASE + 0x00000008)
#define PCCIF1_TCHNUM		(PCCIF1_AP_BASE + 0x0000000C)
#define PCCIF1_RCHNUM		(PCCIF1_AP_BASE + 0x00000010)
#define PCCIF1_ACK		(PCCIF1_AP_BASE + 0x00000014)
#define PCCIF1_IRQ0_MASK	(PCCIF1_AP_BASE + 0x00000020)
#define PCCIF1_IRQ1_MASK	(PCCIF1_AP_BASE + 0x00000024)
#define PCCIF1_DATA		(PCCIF1_AP_BASE + 0x00000100)

/*--- deprecated, no need now Register spec ----------------------------------------------*/
#if 0
#define PCCIF1_CON_SEQ		(0x0) /* sequential */
#define PCCIF1_CON_ARB		(0x1) /* arbitrate */
#endif

#define LOG_TAG "PCCIF"
#define PCCIF_DEBUG 0
#define CONSOLE_INIT() console_init(gteearg.atf_log_port,\
	UART_CLOCK, UART_BAUDRATE)
#if PCCIF_DEBUG
#define PCCIF_ERROR(fmt, ...) \
	do { \
		CONSOLE_INIT(); \
		ERROR("[%s][ERROR] %s:%d: " fmt, LOG_TAG,\
		__func__, __LINE__, ##__VA_ARGS__); \
		console_uninit(); \
	} while (0)
#define PCCIF_NOTICE(fmt, ...) \
	do { \
		CONSOLE_INIT();\
		NOTICE("[%s][NOTICE] %s:%d: " fmt, LOG_TAG,\
		__func__, __LINE__, ##__VA_ARGS__); \
		console_uninit(); \
	} while (0)
#define PCCIF_STAMP(fmt, ...) \
	INFO("[%s][stamp] %s: " fmt, LOG_TAG,\
		__func__, ##__VA_ARGS__)

#define PCCIF_INFO(fmt, ...)\
		INFO("[%s][NOTICE] %s:%d: " fmt, LOG_TAG,\
		__func__, __LINE__, ##__VA_ARGS__) \

#else
/*
 * #define PCCIF_ERROR(fmt, ...)
 * tf_log("[%s][ERROR] %s: " fmt, LOG_TAG, __func__, ##__VA_ARGS__)
#define PCCIF_NOTICE(fmt, ...)
tf_log("[%s][NOTICE] %s: " fmt, LOG_TAG, __func__, ##__VA_ARGS__)
*/
#define PCCIF_ERROR(fmt, ...) \
	ERROR("[%s][ERROR] %s: " fmt, LOG_TAG, __func__, ##__VA_ARGS__)
#define PCCIF_NOTICE(fmt, ...)
#define PCCIF_INFO(fmt, ...)
#define PCCIF_STAMP(fmt, ...)
#endif


void ccif_irq1_handler(void __unused * cookie) /* For CCIF reserve use */
{
	unsigned int i;
	unsigned int rx_ch_bit_map;
	unsigned int ack_val;

	PCCIF_NOTICE("enter %s\n", __func__);
	rx_ch_bit_map = read32(PCCIF1_RCHNUM);
	rx_ch_bit_map &= ((1<<2)|(1<<3)); /* bit 2 and bit 3 for irq 0 */
	for (i = 0; i < (PCCIF1_CH_NUM/2); i++) {
		ack_val = (4<<i);
		if (rx_ch_bit_map & ack_val)
			sync_write32(PCCIF1_ACK, ack_val);
	}

	rx_ch_bit_map = rx_ch_bit_map >> 2;
	/* Do somthing for chn bit map */
}

void pccif1_hw_init(void)
{
	/* depreated , no need to set Using arbitration mode */
	/* sync_write32(PCCIF1_CON, PCCIF1_CON_ARB);*/
	/* IRQ0 using bit 0,1 */
	sync_write32(PCCIF1_IRQ0_MASK, ((1<<1)|(1<<0)));
	/* IRQ0 using bit 2,3 */
	sync_write32(PCCIF1_IRQ1_MASK, ((1<<2)|(1<<3)));
}

static int ccif_read(unsigned int buf[], unsigned int size, unsigned int offset, unsigned int barrier)
{
	unsigned int i;
	unsigned int ptr;

	if (buf == NULL)
		return -2;
	if (size > (PCCIF1_DATA_SIZE/2/4))
		return -3;
	if (offset >= (PCCIF1_DATA_SIZE/4))
		return -4;

	PCCIF_INFO("%s,size=%d,offset=%d,barrier=%d\n",
		__func__, size, offset, barrier);
	ptr = PCCIF1_DATA + offset * 4;
	for (i = 0; i < size; i++) {
		buf[i] = read32(ptr);
		ptr += 4;
		if ((offset + i) >= barrier)
			break;
	}
	return i;
}

static int ccif_write(unsigned int buf[], unsigned int size, unsigned int offset, unsigned int barrier)
{
	unsigned int i;
	unsigned int ptr;

	if (buf == NULL)
		return -1;
	if (size > (PCCIF1_DATA_SIZE/2/4))
		return -2;
	if (offset >= (PCCIF1_DATA_SIZE/4))
		return -3;

	ptr = PCCIF1_DATA + offset * 4;
	for (i = 0; i < size; i++) {
		PCCIF_NOTICE("%s:i=%d buf[i]=0x%x size=%d\n",
			__func__, i, buf[i], size);
		write32(ptr, buf[i]);
		ptr += 4;
		if ((offset + i) >= barrier)
			break;
	}
	return i;
}

static int ccif_notify_md(unsigned int ch_id)
{
	if (ch_id >= PCCIF1_CH_NUM)
		return -2;
	sync_write32(PCCIF1_TCHNUM, ch_id);
	return 0;
}

int ccif_irq0_user_read(unsigned int buf[], unsigned int size, unsigned int offset)
{
	if (offset > (PCCIF1_DATA_SIZE/4/2))
		return -1;
	return ccif_read(buf, size, offset, PCCIF1_DATA_SIZE/4/2);
}

int ccif_irq1_user_read(unsigned int buf[], unsigned int size, unsigned int offset)
{
	if (offset > (PCCIF1_DATA_SIZE/4/2))
		return -1;
	return ccif_read(buf, size, offset + (PCCIF1_DATA_SIZE/4/2), PCCIF1_DATA_SIZE/4);
}

int ccif_irq0_user_write(unsigned int buf[], unsigned int size, unsigned int offset)
{
	if (offset > (PCCIF1_DATA_SIZE/4/2))
		return -1;
	return ccif_write(buf, size, offset, PCCIF1_DATA_SIZE/4/2);
}

int ccif_irq1_user_write(unsigned int buf[], unsigned int size, unsigned int offset)
{
	if (offset > (PCCIF1_DATA_SIZE/4/2))
		return -1;
	return ccif_write(buf, size, offset + (PCCIF1_DATA_SIZE/4/2), PCCIF1_DATA_SIZE/4);
}

int ccif_irq0_user_notify_md(unsigned int ch)
{
	if (ch >= (PCCIF1_CH_NUM/2))
		return -1;
	return ccif_notify_md(ch);
}

int ccif_irq1_user_notify_md(unsigned int ch)
{
	if (ch >= (PCCIF1_CH_NUM/2))
		return -1;
	return ccif_notify_md(ch + (PCCIF1_CH_NUM/2));
}

void ccif_irq0_handler(void __unused * cookie) /* For CCIF AMMS use */
{
	unsigned int i;
	unsigned int rx_ch_bit_map;
	unsigned int ack_val;
	struct amms_msg_request request;

	PCCIF_STAMP("\n");
	memset(&request, 0, sizeof(request));
	PCCIF_NOTICE("enter %s\n", __func__);
	rx_ch_bit_map = read32(PCCIF1_RCHNUM);
	rx_ch_bit_map &= ((1<<1)|(1<<0)); /* bit 0 and bit 1 for irq 0 */
	for (i = 0; i < (PCCIF1_CH_NUM/2); i++) {
		ack_val = (1<<i);
		if (rx_ch_bit_map & ack_val)
			sync_write32(PCCIF1_ACK, ack_val);
	}

	if (ccif_irq0_user_read((unsigned int *)&request, ((sizeof(request)/4)+((sizeof(request)%4)?1:0)), 0) < 0) {
		PCCIF_ERROR("ccif_irq0_user_read failed\n");
	} else
		handle_amms_request(&request);
	/* Do somthing for chn bit map */
}

static int curr_amms_pos_flag;
/*1: allocated 0:deallocated*/
void pseudo_ccif_handler(void __unused * cookie) /* For CCIF AMMS use */
{
	struct amms_msg_request request;

	memset(&request, 0, sizeof(request));
	if (curr_amms_pos_flag == 1) {
		request.cmd = AMMS_CMD_DEALLOCATE_POS_BUFFER;
		curr_amms_pos_flag = 0;
	} else {
		request.cmd = AMMS_CMD_ALLOCATE_POS_BUFFER;
		curr_amms_pos_flag = 0;
	}

	PCCIF_NOTICE("We are in ccif_irq0_handler\n");
	handle_amms_request(&request);
}
