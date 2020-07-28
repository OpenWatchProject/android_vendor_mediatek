#ifndef __EMMC_H_
#define __EMMC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/mmc/ioctl.h>
#include "storage_otp.h"

#define MMC_RSP_SPI_S1	(1 << 7)		/* one status byte */
#define MMC_RSP_SPI_S2	(1 << 8)		/* second byte */
#define MMC_RSP_SPI_B4	(1 << 9)		/* four data bytes */
#define MMC_RSP_SPI_BUSY (1 << 10)		/* card may send busy */

#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_PRESENT	(1 << 0)

#define MMC_CMD_MASK	(3 << 5)		/* non-SPI command type */

#define MMC_CMD_BCR	(3 << 5)
#define MMC_CMD_BC	(2 << 5)
#define MMC_CMD_ADTC	(1 << 5)
#define MMC_CMD_AC	(0 << 5)

#define MMC_SWITCH                6   /* ac   [31:0] See below   R1b */
#define MMC_SEND_STATUS          13   /* ac   [31:16] RCA        R1  */

#define MMC_READ_SINGLE_BLOCK    17   /* adtc [31:0] data addr   R1  */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0] data addr   R1  */

#define MMC_SET_BLOCK_COUNT      23   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_BLOCK          24   /* adtc [31:0] data addr   R1  */

#define MMC_SET_WRITE_PROT       28   /* ac   [31:0] data addr   R1b */
#define MMC_CLR_WRITE_PROT       29   /* ac   [31:0] data addr   R1b */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0] wpdata addr R1  */
#define MMC_SEND_WRITE_PROT_TYPE      31   /* adtc [31:0] wpdata addr R1  */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)

#define MMC_RSP_SPI_R1	(MMC_RSP_SPI_S1)
#define MMC_RSP_SPI_R1B	(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY)

#define EXT_CSD_USR_WP                  171     /* R/W */
#define EXT_CSD_CMD_SET_NORMAL          (1<<0)
#define EXT_CSD_CMD_SET_SECURE          (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE        (1<<2)
/* USER_WP[171] */

#define EXT_CSD_USR_WP_DIS_PERM_PWD     (1<<7)
#define EXT_CSD_USR_WP_DIS_CD_PERM_WP   (1<<6)
#define EXT_CSD_USR_WP_DIS_PERM_WP      (1<<4)
#define EXT_CSD_USR_WP_DIS_PWR_WP       (1<<3)
#define EXT_CSD_USR_WP_EN_PERM_WP       (1<<2)
#define EXT_CSD_USR_WP_EN_PWR_WP        (1)

/*
 * MMC_SWITCH access modes
 */

#define MMC_SWITCH_MODE_WRITE_BYTE      0x03    /* Set target to value */
#define MMC_SWITCH_MODE_CLEAR_BITS      0x02    /* Clear bits which are 1 in value */
#define MMC_SWITCH_MODE_SET_BITS        0x01    /* Set bits which are 1 in value */
#define MMC_SWITCH_MODE_CMD_SET         0x00    /* Change the command set */

int emmc_storage_init(struct otp *otp_device);

#ifdef __cplusplus
}
#endif

#endif
