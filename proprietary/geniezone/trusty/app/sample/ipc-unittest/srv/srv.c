/*
 * Copyright (C) 2014-2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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

#define LOG_TAG "ipc-unittest-srv"

#include <app/ipc_unittest/common.h>
#include <app/ipc_unittest/uuids.h>

typedef void (*event_handler_proc_t) (const uevent_t *ev);

typedef struct tipc_event_handler {
	event_handler_proc_t proc;
	void *priv;
} tipc_event_handler_t;

typedef struct tipc_srv {
	const char *name;
	uint   msg_num;
	size_t msg_size;
	uint   port_flags;
	size_t port_state_size;
	size_t chan_state_size;
	event_handler_proc_t port_handler;
	event_handler_proc_t chan_handler;
} tipc_srv_t;

typedef struct tipc_srv_state {
	const struct tipc_srv *service;
	handle_t port;
	void *priv;
	tipc_event_handler_t handler;
} tipc_srv_state_t;

/* closer services */
static void closer1_handle_port(const uevent_t *ev);

typedef struct closer1_state {
	uint conn_cnt;
} closer1_state_t;

static void closer2_handle_port(const uevent_t *ev);

typedef struct closer2_state {
	uint conn_cnt;
} closer2_state_t;

static void closer3_handle_port(const uevent_t *ev);

typedef struct closer3_state {
	handle_t chans[4];
	uint chan_cnt;
} closer3_state_t;

/* connect service */
static void connect_handle_port(const uevent_t *ev);

/* datasync service */
static void datasink_handle_port(const uevent_t *ev);
static void datasink_handle_chan(const uevent_t *ev);

/* datasink service has no per channel state so we can
 * just attach handler struct directly to channel handle
 */
static struct tipc_event_handler _datasink_chan_handler = {
	.proc = datasink_handle_chan,
	.priv = NULL,
};

/* echo service */
static void echo_handle_port(const uevent_t *ev);
static void echo_handle_chan(const uevent_t *ev);

typedef struct echo_chan_state {
	struct tipc_event_handler handler;
	uint msg_max_num;
	uint msg_cnt;
	uint msg_next_r;
	uint msg_next_w;
	struct ipc_msg_info msg_queue[0];
} echo_chan_state_t;

/* uuid service */
static void uuid_handle_port(const uevent_t *ev);

/* Other globals */
static bool stopped = false;

/************************************************************************/

#define IPC_PORT_ALLOW_ALL  (  IPC_PORT_ALLOW_NS_CONNECT \
                             | IPC_PORT_ALLOW_TA_CONNECT \
                            )

#define SRV_NAME(name)   SRV_PATH_BASE ".srv." name


static const struct tipc_srv _services[] =
{
	{
		.name = SRV_NAME("closer1"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = closer1_handle_port,
		.port_state_size = sizeof(struct closer1_state),
		.chan_handler = NULL,
	},
	{
		.name = SRV_NAME("closer2"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = closer2_handle_port,
		.port_state_size = sizeof(struct closer2_state),
		.chan_handler = NULL,
	},
	{
		.name = SRV_NAME("closer3"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = closer3_handle_port,
		.port_state_size = sizeof(struct closer3_state),
		.chan_handler = NULL,
	},
	{
		.name = SRV_NAME("connect"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_TA_CONNECT,
		.port_handler = connect_handle_port,
		.chan_handler = NULL,
	},
	/* datasink services */
	{
		.name = SRV_NAME("datasink"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = datasink_handle_port,
		.chan_handler = NULL,
	},
	{
		.name = SRV_NAME("ns_only"),
		.msg_num = 8,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_NS_CONNECT,
		.port_handler = datasink_handle_port,
		.chan_handler = NULL,
	},
	{
		.name = SRV_NAME("ta_only"),
		.msg_num = 8,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_TA_CONNECT,
		.port_handler = datasink_handle_port,
		.chan_handler = NULL,
	},
	/* echo */
	{
		.name = SRV_NAME("echo"),
		.msg_num = 8,
		.msg_size = MAX_PORT_BUF_SIZE,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = echo_handle_port,
		.chan_handler = echo_handle_chan,
	},
	/* uuid  test */
	{
		.name = SRV_NAME("uuid"),
		.msg_num = 2,
		.msg_size = 64,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.port_handler = uuid_handle_port,
		.chan_handler = NULL,
	},
};

static struct tipc_srv_state _srv_states[countof(_services)] = {
	[0 ... (countof(_services) - 1)] = {
		.port = INVALID_IPC_HANDLE,
	},
};

/************************************************************************/

static struct tipc_srv_state *get_srv_state(const uevent_t *ev)
{
	return containerof(ev->cookie, struct tipc_srv_state, handler);
}

static void _destroy_service(struct tipc_srv_state *state)
{
	if (!state) {
		TLOGI("non-null state expected\n");
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
			TLOGI("Failed (%d) to close port %d\n",
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
		TLOGI("null service specified: %p: %p\n");
		return ERR_INVALID_ARGS;
	}

	/* create port */
	int rc = port_create(srv->name, srv->msg_num, srv->msg_size,
			     srv->port_flags);
	if (rc < 0) {
		TLOGI("Failed (%d) to create port\n", rc);
		return rc;
	}

	/* setup port state  */
	state->port = (handle_t)rc;
	state->handler.proc = srv->port_handler;
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
		TLOGI("Failed (%d) to set cookie on port %d\n",
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
		TLOGI("non-null state expected\n");
		return ERR_INVALID_ARGS;
	}

	const struct tipc_srv *srv = state->service;
	_destroy_service(state);
	return _create_service(srv, state);
}

/*
 *  Kill all servoces
 */
static void kill_services(void)
{
	TLOGI ("Terminating unittest services\n");

	/* close any opened ports */
	for (uint i = 0; i < countof(_services); i++) {
		_destroy_service(&_srv_states[i]);
	}
}

/*
 *  Initialize all services
 */
static int init_services(void)
{
	TLOGI ("Init unittest services!!!\n");

	for (uint i = 0; i < countof(_services); i++) {
		int rc = _create_service(&_services[i], &_srv_states[i]);
		if (rc < 0) {
			TLOGI("Failed (%d) to create service %s\n",
			      rc, _services[i].name);
			return rc;
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
		TLOGI("error event (0x%x) for port (%d)\n",
		       ev->event, ev->handle);

		/* recreate service */
		restart_service(get_srv_state(ev));
		return true;
	}

	return false;
}

/****************************** connect test service *********************/

/*
 *  Local wrapper on top of async connect that provides
 *  synchronos connect with timeout.
 */
int sync_connect(const char *path, uint timeout)
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

static void connect_handle_port(const uevent_t *ev)
{
	uuid_t peer_uuid;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		char path[MAX_PORT_PATH_LEN];

		/* accept incomming connection and close it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("accept failed (%d)\n", rc);
			return;
		}
		close (rc);

		/* but then issue a series of connect requests */
		for (uint i = 2; i < MAX_USER_HANDLES; i++) {
			sprintf(path, "%s.port.accept%d", SRV_PATH_BASE, i);
			rc = sync_connect(path, 1000);
			close(rc);
		}
	}
}

/****************************** closer services **************************/

static void closer1_handle_port(const uevent_t *ev)
{
	uuid_t peer_uuid;
	struct closer1_state *st = get_srv_state(ev)->priv;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		/* new connection request, bump counter */
		st->conn_cnt++;

		/* accept it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("accept failed (%d)\n", rc);
			return;
		}
		handle_t chan = (handle_t) rc;

		if (st->conn_cnt & 1) {
			/* sleep a bit */
			nanosleep (0, 0, 100 * MSEC);
		}
		/* and close it */
		rc = close(chan);
		if (rc != NO_ERROR) {
			TLOGI("Failed (%d) to close chan %d\n", rc, chan);
		}
	}
}

static void closer2_handle_port(const uevent_t *ev)
{
	struct closer2_state *st = get_srv_state(ev)->priv;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		/* new connection request, bump counter */
		st->conn_cnt++;
		if (st->conn_cnt & 1) {
			/* sleep a bit */
			nanosleep (0, 0, 100 * MSEC);
		}

		/*
		 * then close the port without accepting any connections
		 * and restart it again
		 */
		restart_service(get_srv_state(ev));
	}
}

static void closer3_handle_port(const uevent_t *ev)
{
	uuid_t peer_uuid;
	struct closer3_state *st = get_srv_state(ev)->priv;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		/* accept connection */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("accept failed (%d)\n", rc);
			return;
		}
		/* add it to connection pool */
		st->chans[st->chan_cnt++] = (handle_t) rc;

		/* attach datasink service handler just in case */
		set_cookie((handle_t)rc, &_datasink_chan_handler);

		/* when max number of connection reached */
		if (st->chan_cnt == countof(st->chans)) {
			/* wait a bit */
			nanosleep (0, 0, 100 * MSEC);

			/* and close them all */
			for (uint i = 0; i < st->chan_cnt; i++ )
				close(st->chans[i]);
			st->chan_cnt = 0;
		}
		return;
	}
}

/****************************** datasync service **************************/


static int datasink_handle_msg(const uevent_t *ev)
{
	int rc;
	ipc_msg_info_t inf;

	/* for all messages */
	for (;;) {
		/* get message */
		rc = get_msg(ev->handle, &inf);
		if (rc == ERR_NO_MSG)
			break; /* no new messages */

		if (rc != NO_ERROR) {
			TLOGI("failed (%d) to get_msg for chan (%d)\n",
			      rc, ev->handle);
			return rc;
		}

		/* and retire it without actually reading  */
		rc = put_msg(ev->handle, inf.id);
		if (rc != NO_ERROR) {
			TLOGI("failed (%d) to putt_msg for chan (%d)\n",
			      rc, ev->handle);
			return rc;
		}
	}

	return NO_ERROR;
}

/*
 *  Datasink service channel handler
 */
static void datasink_handle_chan(const uevent_t *ev)
{
	if ((ev->event & IPC_HANDLE_POLL_ERROR) ||
	    (ev->event & IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
		/* close it as it is in an error state */
		TLOGI("error event (0x%x) for chan (%d)\n",
		       ev->event, ev->handle);
		close(ev->handle);
		return;
	}

	if (ev->event & IPC_HANDLE_POLL_MSG) {
		if (datasink_handle_msg(ev) != 0) {
			close(ev->handle);
			return;
		}
	}

	if (ev->event & IPC_HANDLE_POLL_HUP) {
		/* closed by peer */
		close(ev->handle);
		return;
	}
}

/*
 *  Datasink service port event handler
 */
static void datasink_handle_port(const uevent_t *ev)
{
	uuid_t peer_uuid;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		/* incomming connection: accept it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("failed (%d) to accept on port %d\n",
			       rc, ev->handle);
			return;
		}

		handle_t chan = (handle_t) rc;
		rc = set_cookie(chan, &_datasink_chan_handler);
		if (rc) {
			TLOGI("failed (%d) to set_cookie on chan %d\n",
			       rc, chan);
		}
	}
}

/******************************   echo service    **************************/

static uint8_t echo_msg_buf[MAX_PORT_BUF_SIZE];

static int _echo_handle_msg(const uevent_t *ev, int delay)
{
	int rc;
	iovec_t iov;
	ipc_msg_t msg;
	echo_chan_state_t *st = containerof(ev->cookie, echo_chan_state_t, handler);

	/* get all messages */
	while (st->msg_cnt != st->msg_max_num) {
		rc = get_msg(ev->handle, &st->msg_queue[st->msg_next_w]);
		if (rc == ERR_NO_MSG)
			break; /* no new messages */

		if (rc != NO_ERROR) {
			TLOGI("failed (%d) to get_msg for chan (%d)\n",
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
		iov.base = echo_msg_buf;
		iov.len  = sizeof(echo_msg_buf);
		msg.num_iov = 1;
		msg.iov     = &iov;
		msg.num_handles = 0;
		msg.handles  = NULL;

		/* read msg content */
		rc = read_msg(ev->handle, st->msg_queue[st->msg_next_r].id, 0, &msg);
		if (rc < 0) {
			TLOGI("failed (%d) to read_msg for chan (%d)\n",
			      rc, ev->handle);
			return rc;
		}

		/* update number of bytes received */
		iov.len = (size_t) rc;

		/* optionally sleep a bit an send it back */
		if (delay) {
			nanosleep (0, 0, 1000);
		}

		/* and send it back */
		rc = send_msg(ev->handle, &msg);
		if (rc == ERR_NOT_ENOUGH_BUFFER)
			break;

		if (rc < 0) {
			TLOGI("failed (%d) to send_msg for chan (%d)\n",
			      rc, ev->handle);
			return rc;
		}

		/* retire original message */
		rc = put_msg(ev->handle, st->msg_queue[st->msg_next_r].id);
		if (rc != NO_ERROR) {
			TLOGI("failed (%d) to put_msg for chan (%d)\n",
			      rc, ev->handle);
			return rc;
		}

		/* advance queue */
		st->msg_cnt--;
		st->msg_next_r++;
		if (st->msg_next_r == st->msg_max_num)
			st->msg_next_r = 0;
	}
	return NO_ERROR;
}

static int echo_handle_msg(const uevent_t *ev)
{
	return _echo_handle_msg(ev, false);
}

/*
 * echo service channel handler
 */
static void echo_handle_chan(const uevent_t *ev)
{
	if (ev->event & IPC_HANDLE_POLL_ERROR) {
		/* close it as it is in an error state */
		TLOGI("error event (0x%x) for chan (%d)\n",
		      ev->event, ev->handle);
		goto close_it;
	}

	if (ev->event & (IPC_HANDLE_POLL_MSG |
		         IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
		if (echo_handle_msg(ev) != 0) {
			TLOGI("error event (0x%x) for chan (%d)\n",
			      ev->event, ev->handle);
			goto close_it;
		}
	}

	if (ev->event & IPC_HANDLE_POLL_HUP) {
		goto close_it;
	}

	return;

close_it:
	free(ev->cookie);
	close(ev->handle);
}

/*
 *  echo service port event handler
 */
static void echo_handle_port(const uevent_t *ev)
{
	uuid_t peer_uuid;
	struct echo_chan_state *chan_st;
	const struct tipc_srv *srv = get_srv_state(ev)->service;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		handle_t chan;

		/* incomming connection: accept it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("failed (%d) to accept on port %d\n",
			       rc, ev->handle);
			return;
		}
		chan = (handle_t) rc;

		chan_st = calloc(1, sizeof(struct echo_chan_state) +
		                    sizeof(ipc_msg_info_t) * srv->msg_num);
		if (!chan_st) {
			TLOGI("failed (%d) to callocate state for chan %d\n",
			       rc, chan);
			close(chan);
			return;
		}

		/* init state */
		chan_st->msg_max_num  = srv->msg_num;
		chan_st->handler.proc = srv->chan_handler;
		chan_st->handler.priv = chan_st;

		/* attach it to handle */
		rc = set_cookie(chan, &chan_st->handler);
		if (rc) {
			TLOGI("failed (%d) to set_cookie on chan %d\n",
			       rc, chan);
			free(chan_st);
			close(chan);
			return;
		}
	}
}


/***************************************************************************/

/*
 *  uuid service port event handler
 */
static void uuid_handle_port(const uevent_t *ev)
{
	ipc_msg_t msg;
	iovec_t   iov;
	uuid_t peer_uuid;

	if (handle_port_errors(ev))
		return;

	if (ev->event & IPC_HANDLE_POLL_READY) {
		handle_t chan;

		/* incomming connection: accept it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGI("failed (%d) to accept on port %d\n",
			       rc, ev->handle);
			return;
		}
		chan = (handle_t) rc;

		/* send interface uuid */
		iov.base = &peer_uuid;
		iov.len  = sizeof(peer_uuid);
		msg.num_iov = 1;
		msg.iov     = &iov;
		msg.num_handles = 0;
		msg.handles  = NULL;
		rc = send_msg(chan, &msg);
		if (rc < 0) {
			TLOGI("failed (%d) to send_msg for chan (%d)\n",
			      rc, chan);
		}

		/* and close channel */
		close(chan);
	}
}

/***************************************************************************/

/*
 *  Dispatch event
 */
static void dispatch_event(const uevent_t *ev)
{
	assert(ev);

	if (ev->event == IPC_HANDLE_POLL_NONE) {
		/* not really an event, do nothing */
		TLOGI("got an empty event\n");
		return;
	}

	if (ev->handle == INVALID_IPC_HANDLE) {
		/* not a valid handle  */
		TLOGI("got an event (0x%x) with invalid handle (%d)",
		      ev->event, ev->handle);
		return;
	}

	/* check if we have handler */
	struct tipc_event_handler *handler = ev->cookie;
	if (handler && handler->proc) {
		/* invoke it */
		handler->proc(ev);
		return;
	}

	/* no handler? close it */
	TLOGI("no handler for event (0x%x) with handle %d\n",
	       ev->event, ev->handle);
	close(ev->handle);

	return;
}

/*
 *  Main entry point of service task
 */
int main(void)
{
	int rc;
	uevent_t event;

	/* Initialize service */
	rc = init_services();
	if (rc != NO_ERROR ) {
		TLOGI("Failed (%d) to init service", rc);
		kill_services();
		return -1;
	}

	/* handle events */
	while (!stopped) {
		event.handle = INVALID_IPC_HANDLE;
		event.event  = 0;
		event.cookie = NULL;
		rc = wait_any(&event, -1);
		if (rc < 0) {
			TLOGI("wait_any failed (%d)", rc);
			continue;
		}
		if (rc == NO_ERROR) { /* got an event */
			dispatch_event(&event);
		}
	}

	kill_services();
	return 0;
}

