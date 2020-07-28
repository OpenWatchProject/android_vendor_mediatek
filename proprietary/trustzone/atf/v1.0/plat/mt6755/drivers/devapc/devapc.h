#ifndef _ATF_DAPC_H__
#define _ATF_DAPC_H__
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
     E_MAX,
     E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
 } E_MASK_DOM;


 typedef struct {
     const char          *device_name;
     APC_ATTR            d0_permission;
     APC_ATTR            d1_permission;
 } DEVICE_INFO;
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
#define SRAMROM_BASE           0x10202000

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

#define DEVAPC_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F00))
#define DEVAPC_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F04))
#define DEVAPC_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F08))
#define DEVAPC_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F0C))


/* Device APC PD */
#define DEVAPC_PD_APC_CON          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F00))

#define DEVAPC_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0000))
#define DEVAPC_D0_VIO_MASK_1       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0004))
#define DEVAPC_D0_VIO_MASK_2       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0008))
#define DEVAPC_D0_VIO_MASK_3       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x000C))
#define DEVAPC_D0_VIO_MASK_4       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0010))
#define DEVAPC_D0_VIO_MASK_5       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0014))

#define DEVAPC_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0400))
#define DEVAPC_D0_VIO_STA_1        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0404))
#define DEVAPC_D0_VIO_STA_2        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0408))
#define DEVAPC_D0_VIO_STA_3        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x040C))
#define DEVAPC_D0_VIO_STA_4        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0410))
#define DEVAPC_D0_VIO_STA_5        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0414))

#define DEVAPC_VIO_DBG0            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0900))
#define DEVAPC_VIO_DBG1            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0904))
#define DEVAPC_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F80))
#define DEVAPC_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F84))
#define DEVAPC_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F88))

/* Device APC Infra_AO */
#define INFRA_AO_SEC_CG_CON0        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F84))
#define INFRA_AO_SEC_CG_CON1        ((volatile unsigned int*)(INFRACFG_AO_base+0x0F88))

/* SRAMROM */
#define SRAMROM_SEC_VIO_STA         ((volatile unsigned int*)(SRAMROM_BASE+0x010))
#define SRAMROM_SEC_VIO_ADDR        ((volatile unsigned int*)(SRAMROM_BASE+0x014))
#define SRAMROM_SEC_VIO_CLR         ((volatile unsigned int*)(SRAMROM_BASE+0x018))

/******************************************************************************
*
 * Variable DEFINATION

******************************************************************************/

#define SEJ_CG_PROTECT_BIT         ((0x1) << 5)
#define TRNG_CG_PROTECT_BIT        ((0x1) << 9)

#define MOD_NO_IN_1_DEVAPC              16

#define SECURE_TRANSACTION              1
#define NON_SECURE_TRANSACTION          0

#define MASTER_SPM_PDN                  27

#define SLAVE_SRAMROM_INDEX             157

#endif
