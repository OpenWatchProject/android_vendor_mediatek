#include <linux/mmc/ioctl.h>

#define MMC_BLOCK_MAJOR         179
#define MMC_SEND_EXT_CSD        8

/*
* EXT_CSD fields
*/
#define EXT_CSD_FFU_STATUS              26
#define EXT_CSD_MODE_OPERATION_CODES    29
#define EXT_CSD_MODE_CONFIG             30
#define EXT_CSD_REV                     192
#define EXT_CSD_FFU_ARG                 487
#define EXT_CSD_FFU_FEATURES            492
#define EXT_CSD_SUPPORTED_MODE          493

/*
 * sector size
*/
#define CARD_BLOCK_SIZE         512
#define FFU_DWONLOAD_OP         302

/* Copied from kernel linux/mmc/core.h */
#define MMC_RSP_OPCODE          (1 << 4) /* response contains opcode */
#define MMC_RSP_BUSY            (1 << 3) /* card may send busy */
#define MMC_RSP_CRC             (1 << 2) /* expect valid crc */
#define MMC_RSP_136             (1 << 1) /* 136 bit response */
#define MMC_RSP_PRESENT         (1 << 0)

#define MMC_CMD_ADTC            (1 << 5)
#define MMC_CMD_AC              (0 << 5)

#define MMC_RSP_R1  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
