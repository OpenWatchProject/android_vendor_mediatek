#ifndef COMM_H
#define COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gf_log.h"

typedef struct rsa_key {
    struct {
        uint8_t* buf;
        size_t len;
    } n;
    struct {
        uint8_t* buf;
        size_t len;
    } d;
    struct {
        uint8_t* buf;
        size_t len;
    } e;
} rsa_key_t;

#define SHA1_HASH_LEN 20
#define SHA256_HASH_LEN 32
#define RSA2048_LEN 256

#define LOG_TAG "OEMTA"

#define LOG_DEBUG(...)                    \
    GF_LOGD("[DEBUG]"__VA_ARGS__);

/* info */
#define LOG_WARN(...)                    \
    GF_LOGI("[INFO]"__VA_ARGS__);

/* error */
#define LOG_ERROR(...)                    \
    GF_LOGE("[ERR]"__VA_ARGS__);


#ifdef __cplusplus
}
#endif

#endif
