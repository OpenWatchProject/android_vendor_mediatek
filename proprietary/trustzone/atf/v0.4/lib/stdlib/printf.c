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

#include <stdio.h>
#include <stdarg.h>
#include "mt_cpuxgpt.h"

int (*log_lock_acquire)();
int (*log_write)(unsigned char);
int (*log_lock_release)();

int (*log_lock_acquire2)();
int (*log_write2)(unsigned char);
int (*log_lock_release2)();


/* Choose max of 128 chars for now. */
#define PRINT_BUFFER_SIZE 128
#define TIMESTAMP_BUFFER_SIZE 32
#define ATF_SCHED_CLOCK_UNIT 1000000000 //ns


int printf(const char *fmt, ...)
{
	va_list args;
	char buf[PRINT_BUFFER_SIZE];
	int count;
	char timestamp_buf[TIMESTAMP_BUFFER_SIZE];
	unsigned long long cur_time;
	unsigned long long sec_time;
	unsigned long long ns_time;

    /* try get buffer lock */
    if (log_lock_acquire)
        (*log_lock_acquire)();

    /* in ATF boot time, tiemr for cntpct_el0 is not initialized
     * so it will not count now. 
     */
    cur_time = atf_sched_clock();
    sec_time = cur_time / ATF_SCHED_CLOCK_UNIT;
    ns_time = (cur_time % ATF_SCHED_CLOCK_UNIT)/1000;

    snprintf(timestamp_buf, sizeof(timestamp_buf) - 1,
            "[ATF][%6llu.%06llu]", sec_time, ns_time);

	timestamp_buf[TIMESTAMP_BUFFER_SIZE - 1] = '\0';
	count = 0;
	while (timestamp_buf[count])
	{
                /* output char to ATF log buffer */
                if (log_write)
                    (*log_write)(timestamp_buf[count]);
		if (putchar(timestamp_buf[count]) != EOF) {
			count++;
		} else {
			count = EOF;
			break;
		}
	}

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);
    
	/* Use putchar directly as 'puts()' adds a newline. */
	buf[PRINT_BUFFER_SIZE - 1] = '\0';
	count = 0;
	while (buf[count])
	{
        /* output char to ATF log buffer */
        if (log_write)
            (*log_write)(buf[count]);
        
		if (putchar(buf[count]) != EOF) {
			count++;       
		} else {
			count = EOF;
			break;
		}
	}

    /* release buffer lock */
    if (log_lock_release)
        (*log_lock_release)();

	return count;
}

void bl31_log_service_register(int (*lock_get)(),
    int (*log_putc)(unsigned char),
    int (*lock_release)())
{
    log_lock_acquire = lock_get;
    log_write = log_putc;
    log_lock_release = lock_release;
}

void bl31_log_service_register2(int (*lock_get)(),
    int (*log_putc)(unsigned char),
    int (*lock_release)())
{
    log_lock_acquire2 = lock_get;
    log_write2 = log_putc;
    log_lock_release2 = lock_release;
}

