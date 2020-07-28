#ifndef __ATF_DAPC_H__
#define __ATF_DAPC_H__

#include <platform_def.h>
#include <stdint.h>

/******************************************************************************
 *
 * FUNCTION DEFINATION

 ******************************************************************************/
int start_devapc(void);
void handle_sramrom_vio(void);
unsigned int dump_devapc(int, int, int);
uint64_t sip_hyp_devapc_ctrl(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4);

/******************************************************************************
 *
 * STRUCTURE DEFINATION

 ******************************************************************************/
enum E_TRANSACTION {
	NON_SECURE_TRANSACTION = 0,
	SECURE_TRANSACTION,
	E_TRANSACTION_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

enum APC_ATTR {
	E_NO_PROTECTION = 0,
	E_SEC_RW_ONLY,
	E_SEC_RW_NS_R,
	E_FORBIDDEN,
	E_APC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

enum E_MASK_DOM {
	E_DOMAIN_0 = 0,
	E_DOMAIN_1,
	E_DOMAIN_2,
	E_DOMAIN_3,
	E_DOMAIN_4,
	E_DOMAIN_5,
	E_DOMAIN_6,
	E_DOMAIN_7,
	E_DOMAIN_8,
	E_DOMAIN_9,
	E_DOMAIN_10,
	E_DOMAIN_11,
	E_DOMAIN_12,
	E_DOMAIN_13,
	E_DOMAIN_14,
	E_DOMAIN_15,
	E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

enum DAPC_MASTER_TYPE {
	E_DAPC_INFRA_MASTER = 0,
	E_DAPC_MASTER_TYPE_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

enum DAPC_SLAVE_TYPE {
	E_DAPC_INFRA_SLAVE = 0,
	E_DAPC_MM_SLAVE,
	E_DAPC_MD_SLAVE,
	E_DAPC_SLAVE_TYPE_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

enum DAPC_PD_SLAVE_TYPE {
	E_DAPC_PD_INFRA_MM_MD_SLAVE = 0,
	E_DAPC_PD_SLAVE_TYPE_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
};

struct INFRA_PERI_DEVICE_INFO {
	unsigned char       d0_permission;
	unsigned char       d1_permission;
	unsigned char       d8_permission;
	unsigned char       d2_permission;
	unsigned char       d3_permission;
	unsigned char       d9_permission;
};

#define DAPC_INFRA_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2, PERM_ATTR3, PERM_ATTR4, PERM_ATTR5, PERM_ATTR6) \
{(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3, \
(unsigned char)PERM_ATTR4, (unsigned char)PERM_ATTR5, (unsigned char)PERM_ATTR6}


struct MM_DEVICE_INFO {
	unsigned char       d0_permission;
	unsigned char       d2_permission;
};

struct MD_DEVICE_INFO {
	unsigned char       d0_permission;
};

void set_master_transaction(enum DAPC_MASTER_TYPE master_type, unsigned int master_index,
	enum E_TRANSACTION transaction_type);

#define DAPC_MM_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR1}
#define DAPC_MD_ATTR(DEV_NAME, PERM_ATTR1) {(unsigned char)PERM_ATTR1}

/******************************************************************************
 *
 * UTILITY DEFINATION

 ******************************************************************************/

#define devapc_writel(VAL, REG)		mmio_write_32((uintptr_t)REG, VAL)
#define devapc_readl(REG)		mmio_read_32((uintptr_t)REG)

/******************************************************************************
 *
 * REGISTER ADDRESS DEFINATION

 ******************************************************************************/
#define DEVAPC_AO_INFRA_BASE        0x1001C000
#define DEVAPC_AO_MD_BASE           0x10019000
#define DEVAPC_AO_MM_BASE           0x1000E000

#define DEVAPC_PD_INFRA_BASE        0x10207000

#define SRAMROM_BASE                0x10214000

/*******************************************************************************************/
/* Device APC AO for INFRA */
#define DEVAPC_INFRA_D0_APC_0          ((unsigned int *)(DEVAPC_AO_INFRA_BASE+0x0000))

#define DEVAPC_INFRA_MAS_SEC_0         ((unsigned int *)(DEVAPC_AO_INFRA_BASE+0x1B00))

#define DEVAPC_INFRA_APC_CON           ((unsigned int *)(DEVAPC_AO_INFRA_BASE+0x1F00))

/*******************************************************************************************/
/* Device APC AO for MD */
#define DEVAPC_MD_D0_APC_0             ((unsigned int *)(DEVAPC_AO_MD_BASE+0x0000))

#define DEVAPC_MD_APC_CON              ((unsigned int *)(DEVAPC_AO_MD_BASE+0x0F00))

/*******************************************************************************************/
/* Device APC AO for MM */
#define DEVAPC_MM_D0_APC_0             ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0000))

#define DEVAPC_MM_SEC_EN_0             ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0E00))
#define DEVAPC_MM_SEC_EN_1             ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0E04))
#define DEVAPC_MM_SEC_EN_2             ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0E08))
#define DEVAPC_MM_SEC_EN_3             ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0E0C))

#define DEVAPC_MM_APC_CON              ((unsigned int *)(DEVAPC_AO_MM_BASE+0x0F00))


/*******************************************************************************************/
/* Device APC PD for Infra/MM/MD */
#define DEVAPC_PD_INFRA_VIO_MASK_0     ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0000))

#define DEVAPC_PD_INFRA_VIO_STA_0      ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0400))

#define DEVAPC_PD_INFRA_VIO_DBG0       ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0900))
#define DEVAPC_PD_INFRA_VIO_DBG1       ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0904))

#define DEVAPC_PD_INFRA_APC_CON        ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0F00))

#define DEVAPC_PD_INFRA_VIO_SHIFT_STA  ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0F10))
#define DEVAPC_PD_INFRA_VIO_SHIFT_SEL  ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0F14))
#define DEVAPC_PD_INFRA_VIO_SHIFT_CON  ((unsigned int *)(DEVAPC_PD_INFRA_BASE+0x0F20))


/*******************************************************************************************/

#define INFRA_AO_SEC_CON               ((unsigned int *)(INFRACFG_AO_BASE+0x0F80))

/* INFRACFG AO */
#define INFRA_AO_SEC_CG_CON0           ((unsigned int *)(INFRACFG_AO_BASE+0x0F84))
#define INFRA_AO_SEC_CG_CON1           ((unsigned int *)(INFRACFG_AO_BASE+0x0F88))
#define INFRA_AO_SEC_CG_CON2           ((unsigned int *)(INFRACFG_AO_BASE+0x0F9C))
#define INFRA_AO_SEC_CG_CON3           ((unsigned int *)(INFRACFG_AO_BASE+0x0FA4))

/* PERICFG */
/* #define PERICFG_SEC_CG_CON0            ((unsigned int *)(PERI_BASE+0x0400))
 * #define PERICFG_SEC_CG_CON1            ((unsigned int *)(PERI_BASE+0x0404))
 * #define PERICFG_SEC_CG_CON2            ((unsigned int *)(PERI_BASE+0x0408))
 * #define PERICFG_SEC_CG_CON3            ((unsigned int *)(PERI_BASE+0x040C))
 * #define PERICFG_SEC_CG_CON4            ((unsigned int *)(PERI_BASE+0x0410))
 * #define PERICFG_SEC_CG_CON5            ((unsigned int *)(PERI_BASE+0x0414))
 */

/* PMS(MD devapc) */
#define AP2MD1_PMS_CTRL_EN             ((unsigned int *)0x100018AC)
#define AP2MD1_PMS_CTRL_EN_LOCK        ((unsigned int *)0x100018A8)

/*******************************************************************************************/

#define SRAMROM_SEC_VIO_STA            ((unsigned int *)(SRAMROM_BASE+0x010))
#define SRAMROM_SEC_VIO_ADDR           ((unsigned int *)(SRAMROM_BASE+0x014))
#define SRAMROM_SEC_VIO_CLR            ((unsigned int *)(SRAMROM_BASE+0x018))

#define SRAMROM_ROM_SEC_VIO_STA        ((unsigned int *)(SRAMROM_BASE+0x110))
#define SRAMROM_ROM_SEC_VIO_ADDR       ((unsigned int *)(SRAMROM_BASE+0x114))
#define SRAMROM_ROM_SEC_VIO_CLR        ((unsigned int *)(SRAMROM_BASE+0x118))


/******************************************************************************
 *
 * SIP CTRL TYPE DEFINATION
 *
 ******************************************************************************/
enum DEVAPC_HYP_MODULE_REQ_TYPE {
	DEVAPC_HYP_MODULE_REQ_APU,
	DEVAPC_HYP_MODULE_REQ_NUM,
};

enum DEVAPC_MASTER_REQ_TYPE {
	DEVAPC_MASTER_REQ_SPI,
	DEVAPC_MASTER_REQ_NUM,
};

enum DEVAPC_PROTECT_ON_OFF {
	DEVAPC_PROTECT_DISABLE,
	DEVAPC_PROTECT_ENABLE,
};

enum DEVAPC_SIP_CTRL_TYPE {
	DEVAPC_SIP_MODULE_PERM_SET = 1,
	DEVAPC_SIP_MASTER_DOM_SET,
};

enum DEVAPC_APUSYS_CTRL_INDEX {
	APUSYS_COREA_CTRL_IDX = 201,
	APUSYS_COREB_CTRL_IDX = 206,
};

/******************************************************************************
 *
 * Variable DEFINATION

 ******************************************************************************/
/* If you config register INFRA_AO_SEC_CON(address 0x10000F80) bit[4] = 1,
 * the domain comes from device_apc. By default this register is 0,
 * the domain comes form MD1
 */
#define FORCE_MD1_SIGNAL_FROM_DAPC      ((0x1) << 4)


/* PROTECT BIT FOR INFRACFG AO */
#define SEJ_CG_PROTECT_BIT              ((0x1) << 5)
#define TRNG_CG_PROTECT_BIT             ((0x1) << 9)
#define DEVAPC_CG_PROTECT_BIT           ((0x1) << 20)
#define DXCC_SEC_CORE_CG_PROTECT_BIT    ((0x1) << 27)
#define DXCC_AO_CG_PROTECT_BIT          ((0x1) << 28)
#define AES_CG_PROTECT_BIT              ((0x1) << 29)

/* PROTECT BIT FOR PERICFG */
/* #define UFS_AES_CORE_PROTECT_BIT        ((0x1) << 12)
 * #define DEVAPC_PERI_PD_PROTECT_BIT      ((0x1) << 9)
 * #define DXCC_AO_CORE_PROTECT_BIT        ((0x1) << 16)
 * #define DXCC_Secure_CORE_PROTECT_BIT    ((0x1) << 18)
 */

#define SRAM_SEC_VIO_BIT                (0x1)
#define ROM_SEC_VIO_BIT                 (0x1)

/*******************************************************************************************/

#define MOD_NO_MM_DEVAPC		32
#define MOD_NO_IN_1_DEVAPC              16

#define MASTER_INFRA_SPM                10
#define MASTER_INFRA_SSPM               27
#define MASTER_INFRA_MAX_INDEX          32

#define SLAVE_INFRA_MAX_INDEX           231
#define SLAVE_MM_MAX_INDEX              246
#define SLAVE_MD_MAX_INDEX              45

#define E_DAPC_MM2ND_SLAVE		4
#define SLAVE_MM2ND_MAX_INDEX		103

#endif /* __ATF_DAPC_H__ */

