/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>

#include <tz_private/system.h>
#include <tz_private/log.h>

#include <gz_service.h>
#include <gp_param.h>

#include <tz_cross/ta_system.h>
#include <lib/mtee/mtee_srv.h>


#define LOG_TAG "int_ipc"

#define TRUSTY_CONNECT_TIMEOUT 10000 // 10 sec

#ifndef __NEBULA_HEE__
#define SYS_SRV "com.mediatek.geniezone.srv.sys"
#else
#define SYS_SRV "nebula.com.mediatek.geniezone.srv.sys"
#endif

/**************** tipc helper functions ****************/
/*
 *  Local wrapper on top of async connect that provides
 *  synchronos connect with timeout.
 */
static int sync_connect(const char *path, uint timeout)
{
	int rc;
	uevent_t evt;
	handle_t chan;

	rc = connect(path, IPC_CONNECT_ASYNC | IPC_CONNECT_WAIT_FOR_PORT);
	if (rc >= 0) {
		chan = (handle_t) rc;
		rc = wait(chan, &evt, timeout);
		if (rc == 0) {
			rc = ERR_BAD_STATE;
			if (evt.handle == chan) {
				if (evt.event & IPC_HANDLE_POLL_READY)
					return chan;

				if (evt.event & IPC_HANDLE_POLL_HUP)
					rc = ERR_CHANNEL_CLOSED;
			}
		}
		close(chan);
	}
	return rc;
}

static int try_close_channel(void *data)
{
	handle_t chan = (handle_t)data;
	int rc = NO_ERROR;

	DBG_LOG("%s:\n", __func__);
	rc = close(chan);
	if (rc!=NO_ERROR) {
		ERR_LOG("try close channel failed\n");
	}

	return rc;
}

static int wait_internal_msg(handle_t handle, uint8_t *ipc_buf, int buf_size, ipc_msg_t *msg)
{
	int rc = NO_ERROR;
	uevent_t evt;
	ipc_msg_info_t msg_info;

	// should be blocking here
	rc = wait(handle, &evt, -1);
	if (rc == ERR_NO_MSG) {
		return rc;
	}

	// after blocking, got msg
	rc = get_msg(evt.handle, &msg_info);
	if (rc < 0) {
		ERR_LOG("get msg failed. ret = %d\n", rc);
		return rc;
	}

	msg->iov->base = ipc_buf;
	msg->iov->len = buf_size;
	rc = read_msg(evt.handle, msg_info.id, 0, msg);
	if (rc < 0) {
		ERR_LOG("read msg from chan failed, ret = %d\n", rc);
		return rc;
	}

	put_msg(evt.handle, msg_info.id);
	return rc;
}

#define GZ_MSG_HEADER_LEN (sizeof(GZ_servicecall_cmd_param_t) - GZ_MSG_DATA_MAX_LEN)
static int send_internal_msg(handle_t chan, GZ_servicecall_cmd_param_t *param, ipc_msg_t *msg)
{
	int rc;

	if (!chan || !param || !msg) {
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}
	msg->num_iov = 1;
	msg->handles = NULL;
	msg->num_handles = 0;
	msg->iov->base = param;
	msg->iov->len = GZ_MSG_HEADER_LEN + param->payload_size;
	rc = send_msg(chan, msg);
	if (rc < 0) {
		ERR_LOG("%s: gz client cmd failed\n", __func__);
		return TZ_RESULT_ERROR_COMMUNICATION;
	}

	return TZ_RESULT_SUCCESS;
}

static void make_64_params(MTEEC_PARAM *param, uint32_t paramTypes)
{
	int i;
	uint32_t type;
	uint64_t size;
	uint64_t addr;

	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);
		if (type == TZPT_MEM_INPUT || type == TZPT_MEM_OUTPUT || type == TZPT_MEM_INOUT) {
			size = param[i].mem.size;
			addr = param[i].mem.buffer;

			param[i].mem64.buffer = addr;
			param[i].mem64.size= size;
		}
	}
}

static void recover_64_params(MTEEC_PARAM *param, MTEEC_PARAM *origin, uint32_t paramTypes)
{
	int i;
	uint32_t type;

	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);
		if (type == TZPT_MEM_INPUT || type == TZPT_MEM_OUTPUT || type == TZPT_MEM_INOUT) {
			param[i].mem.buffer = origin[i].mem.buffer;
			param[i].mem.size= origin[i].mem.size;
		}
	}
}
/**************** END tipc helper functions ****************/

static int _mtee_sys_server_chan = -1;

static void __attribute__((constructor))
utee_system_init (void)
{
	_mtee_sys_server_chan = -1;
}

static void __attribute__((destructor))
utee_system_exit (void)
{
	if(_mtee_sys_server_chan != -1) {
		if(NO_ERROR != try_close_channel(_mtee_sys_server_chan)) {
			ERR_LOG("exit close channel fail\n");
		}
		_mtee_sys_server_chan = -1;
	}
}

static TZ_RESULT _TeeServiceCall_body (handle_t *chan, MTEE_SESSION_HANDLE session, uint32_t cmd,
							  uint32_t paramTypes, MTEEC_PARAM origin[4])
{
	iovec_t iov;
	GZ_servicecall_cmd_param_t in_param, out_param;
	ipc_msg_t msg;
	int rc, copied = 0;
	
	/* make input */
	in_param.handle = session;
	in_param.ree_service = 0;
	in_param.paramTypes = paramTypes;
	in_param.command = cmd;
	memcpy(in_param.param, origin, sizeof(MTEEC_PARAM) * 4);

	copied = copy_mem_to_buffer(in_param.paramTypes, in_param.param, (void *)in_param.data);
	if (copied < 0) {
		ERR_LOG("invalid input gp params in service call\n");
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}
	in_param.payload_size = copied;

	/* use mem64 instead to mimic service call from Linux */
	make_64_params(in_param.param, in_param.paramTypes);

	msg.iov = &iov;
	send_internal_msg(chan, &in_param, &msg);

	rc = wait_internal_msg(chan, &out_param, sizeof(GZ_servicecall_cmd_param_t), &msg);

	if (rc < 0) {
		ERR_LOG("wait ret failed(%d)\n", rc);
		return TZ_RESULT_ERROR_COMMUNICATION;
	}

	/* recover mem64 params */
	recover_64_params(out_param.param, origin, paramTypes);

	/* copy mem content to client buffer */
	copied = copy_out_mem_from_buffer(paramTypes, origin, (void *)out_param.data);
	if (copied < 0) {
		ERR_LOG("invalid output gp params\n");
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}
	memcpy(origin, out_param.param, sizeof(MTEEC_PARAM) * 4);

	return TZ_RESULT_SUCCESS;
}


TZ_RESULT UTEE_CreateSession(const char *ta_uuid, MTEE_SESSION_HANDLE *pHandle)
{
	int rc;
	handle_t chan;
	TZ_RESULT ret = TZ_RESULT_SUCCESS;
	MTEE_SESSION_HANDLE session;
	MTEEC_PARAM params[4];
	uint32_t paramTypes;

	if (!pHandle || !ta_uuid)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	DBG_LOG("%s: start\n", __func__);

	// open sys server
	if (_mtee_sys_server_chan == -1) {
		rc = sync_connect(SYS_SRV, TRUSTY_CONNECT_TIMEOUT);
		if (rc < 0) {
			ret = rc;
			ERR_LOG("connect port failed\n");
			goto end_create_session;
		}
		_mtee_sys_server_chan = rc;
	}

	/* connect to server through trusty syscall */
	rc = sync_connect(ta_uuid, TRUSTY_CONNECT_TIMEOUT);
	if (rc < 0) {
		ret = rc;
		ERR_LOG("connect port failed\n");
		goto end_create_session;
	}
	chan = (handle_t)rc;

	// create session
	paramTypes = TZ_ParamTypes2(TZPT_MEM_INPUT, TZPT_VALUE_OUTPUT);
	params[0].mem.buffer = ta_uuid; // FIXME!!!!!
	params[0].mem.size = MTEE_SESSION_NAME_LEN;//(uint32_t)(strlen(ta_uuid)+1); //FIXME!!!
	ret = _TeeServiceCall_body(_mtee_sys_server_chan, 0, TZCMD_SYS_SESSION_CREATE, paramTypes, params);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("create session failed\n");
		goto err_close_chan;
	}
	session = params[1].value.a;

	/* bind the trusty channel handle to session */
	ret = MTEE_SetSessionUserData(session, chan);

	*pHandle = session;
	return ret;

err_close_chan:
	try_close_channel(chan);
end_create_session:	
	return ret;
}

TZ_RESULT UTEE_CloseSession(MTEE_SESSION_HANDLE session)
{
	TZ_RESULT ret = TZ_RESULT_SUCCESS;
	MTEEC_PARAM params[4];
	uint32_t paramTypes;
	handle_t chan;
	int rc;
	
	if (_mtee_sys_server_chan == -1) {
		ERR_LOG("close session failed\n");
		ret = TZ_RESULT_ERROR_GENERIC;
		goto end_close_session;
	}

	/* get trusty handle from session */
	ret = MTEE_GetSessionUserData(session, &chan);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("failed to get channel from session\n");
		goto end_close_session;
	}

	rc = try_close_channel(chan); 
	if (rc != NO_ERROR) {
		ERR_LOG("failed close channel\n");
		goto end_close_session;
	}

	// close session
	paramTypes = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	params[0].value.a = session;
	ret = _TeeServiceCall_body(_mtee_sys_server_chan, 0, TZCMD_SYS_SESSION_CLOSE, paramTypes, params);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("create session failed\n");
		goto end_close_session;
	}

	return TZ_RESULT_SUCCESS;

end_close_session:
	
	return ret;
}

TZ_RESULT UTEE_TeeServiceCall(MTEE_SESSION_HANDLE session, uint32_t cmd,
							  uint32_t paramTypes, MTEEC_PARAM origin[4]) {
	int ret = TZ_RESULT_SUCCESS;	
	handle_t chan;		

	/* get trusty handle from session */
	ret = MTEE_GetSessionUserData(session, &chan);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("failed to get channel from session\n");
		goto end_tee_service_call;
	}

	ret = _TeeServiceCall_body(chan, session, cmd, paramTypes, origin);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("tee service call failed\n");
		goto end_tee_service_call;
	}

	return TZ_RESULT_SUCCESS;

end_tee_service_call:

	return ret;
}

struct UTEE_SHM_RUNLENGTH_ENTRY {
	uint32_t high;	/* (uint64_t) start PA address = high | low */
	uint32_t low;
	uint32_t size;
};
#define MTEE_SHAREDMEM_HANDLE uint32_t
#define MTEE_SECUREMEM_HANDLE uint32_t

TZ_RESULT UTEE_AppendSecureMultichunkmem(MTEE_SESSION_HANDLE session,
					MTEE_SHAREDMEM_HANDLE *cm_handle,
					uint64_t pa, uint32_t size)
{
	TZ_RESULT ret;
	int numOfPA;
	struct UTEE_SHM_RUNLENGTH_ENTRY *tmpAry = NULL;
	MTEEC_PARAM p[4];
	uint32_t paramTypes;
	int round;

	if (cm_handle == NULL)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	/*init*/
	*cm_handle = 0;

	numOfPA =  size / PAGE_SIZE;
	if ((size % PAGE_SIZE) != 0)
		numOfPA++;
	//DBG_LOG("[%s] numOfPA= 0x%x\n", __func__, numOfPA);

	tmpAry = malloc(1 * sizeof(struct UTEE_SHM_RUNLENGTH_ENTRY));
	tmpAry[0].high = (uint32_t) ((uint64_t) pa >> 32);
	tmpAry[0].low = (uint32_t) ((uint64_t) pa & (0x00000000ffffffff));
	tmpAry[0].size = numOfPA;

	for (int i = 0; i < 3; i++) {
		p[i].mem.buffer = NULL;
		p[i].mem.size = 0;
	}

	p[0].mem.buffer = tmpAry;
	p[0].mem.size = sizeof(struct UTEE_SHM_RUNLENGTH_ENTRY);

	DBG_LOG("[%s]====> tmpAry[0] high= 0x%x, low= 0x%x, size= 0x%x\n", __func__,
				(uint32_t)tmpAry[0].high, (uint32_t)tmpAry[0].low, (uint32_t)tmpAry[0].size);

	//DBG_LOG("[%s]====> p[0].mem.buffer= 0x%x, p[0].mem.size= 0x%x\n", __func__,
	//			p[0].mem.buffer, p[0].mem.size);

	/*send PA array*/
	p[3].value.a = round;
	p[3].value.b = numOfPA;
	paramTypes = TZ_ParamTypes4(TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_VALUE_INOUT);
	//paramTypes = TZ_ParamTypes4(TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_VALUE_INOUT);

	ret = UTEE_TeeServiceCall(session, TZCMD_MEM_APPEND_MULTI_CHUNKMEM, paramTypes, p);

	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("[%s(%d)] Error: 0x%x\n", __func__, __LINE__, ret);
	}

	/* send ending command */
	for (int i = 0; i < 3; i++) {
		p[i].mem.buffer = NULL;
		p[i].mem.size = 0;
	}
	p[3].value.a = -99;
	p[3].value.b = numOfPA;

	//paramTypes = TZ_ParamTypes4(TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_VALUE_INOUT);
	paramTypes = TZ_ParamTypes4(TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_VALUE_INOUT);
	ret = UTEE_TeeServiceCall(session, TZCMD_MEM_APPEND_MULTI_CHUNKMEM, paramTypes, p);

	free(tmpAry);

	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("[%s(%d)] Error: 0x%x\n", __func__, __LINE__, ret);
	}

	*cm_handle = p[3].value.a;
	if (*cm_handle == 0) {
		ERR_LOG("[%s(%d)] Error cm_handle: 0x%x\n", __func__, __LINE__, *cm_handle);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return ret;
}


TZ_RESULT UTEE_AllocSecureMultichunkmem(
				MTEE_SESSION_HANDLE session,
				MTEE_SHAREDMEM_HANDLE chm_handle,
				MTEE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size)
{
	//union MTEEC_PARAM p[4];
	MTEEC_PARAM p[4];
	TZ_RESULT ret;

	if ((chm_handle == 0) || (size == 0)) {
		ERR_LOG("[%s]: invalid parameters\n", __func__);
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	p[0].value.a = alignment;
	p[1].value.a = size;
	p[1].value.b = chm_handle;
	ret = UTEE_TeeServiceCall(session, TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC,
					TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT),
					p);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("%s Error: 0x%x\n", __func__, ret);
		return ret;
	}

	*mem_handle = (MTEE_SECUREMEM_HANDLE) p[2].value.a;
	DBG_LOG("Alloc ret mem_handle=0x%x\n", *mem_handle);

	if (*mem_handle == 0) {
		ERR_LOG("[%s]Alloc chmem=NULL: mem_handle=0x%x\n", __func__, *mem_handle);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UTEE_UnreferenceChunkmem(MTEE_SESSION_HANDLE session,
					MTEE_SECUREMEM_HANDLE mem_handle)
{
	TZ_RESULT ret;
	uint32_t count = 0;
	//union MTEEC_PARAM p[4];
	MTEEC_PARAM p[4];

	p[0].value.a = mem_handle;

	ret = UTEE_TeeServiceCall(session, TZCMD_MEM_SECUREMULTICHUNKMEM_UNREF,
					TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT), p);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("UTEE_UnreferenceChunkmem Error: 0x%x\n", ret);
		return ret;
	}

	count = p[1].value.a;
	DBG_LOG("UTEE_UnreferenceChunkmem ret count=0x%x\n", count);

	return ret;
}

TZ_RESULT UTEE_ReleaseSecureMultichunkmem(MTEE_SESSION_HANDLE session,
					MTEE_SHAREDMEM_HANDLE chm_handle)
{
	//union MTEEC_PARAM p[4];
	MTEEC_PARAM p[4];
	TZ_RESULT ret;

	p[0].value.a = (uint32_t) chm_handle;
	ret = UTEE_TeeServiceCall(session, TZCMD_MEM_RELEASE_CHUNKMEM,
				  TZ_ParamTypes1(TZPT_VALUE_INPUT), p);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("[%s] Error: 0x%x\n", __func__, ret);
	}
	return ret;
}

TZ_RESULT UTEE_AppendSecureChunkmemandUse(MTEE_SESSION_HANDLE session,
					MTEE_SHAREDMEM_HANDLE *mem_handle,
					uint64_t pa, uint32_t size, uint32_t alignment)
{
	uint32_t chm_handle;
	int ret;

	/*append chmem*/
	ret = UTEE_AppendSecureMultichunkmem(session, &chm_handle, pa, size);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("UTEE_AppendSecureMultichunkmem() Fail. ret=0x%x\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
	}
	DBG_LOG("[OK]Append chmem. chm_handle=0x%x\n",(uint32_t) chm_handle);

	/*alloc*/
	ret = UTEE_AllocSecureMultichunkmem(session, chm_handle, mem_handle, alignment, size);
	if ((ret != TZ_RESULT_SUCCESS) || (*mem_handle == 0)) {
		ERR_LOG("[%s] UTEE_AllocSecureMultichunkmem() Fail. ret=0x%x\n", __func__, ret);
	}
	DBG_LOG("[OK]Alloc chmem [chm_handle=0x%x, mem_handle=0x%x (alignment=%d, size=%d)]\n",
				chm_handle, *mem_handle, alignment, size);

	return TZ_RESULT_SUCCESS;
}
