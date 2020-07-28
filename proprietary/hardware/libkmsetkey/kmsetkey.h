#ifndef __KMSETKEY_H__
#define __KMSETKEY_H__

#define MAX_MSG_SIZE           4096

#ifdef __cplusplus
extern "C" {
#endif

int32_t ree_import_attest_keybox(const uint8_t *peakb, const uint32_t peakb_len, const uint32_t finish);
int32_t ree_check_attest_keybox(const uint8_t *peakb, const uint32_t peakb_len, const uint32_t finish);

#ifdef __cplusplus
}
#endif

#endif /* __KMSETKEY_H__ */
