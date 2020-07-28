/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <debug.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(MTK_PLAT_PORTING_LAYER)
#include <arch_helpers.h>	/* for read_mpidr() */
#include <log.h>
#include <mt_cpuxgpt.h>
#include <platform.h>
#include <string.h>

#define ATF_SCHED_CLOCK_UNIT 1000000000 /* ns */
#define TIMESTAMP_BUFFER_SIZE 32
int (*log_lock_acquire)();
int (*log_write)(unsigned char);
int (*log_lock_release)();
extern uint64_t normal_time_base;
static char timestamp_buf[TIMESTAMP_BUFFER_SIZE] __attribute__((aligned(8)));

#endif


#define get_num_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, long int) :		\
			    va_arg(_args, int)))

#define get_unum_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, unsigned long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, unsigned long int) :		\
			    va_arg(_args, unsigned int)))

static int string_print(const char *str)
{
	int count = 0;

	assert(str != NULL);

	for ( ; *str != '\0'; str++) {
		(void)putchar(*str);
		count++;
	}

	return count;
}

static int unsigned_num_print(unsigned long long int unum, unsigned int radix,
			      char padc, int padn)
{
	/* Just need enough space to store 64 bit decimal integer */
	char num_buf[20];
	int i = 0, count = 0;
	unsigned int rem;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i] = '0' + rem;
		else
			num_buf[i] = 'a' + (rem - 0xa);
		i++;
		unum /= radix;
	} while (unum > 0U);

	if (padn > 0) {
		while (i < padn) {
			(void)putchar(padc);
			count++;
			padn--;
		}
	}

	while (--i >= 0) {
		(void)putchar(num_buf[i]);
		count++;
	}

	return count;
}

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
 * The following padding specifiers are supported by this print
 * %0NN - Left-pad the number with 0s (NN is a decimal number)
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
int vprintf(const char *fmt, va_list args)
{
	int l_count;
	long long int num;
	unsigned long long int unum;
	char *str;
	char padc = '\0'; /* Padding character */
	int padn; /* Number of characters to pad */
	int count = 0; /* Number of printed characters */

#if defined(MTK_PLAT_PORTING_LAYER)
	/* try get buffer lock */
	if (log_lock_acquire){
		/* use bakery lock instead of spinlock */
		/* we do not need to check if Dcache is enabled */
		(*log_lock_acquire)();
	}
	/* print overwritten msg if it is overwritten */
	print_overwritten_msg();
	print_log_timestamp();
#endif

	while (*fmt != '\0') {
		l_count = 0;
		padn = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					(void)putchar('-');
					unum = (unsigned long long int)-num;
					padn--;
				} else
					unum = (unsigned long long int)num;

				count += unsigned_num_print(unum, 10,
							    padc, padn);
				break;
			case 's':
				str = va_arg(args, char *);
				count += string_print(str);
				break;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum > 0U) {
					count += string_print("0x");
					padn -= 2;
				}

				count += unsigned_num_print(unum, 16,
							    padc, padn);
				break;
			case 'x':
				unum = get_unum_va_args(args, l_count);
				count += unsigned_num_print(unum, 16,
							    padc, padn);
				break;
			case 'z':
				if (sizeof(size_t) == 8U)
					l_count = 2;

				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				count += unsigned_num_print(unum, 10,
							    padc, padn);
				break;
			case '0':
				padc = '0';
				padn = 0;
				fmt++;

				for (;;) {
					char ch = *fmt;
					if ((ch < '0') || (ch > '9')) {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
			default:
#if defined(MTK_PLAT_PORTING_LAYER)
				/* release buffer lock */
				if (log_lock_release)
					(*log_lock_release)();
#endif
				/* Exit on any other format specifier */
				return -1;
			}

			fmt++;
			continue;
		}
		(void)putchar(*fmt);
		fmt++;
		count++;
	}

#if defined(MTK_PLAT_PORTING_LAYER)
	/* release buffer lock */
	if (log_lock_release)
		(*log_lock_release)();
#endif

	return count;
}

int printf(const char *fmt, ...)
{
	int count;
	va_list va;

	va_start(va, fmt);
	count = vprintf(fmt, va);
	va_end(va);

	return count;
}

#if defined(MTK_PLAT_PORTING_LAYER)
void bl31_log_service_register(int (*lock_get)(),
    int (*log_putc)(unsigned char),
    int (*lock_release)())
{
    log_lock_acquire = lock_get;
    log_write = log_putc;
    log_lock_release = lock_release;
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

void print_log_timestamp(void)
{
	unsigned long long cur_time;
	unsigned long long sec_time;
	unsigned long long ns_time;
	int 	count;
	char	*timestamp_bufptr = timestamp_buf;

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
	if (MT_LOG_KTIME) {
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
	while (timestamp_buf[count]) {
		putchar(timestamp_buf[count]);
		count++;
	}
}

void print_overwritten_msg(void)
{
	const char	*log_overwritten_msg = "===Log is overwritten===";
	int	count;
	/* write_pos cross read_pos */
	/* clear flag and print out overwritten message */
	if (get_is_log_overwritten() == 1) {
		clr_is_log_overwritten();
		count = 0;
		while (log_overwritten_msg[count] != '\0') {
			putchar(log_overwritten_msg[count]);
			count++;
		}
	}
}
#endif

