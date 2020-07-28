#ifndef __MTEE_SRV_H__
#define __MTEE_SRV_H__

#include <sys/types.h>
#include <tz_private/mtee_sys_fd.h>

enum MTEE_USER_CMD {
	MTEE_HANDLE_NEW = 0x1,
	MTEE_HANDLE_NEW_TYPE,
	MTEE_HANDLE_GET_OBJ,
	MTEE_HANDLE_PUT_OBJ,
	MTEE_HANDLE_FREE,
	MTEE_SESSION_CREATE,
	MTEE_SESSION_CLOSE,
	MTEE_SESSION_REG_RSC,
	MTEE_SESSION_UNREG_RSC,
	MTEE_SESSION_QUERY_RSC,
	MTEE_SESSION_GET_OBJ,
	MTEE_SESSION_PUT_OBJ,
	MTEE_SESSION_GET_CB_NUM,
	MTEE_SESSION_GET_CBS,
	MTEE_MUTEX_CREATE,
	MTEE_MUTEX_DESTROY,
	MTEE_MUTEX_LOCK,
	MTEE_MUTEX_UNLOCK,
	MTEE_MUTEX_TRYLOCK,
	MTEE_MUTEX_ISLOCK,
	MTEE_SEMA_CREATE,
	MTEE_SEMA_DESTROY,
	MTEE_SEMA_DOWN,
	MTEE_SEMA_DOWN_TIMEOUT,
	MTEE_SEMA_UP,
	MTEE_SEMA_TRYDOWN,
	MTEE_SEMA_GET,
    MTEE_EVENT_CREATE,
    MTEE_EVENT_DESTROY,
    MTEE_EVENT_WAIT,
    MTEE_EVENT_SIGNAL,
    MTEE_EVENT_UNSIGNAL,
    MTEE_DRV_SET_UUID,
    MTEE_DRV_GET_UUID,
    MTEE_THREAD_STACK_START,
    MTEE_THREAD_DUMP_ALL,
    MTEE_HA_CREATE,
    MTEE_HA_START,
    MTEE_HA_UNLOAD,
    MTEE_REE_ONGOING,
    MTEE_REE_ARRIVAL,
    MTEE_THREAD_CURRENT_THREAD,
};

#define MTEE_SESSION_NAME_LEN 32
#define MTEE_HANDLE_TYPE_LEN 8
#define MTEE_DRV_UUID_LEN 32

typedef struct user_handle {
	uint32_t handle;
	char type[MTEE_HANDLE_TYPE_LEN];
	uint32_t obj;
	uint32_t type_p;
	int ret;
} user_handle_t;

typedef struct user_session {
	uint32_t handle;
	uint64_t res;
	uint64_t callback;
	int ret;
	char name[MTEE_SESSION_NAME_LEN];
} user_session_t;

typedef struct user_drv {
	char uuid[MTEE_DRV_UUID_LEN];
	uint32_t drv_id;
	int ret;
} user_drv_t;

typedef struct user_thread {
	uint32_t trusty_thread_stack_start;
	int ret;
} user_thread_t;

typedef struct user_session_cb_node {
	uint64_t callback;
	uint64_t data;
} cb_node_t;

typedef struct user_session_cbs {
	uint32_t handle;
	int cb_num;
	cb_node_t buffer;
} session_cbs_t;

typedef struct user_mutex {
	uint32_t mutex;
	int ret;
} user_mutex_t;

typedef struct user_sema {
	uint32_t sema;
	int timeout;
	int ret;
	uint64_t ksema_ptr;
} user_sema_t;

typedef struct user_event {
	uint32_t event;
    uint32_t timeout;
    int status;
} user_event_t;
typedef struct user_DL {
	uint32_t	elf;
	uint64_t	TAaddr;	 /* TA(loadee) address */
	uint32_t	stack;
	uint32_t	heap;
} user_DL_t;

#endif /* __MTEE_SRV_H__ */
