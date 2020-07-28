/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef ICLIENT_API_DAEMON_H_
#define ICLIENT_API_DAEMON_H_

#include <include/tee_client_api.h>
#include <linux/tee.h>
TEEC_Result ut_initialize_context(const char *name, TEEC_Context *ut_context);
TEEC_Result ut_open_session(int32_t fd, uintptr_t buf, uint32_t buf_len);
void ut_close_session(TEEC_Session *session);
int ut_shm_alloc(int fd, uint64_t size, int *id);
int ut_shm_register_fd(int fd, struct tee_ioctl_shm_register_fd_data *data);
TEEC_Result ut_invoke_command(int32_t fd, uintptr_t buf, uint32_t buf_len);
void ut_request_cancel(int32_t fd, struct tee_ioctl_cancel_arg *arg);
void ut_close_fd(int fd);
#ifdef __cplusplus
extern "C" {
#endif
int ut_shm_get_pa(int32_t fd, int32_t id, uint64_t *pa);
int ut_shm_flush_cache(int32_t fd, int32_t id);
#ifdef __cplusplus
}
#endif
#endif // ICLIENT_API_DAEMON_H_
