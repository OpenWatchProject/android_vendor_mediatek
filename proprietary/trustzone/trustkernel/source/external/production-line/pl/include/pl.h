#ifndef PL_H_
#define PL_H_

/* PL device related operation */
#ifdef __cplusplus
extern "C" {
#endif

struct certificate_desc {
    uint32_t version;
    uint32_t cert_type;
    uint32_t project_count;
    uint32_t revokeid;
};

struct brand_model_platform {
    char *brand; char *model; char *platform;
};

enum device_certificate_type {
    CERTIFICATE_EVALUATE = 1,
    CERTIFICATE_FULL,
    CERTIFICATE_LIMITED,
    CERTIFICATE_REVOKE,
    CERTIFICATE_BUILTIN,
    CERTIFICATE_VOLUME,
    NR_CERTIFICATE_TYPES
};

int pl_device_get_cert_info(struct certificate_desc *certdesc, struct brand_model_platform **bmp);

int pl_device_get_cert_digest(uint8_t *digest, uint32_t *digest_size);

enum device_authorize_state {
    DEVICE_EXPIRED = 0,
    DEVICE_AUTHORIZED,
    DEVICE_EVALUABLE,
};

enum device_verify_state {
    DEVICE_CORRUPTED_CERTIFICATE = 0,
    DEVICE_VERIFIED,
    DEVICE_UNVERIFIED,
};

enum truststore_type {
    TRUSTSTORE_TYPE_RPMB,
    TRUSTSTORE_TYPE_SDRPMB,
    TRUSTSTORE_TYPE_SE,
    NR_TRUSTSTORE_TYPES
};

int pl_device_get_status(uint32_t *version, uint32_t *verify, uint32_t *authorize,
    uint32_t *rpmbvalid, uint32_t *randtoken, struct brand_model_platform *bmp);

int pl_device_get_truststore(uint32_t *type, uint32_t *status, uint32_t *version);

int pl_device_program_cert(char *certbuf, uint32_t certbuf_size, char *hashbuf, uint32_t hashsize, uint32_t *cert_type);

int pl_device_revoke_cert(char *certbuf, uint32_t certbuf_size);

#ifdef __cplusplus
}
#endif

#endif
