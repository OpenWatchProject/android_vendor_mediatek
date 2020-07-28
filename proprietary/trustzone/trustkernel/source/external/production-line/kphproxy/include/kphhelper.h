#ifndef KPH_HELPER_H_
#define KPH_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

int kphh_get_config_filename(char *buf, size_t capacity);

int kphh_get_device_filename(char *buf, size_t capacity);

int kphh_get_pubkey_filename(uint32_t id, char *buf, size_t capacity);

int kphh_get_uuid_filename(uint32_t id, char *buf, size_t capacity);

/* id related APIs */
int kphh_read_ree_id(char *id, size_t capacity);

int kphh_read_ree_id_1(char *id, size_t capacity);

int kphh_read_ree_id_2(char *id, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
