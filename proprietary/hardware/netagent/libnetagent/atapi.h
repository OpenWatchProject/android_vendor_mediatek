#ifndef __LIBATAPI_ATAPI_H__
#define __LIBATAPI_ATAPI_H__

/* ---------------------------------------------------- */
/*  ATAPI - Core                                        */
/* ---------------------------------------------------- */

enum ATAPI_RET_e {
    ATAPI_RET_SUCCESS       = 0,
    ATAPI_RET_ERROR         = 1,
    ATAPI_RET_TIMEOUT       = 2,
    ATAPI_RET_NOT_SUPPORT   = 3,
};

void *atapi_init(int ch_type, char *ch_name);

int atapi_deinit(void *obj);

int atapi_execute_command(void *obj, char *at_cmd, char *result_buffer, int result_buffer_len, char *result_prefix, int cmd_timeout);

int atapi_execute_command_no_response(void *obj, char *at_cmd, int cmd_timeout);

/* ---------------------------------------------------- */
/*  ATAPI - Callback                                    */
/* ---------------------------------------------------- */

int atapi_cb_enter_urc_mode(void *obj);

int atapi_cb_set_user_data(void *obj, void *data);

int atapi_cb_get_user_data(void *obj, void **data);

typedef int (*ATAPI_urc_notify_cb) (void *data, char *urc);

int atapi_cb_set_urc_notify(void *obj, ATAPI_urc_notify_cb fn);

char * atapi_urc_get(void *obj);

int atapi_urc_init(void *obj);

#endif /* __LIBATAPI_ATAPI_H__ */
