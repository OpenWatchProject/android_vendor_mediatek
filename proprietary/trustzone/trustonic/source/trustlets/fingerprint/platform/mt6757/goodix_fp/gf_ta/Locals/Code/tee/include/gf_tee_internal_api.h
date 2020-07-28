#ifndef __GF_TEE_INTERNAL_API_H__
#define __GF_TEE_INTERNAL_API_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_tee_storage.h"
#include "gf_log.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#define UNUSED_VAR(X)   (void)(X)

//memory
void *gf_malloc(int32_t size);
void gf_free(void *buffer);
void *gf_malloc_with_default_value(int32_t size, uint8_t default_value);
void* gf_realloc(void* buf, uint32_t buf_len);

//time
void gf_sleep(uint32_t milliseconds);
gf_error_t gf_get_timestamp(uint64_t *timestamp);

//secure driver
gf_error_t gf_sync_auth_result(uint32_t finger_id, uint64_t time);
gf_error_t gf_wechat_api_get_session_id(uint64_t *session_id);
gf_error_t gf_wechat_api_set_session_id(uint64_t session_id);
gf_error_t gf_secure_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time);

//other
gf_error_t gf_generate_random(void *random_buffer, int32_t len);
gf_error_t gf_get_tee_version(int8_t *buffer,uint32_t len);

//key master
uint32_t gf_get_hmac_key(uint8_t *hmac_key, uint32_t key_length);
gf_error_t gf_generate_hmac(gf_hw_auth_token_t *token);

//spi
gf_error_t gf_oswego_secspi_read_bytes(uint16_t readAddr, uint8_t *buffer, uint32_t readLen) ;
gf_error_t gf_oswego_secspi_read_bytes_high_speed(uint16_t readAddr, uint8_t *buffer, uint32_t readLen);

#ifdef __cplusplus
}
#endif

#endif // __GF_TEE_INTERNAL_API_H__
