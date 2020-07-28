#ifndef __GF_CA_ENTRY_H__
#define __GF_CA_ENTRY_H__

#include <gf_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VAR(v)     ((void)v)

gf_error_t gf_ca_open_session(void);
void gf_ca_close_session(void);
gf_error_t gf_ca_invoke_command(void *cmd, int len);
gf_error_t gf_ca_invoke_command_from_user(void *cmd, int len);

#ifdef __cplusplus
}
#endif

#endif //__GF_CA_ENTRY_H__
