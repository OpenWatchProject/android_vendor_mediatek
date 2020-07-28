/*
 * statusd.h
 *
 * VIA CBP funtion for Linux
 *
 * Copyright (C) 2012 VIA TELECOM Corporation, Inc.
 * Author: jinx@via-telecom.com
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef VIATEL_STATUSD_H
#define VIATEL_STATUSD_H

#include <com_intf.h>
#include <pthread.h>
#include <sys/time.h>

#define  STATUSD_BUFFER_SIZE 2048
#define STATUSD_WAIT_FLASHLESS_TIME_MAX 30//30 S
#define STATUSD_WAIT_MUX_TIME_MAX 10//10 S
#define STATUSD_WAIT_RIL_TIME_MAX 10//10 S
#define STATUSD_WAIT_CTCLIENT_TIME_MAX 10//10 S
#define STATUSD_WAIT_CTCLIENT_EXIT_MAX 1// 1 S
#define statusd_buffer_free(buf) (STATUSD_BUFFER_SIZE - buf->datacount)
#define statusd_buffer_count(buf)  (buf->datacount)
#define statusd_buffer_inc(readp,datacount) do { readp++; datacount--; \
                                       if (readp == buf->endp) readp = buf->data; \
                                     } while (0)


struct process_info_t{
	char process_name[64];
	char module_id;
	pid_t process_pid;
	char  process_state;
	struct timeval start_time;
}process_info_t;

#define PROCESS_MAX CLIENT_MAX

struct socket_info_t{
	struct cli_info_t cli_info[PROCESS_MAX];
	int server_sockfd;
	int maxfd;
};

struct statusd_info_t{
	struct process_info_t process_info[PROCESS_MAX];
	struct socket_info_t socket_info;
	int vpup_state;
};


struct Statusd_Buffer{
	unsigned char data[STATUSD_BUFFER_SIZE];
	unsigned char *readp;
	unsigned char *writep;
	unsigned char *endp;
	unsigned int datacount;
	int newdataready; /*newdataready = 1: new data written to internal buffer. newdataready=0: acknowledged by assembly thread*/
	int input_sleeping; /*input_sleeping = 1 if ser_read_thread (input to buffer) is waiting because buffer is full */
	int flag_found;// set if last character read was flag
	unsigned long received_count;
	unsigned long dropped_count;
};
#define STATUSD_PARSER_MAXARGS 16
#define STATUSD_POWER_CBP_RETRY 10

int statusd_s2csend_cmd(char src_modeule_id,char dst_module_id,char cmd);
int statusd_s2c_comdata(char src_modeule_id,
								char dst_module_id,
								unsigned char type,
								char *buffer,
								unsigned char length);

#endif
