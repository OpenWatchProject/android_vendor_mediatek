/*
 * com_intf.h
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
#ifndef C2KTEL_INTF_H
#define C2KTEL_INTF_H



typedef enum VIA_IPC_CMD_TYPE{
	CMD_CLIENT_START = 0,
	CMD_CLIENT_INIT,
	CMD_CLIENT_READY,
	CMD_CLIENT_EXITING,
	CMD_CLIENT_ERROR,
	CMD_DATA_ACK,
	CMD_DATA_NOACK,
	CMD_RESET_CLIENT,
	CMD_KILL_CLIENT,
	CMD_CLIENT_COUNT,
}VIA_IPC_CMD_TYPE;



typedef enum DATA_TYPE{
	STATUS_DATATYPE_CMD = 0,
	STATUS_DATATYPE_DATA,
	STATUS_DATATYPE_COUNT,
}DATA_TYPE;


typedef enum VIA_IPC_MODULE{
	MODULE_TYPE_FLS= 0,	/*flashless*/
	MODULE_TYPE_MUX,		/*mux*/
	MODULE_TYPE_RIL,		/*ril*/
	MODULE_TYPE_CTC,		/*ctclient*/
	MODULE_TYPE_SR ,		/*server*/
	MODULE_TYPE_DG,		/*debugger*/
	MODULE_TYPE_COUNT,
}VIA_IPC_MODULE;

typedef enum PROCESS_STATE{
	PROCESS_STATE_STARTED = 0,
	PROCESS_STATE_INTIALLING,
	PROCESS_STATE_READY,
	PROCESS_STATE_RUNNING,
	PROCESS_STATE_EXITING,
	PROCESS_STATE_EXITED,
	PROCESS_STATE_ERROR,
	PROCESS_STATE_COUNT,
}PROCESS_STATE;



struct cli_info_t{
	char	module_id ; 	/* Module ID	*/
	int 	cli_sock_fd ;	/* Socket ID		*/
};

struct Via_Ipc_Data{
	unsigned char start_flag;
	unsigned char src_module_id;
	unsigned char dst_module_id;
	unsigned char data_type;
	unsigned char  data_len;
	char *data;
	unsigned char end_flag;
};

#define SOCKETNAME "/dev/socket/statusd"
#define  STATUSD_MAX_DATA_LEN 249
#define PROTOCOL_DATA_LEN 6
#define CLIENT_MAX 6
#define RETRY_TIMES 5
#define STATUSD_FRAME_FLAG 0x27// basic mode flag for frame start and end
#ifdef DEBUGGER
#define KEY_DEBUG_LEVEL "level"
#define KEY_LOG2FILE "log2file"
#define KEY_START_FLS "start fls"
#define KEY_STOP_FLS "stop fls"
#define KEY_START_MUX "start mux"
#define KEY_STOP_MUX "stop mux"
#define KEY_START_RIL "start ril"
#define KEY_STOP_RIL "stop ril"
#define KEY_START_CTC "start ct"
#define KEY_STOP_CTC "stop ct"
#endif

typedef int (*statusd_data_callback)(char*,char *,char *,int*,unsigned char*);

int statusd_register_cominf(char module_id,
									statusd_data_callback callback);
int statusd_deregister_cominf(char module_id);
int statusd_send_comdata(char src_modeule_id,
							char dst_module_id,
							unsigned char type,
							char *buffer,
							unsigned char length);

int statusd_c2ssend_cmd(unsigned char src_modeule_id,
						unsigned char dst_module_id,
						char cmd);

int statusd_c2ssend_data(unsigned char src_modeule_id,
									unsigned char dst_module_id,
									char *buf,unsigned int len);

int com_syslogdump(
	const char *prefix,
	const unsigned char *ptr,
	unsigned int length);

const char* name_inquery(char modeule_id);
const char* type_inquery(char type_id);

#endif
