#ifndef _ATF_DAPC_H__
#define _ATF_DAPC_H__
/******************************************************************************
*
 * FUNCTION DEFINATION

******************************************************************************/
int start_devapc(void);


/******************************************************************************
*
 * STRUCTURE DEFINATION

******************************************************************************/
typedef enum
{
    E_L0=0,
    E_L1,
    E_L2,
    E_L3,
    E_MAX_APC_ATTR,
    E_APC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} APC_ATTR;

// domain index
typedef enum
{
    E_DOMAIN_0 = 0,
    E_DOMAIN_1 ,
    E_DOMAIN_2 ,
    E_DOMAIN_3 ,
    E_DOMAIN_4 ,
    E_DOMAIN_5 ,
    E_DOMAIN_6 ,
    E_DOMAIN_7 ,
    E_MAX_MASK_DOM,
    E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} E_MASK_DOM;

typedef struct {
    unsigned char       d0_permission;
    unsigned char       d1_permission;
} DEVICE_INFO;

#define DAPC_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2}

typedef enum
{
    E_NonSec=0,
    E_Sec,
    E_MAX_MDAPC_ATTR,
    E_MDAPC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
} MDAPC_ATTR;


typedef struct {
    MDAPC_ATTR          read_permission;
    MDAPC_ATTR          write_permission;
} MD_MODULE_INFO;

#define MD_ATTR(DEV_NAME, PERM_ATTR1, PERM_ATTR2) {(unsigned char)PERM_ATTR1, (unsigned char)PERM_ATTR2}
/******************************************************************************
*
 * UTILITY DEFINATION

******************************************************************************/

#define READ_REGISTER_UINT32(reg) \
        (*(volatile unsigned int * const)(uintptr_t)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
        (*(volatile unsigned int * const)(uintptr_t)(reg)) = (val)

#define writel(VAL,REG)  WRITE_REGISTER_UINT32(REG,VAL)
#define readl(REG)       READ_REGISTER_UINT32(REG)

/******************************************************************************
*
 * REGISTER ADDRESS DEFINATION

******************************************************************************/
#define DEVAPC_AO_BASE         0x1000E000
#define DEVAPC_PD_BASE         0x10207000
#define INFRACFG_AO_base       0x10001000

/* Device APC AO */
#define DEVAPC_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0000))
#define DEVAPC_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0100))
#define DEVAPC_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0200))
#define DEVAPC_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0300))
#define DEVAPC_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))
#define DEVAPC_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))
#define DEVAPC_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0600))
#define DEVAPC_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0700))

#define DEVAPC_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A00))
#define DEVAPC_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A04))
#define DEVAPC_MAS_DOM_2           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A08))
#define DEVAPC_MAS_DOM_3           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A0C))

#define DEVAPC_MAS_SEC_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B00))

#define DEVAPC_MD_SEC_EN_0         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D00))
#define DEVAPC_MD_SEC_EN_1         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D04))
#define DEVAPC_MD_SEC_EN_R_0       ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D08))
#define DEVAPC_MD_SEC_EN_R_1       ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0D0C))

#define DEVAPC_MM_SEC_EN_0         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E00))
#define DEVAPC_MM_SEC_EN_1         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E04))
#define DEVAPC_MM_SEC_EN_2         ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0E08))

#define DEVAPC_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F00))
#define DEVAPC_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F04))
#define DEVAPC_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F08))
#define DEVAPC_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F0C))
#define DEVAPC_APC_LOCK_3          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F10))
#define DEVAPC_APC_LOCK_4          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F14))
#define DEVAPC_APC_LOCK_5          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F18))
#define DEVAPC_APC_LOCK_6          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F1C))


/* Device APC PD */
#define DEVAPC_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0000))
#define DEVAPC_D0_VIO_MASK_1       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0004))
#define DEVAPC_D0_VIO_MASK_2       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0008))
#define DEVAPC_D0_VIO_MASK_3       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x000C))
#define DEVAPC_D0_VIO_MASK_4       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0010))
#define DEVAPC_D0_VIO_MASK_5       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0014))
#define DEVAPC_D0_VIO_MASK_6       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0018))
#define DEVAPC_D0_VIO_MASK_7       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x001C))

#define DEVAPC_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0400))
#define DEVAPC_D0_VIO_STA_1        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0404))
#define DEVAPC_D0_VIO_STA_2        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0408))
#define DEVAPC_D0_VIO_STA_3        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x040C))
#define DEVAPC_D0_VIO_STA_4        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0410))
#define DEVAPC_D0_VIO_STA_5        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0414))
#define DEVAPC_D0_VIO_STA_6        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0418))
#define DEVAPC_D0_VIO_STA_7        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x041C))

#define DEVAPC_VIO_DBG0            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0900))
#define DEVAPC_VIO_DBG1            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0904))

#define DEVAPC_PD_APC_CON          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F00))

#define DEVAPC_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F80))
#define DEVAPC_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F84))
#define DEVAPC_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F88))


/* Device APC Infra_AO */
#define INFRA_AO_SEC_CG_CON0        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F84))
#define INFRA_AO_SEC_CG_CON1        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F88))

/******************************************************************************
*
 * Variable DEFINATION

******************************************************************************/

#define SEJ_CG_PROTECT_BIT         ((0x1) << 5)
#define TRNG_CG_PROTECT_BIT        ((0x1) << 9)

#define MOD_NO_IN_1_DEVAPC              16

#define SECURE_TRANSACTION              1
#define NON_SECURE_TRANSACTION          0

#define MASTER_SPM                      10

#define MASTER_MAX_INDEX                29


#endif
