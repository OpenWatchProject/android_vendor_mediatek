#ifndef _SHA2_EXPORT_H_
#define _SHA2_EXPORT_H_

/* ==================== */
/* error code           */
/* ==================== */
#define GCPU_E_OK                   0
#define GCPU_E_HW_INIT_FAILURE      1
#define GCPU_E_HW_UNINIT_FAILURE    2
#define GCPU_E_SW_INIT_FAILURE      3
#define GCPU_E_SW_UNINIT_FAILURE    4
#define GCPU_E_CMD_UNAVAIL          5
#define GCPU_E_NO_INTERRUPT         6
#define GCPU_E_CMD_SEC_ERR          7
#define GCPU_E_INVALID_PARAMS       8
#define GCPU_E_INVALID_RANGE        9
#define GCPU_E_NOT_64B_ALIGNED      10
#define GCPU_E_SELF_TEST_FAILED     11

/* ==================== */
/* exported functions   */
/* ==================== */
unsigned int sha256(const unsigned char *in, unsigned int inlen, unsigned char *hash);
unsigned int sha256_test(void);
unsigned int sha256_init(void);


#endif /* _SHA2_EXPORT_H_ */

