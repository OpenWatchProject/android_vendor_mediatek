

#ifndef __MTEE_SERVER_H__
#define __MTEE_SERVER_H__

#include <tz_private/system.h>

// handler define
typedef int (* init_handler_t) (void *srv);
typedef int (* port_connected_handler_t) (uint32_t chan_handle, void *srv);
typedef int (* chan_disconnect_handler_t) (uint32_t chan_handle, void *chan_st);
typedef int (* service_handler_t) (int , int , uint32_t , MTEEC_PARAM *);
typedef void (*event_handler_proc_t) (const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);

// event handler
typedef struct tipc_event_handler {
	event_handler_proc_t proc;
	void *priv;
	service_handler_t service_handler; // fixe me!!!
} tipc_event_handler_t;

// service table
typedef struct tipc_srv {
	const char *name;
	uint   msg_num;
	size_t msg_size;
	uint   port_flags;
	size_t port_state_size;
	size_t chan_state_size;
	uuid_t peer_uuid;// this means: the server only accepts this uuid connect. if peer_uuid =0, the server accepts everyone.
	init_handler_t init_handler; // service init handler, called once after service initialized
	port_connected_handler_t port_connected_handler; // port connected handler, called when port is connected
	chan_disconnect_handler_t disc_handler; // disconnected handler
	service_handler_t service_handler; // service handler, called when event's comming
} tipc_srv_t;

// service state
typedef struct tipc_srv_state {
	const struct tipc_srv *service;
	handle_t port;
	void *priv;
	tipc_event_handler_t handler;
} tipc_srv_state_t;

// channel state
typedef struct systa_chan_state {
	struct tipc_event_handler handler;
	tipc_srv_t *srv;
	uint msg_max_num;
	uint msg_cnt;
	uint msg_next_r;
	uint msg_next_w;
	struct ipc_msg_info msg_queue[0];
} systa_chan_state_t;


void MTEE_handle_port(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);
void MTEE_handle_chan(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);

void kill_services(const tipc_srv_t *services_p, int count, tipc_srv_state_t *srv_states_p);
int init_services(const tipc_srv_t *services_p, int count, tipc_srv_state_t *srv_states_p);
void dispatch_event(const uevent_t *ev, uint8_t *msg_buf_p, int msg_buf_size);


/* GZ Ree service
*/
enum GZ_ReeServiceCommand {

	REE_SERVICE_CMD_BASE = 0x0,
	REE_SERVICE_CMD_ADD,
	REE_SERVICE_CMD_MUL,
	REE_SERVICE_CMD_NEW_THREAD,
	REE_SERVICE_CMD_KICK_SEM,
	REE_SERVICE_CMD_TEE_INIT_CTX,
	REE_SERVICE_CMD_TEE_FINAL_CTX,
	REE_SERVICE_CMD_TEE_OPEN_SE,
	REE_SERVICE_CMD_TEE_CLOSE_SE,
	REE_SERVICE_CMD_TEE_INVOK_CMD,
	REE_SERVICE_CMD_END
};

int MTEE_ReeServiceCall (int session, int command, uint32_t paramTypes, MTEEC_PARAM param[4]);

#endif /* __MTEE_SERVER_H__ */


