/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <limits.h>
#include <log.h>
#include <mt_cpuxgpt.h>
#include <platform.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#define ATF_SCHED_CLOCK_UNIT 1000000000 //ns
#define TIMESTAMP_BUFFER_SIZE 32
int (*log_lock_acquire)();
int (*log_write)(unsigned char);
int (*log_lock_release)();

/***********************************************************
 * The tf_printf implementation for all BL stages
 ***********************************************************/

#define get_num_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, long long int) :	\
	((lcount) ? va_arg(args, long int) : va_arg(args, int)))

#define get_unum_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, unsigned long long int) :	\
	((lcount) ? va_arg(args, unsigned long int) : va_arg(args, unsigned int)))

static void string_print(const char *str)
{
	while (*str){
		if (log_write)
			(*log_write)(*str);
		putchar(*str++);
	}
}

/* reverse:  reverse string s in place */
static void reverse(char s[])
{
	int c, i, j;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* itoa:  convert n to characters in s */
static int utoa(unsigned int n, char s[])
{
	int i;

	i = 0;
	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	s[i] = '\0';
	reverse(s);
	return i;
}
static int ltoa(unsigned long long n, char s[], unsigned add_zero)
{
	int i = 0;

	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */

	while(add_zero && i < 6){
		s[i++] = '0'; /* add '0' for log readability */
	}
	s[i] = '\0';

	reverse(s);

	return i;
}

#ifdef AARCH32
#define unsigned_num_print(unum, radix)			\
	do {						\
		if ((radix) == 16)			\
			unsigned_hex_print(unum);	\
		else if ((radix) == 10)			\
			unsigned_dec_print(unum);	\
		else					\
			string_print("tf_printf : Unsupported radix");\
	} while (0);

/*
 * Utility function to print an unsigned number in decimal format for AArch32.
 * The function doesn't support printing decimal integers higher than 32 bits
 * to avoid having to implement 64-bit integer compiler library functions.
 */
static void unsigned_dec_print(unsigned long long int unum)
{
	unsigned int local_num;
	/* Just need enough space to store 32 bit decimal integer */
	unsigned char num_buf[10];
	int i = 0, rem;

	if (unum > UINT_MAX) {
		string_print("tf_printf : decimal numbers higher than 32 bits"
				" not supported\n");
		return;
	}

	local_num = (unsigned int)unum;

	do {
		rem = local_num % 10;
		num_buf[i++] = '0' + rem;
	} while (local_num /= 10);

	while (--i >= 0)
		putchar(num_buf[i]);
}

/*
 * Utility function to print an unsigned number in hexadecimal format for
 * AArch32. The function doesn't use 64-bit integer arithmetic to avoid
 * having to implement 64-bit compiler library functions. It splits the
 * 64 bit number into two 32 bit numbers and converts them into equivalent
 * ASCII characters.
 */
static void unsigned_hex_print(unsigned long long int unum)
{
	/* Just need enough space to store 16 characters */
	unsigned char num_buf[16];
	int i = 0, rem;
	uint32_t num_local = 0, num_msb = 0;

	/* Get the LSB of 64 bit unum */
	num_local = (uint32_t)unum;
	/* Get the MSB of 64 bit unum. This works only on Little Endian */
	assert((read_sctlr() & SCTLR_EE_BIT) == 0);
	num_msb = *(((uint32_t *) &unum) + 1);

	do {
		do {
			rem = (num_local & 0xf);
			if (rem < 0xa)
				num_buf[i++] = '0' + rem;
			else
				num_buf[i++] = 'a' + (rem - 0xa);
		} while (num_local >>= 4);

		num_local = num_msb;
		num_msb = 0;
	} while (num_local);

	while (--i >= 0)
		putchar(num_buf[i]);
}

#else

static void unsigned_num_print(unsigned long long int unum, unsigned int radix)
{
	/* Just need enough space to store 64 bit decimal integer */
	unsigned char num_buf[20];
	int i = 0, rem;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i++] = '0' + rem;
		else
			num_buf[i++] = 'a' + (rem - 0xa);
	} while (unum /= radix);

	while (--i >= 0){
		if (log_write)
			(*log_write)(num_buf[i]);
		putchar(num_buf[i]);
	}
}
#endif /* AARCH32 */

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following type specifiers are supported by this print
 * %x - hexadecimal format
 * %s - string format
 * %d or %i - signed decimal format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following length specifiers are supported by this print
 * %l - long int (64-bit on AArch64)
 * %ll - long long int (64-bit on AArch64)
 * %z - size_t sized integer formats (64 bit on AArch64)
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
#define dcache_check()	\
	unsigned long sec_sctlr_el3 = read_sctlr_el3();	\
	if ((sec_sctlr_el3 & 0x4) == 0) {	\
		/* Data Cache is disable in secure world */	\
		/* panic(); */	\
		return;	\
	}

extern uint64_t normal_time_base;
static char timestamp_buf[TIMESTAMP_BUFFER_SIZE] __attribute__((aligned(8)));
void tf_printf(const char *fmt, ...)
{
	va_list args;
	int l_count;
	long long int num;
	unsigned long long int unum;
	unsigned long long int tmpunum;
	char *str;
	unsigned long long cur_time;
	unsigned long long sec_time;
	unsigned long long ns_time;
	int 	count;
	char	*timestamp_bufptr = timestamp_buf;
	unsigned int digit = 0;
	unsigned int tmpdigit = 0;

	/* try get buffer lock */
	if (log_lock_acquire){
		/* use bakery lock instead of spinlock */
		/* we do not need to check if Dcache is enabled */
		(*log_lock_acquire)();
	}

	/* in ATF boot time, tiemr for cntpct_el0 is not initialized
	 * so it will not count now.
	 */
#if !defined(ATF_BYPASS_DRAM)
	cur_time = sched_clock();
#else
    cur_time = 0;
#endif
	sec_time = cur_time / ATF_SCHED_CLOCK_UNIT;
	ns_time = (cur_time % ATF_SCHED_CLOCK_UNIT)/1000;

	*timestamp_bufptr++ = '[';
	*timestamp_bufptr++ = 'A';
	*timestamp_bufptr++ = 'T';
	*timestamp_bufptr++ = 'F';
	*timestamp_bufptr++ = ']';
	*timestamp_bufptr++ = '(';
	count = utoa(platform_get_core_pos(read_mpidr()), timestamp_bufptr);
	timestamp_bufptr += count;
	*timestamp_bufptr++ = ')';
	if(MT_LOG_KTIME){
		*timestamp_bufptr++ = 'K';
		*timestamp_bufptr++ = ':';
	}
	*timestamp_bufptr++ = '[';
	count = ltoa(sec_time, timestamp_bufptr, 0);
	timestamp_bufptr += count;
	*timestamp_bufptr++ = '.';
	count = ltoa(ns_time, timestamp_bufptr, 1);
	timestamp_bufptr += count;
	*timestamp_bufptr++ = ']';
	*timestamp_bufptr++ = '\0';

	timestamp_buf[TIMESTAMP_BUFFER_SIZE - 1] = '\0';
	count = 0;
	while (timestamp_buf[count])
	{
		/* output char to ATF log buffer */
		if (log_write)
			(*log_write)(timestamp_buf[count]);
		putchar(timestamp_buf[count]);
		count++;
	}

	va_start(args, fmt);
	while (*fmt) {
		l_count = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case '0': /* Print leading '0' */
				if(*(fmt+1) > '0' && *(fmt+1) <= '9') {
					fmt++;
					if(*(fmt) == '1' && *(fmt+1) > '0' && *(fmt+1) <= '6') {
						fmt++;
						digit = *fmt - '0' + 10;
					}
					else {
						digit = *fmt - '0';
					}
				}
				fmt++;
				goto loop;
				break;
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					putchar('-');
					if (log_write)
						(*log_write)('-');
					unum = (unsigned long long int)-num;
				} else
					unum = (unsigned long long int)num;

				/* Print leading '0' if any */
				if(digit){
					tmpunum = unum / 10;
					tmpdigit = 1;
					while ((tmpunum)){
						tmpunum /= 10;
						tmpdigit++;
					}
					digit = digit > tmpdigit ? (digit - tmpdigit) : 0;
					for (tmpdigit=0; tmpdigit < digit; tmpdigit++) {
						putchar('0');
						if (log_write)
							(*log_write)('0');
					}
				}
				unsigned_num_print(unum, 10);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(str);
				break;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum)
					string_print("0x");

				unsigned_num_print(unum, 16);
				break;
			case 'x':
				unum = get_unum_va_args(args, l_count);
				/* Print leading '0' if any */
				if(digit){
					tmpunum = unum >> 4;
					tmpdigit = 1;
					while ((tmpunum)){
						tmpunum = tmpunum>> 4;
						tmpdigit++;
					}
					digit = digit > tmpdigit ? (digit - tmpdigit) : 0;
					for (tmpdigit=0; tmpdigit < digit; tmpdigit++) {
						putchar('0');
						if (log_write)
							(*log_write)('0');
					}
				}
				unsigned_num_print(unum, 16);
				break;
			case 'z':
				if (sizeof(size_t) == 8)
					l_count = 2;

				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'u':
				unum = get_unum_va_args(args, l_count);
								/* Print leading '0' if any */
				if(digit){
					tmpunum = unum / 10;
					tmpdigit = 1;
					while ((tmpunum)){
						tmpunum /= 10;
						tmpdigit++;
					}
					digit = digit > tmpdigit ? (digit - tmpdigit) : 0;
					for (tmpdigit=0; tmpdigit < digit; tmpdigit++) {
						putchar('0');
						if (log_write)
							(*log_write)('0');
					}
				}
				unsigned_num_print(unum, 10);
				break;
			default:
				/* Exit on any other format specifier */
				goto exit;
			}
			digit=0;
			fmt++;
			continue;
		}
		if (log_write)
			(*log_write)(*fmt);
		putchar(*fmt++);
	}
exit:
	va_end(args);

	/* release buffer lock */
	if (log_lock_release)
		(*log_lock_release)();
}


void bl31_log_service_register(int (*lock_get)(),
    int (*log_putc)(unsigned char),
    int (*lock_release)())
{
    log_lock_acquire = lock_get;
    log_write = log_putc;
    log_lock_release = lock_release;
}
