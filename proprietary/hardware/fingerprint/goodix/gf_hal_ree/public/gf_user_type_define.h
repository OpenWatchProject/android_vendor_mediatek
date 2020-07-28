#ifndef __GF_USER_TYPE_DEFINE_H__
#define __GF_USER_TYPE_DEFINE_H__

#include <stdint.h>
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DUMP_TEMPLATES_BUF (400 * 1024)

typedef enum {
    GF_USER_CMD_TEST_DUMP_TEMPLATES = 100,
    GF_USER_CMD_TEST_SHARE_MEMORY_PERFORMANCE,
    GF_USER_CMD_GET_LAST_IDENTIFY_ID,
    GF_USER_CMD_GET_AUTHENTICATOR_VERSION,
    GF_USER_CMD_ENUMERATE,
} gf_user_cmd_id_t;

typedef struct gf_dump_templates {
    uint8_t buf[MAX_DUMP_TEMPLATES_BUF];
    uint32_t buf_len;
    uint32_t group_id;
    uint32_t finger_id;
} gf_user_dump_templates_t;

typedef struct {
    uint32_t version;
} gf_user_authenticator_version_t;

typedef struct {
    uint32_t last_identify_id;
} gf_user_last_identify_id_t;

typedef struct {
    uint32_t size;
    uint32_t group_ids[MAX_FINGERS_PER_USER];
    uint32_t finger_ids[MAX_FINGERS_PER_USER];
} gf_user_enumerate_t;

#ifdef __cplusplus
}
#endif

#endif // __GF_COMMON_H__
