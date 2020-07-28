/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_CA_ENTRY_H__
#define __GF_CA_ENTRY_H__

#include <gf_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED_VAR
#define UNUSED_VAR(v)     ((void)v)
#endif

gf_error_t gf_ca_open_session(void);
void gf_ca_close_session(void);
gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id, void *cmd, int len);

void gf_ca_set_handle(int fd);

#ifdef __cplusplus
}
#endif

#endif //__GF_CA_ENTRY_H__
