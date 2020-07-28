
#ifndef _DRRPMBOPS_H
#define _DRRPMBOPS_H

#include "drrpmb_Api.h"

/*
 * Function id definitions
 */
#define FID_DRV_OPEN_SESSION     1
#define FID_DRV_CLOSE_SESSION    2
#define FID_DRV_INIT_DATA        3
#define FID_DRV_EXECUTE          4
#define FID_DRV_ADD_RPMB         5
#define FID_DRV_GET_KEY          6
#define FID_DRV_READ_DATA        7
#define FID_DRV_WRITE_DATA       8
#define FID_DRV_GET_PART_ADDR    9
#define FID_DRV_GET_WCOUNTER     10
#define FID_DRV_GET_REL_WR_SEC_C 11
#define FID_DRV_GET_PART_SIZE    12
#define FID_DRV_PROGRAM_KEY      13
#define FID_DRV_CHECK_FLAG       14
#define FID_DRV_SET_KEY_FLAG     15

/* .. add more when needed */

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
typedef struct {
    uint32_t    uid;
    uint32_t    ret;
    uint32_t    commandId;
    uint8_t     buf[MAX_RPMB_REQUEST_SIZE];
    uint16_t    blks;
    uint16_t    part_addr;
    uint32_t    part_size;
    uint16_t	start_blk;
    uint32_t    result;
    uint16_t    emmc_rel_wr_sec_c;
    uint8_t     mac_key[32];
} tlApiRpmb_t, *tlApiRpmb_ptr;

#define RPMB_SZ_STUFF 196
#define RPMB_SZ_MAC   32
#define RPMB_SZ_DATA  256
#define RPMB_SZ_NONCE 16

#define RPMB_PROGRAM_KEY       1       /* Program RPMB Authentication Key */
#define RPMB_GET_WRITE_COUNTER 2       /* Read RPMB write counter */
#define RPMB_WRITE_DATA        3       /* Write data to RPMB partition */
#define RPMB_READ_DATA         4       /* Read data from RPMB partition */
#define RPMB_RESULT_READ       5       /* Read result request */
#define RPMB_REQ               1       /* RPMB request mark */
#define RPMB_RESP              (1 << 1)/* RPMB response mark */
#define RPMB_AVALIABLE_SECTORS 8       /* 4K page size */

#define RPMB_TYPE_BEG          510
#define RPMB_RES_BEG           508
#define RPMB_BLKS_BEG          506
#define RPMB_ADDR_BEG          504
#define RPMB_WCOUNTER_BEG      500

#define RPMB_NONCE_BEG         484
#define RPMB_DATA_BEG          228
#define RPMB_MAC_BEG           196

#define RPMB_ERR_NONE           0
#define RPMB_ERR_CMP_ERR        1
#define RPMB_ERR_IO_ERR         2
#define RPMB_ERR_TIMEOUT        3
//int drRpmbRegister(uint32_t sid);
//int drRpmbReadData();

struct rpmb_t {
    unsigned char stuff[RPMB_SZ_STUFF];
    unsigned char mac[RPMB_SZ_MAC];
    unsigned char data[RPMB_SZ_DATA];
    unsigned char nonce[RPMB_SZ_NONCE];
    unsigned int write_counter;
    unsigned short address;
    unsigned short block_count;
    unsigned short result;
    unsigned short request;
};

typedef struct _rpmb_part_tbl {
    unsigned int start_blk;
    unsigned int length;
} RPMB_PART_TBL;

void drRpmbPartInit(uint32_t rpmb_size);
int drRpmbInit();
int drRpmbGetKey(tlApiRpmb_ptr pRpmbData);
int drRpmbGetRelWrSecC(tlApiRpmb_ptr pRpmbData);
int drRpmbGetPartAddr(tlApiRpmb_ptr pRpmbData);
int drRpmbGetPartSize(tlApiRpmb_ptr pRpmbData);
int drRpmbReadData(tlApiRpmb_ptr pRpmbData);
int drRpmbGetWcounter(tlApiRpmb_ptr pRpmbData);
int drRpmbWriteData(tlApiRpmb_ptr pRpmbData);


uint16_t cpu_to_be16p(uint16_t *p);
uint32_t cpu_to_be32p(uint32_t *p);

#endif
