#ifndef _KPHPROXY_H_
#define _KPHPROXY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TA_ID_TEE       (0U)
#define TA_ID_IFAA      (1U)
#define TA_ID_SOTER     (6U)

int kph_verify_tee_all(void);

/* verify imported/generated key/data/config */
int kph_verify_ta_key(uint32_t taid);
int kph_remove_ta_key(uint32_t taid);

int kph_verify_ta_data(uint32_t taid);
int kph_verify_ta_data2(uint32_t taid, const char *tag);
int kph_remove_ta_data(uint32_t taid);
int kph_remove_ta_data2(uint32_t taid, const char *tag);

int kph_verify_device_config(void);
int kph_remove_device_config(void);

#define TA_DATA_HEADER_MAGIC    0x7ADA7A
#define TA_DATA_HEADER_VERSION  0x1

#define TA_DATA_PERM_PUBLIC     0x0
#define TA_DATA_PERM_TEE        0x1
#define TA_DATA_PERM_TA         0x2

struct ta_data_descriptor {
    uint32_t perm;
    uint32_t __resv[31];
} __attribute__ ((packed));

struct ta_data_header {
    uint32_t magic;
    uint32_t version;
    uint32_t head_size;
    struct ta_data_descriptor desc;
} __attribute__ ((packed));

int kph_import_ta_data(uint32_t taid,
                    const char *tag,
                    struct ta_data_descriptor *desc,
                    size_t desc_size,
                    void *buf,
                    uint32_t length);

int kph_export_system_ta_data(uint32_t ta_id,
                            const char *tag,
                            void *out,
                            uint32_t *out_len);

int kph_import_keybox(const unsigned char *keybox,
                    uint32_t size);

int kph_export_device_secret(void *prefix,
                            uint32_t prefix_len,
                            void *buf_out,
                            uint32_t *out_len);

int kph_export_system_ta_uuid(uint32_t ta_id,
                            void **out,
                            uint32_t *out_len);

int kph_export_device(uint8_t **buf, size_t *len);

int kph_export_config(uint8_t **buf, size_t *len);

/* zero-terminated buffer */
int kph_import_config(char *buf, const char *sn, const char *rid);

int kph_generate_key_legacy(uint32_t ta_id);

int kph_import_key_legacy(uint32_t ta_id,
                        uint8_t *buf,
                        size_t length);

int kph_export_key_legacy(uint32_t ta_id,
                        uint8_t **key_data,
                        size_t *key_len);

int kph_read_provision_state(uint32_t *state);

int kph_write_provision_state(uint32_t state);

int kph_overwrite_provision_state(uint32_t state);

int kph_ta_version(char *version, size_t capacity);

/* deprecated functions */

int verify_ta_key(uint32_t taid)
	__attribute__((deprecated));

int remove_ta_key(uint32_t taid)
	__attribute__((deprecated));

int verify_ta_data(uint32_t taid)
	__attribute__((deprecated));

int verify_ta_data2(uint32_t taid, const char *tag)
	__attribute__((deprecated));

int remove_ta_data(uint32_t taid)
	__attribute__((deprecated));

int remove_ta_data2(uint32_t taid, const char *tag)
	__attribute__((deprecated));

int verify_device_config(void)
	__attribute__((deprecated));

int remove_device_config(void)
	__attribute__((deprecated));

int import_ta_data(uint32_t taid, const char *tag,
    struct ta_data_descriptor *desc, size_t desc_size, void *buf, uint32_t length)
	__attribute__((deprecated));

int verify_tee_all(void)
	__attribute__((deprecated));

int export_device_secret(void *prefix, uint32_t prefix_len,
    void *buf_out, uint32_t *out_len)
	__attribute__((deprecated));

int export_system_ta_uuid(uint32_t ta_id,
    void *out, uint32_t *out_len)
	__attribute__((deprecated));

int export_system_ta_data(uint32_t ta_id, const char *tag,
    void *out, uint32_t *out_len)
	__attribute__((deprecated));

int read_provision_state(uint32_t *state)
	__attribute__((deprecated));

int write_provision_state(uint32_t state)
	__attribute__((deprecated));

int overwrite_provision_state(uint32_t state)
	__attribute__((deprecated));

int import_keybox(const unsigned char *keybox, uint32_t size)
	__attribute__((deprecated));

#ifdef __cplusplus
}
#endif

#endif
