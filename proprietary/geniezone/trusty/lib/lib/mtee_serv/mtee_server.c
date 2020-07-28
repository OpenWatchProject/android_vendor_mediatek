/*
 * Copyright (C) 2014-2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <err.h>
#include <list.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#include <tz_private/system.h>
#include <tz_private/log.h>
//#include <lib/trusty/uuid.h>
#include <trusty_uuid.h>
#define LOG_TAG "mtee-server"
const struct uuid zero_uuid = ZERO_UUID;

//#define GZ_MTEE_SERVER_DEBUG

//#if defined (GZ_MTEE_SERVER_DEBUG)
//#define TLOGI(fmt, ...) \
//	fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)
//#else
//#define TLOGI(fmt, ...)
//#endif

#include <gp_param.h>
#include <gz_service.h>
#include <mtee_server.h>

#define DEBUG_PARAM 1

void MTEE_handle_port(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);
void MTEE_handle_chan(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);


static struct tipc_srv_state *get_srv_state(const uevent_t *ev)
{
	return containerof(ev->cookie, struct tipc_srv_state, handler);
}

static void _destroy_service(struct tipc_srv_state *state)
{
	if (!state) {
		WARN_LOG("non-null state expected\n");
		return;
	}

	/* free state if any */
	if (state->priv) {
		free(state->priv);
		state->priv = NULL;
	}

	/* close port */
	if (state->port != INVALID_IPC_HANDLE) {
		int rc = close(state->port);
		if (rc != NO_ERROR) {
			ERR_LOG("Failed (%d) to close port %d\n",
				   rc, state->port);
		}
		state->port = INVALID_IPC_HANDLE;
	}

	/* reset handler */
	state->service = NULL;
	state->handler.proc = NULL;
	state->handler.priv = NULL;
}


/*
 *  Create service
 */
static int _create_service(const struct tipc_srv *srv,
						   struct tipc_srv_state *state)
{
	if (!srv || !state) {
		ERR_LOG("null service specified: %p: %p\n");
		return ERR_INVALID_ARGS;
	}

	/* create port */
	int rc = port_create(srv->name, srv->msg_num, srv->msg_size,
				 srv->port_flags);
	if (rc < 0) {
		ERR_LOG("Failed (%d) to create port\n", rc);
		return rc;
	}

	/* setup port state  */
	state->port = (handle_t)rc;
	state->handler.proc = MTEE_handle_port;
	state->handler.priv = state;
	state->service = srv;
	state->priv = NULL;

	if (srv->port_state_size) {
		/* allocate port state */
		state->priv = calloc(1, srv->port_state_size);
		if (!state->priv) {
			rc = ERR_NO_MEMORY;
			goto err_calloc;
		}
	}

	/* attach handler to port handle */
	rc = set_cookie(state->port, &state->handler);
	if (rc < 0) {
		ERR_LOG("Failed (%d) to set cookie on port %d\n",
			  rc, state->port);
		goto err_set_cookie;
	}

	return NO_ERROR;

err_calloc:
err_set_cookie:
	_destroy_service(state);
	return rc;
}


/*
 *  Restart specified service
 */
static int restart_service(struct tipc_srv_state *state)
{
	if (!state) {
		ERR_LOG("non-null state expected\n");
		return ERR_INVALID_ARGS;
	}

	const struct tipc_srv *srv = state->service;
	_destroy_service(state);
	return _create_service(srv, state);
}

/*
 *  Kill all servoces
 */
void kill_services(const tipc_srv_t *services_p, int count, tipc_srv_state_t *srv_states_p)
{
	DBG_LOG ("Terminating unittest services\n");

	/* close any opened ports */
	for (uint i = 0; i < count; i++) {
		_destroy_service(&srv_states_p[i]);
	}
}

/*
 *  Initialize all services
 */
int init_services(const tipc_srv_t *services_p, int count, tipc_srv_state_t *srv_states_p)
{
	tipc_srv_t *srv_p;

	DBG_LOG ("Init services!!!\n");

	for (uint i = 0; i < count; i++) {
		int rc = _create_service(&services_p[i], &srv_states_p[i]);
		if (rc < 0) {
			ERR_LOG("Failed (%d) to create service %s\n",
				  rc, services_p[i].name);
			return rc;
		}

		/* do init handler */
		if (services_p && services_p[i].init_handler){
			services_p[i].init_handler(&services_p[i]);
		}
	}

	return 0;
}

/*
 *  Handle common port errors
 */
static bool handle_port_errors(const uevent_t *ev)
{
	if ((ev->event & IPC_HANDLE_POLL_ERROR) ||
		(ev->event & IPC_HANDLE_POLL_HUP) ||
		(ev->event & IPC_HANDLE_POLL_MSG) ||
		(ev->event & IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
		/* should never happen with port handles */
		ERR_LOG("error event (0x%x) for port (%d)\n",
			   ev->event, ev->handle);

		/* recreate service */
		restart_service(get_srv_state(ev));
		return true;
	}

	return false;
}




/*
 *  Dispatch event
 */
void dispatch_event(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size)
{
	assert(ev);

	if (ev->event == IPC_HANDLE_POLL_NONE) {
		/* not really an event, do nothing */
		DBG_LOG("got an empty event\n");
		return;
	}

	if (ev->handle == INVALID_IPC_HANDLE) {
		/* not a valid handle  */
		ERR_LOG("got an event (0x%x) with invalid handle (%d)",
			  ev->event, ev->handle);
		return;
	}

	/* check if we have handler */
	struct tipc_event_handler *handler = ev->cookie;
	if (handler && handler->proc) {
		/* invoke it */
		handler->proc(ev, msg_buf_p, msg_buf_size);
		return;
	}

	/* no handler? close it */
	WARN_LOG("no handler for event (0x%x) with handle %d\n", ev->event, ev->handle);
	close(ev->handle);

	return;
}

int uuid_compare(uuid_t a, uuid_t b)
{
#if 0
	DBG_LOG("a is server, uuid of a is: %d %d %d %d %d %d %d %d %d %d %d \n",a.time_low, a.time_mid, a.time_hi_and_version,
		a.clock_seq_and_node[0], a.clock_seq_and_node[1], a.clock_seq_and_node[2], a.clock_seq_and_node[3],
		a.clock_seq_and_node[4], a.clock_seq_and_node[5], a.clock_seq_and_node[6], a.clock_seq_and_node[7]);
	DBG_LOG("b is connecter, uuid of b is: %d %d %d %d %d %d %d %d %d %d %d \n",b.time_low, b.time_mid, b.time_hi_and_version,
		b.clock_seq_and_node[0], b.clock_seq_and_node[1], b.clock_seq_and_node[2], b.clock_seq_and_node[3],
		b.clock_seq_and_node[4], b.clock_seq_and_node[5], b.clock_seq_and_node[6], b.clock_seq_and_node[7]);
#endif
	return (a.time_mid == b.time_mid) && (a.time_low== b.time_low) && (a.time_hi_and_version == b.time_hi_and_version)
			&&(a.clock_seq_and_node[0]== b.clock_seq_and_node[0])
			&&(a.clock_seq_and_node[1]== b.clock_seq_and_node[1])
			&&(a.clock_seq_and_node[2]== b.clock_seq_and_node[2])
			&&(a.clock_seq_and_node[3]== b.clock_seq_and_node[3])
			&&(a.clock_seq_and_node[4]== b.clock_seq_and_node[4])
			&&(a.clock_seq_and_node[5]== b.clock_seq_and_node[5])
			&&(a.clock_seq_and_node[6]== b.clock_seq_and_node[6])
			&&(a.clock_seq_and_node[7]== b.clock_seq_and_node[7]);

}
/*
 *  port event handler
 */
void MTEE_handle_port(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size)
{
	uuid_t peer_uuid;
	struct systa_chan_state *chan_st;
	const struct tipc_srv *srv = get_srv_state(ev)->service;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		handle_t chan;

	/* incomming connection: accept it */
	int rc = accept(ev->handle, &peer_uuid);
	if (rc < 0) {
		ERR_LOG("failed (%d) to accept on port %d\n",rc, ev->handle);
		return;
	}
	
	/*check the peer uuid = srv->client_uuid or zero_uuid? */
	if (uuid_compare(srv->peer_uuid,peer_uuid) || uuid_compare(srv->peer_uuid,zero_uuid)) 
		DBG_LOG("server accept! connect it!\n");
	else {
		ERR_LOG("[%s]sorry , your identity is rejected!\n",__func__);
		return ERR_NOT_ALLOWED;
	}
		chan = (handle_t) rc;

		chan_st = calloc(1, sizeof(struct systa_chan_state) +
							sizeof(ipc_msg_info_t) * srv->msg_num);
		if (!chan_st) {
			ERR_LOG("failed (%d) to callocate state for chan %d\n",
				   rc, chan);
			close(chan);
			return;
		}

		/* init state */
		chan_st->msg_max_num  = srv->msg_num;
		chan_st->handler.proc = MTEE_handle_chan;
		chan_st->handler.priv = chan_st;
		chan_st->handler.service_handler = srv->service_handler;
		chan_st->srv = srv;

		/* do port connected handler */
		if (srv && srv->port_connected_handler) {
			rc = srv->port_connected_handler(chan, srv);
			if (rc == ERR_NOT_ENOUGH_BUFFER){
				return -1;
			}
		}

		/* attach it to handle */
		rc = set_cookie(chan, &chan_st->handler);
		if (rc) {
			ERR_LOG("failed (%d) to set_cookie on chan %d\n",
				   rc, chan);
			free(chan_st);
			close(chan);
			return;
		}
	}
}

#define GZ_MSG_HEADER_LEN (sizeof(GZ_servicecall_cmd_param_t) - GZ_MSG_DATA_MAX_LEN)
uint32_t gz_handle;

static int MTEE_handle_msg(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size)
{
	int rc, size;
	uint32_t cmd;
	iovec_t iov;
	ipc_msg_t msg;
	systa_chan_state_t *st = containerof(ev->cookie, systa_chan_state_t, handler);
	gz_sys_msg_t *p_smsg; //geniezone system service message
	MTEEC_PARAM param, *param_p;
	GZ_servicecall_cmd_param_t *gz_param_p;
	GZ_servicecall_cmd_param_t gz_param;
	service_handler_t *service_handler_p;

	/* get all messages */
	while (st->msg_cnt != st->msg_max_num) {
		rc = get_msg(ev->handle, &st->msg_queue[st->msg_next_w]);
		if (rc == ERR_NO_MSG)
			break; /* no new messages */

		if (rc != NO_ERROR) {
			ERR_LOG("failed (%d) to get_msg for chan (%d)\n",
				  rc, ev->handle);
			return rc;
		}

		st->msg_cnt++;
		st->msg_next_w++;
		if (st->msg_next_w == st->msg_max_num)
			st->msg_next_w = 0;
	}

	/* handle all messages in queue */
	while (st->msg_cnt) {
		/* init message structure */
		iov.base = msg_buf_p;
		iov.len  = msg_buf_size;
		msg.num_iov = 1;
		msg.iov	 = &iov;
		msg.num_handles = 0;
		msg.handles  = NULL;

		/*-- read msg content --*/
		rc = read_msg(ev->handle, st->msg_queue[st->msg_next_r].id, 0, &msg);
		if (rc < 0) {
			ERR_LOG("failed (%d) to read_msg for chan (%d)\n",
				  rc, ev->handle);
			return rc;
		}

		/* retire original message */
		rc = put_msg(ev->handle, st->msg_queue[st->msg_next_r].id);
		if (rc != NO_ERROR) {
			ERR_LOG("failed (%d) to put_msg for chan (%d)\n",
				  rc, ev->handle);
			return rc;
		}

		/*-- process open session command --*/


	struct tipc_event_handler *handler = ev->cookie;
	if (handler && handler->service_handler) {

		gz_param_p = (GZ_servicecall_cmd_param_t*)(msg.iov->base);

		// copy header to buffer first
		memcpy(&gz_param, msg.iov->base, GZ_MSG_HEADER_LEN);
		size = gz_param.payload_size;
		//DBG_LOG("rcv msg len= %d, data size = %d\n", msg.iov->len, size);

		if (size < 0 || size > GZ_MSG_DATA_MAX_LEN) {
			ERR_LOG("invalid GZ msg payload size(%d)\n",
					gz_param.payload_size);
			continue;
		}

		// then copy the data part according to data len
		memcpy(&gz_param.data[0], &gz_param_p->data[0], gz_param.payload_size);

		rewrite_inout_param_mem_addr(
				gz_param.paramTypes,
				gz_param.param,
				(uint64_t)gz_param_p->data,
				(uint64_t)gz_param.data);

		// workaround solution for REE service call....
		gz_handle = ev->handle;

		rc = handler->service_handler(gz_param.handle, gz_param.command, gz_param.paramTypes, gz_param.param);
		if (rc == ERR_NOT_ENOUGH_BUFFER)
			return -1;
	}

		// copy memory parameters to buffer
		rc = copy_inout_mem_to_buffer(gz_param.paramTypes,
				gz_param.param, (void *)gz_param.data, 1);
		if (rc < 0) {
			ERR_LOG("failed to copy all mem data to param buffer\n");
			continue;
			//return rc;
		}
		gz_param.payload_size = rc;

		param_p = gz_param.param;

		msg.iov->base = &gz_param;
		msg.iov->len= GZ_MSG_HEADER_LEN + gz_param.payload_size;
		gz_param.ree_service = 0;
		rc = send_msg(ev->handle, &msg);
#if DEBUG_PARAM
		DBG_LOG("TeeServiceCall --> session: 0x%x, command: 0x%x, paramTypes: 0x%x\n", gz_param.handle, gz_param.command, gz_param.paramTypes);
		DBG_LOG("param [0]: 0x%x 0x%x [1]: 0x%x 0x%x [2]: 0x%x 0x%x [3]: 0x%x 0x%x\n",
					param_p[0].value.a, param_p[0].value.b,
					param_p[1].value.a, param_p[1].value.b,
					param_p[2].value.a, param_p[2].value.b,
					param_p[3].value.a, param_p[3].value.b );
#endif

		if (rc < 0) {
			ERR_LOG("failed (%d) to send_msg for chan (%d)\n",
				  rc, ev->handle);
			continue;
			//return rc;
		}

		/* advance queue */
		st->msg_cnt--;
		st->msg_next_r++;
		if (st->msg_next_r == st->msg_max_num)
			st->msg_next_r = 0;
	}
	return NO_ERROR;
}

/*
 *  channel handler
 */
void MTEE_handle_chan(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size)
{
	tipc_srv_t *srv;
	const systa_chan_state_t *chan_st;
	chan_st = (systa_chan_state_t*)ev->cookie;
	if(chan_st) srv = chan_st->srv;

	if (ev->event & IPC_HANDLE_POLL_ERROR) {
		/* close it as it is in an error state */
		ERR_LOG("error event (0x%x) for chan (%d)\n",
			  ev->event, ev->handle);
		goto close_it;
	}

	if (ev->event & (IPC_HANDLE_POLL_MSG |
				 IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
				if (MTEE_handle_msg(ev, msg_buf_p,msg_buf_size ) != 0) {
			ERR_LOG("error event (0x%x) for chan (%d)\n",
				  ev->event, ev->handle);
			goto close_it;
		}
	}

	if (ev->event & IPC_HANDLE_POLL_HUP) {
		//DBG_LOG("%s: handle poll hung up\n", __func__);
		goto close_it;
	}

	return;

close_it:
	//DBG_LOG("%s: close chan %u\n", __func__, ev->handle);
	/*  call disconnect callbacks */
	if (srv && srv->disc_handler) {
		srv->disc_handler(ev->handle, srv);
    } else {
        if (!srv) {
            ERR_LOG("srv is NULL\n");
        } else {
            ERR_LOG("srv disconnect handler is NULL\n");
        }
    }
	free(ev->cookie);
	close(ev->handle);
}


#define REE_SERVICECALL_MSG_BUFFER_SIZE 1024;

int MTEE_ReeServiceCall (int session, int command, uint32_t paramTypes, MTEEC_PARAM param[4])
{
	GZ_servicecall_cmd_param_t ree_gz_param;
	iovec_t iov;
	ipc_msg_t msg;
	char *msg_buf_p;
	int msg_buf_size;
	uevent_t event;
	uint32_t id;
	int rc = 0;
	int handle;
	ipc_msg_info_t msg_info;

	handle = gz_handle; // workaround solutoin, fix me!!!!

	msg_buf_size = REE_SERVICECALL_MSG_BUFFER_SIZE;
	msg_buf_p = malloc (msg_buf_size); // fix me, must use MTEE_XXXXX

	/* init message structure */
	iov.base = msg_buf_p;
	iov.len  = msg_buf_size;
	msg.num_iov = 1;
	msg.iov	 = &iov;
	msg.num_handles = 0;
	msg.handles  = NULL;

	// send REE service command to REE
	ree_gz_param.ree_service = 1;
	memcpy (ree_gz_param.param, param, sizeof (MTEEC_PARAM)*4); // hard code, FIXME!!!
	ree_gz_param.paramTypes = paramTypes;
	ree_gz_param.command = command;
	ree_gz_param.handle = session;

	// copy mem buffer
	rc = copy_mem_to_buffer(ree_gz_param.paramTypes, ree_gz_param.param, (void *)ree_gz_param.data);
	if (rc < 0) {
		ERR_LOG("failed to copy all mem data to param buffer\n");
		return rc;
	}
	ree_gz_param.payload_size = rc;

	msg.iov->base = &ree_gz_param;
	msg.iov->len= GZ_MSG_HEADER_LEN + ree_gz_param.payload_size;
	rc = send_msg(handle, &msg);

	// wait REE ack
	wait(handle, &event, -1);

	// get REE ack
	rc = get_msg(event.handle, &msg_info);
	if (rc == ERR_NO_MSG){
		ERR_LOG("failed (%d) to get_msg for chan (%d)\n",
			rc, event.handle);
		return rc;
	}

	msg.iov->base = &ree_gz_param;
	msg.iov->len= sizeof(ree_gz_param);
	rc = read_msg(event.handle, msg_info.id, 0, &msg);
	if (rc < 0) {
		ERR_LOG("failed (%d) to read_msg for chan (%d)\n",
			rc, event.handle);
		return rc;
	}

	// FIXME!! this function will overwrite param.mem.buffer address
	rewrite_param_mem_addr(ree_gz_param.paramTypes, ree_gz_param.param, (uint32_t)ree_gz_param.data);

	// copy mem buffer
	rc = copy_out_mem_from_buffer(ree_gz_param.paramTypes, ree_gz_param.param, (void *)ree_gz_param.data);
	if(rc < 0) {
		ERR_LOG("failed to copy all mem data from param buffer\n");
		return rc;
	}

	memcpy (param, ree_gz_param.param, sizeof (MTEEC_PARAM)*4); // hard code, fix me!!!

#ifdef GZ_MULTI_THREAD_DEBUG
	DBG_LOG ("===> %s, ack session %d, command %x\n", __FUNCTION__, ree_gz_param.handle, ree_gz_param.command);
	DBG_LOG ("===> %s, param values %x, %x, %x, %x\n", __FUNCTION__,
		ree_gz_param.param[0].value.a, ree_gz_param.param[1].value.a, ree_gz_param.param[2].value.a, ree_gz_param.param[3].value.a);
#endif

	put_msg (event.handle, msg_info.id);

	free (msg_buf_p); // fix me, must use MTEE_XXXXX

	return 0;

}

/***************************************************************/
/*   sync with original MTEE   */
/***************************************************************/

TZ_RESULT MTEE_PMGet(const char *pmdev_name, int *res)
{
#if 0
    int ret;
    void *ptr = MTEE_GetReeParamAddress();

    if (pmdev_name == NULL)
        return TZ_RESULT_ERROR_BAD_PARAMETERS;

    strncpy(ptr, pmdev_name, REE_SERVICE_BUFFER_SIZE);
    ret = MTEE_ReeServiceCall(REE_SERV_PMGET);
    if (res != NULL)
        *res = *(int *)ptr;

    return ret;
#endif

	//MTEE_LOG(MTEE_LOG_LVL_WARN, "This method [%s] is not supported.\n", __func__);
	print_mtee_orig_msg;
	return; //not support
}

TZ_RESULT MTEE_PMPut(const char *pmdev_name, int *res)
{
#if 0
    int ret;
    void *ptr = MTEE_GetReeParamAddress();

    if (pmdev_name == NULL) {
        return TZ_RESULT_ERROR_BAD_PARAMETERS;
    }

    strncpy(ptr, pmdev_name, REE_SERVICE_BUFFER_SIZE);
    ret = MTEE_ReeServiceCall(REE_SERV_PMPUT);
    if (res != NULL)
        *res = *(int *)ptr;

    return ret;
#endif

	//MTEE_LOG(MTEE_LOG_LVL_WARN, "This method [%s] is not supported.\n", __func__);
	print_mtee_orig_msg;
	return; //not support
}

